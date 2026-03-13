/*
 * XENLY - Xenly Virtual Machine (XVM)
 * Compiler: AST → bytecode
 */
package compiler

import (
	"encoding/binary"
	"fmt"
	"math"

	"xvm/pkg/bytecode"
)

// Compiler compiles an AST to a bytecode.Module.
type Compiler struct {
	module  *bytecode.Module
	errors  []string
	// current function being compiled (nil = main chunk)
	current *fnState
	// global scope for class name resolution
	classes map[string]uint32 // class name → class table index
}

// fnState tracks compilation state for a function body.
type fnState struct {
	code      []byte
	fnIdx     uint32 // index in module.Functions (math.MaxUint32 = main chunk)
	loopBreaks []int  // positions to patch for break
	loopContinues []int // positions to patch for continue
	loopStart int    // position of loop start (for continue)
	parent    *fnState
}

// NewCompiler creates a new Compiler for the given module.
func NewCompiler() *Compiler {
	return &Compiler{
		module:  &bytecode.Module{},
		classes: make(map[string]uint32),
	}
}

// Compile compiles the given program AST and returns the module.
func (c *Compiler) Compile(program *ASTNode) (*bytecode.Module, []string) {
	c.current = &fnState{fnIdx: math.MaxUint32}
	c.compileStmts(program.Children)
	c.emit(bytecode.OP_HALT)
	c.module.MainCode = c.current.code
	return c.module, c.errors
}

// ─── Scope helpers ────────────────────────────────────────────────────────────

func (c *Compiler) emitScope() {
	c.emit(bytecode.OP_PUSH_SCOPE)
}

func (c *Compiler) emitEndScope() {
	c.emit(bytecode.OP_POP_SCOPE)
}

// ─── Emit helpers ─────────────────────────────────────────────────────────────

func (c *Compiler) code() []byte {
	return c.current.code
}

func (c *Compiler) setCode(b []byte) {
	c.current.code = b
}

func (c *Compiler) emit(op bytecode.Opcode) {
	c.current.code = append(c.current.code, byte(op))
}

func (c *Compiler) emitU8(v uint8) {
	c.current.code = append(c.current.code, v)
}

func (c *Compiler) emitU32(v uint32) {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], v)
	c.current.code = append(c.current.code, buf[:]...)
}

func (c *Compiler) emitI32(v int32) {
	c.emitU32(uint32(v))
}

func (c *Compiler) emitOp1(op bytecode.Opcode, arg uint8) {
	c.emit(op)
	c.emitU8(arg)
}

func (c *Compiler) emitOp4(op bytecode.Opcode, arg uint32) {
	c.emit(op)
	c.emitU32(arg)
}

func (c *Compiler) emitOpI4(op bytecode.Opcode, arg int32) {
	c.emit(op)
	c.emitI32(arg)
}

func (c *Compiler) emitOp4U1(op bytecode.Opcode, arg4 uint32, arg1 uint8) {
	c.emit(op)
	c.emitU32(arg4)
	c.emitU8(arg1)
}

// currentPos returns the current byte position.
func (c *Compiler) currentPos() int {
	return len(c.current.code)
}

// emitJump emits a jump with a placeholder; returns the position of the operand.
func (c *Compiler) emitJump(op bytecode.Opcode) int {
	c.emit(op)
	pos := c.currentPos()
	c.emitI32(0)
	return pos
}

// patchJump patches a previously emitted jump to the current position.
func (c *Compiler) patchJump(patchPos int) {
	target := int32(c.currentPos())
	binary.LittleEndian.PutUint32(c.current.code[patchPos:], uint32(target))
}

// ─── Constant pool helpers ────────────────────────────────────────────────────

func (c *Compiler) constString(s string) uint32 {
	return c.module.PoolString(s)
}

func (c *Compiler) constNumber(n float64) uint32 {
	return c.module.PoolNumber(n)
}

func (c *Compiler) pushString(s string) {
	c.emitOp4(bytecode.OP_PUSH_CONST, c.constString(s))
}

func (c *Compiler) pushNumber(n float64) {
	c.emitOp4(bytecode.OP_PUSH_CONST, c.constNumber(n))
}

// ─── Statement compilation ────────────────────────────────────────────────────

func (c *Compiler) compileStmts(stmts []*ASTNode) {
	for _, s := range stmts {
		if s != nil {
			c.compileStmt(s)
		}
	}
}

