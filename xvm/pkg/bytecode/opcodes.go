/*
 * XENLY - Xenly Virtual Machine (XVM)
 * XVM Bytecode Instruction Set
 *
 * Created, designed, and developed by Cyril John Magayaga.
 * XVM Go implementation: xenlybyc (bytecode compiler) + xenlyrun (VM launcher)
 *
 * Supported platforms: Windows, macOS, Linux
 */
package bytecode

// Opcode is a single-byte VM instruction.
type Opcode byte

const (
	// ── Stack manipulation ────────────────────────────────────────────────
	OP_NOP   Opcode = 0x00 // no-op
	OP_HALT  Opcode = 0x01 // stop execution
	OP_POP   Opcode = 0x02 // discard top of stack
	OP_DUP   Opcode = 0x03 // duplicate top of stack
	OP_SWAP  Opcode = 0x04 // swap top two values

	// ── Literals ──────────────────────────────────────────────────────────
	OP_PUSH_NULL  Opcode = 0x10 // push null
	OP_PUSH_TRUE  Opcode = 0x11 // push true
	OP_PUSH_FALSE Opcode = 0x12 // push false
	OP_PUSH_CONST Opcode = 0x13 // push constant[uint32]

	// ── Arithmetic ────────────────────────────────────────────────────────
	OP_ADD Opcode = 0x20 // number add OR string concat
	OP_SUB Opcode = 0x21
	OP_MUL Opcode = 0x22
	OP_DIV Opcode = 0x23
	OP_MOD Opcode = 0x24
	OP_POW Opcode = 0x25 // **
	OP_NEG Opcode = 0x26 // unary minus

	// ── Bitwise ───────────────────────────────────────────────────────────
	OP_BIT_AND Opcode = 0x27 // &
	OP_BIT_OR  Opcode = 0x28 // |
	OP_BIT_XOR Opcode = 0x29 // ^
	OP_BIT_NOT Opcode = 0x2A // ~ (unary)
	OP_SHL     Opcode = 0x2B // <<
	OP_SHR     Opcode = 0x2C // >>

	// ── Comparison ────────────────────────────────────────────────────────
	OP_EQ  Opcode = 0x30
	OP_NEQ Opcode = 0x31
	OP_LT  Opcode = 0x32
	OP_LE  Opcode = 0x33
	OP_GT  Opcode = 0x34
	OP_GE  Opcode = 0x35

	// ── Logical ───────────────────────────────────────────────────────────
	OP_AND     Opcode = 0x40 // &&
	OP_OR      Opcode = 0x41 // ||
	OP_NOT     Opcode = 0x42 // !
	OP_NULLISH Opcode = 0x43 // ?? (null coalescing)

	// ── Variables ─────────────────────────────────────────────────────────
	OP_LOAD        Opcode = 0x50 // load var name[uint32]
	OP_STORE       Opcode = 0x51 // assign var name[uint32]
	OP_STORE_NEW   Opcode = 0x52 // declare var name[uint32]
	OP_STORE_CONST Opcode = 0x53 // declare const name[uint32]
	OP_STORE_LET   Opcode = 0x54 // declare let (block-scoped) name[uint32]

	// ── Control flow ──────────────────────────────────────────────────────
	OP_JUMP       Opcode = 0x60 // unconditional jump to abs addr[int32]
	OP_JUMP_TRUE  Opcode = 0x61 // jump if top truthy, pop condition[int32]
	OP_JUMP_FALSE Opcode = 0x62 // jump if top falsy,  pop condition[int32]
	OP_JUMP_NULL  Opcode = 0x63 // jump if top is null (peek, don't pop)[int32]

	// ── Functions ─────────────────────────────────────────────────────────
	OP_CALL         Opcode = 0x70 // call function  argc[uint8]
	OP_CALL_METHOD  Opcode = 0x71 // call method    name_idx[uint32] argc[uint8]
	OP_RETURN       Opcode = 0x72 // return null
	OP_RETURN_VAL   Opcode = 0x73 // return top of stack
	OP_MAKE_FN      Opcode = 0x74 // push function value  fn_idx[uint32]
	OP_MAKE_CLOSURE Opcode = 0x75 // push closure (captures env) fn_idx[uint32]

	// ── Scope ─────────────────────────────────────────────────────────────
	OP_PUSH_SCOPE Opcode = 0x80 // enter new lexical scope
	OP_POP_SCOPE  Opcode = 0x81 // exit current scope

	// ── Arrays ────────────────────────────────────────────────────────────
	OP_MAKE_ARRAY Opcode = 0x90 // build array from stack count[uint32]
	OP_INDEX_GET  Opcode = 0x91 // arr[idx]  (pop idx, pop arr, push val)
	OP_INDEX_SET  Opcode = 0x92 // arr[idx]=val (pop val, pop idx, pop arr)
	OP_ARRAY_LEN  Opcode = 0x93 // push length of array on TOS

	// ── Objects ───────────────────────────────────────────────────────────
	OP_MAKE_OBJ     Opcode = 0xA0 // build object from stack  pair_count[uint32]
	OP_PROP_GET     Opcode = 0xA1 // obj.name  name_idx[uint32]
	OP_PROP_SET     Opcode = 0xA2 // obj.name=val  name_idx[uint32]
	OP_PROP_GET_DYN Opcode = 0xA3 // obj[name_on_stack]
	OP_PROP_SET_DYN Opcode = 0xA4 // obj[name_on_stack]=val

	// ── OOP ───────────────────────────────────────────────────────────────
	OP_NEW          Opcode = 0xB0 // new ClassName(args)  name_idx[uint32] argc[uint8]
	OP_THIS_LOAD    Opcode = 0xB1 // push 'this'
	OP_SUPER_CALL   Opcode = 0xB2 // super(args)  argc[uint8]
	OP_TYPEOF       Opcode = 0xB3 // typeof expr → string
	OP_INSTANCEOF   Opcode = 0xB4 // expr instanceof ClassName  name_idx[uint32]
	OP_DEFINE_CLASS Opcode = 0xB5 // define class  class_idx[uint32]

	// ── Built-in I/O ──────────────────────────────────────────────────────
	OP_PRINT Opcode = 0xC0 // print count[uint8] values
	OP_INPUT Opcode = 0xC1 // input, has_prompt[uint8]
	OP_SLEEP Opcode = 0xC2 // sleep(ms)
	OP_ASSERT Opcode = 0xC3 // assert(cond, msg)

	// ── Enums / Pattern matching ──────────────────────────────────────────
	OP_MAKE_VARIANT  Opcode = 0xD0 // create ADT variant  tag_idx[uint32] fields[uint8]
	OP_MATCH_TAG     Opcode = 0xD1 // peek TOS, push bool (tag matches) tag_idx[uint32]
	OP_EXTRACT_FIELD Opcode = 0xD2 // push variant field  field_idx[uint8]

	// ── Iteration (for-in) ────────────────────────────────────────────────
	OP_ITER_BEGIN Opcode = 0xE0 // pop array, push iterator cursor (int=-1 sentinel)
	OP_ITER_NEXT  Opcode = 0xE1 // advance iterator; jump if exhausted  done_addr[int32]
	OP_ITER_VALUE Opcode = 0xE2 // push current element (iterator on TOS)

	// ── Misc ──────────────────────────────────────────────────────────────
	OP_TO_STRING  Opcode = 0xF0 // coerce TOS to string
	OP_CONCAT_STR Opcode = 0xF1 // concatenate count[uint8] strings
	OP_INC        Opcode = 0xF2 // increment variable name[uint32]  (x++)
	OP_DEC        Opcode = 0xF3 // decrement variable name[uint32]  (x--)
	OP_SPREAD     Opcode = 0xF4 // spread array onto stack  result_count[uint32] pushed
)

