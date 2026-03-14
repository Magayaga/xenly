/*
 * XENLY - Xenly Virtual Machine (XVM)
 * VM: Stack-based executor
 *
 * The XVM is a register-less, stack-based virtual machine.
 * Each call frame owns a local Env scope and a value stack.
 */
package vm

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"math"
	"os"
	"strings"
	"time"

	"xvm/pkg/bytecode"
)

// XVM is the Xenly Virtual Machine.
type XVM struct {
	module  *bytecode.Module
	global  *Env
	classes []*ClassVal
	fns     []*FunctionVal
	reader  *bufio.Reader
}

// NewXVM creates a new XVM from a compiled module.
func NewXVM(mod *bytecode.Module) *XVM {
	xvm := &XVM{
		module: mod,
		global: NewEnv(nil),
		reader: bufio.NewReader(os.Stdin),
	}
	RegisterBuiltins(xvm.global)
	xvm.buildFunctions()
	xvm.buildClasses()
	return xvm
}

func (xvm *XVM) buildFunctions() {
	xvm.fns = make([]*FunctionVal, len(xvm.module.Functions))
	for i, fn := range xvm.module.Functions {
		params := make([]ParamDef, len(fn.Params))
		for j, p := range fn.Params {
			params[j] = ParamDef{
				Name:       xvm.poolStr(p.NameIdx),
				HasDefault: p.HasDefault,
				IsRest:     p.IsRest,
			}
		}
		xvm.fns[i] = &FunctionVal{
			Name:    xvm.poolStr(fn.NameIdx),
			Params:  params,
			Code:    fn.Code,
			IsAsync: fn.IsAsync,
		}
	}
}

func (xvm *XVM) buildClasses() {
	xvm.classes = make([]*ClassVal, len(xvm.module.Classes))
	for i, cd := range xvm.module.Classes {
		xvm.classes[i] = &ClassVal{
			Name:    xvm.poolStr(cd.NameIdx),
			Methods: make(map[string]*FunctionVal),
		}
	}
	for i, cd := range xvm.module.Classes {
		cls := xvm.classes[i]
		if cd.HasParent {
			parentName := xvm.poolStr(cd.ParentIdx)
			for _, c2 := range xvm.classes {
				if c2.Name == parentName {
					cls.Parent = c2
					break
				}
			}
		}
		for _, mth := range cd.Methods {
			if int(mth.FnIdx) < len(xvm.fns) {
				cls.Methods[xvm.poolStr(mth.NameIdx)] = xvm.fns[mth.FnIdx]
			}
		}
	}
}

func (xvm *XVM) poolStr(idx uint32) string {
	if int(idx) >= len(xvm.module.Pool) {
		return ""
	}
	c := xvm.module.Pool[idx]
	if c.Type == bytecode.ConstString {
		return c.StrVal
	}
	return ""
}

func (xvm *XVM) poolVal(idx uint32) *Value {
	if int(idx) >= len(xvm.module.Pool) {
		return Null()
	}
	c := xvm.module.Pool[idx]
	switch c.Type {
	case bytecode.ConstNull:
		return Null()
	case bytecode.ConstNumber:
		return Number(c.NumVal)
	case bytecode.ConstString:
		return String(c.StrVal)
	case bytecode.ConstBool:
		return Bool(c.BoolVal)
	}
	return Null()
}

// Run executes the module's main code chunk.
func (xvm *XVM) Run() error {
	_, err := xvm.exec(xvm.module.MainCode, xvm.global, nil, nil)
	return err
}

