/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
/*
 * xly_rt.c  —  Xenly native-compiler runtime library
 *
 * Compiled Xenly binaries call into this.  It is compiled once into
 * libxly_rt.a and linked into every xenlyc output.
 *
 * The tagged-value layout mirrors the interpreter's Value struct so that
 * modules.c (100+ stdlib/sys functions) can be linked in directly and
 * dispatched through xly_call_module().
 *
 * additions (systems programming tier):
 *   — Raw memory access: xly_ptr_read/write + typed u8/u16/u32/u64 variants
 *   — Direct Linux syscall: xly_syscall (nr, a1-a6)
 *   — String primitives: xly_str_len, xly_str_slice, xly_str_byte,
 *                        xly_str_concat, xly_str_find, xly_str_repeat
 *   — Numeric helpers:   xly_int, xly_abs, xly_floor, xly_ceil, xly_round,
 *                        xly_sqrt, xly_pow, xly_min, xly_max, xly_clamp
 *   — IEEE 754 ops:      xly_num_bits, xly_bits_num
 *   — Error I/O:         xly_write_stderr, xly_eprint, xly_perror
 *   — Array extras:      xly_array_pop, xly_array_slice, xly_array_concat,
 *                        xly_array_reverse, xly_array_contains
 */

#include "xly_rt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

/* ── internal struct layout ─────────────────────────────────────────────
 * Must match interpreter.h  Value / ValueType exactly so that modules.c
 * functions (which take Value**) work unchanged.
 *
 * Exact enum values from interpreter.h (sequential, starting at 0):
 *   VAL_NUMBER=0  VAL_STRING=1  VAL_BOOL=2    VAL_NULL=3
 *   VAL_FUNCTION=4  VAL_BUILTIN_FN=5  VAL_RETURN=6  VAL_BREAK=7
 *   VAL_CONTINUE=8  VAL_CLASS=9  VAL_INSTANCE=10  VAL_ARRAY=11
 *   VAL_ENUM_VARIANT=12
 *
 * Exact field offsets (64-bit):
 *   type=0  num=8  str=16  boolean=24  local=28  fn_shared=32  refcount=36
 *   fn=40  builtin_fn=48  inner=56  class_def=64  instance=72
 *   array=80  array_len=88  array_cap=96
 *                                                                         */

typedef enum {
    VAL_NUMBER      = 0,
    VAL_STRING      = 1,
    VAL_BOOL        = 2,
    VAL_NULL        = 3,
    VAL_FUNCTION    = 4,
    VAL_BUILTIN_FN  = 5,   /* built-in C function (interpreter only)     */
    VAL_RETURN      = 6,   /* sentinel: wraps a return value              */
    VAL_BREAK       = 7,   /* sentinel: signals break                     */
    VAL_CONTINUE    = 8,   /* sentinel: signals continue                  */
    VAL_CLASS       = 9,   /* a class definition                          */
    VAL_INSTANCE    = 10,  /* an instantiated object                      */
    VAL_ARRAY       = 11,  /* a dynamic array of Values                   */
    VAL_ENUM_VARIANT= 12,  /* an ADT variant instance                     */
} ValType;

/* Forward-declare the fields that modules.c touches.  We keep the struct
 * layout byte-for-byte identical to interpreter.h's Value.
 *
 * IMPORTANT: For compiled code, the raw function pointer (code label) is
 * stored in the `builtin_fn` slot (offset 48).  The `fn` slot (offset 40)
 * is kept NULL so modules.c does not mistake this for an interpreter FnDef.
 * `inner` (offset 56) is reused to store the closure env pointer.         */
struct XlyVal {
    ValType   type;          /* offset  0 — 4 bytes + 4 pad               */
    double    num;           /* offset  8                                  */
    char     *str;           /* offset 16                                  */
    int       boolean;       /* offset 24                                  */
    int       local;         /* offset 28                                  */
    int       fn_shared;     /* offset 32 — must exist; unused at runtime  */
    int       refcount;      /* offset 36 — must exist; unused at runtime  */
    void     *fn;            /* offset 40 — FnDef* in interp; NULL here    */
    void     *builtin_fn;    /* offset 48 — raw fn ptr for compiled fns    */
    struct XlyVal *inner;    /* offset 56 — closure env ptr (reused)       */
    void     *class_def;     /* offset 64 — unused in compiled code        */
    void     *instance;      /* offset 72 — unused in compiled code        */
    struct XlyVal **array;   /* offset 80 — VAL_ARRAY elements             */
    size_t    array_len;     /* offset 88                                  */
    size_t    array_cap;     /* offset 96                                  */
    /* variant struct at offset 104 — unused, but sizeof matches Value */
    struct {
        char            *tag;
        struct XlyVal  **fields;
        size_t           field_count;
    } variant;               /* offset 104                                 */
};

/* ══════════════════════════════════════════════════════════════════════════════
 * CONSTRUCTORS
 * ══════════════════════════════════════════════════════════════════════════════ */

XlyVal *xly_num(double n) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type = VAL_NUMBER;
    v->num  = n;
    return v;
}

XlyVal *xly_str(const char *s) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type = VAL_STRING;
    v->str  = s ? strdup(s) : strdup("");
    return v;
}

