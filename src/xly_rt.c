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
#include "unicode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif
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

/* xly_make_variant(tag_str, fields_array, nfields) → VAL_ENUM_VARIANT
 * tag_str    : XlyVal* string holding the variant name (e.g. "Some")
 * fields_arr : XlyVal** array of field values (may be NULL if nfields==0)
 * nfields    : XlyVal* number holding field count
 * Called from compiled code for both parameterless and parametric variants. */
XlyVal *xly_make_variant(XlyVal *tag_val, XlyVal **fields, int nfields) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type = VAL_ENUM_VARIANT;
    v->variant.tag = strdup(tag_val && tag_val->str ? tag_val->str : "?");
    v->variant.field_count = (size_t)nfields;
    if (nfields > 0 && fields) {
        v->variant.fields = (XlyVal**)malloc(sizeof(XlyVal*) * (size_t)nfields);
        for (int i = 0; i < nfields; i++)
            v->variant.fields[i] = fields[i];
    } else {
        v->variant.fields = NULL;
    }
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
        case VAL_ENUM_VARIANT: {
            const char *tag = v->variant.tag ? v->variant.tag : "?";
            if (v->variant.field_count == 0) return strdup(tag);
            /* Tag(field0, field1, ...) */
            size_t cap = 128;
            char  *out = (char*)malloc(cap);
            size_t pos = 0;
            size_t tlen = strlen(tag);
            while (pos + tlen + 2 >= cap) { cap *= 2; out = (char*)realloc(out, cap); }
            memcpy(out + pos, tag, tlen); pos += tlen;
            out[pos++] = '(';
            for (size_t i = 0; i < v->variant.field_count; i++) {
                if (i) { out[pos++] = ','; out[pos++] = ' '; }
                char *fs = xly_to_cstr(v->variant.fields[i]);
                size_t flen = strlen(fs);
                while (pos + flen + 4 >= cap) { cap *= 2; out = (char*)realloc(out, cap); }
                memcpy(out + pos, fs, flen); pos += flen;
                free(fs);
            }
            if (pos + 2 >= cap) { cap *= 2; out = (char*)realloc(out, cap); }
            out[pos++] = ')';
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

/* xly_str_len(s) → number: Unicode codepoint count (not byte count).
 * Use the new utf8_strlen from unicode.h for correct multi-byte handling. */
XlyVal *xly_str_len(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)utf8_strlen(s->str));
}

/* xly_str_byte_len(s) → number: raw byte length (old xly_str_len behaviour) */
XlyVal *xly_str_byte_len(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)strlen(s->str));
}

/* xly_str_slice(s, start, end) → string: codepoints [start,end).
 * Now Unicode-aware: indices are codepoint positions, not byte offsets. */