// exec runs a bytecode chunk inside a given environment.
func (xvm *XVM) exec(code []byte, env *Env, thisVal *Value, class *ClassVal) (*Value, error) {
	stack := make([]*Value, 0, 64)
	scopes := []*Env{env}

	push := func(v *Value) {
		if v == nil {
			v = Null()
		}
		stack = append(stack, v)
	}
	pop := func() *Value {
		if len(stack) == 0 {
			return Null()
		}
		v := stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		return v
	}
	peek := func() *Value {
		if len(stack) == 0 {
			return Null()
		}
		return stack[len(stack)-1]
	}
	currentEnv := func() *Env { return scopes[len(scopes)-1] }

	readU8 := func(ip *int) uint8 {
		v := code[*ip]
		*ip++
		return v
	}
	readU32 := func(ip *int) uint32 {
		v := binary.LittleEndian.Uint32(code[*ip : *ip+4])
		*ip += 4
		return v
	}
	readI32 := func(ip *int) int32 { return int32(readU32(ip)) }

	ip := 0
	for ip < len(code) {
		op := bytecode.Opcode(code[ip])
		ip++

		switch op {

		// ── Stack ──────────────────────────────────────────────────────────
		case bytecode.OP_NOP:
		case bytecode.OP_HALT:
			return Null(), nil
		case bytecode.OP_POP:
			pop()
		case bytecode.OP_DUP:
			push(peek())
		case bytecode.OP_SWAP:
			if len(stack) >= 2 {
				n := len(stack)
				stack[n-1], stack[n-2] = stack[n-2], stack[n-1]
			}

		// ── Literals ────────────────────────────────────────────────────────
		case bytecode.OP_PUSH_NULL:
			push(Null())
		case bytecode.OP_PUSH_TRUE:
			push(True())
		case bytecode.OP_PUSH_FALSE:
			push(False())
		case bytecode.OP_PUSH_CONST:
			push(xvm.poolVal(readU32(&ip)))

		// ── Arithmetic ──────────────────────────────────────────────────────
		case bytecode.OP_ADD:
			b, a := pop(), pop()
			if a.Tag == TypeString || b.Tag == TypeString {
				push(String(a.String() + b.String()))
			} else {
				push(Number(a.NumVal + b.NumVal))
			}
		case bytecode.OP_SUB:
			b, a := pop(), pop()
			push(Number(a.NumVal - b.NumVal))
		case bytecode.OP_MUL:
			b, a := pop(), pop()
			push(Number(a.NumVal * b.NumVal))
		case bytecode.OP_DIV:
			b, a := pop(), pop()
			if b.NumVal == 0 {
				push(Number(math.NaN()))
			} else {
				push(Number(a.NumVal / b.NumVal))
			}
		case bytecode.OP_MOD:
			b, a := pop(), pop()
			push(Number(math.Mod(a.NumVal, b.NumVal)))
		case bytecode.OP_POW:
			b, a := pop(), pop()
			push(Number(math.Pow(a.NumVal, b.NumVal)))
		case bytecode.OP_NEG:
			push(Number(-pop().NumVal))

		// ── Bitwise ─────────────────────────────────────────────────────────
		case bytecode.OP_BIT_AND:
			b, a := pop(), pop()
			push(Number(float64(int64(a.NumVal) & int64(b.NumVal))))
		case bytecode.OP_BIT_OR:
			b, a := pop(), pop()
			push(Number(float64(int64(a.NumVal) | int64(b.NumVal))))
		case bytecode.OP_BIT_XOR:
			b, a := pop(), pop()
			push(Number(float64(int64(a.NumVal) ^ int64(b.NumVal))))
		case bytecode.OP_BIT_NOT:
			push(Number(float64(^int64(pop().NumVal))))
		case bytecode.OP_SHL:
			b, a := pop(), pop()
			push(Number(float64(int64(a.NumVal) << uint(b.NumVal))))
		case bytecode.OP_SHR:
			b, a := pop(), pop()
			push(Number(float64(int64(a.NumVal) >> uint(b.NumVal))))

		// ── Comparison ──────────────────────────────────────────────────────
		case bytecode.OP_EQ:
			b, a := pop(), pop()
			push(Bool(a.Equal(b)))
		case bytecode.OP_NEQ:
			b, a := pop(), pop()
			push(Bool(!a.Equal(b)))
		case bytecode.OP_LT:
			b, a := pop(), pop()
			if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal < b.StrVal))
			} else {
				push(Bool(a.NumVal < b.NumVal))
			}
		case bytecode.OP_LE:
			b, a := pop(), pop()
			if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal <= b.StrVal))
			} else {
				push(Bool(a.NumVal <= b.NumVal))
			}
		case bytecode.OP_GT:
			b, a := pop(), pop()
			if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal > b.StrVal))
			} else {
				push(Bool(a.NumVal > b.NumVal))
			}
		case bytecode.OP_GE:
			b, a := pop(), pop()
			if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal >= b.StrVal))
			} else {
				push(Bool(a.NumVal >= b.NumVal))
			}

		// ── Logical ─────────────────────────────────────────────────────────
		case bytecode.OP_AND:
			b, a := pop(), pop()
			if a.Truthy() {
				push(b)
			} else {
				push(a)
			}
		case bytecode.OP_OR:
			b, a := pop(), pop()
			if a.Truthy() {
				push(a)
			} else {
				push(b)
			}
		case bytecode.OP_NOT:
			push(Bool(!pop().Truthy()))
		case bytecode.OP_NULLISH:
			b, a := pop(), pop()
			if a.IsNull() {
				push(b)
			} else {
				push(a)
			}

		// ── Variables ────────────────────────────────────────────────────────
		case bytecode.OP_LOAD:
			name := xvm.poolStr(readU32(&ip))
			push(currentEnv().MustGet(name))
		case bytecode.OP_STORE:
			name := xvm.poolStr(readU32(&ip))
			if err := currentEnv().Set(name, peek()); err != nil {
				return Null(), fmt.Errorf("line ?: %w", err)
			}
		case bytecode.OP_STORE_NEW, bytecode.OP_STORE_LET:
			name := xvm.poolStr(readU32(&ip))
			currentEnv().Define(name, pop(), false)
		case bytecode.OP_STORE_CONST:
			name := xvm.poolStr(readU32(&ip))
			currentEnv().Define(name, pop(), true)

		// ── Control flow ─────────────────────────────────────────────────────
		case bytecode.OP_JUMP:
			ip = int(readI32(&ip))
		case bytecode.OP_JUMP_TRUE:
			addr := readI32(&ip)
			if pop().Truthy() {
				ip = int(addr)
			}
		case bytecode.OP_JUMP_FALSE:
			addr := readI32(&ip)
			if !pop().Truthy() {
				ip = int(addr)
			}
		case bytecode.OP_JUMP_NULL:
			addr := readI32(&ip)
			if peek().IsNull() {
				ip = int(addr)
			}

		// ── Scope ────────────────────────────────────────────────────────────
		case bytecode.OP_PUSH_SCOPE:
			scopes = append(scopes, NewEnv(currentEnv()))
		case bytecode.OP_POP_SCOPE:
			if len(scopes) > 1 {
				scopes = scopes[:len(scopes)-1]
			}

		// ── Functions ────────────────────────────────────────────────────────
		case bytecode.OP_MAKE_FN:
			idx := readU32(&ip)
			if int(idx) < len(xvm.fns) {
				push(&Value{Tag: TypeFunction, FnVal: xvm.fns[idx]})
			} else {
				push(Null())
			}
		case bytecode.OP_MAKE_CLOSURE:
			idx := readU32(&ip)
			if int(idx) < len(xvm.fns) {
				fn := xvm.fns[idx]
				closure := &FunctionVal{
					Name:    fn.Name,
					Params:  fn.Params,
					Code:    fn.Code,
					IsAsync: fn.IsAsync,
					Closure: currentEnv(),
				}
				push(&Value{Tag: TypeFunction, FnVal: closure})
			} else {
				push(Null())
			}
		case bytecode.OP_CALL:
			argc := int(readU8(&ip))
			args := make([]*Value, argc)
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
			callee := pop()
			result, err := xvm.callValue(callee, args, nil, nil)
			if err != nil {
				return Null(), err
			}
			push(result)
		case bytecode.OP_CALL_METHOD:
			nameIdx := readU32(&ip)
			argc := int(readU8(&ip))
			args := make([]*Value, argc)
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
			receiver := pop()
			result, err := xvm.callMethod(receiver, xvm.poolStr(nameIdx), args)
			if err != nil {
				return Null(), err
			}
			push(result)
		case bytecode.OP_RETURN:
			return Null(), nil
		case bytecode.OP_RETURN_VAL:
			return pop(), nil

		// ── Arrays ───────────────────────────────────────────────────────────
		case bytecode.OP_MAKE_ARRAY:
			count := int(readU32(&ip))
			items := make([]*Value, count)
			for i := count - 1; i >= 0; i-- {
				items[i] = pop()
			}
			push(Array(items))
		case bytecode.OP_INDEX_GET:
			idx := pop()
			arr := pop()
			push(xvm.indexGet(arr, idx))
		case bytecode.OP_INDEX_SET:
			val := pop()
			idx := pop()
			arr := pop()
			xvm.indexSet(arr, idx, val)
		case bytecode.OP_ARRAY_LEN:
			a := pop()
			if a.Tag == TypeArray {
				push(Number(float64(len(a.ArrayVal))))
			} else if a.Tag == TypeString {
				push(Number(float64(len([]rune(a.StrVal)))))
			} else {
				push(Number(0))
			}

		// ── Objects ──────────────────────────────────────────────────────────
		case bytecode.OP_MAKE_OBJ:
			count := int(readU32(&ip))
			m := make(map[string]*Value, count)
			// pairs pushed as (key, val) * count
			pairs := make([]*Value, count*2)
			for i := count*2 - 1; i >= 0; i-- {
				pairs[i] = pop()
			}
			for i := 0; i < count; i++ {
				k := pairs[i*2].String()
				v := pairs[i*2+1]
				m[k] = v
			}
			push(Object(m))
		case bytecode.OP_PROP_GET:
			name := xvm.poolStr(readU32(&ip))
			push(xvm.propGet(pop(), name))
		case bytecode.OP_PROP_SET:
			name := xvm.poolStr(readU32(&ip))
			val := pop()
			obj := pop()
			xvm.propSet(obj, name, val)
		case bytecode.OP_PROP_GET_DYN:
			name := pop()
			push(xvm.propGet(pop(), name.String()))
		case bytecode.OP_PROP_SET_DYN:
			val := pop()
			name := pop()
			xvm.propSet(pop(), name.String(), val)

		// ── OOP ──────────────────────────────────────────────────────────────
		case bytecode.OP_DEFINE_CLASS:
			idx := readU32(&ip)
			if int(idx) < len(xvm.classes) {
				cls := xvm.classes[idx]
				// Attach closure to all methods
				for _, fn := range cls.Methods {
					if fn.Closure == nil {
						fn.Closure = currentEnv()
					}
				}
				push(&Value{Tag: TypeClass, ClassVal: cls})
			} else {
				push(Null())
			}
		case bytecode.OP_NEW:
			nameIdx := readU32(&ip)
			argc := int(readU8(&ip))
			args := make([]*Value, argc)
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
			result, err := xvm.instantiateClass(xvm.poolStr(nameIdx), args, currentEnv())
			if err != nil {
				return Null(), err
			}
			push(result)
		case bytecode.OP_THIS_LOAD:
			if thisVal != nil {
				push(thisVal)
			} else {
				push(Null())
			}
		case bytecode.OP_SUPER_CALL:
			argc := int(readU8(&ip))
			args := make([]*Value, argc)
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
			// Use the class we are *currently executing in* (not thisVal.Class)
			// so that deep chains (Square→Rect→Shape) step up exactly one level.
			var superParent *ClassVal
			if class != nil {
				superParent = class.Parent
			} else if thisVal != nil && thisVal.Tag == TypeInstance && thisVal.InstVal != nil {
				superParent = thisVal.InstVal.Class.Parent
			}
			if superParent != nil {
				if initFn, ok := superParent.Methods["init"]; ok {
					callEnv := NewEnv(currentEnv())
					xvm.bindArgs(initFn, args, callEnv)
					xvm.exec(initFn.Code, callEnv, thisVal, superParent)
				}
			}
		case bytecode.OP_TYPEOF:
			push(String(pop().TypeName()))
		case bytecode.OP_INSTANCEOF:
			name := xvm.poolStr(readU32(&ip))
			a := pop()
			result := false
			if a.Tag == TypeInstance && a.InstVal != nil {
				for cls := a.InstVal.Class; cls != nil; cls = cls.Parent {
					if cls.Name == name {
						result = true
						break
					}
				}
			}
			push(Bool(result))

		// ── Built-in I/O ─────────────────────────────────────────────────────
		case bytecode.OP_PRINT:
			count := int(readU8(&ip))
			parts := make([]string, count)
			vals := make([]*Value, count)
			for i := count - 1; i >= 0; i-- {
				vals[i] = pop()
			}
			for i, v := range vals {
				parts[i] = v.String()
			}
			fmt.Fprintln(stdout, strings.Join(parts, " "))
		case bytecode.OP_INPUT:
			hasPrompt := readU8(&ip)
			if hasPrompt != 0 {
				fmt.Fprint(stdout, pop().String())
				if f, ok := stdout.(interface{ Flush() error }); ok {
					f.Flush()
				}
			}
			line, _ := xvm.reader.ReadString('\n')
			push(String(strings.TrimRight(line, "\r\n")))
		case bytecode.OP_SLEEP:
			ms := pop().NumVal
			time.Sleep(time.Duration(ms) * time.Millisecond)
		case bytecode.OP_ASSERT:
			msg := pop()
			cond := pop()
			if !cond.Truthy() {
				return Null(), fmt.Errorf("XVM assertion failed: %s", msg.String())
			}

		// ── Enum / Pattern matching ───────────────────────────────────────────
		case bytecode.OP_MAKE_VARIANT:
			tagIdx := readU32(&ip)
			fieldCount := int(readU8(&ip))
			fields := make([]*Value, fieldCount)
			for i := fieldCount - 1; i >= 0; i-- {
				fields[i] = pop()
			}
			push(&Value{
				Tag:       TypeVariant,
				VarTag:    xvm.poolStr(tagIdx),
				VarFields: fields,
			})
		case bytecode.OP_MATCH_TAG:
			tagName := xvm.poolStr(readU32(&ip))
			subject := pop() // pop subject, push bool result
			push(Bool(subject.Tag == TypeVariant && subject.VarTag == tagName))
		case bytecode.OP_EXTRACT_FIELD:
			fieldIdx := int(readU8(&ip))
			v := peek() // peek: keep variant on stack for multiple extractions
			if v.Tag == TypeVariant && fieldIdx < len(v.VarFields) {
				push(v.VarFields[fieldIdx])
			} else {
				push(Null())
			}

		// ── Iteration ─────────────────────────────────────────────────────────
		case bytecode.OP_ITER_BEGIN:
			arr := pop()
			var items []*Value
			switch arr.Tag {
			case TypeArray:
				items = arr.ArrayVal
			case TypeString:
				runes := []rune(arr.StrVal)
				items = make([]*Value, len(runes))
				for i, r := range runes {
					items[i] = String(string(r))
				}
			case TypeObject:
				for k := range arr.ObjVal {
					items = append(items, String(k))
				}
			}
			push(&Value{Tag: TypeIterator, IterVal: &IterVal{Items: items, Index: -1}})
		case bytecode.OP_ITER_NEXT:
			addr := readI32(&ip)
			it := peek()
			if it.Tag != TypeIterator {
				ip = int(addr)
				continue
			}
			it.IterVal.Index++
			if it.IterVal.Index >= len(it.IterVal.Items) {
				ip = int(addr)
			}
		case bytecode.OP_ITER_VALUE:
			it := peek()
			if it.Tag == TypeIterator && it.IterVal.Index < len(it.IterVal.Items) {
				push(it.IterVal.Items[it.IterVal.Index])
			} else {
				push(Null())
			}

		// ── Misc ─────────────────────────────────────────────────────────────
		case bytecode.OP_TO_STRING:
			push(String(pop().String()))
		case bytecode.OP_CONCAT_STR:
			count := int(readU8(&ip))
			parts := make([]string, count)
			for i := count - 1; i >= 0; i-- {
				parts[i] = pop().String()
			}
			push(String(strings.Join(parts, "")))
		case bytecode.OP_INC:
			name := xvm.poolStr(readU32(&ip))
			v := currentEnv().MustGet(name)
			currentEnv().Set(name, Number(v.NumVal+1))
		case bytecode.OP_DEC:
			name := xvm.poolStr(readU32(&ip))
			v := currentEnv().MustGet(name)
			currentEnv().Set(name, Number(v.NumVal-1))
		case bytecode.OP_SPREAD:
			arr := pop()
			if arr.Tag == TypeArray {
				for _, item := range arr.ArrayVal {
					push(item)
				}
			} else {
				push(arr)
			}

		default:
			return Null(), fmt.Errorf("XVM: unknown opcode 0x%02X at ip=%d", op, ip-1)
		}
	}
	return Null(), nil
}

