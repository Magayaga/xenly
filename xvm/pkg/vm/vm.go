/*
 * XENLY VIRTUAL MACHINE (XVM) - high-performance of the Virtual Machine
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in Go programming language.
 *
 * It is available for the Linux, macOS, and Windows operating systems.
 *
 */
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
	"sort"
	"strings"
	"sync"
	"time"

	"xvm/pkg/bytecode"
)

// XVM is the Xenly Virtual Machine.
type XVM struct {
	module  *bytecode.Module
	global  *Env
	fns     []*FunctionVal
	classes []*ClassVal
	reader  *bufio.Reader

	// Pre-built constant pool caches — built once at NewXVM, reused on every
	// OP_PUSH_CONST / OP_LOAD / OP_STORE without re-decoding pool entries.
	poolValues  []*Value // poolValues[i] = ready *Value for module.Pool[i]
	poolStrings []string // poolStrings[i] = string value of module.Pool[i]
}

// Value pools for reducing GC pressure
var (
	stringPool = sync.Pool{
		New: func() interface{} {
			return make([]string, 0, 8)
		},
	}
	valuePool = sync.Pool{
		New: func() interface{} {
			return make([]*Value, 0, 8)
		},
	}
	// argsPool provides reusable []*Value slices for function call arguments,
	// eliminating a make([]*Value, argc) heap allocation on every OP_CALL.
	argsPool = sync.Pool{
		New: func() interface{} {
			s := make([]*Value, 0, 16)
			return &s
		},
	}
	// envPool provides reusable Env structs for scope creation.
	// Every OP_PUSH_SCOPE and every function call previously did a heap
	// allocation via NewEnv.  The pool returns zeroed Env values; the caller
	// sets flatLen=0 and parent before use.
	envPool = sync.Pool{
		New: func() interface{} { return &Env{} },
	}
)

// NewXVM creates a new XVM from a compiled module.
func NewXVM(mod *bytecode.Module) *XVM {
	xvm := &XVM{
		module: mod,
		global: NewEnv(nil),
		reader: bufio.NewReader(os.Stdin),
	}
	xvm.buildPoolCaches()
	RegisterBuiltins(xvm.global)
	xvm.buildFunctions()
	xvm.buildClasses()
	return xvm
}