func (c *Compiler) compileStmt(n *ASTNode) {
	if n == nil {
		return
	}
	switch n.Kind {
	case NodeVarDecl:
		c.compileVarDecl(n, bytecode.OP_STORE_NEW)
	case NodeConstDecl:
		c.compileVarDecl(n, bytecode.OP_STORE_CONST)
	case NodeFnDecl:
		c.compileFnDecl(n)
	case NodeClassDecl:
		c.compileClassDecl(n)
	case NodeEnumDecl:
		c.compileEnumDecl(n)
	case NodeBlock:
		c.emitScope()
		c.compileStmts(n.Children)
		c.emitEndScope()
	case NodeExprStmt:
		if len(n.Children) > 0 {
			c.compileExpr(n.Children[0])
			c.emit(bytecode.OP_POP) // discard result
		}
	case NodeAssign:
		c.compileAssign(n)
	case NodeCompoundAssign:
		c.compileCompoundAssign(n)
	case NodeReturn:
		if len(n.Children) == 0 {
			c.emit(bytecode.OP_RETURN)
		} else {
			c.compileExpr(n.Children[0])
			c.emit(bytecode.OP_RETURN_VAL)
		}
	case NodeIf:
		c.compileIf(n)
	case NodeWhile:
		c.compileWhile(n)
	case NodeDoWhile:
		c.compileDoWhile(n)
	case NodeFor:
		c.compileFor(n)
	case NodeForIn:
		c.compileForIn(n)
	case NodeSwitch:
		c.compileSwitch(n)
	case NodeBreak:
		// Emit a placeholder jump; patch in loop end
		pos := c.emitJump(bytecode.OP_JUMP)
		c.current.loopBreaks = append(c.current.loopBreaks, pos)
	case NodeContinue:
		pos := c.emitJump(bytecode.OP_JUMP)
		c.current.loopContinues = append(c.current.loopContinues, pos)
	case NodePrint:
		c.compilePrint(n)
	case NodeIncrement:
		c.compileIncDec(n, bytecode.OP_INC)
	case NodeDecrement:
		c.compileIncDec(n, bytecode.OP_DEC)
	case NodeExport:
		if len(n.Children) > 0 {
			c.compileStmt(n.Children[0])
		}
	case NodeImport:
		// Runtime: nothing to emit; module is resolved at load time
		c.emit(bytecode.OP_NOP)
	case NodeNamespace:
		c.emitScope()
		if len(n.Children) > 0 {
			c.compileStmts(n.Children[0].Children)
		}
		c.emitEndScope()
	case NodeTypeAlias, NodeRequires, NodeEnsures, NodeInvariant:
		c.emit(bytecode.OP_NOP) // compile-time only
	case NodeAssert:
		c.compileAssert(n)
	case NodeSpawn:
		if len(n.Children) > 0 {
			c.compileExpr(n.Children[0])
			c.emit(bytecode.OP_POP)
		}
	default:
		// Expression that looks like a statement
		c.compileExpr(n)
		c.emit(bytecode.OP_POP)
	}
}

func (c *Compiler) compileVarDecl(n *ASTNode, storeOp bytecode.Opcode) {
	nameIdx := c.constString(n.StrVal)
	if len(n.Children) == 0 {
		c.emit(bytecode.OP_PUSH_NULL)
	} else {
		c.compileExpr(n.Children[0])
	}
	c.emitOp4(storeOp, nameIdx)
}

func (c *Compiler) compileFnDecl(n *ASTNode) {
	fnIdx := c.compileFn(n)
	if n.StrVal != "" {
		nameIdx := c.constString(n.StrVal)
		c.emitOp4(bytecode.OP_MAKE_CLOSURE, fnIdx)
		c.emitOp4(bytecode.OP_STORE_NEW, nameIdx)
	}
}

// compileFn compiles a function body and returns its index in the function table.
func (c *Compiler) compileFn(n *ASTNode) uint32 {
	// Build param list
	params := make([]bytecode.Param, len(n.Params))
	for i, p := range n.Params {
		params[i] = bytecode.Param{
			NameIdx:    c.constString(p.Name),
			HasDefault: p.DefaultValue != nil,
			IsRest:     p.IsRest,
		}
	}

	// Allocate function slot
	fnIdx := uint32(len(c.module.Functions))
	c.module.Functions = append(c.module.Functions, bytecode.FnDef{
		NameIdx: c.constString(n.StrVal),
		Params:  params,
		IsAsync: n.IsAsync,
	})

	// Save parent state and switch to new function context
	parent := c.current
	c.current = &fnState{fnIdx: fnIdx, parent: parent}

	// Bind default parameter values
	for i, p := range n.Params {
		if p.DefaultValue != nil {
			nameIdx := c.constString(p.Name)
			// if param is null/undefined, assign default
			c.emitOp4(bytecode.OP_LOAD, nameIdx)
			jmpPos := c.emitJump(bytecode.OP_JUMP_NULL)
			jmpSkip := c.emitJump(bytecode.OP_JUMP)
			c.patchJump(jmpPos)
			c.compileExpr(p.DefaultValue)
			c.emitOp4(bytecode.OP_STORE, nameIdx)
			c.patchJump(jmpSkip)
			_ = i
		}
	}

	// Compile body
	body := n.Children[0]
	if body != nil {
		if body.Kind == NodeBlock {
			c.compileStmts(body.Children)
		} else if body.Kind == NodeReturn {
			c.compileStmt(body)
		} else {
			c.compileStmt(body)
		}
	}
	c.emit(bytecode.OP_RETURN)

	// Finalize
	c.module.Functions[fnIdx].Code = c.current.code
	c.current = parent
	return fnIdx
}

