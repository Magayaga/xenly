#ifndef AST_H
#define AST_H

#include <stddef.h>

// ─── Node Types ──────────────────────────────────────────────────────────────
typedef enum {
    NODE_PROGRAM,           // Root: list of statements
    NODE_VAR_DECL,          // var x = expr
    NODE_ASSIGN,            // x = expr
    NODE_COMPOUND_ASSIGN,   // x += expr, x -= expr, etc.
    NODE_INCREMENT,         // x++
    NODE_DECREMENT,         // x--
    NODE_FN_DECL,           // fn name(params) { body }
    NODE_FN_CALL,           // name(args)
    NODE_RETURN,            // return expr
    NODE_IF,                // if (cond) { body } else { body }
    NODE_WHILE,             // while (cond) { body }
    NODE_FOR,               // for (init; cond; update) { body }
    NODE_FOR_IN,            // for (var x in arr) { body }
    NODE_BREAK,             // break
    NODE_CONTINUE,          // continue
    NODE_DO_WHILE,          // do { body } while (cond)
    NODE_SWITCH,            // switch (expr) { case val: stmts... }
    NODE_TERNARY,           // cond ? true_expr : false_expr
    NODE_PRINT,             // print(expr, expr, ...)
    NODE_INPUT,             // input("prompt")
    NODE_IMPORT,            // import "module" [as alias] | from "module" import name1, name2
    NODE_BLOCK,             // { stmts }
    NODE_BINARY,            // left op right
    NODE_UNARY,             // op right  (negation, not)
    NODE_NUMBER,            // 42 / 3.14
    NODE_STRING,            // "hello"
    NODE_BOOL,              // true / false
    NODE_NULL,              // null
    NODE_IDENTIFIER,        // variable name
    NODE_METHOD_CALL,       // obj.method(args)  — for module functions AND instance methods
    NODE_EXPR_STMT,         // expression used as statement

    // OOP
    NODE_CLASS_DECL,        // class Name extends Parent { methods... }
    NODE_NEW,               // new ClassName(args)
    NODE_THIS,              // this
    NODE_SUPER_CALL,        // super(args)  — calls parent init
    NODE_PROPERTY_GET,      // obj.prop     — read a property
    NODE_PROPERTY_SET,      // obj.prop = val — write a property

    // Generics / reflection
    NODE_TYPEOF,            // typeof(expr)  — returns type name as string
    NODE_INSTANCEOF,        // expr instanceof ClassName — bool check
    NODE_CALL_EXPR,         // expr(args)  — call any expression that evaluates to a function

    // Modular programming
    NODE_EXPORT,            // export fn/class  — marks a declaration as exported from a module

    // Concurrency
    NODE_SPAWN,             // spawn expr  — launches async function in background
    NODE_AWAIT,             // await expr  — calls async function and waits for result

    // Functional programming
    NODE_ARRAY_LITERAL,     // [expr, expr, ...]  — array literal
    NODE_ARROW_FN,          // (params) => expr  — arrow function
    NODE_NULLISH,           // expr ?? default  — null coalescing
    NODE_INDEX,             // arr[index]  — array/object indexing
    NODE_CONST_DECL,        // const x = expr  — immutable binding
    NODE_ENUM_DECL,         // enum Option { Some(value), None }  — algebraic data type
    NODE_ENUM_VARIANT,      // Some(value)  — enum variant construction
    NODE_MATCH,             // match expr { pattern => expr, ... }  — pattern matching
    NODE_MATCH_ARM,         // pattern => expr  — single match arm
    NODE_PATTERN,           // Pattern for matching (variant, literal, identifier)
    NODE_NAMED_ARG,         // name: value — named argument in function call
    NODE_OBJECT_LITERAL,    // {key: value, ...} — anonymous object/record type
    NODE_OPERATOR_OVERLOAD, // Special operator method (__add__, __mul__, etc.)
    NODE_NAMESPACE,         // namespace Name { declarations }
    NODE_NAMESPACE_ACCESS,  // Namespace.member
    NODE_TUPLE_LITERAL,     // (expr, expr, ...) — tuple literal
    NODE_TYPE_SIGNATURE,    // : Type — explicit type signature
    NODE_TYPE_ALIAS,        // type Alias = Type — type alias declaration
    
    // Design by Contract
    NODE_REQUIRES,          // requires (condition) — precondition
    NODE_ENSURES,           // ensures (condition) — postcondition
    NODE_INVARIANT,         // invariant (condition) — class invariant
    NODE_ASSERT,            // assert(condition, message) — runtime assertion

    // Concurrency

    // Modular programming
} NodeType;

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct Param   Param;

// ─── Function Parameter ─────────────────────────────────────────────────────
struct Param {
    char *name;
    char *type_annotation;  // optional: "number", "string", "bool", "any", or NULL
    struct ASTNode *default_value;  // optional: default value expression
    int is_optional;        // flag: parameter can be omitted
};

// ─── AST Node ────────────────────────────────────────────────────────────────
struct ASTNode {
    NodeType type;
    int      line;

    // Shared fields (used depending on node type):
    char    *str_value;     // identifier name, string literal, operator symbol
    double   num_value;     // numeric literal
    int      bool_value;    // 1 = true, 0 = false

    // Children / sub-nodes (dynamically allocated arrays)
    ASTNode **children;
    size_t    child_count;
    size_t    child_capacity;

    // Function declarations: parameter list
    Param   *params;
    size_t   param_count;

    // Type annotations (gradual typing)
    char    *type_annotation;      // for variables: var x: number
    char    *return_type;          // for functions: fn foo(): number
    
    // Design by Contract
    ASTNode *requires_clause;      // precondition for functions
    ASTNode *ensures_clause;       // postcondition for functions
    ASTNode **invariants;          // class invariants
    size_t   invariant_count;

    // Compound assign operator type (stored as string: "+=", "-=", etc.)
    // reuses str_value
};

// ─── API ─────────────────────────────────────────────────────────────────────
ASTNode *ast_node_create(NodeType type, int line);
void     ast_node_destroy(ASTNode *node);
void     ast_node_add_child(ASTNode *parent, ASTNode *child);
void     ast_print(ASTNode *node, int indent);  // Debug pretty-print

#endif // AST_H