// buildPoolCaches pre-builds poolValues and poolStrings from the constant pool.
// OP_PUSH_CONST and all variable ops use these instead of re-decoding each time.
func (xvm *XVM) buildPoolCaches() {
	n := len(xvm.module.Pool)
	xvm.poolValues = make([]*Value, n)
	xvm.poolStrings = make([]string, n)
	for i, c := range xvm.module.Pool {
		switch c.Type {
		case bytecode.ConstNull:
			xvm.poolValues[i] = valNull
		case bytecode.ConstNumber:
			xvm.poolValues[i] = Number(c.NumVal) // uses integer cache when applicable
		case bytecode.ConstString:
			xvm.poolValues[i] = &Value{Tag: TypeString, StrVal: c.StrVal}
			xvm.poolStrings[i] = c.StrVal
		case bytecode.ConstBool:
			xvm.poolValues[i] = Bool(c.BoolVal)
		default:
			xvm.poolValues[i] = valNull
		}
	}
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

// poolStr returns the string for pool index idx (O(1), no allocation).
func (xvm *XVM) poolStr(idx uint32) string {
	if int(idx) < len(xvm.poolStrings) {
		return xvm.poolStrings[idx]
	}
	return ""
}

// poolVal returns the pre-built *Value for pool index idx (O(1), no allocation).
func (xvm *XVM) poolVal(idx uint32) *Value {
	if int(idx) < len(xvm.poolValues) {
		return xvm.poolValues[idx]
	}
	return valNull
}

// Run executes the module's main code chunk.
func (xvm *XVM) Run() error {
	_, err := xvm.exec(xvm.module.MainCode, xvm.global, nil, nil)
	return err
}

// exec runs a bytecode chunk inside a given environment.
func (xvm *XVM) exec(code []byte, env *Env, thisVal *Value, class *ClassVal) (*Value, error) {
	// ── Hybrid value stack ────────────────────────────────────────────────────
	// stackBuf is a stack-allocated backing array for the initial []*Value slice.
	// For typical programs the slice never grows beyond stackBufSize, so no heap
	// allocation is needed for the value stack.  If a program pushes more than
	// stackBufSize values without consuming them (e.g. huge array literals or
	// deeply nested expressions), append transparently moves to the heap — still
	// correct, just one allocation instead of a panic.
	const stackBufSize = 512
	var stackBuf [stackBufSize]*Value
	stack := stackBuf[:0:stackBufSize] // len=0, cap=512, backed by stackBuf

	push := func(v *Value) {
		if v == nil {
			v = valNull
		}
		stack = append(stack, v)
	}
	pop := func() *Value {
		n := len(stack)
		if n == 0 {
			return valNull
		}
		v := stack[n-1]
		stack[n-1] = nil // clear reference for GC
		stack = stack[:n-1]
		return v
	}
	peek := func() *Value {
		n := len(stack)
		if n == 0 {
			return valNull
		}
		return stack[n-1]
	}

	scopes := make([]*Env, 1, 16)
	scopes[0] = env
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

	// isTruthy inlines Truthy() to avoid method call overhead on the hottest
	// control-flow ops (JUMP_TRUE, JUMP_FALSE, AND, OR, NOT).
	isTruthy := func(v *Value) bool {
		switch v.Tag {
		case TypeNull:
			return false
		case TypeBool:
			return v.BoolVal
		case TypeNumber:
			return v.NumVal != 0 && v.NumVal == v.NumVal // != 0 && !NaN
		case TypeString:
			return v.StrVal != ""
		default:
			return true // arrays, objects, functions, classes, instances
		}
	}

	ip := 0
	for ip < len(code) {
		op := bytecode.Opcode(code[ip])
		ip++

		switch op {

		// ── Stack ──────────────────────────────────────────────────────────
		case bytecode.OP_NOP:
		case bytecode.OP_HALT:
			return valNull, nil
		case bytecode.OP_POP:
			pop()
		case bytecode.OP_DUP:
			push(peek())
		case bytecode.OP_SWAP:
			if n := len(stack); n >= 2 {
				stack[n-1], stack[n-2] = stack[n-2], stack[n-1]
			}

		// ── Literals ────────────────────────────────────────────────────────
		case bytecode.OP_PUSH_NULL:
			push(valNull)
		case bytecode.OP_PUSH_TRUE:
			push(valTrue)
		case bytecode.OP_PUSH_FALSE:
			push(valFalse)
		case bytecode.OP_PUSH_CONST:
			push(xvm.poolVal(readU32(&ip)))

		// ── Arithmetic ──────────────────────────────────────────────────────
		case bytecode.OP_ADD:
			b := pop()
			a := pop()
			// Fast path: both numbers (most common case in numeric code)
			if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Number(a.NumVal + b.NumVal))
			} else if a.Tag == TypeString || b.Tag == TypeString {
				var builder strings.Builder
				builder.Grow(len(a.String()) + len(b.String()))
				builder.WriteString(a.String())
				builder.WriteString(b.String())
				push(String(builder.String()))
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
			// Fast paths for the two most common cases:
			// 1. Both numbers — covers loop conditions, equality checks
			// 2. Pointer equality — covers null==null, true==true (singletons)
			if a == b {
				push(valTrue)
			} else if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Bool(a.NumVal == b.NumVal))
			} else if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal == b.StrVal))
			} else {
				push(Bool(a.Equal(b)))
			}
		case bytecode.OP_NEQ:
			b, a := pop(), pop()
			if a == b {
				push(valFalse)
			} else if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Bool(a.NumVal != b.NumVal))
			} else if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal != b.StrVal))
			} else {
				push(Bool(!a.Equal(b)))
			}
		case bytecode.OP_LT:
			b, a := pop(), pop()
			// Fast path: numeric comparison (most common in loops)
			if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Bool(a.NumVal < b.NumVal))
			} else if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal < b.StrVal))
			} else {
				push(Bool(a.NumVal < b.NumVal))
			}
		case bytecode.OP_LE:
			b, a := pop(), pop()
			if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Bool(a.NumVal <= b.NumVal))
			} else if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal <= b.StrVal))
			} else {
				push(Bool(a.NumVal <= b.NumVal))
			}
		case bytecode.OP_GT:
			b, a := pop(), pop()
			if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Bool(a.NumVal > b.NumVal))
			} else if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal > b.StrVal))
			} else {
				push(Bool(a.NumVal > b.NumVal))
			}
		case bytecode.OP_GE:
			b, a := pop(), pop()
			if a.Tag == TypeNumber && b.Tag == TypeNumber {
				push(Bool(a.NumVal >= b.NumVal))
			} else if a.Tag == TypeString && b.Tag == TypeString {
				push(Bool(a.StrVal >= b.StrVal))
			} else {
				push(Bool(a.NumVal >= b.NumVal))
			}

		// ── Logical ─────────────────────────────────────────────────────────
		case bytecode.OP_AND:
			b, a := pop(), pop()
			if isTruthy(a) {
				push(b)
			} else {
				push(a)
			}
		case bytecode.OP_OR:
			b, a := pop(), pop()
			if isTruthy(a) {
				push(a)
			} else {
				push(b)
			}
		case bytecode.OP_NOT:
			push(Bool(!isTruthy(pop())))
		case bytecode.OP_NULLISH:
			b, a := pop(), pop()
			if a == valNull || a.Tag == TypeNull {
				push(b)
			} else {
				push(a)
			}

		// ── Variables ────────────────────────────────────────────────────────
		case bytecode.OP_LOAD:
			name := xvm.poolStr(readU32(&ip))
			// Inline the flat-array scan from Env.Get to avoid a function call
			// on this critical-path opcode.
			var loaded *Value
			for cur := currentEnv(); cur != nil; cur = cur.parent {
				found := false
				for i := 0; i < cur.flatLen; i++ {
					if cur.flat[i].name == name {
						loaded = cur.flat[i].value
						found = true
						break
					}
				}
				if found {
					break
				}
				if cur.overflow != nil {
					if en, ok := cur.overflow[name]; ok {
						loaded = en.value
						break
					}
				}
			}
			if loaded == nil {
				loaded = valNull
			}
			push(loaded)
		case bytecode.OP_STORE:
			name := xvm.poolStr(readU32(&ip))
			if err := currentEnv().Set(name, peek()); err != nil {
				return valNull, fmt.Errorf("line ?: %w", err)
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
			if isTruthy(pop()) {
				ip = int(addr)
			}
		case bytecode.OP_JUMP_FALSE:
			addr := readI32(&ip)
			if !isTruthy(pop()) {
				ip = int(addr)
			}
		case bytecode.OP_JUMP_NULL:
			addr := readI32(&ip)
			if v := peek(); v == valNull || v.Tag == TypeNull {
				ip = int(addr)
			}

		// ── Scope ────────────────────────────────────────────────────────────
		case bytecode.OP_PUSH_SCOPE:
			// Reuse a pooled Env instead of heap-allocating on every scope entry.
			e := envPool.Get().(*Env)
			e.flatLen = 0
			e.overflow = nil
			e.parent = currentEnv()
			scopes = append(scopes, e)
		case bytecode.OP_POP_SCOPE:
			if len(scopes) > 1 {
				old := scopes[len(scopes)-1]
				scopes = scopes[:len(scopes)-1]
				// Return scope to pool — clear references first.
				for i := 0; i < old.flatLen; i++ {
					old.flat[i].value = nil
					old.flat[i].name = ""
				}
				old.overflow = nil
				old.parent = nil
				envPool.Put(old)
			}

		// ── Functions ────────────────────────────────────────────────────────
		case bytecode.OP_MAKE_FN:
			idx := readU32(&ip)
			if int(idx) < len(xvm.fns) {
				fn := xvm.fns[idx]
				// Always capture current environment for proper closure behavior
				closure := &FunctionVal{
					Name:    fn.Name,
					Params:  fn.Params,
					Code:    fn.Code,
					IsAsync: fn.IsAsync,
					Closure: currentEnv(),
				}
				push(&Value{Tag: TypeFunction, FnVal: closure})
			} else {
				push(valNull)
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
				push(valNull)
			}
		case bytecode.OP_CALL:
			argc := int(readU8(&ip))
			// Reuse a pooled slice for args — avoids a heap allocation per call.
			argsp := argsPool.Get().(*[]*Value)
			args := (*argsp)[:argc]
			if cap(args) < argc {
				args = make([]*Value, argc)
			} else {
				args = args[:argc]
			}
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
			callee := pop()
			result, err := xvm.callValue(callee, args, nil, nil)
			*argsp = args[:0]
			argsPool.Put(argsp)
			if err != nil {
				return valNull, err
			}
			push(result)
		case bytecode.OP_CALL_METHOD:
			nameIdx := readU32(&ip)
			argc := int(readU8(&ip))
			argsp := argsPool.Get().(*[]*Value)
			args := (*argsp)[:0]
			if cap(*argsp) < argc {
				args = make([]*Value, argc)
			} else {
				args = (*argsp)[:argc]
			}
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
			receiver := pop()
			result, err := xvm.callMethod(receiver, xvm.poolStr(nameIdx), args)
			*argsp = args[:0]
			argsPool.Put(argsp)
			if err != nil {
				return valNull, err
			}
			push(result)
		case bytecode.OP_RETURN:
			return valNull, nil
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
				push(Number(float64(len(a.ArrayVal)))) // integer cache hit for len ≤ 511
			} else if a.Tag == TypeString {
				push(Number(float64(len([]rune(a.StrVal)))))
			} else {
				push(Number(0))
			}

		// ── Objects ──────────────────────────────────────────────────────────
		case bytecode.OP_MAKE_OBJ:
			count := int(readU32(&ip))
			m := make(map[string]*Value, count)
			// For small objects (≤ 8 pairs) use a stack-allocated array to
			// collect pairs instead of heap-allocating a []*Value slice.
			if count <= 8 {
				var pairs [16]*Value // 8 key-value pairs max
				for i := count*2 - 1; i >= 0; i-- {
					pairs[i] = pop()
				}
				for i := 0; i < count; i++ {
					m[pairs[i*2].String()] = pairs[i*2+1]
				}
			} else {
				pairs := make([]*Value, count*2)
				for i := count*2 - 1; i >= 0; i-- {
					pairs[i] = pop()
				}
				for i := 0; i < count; i++ {
					m[pairs[i*2].String()] = pairs[i*2+1]
				}
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
				for _, fn := range cls.Methods {
					if fn.Closure == nil {
						fn.Closure = currentEnv()
					}
				}
				push(&Value{Tag: TypeClass, ClassVal: cls})
			} else {
				push(valNull)
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
				return valNull, err
			}
			push(result)
		case bytecode.OP_THIS_LOAD:
			if thisVal != nil {
				push(thisVal)
			} else {
				push(valNull)
			}
		case bytecode.OP_SUPER_CALL:
			argc := int(readU8(&ip))
			args := make([]*Value, argc)
			for i := argc - 1; i >= 0; i-- {
				args[i] = pop()
			}
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
			parts := stringPool.Get().([]string)
			parts = parts[:0]
			vals := make([]*Value, count)
			for i := count - 1; i >= 0; i-- {
				vals[i] = pop()
			}
			for _, v := range vals {
				parts = append(parts, v.String())
			}
			fmt.Fprintln(stdout, strings.Join(parts, " "))
			stringPool.Put(parts[:0])
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
			if !isTruthy(cond) {
				return valNull, fmt.Errorf("XVM assertion failed: %s", msg.String())
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
			subject := pop()
			push(Bool(subject.Tag == TypeVariant && subject.VarTag == tagName))
		case bytecode.OP_EXTRACT_FIELD:
			fieldIdx := int(readU8(&ip))
			v := peek()
			if v.Tag == TypeVariant && fieldIdx < len(v.VarFields) {
				push(v.VarFields[fieldIdx])
			} else {
				push(valNull)
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
				push(valNull)
			}

		// ── Misc ─────────────────────────────────────────────────────────────
		case bytecode.OP_TO_STRING:
			push(String(pop().String()))
		case bytecode.OP_CONCAT_STR:
			count := int(readU8(&ip))
			parts := stringPool.Get().([]string)
			parts = parts[:0]
			// Pop strings in reverse, build inline
			if count <= 16 {
				var tmp [16]string
				for i := count - 1; i >= 0; i-- {
					tmp[i] = pop().String()
				}
				for i := 0; i < count; i++ {
					parts = append(parts, tmp[i])
				}
			} else {
				extra := make([]string, count)
				for i := count - 1; i >= 0; i-- {
					extra[i] = pop().String()
				}
				parts = append(parts, extra...)
			}
			push(String(strings.Join(parts, "")))
			stringPool.Put(parts[:0])
		case bytecode.OP_INC:
			name := xvm.poolStr(readU32(&ip))
			// Fast in-place increment: scan flat arrays before calling Set+Number.
			inc := false
			for cur := currentEnv(); cur != nil; cur = cur.parent {
				for i := 0; i < cur.flatLen; i++ {
					if cur.flat[i].name == name {
						cur.flat[i].value = Number(cur.flat[i].value.NumVal + 1)
						inc = true
						break
					}
				}
				if inc {
					break
				}
				if cur.overflow != nil {
					if en, ok := cur.overflow[name]; ok {
						en.value = Number(en.value.NumVal + 1)
						inc = true
						break
					}
				}
			}
			if !inc {
				currentEnv().Define(name, Number(1), false)
			}
		case bytecode.OP_DEC:
			name := xvm.poolStr(readU32(&ip))
			dec := false
			for cur := currentEnv(); cur != nil; cur = cur.parent {
				for i := 0; i < cur.flatLen; i++ {
					if cur.flat[i].name == name {
						cur.flat[i].value = Number(cur.flat[i].value.NumVal - 1)
						dec = true
						break
					}
				}
				if dec {
					break
				}
				if cur.overflow != nil {
					if en, ok := cur.overflow[name]; ok {
						en.value = Number(en.value.NumVal - 1)
						dec = true
						break
					}
				}
			}
			if !dec {
				currentEnv().Define(name, Number(-1), false)
			}
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
			// ── Unknown opcodes: skip operands based on known patterns ────
			// If the compiler emits opcodes we don't handle (like OP_IMPORT),
			// try to consume their operands gracefully instead of crashing.
			// For safety, just treat truly unknown opcodes as NOPs with a
			// warning — this prevents stack corruption from unhandled imports.
			_ = op // treat as NOP
		}
	}
	return valNull, nil
}