func (c *Compiler) compileClassDecl(n *ASTNode) {
	cd := bytecode.ClassDef{
		NameIdx: c.constString(n.StrVal),
	}
	// children[0] = parent ident node or nil
	if len(n.Children) > 0 && n.Children[0] != nil {
		cd.HasParent = true
		cd.ParentIdx = c.constString(n.Children[0].StrVal)
	}

	// Compile methods (children[1..])
	classIdx := uint32(len(c.module.Classes))
	c.module.Classes = append(c.module.Classes, cd)
	c.classes[n.StrVal] = classIdx

	for _, child := range n.Children[1:] {
		if child == nil {
			continue
		}
		if child.Kind == NodeFnDecl {
			fnIdx := c.compileFn(child)
			methodNameIdx := c.constString(child.StrVal)
			c.module.Classes[classIdx].Methods = append(
				c.module.Classes[classIdx].Methods,
				bytecode.ClassMethod{NameIdx: methodNameIdx, FnIdx: fnIdx},
			)
		}
		// Field declarations are handled at runtime (init method)
	}

	c.emitOp4(bytecode.OP_DEFINE_CLASS, classIdx)
	nameIdx := c.constString(n.StrVal)
	c.emitOp4(bytecode.OP_STORE_NEW, nameIdx)
}

func (c *Compiler) compileEnumDecl(n *ASTNode) {
	// Each variant becomes either:
	//   - A pre-built variant VALUE  (0 params  → matches C interpreter behaviour)
	//   - A constructor FUNCTION     (≥1 params → callable)
	for _, variant := range n.Children {
		if variant == nil || variant.Kind != NodeEnumVariant {
			continue
		}
		varName := variant.StrVal
		fieldCount := len(variant.Children)
		varNameIdx := c.constString(varName)

		if fieldCount == 0 {
			// Nullary variant: store a pre-built variant value directly.
			// Emit: MAKE_VARIANT tag 0  →  STORE_NEW varName
			tagIdx := c.constString(varName)
			c.emitOp4(bytecode.OP_MAKE_VARIANT, tagIdx)
			c.emitU8(0)
			c.emitOp4(bytecode.OP_STORE_NEW, varNameIdx)
			continue
		}

		// Parametric variant: build a synthetic constructor function.
		params := make([]bytecode.Param, fieldCount)
		for i := 0; i < fieldCount; i++ {
			fieldName := fmt.Sprintf("f%d", i)
			if i < len(variant.Children) && variant.Children[i] != nil {
				if variant.Children[i].StrVal != "" {
					fieldName = variant.Children[i].StrVal
				}
			}
			params[i] = bytecode.Param{NameIdx: c.constString(fieldName)}
		}

		fnIdx := uint32(len(c.module.Functions))
		tagIdx := c.constString(varName)
		c.module.Functions = append(c.module.Functions, bytecode.FnDef{
			NameIdx: tagIdx,
			Params:  params,
		})

		parent := c.current
		c.current = &fnState{fnIdx: fnIdx}

		for i := 0; i < fieldCount; i++ {
			c.emitOp4(bytecode.OP_LOAD, params[i].NameIdx)
		}
		c.emitOp4(bytecode.OP_MAKE_VARIANT, tagIdx)
		c.emitU8(uint8(fieldCount))
		c.emit(bytecode.OP_RETURN_VAL)

		c.module.Functions[fnIdx].Code = c.current.code
		c.current = parent

		c.emitOp4(bytecode.OP_MAKE_CLOSURE, fnIdx)
		c.emitOp4(bytecode.OP_STORE_NEW, varNameIdx)
	}
	// Also store the enum name as a placeholder (ignored at runtime)
	nameIdx := c.constString(n.StrVal)
	c.emitOp4(bytecode.OP_MAKE_OBJ, 0)
	c.emitOp4(bytecode.OP_STORE_NEW, nameIdx)
}

func (c *Compiler) compileAssign(n *ASTNode) {
	if len(n.Children) < 2 {
		return
	}
	lhs := n.Children[0]
	rhs := n.Children[1]

	switch lhs.Kind {
	case NodeIdent:
		c.compileExpr(rhs)
		c.emitOp4(bytecode.OP_STORE, c.constString(lhs.StrVal))
		c.emit(bytecode.OP_POP) // OP_STORE peeks; pop the residual
	case NodePropertyGet:
		c.compileExpr(lhs.Children[0]) // object
		c.compileExpr(rhs)
		c.emitOp4(bytecode.OP_PROP_SET, c.constString(lhs.StrVal))
		// OP_PROP_SET pops both obj and val — no residual
	case NodeIndex:
		c.compileExpr(lhs.Children[0]) // array
		c.compileExpr(lhs.Children[1]) // index
		c.compileExpr(rhs)
		c.emit(bytecode.OP_INDEX_SET)
		// OP_INDEX_SET pops arr, idx, val — no residual
	default:
		c.compileExpr(rhs)
		c.emit(bytecode.OP_POP)
	}
}

func (c *Compiler) compileCompoundAssign(n *ASTNode) {
	if len(n.Children) < 2 {
		return
	}
	lhs := n.Children[0]
	rhs := n.Children[1]
	op := n.StrVal

	// Load lhs, compute rhs, apply op, store back
	c.compileExpr(lhs)
	c.compileExpr(rhs)
	switch op {
	case "+=":
		c.emit(bytecode.OP_ADD)
	case "-=":
		c.emit(bytecode.OP_SUB)
	case "*=":
		c.emit(bytecode.OP_MUL)
	case "/=":
		c.emit(bytecode.OP_DIV)
	case "%=":
		c.emit(bytecode.OP_MOD)
	case "**=":
		c.emit(bytecode.OP_POW)
	}

	switch lhs.Kind {
	case NodeIdent:
		c.emitOp4(bytecode.OP_STORE, c.constString(lhs.StrVal))
		c.emit(bytecode.OP_POP) // OP_STORE peeks; pop the residual
	case NodePropertyGet:
		c.compileExpr(lhs.Children[0])
		c.emit(bytecode.OP_SWAP)
		c.emitOp4(bytecode.OP_PROP_SET, c.constString(lhs.StrVal))
		// PROP_SET pops both — no residual
	}
}

