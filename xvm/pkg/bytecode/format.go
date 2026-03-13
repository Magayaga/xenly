/*
 * XENLY - Xenly Virtual Machine (XVM)
 * XYC Binary File Format
 *
 * .xyc file layout:
 *
 *   Header (12 bytes)
 *     [0:4]   Magic    "XYC\x01"
 *     [4:5]   Version major (uint8)
 *     [5:6]   Version minor (uint8)
 *     [6:8]   Flags (uint16, reserved = 0)
 *     [8:12]  Constant pool count (uint32 LE)
 *
 *   Constant Pool  (pool_count entries)
 *     Each entry:
 *       [0]     Type  0=null  1=number  2=string  3=bool
 *       number: [1:9]   float64 LE
 *       string: [1:5]   len uint32 LE  [5:5+len]  UTF-8
 *       bool:   [1]     0x00 | 0x01
 *
 *   Class Table  (uint32 count, then entries)
 *     Each ClassDef:
 *       name_idx       uint32  (pool index)
 *       has_parent     uint8   (0 or 1)
 *       parent_idx     uint32  (pool index, only if has_parent=1)
 *       method_count   uint32
 *       Each method:
 *         method_name_idx  uint32
 *         fn_idx           uint32
 *
 *   Function Table  (uint32 count, then entries)
 *     Each FnDef:
 *       name_idx        uint32
 *       param_count     uint32
 *       Each param:
 *         name_idx      uint32
 *         has_default   uint8
 *         is_rest       uint8
 *       is_async        uint8
 *       code_len        uint32
 *       code            []byte
 *
 *   Main chunk:
 *     code_len          uint32
 *     code              []byte
 */
package bytecode

import (
	"encoding/binary"
	"fmt"
	"io"
	"math"
	"os"
)

var Magic = [4]byte{'X', 'Y', 'C', 0x01}

const (
	ConstNull   byte = 0
	ConstNumber byte = 1
	ConstString byte = 2
	ConstBool   byte = 3
)

// ─── Constant Pool ────────────────────────────────────────────────────────────

type Constant struct {
	Type    byte
	NumVal  float64
	StrVal  string
	BoolVal bool
}

// ─── Param ───────────────────────────────────────────────────────────────────

type Param struct {
	NameIdx    uint32
	HasDefault bool
	IsRest     bool
}

// ─── FnDef ───────────────────────────────────────────────────────────────────

type FnDef struct {
	NameIdx uint32
	Params  []Param
	IsAsync bool
	Code    []byte
}

// ─── ClassMethod ─────────────────────────────────────────────────────────────

type ClassMethod struct {
	NameIdx uint32
	FnIdx   uint32
}

// ─── ClassDef ────────────────────────────────────────────────────────────────

type ClassDef struct {
	NameIdx   uint32
	HasParent bool
	ParentIdx uint32 // pool index of parent class name
	Methods   []ClassMethod
}

// ─── Module (top-level file) ─────────────────────────────────────────────────

type Module struct {
	Pool      []Constant
	Classes   []ClassDef
	Functions []FnDef
	MainCode  []byte
}

// PoolString interns a string and returns its index.
func (m *Module) PoolString(s string) uint32 {
	for i, c := range m.Pool {
		if c.Type == ConstString && c.StrVal == s {
			return uint32(i)
		}
	}
	m.Pool = append(m.Pool, Constant{Type: ConstString, StrVal: s})
	return uint32(len(m.Pool) - 1)
}

// PoolNumber interns a number and returns its index.
func (m *Module) PoolNumber(n float64) uint32 {
	for i, c := range m.Pool {
		if c.Type == ConstNumber && c.NumVal == n {
			return uint32(i)
		}
	}
	m.Pool = append(m.Pool, Constant{Type: ConstNumber, NumVal: n})
	return uint32(len(m.Pool) - 1)
}

