/*
 * xly_rt_compiler_stub.c
 *
 * modules_get() for compiled Xenly programs (linked via libxly_rtc.a).
 *
 * FIX: this previously always returned "not found" for every module, on
 * the assumption that "compiled programs do not support dynamic module
 * import at runtime; any import statement is resolved at compile time."
 * That assumption was only true for sys.* CONSTANTS, which src/codegen.c
 * inlines directly. Every sys.* FUNCTION (getpid, open, read, mmap,
 * sockets, clocks, uname, bit ops, syslog, ...) fell through to this stub
 * and silently returned null while printing "[xenly] unknown module 'sys'"
 * to stderr, for every single call.
 *
 * This now provides a real sys module by including the same sys_module.inc
 * fragment src/modules.c uses, compiled here against XlyVal instead of the
 * interpreter's Value (see sys_module.inc's header comment for why that's
 * ABI-safe). reflect and http are intentionally NOT included here: they
 * need interpreter-only glue (Environment for reflect, HTTP server plumbing
 * for http) that compiled Xenly binaries don't link against.
 */
#if !defined(__APPLE__)
#define _GNU_SOURCE
#endif
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "xly_rt.h"

/* XlyVal is opaque outside xly_rt.c (only forward-declared in xly_rt.h), but
 * sys_module.inc needs direct field access (->type, ->num, ->str, ->array,
 * ->array_len), the same way it does when compiled into modules.c against
 * the interpreter's Value. Both structs share an identical layout (see the
 * XlyVal struct comment in xly_rt.c) and an identical ValType/ValueType
 * enum, so a local struct with the same field order/types (void* standing
 * in for the pointer fields sys code never touches) accesses the exact same
 * offsets in objects that xly_num()/xly_str()/etc. actually allocate. */
typedef enum {
    VAL_NUMBER = 0, VAL_STRING = 1, VAL_BOOL = 2, VAL_NULL = 3,
    VAL_FUNCTION = 4, VAL_BUILTIN_FN = 5, VAL_RETURN = 6, VAL_BREAK = 7,
    VAL_CONTINUE = 8, VAL_CLASS = 9, VAL_INSTANCE = 10, VAL_ARRAY = 11,
    VAL_ENUM_VARIANT = 12,
} ValueType;

typedef struct Value {
    ValueType     type;
    double        num;
    char         *str;
    int           boolean;
    int           local;
    int           fn_shared;
    int           refcount;
    void         *fn;
    void         *builtin_fn;
    struct Value *inner;
    void         *class_def;
    void         *instance;
    struct Value **array;
    size_t        array_len;
    size_t        array_cap;
    struct {
        char           *tag;
        struct Value  **fields;
        size_t          field_count;
    } variant;
} Value;

typedef Value *(*NativeFn)(Value **args, size_t argc);
typedef struct { char *name; NativeFn fn; } NativeFunc;
typedef struct { char *name; NativeFunc *functions; size_t fn_count; } Module;

/* value_number/_string/_bool/_null/_array aren't declared in xly_rt.h (only
 * defined in xly_rt.c), so declare them here against our local Value* so
 * sys_module.inc's calls resolve to the exact same symbols xly_rt.c
 * defines. */
Value *value_number(double n);
Value *value_string(const char *s);
Value *value_bool(int b);
Value *value_null(void);
Value *value_array(Value **items, size_t len);

/* value_to_string IS already declared in xly_rt.h (against XlyVal*); this
 * macro casts our locally-visible Value* to XlyVal* at the call site so the
 * call type-checks cleanly instead of relying on an implicit pointer
 * conversion warning (harmless here since the structs are ABI-identical,
 * but there's no reason to leave the warning in). The preprocessor does not
 * re-expand a macro within its own expansion, so this correctly calls the
 * real xly_rt.c function rather than looping. */
#define value_to_string(v) value_to_string((XlyVal *)(v))

#include "sys_module.inc"

int modules_get(const char *name, void *out) {
    if (strcmp(name, "sys") == 0) {
        *(Module *)out = module_sys();
        return 1;
    }
    return 0;
}
