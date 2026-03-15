/*
 * XENLY - Xenly Virtual Machine (XVM)
 * VM: Value representation
 */
package vm

import (
	"fmt"
	"math"
	"strconv"
	"strings"
)

// ValueType identifies the runtime type of a Value.
type ValueType int

const (
	TypeNull ValueType = iota
	TypeNumber
	TypeString
	TypeBool
	TypeFunction
	TypeBuiltin
	TypeArray
	TypeObject
	TypeClass
	TypeInstance
	TypeVariant
	TypeIterator
)

// Value is a dynamically typed runtime value.
type Value struct {
	Tag       ValueType
	NumVal    float64
	StrVal    string
	BoolVal   bool
	FnVal     *FunctionVal
	BuiltinV  BuiltinFn
	ArrayVal  []*Value
	ObjVal    map[string]*Value
	ClassVal  *ClassVal
	InstVal   *InstanceVal
	VarTag    string   // for TypeVariant
	VarFields []*Value // for TypeVariant
	IterVal   *IterVal
}

// BuiltinFn is the signature for native built-in functions.
type BuiltinFn func(args []*Value) (*Value, error)

// FunctionVal holds a compiled function.
type FunctionVal struct {
	Name    string
	Params  []ParamDef
	Code    []byte
	IsAsync bool
	Closure *Env // captured environment (closures)
}

// ParamDef describes one function parameter.
type ParamDef struct {
	Name       string
	HasDefault bool
	IsRest     bool
	Default    *Value // pre-compiled? no — for now, null
}

// ClassVal holds a class definition.
type ClassVal struct {
	Name    string
	Parent  *ClassVal
	Methods map[string]*FunctionVal
}

// InstanceVal is an instantiated object.
type InstanceVal struct {
	Class  *ClassVal
	Fields map[string]*Value
}

// IterVal is an iterator over an array.
type IterVal struct {
	Items []*Value
	Index int
}

// ─── Constructors ─────────────────────────────────────────────────────────────

func Null() *Value  { return &Value{Tag: TypeNull} }
func True() *Value  { return &Value{Tag: TypeBool, BoolVal: true} }
func False() *Value { return &Value{Tag: TypeBool, BoolVal: false} }
func Bool(b bool) *Value {
	if b {
		return True()
	}
	return False()
}
func Number(n float64) *Value { return &Value{Tag: TypeNumber, NumVal: n} }
func String(s string) *Value  { return &Value{Tag: TypeString, StrVal: s} }
func Array(items []*Value) *Value {
	if items == nil {
		items = []*Value{}
	}
	return &Value{Tag: TypeArray, ArrayVal: items}
}
func Object(m map[string]*Value) *Value {
	if m == nil {
		m = make(map[string]*Value)
	}
	return &Value{Tag: TypeObject, ObjVal: m}
}