// PoolNull returns the pool index for null (always 0).
func (m *Module) PoolNull() uint32 {
	for i, c := range m.Pool {
		if c.Type == ConstNull {
			return uint32(i)
		}
	}
	m.Pool = append(m.Pool, Constant{Type: ConstNull})
	return uint32(len(m.Pool) - 1)
}

// ─── Write ────────────────────────────────────────────────────────────────────

func Write(m *Module, path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()

	le := binary.LittleEndian

	write := func(data interface{}) error {
		return binary.Write(f, le, data)
	}
	writeBytes := func(b []byte) error {
		_, err := f.Write(b)
		return err
	}
	writeU32 := func(v uint32) error { return write(v) }
	writeU8 := func(v uint8) error { return write(v) }
	writeU16 := func(v uint16) error { return write(v) }

	// Header
	if _, err := f.Write(Magic[:]); err != nil {
		return err
	}
	if err := writeU8(1); err != nil { // major
		return err
	}
	if err := writeU8(0); err != nil { // minor
		return err
	}
	if err := writeU16(0); err != nil { // flags
		return err
	}
	if err := writeU32(uint32(len(m.Pool))); err != nil {
		return err
	}

	// Constant pool
	for _, c := range m.Pool {
		if err := writeU8(c.Type); err != nil {
			return err
		}
		switch c.Type {
		case ConstNull:
			// nothing
		case ConstNumber:
			bits := math.Float64bits(c.NumVal)
			if err := write(bits); err != nil {
				return err
			}
		case ConstString:
			b := []byte(c.StrVal)
			if err := writeU32(uint32(len(b))); err != nil {
				return err
			}
			if err := writeBytes(b); err != nil {
				return err
			}
		case ConstBool:
			v := uint8(0)
			if c.BoolVal {
				v = 1
			}
			if err := writeU8(v); err != nil {
				return err
			}
		}
	}

	// Class table
	if err := writeU32(uint32(len(m.Classes))); err != nil {
		return err
	}
	for _, cd := range m.Classes {
		if err := writeU32(cd.NameIdx); err != nil {
			return err
		}
		hp := uint8(0)
		if cd.HasParent {
			hp = 1
		}
		if err := writeU8(hp); err != nil {
			return err
		}
		if cd.HasParent {
			if err := writeU32(cd.ParentIdx); err != nil {
				return err
			}
		}
		if err := writeU32(uint32(len(cd.Methods))); err != nil {
			return err
		}
		for _, mth := range cd.Methods {
			if err := writeU32(mth.NameIdx); err != nil {
				return err
			}
			if err := writeU32(mth.FnIdx); err != nil {
				return err
			}
		}
	}

	// Function table
	if err := writeU32(uint32(len(m.Functions))); err != nil {
		return err
	}
	for _, fn := range m.Functions {
		if err := writeU32(fn.NameIdx); err != nil {
			return err
		}
		if err := writeU32(uint32(len(fn.Params))); err != nil {
			return err
		}
		for _, p := range fn.Params {
			if err := writeU32(p.NameIdx); err != nil {
				return err
			}
			hd := uint8(0)
			if p.HasDefault {
				hd = 1
			}
			if err := writeU8(hd); err != nil {
				return err
			}
			ir := uint8(0)
			if p.IsRest {
				ir = 1
			}
			if err := writeU8(ir); err != nil {
				return err
			}
		}
		ia := uint8(0)
		if fn.IsAsync {
			ia = 1
		}
		if err := writeU8(ia); err != nil {
			return err
		}
		if err := writeU32(uint32(len(fn.Code))); err != nil {
			return err
		}
		if err := writeBytes(fn.Code); err != nil {
			return err
		}
	}

	// Main code
	if err := writeU32(uint32(len(m.MainCode))); err != nil {
		return err
	}
	return writeBytes(m.MainCode)
}

// ─── Read ─────────────────────────────────────────────────────────────────────