// OpName maps opcodes to human-readable names for disassembly.
var OpName = map[Opcode]string{
	OP_NOP: "NOP", OP_HALT: "HALT", OP_POP: "POP", OP_DUP: "DUP", OP_SWAP: "SWAP",
	OP_PUSH_NULL: "PUSH_NULL", OP_PUSH_TRUE: "PUSH_TRUE", OP_PUSH_FALSE: "PUSH_FALSE",
	OP_PUSH_CONST: "PUSH_CONST",
	OP_ADD: "ADD", OP_SUB: "SUB", OP_MUL: "MUL", OP_DIV: "DIV",
	OP_MOD: "MOD", OP_POW: "POW", OP_NEG: "NEG",
	OP_BIT_AND: "BIT_AND", OP_BIT_OR: "BIT_OR", OP_BIT_XOR: "BIT_XOR",
	OP_BIT_NOT: "BIT_NOT", OP_SHL: "SHL", OP_SHR: "SHR",
	OP_EQ: "EQ", OP_NEQ: "NEQ", OP_LT: "LT", OP_LE: "LE", OP_GT: "GT", OP_GE: "GE",
	OP_AND: "AND", OP_OR: "OR", OP_NOT: "NOT", OP_NULLISH: "NULLISH",
	OP_LOAD: "LOAD", OP_STORE: "STORE", OP_STORE_NEW: "STORE_NEW",
	OP_STORE_CONST: "STORE_CONST", OP_STORE_LET: "STORE_LET",
	OP_JUMP: "JUMP", OP_JUMP_TRUE: "JUMP_TRUE", OP_JUMP_FALSE: "JUMP_FALSE",
	OP_JUMP_NULL: "JUMP_NULL",
	OP_CALL: "CALL", OP_CALL_METHOD: "CALL_METHOD", OP_RETURN: "RETURN",
	OP_RETURN_VAL: "RETURN_VAL", OP_MAKE_FN: "MAKE_FN", OP_MAKE_CLOSURE: "MAKE_CLOSURE",
	OP_PUSH_SCOPE: "PUSH_SCOPE", OP_POP_SCOPE: "POP_SCOPE",
	OP_MAKE_ARRAY: "MAKE_ARRAY", OP_INDEX_GET: "INDEX_GET", OP_INDEX_SET: "INDEX_SET",
	OP_ARRAY_LEN: "ARRAY_LEN",
	OP_MAKE_OBJ: "MAKE_OBJ", OP_PROP_GET: "PROP_GET", OP_PROP_SET: "PROP_SET",
	OP_PROP_GET_DYN: "PROP_GET_DYN", OP_PROP_SET_DYN: "PROP_SET_DYN",
	OP_NEW: "NEW", OP_THIS_LOAD: "THIS_LOAD", OP_SUPER_CALL: "SUPER_CALL",
	OP_TYPEOF: "TYPEOF", OP_INSTANCEOF: "INSTANCEOF", OP_DEFINE_CLASS: "DEFINE_CLASS",
	OP_PRINT: "PRINT", OP_INPUT: "INPUT", OP_SLEEP: "SLEEP", OP_ASSERT: "ASSERT",
	OP_MAKE_VARIANT: "MAKE_VARIANT", OP_MATCH_TAG: "MATCH_TAG",
	OP_EXTRACT_FIELD: "EXTRACT_FIELD",
	OP_ITER_BEGIN: "ITER_BEGIN", OP_ITER_NEXT: "ITER_NEXT", OP_ITER_VALUE: "ITER_VALUE",
	OP_TO_STRING: "TO_STRING", OP_CONCAT_STR: "CONCAT_STR",
	OP_INC: "INC", OP_DEC: "DEC", OP_SPREAD: "SPREAD",
}