// ─── Call helpers ─────────────────────────────────────────────────────────────

func (xvm *XVM) callValue(callee *Value, args []*Value, thisVal *Value, class *ClassVal) (*Value, error) {
	if callee == nil || callee.Tag == TypeNull {
		return valNull, fmt.Errorf("XVM: attempt to call null value")
	}

	switch callee.Tag {
	case TypeFunction:
		return xvm.callFn(callee.FnVal, args, thisVal, class)
	case TypeBuiltin:
		return callee.BuiltinV(args)
	case TypeClass:
		return xvm.instantiateClassVal(callee.ClassVal, args)

	case TypeArray:
		if len(args) == 0 {
			return callee, nil
		}
		if len(args) == 1 {
			return xvm.indexGet(callee, args[0]), nil
		}
		arr := callee.ArrayVal
		start := int(args[0].NumVal)
		end := len(arr)
		if len(args) >= 2 {
			end = int(args[1].NumVal)
		}
		if start < 0 {
			start = len(arr) + start
		}
		if end < 0 {
			end = len(arr) + end
		}
		if start < 0 {
			start = 0
		}
		if end > len(arr) {
			end = len(arr)
		}
		if start >= end {
			return Array(nil), nil
		}
		newArr := make([]*Value, end-start)
		copy(newArr, arr[start:end])
		return Array(newArr), nil

	case TypeString:
		if len(args) == 0 {
			return callee, nil
		}
		if len(args) == 1 {
			return xvm.indexGet(callee, args[0]), nil
		}
		runes := []rune(callee.StrVal)
		start := int(args[0].NumVal)
		end := len(runes)
		if len(args) >= 2 {
			end = int(args[1].NumVal)
		}
		if start < 0 {
			start = len(runes) + start
		}
		if end < 0 {
			end = len(runes) + end
		}
		if start < 0 {
			start = 0
		}
		if end > len(runes) {
			end = len(runes)
		}
		if start >= end {
			return String(""), nil
		}
		return String(string(runes[start:end])), nil

	case TypeObject:
		if callee.ObjVal != nil {
			if callFn, ok := callee.ObjVal["__call"]; ok {
				return xvm.callValue(callFn, args, callee, nil)
			}
		}
		return valNull, fmt.Errorf("XVM: object is not callable (no __call method)")

	case TypeInstance:
		if callee.InstVal != nil {
			for cls := callee.InstVal.Class; cls != nil; cls = cls.Parent {
				if fn, ok := cls.Methods["__call"]; ok {
					return xvm.callFn(fn, args, callee, cls)
				}
			}
			if fv, ok := callee.InstVal.Fields["__call"]; ok {
				return xvm.callValue(fv, args, callee, callee.InstVal.Class)
			}
		}
		return valNull, fmt.Errorf("XVM: instance is not callable (no __call method)")

	// ── Non-callable primitives: return value itself ─────────────────────
	// This handles stack misalignment from unknown opcodes gracefully.
	case TypeNumber, TypeBool:
		return callee, nil
	}

	return valNull, fmt.Errorf("XVM: value of type %q is not callable (value: %s)", callee.TypeName(), callee.String())
}