// ─── Call helpers ─────────────────────────────────────────────────────────────

func (xvm *XVM) callValue(callee *Value, args []*Value, thisVal *Value, class *ClassVal) (*Value, error) {
	if callee == nil || callee.Tag == TypeNull {
		return Null(), fmt.Errorf("XVM: attempt to call null value")
	}
	switch callee.Tag {
	case TypeFunction:
		return xvm.callFn(callee.FnVal, args, thisVal, class)
	case TypeBuiltin:
		return callee.BuiltinV(args)
	case TypeClass:
		return xvm.instantiateClassVal(callee.ClassVal, args)
	}
	return Null(), fmt.Errorf("XVM: value of type %q is not callable", callee.TypeName())
}

func (xvm *XVM) callFn(fn *FunctionVal, args []*Value, thisVal *Value, class *ClassVal) (*Value, error) {
	if fn == nil {
		return Null(), fmt.Errorf("XVM: nil function")
	}
	parent := fn.Closure
	if parent == nil {
		parent = xvm.global
	}
	callEnv := NewEnv(parent)
	xvm.bindArgs(fn, args, callEnv)
	return xvm.exec(fn.Code, callEnv, thisVal, class)
}

func (xvm *XVM) bindArgs(fn *FunctionVal, args []*Value, env *Env) {
	for i, p := range fn.Params {
		if p.IsRest {
			rest := make([]*Value, 0)
			for j := i; j < len(args); j++ {
				rest = append(rest, args[j])
			}
			env.Define(p.Name, Array(rest), false)
			return
		}
		val := Null()
		if i < len(args) {
			val = args[i]
		}
		env.Define(p.Name, val, false)
	}
}