func (c *Compiler) compileIf(n *ASTNode) {
	// children: [cond, then, else?]
	c.compileExpr(n.Children[0])
	elseJump := c.emitJump(bytecode.OP_JUMP_FALSE)

	c.emitScope()
	c.compileStmt(n.Children[1])
	c.emitEndScope()

	if len(n.Children) > 2 && n.Children[2] != nil {
		endJump := c.emitJump(bytecode.OP_JUMP)
		c.patchJump(elseJump)
		c.emitScope()
		c.compileStmt(n.Children[2])
		c.emitEndScope()
		c.patchJump(endJump)
	} else {
		c.patchJump(elseJump)
	}
}

func (c *Compiler) compileWhile(n *ASTNode) {
	// children: [cond, body]
	loopStart := c.currentPos()
	c.emitScope()

	c.compileExpr(n.Children[0])
	endJump := c.emitJump(bytecode.OP_JUMP_FALSE)

	oldBreaks := c.current.loopBreaks
	oldContinues := c.current.loopContinues
	oldStart := c.current.loopStart
	c.current.loopBreaks = nil
	c.current.loopContinues = nil
	c.current.loopStart = loopStart

	c.compileStmt(n.Children[1])

	// Continue → jump to loop start
	for _, pos := range c.current.loopContinues {
		binary.LittleEndian.PutUint32(c.current.code[pos:], uint32(loopStart))
	}
	c.emitOpI4(bytecode.OP_JUMP, int32(loopStart))
	c.patchJump(endJump)
	// Break → jump here
	for _, pos := range c.current.loopBreaks {
		c.patchJumpAt(pos)
	}

	c.emitEndScope()
	c.current.loopBreaks = oldBreaks
	c.current.loopContinues = oldContinues
	c.current.loopStart = oldStart
}

func (c *Compiler) compileDoWhile(n *ASTNode) {
	// children: [body, cond]
	loopStart := c.currentPos()

	oldBreaks := c.current.loopBreaks
	oldContinues := c.current.loopContinues
	c.current.loopBreaks = nil
	c.current.loopContinues = nil
	c.current.loopStart = loopStart

	c.emitScope()
	c.compileStmt(n.Children[0])
	c.emitEndScope()

	condStart := c.currentPos()
	for _, pos := range c.current.loopContinues {
		binary.LittleEndian.PutUint32(c.current.code[pos:], uint32(condStart))
	}

	c.compileExpr(n.Children[1])
	c.emitOpI4(bytecode.OP_JUMP_TRUE, int32(loopStart))

	for _, pos := range c.current.loopBreaks {
		c.patchJumpAt(pos)
	}
	c.current.loopBreaks = oldBreaks
	c.current.loopContinues = oldContinues
}

func (c *Compiler) compileFor(n *ASTNode) {
	// children: [init, cond, update, body]
	c.emitScope()

	// init
	if n.Children[0] != nil {
		c.compileStmt(n.Children[0])
	}

	loopStart := c.currentPos()
	var endJump int = -1

	// cond
	if n.Children[1] != nil {
		c.compileExpr(n.Children[1])
		endJump = c.emitJump(bytecode.OP_JUMP_FALSE)
	}

	oldBreaks := c.current.loopBreaks
	oldContinues := c.current.loopContinues
	c.current.loopBreaks = nil
	c.current.loopContinues = nil
	c.current.loopStart = loopStart

	// body
	c.compileStmt(n.Children[3])

	// update
	updateStart := c.currentPos()
	for _, pos := range c.current.loopContinues {
		binary.LittleEndian.PutUint32(c.current.code[pos:], uint32(updateStart))
	}
	if n.Children[2] != nil {
		c.compileExpr(n.Children[2])
		c.emit(bytecode.OP_POP)
	}
	c.emitOpI4(bytecode.OP_JUMP, int32(loopStart))

	if endJump >= 0 {
		c.patchJump(endJump)
	}
	for _, pos := range c.current.loopBreaks {
		c.patchJumpAt(pos)
	}

	c.emitEndScope()
	c.current.loopBreaks = oldBreaks
	c.current.loopContinues = oldContinues
}

