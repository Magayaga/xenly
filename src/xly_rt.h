/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#ifndef XLY_RT_H
#define XLY_RT_H

/*
 * xly_rt  —  Xenly compiled-binary runtime
 *
 * Every Xenly value at runtime is an opaque pointer to an XlyVal on the heap.
 * The runtime owns all allocation / GC (for now: no GC, arena-style leak-free
 * via process exit).  Compiled code never touches the struct internals directly;
 * it calls only the functions declared here.
 *
 * Calling convention: System V AMD64.  All functions take / return XlyVal*.
 */

#include <stddef.h>   /* size_t */
#include <stdint.h>   /* int64_t */

/* ── opaque value pointer ──────────────────────────────────────────────── */
typedef struct XlyVal XlyVal;

/* ── constructors ──────────────────────────────────────────────────────── */
XlyVal *xly_num(double n);
XlyVal *xly_str(const char *s);        /* copies s */
XlyVal *xly_bool(int b);               /* 0 or 1   */
XlyVal *xly_null(void);

/* ── arithmetic / comparison  (both args must be valid XlyVal*) ─────── */
XlyVal *xly_add(XlyVal *a, XlyVal *b);   /* num+num  or  str+str  or  str+any */
XlyVal *xly_sub(XlyVal *a, XlyVal *b);
XlyVal *xly_mul(XlyVal *a, XlyVal *b);
XlyVal *xly_div(XlyVal *a, XlyVal *b);
XlyVal *xly_mod(XlyVal *a, XlyVal *b);
XlyVal *xly_neg(XlyVal *a);             /* unary minus */
XlyVal *xly_not(XlyVal *a);             /* logical not */

/* comparison → xly_bool */
XlyVal *xly_eq(XlyVal *a, XlyVal *b);
XlyVal *xly_neq(XlyVal *a, XlyVal *b);
XlyVal *xly_lt(XlyVal *a, XlyVal *b);
XlyVal *xly_gt(XlyVal *a, XlyVal *b);
XlyVal *xly_lte(XlyVal *a, XlyVal *b);
XlyVal *xly_gte(XlyVal *a, XlyVal *b);

/* ── truthiness  (C int, not XlyVal*) ──────────────────────────────── */
int     xly_truthy(XlyVal *v);

/* ── I/O ───────────────────────────────────────────────────────────── */
/* xly_print: variadic-style via an array of values + count.
 * Prints space-separated, trailing newline. */
void    xly_print(XlyVal **vals, size_t n);

/* xly_input: print prompt, read a line, return as xly_str */
XlyVal *xly_input(XlyVal *prompt);

/* ── string conversion ─────────────────────────────────────────────── */
char   *xly_to_cstr(XlyVal *v);         /* caller must free() */
XlyVal *xly_typeof(XlyVal *v);          /* returns xly_str("number"|"string"|…) */
char   *value_to_string(XlyVal *v);     /* returns C string (caller must free) */

/* ── value utilities ───────────────────────────────────────────────── */
XlyVal *value_clone(XlyVal *v);         /* deep copy */
void    value_destroy(XlyVal *v);       /* free memory */

/* ── array operations ──────────────────────────────────────────────── */
XlyVal *xly_array_create(XlyVal **elems, size_t n);  /* copies the pointer array */
size_t  xly_array_len(XlyVal *arr);
XlyVal *xly_array_get(XlyVal *arr, size_t idx);
void    xly_array_set(XlyVal *arr, size_t idx, XlyVal *val);
XlyVal *xly_array_push(XlyVal *arr, XlyVal *val);

/* ── module dispatch  (used by compiled module.fn() calls) ─────────── */
/*
 * xly_call_module(module_name, fn_name, args, argc)
 *
 * Dispatches to the same native functions that the interpreter uses.
 * module_name / fn_name are plain C string literals baked into .rodata.
 */
XlyVal *xly_call_module(const char *mod, const char *fn,
                         XlyVal **args, size_t argc);

/* ── process exit ──────────────────────────────────────────────────── */
void    xly_exit(int code);   /* calls _exit() */

#endif /* XLY_RT_H */