XlyVal *xly_str_slice(XlyVal *s, XlyVal *start, XlyVal *end) {
    if (!s || s->type != VAL_STRING) return xly_str("");
    size_t clen = utf8_strlen(s->str);
    size_t st = start ? (size_t)(long long)start->num : 0;
    size_t en = end   ? (size_t)(long long)end->num   : clen;
    if (st > clen) st = clen;
    if (en > clen) en = clen;
    if (en < st)   en = st;
    /* allocate worst-case: (en-st)*4 + 1 bytes */
    size_t buf_size = (en - st) * UTF8_MAX_BYTES + 1;
    char *buf = (char *)malloc(buf_size);
    if (!buf) return xly_str("");
    utf8_slice(s->str, st, en, buf, buf_size);
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

    /* ── Special higher-order array functions ───────────────────────────────
     * These require xly_call_fnval (not available in modules.c), so they are
     * intercepted here before the general modules_get() dispatch.            */
    if (strcmp(mod, "array") == 0) {

        /* array.length(arr) — alias for array.len */
        if (strcmp(fn, "length") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_num(0.0);
            return xly_num((double)args[0]->array_len);
        }

        /* array.map(arr, fn) — new array with fn applied to every element */
        if (strcmp(fn, "map") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_array_create(NULL, 0);
            XlyVal *arr  = args[0];
            XlyVal *cb   = args[1];
            size_t   n   = arr->array_len;
            XlyVal **out = (XlyVal **)malloc(sizeof(XlyVal *) * (n ? n : 1));
            for (size_t k = 0; k < n; k++) {
                XlyVal *ea[1] = { arr->array[k] };
                XlyVal *r = xly_call_fnval(cb, ea, 1);
                out[k] = r ? r : xly_null();
            }
            XlyVal *result = xly_array_create(out, n);
            free(out);
            return result;
        }

        /* array.filter(arr, fn) — new array with elements where fn returns truthy */
        if (strcmp(fn, "filter") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_array_create(NULL, 0);
            XlyVal *arr  = args[0];
            XlyVal *cb   = args[1];
            size_t   n   = arr->array_len;
            XlyVal **out = (XlyVal **)malloc(sizeof(XlyVal *) * (n ? n : 1));
            size_t out_n = 0;
            for (size_t k = 0; k < n; k++) {
                XlyVal *ea[1]  = { arr->array[k] };
                XlyVal *keep   = xly_call_fnval(cb, ea, 1);
                int     truthy = keep &&
                                 keep->type != VAL_NULL &&
                                 !(keep->type == VAL_BOOL && !keep->boolean) &&
                                 !(keep->type == VAL_NUMBER && keep->num == 0.0);
                if (truthy) out[out_n++] = arr->array[k];
            }
            XlyVal *result = xly_array_create(out, out_n);
            free(out);
            return result;
        }

        /* array.reduce(arr, fn, initial) — fold array to a single value */
        if (strcmp(fn, "reduce") == 0) {
            if (argc < 3 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return (argc >= 3 && args) ? args[2] : xly_null();
            XlyVal *arr = args[0];
            XlyVal *cb  = args[1];
            XlyVal *acc = args[2];
            for (size_t k = 0; k < arr->array_len; k++) {
                XlyVal *ea[2] = { acc, arr->array[k] };
                XlyVal *next  = xly_call_fnval(cb, ea, 2);
                acc = next ? next : xly_null();
            }
            return acc;
        }

        /* array.forEach(arr, fn) — call fn for each element, return null */
        if (strcmp(fn, "forEach") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_null();
            XlyVal *arr = args[0];
            XlyVal *cb  = args[1];
            for (size_t k = 0; k < arr->array_len; k++) {
                XlyVal *ea[2] = { arr->array[k], xly_num((double)k) };
                xly_call_fnval(cb, ea, 1);
            }
            return xly_null();
        }

        /* array.find(arr, fn) — return first element where fn is truthy, or null */
        if (strcmp(fn, "find") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_null();
            XlyVal *arr = args[0];
            XlyVal *cb  = args[1];
            for (size_t k = 0; k < arr->array_len; k++) {
                XlyVal *ea[1]  = { arr->array[k] };
                XlyVal *keep   = xly_call_fnval(cb, ea, 1);
                int     truthy = keep &&
                                 keep->type != VAL_NULL &&
                                 !(keep->type == VAL_BOOL && !keep->boolean);
                if (truthy) return arr->array[k];
            }
            return xly_null();
        }

        /* array.some(arr, fn) — true if any element passes fn */
        if (strcmp(fn, "some") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_bool(0);
            XlyVal *arr = args[0];
            XlyVal *cb  = args[1];
            for (size_t k = 0; k < arr->array_len; k++) {
                XlyVal *ea[1]  = { arr->array[k] };
                XlyVal *r      = xly_call_fnval(cb, ea, 1);
                int     truthy = r &&
                                 r->type != VAL_NULL &&
                                 !(r->type == VAL_BOOL && !r->boolean);
                if (truthy) return xly_bool(1);
            }
            return xly_bool(0);
        }

        /* array.every(arr, fn) — true if all elements pass fn */
        if (strcmp(fn, "every") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_bool(1);
            XlyVal *arr = args[0];
            XlyVal *cb  = args[1];
            for (size_t k = 0; k < arr->array_len; k++) {
                XlyVal *ea[1]  = { arr->array[k] };
                XlyVal *r      = xly_call_fnval(cb, ea, 1);
                int     truthy = r &&
                                 r->type != VAL_NULL &&
                                 !(r->type == VAL_BOOL && !r->boolean);
                if (!truthy) return xly_bool(0);
            }
            return xly_bool(1);
        }

        /* ── basic accessors/mutators — thin wrappers over existing helpers ── */
        if (strcmp(fn, "len") == 0)
            return xly_num((double)xly_array_len(args && argc >= 1 ? args[0] : NULL));
        if (strcmp(fn, "get") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY) return xly_null();
            long long idx = (long long)args[1]->num;
            long long len = (long long)args[0]->array_len;
            if (idx < 0) idx += len;
            if (idx < 0 || idx >= len) return xly_null();
            return xly_array_get(args[0], (size_t)idx);
        }
        if (strcmp(fn, "set") == 0) {
            if (argc < 3 || !args || !args[0] || args[0]->type != VAL_ARRAY) return xly_null();
            long long idx = (long long)args[1]->num;
            long long len = (long long)args[0]->array_len;
            if (idx < 0) idx += len;
            if (idx < 0 || idx >= len) return args[0];
            xly_array_set(args[0], (size_t)idx, args[2]);
            return args[0];
        }
        if (strcmp(fn, "push") == 0) {
            if (argc < 2 || !args || !args[0]) return xly_num(0);
            return xly_array_push(args[0], args[1]);
        }
        if (strcmp(fn, "pop") == 0)
            return xly_array_pop(argc >= 1 && args ? args[0] : NULL);
        if (strcmp(fn, "shift") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY || args[0]->array_len == 0)
                return xly_null();
            XlyVal *arr = args[0];
            XlyVal *first = arr->array[0];
            memmove(arr->array, arr->array + 1, sizeof(XlyVal*) * (arr->array_len - 1));
            arr->array_len--;
            return first;
        }
        if (strcmp(fn, "unshift") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY) return xly_num(0);
            XlyVal *arr = args[0];
            if (arr->array_len >= arr->array_cap) {
                arr->array_cap = arr->array_cap ? arr->array_cap * 2 : 4;
                arr->array = (XlyVal**)realloc(arr->array, sizeof(XlyVal*) * arr->array_cap);
            }
            memmove(arr->array + 1, arr->array, sizeof(XlyVal*) * arr->array_len);
            arr->array[0] = args[1];
            arr->array_len++;
            return arr;
        }
        if (strcmp(fn, "create") == 0) {
            long long n = (argc >= 1 && args && args[0]) ? (long long)args[0]->num : 0;
            if (n < 0) n = 0;
            XlyVal *fill = (argc >= 2 && args) ? args[1] : NULL;
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * (size_t)(n > 0 ? n : 1));
            for (long long i = 0; i < n; i++)
                items[i] = fill ? fill : xly_num(0);
            XlyVal *r = xly_array_create(items, (size_t)n);
            free(items);
            return r;
        }
        if (strcmp(fn, "of") == 0)
            return xly_array_create(args, argc);
        if (strcmp(fn, "empty") == 0)
            return xly_array_create(NULL, 0);
        if (strcmp(fn, "fill") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY) return xly_null();
            for (size_t k = 0; k < args[0]->array_len; k++) args[0]->array[k] = args[1];
            return args[0];
        }
        if (strcmp(fn, "concat") == 0)
            return xly_array_concat(argc >= 1 && args ? args[0] : NULL,
                                     argc >= 2 && args ? args[1] : NULL);
        if (strcmp(fn, "slice") == 0)
            return xly_array_slice(argc >= 1 && args ? args[0] : NULL,
                                    argc >= 2 && args ? args[1] : NULL,
                                    argc >= 3 && args ? args[2] : NULL);
        if (strcmp(fn, "reverse") == 0)
            return xly_array_reverse(argc >= 1 && args ? args[0] : NULL);
        if (strcmp(fn, "contains") == 0)
            return xly_array_contains(argc >= 1 && args ? args[0] : NULL,
                                       argc >= 2 && args ? args[1] : NULL);
        if (strcmp(fn, "indexOf") == 0) {
            if (argc < 2 || !args || !args[0] || args[0]->type != VAL_ARRAY) return xly_num(-1);
            for (size_t k = 0; k < args[0]->array_len; k++)
                if (vals_equal(args[0]->array[k], args[1])) return xly_num((double)k);
            return xly_num(-1);
        }
        if (strcmp(fn, "sum") == 0) {
            double s = 0;
            if (argc >= 1 && args && args[0] && args[0]->type == VAL_ARRAY)
                for (size_t k = 0; k < args[0]->array_len; k++)
                    if (args[0]->array[k]->type == VAL_NUMBER) s += args[0]->array[k]->num;
            return xly_num(s);
        }
        if (strcmp(fn, "min") == 0 || strcmp(fn, "max") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY || args[0]->array_len == 0)
                return xly_null();
            int is_min = strcmp(fn, "min") == 0;
            double m = 0; int found = 0;
            for (size_t k = 0; k < args[0]->array_len; k++) {
                if (args[0]->array[k]->type != VAL_NUMBER) continue;
                double v = args[0]->array[k]->num;
                if (!found || (is_min ? v < m : v > m)) m = v;
                found = 1;
            }
            return found ? xly_num(m) : xly_null();
        }
        if (strcmp(fn, "unique") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_array_create(NULL, 0);
            XlyVal *arr = args[0];
            size_t cap = arr->array_len ? arr->array_len : 1;
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * cap);
            size_t count = 0;
            for (size_t k = 0; k < arr->array_len; k++) {
                int dup = 0;
                for (size_t j = 0; j < count; j++)
                    if (vals_equal(arr->array[k], items[j])) { dup = 1; break; }
                if (!dup) items[count++] = arr->array[k];
            }
            XlyVal *r = xly_array_create(items, count);
            free(items);
            return r;
        }
        if (strcmp(fn, "flatten") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_array_create(NULL, 0);
            XlyVal *arr = args[0];
            size_t cap = 16, count = 0;
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * cap);
            for (size_t k = 0; k < arr->array_len; k++) {
                XlyVal *el = arr->array[k];
                if (el->type == VAL_ARRAY) {
                    for (size_t j = 0; j < el->array_len; j++) {
                        if (count >= cap) { cap *= 2; items = (XlyVal**)realloc(items, sizeof(XlyVal*)*cap); }
                        items[count++] = el->array[j];
                    }
                } else {
                    if (count >= cap) { cap *= 2; items = (XlyVal**)realloc(items, sizeof(XlyVal*)*cap); }
                    items[count++] = el;
                }
            }
            XlyVal *r = xly_array_create(items, count);
            free(items);
            return r;
        }
        if (strcmp(fn, "range") == 0) {
            if (argc < 1 || !args) return xly_array_create(NULL, 0);
            double start = 0, end_val, step = 1;
            if (argc == 1) end_val = args[0]->num;
            else { start = args[0]->num; end_val = args[1]->num; }
            if (argc >= 3) step = args[2]->num;
            if (step == 0) return xly_array_create(NULL, 0);
            size_t count = 0;
            if (step > 0) for (double v = start; v < end_val; v += step) count++;
            else          for (double v = start; v > end_val; v += step) count++;
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * (count ? count : 1));
            size_t idx = 0;
            if (step > 0) for (double v = start; v < end_val; v += step) items[idx++] = xly_num(v);
            else          for (double v = start; v > end_val; v += step) items[idx++] = xly_num(v);
            XlyVal *r = xly_array_create(items, idx);
            free(items);
            return r;
        }
        if (strcmp(fn, "join") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY) return xly_str("");
            const char *sep = (argc >= 2 && args[1] && args[1]->type == VAL_STRING) ? args[1]->str : ",";
            XlyVal *arr = args[0];
            size_t cap = 128, pos = 0;
            char *buf = (char*)malloc(cap);
            buf[0] = '\0';
            for (size_t k = 0; k < arr->array_len; k++) {
                if (k > 0) {
                    size_t sl = strlen(sep);
                    while (pos + sl + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
                    memcpy(buf + pos, sep, sl); pos += sl;
                }
                char *elem = xly_to_cstr(arr->array[k]);
                size_t el = strlen(elem);
                while (pos + el + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
                memcpy(buf + pos, elem, el); pos += el;
                free(elem);
            }
            buf[pos] = '\0';
            XlyVal *r = xly_str(buf);
            free(buf);
            return r;
        }
        if (strcmp(fn, "sort") == 0 || strcmp(fn, "sortDesc") == 0) {
            if (argc < 1 || !args || !args[0] || args[0]->type != VAL_ARRAY)
                return xly_array_create(NULL, 0);
            XlyVal *arr = args[0];
            size_t n = arr->array_len;
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * (n ? n : 1));
            memcpy(items, arr->array, sizeof(XlyVal*) * n);
            int desc = strcmp(fn, "sortDesc") == 0;
            int by_str = (n > 0 && items[0]->type == VAL_STRING);
            /* simple insertion sort — arrays here are small (benchmarks aside) */
            for (size_t i = 1; i < n; i++) {
                XlyVal *key = items[i];
                size_t j = i;
                while (j > 0) {
                    int gt;
                    if (by_str) {
                        const char *a = (items[j-1]->type == VAL_STRING) ? items[j-1]->str : "";
                        const char *b = (key->type == VAL_STRING) ? key->str : "";
                        gt = desc ? (strcmp(a,b) < 0) : (strcmp(a,b) > 0);
                    } else {
                        double a = (items[j-1]->type == VAL_NUMBER) ? items[j-1]->num : 0;
                        double b = (key->type == VAL_NUMBER) ? key->num : 0;
                        gt = desc ? (a < b) : (a > b);
                    }
                    if (!gt) break;
                    items[j] = items[j-1];
                    j--;
                }
                items[j] = key;
            }
            XlyVal *r = xly_array_create(items, n);
            free(items);
            return r;
        }
    }

    if (strcmp(mod, "io") == 0) {
        if (strcmp(fn, "write") == 0) {
            for (size_t k = 0; k < argc; k++) {
                char *s = xly_to_cstr(args[k]);
                fputs(s, stdout);
                free(s);
            }
            fflush(stdout);
            return xly_null();
        }
        if (strcmp(fn, "writeln") == 0) {
            for (size_t k = 0; k < argc; k++) {
                if (k > 0) fputs(" ", stdout);
                char *s = xly_to_cstr(args[k]);
                fputs(s, stdout);
                free(s);
            }
            fputs("\n", stdout);
            return xly_null();
        }
        if (strcmp(fn, "read") == 0) {
            if (argc > 0 && args[0]) {
                char *s = xly_to_cstr(args[0]);
                fputs(s, stdout); fflush(stdout); free(s);
            }
            char buf[4096];
            if (fgets(buf, sizeof(buf), stdin)) {
                size_t len = strlen(buf);
                if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
                return xly_str(buf);
            }
            return xly_str("");
        }
    }

    if (strcmp(mod, "string") == 0) {
        XlyVal *a0 = argc >= 1 ? args[0] : NULL;
        int a0_is_str = a0 && a0->type == VAL_STRING;

        if (strcmp(fn, "len") == 0)
            return xly_num(a0_is_str ? (double)strlen(a0->str) : 0.0);
        if (strcmp(fn, "toString") == 0) {
            if (!a0) return xly_str("null");
            char *s = xly_to_cstr(a0);
            XlyVal *r = xly_str(s);
            free(s);
            return r;
        }
        if (strcmp(fn, "toNumber") == 0) {
            if (!a0) return xly_num(0);
            if (a0->type == VAL_NUMBER) return xly_num(a0->num);
            if (a0->type == VAL_STRING) return xly_num(strtod(a0->str, NULL));
            if (a0->type == VAL_BOOL)   return xly_num(a0->boolean ? 1.0 : 0.0);
            return xly_num(0);
        }
        if (strcmp(fn, "upper") == 0) {
            if (!a0_is_str) return xly_str("");
            char *copy = strdup(a0->str);
            for (char *p = copy; *p; p++) *p = (char)toupper((unsigned char)*p);
            XlyVal *r = xly_str(copy); free(copy); return r;
        }
        if (strcmp(fn, "lower") == 0) {
            if (!a0_is_str) return xly_str("");
            char *copy = strdup(a0->str);
            for (char *p = copy; *p; p++) *p = (char)tolower((unsigned char)*p);
            XlyVal *r = xly_str(copy); free(copy); return r;
        }
        if (strcmp(fn, "contains") == 0)
            return xly_bool(argc >= 2 && a0_is_str && args[1]->type == VAL_STRING &&
                             strstr(a0->str, args[1]->str) != NULL);
        if (strcmp(fn, "startsWith") == 0)
            return xly_bool(argc >= 2 && a0_is_str && args[1]->type == VAL_STRING &&
                             strncmp(a0->str, args[1]->str, strlen(args[1]->str)) == 0);
        if (strcmp(fn, "endsWith") == 0) {
            if (argc < 2 || !a0_is_str || args[1]->type != VAL_STRING) return xly_bool(0);
            size_t slen = strlen(a0->str), plen = strlen(args[1]->str);
            if (plen > slen) return xly_bool(0);
            return xly_bool(strcmp(a0->str + slen - plen, args[1]->str) == 0);
        }
        if (strcmp(fn, "indexOf") == 0) {
            if (argc < 2 || !a0_is_str || args[1]->type != VAL_STRING) return xly_num(-1);
            const char *p = strstr(a0->str, args[1]->str);
            return xly_num(p ? (double)(p - a0->str) : -1.0);
        }
        if (strcmp(fn, "lastIndexOf") == 0) {
            if (argc < 2 || !a0_is_str || args[1]->type != VAL_STRING) return xly_num(-1);
            const char *needle = args[1]->str;
            size_t nlen = strlen(needle);
            const char *last = NULL, *p = a0->str;
            while ((p = strstr(p, needle)) != NULL) { last = p; p += nlen; }
            return xly_num(last ? (double)(last - a0->str) : -1.0);
        }
        if (strcmp(fn, "charAt") == 0) {
            if (argc < 2 || !a0_is_str) return xly_str("");
            int idx = (int)args[1]->num, len = (int)strlen(a0->str);
            if (idx < 0 || idx >= len) return xly_str("");
            char buf[2] = { a0->str[idx], '\0' };
            return xly_str(buf);
        }
        if (strcmp(fn, "charCodeAt") == 0) {
            if (argc < 2 || !a0_is_str) return xly_num(-1);
            int idx = (int)args[1]->num, len = (int)strlen(a0->str);
            if (idx < 0 || idx >= len) return xly_num(-1);
            return xly_num((double)(unsigned char)a0->str[idx]);
        }
        if (strcmp(fn, "fromCharCode") == 0) {
            if (argc < 1) return xly_str("");
            char buf[2] = { (char)(int)args[0]->num, '\0' };
            return xly_str(buf);
        }
        if (strcmp(fn, "repeat") == 0)
            return xly_str_repeat(a0, argc >= 2 ? args[1] : NULL);
        if (strcmp(fn, "reverse") == 0) {
            if (!a0_is_str) return xly_str("");
            size_t len = strlen(a0->str);
            char *buf = (char*)malloc(len + 1);
            for (size_t k = 0; k < len; k++) buf[k] = a0->str[len - 1 - k];
            buf[len] = '\0';
            XlyVal *r = xly_str(buf); free(buf); return r;
        }
        if (strcmp(fn, "trim") == 0) {
            if (!a0_is_str) return xly_str("");
            char *s = strdup(a0->str);
            char *start = s;
            while (*start && isspace((unsigned char)*start)) start++;
            char *end = start + strlen(start);
            while (end > start && isspace((unsigned char)*(end-1))) end--;
            *end = '\0';
            XlyVal *r = xly_str(start); free(s); return r;
        }
        if (strcmp(fn, "trimStart") == 0) {
            if (!a0_is_str) return xly_str("");
            const char *s = a0->str;
            while (*s && isspace((unsigned char)*s)) s++;
            return xly_str(s);
        }
        if (strcmp(fn, "trimEnd") == 0) {
            if (!a0_is_str) return xly_str("");
            char *s = strdup(a0->str);
            char *end = s + strlen(s);
            while (end > s && isspace((unsigned char)*(end-1))) end--;
            *end = '\0';
            XlyVal *r = xly_str(s); free(s); return r;
        }
        if (strcmp(fn, "replace") == 0) {
            if (argc < 3 || !a0_is_str || args[1]->type != VAL_STRING || args[2]->type != VAL_STRING)
                return xly_str("");
            const char *src = a0->str, *old = args[1]->str, *neu = args[2]->str;
            size_t old_len = strlen(old);
            if (old_len == 0) return xly_str(src);
            size_t count = 0;
            const char *p = src;
            while ((p = strstr(p, old)) != NULL) { count++; p += old_len; }
            size_t new_len = strlen(neu), src_len = strlen(src);
            size_t buf_size = src_len - count * old_len + count * new_len + 1;
            char *buf = (char*)malloc(buf_size);
            char *out = buf;
            p = src;
            while (*p) {
                if (strstr(p, old) == p) { memcpy(out, neu, new_len); out += new_len; p += old_len; }
                else { *out++ = *p++; }
            }
            *out = '\0';
            XlyVal *r = xly_str(buf); free(buf); return r;
        }
        if (strcmp(fn, "substr") == 0) {
            if (argc < 2 || !a0_is_str) return xly_str("");
            const char *s = a0->str;
            int start = (int)args[1]->num, len = (int)strlen(s);
            if (start < 0) start = 0;
            if (start >= len) return xly_str("");
            int count = (argc >= 3) ? (int)args[2]->num : (len - start);
            if (count < 0) count = 0;
            if (start + count > len) count = len - start;
            char *buf = (char*)malloc((size_t)count + 1);
            memcpy(buf, s + start, (size_t)count);
            buf[count] = '\0';
            XlyVal *r = xly_str(buf); free(buf); return r;
        }
        if (strcmp(fn, "slice") == 0)
            return xly_str_slice(a0, argc >= 2 ? args[1] : NULL, argc >= 3 ? args[2] : NULL);
        if (strcmp(fn, "padStart") == 0 || strcmp(fn, "padEnd") == 0) {
            if (argc < 2 || !a0_is_str) return xly_str("");
            const char *s = a0->str;
            int pad = (int)args[1]->num;
            const char *ch = (argc >= 3 && args[2]->type == VAL_STRING && args[2]->str[0]) ? args[2]->str : " ";
            int slen = (int)strlen(s);
            if (slen >= pad) return xly_str(s);
            int need = pad - slen, chlen = (int)strlen(ch);
            char *buf = (char*)malloc((size_t)pad + 1);
            if (strcmp(fn, "padStart") == 0) {
                for (int i = 0; i < need; i++) buf[i] = ch[i % chlen];
                memcpy(buf + need, s, (size_t)slen);
            } else {
                memcpy(buf, s, (size_t)slen);
                for (int i = slen; i < pad; i++) buf[i] = ch[(i - slen) % chlen];
            }
            buf[pad] = '\0';
            XlyVal *r = xly_str(buf); free(buf); return r;
        }
        if (strcmp(fn, "split") == 0) {
            if (!a0_is_str) return xly_array_create(NULL, 0);
            const char *s = a0->str;
            const char *sep = (argc >= 2 && args[1]->type == VAL_STRING) ? args[1]->str : " ";
            size_t sep_len = strlen(sep);
            size_t parts = 1;
            if (sep_len > 0) { const char *p = s; while ((p = strstr(p, sep)) != NULL) { parts++; p += sep_len; } }
            XlyVal **items = (XlyVal**)malloc(sizeof(XlyVal*) * parts);
            size_t idx = 0;
            if (sep_len == 0) {
                size_t slen = strlen(s);
                items = (XlyVal**)realloc(items, sizeof(XlyVal*) * (slen ? slen : 1));
                if (slen == 0) { items[0] = xly_str(""); idx = 1; }
                else for (size_t k = 0; k < slen; k++) { char c[2] = {s[k], 0}; items[k] = xly_str(c); idx = k + 1; }
            } else {
                const char *p = s;
                while (1) {
                    const char *found = strstr(p, sep);
                    if (!found) { items[idx++] = xly_str(p); break; }
                    size_t chunk = (size_t)(found - p);
                    char *buf = (char*)malloc(chunk + 1);
                    memcpy(buf, p, chunk); buf[chunk] = '\0';
                    items[idx++] = xly_str(buf);
                    free(buf);
                    p = found + sep_len;
                }
            }
            XlyVal *r = xly_array_create(items, idx);
            free(items);
            return r;
        }
        if (strcmp(fn, "join") == 0) {
            if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY) return xly_str("");
            const char *sep = (argc >= 2 && args[1]->type == VAL_STRING) ? args[1]->str : ",";
            XlyVal *arr = args[0];
            size_t cap = 128, pos = 0;
            char *buf = (char*)malloc(cap);
            buf[0] = '\0';
            for (size_t k = 0; k < arr->array_len; k++) {
                if (k > 0) {
                    size_t sl = strlen(sep);
                    while (pos + sl + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
                    memcpy(buf + pos, sep, sl); pos += sl;
                }
                char *elem = xly_to_cstr(arr->array[k]);
                size_t el = strlen(elem);
                while (pos + el + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
                memcpy(buf + pos, elem, el); pos += el;
                free(elem);
            }
            buf[pos] = '\0';
            XlyVal *r = xly_str(buf); free(buf); return r;
        }
        if (strcmp(fn, "unicodeLength") == 0)
            return xly_utf8_len(a0);
        if (strcmp(fn, "unicodeCharAt") == 0)
            return xly_utf8_char_at(a0, argc >= 2 ? args[1] : NULL);
        if (strcmp(fn, "codePointAt") == 0)
            return xly_uni_codepoint_at(a0, argc >= 2 ? args[1] : NULL);
        if (strcmp(fn, "fromCodePoint") == 0)
            return xly_uni_from_codepoint(argc >= 1 ? args[0] : NULL);
        if (strcmp(fn, "normalize") == 0)
            return a0_is_str ? xly_str(a0->str) : xly_str("");
    }

    if (strcmp(mod, "math") == 0) {
        XlyVal *a0 = argc >= 1 ? args[0] : NULL;
        XlyVal *a1 = argc >= 2 ? args[1] : NULL;
        double  n0 = a0 ? a0->num : 0.0;
        double  n1 = a1 ? a1->num : 0.0;

        if (strcmp(fn, "abs") == 0)   return xly_num(fabs(n0));
        if (strcmp(fn, "sqrt") == 0)  return xly_num(sqrt(n0));
        if (strcmp(fn, "cbrt") == 0)  return xly_num(cbrt(n0));
        if (strcmp(fn, "pow") == 0)   return xly_num(pow(n0, n1));
        if (strcmp(fn, "hypot") == 0) return xly_num(hypot(n0, n1));
        if (strcmp(fn, "sign") == 0)  return xly_num(n0 > 0 ? 1.0 : (n0 < 0 ? -1.0 : 0.0));
        if (strcmp(fn, "fmod") == 0)  return xly_num(fmod(n0, n1));
        if (strcmp(fn, "clamp") == 0) {
            double lo = a1 ? a1->num : 0.0, hi = (argc >= 3 && args[2]) ? args[2]->num : 0.0;
            double v = n0;
            if (v < lo) v = lo;
            if (v > hi) v = hi;
            return xly_num(v);
        }
        if (strcmp(fn, "exp") == 0)    return xly_num(exp(n0));
        if (strcmp(fn, "floor") == 0)  return xly_num(floor(n0));
        if (strcmp(fn, "ceil") == 0)   return xly_num(ceil(n0));
        if (strcmp(fn, "round") == 0)  return xly_num(round(n0));
        if (strcmp(fn, "trunc") == 0)  return xly_num(trunc(n0));
        if (strcmp(fn, "max") == 0)    return xly_num(n0 > n1 ? n0 : n1);
        if (strcmp(fn, "min") == 0)    return xly_num(n0 < n1 ? n0 : n1);
        if (strcmp(fn, "sin") == 0)    return xly_num(sin(n0));
        if (strcmp(fn, "cos") == 0)    return xly_num(cos(n0));
        if (strcmp(fn, "tan") == 0)    return xly_num(tan(n0));
        if (strcmp(fn, "asin") == 0)   return xly_num(asin(n0));
        if (strcmp(fn, "acos") == 0)   return xly_num(acos(n0));
        if (strcmp(fn, "atan") == 0)   return xly_num(atan(n0));
        if (strcmp(fn, "atan2") == 0)  return xly_num(atan2(n0, n1));
        if (strcmp(fn, "log") == 0)    return xly_num(log(n0));
        if (strcmp(fn, "log2") == 0)   return xly_num(log2(n0));
        if (strcmp(fn, "log10") == 0)  return xly_num(log10(n0));
        if (strcmp(fn, "random") == 0) {
            static int seeded = 0;
            if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
            return xly_num((double)rand() / (double)RAND_MAX);
        }
        if (strcmp(fn, "randomInt") == 0) {
            static int seeded = 0;
            if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
            int lo = (int)n0, hi = (int)n1;
            if (hi <= lo) return xly_num((double)lo);
            return xly_num((double)(lo + rand() % (hi - lo)));
        }
        if (strcmp(fn, "PI") == 0)   return xly_num(M_PI);
        if (strcmp(fn, "E") == 0)    return xly_num(M_E);
        if (strcmp(fn, "INF") == 0)  return xly_num(HUGE_VAL);
        if (strcmp(fn, "NAN") == 0)  return xly_num(NAN);
        if (strcmp(fn, "isNaN") == 0)    return xly_bool(isnan(n0));
        if (strcmp(fn, "isInf") == 0)    return xly_bool(isinf(n0));
        if (strcmp(fn, "isFinite") == 0) return xly_bool(isfinite(n0));
        if (strcmp(fn, "complex") == 0) {
            XlyVal **elems = (XlyVal**)malloc(sizeof(XlyVal*) * 2);
            elems[0] = xly_num(n0);
            elems[1] = xly_num(argc >= 2 ? n1 : 0.0);
            XlyVal *r = xly_array_create(elems, 2);
            free(elems);
            return r;
        }
        if (strcmp(fn, "complexAdd") == 0 || strcmp(fn, "complexMul") == 0) {
            if (argc < 2 || !a0 || !a1 || a0->type != VAL_ARRAY || a1->type != VAL_ARRAY ||
                a0->array_len < 2 || a1->array_len < 2) return xly_null();
            double r1 = a0->array[0]->num, i1 = a0->array[1]->num;
            double r2 = a1->array[0]->num, i2 = a1->array[1]->num;
            XlyVal **elems = (XlyVal**)malloc(sizeof(XlyVal*) * 2);
            if (strcmp(fn, "complexAdd") == 0) {
                elems[0] = xly_num(r1 + r2);
                elems[1] = xly_num(i1 + i2);
            } else {
                elems[0] = xly_num(r1 * r2 - i1 * i2);
                elems[1] = xly_num(r1 * i2 + i1 * r2);
            }
            XlyVal *r = xly_array_create(elems, 2);
            free(elems);
            return r;
        }
        if (strcmp(fn, "complexAbs") == 0) {
            if (!a0 || a0->type != VAL_ARRAY || a0->array_len < 2) return xly_num(0);
            double re = a0->array[0]->num, im = a0->array[1]->num;
            return xly_num(sqrt(re*re + im*im));
        }
        if (strcmp(fn, "complexConj") == 0) {
            if (!a0 || a0->type != VAL_ARRAY || a0->array_len < 2) return xly_null();
            XlyVal **elems = (XlyVal**)malloc(sizeof(XlyVal*) * 2);
            elems[0] = xly_num(a0->array[0]->num);
            elems[1] = xly_num(-a0->array[1]->num);
            XlyVal *r = xly_array_create(elems, 2);
            free(elems);
            return r;
        }
        if (strcmp(fn, "complexPhase") == 0) {
            if (!a0 || a0->type != VAL_ARRAY || a0->array_len < 2) return xly_num(0);
            return xly_num(atan2(a0->array[1]->num, a0->array[0]->num));
        }
        if (strcmp(fn, "sum") == 0 || strcmp(fn, "product") == 0 || strcmp(fn, "mean") == 0) {
            int is_sum = strcmp(fn, "sum") == 0, is_mean = strcmp(fn, "mean") == 0;
            double total = is_sum || is_mean ? 0.0 : 1.0;
            size_t count = 0;
            if (a0 && a0->type == VAL_ARRAY) {
                for (size_t k = 0; k < a0->array_len; k++)
                    if (a0->array[k]->type == VAL_NUMBER) {
                        if (is_sum || is_mean) total += a0->array[k]->num; else total *= a0->array[k]->num;
                        count++;
                    }
            } else {
                for (size_t k = 0; k < argc; k++)
                    if (args[k]->type == VAL_NUMBER) {
                        if (is_sum || is_mean) total += args[k]->num; else total *= args[k]->num;
                        count++;
                    }
            }
            if (is_mean) return xly_num(count > 0 ? total / (double)count : 0.0);
            return xly_num(total);
        }
        if (strcmp(fn, "median") == 0 || strcmp(fn, "variance") == 0 || strcmp(fn, "stddev") == 0) {
            if (!a0 || a0->type != VAL_ARRAY || a0->array_len == 0) return xly_num(0);
            double *nums = (double*)malloc(sizeof(double) * a0->array_len);
            size_t count = 0;
            for (size_t k = 0; k < a0->array_len; k++)
                if (a0->array[k]->type == VAL_NUMBER) nums[count++] = a0->array[k]->num;
            if (count == 0) { free(nums); return xly_num(0); }
            if (strcmp(fn, "median") == 0) {
                for (size_t i = 0; i < count - 1; i++)
                    for (size_t j = 0; j < count - i - 1; j++)
                        if (nums[j] > nums[j+1]) { double t = nums[j]; nums[j] = nums[j+1]; nums[j+1] = t; }
                double result = (count % 2 == 0) ? (nums[count/2-1] + nums[count/2]) / 2.0 : nums[count/2];
                free(nums);
                return xly_num(result);
            }
            double mean = 0;
            for (size_t k = 0; k < count; k++) mean += nums[k];
            mean /= (double)count;
            double var = 0;
            for (size_t k = 0; k < count; k++) { double d = nums[k] - mean; var += d * d; }
            var /= (double)count;
            free(nums);
            return xly_num(strcmp(fn, "stddev") == 0 ? sqrt(var) : var);
        }
        if (strcmp(fn, "gcd") == 0) {
            long long a = llabs((long long)n0), b = llabs((long long)n1);
            while (b != 0) { long long t = b; b = a % b; a = t; }
            return xly_num((double)a);
        }
        if (strcmp(fn, "lcm") == 0) {
            long long a = (long long)n0, b = (long long)n1;
            if (a == 0 || b == 0) return xly_num(0);
            long long x = llabs(a), y = llabs(b), g = x, h = y;
            while (h != 0) { long long t = h; h = g % h; g = t; }
            return xly_num((double)((x / g) * y));
        }
        if (strcmp(fn, "factorial") == 0) {
            int n = (int)n0;
            if (n < 0) return xly_num(NAN);
            if (n > 170) return xly_num(INFINITY);
            double result = 1;
            for (int i = 2; i <= n; i++) result *= i;
            return xly_num(result);
        }
        if (strcmp(fn, "combinations") == 0 || strcmp(fn, "permutations") == 0) {
            int n = (int)n0, k = (int)n1;
            if (k > n || k < 0 || n < 0) return xly_num(0);
            if (strcmp(fn, "permutations") == 0) {
                double result = 1;
                for (int i = 0; i < k; i++) result *= (n - i);
                return xly_num(result);
            }
            if (k == 0 || k == n) return xly_num(1);
            int kk = (k > n - k) ? n - k : k;
            double result = 1;
            for (int i = 0; i < kk; i++) { result *= (n - i); result /= (i + 1); }
            return xly_num(result);
        }
        if (strcmp(fn, "lerp") == 0) {
            double b = a1 ? a1->num : 0.0, t = (argc >= 3 && args[2]) ? args[2]->num : 0.0;
            return xly_num(n0 + (b - n0) * t);
        }
        if (strcmp(fn, "degrees") == 0) return xly_num(n0 * 180.0 / M_PI);
        if (strcmp(fn, "radians") == 0) return xly_num(n0 * M_PI / 180.0);
    }

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
        /* Index by codepoint (Unicode-aware), not byte */
        size_t clen = utf8_strlen(collection->str);
        if (idx < 0) idx = (int)clen + idx;
        if (idx < 0 || (size_t)idx >= clen) return xly_null();
        char buf[UTF8_MAX_BYTES + 1];
        utf8_slice(collection->str, (size_t)idx, (size_t)idx + 1,
                   buf, sizeof(buf));
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

/* ══════════════════════════════════════════════════════════════════════════════
 * UNICODE / UTF-8 RUNTIME — xly_rt.c
 *
 * These functions expose the full unicode.h API through the XlyVal* ABI.
 * Every compiled Xenly binary links xly_rt + unicode and can call all of
 * these without any C interop.
 *
 * Convention for "codepoint" arguments:
 *   - If the argument is VAL_NUMBER its value is the codepoint (uint32_t).
 *   - If the argument is VAL_STRING its first codepoint is used.
 * ══════════════════════════════════════════════════════════════════════════════ */

/* Internal helper: extract a codepoint from a number or 1-char string */
static uint32_t xly_to_cp(XlyVal *v) {
    if (!v) return 0;
    if (v->type == VAL_NUMBER) return (uint32_t)(unsigned long long)v->num;
    if (v->type == VAL_STRING && v->str && v->str[0]) {
        size_t bytes;
        return utf8_decode(v->str, &bytes);
    }
    return 0;
}

/* ── UTF-8 string length / indexing ──────────────────────────────────────── */

XlyVal *xly_utf8_len(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)utf8_strlen(s->str));
}