func (c *Compiler) compileForIn(n *ASTNode) {
	// n.StrVal = variable name, children[0] = iterable, children[1] = body
	c.emitScope()
	varNameIdx := c.constString(n.StrVal)

	// Evaluate iterable, begin iteration
	c.compileExpr(n.Children[0])
	c.emit(bytecode.OP_ITER_BEGIN)

	loopStart := c.currentPos()
	endJumpPos := c.emitJump(bytecode.OP_ITER_NEXT)

	// Store current element in loop variable
	c.emit(bytecode.OP_ITER_VALUE)
	c.emitOp4(bytecode.OP_STORE_NEW, varNameIdx)

	oldBreaks := c.current.loopBreaks
	oldContinues := c.current.loopContinues
	c.current.loopBreaks = nil
	c.current.loopContinues = nil
	c.current.loopStart = loopStart

	c.compileStmt(n.Children[1])

	for _, pos := range c.current.loopContinues {
		binary.LittleEndian.PutUint32(c.current.code[pos:], uint32(loopStart))
	}
	c.emitOpI4(bytecode.OP_JUMP, int32(loopStart))
	c.patchJump(endJumpPos)
	c.emit(bytecode.OP_POP) // pop iterator

	for _, pos := range c.current.loopBreaks {
		c.patchJumpAt(pos)
	}

	c.emitEndScope()
	c.current.loopBreaks = oldBreaks
	c.current.loopContinues = oldContinues
}

func (c *Compiler) compileSwitch(n *ASTNode) {
	// n.Children[0] = discriminant, rest = case arms + optional default
	c.compileExpr(n.Children[0])
	// store discriminant in a temp
	tempIdx := c.constString("__switch__")
	c.emitOp4(bytecode.OP_STORE_NEW, tempIdx)

	var endJumps []int
	for _, arm := range n.Children[1:] {
		if arm.StrVal == "default" {
			// default block
			c.emitScope()
			c.compileStmts(arm.Children)
			c.emitEndScope()
			endJumps = append(endJumps, c.emitJump(bytecode.OP_JUMP))
		} else if arm.Kind == NodeIf && arm.StrVal == "case" {
			// case comparison
			c.emitOp4(bytecode.OP_LOAD, tempIdx)
			c.compileExpr(arm.Children[0])
			c.emit(bytecode.OP_EQ)
			skipJump := c.emitJump(bytecode.OP_JUMP_FALSE)
			c.emitScope()
			c.compileStmt(arm.Children[1])
			c.emitEndScope()
			endJumps = append(endJumps, c.emitJump(bytecode.OP_JUMP))
			c.patchJump(skipJump)
		}
	}
	for _, pos := range endJumps {
		c.patchJumpAt(pos)
	}
}

func (c *Compiler) compilePrint(n *ASTNode) {
	for _, arg := range n.Children {
		c.compileExpr(arg)
	}
	c.emitOp1(bytecode.OP_PRINT, uint8(len(n.Children)))
}

func (c *Compiler) compileIncDec(n *ASTNode, op bytecode.Opcode) {
	if len(n.Children) == 0 {
		return
	}
	target := n.Children[0]
	switch target.Kind {
	case NodeIdent:
		c.emitOp4(op, c.constString(target.StrVal))
	case NodePropertyGet:
		// obj.prop++ → load obj, dup, prop_get, ±1, prop_set
		// Stack trace:
		//   compileExpr(obj)          → [obj]
		//   DUP                       → [obj, obj]
		//   PROP_GET name             → [obj, old_val]   (pops obj copy)
		//   pushNumber(1)             → [obj, old_val, 1]
		//   ADD/SUB                   → [obj, new_val]
		//   PROP_SET name             → []  (pops new_val then obj)
		c.compileExpr(target.Children[0]) // push obj
		c.emit(bytecode.OP_DUP)           // dup obj for PROP_SET later
		c.emitOp4(bytecode.OP_PROP_GET, c.constString(target.StrVal))
		c.pushNumber(1)
		if op == bytecode.OP_INC {
			c.emit(bytecode.OP_ADD)
		} else {
			c.emit(bytecode.OP_SUB)
		}
		// Stack is now [obj, new_val] — PROP_SET pops val then obj, correct order
		c.emitOp4(bytecode.OP_PROP_SET, c.constString(target.StrVal))
	case NodeIndex:
		// arr[i]++ → load arr, load idx, dup arr, dup idx, index_get, ±1, rotate, index_set
		c.compileExpr(target.Children[0]) // array
		c.compileExpr(target.Children[1]) // index
		c.emit(bytecode.OP_DUP)           // dup index
		// Load the current value via a temp: compile arr[i] from scratch
		c.compileExpr(target.Children[0])
		c.compileExpr(target.Children[1])
		c.emit(bytecode.OP_INDEX_GET)
		c.pushNumber(1)
		if op == bytecode.OP_INC {
			c.emit(bytecode.OP_ADD)
		} else {
			c.emit(bytecode.OP_SUB)
		}
		// stack: [arr, idx, idx, newval] - need [arr, idx, newval]
		// Swap top two to get [arr, idx, newval]
		c.emit(bytecode.OP_SWAP)
		c.emit(bytecode.OP_POP) // discard extra idx copy
		c.emit(bytecode.OP_INDEX_SET)
	}
}

func (c *Compiler) compileAssert(n *ASTNode) {
	if len(n.Children) > 0 {
		c.compileExpr(n.Children[0])
	} else {
		c.emit(bytecode.OP_PUSH_TRUE)
	}
	if len(n.Children) > 1 {
		c.compileExpr(n.Children[1])
	} else {
		c.pushString("assertion failed")
	}
	c.emit(bytecode.OP_ASSERT)
}