// OpOperandSize returns the byte count of operands following the opcode byte.
var OpOperandSize = map[Opcode]int{
	OP_NOP: 0, OP_HALT: 0, OP_POP: 0, OP_DUP: 0, OP_SWAP: 0,
	OP_PUSH_NULL: 0, OP_PUSH_TRUE: 0, OP_PUSH_FALSE: 0, OP_PUSH_CONST: 4,
	OP_ADD: 0, OP_SUB: 0, OP_MUL: 0, OP_DIV: 0, OP_MOD: 0, OP_POW: 0, OP_NEG: 0,
	OP_BIT_AND: 0, OP_BIT_OR: 0, OP_BIT_XOR: 0, OP_BIT_NOT: 0, OP_SHL: 0, OP_SHR: 0,
	OP_EQ: 0, OP_NEQ: 0, OP_LT: 0, OP_LE: 0, OP_GT: 0, OP_GE: 0,
	OP_AND: 0, OP_OR: 0, OP_NOT: 0, OP_NULLISH: 0,
	OP_LOAD: 4, OP_STORE: 4, OP_STORE_NEW: 4, OP_STORE_CONST: 4, OP_STORE_LET: 4,
	OP_JUMP: 4, OP_JUMP_TRUE: 4, OP_JUMP_FALSE: 4, OP_JUMP_NULL: 4,
	OP_CALL: 1, OP_CALL_METHOD: 5, OP_RETURN: 0, OP_RETURN_VAL: 0,
	OP_MAKE_FN: 4, OP_MAKE_CLOSURE: 4,
	OP_PUSH_SCOPE: 0, OP_POP_SCOPE: 0,
	OP_MAKE_ARRAY: 4, OP_INDEX_GET: 0, OP_INDEX_SET: 0, OP_ARRAY_LEN: 0,
	OP_MAKE_OBJ: 4, OP_PROP_GET: 4, OP_PROP_SET: 4,
	OP_PROP_GET_DYN: 0, OP_PROP_SET_DYN: 0,
	OP_NEW: 5, OP_THIS_LOAD: 0, OP_SUPER_CALL: 1,
	OP_TYPEOF: 0, OP_INSTANCEOF: 4, OP_DEFINE_CLASS: 4,
	OP_PRINT: 1, OP_INPUT: 1, OP_SLEEP: 0, OP_ASSERT: 0,
	OP_MAKE_VARIANT: 5, OP_MATCH_TAG: 4, OP_EXTRACT_FIELD: 1,
	OP_ITER_BEGIN: 0, OP_ITER_NEXT: 4, OP_ITER_VALUE: 0,
	OP_TO_STRING: 0, OP_CONCAT_STR: 1,
	OP_INC: 4, OP_DEC: 4, OP_SPREAD: 0,
}