XlyVal *xly_bool(int b) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type    = VAL_BOOL;
    v->boolean = b ? 1 : 0;
    return v;
}

XlyVal *xly_null(void) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type = VAL_NULL;
    return v;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * TRUTHINESS
 * ══════════════════════════════════════════════════════════════════════════════ */

int xly_truthy(XlyVal *v) {
    if (!v) return 0;
    switch (v->type) {
        case VAL_NULL:   return 0;
        case VAL_BOOL:   return v->boolean;
        case VAL_NUMBER: return v->num != 0.0;
        case VAL_STRING: return v->str && v->str[0] != '\0';
        case VAL_ARRAY:  return v->array_len > 0;
        default:         return 1;
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 * STRING CONVERSION
 * ══════════════════════════════════════════════════════════════════════════════ */

char *xly_to_cstr(XlyVal *v) {
    if (!v) return strdup("null");
    char buf[64];
    switch (v->type) {
        case VAL_NUMBER:
            /* Integer formatting: avoid "1.0" for clean output */
            if (v->num == (double)(long long)v->num && fabs(v->num) < 1e15)
                snprintf(buf, sizeof(buf), "%lld", (long long)v->num);
            else
                snprintf(buf, sizeof(buf), "%g", v->num);
            return strdup(buf);
        case VAL_STRING:  return strdup(v->str ? v->str : "");
        case VAL_BOOL:    return strdup(v->boolean ? "true" : "false");
        case VAL_NULL:    return strdup("null");
        case VAL_ARRAY: {
            size_t cap = 128;
            char  *out = (char*)malloc(cap);
            size_t pos = 0;
            out[pos++] = '[';
            for (size_t i = 0; i < v->array_len; i++) {
                if (i) { out[pos++] = ','; out[pos++] = ' '; }
                char *es = xly_to_cstr(v->array[i]);
                size_t elen = strlen(es);
                int is_str = (v->array[i] && v->array[i]->type == VAL_STRING);
                size_t need = elen + (is_str ? 2 : 0) + 4;
                while (pos + need >= cap) { cap *= 2; out = (char*)realloc(out, cap); }
                if (is_str) { out[pos++] = '"'; memcpy(out+pos, es, elen); pos += elen; out[pos++] = '"'; }
                else        { memcpy(out+pos, es, elen); pos += elen; }
                free(es);
            }
            out[pos++] = ']';
            out[pos] = '\0';
            return out;
        }
        default: return strdup("<object>");
    }
}

XlyVal *xly_typeof(XlyVal *v) {
    if (!v) return xly_str("null");
    switch (v->type) {
        case VAL_NUMBER:   return xly_str("number");
        case VAL_STRING:   return xly_str("string");
        case VAL_BOOL:     return xly_str("bool");
        case VAL_NULL:     return xly_str("null");
        case VAL_ARRAY:    return xly_str("array");
        case VAL_FUNCTION: return xly_str("function");
        default:           return xly_str("object");
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 * ARITHMETIC
 * ══════════════════════════════════════════════════════════════════════════════ */

XlyVal *xly_add(XlyVal *a, XlyVal *b) {
    if (!a || !b) return xly_null();
    if (a->type == VAL_STRING || b->type == VAL_STRING) {
        char *sa = xly_to_cstr(a), *sb = xly_to_cstr(b);
        size_t la = strlen(sa), lb = strlen(sb);
        char *cat = (char*)malloc(la + lb + 1);
        memcpy(cat, sa, la); memcpy(cat+la, sb, lb); cat[la+lb] = '\0';
        free(sa); free(sb);
        XlyVal *r = xly_str(cat); free(cat);
        return r;
    }
    return xly_num(a->num + b->num);
}

XlyVal *xly_sub(XlyVal *a, XlyVal *b) { return xly_num(a->num - b->num); }
XlyVal *xly_mul(XlyVal *a, XlyVal *b) { return xly_num(a->num * b->num); }

XlyVal *xly_div(XlyVal *a, XlyVal *b) {
    /* Guard: division by zero returns NaN instead of crashing */
    if (!b || b->num == 0.0) return xly_num(a && a->num == 0.0 ? 0.0/0.0 : (a ? a->num / 0.0 : 0.0));
    return xly_num(a->num / b->num);
}

XlyVal *xly_mod(XlyVal *a, XlyVal *b) {
    if (!b || b->num == 0.0) return xly_num(0.0/0.0);
    return xly_num(fmod(a->num, b->num));
}

XlyVal *xly_neg(XlyVal *a)  { return xly_num(-a->num); }
XlyVal *xly_not(XlyVal *a)  { return xly_bool(!xly_truthy(a)); }

/* ══════════════════════════════════════════════════════════════════════════════
 * COMPARISON
 * ══════════════════════════════════════════════════════════════════════════════ */

static int vals_equal(XlyVal *a, XlyVal *b) {
    if (!a || !b) return (!a && !b);
    if (a->type != b->type) return 0;
    switch (a->type) {
        case VAL_NUMBER: return a->num == b->num;
        case VAL_STRING: return strcmp(a->str ? a->str : "", b->str ? b->str : "") == 0;
        case VAL_BOOL:   return a->boolean == b->boolean;
        case VAL_NULL:   return 1;
        default:         return a == b;
    }
}

XlyVal *xly_eq (XlyVal *a, XlyVal *b) { return xly_bool( vals_equal(a,b)); }
XlyVal *xly_neq(XlyVal *a, XlyVal *b) { return xly_bool(!vals_equal(a,b)); }
XlyVal *xly_lt (XlyVal *a, XlyVal *b) { return xly_bool(a->num <  b->num); }
XlyVal *xly_gt (XlyVal *a, XlyVal *b) { return xly_bool(a->num >  b->num); }
XlyVal *xly_lte(XlyVal *a, XlyVal *b) { return xly_bool(a->num <= b->num); }
XlyVal *xly_gte(XlyVal *a, XlyVal *b) { return xly_bool(a->num >= b->num); }

/* ══════════════════════════════════════════════════════════════════════════════
 * I/O
 * ══════════════════════════════════════════════════════════════════════════════ */

void xly_print(XlyVal **vals, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (i) putchar(' ');
        char *s = xly_to_cstr(vals[i]);
        fputs(s, stdout);
        free(s);
    }
    putchar('\n');
    fflush(stdout);
}

XlyVal *xly_input(XlyVal *prompt) {
    if (prompt) {
        char *ps = xly_to_cstr(prompt);
        fputs(ps, stdout);
        fflush(stdout);
        free(ps);
    }
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) return xly_str("");
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
    return xly_str(buf);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * SYSTEMS PROGRAMMING — STRING PRIMITIVES
 * ══════════════════════════════════════════════════════════════════════════════ */

/* xly_str_len(s) → number: byte length of string (O(1) via strlen) */
XlyVal *xly_str_len(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)strlen(s->str));
}

/* xly_str_slice(s, start, end) → string: bytes [start,end) */
XlyVal *xly_str_slice(XlyVal *s, XlyVal *start, XlyVal *end) {
    if (!s || s->type != VAL_STRING) return xly_str("");
    size_t len = strlen(s->str);
    size_t st  = start ? (size_t)(long long)start->num : 0;
    size_t en  = end   ? (size_t)(long long)end->num   : len;
    if (st > len) st = len;
    if (en > len) en = len;
    if (en < st)  en = st;
    char *buf = (char*)malloc(en - st + 1);
    memcpy(buf, s->str + st, en - st);
    buf[en - st] = '\0';
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

/* xly_str_byte(s, idx) → number: byte value (0-255) of character at idx */
XlyVal *xly_str_byte(XlyVal *s, XlyVal *idx) {
    if (!s || s->type != VAL_STRING || !idx) return xly_num(-1);
    int i = (int)idx->num;
    int len = (int)strlen(s->str);
    if (i < 0 || i >= len) return xly_num(-1);
    return xly_num((double)(unsigned char)s->str[i]);
}

/* xly_str_concat(a, b) → string: always concatenates as strings */
XlyVal *xly_str_concat(XlyVal *a, XlyVal *b) {
    char *sa = xly_to_cstr(a), *sb = xly_to_cstr(b);
    size_t la = strlen(sa), lb = strlen(sb);
    char *buf = (char*)malloc(la + lb + 1);
    memcpy(buf, sa, la); memcpy(buf+la, sb, lb); buf[la+lb] = '\0';
    free(sa); free(sb);
    XlyVal *r = xly_str(buf); free(buf);
    return r;
}

/* xly_str_find(haystack, needle) → number: first index or -1 */
XlyVal *xly_str_find(XlyVal *haystack, XlyVal *needle) {
    if (!haystack || haystack->type != VAL_STRING ||
        !needle   || needle->type   != VAL_STRING)
        return xly_num(-1);
    const char *pos = strstr(haystack->str, needle->str);
    if (!pos) return xly_num(-1);
    return xly_num((double)(pos - haystack->str));
}

/* xly_str_repeat(s, n) → string: s repeated n times */
XlyVal *xly_str_repeat(XlyVal *s, XlyVal *n) {
    if (!s || s->type != VAL_STRING || !n) return xly_str("");
    size_t times = (size_t)(long long)n->num;
    if (times == 0) return xly_str("");
    size_t slen = strlen(s->str);
    size_t total = slen * times;
    char *buf = (char*)malloc(total + 1);
    for (size_t i = 0; i < times; i++) memcpy(buf + i * slen, s->str, slen);
    buf[total] = '\0';
    XlyVal *r = xly_str(buf); free(buf);
    return r;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * SYSTEMS PROGRAMMING — NUMERIC HELPERS
 * ══════════════════════════════════════════════════════════════════════════════ */

/* xly_int(v) — truncate to integer (like C (int) cast) */
XlyVal *xly_int(XlyVal *v) {
    if (!v || v->type != VAL_NUMBER) return xly_num(0);
    return xly_num((double)(long long)v->num);
}

XlyVal *xly_abs  (XlyVal *v) { return xly_num(fabs(v->num));  }
XlyVal *xly_floor(XlyVal *v) { return xly_num(floor(v->num)); }
XlyVal *xly_ceil (XlyVal *v) { return xly_num(ceil(v->num));  }
XlyVal *xly_round(XlyVal *v) { return xly_num(round(v->num)); }
XlyVal *xly_sqrt (XlyVal *v) { return xly_num(sqrt(v->num));  }

XlyVal *xly_pow(XlyVal *base, XlyVal *exp) {
    return xly_num(pow(base->num, exp->num));
}

XlyVal *xly_min(XlyVal *a, XlyVal *b) {
    return xly_num(a->num < b->num ? a->num : b->num);
}

XlyVal *xly_max(XlyVal *a, XlyVal *b) {
    return xly_num(a->num > b->num ? a->num : b->num);
}

XlyVal *xly_clamp(XlyVal *v, XlyVal *lo, XlyVal *hi) {
    double x = v->num;
    if (x < lo->num) x = lo->num;
    if (x > hi->num) x = hi->num;
    return xly_num(x);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * SYSTEMS PROGRAMMING — RAW MEMORY ACCESS
 *
 * These functions allow Xenly compiled programs to directly read/write
 * memory regions obtained from sys.mmap() — enabling zero-copy I/O,
 * shared memory IPC, and memory-mapped file parsing.
 * ══════════════════════════════════════════════════════════════════════════════ */

/* xly_ptr_read(ptr_num, offset, size) → string of raw bytes */
XlyVal *xly_ptr_read(XlyVal *ptr_num, XlyVal *offset, XlyVal *size) {
    if (!ptr_num || !offset || !size) return xly_null();
    uint8_t *base   = (uint8_t *)(uintptr_t)(long long)ptr_num->num;
    size_t   off    = (size_t)(long long)offset->num;
    size_t   count  = (size_t)(long long)size->num;
    if (count == 0 || count > 67108864) return xly_str("");  /* cap at 64 MB */
    char *buf = (char*)malloc(count + 1);
    if (!buf) return xly_null();
    memcpy(buf, base + off, count);
    buf[count] = '\0';
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

/* xly_ptr_write(ptr_num, offset, data) → number: bytes written */
XlyVal *xly_ptr_write(XlyVal *ptr_num, XlyVal *offset, XlyVal *data) {
    if (!ptr_num || !offset || !data || data->type != VAL_STRING) return xly_num(-1);
    uint8_t    *base = (uint8_t *)(uintptr_t)(long long)ptr_num->num;
    size_t      off  = (size_t)(long long)offset->num;
    const char *src  = data->str;
    size_t      len  = strlen(src);
    memcpy(base + off, src, len);
    return xly_num((double)len);
}

/* Typed fixed-width memory reads — like C (uint8_t*), (uint16_t*), etc. */
XlyVal *xly_ptr_read_u8 (XlyVal *p, XlyVal *o) {
    uint8_t  *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    return xly_num((double)b[off]); }
XlyVal *xly_ptr_read_u16(XlyVal *p, XlyVal *o) {
    uint8_t  *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    uint16_t  v; memcpy(&v, b+off, 2); return xly_num((double)v); }
XlyVal *xly_ptr_read_u32(XlyVal *p, XlyVal *o) {
    uint8_t  *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    uint32_t  v; memcpy(&v, b+off, 4); return xly_num((double)v); }
XlyVal *xly_ptr_read_u64(XlyVal *p, XlyVal *o) {
    uint8_t  *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    uint64_t  v; memcpy(&v, b+off, 8); return xly_num((double)(long long)v); }

/* Typed fixed-width memory writes */
XlyVal *xly_ptr_write_u8 (XlyVal *p, XlyVal *o, XlyVal *v) {
    uint8_t *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    b[off] = (uint8_t)(long long)v->num; return xly_num(1); }
XlyVal *xly_ptr_write_u16(XlyVal *p, XlyVal *o, XlyVal *v) {
    uint8_t *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    uint16_t val = (uint16_t)(long long)v->num; memcpy(b+off, &val, 2); return xly_num(2); }
XlyVal *xly_ptr_write_u32(XlyVal *p, XlyVal *o, XlyVal *v) {
    uint8_t *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    uint32_t val = (uint32_t)(long long)v->num; memcpy(b+off, &val, 4); return xly_num(4); }
XlyVal *xly_ptr_write_u64(XlyVal *p, XlyVal *o, XlyVal *v) {
    uint8_t *b = (uint8_t *)(uintptr_t)(long long)p->num; size_t off = (size_t)(long long)o->num;
    uint64_t val = (uint64_t)(long long)v->num; memcpy(b+off, &val, 8); return xly_num(8); }

/* ══════════════════════════════════════════════════════════════════════════════
 * SYSTEMS PROGRAMMING — DIRECT LINUX SYSCALL
 *
 * xly_syscall wraps the Linux syscall(2) function directly.
 * This lets Xenly programs call any syscall by number — the lowest-level
 * interface to the kernel, used by:
 *   — Custom memory allocators (mmap without libc overhead)
 *   — Sandboxed environments (restricting seccomp filters)
 *   — Performance-critical I/O (bypassing stdio buffering)
 *   — OS programming coursework / teaching
 * ══════════════════════════════════════════════════════════════════════════════ */

#if defined(__linux__)
#include <sys/syscall.h>
/* explicit prototype since -std=c11 may hide it */
extern long syscall(long nr, ...);

XlyVal *xly_syscall(XlyVal *nr, XlyVal *a1, XlyVal *a2,
                    XlyVal *a3, XlyVal *a4, XlyVal *a5, XlyVal *a6) {
    long n   = nr ? (long)nr->num : 0;
    long r1  = a1 ? (long)(long long)a1->num : 0;
    long r2  = a2 ? (long)(long long)a2->num : 0;
    long r3  = a3 ? (long)(long long)a3->num : 0;
    long r4  = a4 ? (long)(long long)a4->num : 0;
    long r5  = a5 ? (long)(long long)a5->num : 0;
    long r6  = a6 ? (long)(long long)a6->num : 0;
    long ret = syscall(n, r1, r2, r3, r4, r5, r6);
    return xly_num((double)ret);
}
#else
/* Stub for non-Linux platforms */
XlyVal *xly_syscall(XlyVal *nr, XlyVal *a1, XlyVal *a2,
                    XlyVal *a3, XlyVal *a4, XlyVal *a5, XlyVal *a6) {
    (void)nr; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    xly_write_stderr("[xly_rt] xly_syscall: not supported on this platform\n");
    return xly_num(-1);
}
#endif

/* ══════════════════════════════════════════════════════════════════════════════
 * SYSTEMS PROGRAMMING — IEEE 754 BIT REINTERPRETATION
 *
 * Essential for:
 *   — Fast inverse square root (Quake III algorithm)
 *   — NaN boxing / tagged pointer schemes
 *   — Floating-point bit manipulation for compression codecs
 * ══════════════════════════════════════════════════════════════════════════════ */

/* xly_num_bits(v) → number: bit pattern of double as uint64 */
XlyVal *xly_num_bits(XlyVal *v) {
    if (!v || v->type != VAL_NUMBER) return xly_num(0);
    uint64_t bits;
    memcpy(&bits, &v->num, 8);
    return xly_num((double)(long long)bits);
}

/* xly_bits_num(v) → number: uint64 bit pattern reinterpreted as double */
XlyVal *xly_bits_num(XlyVal *v) {
    if (!v || v->type != VAL_NUMBER) return xly_num(0.0);
    uint64_t bits = (uint64_t)(long long)v->num;
    double   d;
    memcpy(&d, &bits, 8);
    return xly_num(d);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * ERROR I/O
 *
 * Systems programs need to write to stderr without going through the normal
 * print path. These functions bypass stdout buffering and write directly
 * to fd 2 via write(2), matching the sys.write(sys.STDERR(), ...) idiom.
 * ══════════════════════════════════════════════════════════════════════════════ */

void xly_write_stderr(const char *msg) {
    if (!msg) return;
    size_t len = strlen(msg);
    ssize_t written = 0;
    while ((size_t)written < len) {
        ssize_t r = write(2, msg + written, len - (size_t)written);
        if (r <= 0) break;
        written += r;
    }
}

/* xly_eprint(v) → null: print value to stderr, return null */
XlyVal *xly_eprint(XlyVal *v) {
    char *s = xly_to_cstr(v);
    xly_write_stderr(s);
    xly_write_stderr("\n");
    free(s);
    return xly_null();
}

/* xly_perror(msg) → null: print "msg: strerror(errno)\n" to stderr */
XlyVal *xly_perror(XlyVal *msg) {
    char *s = msg ? xly_to_cstr(msg) : strdup("error");
    char buf[512];
    snprintf(buf, sizeof(buf), "%s: %s\n", s, strerror(errno));
    xly_write_stderr(buf);
    free(s);
    return xly_null();
}

/* ══════════════════════════════════════════════════════════════════════════════
 * ARRAY OPERATIONS
 * ══════════════════════════════════════════════════════════════════════════════ */

XlyVal *xly_array_create(XlyVal **elems, size_t n) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type      = VAL_ARRAY;
    v->array_len = n;
    v->array_cap = n ? n : 4;
    v->array     = (XlyVal**)malloc(sizeof(XlyVal*) * v->array_cap);
    if (n) memcpy(v->array, elems, sizeof(XlyVal*) * n);
    return v;
}

size_t  xly_array_len(XlyVal *arr)            { return arr ? arr->array_len : 0; }
XlyVal *xly_array_get(XlyVal *arr, size_t i)  { return (arr && i < arr->array_len) ? arr->array[i] : xly_null(); }
void    xly_array_set(XlyVal *arr, size_t i, XlyVal *val) { if (arr && i < arr->array_len) arr->array[i] = val; }

XlyVal *xly_array_push(XlyVal *arr, XlyVal *val) {
    if (!arr || arr->type != VAL_ARRAY) return xly_null();
    if (arr->array_len >= arr->array_cap) {
        arr->array_cap = arr->array_cap ? arr->array_cap * 2 : 4;
        arr->array = (XlyVal**)realloc(arr->array, sizeof(XlyVal*)*arr->array_cap);
    }
    arr->array[arr->array_len++] = val;
    return arr;
}

/* xly_array_pop(arr) → last element (or null if empty) */
XlyVal *xly_array_pop(XlyVal *arr) {
    if (!arr || arr->type != VAL_ARRAY || arr->array_len == 0) return xly_null();
    return arr->array[--arr->array_len];
}

/* xly_array_slice(arr, start, end) → new array [start, end) */
XlyVal *xly_array_slice(XlyVal *arr, XlyVal *start, XlyVal *end) {
    if (!arr || arr->type != VAL_ARRAY) return xly_array_create(NULL, 0);
    size_t len = arr->array_len;
    size_t st  = start ? (size_t)(long long)start->num : 0;
    size_t en  = end   ? (size_t)(long long)end->num   : len;
    if (st > len) st = len;
    if (en > len) en = len;
    if (en < st)  en = st;
    size_t   count = en - st;
    XlyVal **elems = count ? (XlyVal**)malloc(sizeof(XlyVal*)*count) : NULL;
    for (size_t i = 0; i < count; i++) elems[i] = arr->array[st + i];
    XlyVal *r = xly_array_create(elems, count);
    free(elems);
    return r;
}

/* xly_array_concat(a, b) → new array with elements of both */
XlyVal *xly_array_concat(XlyVal *a, XlyVal *b) {
    size_t la = a ? a->array_len : 0;
    size_t lb = b ? b->array_len : 0;
    size_t total = la + lb;
    XlyVal **elems = total ? (XlyVal**)malloc(sizeof(XlyVal*)*total) : NULL;
    for (size_t i = 0; i < la; i++) elems[i]    = a->array[i];
    for (size_t i = 0; i < lb; i++) elems[la+i] = b->array[i];
    XlyVal *r = xly_array_create(elems, total);
    free(elems);
    return r;
}

/* xly_array_reverse(arr) → new reversed array */
XlyVal *xly_array_reverse(XlyVal *arr) {
    if (!arr || arr->type != VAL_ARRAY) return xly_array_create(NULL, 0);
    size_t   n     = arr->array_len;
    XlyVal **elems = n ? (XlyVal**)malloc(sizeof(XlyVal*)*n) : NULL;
    for (size_t i = 0; i < n; i++) elems[i] = arr->array[n-1-i];
    XlyVal *r = xly_array_create(elems, n);
    free(elems);
    return r;
}

/* xly_array_contains(arr, val) → bool */
XlyVal *xly_array_contains(XlyVal *arr, XlyVal *val) {
    if (!arr || arr->type != VAL_ARRAY) return xly_bool(0);
    for (size_t i = 0; i < arr->array_len; i++)
        if (vals_equal(arr->array[i], val)) return xly_bool(1);
    return xly_bool(0);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * MODULE DISPATCH
 *
 * Bridges compiled code → the same 160+ module functions the interpreter uses.
 * modules_get() is defined in modules.c and compiled into libxly_rt.a.
 * ══════════════════════════════════════════════════════════════════════════════ */

typedef XlyVal* (*NativeFn)(XlyVal **args, size_t argc);
typedef struct { const char *name; NativeFn fn; } NFunc;
typedef struct { const char *name; NFunc *functions; size_t fn_count; } Mod;

extern int modules_get(const char *name, void *out);

XlyVal *xly_call_module(const char *mod, const char *fn,
                         XlyVal **args, size_t argc) {
    Mod m;
    memset(&m, 0, sizeof(m));
    if (!modules_get(mod, &m)) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[xenly] unknown module '%s'\n", mod);
        xly_write_stderr(buf);
        return xly_null();
    }
    for (size_t i = 0; i < m.fn_count; i++) {
        if (strcmp(m.functions[i].name, fn) == 0)
            return m.functions[i].fn(args, argc);
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "[xenly] '%s' not found in module '%s'\n", fn, mod);
    xly_write_stderr(buf);
    return xly_null();
}

/* ══════════════════════════════════════════════════════════════════════════════
 * INDEXING
 * ══════════════════════════════════════════════════════════════════════════════ */

XlyVal *xly_index(XlyVal *collection, XlyVal *index_val) {
    if (!collection || !index_val) return xly_null();
    if (index_val->type != VAL_NUMBER) return xly_null();
    int idx = (int)index_val->num;
    if (collection->type == VAL_ARRAY) {
        if (idx < 0 || (size_t)idx >= collection->array_len) return xly_null();
        return collection->array[idx];
    }
    if (collection->type == VAL_STRING) {
        int len = (int)strlen(collection->str);
        if (idx < 0 || idx >= len) return xly_null();
        char buf[2] = { collection->str[idx], '\0' };
        return xly_str(buf);
    }
    return xly_null();
}

/* ══════════════════════════════════════════════════════════════════════════════
 * PROCESS EXIT
 * ══════════════════════════════════════════════════════════════════════════════ */

void xly_exit(int code) { exit(code); }

/* ── first-class function values ──────────────────────────────────────────── */

/* Wrap a raw C function pointer as a VAL_FUNCTION XlyVal*.
 * The raw code pointer is stored in `builtin_fn` (offset 48).
 * `fn` (offset 40) is kept NULL so modules.c does not confuse this
 * for an interpreter FnDef*.  `inner` (offset 56) stays NULL → plain fn. */
XlyVal *xly_make_fn(void *fp) {
    XlyVal *v = calloc(1, sizeof(XlyVal));
    v->type       = VAL_FUNCTION;
    v->builtin_fn = fp;   /* raw code pointer — offset 48 */
    return v;
}

/* Create a closure: function pointer + captured environment.
 * The environment is a heap-allocated array of XlyVal* values.
 * Stored in `inner` (offset 56).  `builtin_fn` (offset 48) holds the ptr. */
XlyVal *xly_make_closure(void *fp, XlyVal **env, int env_size) {
    XlyVal *v = calloc(1, sizeof(XlyVal));
    v->type       = VAL_FUNCTION;
    v->builtin_fn = fp;   /* raw code pointer — offset 48 */
    if (env_size > 0 && env) {
        XlyVal **captured = malloc(sizeof(XlyVal*) * (size_t)env_size);
        memcpy(captured, env, sizeof(XlyVal*) * (size_t)env_size);
        v->inner = (XlyVal*)captured;  /* closure env — offset 56 */
    }
    return v;
}

/* Get the environment pointer from a closure XlyVal* (NULL if none). */
XlyVal **xly_closure_env(XlyVal *closure) {
    if (!closure || closure->type != VAL_FUNCTION) return NULL;
    return (XlyVal**)closure->inner;
}

/* Call a VAL_FUNCTION XlyVal* with 0–6 args.
 * For plain functions (inner == NULL): call builtin_fn(args[0], ...)
 * For closures (inner != NULL):        call builtin_fn(env, args[0], ...) */
XlyVal *xly_call_fnval(XlyVal *fn_val, XlyVal **args, int argc) {
    if (!fn_val || fn_val->type != VAL_FUNCTION || !fn_val->builtin_fn)
        return xly_null();
    XlyVal **env = (XlyVal**)fn_val->inner;  /* NULL for plain fns */
    typedef XlyVal *(*F0)(void);
    typedef XlyVal *(*F1)(XlyVal*);
    typedef XlyVal *(*F2)(XlyVal*,XlyVal*);
    typedef XlyVal *(*F3)(XlyVal*,XlyVal*,XlyVal*);
    typedef XlyVal *(*F4)(XlyVal*,XlyVal*,XlyVal*,XlyVal*);
    typedef XlyVal *(*F5)(XlyVal*,XlyVal*,XlyVal*,XlyVal*,XlyVal*);
    typedef XlyVal *(*F6)(XlyVal*,XlyVal*,XlyVal*,XlyVal*,XlyVal*,XlyVal*);
    typedef XlyVal *(*F7)(XlyVal*,XlyVal*,XlyVal*,XlyVal*,XlyVal*,XlyVal*,XlyVal*);
    if (!env) {
        /* Plain function — call with args directly */
        switch (argc) {
            case 0: return ((F0)fn_val->builtin_fn)();
            case 1: return ((F1)fn_val->builtin_fn)(args[0]);
            case 2: return ((F2)fn_val->builtin_fn)(args[0],args[1]);
            case 3: return ((F3)fn_val->builtin_fn)(args[0],args[1],args[2]);
            case 4: return ((F4)fn_val->builtin_fn)(args[0],args[1],args[2],args[3]);
            case 5: return ((F5)fn_val->builtin_fn)(args[0],args[1],args[2],args[3],args[4]);
            case 6: return ((F6)fn_val->builtin_fn)(args[0],args[1],args[2],args[3],args[4],args[5]);
            default: return xly_null();
        }
    } else {
        /* Closure — prepend env as first hidden arg */
        switch (argc) {
            case 0: return ((F1)fn_val->builtin_fn)((XlyVal*)env);
            case 1: return ((F2)fn_val->builtin_fn)((XlyVal*)env,args[0]);
            case 2: return ((F3)fn_val->builtin_fn)((XlyVal*)env,args[0],args[1]);
            case 3: return ((F4)fn_val->builtin_fn)((XlyVal*)env,args[0],args[1],args[2]);
            case 4: return ((F5)fn_val->builtin_fn)((XlyVal*)env,args[0],args[1],args[2],args[3]);
            case 5: return ((F6)fn_val->builtin_fn)((XlyVal*)env,args[0],args[1],args[2],args[3],args[4]);
            case 6: return ((F7)fn_val->builtin_fn)((XlyVal*)env,args[0],args[1],args[2],args[3],args[4],args[5]);
            default: return xly_null();
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════════════
 * OBJECT / INSTANCE OPERATIONS
 *
 * Compiled Xenly object literals { key: val, ... } use VAL_INSTANCE with
 * a flat key-value store allocated in the `instance` field.
 * Layout: XlyObjStore { char **keys; XlyVal **vals; size_t count; size_t cap; }
 * ══════════════════════════════════════════════════════════════════════════════ */

typedef struct {
    char    **keys;
    XlyVal  **vals;
    size_t    count;
    size_t    cap;
} XlyObjStore;

/* Create an empty object (VAL_INSTANCE). */
XlyVal *xly_obj_new(void) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type = VAL_INSTANCE;
    XlyObjStore *s = (XlyObjStore*)calloc(1, sizeof(XlyObjStore));
    s->cap  = 4;
    s->keys = (char**)malloc(sizeof(char*) * s->cap);
    s->vals = (XlyVal**)malloc(sizeof(XlyVal*) * s->cap);
    v->instance = s;
    return v;
}

/* Set (or add) a field on an object. */
void xly_obj_set(XlyVal *obj, const char *key, XlyVal *val) {
    if (!obj || obj->type != VAL_INSTANCE || !key) return;
    XlyObjStore *s = (XlyObjStore*)obj->instance;
    if (!s) return;
    for (size_t i = 0; i < s->count; i++) {
        if (strcmp(s->keys[i], key) == 0) { s->vals[i] = val; return; }
    }
    if (s->count >= s->cap) {
        s->cap *= 2;
        s->keys = (char**)realloc(s->keys, sizeof(char*) * s->cap);
        s->vals = (XlyVal**)realloc(s->vals, sizeof(XlyVal*) * s->cap);
    }
    s->keys[s->count] = strdup(key);
    s->vals[s->count] = val;
    s->count++;
}

/* Get a field from an object (returns xly_null() if not found). */
XlyVal *xly_obj_get(XlyVal *obj, const char *key) {
    if (!obj || !key) return xly_null();
    if (obj->type == VAL_INSTANCE) {
        XlyObjStore *s = (XlyObjStore*)obj->instance;
        if (s) {
            for (size_t i = 0; i < s->count; i++)
                if (strcmp(s->keys[i], key) == 0) return s->vals[i];
        }
        return xly_null();
    }
    /* Support field access on arrays too: .length */
    if (obj->type == VAL_ARRAY) {
        if (strcmp(key, "length") == 0) return xly_num((double)obj->array_len);
        return xly_null();
    }
    return xly_null();
}

/* ── this / self support ─────────────────────────────────────────────────
 * When a method is called via xly_obj_call, we store the receiver object
 * in a global so compiled functions can retrieve it via xly_this().
 * This is not thread-safe, but matches the single-threaded interpreter.   */
static XlyVal *g_this = NULL;
XlyVal *xly_this(void) { return g_this ? g_this : xly_null(); }

/* Call a method on an object: obj.method(args, argc).
 * Looks up field `method` on `obj`; if it's a VAL_FUNCTION, calls it.
 * Sets g_this = obj so `this` is accessible inside the method body. */
XlyVal *xly_obj_call(XlyVal *obj, const char *method, XlyVal **args, int argc) {
    XlyVal *fn_val = xly_obj_get(obj, method);
    if (!fn_val || fn_val->type != VAL_FUNCTION) return xly_null();
    XlyVal *prev_this = g_this;
    g_this = obj;
    XlyVal *result = xly_call_fnval(fn_val, args, argc);
    g_this = prev_this;
    return result;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * MUTABLE CLOSURE CAPTURE CELLS
 *
 * For `var` variables captured by closures, we use a heap-allocated cell
 * (a single XlyVal* on the heap).  All closures sharing the capture hold a
 * pointer to the SAME cell, so mutations are visible across calls.
 *
 * xly_make_cell(val) → XlyVal** (heap pointer, initially pointing to val)
 * xly_cell_get(cell) → XlyVal*
 * xly_cell_set(cell, val)
 * ══════════════════════════════════════════════════════════════════════════════ */

XlyVal **xly_make_cell(XlyVal *initial) {
    XlyVal **cell = (XlyVal**)malloc(sizeof(XlyVal*));
    *cell = initial;
    return cell;
}

XlyVal *xly_cell_get(XlyVal **cell) {
    return cell ? *cell : xly_null();
}

void xly_cell_set(XlyVal **cell, XlyVal *val) {
    if (cell) *cell = val;
}


XlyVal *value_number(double n)        { return xly_num(n);   }
XlyVal *value_string(const char *s)   { return xly_str(s);   }
XlyVal *value_bool(int b)             { return xly_bool(b);  }
XlyVal *value_null(void)              { return xly_null();   }

XlyVal *value_array(XlyVal **items, size_t len) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type      = VAL_ARRAY;
    v->array     = items;
    v->array_len = len;
    v->array_cap = items ? (len ? len : 4) : 0;
    return v;
}

void value_destroy(XlyVal *v) {
    if (!v) return;
    switch (v->type) {
        case VAL_STRING: free(v->str); free(v); break;
        case VAL_NUMBER:
        case VAL_BOOL:
        case VAL_NULL:   free(v); break;
        default: break;  /* long-lived: arrays, functions, classes */
    }
}

char *value_to_string(XlyVal *v) { return xly_to_cstr(v); }

XlyVal *value_clone(XlyVal *v) {
    if (!v) return xly_null();
    switch (v->type) {
        case VAL_NUMBER: return xly_num(v->num);
        case VAL_STRING: return xly_str(v->str);
        case VAL_BOOL:   return xly_bool(v->boolean);
        case VAL_NULL:   return xly_null();
        case VAL_ARRAY: {
            size_t alloc = v->array_len > 0 ? v->array_len : 4;
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * alloc);
            for (size_t i = 0; i < v->array_len; i++) items[i] = value_clone(v->array[i]);
            return xly_array_create(items, v->array_len);
        }
        default: return v;
    }
}