// Clone creates a deep copy of a value. Use sparingly - only when needed for isolation.
// For performance-critical paths, consider using shallow copy or reference sharing.
func (v *Value) Clone() *Value {
	if v == nil {
		return nil
	}

	switch v.Tag {
	case TypeNull:
		return Null()
	case TypeNumber:
		return Number(v.NumVal)
	case TypeString:
		return String(v.StrVal)
	case TypeBool:
		return Bool(v.BoolVal)
	case TypeArray:
		// For arrays, we can optimize by using shallow copy when safe
		// Deep clone only if elements are mutable types
		if v.ArrayVal == nil {
			return Array(nil)
		}
		// Check if we can use shallow copy (all elements are immutable)
		canShallow := true
		for _, item := range v.ArrayVal {
			if item.Tag == TypeArray || item.Tag == TypeObject {
				canShallow = false
				break
			}
		}
		if canShallow {
			// Shallow copy - much faster
			cloned := make([]*Value, len(v.ArrayVal))
			copy(cloned, v.ArrayVal)
			return Array(cloned)
		}
		// Fall back to deep clone for mutable elements
		cloned := make([]*Value, len(v.ArrayVal))
		for i, item := range v.ArrayVal {
			cloned[i] = item.Clone()
		}
		return Array(cloned)
	case TypeObject:
		// Deep clone object properties
		if v.ObjVal == nil {
			return Object(nil)
		}
		cloned := make(map[string]*Value)
		for k, val := range v.ObjVal {
			cloned[k] = val.Clone()
		}
		return Object(cloned)
	case TypeFunction:
		// Functions are reference types, shallow copy is fine
		return &Value{
			Tag:   TypeFunction,
			FnVal: v.FnVal,
		}
	case TypeBuiltin:
		// Builtins are reference types
		return &Value{
			Tag:      TypeBuiltin,
			BuiltinV: v.BuiltinV,
		}
	case TypeClass:
		// Classes are reference types
		return &Value{
			Tag:      TypeClass,
			ClassVal: v.ClassVal,
		}
	case TypeInstance:
		// Deep clone instance fields
		if v.InstVal == nil {
			return &Value{Tag: TypeInstance, InstVal: nil}
		}
		clonedFields := make(map[string]*Value)
		for k, val := range v.InstVal.Fields {
			clonedFields[k] = val.Clone()
		}
		clonedInst := &InstanceVal{
			Class:  v.InstVal.Class,
			Fields: clonedFields,
		}
		return &Value{Tag: TypeInstance, InstVal: clonedInst}
	case TypeVariant:
		// Deep clone variant fields
		if v.VarFields == nil {
			return &Value{Tag: TypeVariant, VarTag: v.VarTag, VarFields: nil}
		}
		clonedFields := make([]*Value, len(v.VarFields))
		for i, field := range v.VarFields {
			clonedFields[i] = field.Clone()
		}
		return &Value{
			Tag:       TypeVariant,
			VarTag:    v.VarTag,
			VarFields: clonedFields,
		}
	case TypeIterator:
		// Deep clone iterator items
		if v.IterVal == nil {
			return &Value{Tag: TypeIterator, IterVal: nil}
		}
		clonedItems := make([]*Value, len(v.IterVal.Items))
		for i, item := range v.IterVal.Items {
			clonedItems[i] = item.Clone()
		}
		clonedIter := &IterVal{
			Items: clonedItems,
			Index: v.IterVal.Index,
		}
		return &Value{Tag: TypeIterator, IterVal: clonedIter}
	default:
		return Null()
	}
}

// ─── Truth testing ────────────────────────────────────────────────────────────

func (v *Value) Truthy() bool {
	if v == nil {
		return false
	}
	switch v.Tag {
	case TypeNull:
		return false
	case TypeBool:
		return v.BoolVal
	case TypeNumber:
		return v.NumVal != 0 && !math.IsNaN(v.NumVal)
	case TypeString:
		return v.StrVal != ""
	case TypeArray:
		return true
	case TypeObject:
		return true
	case TypeFunction, TypeBuiltin, TypeClass:
		return true
	}
	return true
}

// IsNull returns true if the value is null.
func (v *Value) IsNull() bool {
	return v == nil || v.Tag == TypeNull
}

// ─── Equality ─────────────────────────────────────────────────────────────────

func (a *Value) Equal(b *Value) bool {
	if a == nil || a.Tag == TypeNull {
		return b == nil || b.Tag == TypeNull
	}
	if b == nil || b.Tag == TypeNull {
		return false
	}
	if a.Tag != b.Tag {
		// number == string coercion: not done in Xenly (strict equality)
		return false
	}
	switch a.Tag {
	case TypeNumber:
		return a.NumVal == b.NumVal
	case TypeString:
		return a.StrVal == b.StrVal
	case TypeBool:
		return a.BoolVal == b.BoolVal
	case TypeArray:
		return &a == &b // reference equality
	case TypeObject:
		return &a == &b
	}
	return false
}

// ─── String conversion ────────────────────────────────────────────────────────

