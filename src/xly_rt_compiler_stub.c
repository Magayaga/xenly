/*
 * xly_rt_compiler_stub.c
 *
 * Stub implementations for symbols that xly_rt references but are only
 * provided by the interpreter (modules.c / interpreter.c).  These stubs
 * allow compiled Xenly programs to link without dragging in the interpreter.
 */
#include <stddef.h>

/* modules_get is referenced by xly_rt.c's import support.
 * Compiled programs do not support dynamic module import at runtime;
 * any import statement is resolved at compile time.  Return 0 (not found). */
int modules_get(const char *name, void *out) {
    (void)name;
    (void)out;
    return 0;
}
