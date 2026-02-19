#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <stddef.h>

// ─── Value Types ─────────────────────────────────────────────────────────────
typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_NULL,
    VAL_FUNCTION,   // user-defined fn
    VAL_RETURN,     // sentinel: wraps a return value
    VAL_BREAK,      // sentinel: signals break
    VAL_CONTINUE,   // sentinel: signals continue
    VAL_CLASS,      // a class definition
    VAL_INSTANCE,   // an instantiated object
    VAL_ARRAY,      // a dynamic array of Values
    VAL_ENUM_VARIANT, // an ADT variant instance (tagged union)
} ValueType;

typedef struct Value Value;
typedef struct Environment Environment;

// ─── Function definition (stored inside a Value) ────────────────────────────
typedef struct {
    char    *name;
    Param   *params;
    size_t   param_count;
    ASTNode *body;          // NODE_BLOCK
    Environment *closure;   // captured env at definition time
    int      is_async;      // 1 = async function (can be spawned)
} FnDef;

// ─── Class definition (stored inside a VAL_CLASS value) ─────────────────────
typedef struct ClassDef {
    char            *name;          // class name
    struct ClassDef *parent;        // parent ClassDef, or NULL
    // Methods are stored as a linked list of EnvEntry-style pairs
    // but we reuse a simple Environment as a method table.
    // Each entry maps method_name → VAL_FUNCTION value.
    Environment     *methods;       // method table (does NOT own parent chain)
} ClassDef;

// ─── Instance data (stored inside a VAL_INSTANCE value) ──────────────────────
typedef struct {
    ClassDef    *class_def;         // which class this is an instance of
    Environment *fields;            // instance fields (properties), flat — no parent
} InstanceData;

// ─── Value ───────────────────────────────────────────────────────────────────
struct Value {
    ValueType     type;
    double        num;
    char         *str;              // owned string (for VAL_STRING)
    int           boolean;
    int           local;            // 1 = local wrapper, freed by env_destroy (e.g. __super__)
    int           fn_shared;        // 1 = FnDef is a shared reference (don't free on destroy)
    FnDef        *fn;               // for VAL_FUNCTION
    Value        *inner;            // for VAL_RETURN: wraps the actual return value
    ClassDef     *class_def;        // for VAL_CLASS
    InstanceData *instance;         // for VAL_INSTANCE
    Value       **array;            // for VAL_ARRAY: owned array of Value*
    size_t        array_len;        // for VAL_ARRAY: number of elements
    size_t        array_cap;        // for VAL_ARRAY: allocated capacity
    
    // ADT variant data
    struct {
        char     *tag;              // variant name (e.g. "Some", "None")
        Value   **fields;           // variant field values
        size_t    field_count;      // number of fields
    } variant;                      // for VAL_ENUM_VARIANT
};

// ─── Environment (scope chain) ───────────────────────────────────────────────
typedef struct EnvEntry {
    char  *name;
    Value *value;
    int    is_const;        // 1 if this is a const binding (immutable)
    struct EnvEntry *next;
} EnvEntry;

struct Environment {
    EnvEntry    *entries;
    Environment *parent;    // enclosing scope (NULL for global)
    int          refcount;  // reference count: closures retain, env_destroy releases
};

// ─── Native (built-in) function pointer ──────────────────────────────────────
typedef Value* (*NativeFn)(Value **args, size_t argc);

typedef struct {
    char      *name;
    NativeFn   fn;
} NativeFunc;

// ─── Module ──────────────────────────────────────────────────────────────────
typedef struct {
    char        *name;          // module identifier (e.g. "math")
    NativeFunc  *functions;     // array of native functions
    size_t       fn_count;
} Module;

// ─── User-defined Module (loaded from .xe files) ────────────────────────────
typedef struct {
    char        *name;          // module identifier (basename without .xe)
    char        *filepath;      // resolved absolute/relative path for dedup
    Environment *exports;       // exported names → Values (flat env, no parent)
    ASTNode     *ast;           // parsed AST — must outlive the module (FnDef bodies point here)
} UserModule;

// ─── Async Task (for spawn) ──────────────────────────────────────────────────
typedef struct Task {
    FnDef       *fn;            // async function to execute
    Value      **args;          // arguments (owned by task)
    size_t       argc;
    Environment *call_env;      // environment for this call
    int          completed;     // 1 when done
    Value       *result;        // return value (NULL until completed)
    struct Task *next;          // linked list
} Task;

// ─── Interpreter State ───────────────────────────────────────────────────────
typedef struct {
    Environment *global;
    Module      *modules;       // loaded native modules
    size_t       module_count;
    UserModule  *user_modules;  // loaded user .xe modules
    size_t       user_module_count;
    char        *source_dir;    // directory of the main file (for relative imports)
    char       **loading_files; // stack of files currently being loaded (circular-import guard)
    size_t       loading_count;
    int          had_error;

    // Transient state during module eval: collects exported names
    Environment *current_exports;   // non-NULL only while evaluating a module file

    // Async runtime: task queue for spawn
    Task        *task_queue;        // linked list of spawned tasks
    Task        *task_queue_tail;

    // Array registry: all allocated arrays, freed at shutdown
    Value      **all_arrays;        // flat array of every VAL_ARRAY ever created
    size_t       all_arrays_count;
    size_t       all_arrays_cap;
} Interpreter;

// ─── API ─────────────────────────────────────────────────────────────────────
Interpreter *interpreter_create(void);
void         interpreter_destroy(Interpreter *interp);
void         interpreter_set_source_dir(Interpreter *interp, const char *dir);
Value       *interpreter_run(Interpreter *interp, ASTNode *program);

// Value helpers
Value *value_number(double n);
Value *value_string(const char *s);
Value *value_bool(int b);
Value *value_null(void);
Value *value_array(Value **items, size_t len);   // takes ownership of items array and each item
Value *value_variant(const char *tag, Value **fields, size_t field_count);  // creates ADT variant
void   value_destroy(Value *v);
char  *value_to_string(Value *v);   // returns a newly allocated string

// Environment
Environment *env_create(Environment *parent);
void         env_retain(Environment *env);   // increment refcount (for closures)
void         env_destroy(Environment *env);  // decrement refcount; free when 0
void         env_set(Environment *env, const char *name, Value *val);
void         env_set_const(Environment *env, const char *name, Value *val);  // set immutable binding
Value       *env_get(Environment *env, const char *name);

#endif // INTERPRETER_H