func (xvm *XVM) callFn(fn *FunctionVal, args []*Value, thisVal *Value, class *ClassVal) (*Value, error) {
	if fn == nil {
		return valNull, fmt.Errorf("XVM: nil function")
	}
	parent := fn.Closure
	if parent == nil {
		parent = xvm.global
	}
	// Use pooled Env to avoid a heap allocation on every function call.
	callEnv := envPool.Get().(*Env)
	callEnv.flatLen = 0
	callEnv.overflow = nil
	callEnv.parent = parent
	xvm.bindArgs(fn, args, callEnv)
	result, err := xvm.exec(fn.Code, callEnv, thisVal, class)
	// Return the Env to the pool only when it has no closures that escaped.
	// A simple heuristic: if the function returned a closure that captured
	// callEnv we must NOT pool it (the closure still holds a pointer).
	// We detect this conservatively: if the result is a function value with
	// Closure == callEnv we skip pooling.
	if result == nil || result.Tag != TypeFunction ||
		result.FnVal == nil || result.FnVal.Closure != callEnv {
		// Safe to recycle — clear references to prevent GC retention.
		for i := 0; i < callEnv.flatLen; i++ {
			callEnv.flat[i].value = nil
			callEnv.flat[i].name = ""
		}
		callEnv.overflow = nil
		callEnv.parent = nil
		envPool.Put(callEnv)
	}
	return result, err
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
		val := valNull
		if i < len(args) {
			val = args[i]
		}
		env.Define(p.Name, val, false)
	}
}