func Read(path string) (*Module, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	le := binary.LittleEndian

	readU8 := func() (uint8, error) {
		var v uint8
		err := binary.Read(f, le, &v)
		return v, err
	}
	readU16 := func() (uint16, error) {
		var v uint16
		err := binary.Read(f, le, &v)
		return v, err
	}
	readU32 := func() (uint32, error) {
		var v uint32
		err := binary.Read(f, le, &v)
		return v, err
	}
	readU64 := func() (uint64, error) {
		var v uint64
		err := binary.Read(f, le, &v)
		return v, err
	}
	readBytes := func(n uint32) ([]byte, error) {
		b := make([]byte, n)
		_, err := io.ReadFull(f, b)
		return b, err
	}

	// Header
	var magic [4]byte
	if _, err := io.ReadFull(f, magic[:]); err != nil {
		return nil, fmt.Errorf("read magic: %w", err)
	}
	if magic != Magic {
		return nil, fmt.Errorf("not a valid .xyc file (bad magic)")
	}
	major, _ := readU8()
	minor, _ := readU8()
	_ = major
	_ = minor
	_, _ = readU16() // flags
	poolCount, err := readU32()
	if err != nil {
		return nil, fmt.Errorf("read pool count: %w", err)
	}

	m := &Module{}

	// Pool
	m.Pool = make([]Constant, poolCount)
	for i := range m.Pool {
		typ, err := readU8()
		if err != nil {
			return nil, fmt.Errorf("pool[%d] type: %w", i, err)
		}
		m.Pool[i].Type = typ
		switch typ {
		case ConstNull:
		case ConstNumber:
			bits, err := readU64()
			if err != nil {
				return nil, err
			}
			m.Pool[i].NumVal = math.Float64frombits(bits)
		case ConstString:
			slen, err := readU32()
			if err != nil {
				return nil, err
			}
			b, err := readBytes(slen)
			if err != nil {
				return nil, err
			}
			m.Pool[i].StrVal = string(b)
		case ConstBool:
			v, err := readU8()
			if err != nil {
				return nil, err
			}
			m.Pool[i].BoolVal = v != 0
		default:
			return nil, fmt.Errorf("unknown pool type %d", typ)
		}
	}

	// Class table
	classCount, err := readU32()
	if err != nil {
		return nil, err
	}
	m.Classes = make([]ClassDef, classCount)
	for i := range m.Classes {
		cd := &m.Classes[i]
		if cd.NameIdx, err = readU32(); err != nil {
			return nil, err
		}
		hp, err := readU8()
		if err != nil {
			return nil, err
		}
		cd.HasParent = hp != 0
		if cd.HasParent {
			if cd.ParentIdx, err = readU32(); err != nil {
				return nil, err
			}
		}
		mCount, err := readU32()
		if err != nil {
			return nil, err
		}
		cd.Methods = make([]ClassMethod, mCount)
		for j := range cd.Methods {
			if cd.Methods[j].NameIdx, err = readU32(); err != nil {
				return nil, err
			}
			if cd.Methods[j].FnIdx, err = readU32(); err != nil {
				return nil, err
			}
		}
	}

	// Function table
	fnCount, err := readU32()
	if err != nil {
		return nil, err
	}
	m.Functions = make([]FnDef, fnCount)
	for i := range m.Functions {
		fn := &m.Functions[i]
		if fn.NameIdx, err = readU32(); err != nil {
			return nil, err
		}
		pCount, err := readU32()
		if err != nil {
			return nil, err
		}
		fn.Params = make([]Param, pCount)
		for j := range fn.Params {
			if fn.Params[j].NameIdx, err = readU32(); err != nil {
				return nil, err
			}
			hd, err := readU8()
			if err != nil {
				return nil, err
			}
			fn.Params[j].HasDefault = hd != 0
			ir, err := readU8()
			if err != nil {
				return nil, err
			}
			fn.Params[j].IsRest = ir != 0
		}
		ia, err := readU8()
		if err != nil {
			return nil, err
		}
		fn.IsAsync = ia != 0
		cLen, err := readU32()
		if err != nil {
			return nil, err
		}
		if fn.Code, err = readBytes(cLen); err != nil {
			return nil, err
		}
	}

	// Main code
	mLen, err := readU32()
	if err != nil {
		return nil, err
	}
	if m.MainCode, err = readBytes(mLen); err != nil {
		return nil, err
	}

	return m, nil
}