XlyVal *xly_utf8_char_at(XlyVal *s, XlyVal *index) {
    if (!s || s->type != VAL_STRING || !index) return xly_str("");
    size_t idx = (size_t)(long long)index->num;
    char buf[UTF8_MAX_BYTES + 1];
    utf8_slice(s->str, idx, idx + 1, buf, sizeof(buf));
    return xly_str(buf);
}

XlyVal *xly_utf8_slice(XlyVal *s, XlyVal *start, XlyVal *end) {
    /* Re-use the updated xly_str_slice which is now codepoint-aware */
    return xly_str_slice(s, start, end);
}

XlyVal *xly_utf8_byte_offset(XlyVal *s, XlyVal *index) {
    if (!s || s->type != VAL_STRING || !index) return xly_num(0);
    size_t idx = (size_t)(long long)index->num;
    return xly_num((double)utf8_index_byte(s->str, idx));
}

XlyVal *xly_utf8_is_valid(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_bool(0);
    return xly_bool(utf8_is_valid(s->str));
}

XlyVal *xly_utf8_sanitize(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_str("");
    size_t src_len = strlen(s->str);
    size_t buf_size = src_len * 3 + 1;
    char *buf = (char *)malloc(buf_size);
    if (!buf) return xly_str("");
    utf8_sanitize(s->str, buf, buf_size);
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

XlyVal *xly_utf8_reverse(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_str("");
    size_t src_len = strlen(s->str);
    char *buf = (char *)malloc(src_len + 1);
    if (!buf) return xly_str("");
    utf8_reverse(s->str, buf, src_len + 1);
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

XlyVal *xly_utf8_display_width(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)utf8_display_width(s->str));
}