func (xvm *XVM) callMethod(receiver *Value, name string, args []*Value) (*Value, error) {
	if receiver == nil || receiver.Tag == TypeNull {
		return Null(), fmt.Errorf("XVM: cannot call method %q on null", name)
	}

	// Check built-in methods first
	if result, handled, err := xvm.builtinMethod(receiver, name, args); handled {
		return result, err
	}

	// Instance method dispatch
	if receiver.Tag == TypeInstance && receiver.InstVal != nil {
		for cls := receiver.InstVal.Class; cls != nil; cls = cls.Parent {
			if fn, ok := cls.Methods[name]; ok {
				return xvm.callFn(fn, args, receiver, cls)
			}
		}
		// Check if a field is callable
		if fv, ok := receiver.InstVal.Fields[name]; ok {
			return xvm.callValue(fv, args, receiver, receiver.InstVal.Class)
		}
		return Null(), fmt.Errorf("XVM: method %q not found on %s", name, receiver.InstVal.Class.Name)
	}

	// Class static call
	if receiver.Tag == TypeClass && receiver.ClassVal != nil {
		if fn, ok := receiver.ClassVal.Methods[name]; ok {
			return xvm.callFn(fn, args, nil, receiver.ClassVal)
		}
	}

	// Object with callable field
	if receiver.Tag == TypeObject && receiver.ObjVal != nil {
		if fv, ok := receiver.ObjVal[name]; ok {
			return xvm.callValue(fv, args, receiver, nil)
		}
	}

	return Null(), fmt.Errorf("XVM: cannot call method %q on %s", name, receiver.TypeName())
}