// ─── Disassemble ─────────────────────────────────────────────────────────────

func Disassemble(m *Module, w io.Writer) {
	poolStr := func(idx uint32) string {
		if int(idx) >= len(m.Pool) {
			return fmt.Sprintf("pool[%d]", idx)
		}
		c := m.Pool[idx]
		switch c.Type {
		case ConstNull:
			return "null"
		case ConstNumber:
			return fmt.Sprintf("%g", c.NumVal)
		case ConstString:
			return fmt.Sprintf("%q", c.StrVal)
		case ConstBool:
			if c.BoolVal {
				return "true"
			}
			return "false"
		}
		return "?"
	}

	disasmChunk := func(label string, code []byte) {
		fmt.Fprintf(w, "\n=== %s ===\n", label)
		ip := 0
		for ip < len(code) {
			op := Opcode(code[ip])
			name, ok := OpName[op]
			if !ok {
				fmt.Fprintf(w, "  %04d  UNKNOWN(0x%02X)\n", ip, code[ip])
				ip++
				continue
			}
			opSz := OpOperandSize[op]
			fmt.Fprintf(w, "  %04d  %-18s", ip, name)
			ip++
			switch opSz {
			case 0:
			case 1:
				if ip < len(code) {
					fmt.Fprintf(w, "  %d", code[ip])
					ip++
				}
			case 4:
				if ip+4 <= len(code) {
					v := binary.LittleEndian.Uint32(code[ip : ip+4])
					// If PUSH_CONST or LOAD/STORE, show pool value
					if op == OP_PUSH_CONST || op == OP_LOAD || op == OP_STORE ||
						op == OP_STORE_NEW || op == OP_STORE_CONST || op == OP_STORE_LET ||
						op == OP_PROP_GET || op == OP_PROP_SET || op == OP_INSTANCEOF ||
						op == OP_INC || op == OP_DEC || op == OP_MATCH_TAG ||
						op == OP_MAKE_FN || op == OP_MAKE_CLOSURE || op == OP_DEFINE_CLASS {
						fmt.Fprintf(w, "  %d (%s)", v, poolStr(v))
					} else {
						fmt.Fprintf(w, "  %d", int32(v))
					}
					ip += 4
				}
			case 5:
				if ip+5 <= len(code) {
					v := binary.LittleEndian.Uint32(code[ip : ip+4])
					fmt.Fprintf(w, "  %d (%s) argc=%d", v, poolStr(v), code[ip+4])
					ip += 5
				}
			}
			fmt.Fprintln(w)
		}
	}

	fmt.Fprintf(w, "XYC Disassembly\n")
	fmt.Fprintf(w, "Pool (%d constants):\n", len(m.Pool))
	for i, c := range m.Pool {
		fmt.Fprintf(w, "  [%d] %s\n", i, poolStr(uint32(i)))
		_ = c
	}

	fmt.Fprintf(w, "\nClasses (%d):\n", len(m.Classes))
	for i, cd := range m.Classes {
		parent := ""
		if cd.HasParent {
			parent = " extends " + poolStr(cd.ParentIdx)
		}
		fmt.Fprintf(w, "  [%d] class %s%s  (%d methods)\n",
			i, poolStr(cd.NameIdx), parent, len(cd.Methods))
	}

	fmt.Fprintf(w, "\nFunctions (%d):\n", len(m.Functions))
	for i, fn := range m.Functions {
		fmt.Fprintf(w, "  [%d] fn %s\n", i, poolStr(fn.NameIdx))
	}

	disasmChunk("main", m.MainCode)

	for i, fn := range m.Functions {
		disasmChunk(fmt.Sprintf("fn[%d] %s", i, poolStr(fn.NameIdx)), fn.Code)
	}
}