XlyVal *xly_utf8_contains_rune(XlyVal *s, XlyVal *cp_val) {
    if (!s || s->type != VAL_STRING) return xly_bool(0);
    uint32_t cp = xly_to_cp(cp_val);
    return xly_bool(utf8_contains_rune(s->str, cp));
}

XlyVal *xly_utf8_count_rune(XlyVal *s, XlyVal *cp_val) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    uint32_t cp = xly_to_cp(cp_val);
    return xly_num((double)utf8_count_rune(s->str, cp));
}

XlyVal *xly_utf8_index_rune(XlyVal *s, XlyVal *cp_val) {
    if (!s || s->type != VAL_STRING) return xly_num(-1);
    uint32_t cp = xly_to_cp(cp_val);
    size_t off = utf8_index_rune(s->str, cp);
    return xly_num(off == (size_t)-1 ? -1.0 : (double)off);
}

XlyVal *xly_utf8_equal_fold(XlyVal *a, XlyVal *b) {
    if (!a || a->type != VAL_STRING || !b || b->type != VAL_STRING)
        return xly_bool(0);
    return xly_bool(utf8_equal_fold(a->str, b->str));
}

/* ── Unicode codepoint operations ────────────────────────────────────────── */

XlyVal *xly_uni_codepoint_at(XlyVal *s, XlyVal *index) {
    if (!s || s->type != VAL_STRING || !index) return xly_num(0);
    size_t idx = (size_t)(long long)index->num;
    const char *pos = utf8_char_at(s->str, idx);
    if (!pos || !*pos) return xly_num(0);
    size_t bytes;
    uint32_t cp = utf8_decode(pos, &bytes);
    return xly_num((double)cp);
}