func (c *Compiler) patchJumpAt(pos int) {
	target := int32(c.currentPos())
	binary.LittleEndian.PutUint32(c.current.code[pos:], uint32(target))
}

// ─── Expression compilation ───────────────────────────────────────────────────

func (c *Compiler) compileExpr(n *ASTNode) {
	if n == nil {
		c.emit(bytecode.OP_PUSH_NULL)
		return
	}
	switch n.Kind {
	case NodeNumber:
		c.pushNumber(n.NumVal)
	case NodeString:
		c.pushString(n.StrVal)
	case NodeBool:
		if n.BoolVal {
			c.emit(bytecode.OP_PUSH_TRUE)
		} else {
			c.emit(bytecode.OP_PUSH_FALSE)
		}
	case NodeNull:
		c.emit(bytecode.OP_PUSH_NULL)
	case NodeIdent:
		c.emitOp4(bytecode.OP_LOAD, c.constString(n.StrVal))
	case NodeBinary:
		c.compileBinary(n)
	case NodeUnary:
		c.compileUnary(n)
	case NodeTernary:
		c.compileTernary(n)
	case NodeNullish:
		c.compileNullish(n)
	case NodeFnCall:
		c.compileFnCall(n)
	case NodeMethodCall:
		c.compileMethodCall(n)
	case NodeCallExpr:
		c.compileCallExpr(n)
	case NodePropertyGet:
		c.compileExpr(n.Children[0])
		c.emitOp4(bytecode.OP_PROP_GET, c.constString(n.StrVal))
	case NodePropertySet:
		// Should be handled in assign path
		c.compileExpr(n.Children[0])
		c.compileExpr(n.Children[1])
		c.emitOp4(bytecode.OP_PROP_SET, c.constString(n.StrVal))
	case NodeIndex:
		c.compileExpr(n.Children[0])
		c.compileExpr(n.Children[1])
		c.emit(bytecode.OP_INDEX_GET)
	case NodeThis:
		c.emit(bytecode.OP_THIS_LOAD)
	case NodeSuper:
		// super(args)
		for _, arg := range n.Children {
			c.compileExpr(arg)
		}
		c.emitOp1(bytecode.OP_SUPER_CALL, uint8(len(n.Children)))
	case NodeNew:
		c.compileNew(n)
	case NodeTypeof:
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_TYPEOF)
	case NodeInstanceof:
		c.compileExpr(n.Children[0])
		className := ""
		if n.Children[1] != nil {
			className = n.Children[1].StrVal
		}
		c.emitOp4(bytecode.OP_INSTANCEOF, c.constString(className))
	case NodeArrayLiteral:
		c.compileArrayLiteral(n)
	case NodeObjectLiteral:
		c.compileObjectLiteral(n)
	case NodeArrowFn:
		fnIdx := c.compileFn(n)
		c.emitOp4(bytecode.OP_MAKE_CLOSURE, fnIdx)
	case NodeFnDecl:
		fnIdx := c.compileFn(n)
		c.emitOp4(bytecode.OP_MAKE_CLOSURE, fnIdx)
	case NodeAssign:
		c.compileAssignExpr(n)
	case NodeCompoundAssign:
		c.compileCompoundAssign(n)
	case NodeIncrement:
		c.compileIncDec(n, bytecode.OP_INC)
		if len(n.Children) > 0 {
			c.emitOp4(bytecode.OP_LOAD, c.constString(n.Children[0].StrVal))
		}
	case NodeDecrement:
		c.compileIncDec(n, bytecode.OP_DEC)
		if len(n.Children) > 0 {
			c.emitOp4(bytecode.OP_LOAD, c.constString(n.Children[0].StrVal))
		}
	case NodeInput:
		c.compileInput(n)
	case NodeSleep:
		c.compileSleep(n)
	case NodeMatch:
		c.compileMatch(n)
	case NodeSpread:
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_SPREAD)
	case NodeAwait:
		if len(n.Children) > 0 {
			c.compileExpr(n.Children[0])
		}
	case NodeTupleLiteral:
		// Compile as array
		for _, child := range n.Children {
			c.compileExpr(child)
		}
		c.emitOp4(bytecode.OP_MAKE_ARRAY, uint32(len(n.Children)))
	case NodeNamespaceAccess:
		c.emitOp4(bytecode.OP_LOAD, c.constString(n.StrVal))
		if len(n.Children) > 0 {
			c.emitOp4(bytecode.OP_PROP_GET, c.constString(n.Children[0].StrVal))
		}
	case NodeExprStmt:
		if len(n.Children) > 0 {
			c.compileExpr(n.Children[0])
		}
	default:
		// Unhandled: push null
		c.emit(bytecode.OP_PUSH_NULL)
	}
}