// ─── Property access ──────────────────────────────────────────────────────────

func (xvm *XVM) propGet(obj *Value, name string) *Value {
	if obj == nil || obj.Tag == TypeNull {
		return Null()
	}
	switch obj.Tag {
	case TypeObject:
		if v, ok := obj.ObjVal[name]; ok {
			return v
		}
	case TypeInstance:
		if obj.InstVal != nil {
			if v, ok := obj.InstVal.Fields[name]; ok {
				return v
			}
			// Look for a method and return it as a bound function value
			for cls := obj.InstVal.Class; cls != nil; cls = cls.Parent {
				if fn, ok := cls.Methods[name]; ok {
					return &Value{Tag: TypeFunction, FnVal: fn}
				}
			}
		}
	case TypeClass:
		if obj.ClassVal != nil {
			if fn, ok := obj.ClassVal.Methods[name]; ok {
				return &Value{Tag: TypeFunction, FnVal: fn}
			}
		}
	case TypeArray:
		switch name {
		case "length", "len":
			return Number(float64(len(obj.ArrayVal)))
		}
	case TypeString:
		switch name {
		case "length", "len":
			return Number(float64(len([]rune(obj.StrVal))))
		}
	}
	return Null()
}

func (xvm *XVM) propSet(obj *Value, name string, val *Value) {
	if obj == nil {
		return
	}
	switch obj.Tag {
	case TypeObject:
		if obj.ObjVal == nil {
			obj.ObjVal = make(map[string]*Value)
		}
		obj.ObjVal[name] = val
	case TypeInstance:
		if obj.InstVal != nil {
			if obj.InstVal.Fields == nil {
				obj.InstVal.Fields = make(map[string]*Value)
			}
			obj.InstVal.Fields[name] = val
		}
	}
}