XlyVal *xly_uni_from_codepoint(XlyVal *cp_val) {
    uint32_t cp = xly_to_cp(cp_val);
    char buf[UTF8_MAX_BYTES + 1] = {0};
    size_t n = utf8_encode(cp, buf);
    if (n == 0) return xly_str("");
    buf[n] = '\0';
    return xly_str(buf);
}

/* ── Unicode character classification ────────────────────────────────────── */

XlyVal *xly_uni_is_letter (XlyVal *v) { return xly_bool(unicode_is_letter (xly_to_cp(v))); }
XlyVal *xly_uni_is_upper  (XlyVal *v) { return xly_bool(unicode_is_upper  (xly_to_cp(v))); }
XlyVal *xly_uni_is_lower  (XlyVal *v) { return xly_bool(unicode_is_lower  (xly_to_cp(v))); }
XlyVal *xly_uni_is_title  (XlyVal *v) { return xly_bool(unicode_is_title  (xly_to_cp(v))); }
XlyVal *xly_uni_is_digit  (XlyVal *v) { return xly_bool(unicode_is_digit  (xly_to_cp(v))); }
XlyVal *xly_uni_is_number (XlyVal *v) { return xly_bool(unicode_is_number (xly_to_cp(v))); }
XlyVal *xly_uni_is_space  (XlyVal *v) { return xly_bool(unicode_is_space  (xly_to_cp(v))); }
XlyVal *xly_uni_is_punct  (XlyVal *v) { return xly_bool(unicode_is_punct  (xly_to_cp(v))); }
XlyVal *xly_uni_is_symbol (XlyVal *v) { return xly_bool(unicode_is_symbol (xly_to_cp(v))); }
XlyVal *xly_uni_is_mark   (XlyVal *v) { return xly_bool(unicode_is_mark   (xly_to_cp(v))); }
XlyVal *xly_uni_is_control(XlyVal *v) { return xly_bool(unicode_is_control(xly_to_cp(v))); }
XlyVal *xly_uni_is_graphic(XlyVal *v) { return xly_bool(unicode_is_graphic(xly_to_cp(v))); }
XlyVal *xly_uni_is_print  (XlyVal *v) { return xly_bool(unicode_is_print  (xly_to_cp(v))); }