func (c *Compiler) compileBinary(n *ASTNode) {
	if len(n.Children) < 2 {
		return
	}
	// Short-circuit for && and ||
	switch n.StrVal {
	case "&&":
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_DUP)
		endJump := c.emitJump(bytecode.OP_JUMP_FALSE)
		c.emit(bytecode.OP_POP)
		c.compileExpr(n.Children[1])
		c.patchJump(endJump)
		return
	case "||":
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_DUP)
		endJump := c.emitJump(bytecode.OP_JUMP_TRUE)
		c.emit(bytecode.OP_POP)
		c.compileExpr(n.Children[1])
		c.patchJump(endJump)
		return
	}

	c.compileExpr(n.Children[0])
	c.compileExpr(n.Children[1])

	switch n.StrVal {
	case "+":
		c.emit(bytecode.OP_ADD)
	case "-":
		c.emit(bytecode.OP_SUB)
	case "*":
		c.emit(bytecode.OP_MUL)
	case "/":
		c.emit(bytecode.OP_DIV)
	case "%":
		c.emit(bytecode.OP_MOD)
	case "**":
		c.emit(bytecode.OP_POW)
	case "==":
		c.emit(bytecode.OP_EQ)
	case "!=":
		c.emit(bytecode.OP_NEQ)
	case "<":
		c.emit(bytecode.OP_LT)
	case "<=":
		c.emit(bytecode.OP_LE)
	case ">":
		c.emit(bytecode.OP_GT)
	case ">=":
		c.emit(bytecode.OP_GE)
	case "&":
		c.emit(bytecode.OP_BIT_AND)
	case "|":
		c.emit(bytecode.OP_BIT_OR)
	case "^":
		c.emit(bytecode.OP_BIT_XOR)
	case "<<":
		c.emit(bytecode.OP_SHL)
	case ">>":
		c.emit(bytecode.OP_SHR)
	default:
		c.errors = append(c.errors, fmt.Sprintf("unknown binary op %q", n.StrVal))
	}
}

func (c *Compiler) compileUnary(n *ASTNode) {
	switch n.StrVal {
	case "-":
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_NEG)
	case "!":
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_NOT)
	case "~":
		c.compileExpr(n.Children[0])
		c.emit(bytecode.OP_BIT_NOT)
	case "++pre", "--pre":
		inner := n.Children[0]
		nameIdx := c.constString(inner.StrVal)
		if n.StrVal == "++pre" {
			c.emitOp4(bytecode.OP_INC, nameIdx)
		} else {
			c.emitOp4(bytecode.OP_DEC, nameIdx)
		}
		c.emitOp4(bytecode.OP_LOAD, nameIdx)
	}
}

func (c *Compiler) compileTernary(n *ASTNode) {
	// [cond, then, else]
	c.compileExpr(n.Children[0])
	elseJump := c.emitJump(bytecode.OP_JUMP_FALSE)
	c.compileExpr(n.Children[1])
	endJump := c.emitJump(bytecode.OP_JUMP)
	c.patchJump(elseJump)
	c.compileExpr(n.Children[2])
	c.patchJump(endJump)
}

func (c *Compiler) compileNullish(n *ASTNode) {
	// left ?? right
	// Desired: if left is null → right, else → left (single value on stack)
	c.compileExpr(n.Children[0]) // [left]
	c.emit(bytecode.OP_DUP)      // [left, left]
	// JUMP_NULL peeks top; if null, jumps to isNull label
	isNullJump := c.emitJump(bytecode.OP_JUMP_NULL)
	// NOT NULL path: dup copy stays, discard it, keep original
	c.emit(bytecode.OP_POP)            // [left]
	endJump := c.emitJump(bytecode.OP_JUMP) // jump past right expr
	// NULL path:
	c.patchJump(isNullJump)       // [null, null_dup] - dup is still on top (JUMP_NULL peeks)
	c.emit(bytecode.OP_POP)       // [null]
	c.emit(bytecode.OP_POP)       // []
	c.compileExpr(n.Children[1])  // [right]
	c.patchJump(endJump)
}

func (c *Compiler) compileFnCall(n *ASTNode) {
	// Push arguments, push function, call
	for _, arg := range n.Children {
		c.compileExpr(arg)
	}
	c.emitOp4(bytecode.OP_LOAD, c.constString(n.StrVal))
	c.emitOp1(bytecode.OP_CALL, uint8(len(n.Children)))
}

func (c *Compiler) compileMethodCall(n *ASTNode) {
	// n.Children[0] = receiver, n.Children[1..] = args, n.StrVal = method name
	c.compileExpr(n.Children[0]) // receiver
	for _, arg := range n.Children[1:] {
		c.compileExpr(arg)
	}
	c.emitOp4U1(bytecode.OP_CALL_METHOD,
		c.constString(n.StrVal),
		uint8(len(n.Children)-1))
}

func (c *Compiler) compileCallExpr(n *ASTNode) {
	// n.Children[0] = callee expr, n.Children[1..] = args
	for _, arg := range n.Children[1:] {
		c.compileExpr(arg)
	}
	c.compileExpr(n.Children[0])
	c.emitOp1(bytecode.OP_CALL, uint8(len(n.Children)-1))
}

func (c *Compiler) compileNew(n *ASTNode) {
	for _, arg := range n.Children {
		c.compileExpr(arg)
	}
	c.emitOp4U1(bytecode.OP_NEW,
		c.constString(n.StrVal),
		uint8(len(n.Children)))
}

func (c *Compiler) compileArrayLiteral(n *ASTNode) {
	for _, elem := range n.Children {
		c.compileExpr(elem)
	}
	c.emitOp4(bytecode.OP_MAKE_ARRAY, uint32(len(n.Children)))
}