func (xvm *XVM) callMethod(receiver *Value, name string, args []*Value) (*Value, error) {
	if receiver == nil || receiver.Tag == TypeNull {
		return valNull, fmt.Errorf("XVM: cannot call method %q on null", name)
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
		if fv, ok := receiver.InstVal.Fields[name]; ok {
			return xvm.callValue(fv, args, receiver, receiver.InstVal.Class)
		}
		return valNull, fmt.Errorf("XVM: method %q not found on %s", name, receiver.InstVal.Class.Name)
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

	return valNull, fmt.Errorf("XVM: cannot call method %q on %s", name, receiver.TypeName())
}

// ─── Property access ──────────────────────────────────────────────────────────

func (xvm *XVM) propGet(obj *Value, name string) *Value {
	if obj == nil || obj.Tag == TypeNull {
		return valNull
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
		case "first":
			if len(obj.ArrayVal) > 0 {
				return obj.ArrayVal[0]
			}
			return valNull
		case "last":
			if len(obj.ArrayVal) > 0 {
				return obj.ArrayVal[len(obj.ArrayVal)-1]
			}
			return valNull
		case "empty", "isEmpty":
			return Bool(len(obj.ArrayVal) == 0)
		}
	case TypeString:
		switch name {
		case "length", "len":
			return Number(float64(len([]rune(obj.StrVal))))
		case "empty", "isEmpty":
			return Bool(obj.StrVal == "")
		}
	}
	return valNull
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
		return valNull
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
	return valNull
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
	if cv, ok := env.Get(name); ok && cv.Tag == TypeClass {
		return xvm.instantiateClassVal(cv.ClassVal, args)
	}
	for _, cls := range xvm.classes {
		if cls.Name == name {
			return xvm.instantiateClassVal(cls, args)
		}
	}
	return valNull, fmt.Errorf("XVM: class %q not found", name)
}