// ─── Index access ─────────────────────────────────────────────────────────────

func (xvm *XVM) indexGet(container *Value, idx *Value) *Value {
	if container == nil {
		return Null()
	}
	switch container.Tag {
	case TypeArray:
		i := int(idx.NumVal)
		if i < 0 {
			i = len(container.ArrayVal) + i
		}
		if i >= 0 && i < len(container.ArrayVal) {
			return container.ArrayVal[i]
		}
	case TypeString:
		runes := []rune(container.StrVal)
		i := int(idx.NumVal)
		if i < 0 {
			i = len(runes) + i
		}
		if i >= 0 && i < len(runes) {
			return String(string(runes[i]))
		}
	case TypeObject:
		if v, ok := container.ObjVal[idx.String()]; ok {
			return v
		}
	case TypeInstance:
		if container.InstVal != nil {
			if v, ok := container.InstVal.Fields[idx.String()]; ok {
				return v
			}
		}
	}
	return Null()
}

func (xvm *XVM) indexSet(container *Value, idx *Value, val *Value) {
	if container == nil {
		return
	}
	switch container.Tag {
	case TypeArray:
		i := int(idx.NumVal)
		if i < 0 {
			i = len(container.ArrayVal) + i
		}
		if i >= 0 && i < len(container.ArrayVal) {
			container.ArrayVal[i] = val
		} else if i == len(container.ArrayVal) {
			container.ArrayVal = append(container.ArrayVal, val)
		}
	case TypeObject:
		if container.ObjVal == nil {
			container.ObjVal = make(map[string]*Value)
		}
		container.ObjVal[idx.String()] = val
	}
}

// ─── Class instantiation ──────────────────────────────────────────────────────

func (xvm *XVM) instantiateClass(name string, args []*Value, env *Env) (*Value, error) {
	// Find class in environment first (may be user-defined after module load)
	if cv, ok := env.Get(name); ok && cv.Tag == TypeClass {
		return xvm.instantiateClassVal(cv.ClassVal, args)
	}
	// Fall back to module class table
	for _, cls := range xvm.classes {
		if cls.Name == name {
			return xvm.instantiateClassVal(cls, args)
		}
	}
	return Null(), fmt.Errorf("XVM: class %q not found", name)
}

func (xvm *XVM) instantiateClassVal(cls *ClassVal, args []*Value) (*Value, error) {
	if cls == nil {
		return Null(), fmt.Errorf("XVM: nil class")
	}
	inst := &InstanceVal{
		Class:  cls,
		Fields: make(map[string]*Value),
	}
	instVal := &Value{Tag: TypeInstance, InstVal: inst}

	// Call init method if present
	if initFn, ok := cls.Methods["init"]; ok {
		_, err := xvm.callFn(initFn, args, instVal, cls)
		if err != nil {
			return Null(), err
		}
	}
	return instVal, nil
}

// ─── Built-in method dispatch ─────────────────────────────────────────────────

func (xvm *XVM) builtinMethod(receiver *Value, name string, args []*Value) (*Value, bool, error) {
	switch receiver.Tag {
	case TypeArray:
		return xvm.arrayMethod(receiver, name, args)
	case TypeString:
		return xvm.stringMethod(receiver, name, args)
	case TypeObject:
		return xvm.objectMethod(receiver, name, args)
	}
	return nil, false, nil
}

