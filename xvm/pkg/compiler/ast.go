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
 * Compiler: AST node definitions
 */
package compiler

// NodeKind identifies the kind of AST node.
type NodeKind int

const (
	// ── Program / Declarations ────────────────────────────────────────────
	NodeProgram     NodeKind = iota // root
	NodeVarDecl                     // var/let x = expr
	NodeConstDecl                   // const x = expr
	NodeFnDecl                      // fn name(params) { body }
	NodeClassDecl                   // class Name [extends P] { ... }
	NodeEnumDecl                    // enum Name { Variant(fields), ... }
	NodeTypeAlias                   // type Alias = Type
	NodeNamespace                   // namespace Name { ... }
	NodeExport                      // export fn/class
	NodeImport                      // import "mod" [as alias] | from "mod" import ...

	// ── Statements ────────────────────────────────────────────────────────
	NodeBlock        // { stmts }
	NodeExprStmt     // expression used as statement
	NodeAssign       // x = expr
	NodeCompoundAssign // x += expr etc.
	NodeReturn       // return [expr]
	NodeIf           // if (cond) body [else body]
	NodeWhile        // while (cond) body
	NodeDoWhile      // do body while (cond)
	NodeFor          // for (init; cond; update) body
	NodeForIn        // for (var x in expr) body
	NodeSwitch       // switch (expr) { case val: stmts... }
	NodeBreak        // break
	NodeContinue     // continue
	NodeIncrement    // x++
	NodeDecrement    // x--
	NodePrint        // print(args...)
	NodeInput        // input([prompt])
	NodeSleep        // sleep(ms)
	NodeAssert       // assert(cond, msg)
	NodeRequires     // requires (cond)
	NodeEnsures      // ensures (cond)
	NodeInvariant    // invariant (cond)
	NodeSpawn        // spawn expr
	NodeAwait        // await expr

	// ── Expressions ───────────────────────────────────────────────────────
	NodeBinary        // left op right
	NodeUnary         // op right
	NodeTernary       // cond ? t : f
	NodeNullish       // a ?? b
	NodeNumber        // 42
	NodeString        // "hello"
	NodeBool          // true/false
	NodeNull          // null
	NodeIdent         // varName
	NodeFnCall        // fn(args)
	NodeMethodCall    // obj.method(args)
	NodeCallExpr      // expr(args)  — call any expression
	NodePropertyGet   // obj.prop
	NodePropertySet   // obj.prop = val
	NodeIndex         // arr[idx]
	NodeIndexAssign   // arr[idx] = val
	NodeThis          // this
	NodeSuper         // super(args)
	NodeNew           // new ClassName(args)
	NodeTypeof        // typeof expr
	NodeInstanceof    // expr instanceof Class
	NodeArrayLiteral  // [items...]
	NodeObjectLiteral // {key: val, ...}
	NodeArrowFn       // (params) => expr
	NodeEnumVariant   // Tag(fields)
	NodeMatch         // match expr { arms... }
	NodeMatchArm      // Pattern => expr
	NodePattern       // match pattern
	NodeNamedArg      // name: value (named argument)
	NodeTupleLiteral  // (a, b, c)
	NodeNamespaceAccess // Ns.member
	NodeSpread        // ...expr
)

// ASTNode is a node in the abstract syntax tree.
type ASTNode struct {
	Kind     NodeKind
	Line     int
	// Shared value fields
	StrVal   string  // identifier, string literal, operator symbol
	NumVal   float64 // numeric literal
	BoolVal  bool    // bool literal

	// Children
	Children []*ASTNode

	// Function / arrow function parameters
	Params []*FnParam

	// Type info (for gradual typing)
	TypeAnnotation string
	ReturnType     string

	// Extra flags
	IsAsync  bool // async fn
	IsExport bool // export declaration
}

// FnParam represents a function parameter.
type FnParam struct {
	Name         string
	TypeAnnot    string
	DefaultValue *ASTNode // may be nil
	IsRest       bool     // ...rest param
	IsOptional   bool
}

// Child helpers

func (n *ASTNode) AddChild(child *ASTNode) {
	n.Children = append(n.Children, child)
}

func NewNode(kind NodeKind, line int) *ASTNode {
	return &ASTNode{Kind: kind, Line: line}
}

func NumberNode(val float64, line int) *ASTNode {
	n := NewNode(NodeNumber, line)
	n.NumVal = val
	return n
}

func StringNode(val string, line int) *ASTNode {
	n := NewNode(NodeString, line)
	n.StrVal = val
	return n
}

func BoolNode(val bool, line int) *ASTNode {
	n := NewNode(NodeBool, line)
	n.BoolVal = val
	return n
}

func IdentNode(name string, line int) *ASTNode {
	n := NewNode(NodeIdent, line)
	n.StrVal = name
	return n
}