XlyVal *xly_uni_digit_value(XlyVal *v) {
    return xly_num((double)unicode_digit_value(xly_to_cp(v)));
}

/* ── Case conversion ─────────────────────────────────────────────────────── */

XlyVal *xly_uni_to_upper(XlyVal *v) {
    return xly_num((double)unicode_to_upper(xly_to_cp(v)));
}
XlyVal *xly_uni_to_lower(XlyVal *v) {
    return xly_num((double)unicode_to_lower(xly_to_cp(v)));
}
XlyVal *xly_uni_to_title(XlyVal *v) {
    return xly_num((double)unicode_to_title(xly_to_cp(v)));
}
XlyVal *xly_uni_simple_fold(XlyVal *v) {
    return xly_num((double)unicode_simple_fold(xly_to_cp(v)));
}

/* String-level case conversion */

static XlyVal *_utf8_case(XlyVal *s,
    size_t (*fn)(const char *, char *, size_t)) {
    if (!s || s->type != VAL_STRING) return xly_str("");
    size_t src_len = strlen(s->str);
    /* Worst case: each byte expands to 3 (ß→SS is 2 codepoints; handled by to_upper) */
    size_t buf_size = src_len * 3 + 1;
    char *buf = (char *)malloc(buf_size);
    if (!buf) return xly_str("");
    fn(s->str, buf, buf_size);
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

XlyVal *xly_utf8_to_upper(XlyVal *s) { return _utf8_case(s, utf8_to_upper); }
XlyVal *xly_utf8_to_lower(XlyVal *s) { return _utf8_case(s, utf8_to_lower); }
XlyVal *xly_utf8_to_title(XlyVal *s) { return _utf8_case(s, utf8_to_title); }

/* ── Grapheme clusters ───────────────────────────────────────────────────── */

XlyVal *xly_grapheme_count(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)grapheme_count(s->str));
}