func (xvm *XVM) arrayMethod(arr *Value, name string, args []*Value) (*Value, bool, error) {
	switch name {
	case "push":
		for _, a := range args {
			arr.ArrayVal = append(arr.ArrayVal, a)
		}
		return Number(float64(len(arr.ArrayVal))), true, nil
	case "pop":
		if len(arr.ArrayVal) == 0 {
			return Null(), true, nil
		}
		v := arr.ArrayVal[len(arr.ArrayVal)-1]
		arr.ArrayVal = arr.ArrayVal[:len(arr.ArrayVal)-1]
		return v, true, nil
	case "shift":
		if len(arr.ArrayVal) == 0 {
			return Null(), true, nil
		}
		v := arr.ArrayVal[0]
		arr.ArrayVal = arr.ArrayVal[1:]
		return v, true, nil
	case "unshift":
		newItems := append(args, arr.ArrayVal...)
		arr.ArrayVal = newItems
		return Number(float64(len(arr.ArrayVal))), true, nil
	case "length", "len":
		return Number(float64(len(arr.ArrayVal))), true, nil
	case "join":
		sep := ", "
		if len(args) > 0 {
			sep = args[0].String()
		}
		parts := make([]string, len(arr.ArrayVal))
		for i, v := range arr.ArrayVal {
			parts[i] = v.String()
		}
		return String(strings.Join(parts, sep)), true, nil
	case "reverse":
		for i, j := 0, len(arr.ArrayVal)-1; i < j; i, j = i+1, j-1 {
			arr.ArrayVal[i], arr.ArrayVal[j] = arr.ArrayVal[j], arr.ArrayVal[i]
		}
		return arr, true, nil
	case "slice":
		start, end := 0, len(arr.ArrayVal)
		if len(args) > 0 {
			start = int(args[0].NumVal)
		}
		if len(args) > 1 {
			end = int(args[1].NumVal)
		}
		if start < 0 {
			start = len(arr.ArrayVal) + start
		}
		if end < 0 {
			end = len(arr.ArrayVal) + end
		}
		if start < 0 {
			start = 0
		}
		if end > len(arr.ArrayVal) {
			end = len(arr.ArrayVal)
		}
		newArr := make([]*Value, end-start)
		copy(newArr, arr.ArrayVal[start:end])
		return Array(newArr), true, nil
	case "indexOf":
		if len(args) == 0 {
			return Number(-1), true, nil
		}
		for i, v := range arr.ArrayVal {
			if v.Equal(args[0]) {
				return Number(float64(i)), true, nil
			}
		}
		return Number(-1), true, nil
	case "includes", "contains":
		if len(args) == 0 {
			return False(), true, nil
		}
		for _, v := range arr.ArrayVal {
			if v.Equal(args[0]) {
				return True(), true, nil
			}
		}
		return False(), true, nil
	case "map":
		if len(args) == 0 || args[0].Tag != TypeFunction {
			return arr, true, nil
		}
		result := make([]*Value, len(arr.ArrayVal))
		for i, item := range arr.ArrayVal {
			v, err := xvm.callFn(args[0].FnVal, []*Value{item, Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return Null(), true, err
			}
			result[i] = v
		}
		return Array(result), true, nil
	case "filter":
		if len(args) == 0 || args[0].Tag != TypeFunction {
			return arr, true, nil
		}
		var result []*Value
		for i, item := range arr.ArrayVal {
			v, err := xvm.callFn(args[0].FnVal, []*Value{item, Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return Null(), true, err
			}
			if v.Truthy() {
				result = append(result, item)
			}
		}
		return Array(result), true, nil
	case "reduce":
		if len(args) == 0 || args[0].Tag != TypeFunction {
			return Null(), true, nil
		}
		acc := Null()
		start := 0
		if len(args) > 1 {
			acc = args[1]
		} else if len(arr.ArrayVal) > 0 {
			acc = arr.ArrayVal[0]
			start = 1
		}
		for i := start; i < len(arr.ArrayVal); i++ {
			v, err := xvm.callFn(args[0].FnVal, []*Value{acc, arr.ArrayVal[i], Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return Null(), true, err
			}
			acc = v
		}
		return acc, true, nil
	case "forEach":
		if len(args) > 0 && args[0].Tag == TypeFunction {
			for i, item := range arr.ArrayVal {
				if _, err := xvm.callFn(args[0].FnVal, []*Value{item, Number(float64(i)), arr}, nil, nil); err != nil {
					return Null(), true, err
				}
			}
		}
		return Null(), true, nil
	case "find":
		if len(args) > 0 && args[0].Tag == TypeFunction {
			for _, item := range arr.ArrayVal {
				v, err := xvm.callFn(args[0].FnVal, []*Value{item}, nil, nil)
				if err != nil {
					return Null(), true, err
				}
				if v.Truthy() {
					return item, true, nil
				}
			}
		}
		return Null(), true, nil
	case "some":
		if len(args) > 0 && args[0].Tag == TypeFunction {
			for _, item := range arr.ArrayVal {
				v, err := xvm.callFn(args[0].FnVal, []*Value{item}, nil, nil)
				if err != nil {
					return Null(), true, err
				}
				if v.Truthy() {
					return True(), true, nil
				}
			}
		}
		return False(), true, nil
	case "every":
		if len(args) > 0 && args[0].Tag == TypeFunction {
			for _, item := range arr.ArrayVal {
				v, err := xvm.callFn(args[0].FnVal, []*Value{item}, nil, nil)
				if err != nil {
					return Null(), true, err
				}
				if !v.Truthy() {
					return False(), true, nil
				}
			}
		}
		return True(), true, nil
	case "concat":
		result := append([]*Value{}, arr.ArrayVal...)
		for _, a := range args {
			if a.Tag == TypeArray {
				result = append(result, a.ArrayVal...)
			} else {
				result = append(result, a)
			}
		}
		return Array(result), true, nil
	case "flat":
		var result []*Value
		for _, item := range arr.ArrayVal {
			if item.Tag == TypeArray {
				result = append(result, item.ArrayVal...)
			} else {
				result = append(result, item)
			}
		}
		return Array(result), true, nil
	case "toString":
		parts := make([]string, len(arr.ArrayVal))
		for i, v := range arr.ArrayVal {
			parts[i] = v.String()
		}
		return String("[" + strings.Join(parts, ", ") + "]"), true, nil
	}
	return nil, false, nil
}

func (xvm *XVM) stringMethod(str *Value, name string, args []*Value) (*Value, bool, error) {
	s := str.StrVal
	switch name {
	case "length", "len":
		return Number(float64(len([]rune(s)))), true, nil
	case "toUpperCase", "upper":
		return String(strings.ToUpper(s)), true, nil
	case "toLowerCase", "lower":
		return String(strings.ToLower(s)), true, nil
	case "trim":
		return String(strings.TrimSpace(s)), true, nil
	case "trimStart", "trimLeft":
		return String(strings.TrimLeft(s, " \t\r\n")), true, nil
	case "trimEnd", "trimRight":
		return String(strings.TrimRight(s, " \t\r\n")), true, nil
	case "split":
		sep := ""
		if len(args) > 0 {
			sep = args[0].String()
		}
		parts := strings.Split(s, sep)
		items := make([]*Value, len(parts))
		for i, p := range parts {
			items[i] = String(p)
		}
		return Array(items), true, nil
	case "includes", "contains":
		if len(args) == 0 {
			return False(), true, nil
		}
		return Bool(strings.Contains(s, args[0].String())), true, nil
	case "startsWith":
		if len(args) == 0 {
			return False(), true, nil
		}
		return Bool(strings.HasPrefix(s, args[0].String())), true, nil
	case "endsWith":
		if len(args) == 0 {
			return False(), true, nil
		}
		return Bool(strings.HasSuffix(s, args[0].String())), true, nil
	case "indexOf":
		if len(args) == 0 {
			return Number(-1), true, nil
		}
		idx := strings.Index(s, args[0].String())
		return Number(float64(idx)), true, nil
	case "replace":
		if len(args) < 2 {
			return str, true, nil
		}
		return String(strings.Replace(s, args[0].String(), args[1].String(), 1)), true, nil
	case "replaceAll":
		if len(args) < 2 {
			return str, true, nil
		}
		return String(strings.ReplaceAll(s, args[0].String(), args[1].String())), true, nil
	case "slice", "substring":
		runes := []rune(s)
		start, end := 0, len(runes)
		if len(args) > 0 {
			start = int(args[0].NumVal)
		}
		if len(args) > 1 {
			end = int(args[1].NumVal)
		}
		if start < 0 {
			start = len(runes) + start
		}
		if start < 0 {
			start = 0
		}
		if end > len(runes) {
			end = len(runes)
		}
		if start > end {
			return String(""), true, nil
		}
		return String(string(runes[start:end])), true, nil
	case "charAt":
		runes := []rune(s)
		if len(args) == 0 {
			return String(""), true, nil
		}
		i := int(args[0].NumVal)
		if i < 0 || i >= len(runes) {
			return String(""), true, nil
		}
		return String(string(runes[i])), true, nil
	case "charCodeAt":
		runes := []rune(s)
		if len(args) == 0 {
			return Number(0), true, nil
		}
		i := int(args[0].NumVal)
		if i < 0 || i >= len(runes) {
			return Number(math.NaN()), true, nil
		}
		return Number(float64(runes[i])), true, nil
	case "repeat":
		if len(args) == 0 {
			return str, true, nil
		}
		n := int(args[0].NumVal)
		return String(strings.Repeat(s, n)), true, nil
	case "padStart":
		if len(args) == 0 {
			return str, true, nil
		}
		targetLen := int(args[0].NumVal)
		padChar := " "
		if len(args) > 1 {
			padChar = args[1].String()
		}
		runes := []rune(s)
		for len(runes) < targetLen {
			runes = append([]rune(padChar), runes...)
		}
		return String(string(runes)), true, nil
	case "padEnd":
		if len(args) == 0 {
			return str, true, nil
		}
		targetLen := int(args[0].NumVal)
		padChar := " "
		if len(args) > 1 {
			padChar = args[1].String()
		}
		runes := []rune(s)
		for len(runes) < targetLen {
			runes = append(runes, []rune(padChar)...)
		}
		return String(string(runes)), true, nil
	case "toString":
		return str, true, nil
	}
	return nil, false, nil
}

func (xvm *XVM) objectMethod(obj *Value, name string, args []*Value) (*Value, bool, error) {
	switch name {
	case "keys":
		keys := make([]*Value, 0, len(obj.ObjVal))
		for k := range obj.ObjVal {
			keys = append(keys, String(k))
		}
		return Array(keys), true, nil
	case "values":
		vals := make([]*Value, 0, len(obj.ObjVal))
		for _, v := range obj.ObjVal {
			vals = append(vals, v)
		}
		return Array(vals), true, nil
	case "entries":
		entries := make([]*Value, 0, len(obj.ObjVal))
		for k, v := range obj.ObjVal {
			entries = append(entries, Array([]*Value{String(k), v}))
		}
		return Array(entries), true, nil
	case "has", "hasOwnProperty":
		if len(args) == 0 {
			return False(), true, nil
		}
		_, ok := obj.ObjVal[args[0].String()]
		return Bool(ok), true, nil
	case "toString":
		return String(obj.String()), true, nil
	}
	return nil, false, nil
}