func (c *Compiler) compileObjectLiteral(n *ASTNode) {
	for _, pair := range n.Children {
		c.pushString(pair.StrVal)
		c.compileExpr(pair.Children[0])
	}
	c.emitOp4(bytecode.OP_MAKE_OBJ, uint32(len(n.Children)))
}

func (c *Compiler) compileAssignExpr(n *ASTNode) {
	if len(n.Children) < 2 {
		return
	}
	lhs := n.Children[0]
	rhs := n.Children[1]
	switch lhs.Kind {
	case NodeIdent:
		c.compileExpr(rhs)
		c.emit(bytecode.OP_DUP)
		c.emitOp4(bytecode.OP_STORE, c.constString(lhs.StrVal))
	case NodePropertyGet:
		c.compileExpr(lhs.Children[0])
		c.compileExpr(rhs)
		c.emit(bytecode.OP_DUP)
		c.emitOp4(bytecode.OP_PROP_SET, c.constString(lhs.StrVal))
	case NodeIndex:
		c.compileExpr(lhs.Children[0])
		c.compileExpr(lhs.Children[1])
		c.compileExpr(rhs)
		c.emit(bytecode.OP_DUP)
		c.emit(bytecode.OP_INDEX_SET)
	}
}

func (c *Compiler) compileInput(n *ASTNode) {
	if len(n.Children) > 0 {
		c.compileExpr(n.Children[0])
		c.emitOp1(bytecode.OP_INPUT, 1)
	} else {
		c.emitOp1(bytecode.OP_INPUT, 0)
	}
}

func (c *Compiler) compileSleep(n *ASTNode) {
	if len(n.Children) > 0 {
		c.compileExpr(n.Children[0])
	} else {
		c.pushNumber(0)
	}
	c.emit(bytecode.OP_SLEEP)
}

func (c *Compiler) compileMatch(n *ASTNode) {
	// n.Children[0] = discriminant, rest = arms
	// Store discriminant in a temp variable
	c.compileExpr(n.Children[0])
	tempIdx := c.constString("__match__")
	c.emitOp4(bytecode.OP_STORE_NEW, tempIdx)

	var endJumps []int

	for _, arm := range n.Children[1:] {
		if arm.Kind != NodeMatchArm || len(arm.Children) < 2 {
			continue
		}
		pat := arm.Children[0]
		expr := arm.Children[1]

		// ─ Wildcard _ — always matches, no check needed ─────────────────────
		if pat.StrVal == "_" {
			c.compileExpr(expr)
			endJumps = append(endJumps, c.emitJump(bytecode.OP_JUMP))
			continue
		}

		// ─ Literal pattern (number/string/bool stored in pat.Children[0]) ───
		if pat.StrVal == "" && len(pat.Children) > 0 {
			// Load discriminant, push literal, EQ, JUMP_FALSE skip
			c.emitOp4(bytecode.OP_LOAD, tempIdx)
			c.compileExpr(pat.Children[0])
			c.emit(bytecode.OP_EQ)
			skipJump := c.emitJump(bytecode.OP_JUMP_FALSE)
			c.compileExpr(expr)
			endJumps = append(endJumps, c.emitJump(bytecode.OP_JUMP))
			c.patchJump(skipJump)
			continue
		}

		// ─ Variant pattern — no bindings e.g. None, Red ─────────────────────
		if len(pat.Children) == 0 && pat.StrVal != "" {
			// LOAD temp; MATCH_TAG (pops, pushes bool); JUMP_FALSE skip
			c.emitOp4(bytecode.OP_LOAD, tempIdx)
			c.emitOp4(bytecode.OP_MATCH_TAG, c.constString(pat.StrVal))
			skipJump := c.emitJump(bytecode.OP_JUMP_FALSE)
			c.compileExpr(expr)
			endJumps = append(endJumps, c.emitJump(bytecode.OP_JUMP))
			c.patchJump(skipJump)
			continue
		}

		// ─ Variant pattern — with bindings e.g. Some(val), Circle(r) ────────
		if len(pat.Children) > 0 && pat.StrVal != "" {
			// LOAD temp; MATCH_TAG (pops, pushes bool); JUMP_FALSE skip
			c.emitOp4(bytecode.OP_LOAD, tempIdx)
			c.emitOp4(bytecode.OP_MATCH_TAG, c.constString(pat.StrVal))
			skipJump := c.emitJump(bytecode.OP_JUMP_FALSE)
			// Matched — reload and extract fields
			c.emitOp4(bytecode.OP_LOAD, tempIdx)
			for i, bind := range pat.Children {
				c.emit(bytecode.OP_DUP)
				c.emitOp1(bytecode.OP_EXTRACT_FIELD, uint8(i))
				c.emitOp4(bytecode.OP_STORE_NEW, c.constString(bind.StrVal))
			}
			c.emit(bytecode.OP_POP) // pop the variant used for extractions
			c.compileExpr(expr)
			endJumps = append(endJumps, c.emitJump(bytecode.OP_JUMP))
			c.patchJump(skipJump)
			continue
		}
	}

	c.emit(bytecode.OP_PUSH_NULL) // default if no arm matched
	for _, pos := range endJumps {
		c.patchJumpAt(pos)
	}
}