XlyVal *xly_grapheme_at(XlyVal *s, XlyVal *index) {
    if (!s || s->type != VAL_STRING || !index) return xly_str("");
    size_t idx = (size_t)(long long)index->num;
    const char *pos = grapheme_cluster_at(s->str, idx);
    if (!pos || !*pos) return xly_str("");
    size_t len = grapheme_next_break(pos);
    if (len == 0) return xly_str("");
    char *buf = (char *)malloc(len + 1);
    if (!buf) return xly_str("");
    memcpy(buf, pos, len);
    buf[len] = '\0';
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

XlyVal *xly_grapheme_next_break(XlyVal *s) {
    if (!s || s->type != VAL_STRING) return xly_num(0);
    return xly_num((double)grapheme_next_break(s->str));
}

/* ── Normalization ───────────────────────────────────────────────────────── */

static XlyVal *_utf8_norm(XlyVal *s,
    size_t (*fn)(const char *, char *, size_t)) {
    if (!s || s->type != VAL_STRING) return xly_str("");
    size_t src_len = strlen(s->str);
    size_t buf_size = src_len * 4 + 1;
    char *buf = (char *)malloc(buf_size);
    if (!buf) return xly_str("");
    fn(s->str, buf, buf_size);
    XlyVal *r = xly_str(buf);
    free(buf);
    return r;
}

XlyVal *xly_utf8_nfc(XlyVal *s) { return _utf8_norm(s, utf8_nfc); }
XlyVal *xly_utf8_nfd(XlyVal *s) { return _utf8_norm(s, utf8_nfd); }

/* ── UTF-16 interop ──────────────────────────────────────────────────────── */

XlyVal *xly_utf16_encode(XlyVal *cp_val) {
    uint32_t cp = xly_to_cp(cp_val);
    if (cp <= 0xFFFF && !utf16_is_surrogate(cp)) {
        XlyVal *elem = xly_num((double)cp);
        return xly_array_create(&elem, 1);
    }
    uint16_t high, low;
    if (!utf16_encode_pair(cp, &high, &low)) {
        XlyVal *elem = xly_num((double)cp);
        return xly_array_create(&elem, 1);
    }
    XlyVal *elems[2];
    elems[0] = xly_num((double)high);
    elems[1] = xly_num((double)low);
    return xly_array_create(elems, 2);
}

XlyVal *xly_utf16_decode(XlyVal *high_val, XlyVal *low_val) {
    uint16_t high = (uint16_t)(unsigned)xly_to_cp(high_val);
    uint16_t low  = (uint16_t)(unsigned)xly_to_cp(low_val);
    uint32_t cp = utf16_decode_pair(high, low);
    return xly_num((double)cp);
}

XlyVal *xly_uni_is_surrogate(XlyVal *v) {
    return xly_bool(utf16_is_surrogate(xly_to_cp(v)));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REFLECT RUNTIME HELPERS
 *
 * In xly_rt.c the XlyVal.instance field is void* — the interpreter's
 * InstanceData layout is not directly accessible.  We use the interpreter's
 * public obj API (xly_obj_get / xly_obj_set / xly_obj_new) which are
 * already implemented in this file and work through the same void* handle.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Helper: iterate object keys via the interpreter-level obj store.
 * We use the xly_array + xly_obj_get pattern: list all known keys by
 * walking the internal env pointer through the void* handle.
 * Since we can't dereference instance->fields directly, we call
 * xly_call_module("reflect","keys",…) — but that would be circular.
 * Instead, expose a thin wrapper that calls back into the interpreter
 * via xly_call_module which IS available here.                        */
XlyVal *xly_reflect_keys(XlyVal *obj) {
    if (!obj) return xly_array_create(NULL, 0);
    XlyVal *args[1] = { obj };
    return xly_call_module("reflect", "keys", args, 1);
}

XlyVal *xly_reflect_get(XlyVal *obj, XlyVal *key) {
    if (!obj || !key) return xly_null();
    XlyVal *args[2] = { obj, key };
    return xly_call_module("reflect", "get", args, 2);
}

XlyVal *xly_reflect_set(XlyVal *obj, XlyVal *key, XlyVal *val) {
    if (!obj || !key || !val) return xly_bool(0);
    XlyVal *args[3] = { obj, key, val };
    return xly_call_module("reflect", "set", args, 3);
}

XlyVal *xly_reflect_has(XlyVal *obj, XlyVal *key) {
    if (!obj || !key) return xly_bool(0);
    XlyVal *args[2] = { obj, key };
    return xly_call_module("reflect", "has", args, 2);
}

XlyVal *xly_reflect_delete(XlyVal *obj, XlyVal *key) {
    if (!obj || !key) return xly_bool(0);
    XlyVal *args[2] = { obj, key };
    return xly_call_module("reflect", "delete", args, 2);
}

XlyVal *xly_reflect_freeze(XlyVal *obj) {
    if (!obj) return xly_null();
    /* Set the frozen sentinel directly — local is accessible in xly_rt.c */
    obj->local = 99;
    return obj;
}

XlyVal *xly_reflect_is_frozen(XlyVal *obj) {
    return xly_bool(obj && obj->local == 99);
}

XlyVal *xly_reflect_define(XlyVal *obj, XlyVal *key, XlyVal *desc) {
    if (!obj || !key) return xly_null();
    XlyVal *args[3] = { obj, key, desc };
    return xly_call_module("reflect", "define", args, 3);
}

XlyVal *xly_reflect_apply(XlyVal *fn, XlyVal *args_arr) {
    if (!fn) return xly_null();
    size_t argc = (args_arr && args_arr->type == VAL_ARRAY)
                ? args_arr->array_len : 0;
    XlyVal **argv = argc ? args_arr->array : NULL;
    return xly_call_fnval(fn, argv, (int)argc);
}

XlyVal *xly_reflect_construct(XlyVal *cls, XlyVal *args_arr) {
    if (!cls) return xly_null();
    XlyVal *args[2] = { cls, args_arr ? args_arr : xly_array_create(NULL,0) };
    return xly_call_module("reflect", "construct", args, 2);
}

XlyVal *xly_index_set(XlyVal *collection, XlyVal *index, XlyVal *val) {
    if (!collection || !index || !val) return xly_null();
    /* For arrays with numeric index */
    if (collection->type == VAL_ARRAY && index->type == VAL_NUMBER) {
        size_t idx = (size_t)(long long)index->num;
        if (idx < collection->array_len)
            collection->array[idx] = val;
        return val;
    }
    /* For instances with string key — via reflect_set */
    return xly_reflect_set(collection, index, val);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GENERATOR RUNTIME HELPERS
 *
 * Native generator bodies compiled by xenlyc call xly_gen_yield() to
 * suspend, and the iterator's .next() calls through xly_for_of_next().
 *
 * Thread-local yield state: _gen_yield_val / _gen_yielded.
 * On macOS this is safe for single-threaded generator use.  Multi-threaded
 * generator use requires per-generator state (future work).
 * ═══════════════════════════════════════════════════════════════════════════ */

#if defined(__GNUC__) || defined(__clang__)
#  define XLY_THREAD_LOCAL __thread
#else
#  define XLY_THREAD_LOCAL
#endif

static XLY_THREAD_LOCAL XlyVal *_gen_yield_val = NULL;
static XLY_THREAD_LOCAL int     _gen_yielded   = 0;

XlyVal *xly_gen_create(void *fn_ptr, void *env_ptr) {
    (void)env_ptr;
    XlyVal *iter = xly_obj_new();
    xly_obj_set(iter, "done",       xly_bool(0));
    xly_obj_set(iter, "value",      xly_null());
    xly_obj_set(iter, "__gen_fn__", xly_num((double)(uintptr_t)fn_ptr));
    return iter;
}

void xly_gen_yield(XlyVal *val) {
    _gen_yield_val = val;
    _gen_yielded   = 1;
    /* The generator body returns here; xly_for_of_next detects the yield */
}

XlyVal *xly_for_of_next(XlyVal *iter) {
    if (!iter) return NULL;

    /* Check done flag */
    XlyVal *done = xly_obj_get(iter, "done");
    if (done && xly_truthy(done)) return NULL;

    /* Get generator body function pointer */
    XlyVal *fn_val = xly_obj_get(iter, "__gen_fn__");
    if (!fn_val || fn_val->type != VAL_NUMBER) return NULL;

    typedef void (*GenBodyFn)(XlyVal *);
    GenBodyFn body = (GenBodyFn)(uintptr_t)(unsigned long long)(fn_val->num);

    _gen_yielded   = 0;
    _gen_yield_val = NULL;
    body(iter);   /* body calls xly_gen_yield() then returns */

    if (!_gen_yielded) {
        xly_obj_set(iter, "done",  xly_bool(1));
        xly_obj_set(iter, "value", xly_null());
        return NULL;
    }

    XlyVal *yval = _gen_yield_val ? _gen_yield_val : xly_null();
    xly_obj_set(iter, "value", yval);
    return yval;
}