func (v *Value) String() string {
	if v == nil {
		return "null"
	}
	switch v.Tag {
	case TypeNull:
		return "null"
	case TypeNumber:
		if math.IsNaN(v.NumVal) {
			return "NaN"
		}
		if math.IsInf(v.NumVal, 1) {
			return "Inf"
		}
		if math.IsInf(v.NumVal, -1) {
			return "-Inf"
		}
		// Match C interpreter: whole numbers print as integer, floats as %g (6 sig figs)
		if v.NumVal == math.Trunc(v.NumVal) && math.Abs(v.NumVal) < 1e15 {
			return strconv.FormatInt(int64(v.NumVal), 10)
		}
		return fmt.Sprintf("%g", v.NumVal)
	case TypeString:
		return v.StrVal
	case TypeBool:
		if v.BoolVal {
			return "true"
		}
		return "false"
	case TypeArray:
		parts := make([]string, len(v.ArrayVal))
		for i, e := range v.ArrayVal {
			parts[i] = e.String()
		}
		return "[" + strings.Join(parts, ", ") + "]"
	case TypeObject:
		parts := make([]string, 0, len(v.ObjVal))
		for k, val := range v.ObjVal {
			parts = append(parts, fmt.Sprintf("%s: %s", k, val.String()))
		}
		return "{" + strings.Join(parts, ", ") + "}"
	case TypeFunction:
		if v.FnVal != nil {
			return fmt.Sprintf("<fn %s>", v.FnVal.Name)
		}
		return "<fn>"
	case TypeBuiltin:
		return "<builtin>"
	case TypeClass:
		if v.ClassVal != nil {
			return fmt.Sprintf("<class %s>", v.ClassVal.Name)
		}
		return "<class>"
	case TypeInstance:
		if v.InstVal != nil {
			return fmt.Sprintf("<%s instance>", v.InstVal.Class.Name)
		}
		return "<instance>"
	case TypeVariant:
		if len(v.VarFields) == 0 {
			return v.VarTag
		}
		parts := make([]string, len(v.VarFields))
		for i, f := range v.VarFields {
			parts[i] = f.String()
		}
		return fmt.Sprintf("%s(%s)", v.VarTag, strings.Join(parts, ", "))
	}
	return "?"
}

// TypeName returns the Xenly type name of the value.
func (v *Value) TypeName() string {
	if v == nil {
		return "null"
	}
	switch v.Tag {
	case TypeNull:
		return "null"
	case TypeNumber:
		return "number"
	case TypeString:
		return "string"
	case TypeBool:
		return "bool"
	case TypeFunction, TypeBuiltin:
		return "function"
	case TypeArray:
		return "array"
	case TypeObject:
		return "object"
	case TypeClass:
		return "class"
	case TypeInstance:
		if v.InstVal != nil {
			return v.InstVal.Class.Name
		}
		return "object"
	case TypeVariant:
		return v.VarTag
	}
	return "unknown"
}

// ─── Environment ──────────────────────────────────────────────────────────────

// Env is a lexical scope environment.
type Env struct {
	vars   map[string]*envEntry
	parent *Env
}

type envEntry struct {
	value   *Value
	isConst bool
}

// NewEnv creates a new environment.
func NewEnv(parent *Env) *Env {
	return &Env{vars: make(map[string]*envEntry), parent: parent}
}

// Define declares a new variable in the current scope.
func (e *Env) Define(name string, val *Value, isConst bool) {
	if val == nil {
		val = Null()
	}
	e.vars[name] = &envEntry{value: val, isConst: isConst}
}

// Set assigns to an existing variable (searching up the scope chain).
func (e *Env) Set(name string, val *Value) error {
	for cur := e; cur != nil; cur = cur.parent {
		if entry, ok := cur.vars[name]; ok {
			if entry.isConst {
				return fmt.Errorf("cannot assign to constant %q", name)
			}
			if val == nil {
				val = Null()
			}
			entry.value = val
			return nil
		}
	}
	// Auto-define at current scope if not found (global fallback)
	if val == nil {
		val = Null()
	}
	e.vars[name] = &envEntry{value: val}
	return nil
}

// Get retrieves a variable's value (searching up the scope chain).
func (e *Env) Get(name string) (*Value, bool) {
	for cur := e; cur != nil; cur = cur.parent {
		if entry, ok := cur.vars[name]; ok {
			return entry.value, true
		}
	}
	return nil, false
}

// MustGet retrieves a variable or returns null.
func (e *Env) MustGet(name string) *Value {
	v, ok := e.Get(name)
	if !ok {
		return Null()
	}
	return v
}