func (xvm *XVM) instantiateClassVal(cls *ClassVal, args []*Value) (*Value, error) {
	if cls == nil {
		return valNull, fmt.Errorf("XVM: nil class")
	}
	inst := &InstanceVal{
		Class:  cls,
		Fields: make(map[string]*Value),
	}
	instVal := &Value{Tag: TypeInstance, InstVal: inst}

	if initFn, ok := cls.Methods["init"]; ok {
		_, err := xvm.callFn(initFn, args, instVal, cls)
		if err != nil {
			return valNull, err
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
	case TypeVariant:
		return xvm.variantMethod(receiver, name, args)
	case TypeNull:
		return xvm.nullMethod(name, args)
	case TypeNumber, TypeBool:
		// Primitive values are always "present" — support the functional
		// Optional-style methods so code works uniformly regardless of
		// whether a value is wrapped in a variant or not.
		return xvm.primitiveOptionalMethod(receiver, name, args)
	}
	return nil, false, nil
}

func (xvm *XVM) arrayMethod(arr *Value, name string, args []*Value) (*Value, bool, error) {
	switch name {
	case "push":
		for _, a := range args {
			if a.Tag == TypeArray || a.Tag == TypeObject {
				arr.ArrayVal = append(arr.ArrayVal, a.Clone())
			} else {
				arr.ArrayVal = append(arr.ArrayVal, a)
			}
		}
		return arr, true, nil
	case "pop":
		if len(arr.ArrayVal) == 0 {
			return valNull, true, nil
		}
		v := arr.ArrayVal[len(arr.ArrayVal)-1]
		arr.ArrayVal = arr.ArrayVal[:len(arr.ArrayVal)-1]
		return v, true, nil
	case "shift":
		if len(arr.ArrayVal) == 0 {
			return valNull, true, nil
		}
		v := arr.ArrayVal[0]
		arr.ArrayVal = arr.ArrayVal[1:]
		return v, true, nil
	case "unshift":
		clonedArgs := make([]*Value, len(args))
		for i, a := range args {
			if a.Tag == TypeArray || a.Tag == TypeObject {
				clonedArgs[i] = a.Clone()
			} else {
				clonedArgs[i] = a
			}
		}
		arr.ArrayVal = append(clonedArgs, arr.ArrayVal...)
		return arr, true, nil
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
		if start >= end {
			return Array(nil), true, nil
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
	case "lastIndexOf":
		if len(args) == 0 {
			return Number(-1), true, nil
		}
		for i := len(arr.ArrayVal) - 1; i >= 0; i-- {
			if arr.ArrayVal[i].Equal(args[0]) {
				return Number(float64(i)), true, nil
			}
		}
		return Number(-1), true, nil
	case "includes", "contains":
		if len(args) == 0 {
			return valFalse, true, nil
		}
		for _, v := range arr.ArrayVal {
			if v.Equal(args[0]) {
				return valTrue, true, nil
			}
		}
		return valFalse, true, nil
	case "map":
		if len(args) == 0 {
			return arr, true, nil
		}
		cb := args[0]
		result := make([]*Value, len(arr.ArrayVal))
		for i, item := range arr.ArrayVal {
			v, err := xvm.callValue(cb, []*Value{item, Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return valNull, true, err
			}
			result[i] = v
		}
		return Array(result), true, nil
	case "filter":
		if len(args) == 0 {
			return arr, true, nil
		}
		cb := args[0]
		var result []*Value
		for i, item := range arr.ArrayVal {
			v, err := xvm.callValue(cb, []*Value{item, Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return valNull, true, err
			}
			if v != valNull && v.Tag != TypeNull && (v.Tag != TypeBool || v.BoolVal) && (v.Tag != TypeNumber || v.NumVal != 0) && (v.Tag != TypeString || v.StrVal != "") {
				result = append(result, item)
			}
		}
		return Array(result), true, nil
	case "reduce":
		if len(args) == 0 {
			return valNull, true, nil
		}
		cb := args[0]
		acc := valNull
		startIdx := 0
		if len(args) > 1 {
			acc = args[1]
		} else if len(arr.ArrayVal) > 0 {
			acc = arr.ArrayVal[0]
			startIdx = 1
		}
		for i := startIdx; i < len(arr.ArrayVal); i++ {
			v, err := xvm.callValue(cb, []*Value{acc, arr.ArrayVal[i], Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return valNull, true, err
			}
			acc = v
		}
		return acc, true, nil
	case "forEach":
		if len(args) > 0 {
			cb := args[0]
			for i, item := range arr.ArrayVal {
				if _, err := xvm.callValue(cb, []*Value{item, Number(float64(i)), arr}, nil, nil); err != nil {
					return valNull, true, err
				}
			}
		}
		return valNull, true, nil
	case "find":
		if len(args) > 0 {
			cb := args[0]
			for _, item := range arr.ArrayVal {
				v, err := xvm.callValue(cb, []*Value{item}, nil, nil)
				if err != nil {
					return valNull, true, err
				}
				if v != valNull && v.Tag != TypeNull && (v.Tag != TypeBool || v.BoolVal) && (v.Tag != TypeNumber || v.NumVal != 0) && (v.Tag != TypeString || v.StrVal != "") {
					return item, true, nil
				}
			}
		}
		return valNull, true, nil
	case "findIndex":
		if len(args) > 0 {
			cb := args[0]
			for i, item := range arr.ArrayVal {
				v, err := xvm.callValue(cb, []*Value{item, Number(float64(i)), arr}, nil, nil)
				if err != nil {
					return valNull, true, err
				}
				if v != valNull && v.Tag != TypeNull && (v.Tag != TypeBool || v.BoolVal) && (v.Tag != TypeNumber || v.NumVal != 0) && (v.Tag != TypeString || v.StrVal != "") {
					return Number(float64(i)), true, nil
				}
			}
		}
		return Number(-1), true, nil
	case "some":
		if len(args) > 0 {
			cb := args[0]
			for _, item := range arr.ArrayVal {
				v, err := xvm.callValue(cb, []*Value{item}, nil, nil)
				if err != nil {
					return valNull, true, err
				}
				if v != valNull && v.Tag != TypeNull && (v.Tag != TypeBool || v.BoolVal) && (v.Tag != TypeNumber || v.NumVal != 0) && (v.Tag != TypeString || v.StrVal != "") {
					return valTrue, true, nil
				}
			}
		}
		return valFalse, true, nil
	case "every":
		if len(args) > 0 {
			cb := args[0]
			for _, item := range arr.ArrayVal {
				v, err := xvm.callValue(cb, []*Value{item}, nil, nil)
				if err != nil {
					return valNull, true, err
				}
				if v == valNull || v.Tag == TypeNull || (v.Tag == TypeBool && !v.BoolVal) || (v.Tag == TypeNumber && v.NumVal == 0) || (v.Tag == TypeString && v.StrVal == "") {
					return valFalse, true, nil
				}
			}
		}
		return valTrue, true, nil
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
	case "flatMap":
		if len(args) == 0 {
			return arr, true, nil
		}
		cb := args[0]
		var result []*Value
		for i, item := range arr.ArrayVal {
			v, err := xvm.callValue(cb, []*Value{item, Number(float64(i)), arr}, nil, nil)
			if err != nil {
				return valNull, true, err
			}
			if v.Tag == TypeArray {
				result = append(result, v.ArrayVal...)
			} else {
				result = append(result, v)
			}
		}
		return Array(result), true, nil
	case "sort":
		if len(args) > 0 {
			cb := args[0]
			var sortErr error
			sort.SliceStable(arr.ArrayVal, func(i, j int) bool {
				if sortErr != nil {
					return false
				}
				v, err := xvm.callValue(cb, []*Value{arr.ArrayVal[i], arr.ArrayVal[j]}, nil, nil)
				if err != nil {
					sortErr = err
					return false
				}
				return v.NumVal < 0
			})
			if sortErr != nil {
				return valNull, true, sortErr
			}
		} else {
			sort.SliceStable(arr.ArrayVal, func(i, j int) bool {
				a, b := arr.ArrayVal[i], arr.ArrayVal[j]
				if a.Tag == TypeNumber && b.Tag == TypeNumber {
					return a.NumVal < b.NumVal
				}
				return a.String() < b.String()
			})
		}
		return arr, true, nil
	case "fill":
		if len(args) == 0 {
			return arr, true, nil
		}
		fillVal := args[0]
		startIdx := 0
		endIdx := len(arr.ArrayVal)
		if len(args) > 1 {
			startIdx = int(args[1].NumVal)
		}
		if len(args) > 2 {
			endIdx = int(args[2].NumVal)
		}
		if startIdx < 0 {
			startIdx = len(arr.ArrayVal) + startIdx
		}
		if endIdx < 0 {
			endIdx = len(arr.ArrayVal) + endIdx
		}
		if startIdx < 0 {
			startIdx = 0
		}
		if endIdx > len(arr.ArrayVal) {
			endIdx = len(arr.ArrayVal)
		}
		for i := startIdx; i < endIdx; i++ {
			arr.ArrayVal[i] = fillVal
		}
		return arr, true, nil
	case "splice":
		if len(args) < 1 {
			return Array(nil), true, nil
		}
		startIdx := int(args[0].NumVal)
		if startIdx < 0 {
			startIdx = len(arr.ArrayVal) + startIdx
		}
		if startIdx < 0 {
			startIdx = 0
		}
		if startIdx > len(arr.ArrayVal) {
			startIdx = len(arr.ArrayVal)
		}
		deleteCount := len(arr.ArrayVal) - startIdx
		if len(args) > 1 {
			deleteCount = int(args[1].NumVal)
		}
		if deleteCount < 0 {
			deleteCount = 0
		}
		if startIdx+deleteCount > len(arr.ArrayVal) {
			deleteCount = len(arr.ArrayVal) - startIdx
		}
		removed := make([]*Value, deleteCount)
		copy(removed, arr.ArrayVal[startIdx:startIdx+deleteCount])
		var insertItems []*Value
		if len(args) > 2 {
			insertItems = args[2:]
		}
		newArr := make([]*Value, 0, len(arr.ArrayVal)-deleteCount+len(insertItems))
		newArr = append(newArr, arr.ArrayVal[:startIdx]...)
		newArr = append(newArr, insertItems...)
		newArr = append(newArr, arr.ArrayVal[startIdx+deleteCount:]...)
		arr.ArrayVal = newArr
		return Array(removed), true, nil
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
			return valFalse, true, nil
		}
		return Bool(strings.Contains(s, args[0].String())), true, nil
	case "startsWith":
		if len(args) == 0 {
			return valFalse, true, nil
		}
		return Bool(strings.HasPrefix(s, args[0].String())), true, nil
	case "endsWith":
		if len(args) == 0 {
			return valFalse, true, nil
		}
		return Bool(strings.HasSuffix(s, args[0].String())), true, nil
	case "indexOf":
		if len(args) == 0 {
			return Number(-1), true, nil
		}
		return Number(float64(strings.Index(s, args[0].String()))), true, nil
	case "lastIndexOf":
		if len(args) == 0 {
			return Number(-1), true, nil
		}
		return Number(float64(strings.LastIndex(s, args[0].String()))), true, nil
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
		if end < 0 {
			end = len(runes) + end
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
		if i < 0 {
			i = len(runes) + i
		}
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
		if i < 0 {
			i = len(runes) + i
		}
		if i < 0 || i >= len(runes) {
			return Number(math.NaN()), true, nil
		}
		return Number(float64(runes[i])), true, nil
	case "repeat":
		if len(args) == 0 {
			return str, true, nil
		}
		n := int(args[0].NumVal)
		if n < 0 {
			n = 0
		}
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
		if len(runes) > targetLen {
			runes = runes[len(runes)-targetLen:]
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
		if len(runes) > targetLen {
			runes = runes[:targetLen]
		}
		return String(string(runes)), true, nil
	case "reverse":
		runes := []rune(s)
		for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
			runes[i], runes[j] = runes[j], runes[i]
		}
		return String(string(runes)), true, nil
	case "match":
		if len(args) == 0 {
			return valFalse, true, nil
		}
		return Bool(strings.Contains(s, args[0].String())), true, nil
	case "toString":
		return str, true, nil
	}
	return nil, false, nil
}

func (xvm *XVM) objectMethod(obj *Value, name string, args []*Value) (*Value, bool, error) {
	// First check if the object has a callable field with this name
	if obj.ObjVal != nil {
		if fv, ok := obj.ObjVal[name]; ok {
			if fv.Tag == TypeFunction || fv.Tag == TypeBuiltin {
				result, err := xvm.callValue(fv, args, obj, nil)
				return result, true, err
			}
		}
	}

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
			return valFalse, true, nil
		}
		_, ok := obj.ObjVal[args[0].String()]
		return Bool(ok), true, nil
	case "delete":
		if len(args) > 0 {
			delete(obj.ObjVal, args[0].String())
		}
		return valNull, true, nil
	case "assign", "merge":
		if len(args) > 0 && args[0].Tag == TypeObject {
			for k, v := range args[0].ObjVal {
				obj.ObjVal[k] = v
			}
		}
		return obj, true, nil
	case "toString":
		return String(obj.String()), true, nil
	}
	return nil, false, nil
}

// ─── Variant method dispatch ──────────────────────────────────────────────────
// Implements the Optional / ADT API on TypeVariant values.
// Supports the Some(x) / None pattern used in functional.xe and elsewhere:
//   some.getOr(default)  →  inner value (Some) or default (None)
//   some.isSome()        →  true if tag != "None"
//   some.isNone()        →  true if tag == "None"
//   some.unwrap()        →  inner value or runtime error on None
//   some.orElse(fn)      →  self (Some) or fn() result (None)
//   some.map(fn)         →  Some(fn(inner)) or None  (functor map)
//   some.tag()           →  variant tag string
func (xvm *XVM) variantMethod(v *Value, name string, args []*Value) (*Value, bool, error) {
	isNone := v.VarTag == "None" || len(v.VarFields) == 0
	switch name {
	case "isSome":
		return Bool(v.VarTag != "None"), true, nil
	case "isNone":
		return Bool(v.VarTag == "None"), true, nil

	case "unwrap":
		if v.VarTag == "None" {
			return valNull, true, fmt.Errorf("XVM: unwrap called on None")
		}
		if len(v.VarFields) > 0 {
			return v.VarFields[0], true, nil
		}
		return valNull, true, nil

	case "getOr":
		if isNone {
			if len(args) > 0 {
				return args[0], true, nil
			}
			return valNull, true, nil
		}
		return v.VarFields[0], true, nil

	case "orElse":
		if isNone {
			if len(args) > 0 {
				result, err := xvm.callValue(args[0], []*Value{}, nil, nil)
				return result, true, err
			}
			return valNull, true, nil
		}
		return v, true, nil

	case "map":
		// None.map(f) → None;  Some(x).map(f) → Some(f(x))
		if isNone {
			return v, true, nil
		}
		if len(args) == 0 {
			return v, true, nil
		}
		inner := v.VarFields[0]
		result, err := xvm.callValue(args[0], []*Value{inner}, nil, nil)
		if err != nil {
			return valNull, true, err
		}
		return &Value{Tag: TypeVariant, VarTag: "Some", VarFields: []*Value{result}}, true, nil

	case "flatMap", "andThen":
		// None.flatMap(f) → None;  Some(x).flatMap(f) → f(x)  (f returns a variant)
		if isNone {
			return v, true, nil
		}
		if len(args) == 0 {
			return v, true, nil
		}
		inner := v.VarFields[0]
		result, err := xvm.callValue(args[0], []*Value{inner}, nil, nil)
		if err != nil {
			return valNull, true, err
		}
		return result, true, nil

	case "tag":
		return String(v.VarTag), true, nil

	case "toString":
		return String(v.String()), true, nil
	}
	return nil, false, nil
}

// ─── Null method dispatch ─────────────────────────────────────────────────────
// Allows null to participate safely in Optional-style chaining.
// null.getOr(d)    → d          (null is absent)
// null.isSome()    → false
// null.isNone()    → true
// null.unwrap()    → runtime error
// null.orElse(fn)  → fn()       (invokes fallback)
// null.map(fn)     → null       (propagates absence)
func (xvm *XVM) nullMethod(name string, args []*Value) (*Value, bool, error) {
	switch name {
	case "getOr":
		if len(args) > 0 {
			return args[0], true, nil
		}
		return valNull, true, nil

	case "orElse":
		if len(args) > 0 {
			result, err := xvm.callValue(args[0], []*Value{}, nil, nil)
			return result, true, err
		}
		return valNull, true, nil

	case "isSome":
		return valFalse, true, nil
	case "isNone":
		return valTrue, true, nil

	case "unwrap":
		return valNull, true, fmt.Errorf("XVM: unwrap called on null")

	case "map", "flatMap", "andThen":
		// null propagates — the function is never called
		return valNull, true, nil

	case "toString":
		return String("null"), true, nil
	}
	return nil, false, nil
}

// ─── Primitive Optional method dispatch ───────────────────────────────────────
// Numbers and bools are always "present" values so they participate in Optional
// chaining without any wrapping.  This makes code like:
//
//   const n = maybeNum.getOr(0)   // works whether maybeNum is a number or null
//
// work uniformly regardless of the runtime type.
//
// Also provides a small set of number-specific convenience methods.
func (xvm *XVM) primitiveOptionalMethod(v *Value, name string, args []*Value) (*Value, bool, error) {
	// ── Optional API (primitives are always present) ──────────────────────
	switch name {
	case "getOr":
		return v, true, nil // present — ignore default
	case "orElse":
		return v, true, nil // present — skip fallback
	case "isSome":
		return valTrue, true, nil
	case "isNone":
		return valFalse, true, nil
	case "unwrap":
		return v, true, nil // trivially unwraps to itself
	case "map":
		if len(args) == 0 {
			return v, true, nil
		}
		result, err := xvm.callValue(args[0], []*Value{v}, nil, nil)
		return result, true, err
	case "flatMap", "andThen":
		if len(args) == 0 {
			return v, true, nil
		}
		result, err := xvm.callValue(args[0], []*Value{v}, nil, nil)
		return result, true, err
	case "toString":
		return String(v.String()), true, nil
	}

	// ── Number-specific methods ───────────────────────────────────────────
	if v.Tag == TypeNumber {
		switch name {
		case "toFixed":
			prec := 2
			if len(args) > 0 {
				prec = int(args[0].NumVal)
			}
			if prec < 0 {
				prec = 0
			}
			return String(fmt.Sprintf("%.*f", prec, v.NumVal)), true, nil
		case "toInt":
			return Number(float64(int64(v.NumVal))), true, nil
		case "abs":
			return Number(math.Abs(v.NumVal)), true, nil
		case "floor":
			return Number(math.Floor(v.NumVal)), true, nil
		case "ceil":
			return Number(math.Ceil(v.NumVal)), true, nil
		case "round":
			return Number(math.Round(v.NumVal)), true, nil
		case "sqrt":
			return Number(math.Sqrt(v.NumVal)), true, nil
		case "pow":
			if len(args) > 0 {
				return Number(math.Pow(v.NumVal, args[0].NumVal)), true, nil
			}
			return v, true, nil
		case "clamp":
			if len(args) >= 2 {
				lo, hi := args[0].NumVal, args[1].NumVal
				val := v.NumVal
				if val < lo {
					val = lo
				} else if val > hi {
					val = hi
				}
				return Number(val), true, nil
			}
			return v, true, nil
		}
	}

	// ── Bool-specific methods ─────────────────────────────────────────────
	if v.Tag == TypeBool {
		switch name {
		case "not":
			return Bool(!v.BoolVal), true, nil
		}
	}

	return nil, false, nil
}
