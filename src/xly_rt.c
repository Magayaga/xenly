/*
 * xly_rt.c  —  Xenly native-compiler runtime library
 *
 * Compiled Xenly binaries call into this.  It is compiled once into
 * libxly_rt.a and linked into every xenlyc output.
 *
 * The tagged-value layout mirrors the interpreter's Value struct so that
 * the existing modules.c (100 stdlib functions) can be linked in directly
 * and called through xly_call_module().
 */

#include "xly_rt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

/* ── internal struct layout ─────────────────────────────────────────────
 * Must match interpreter.h  Value / ValueType exactly so that modules.c
 * functions (which take Value**) work unchanged.                          */

typedef enum {
    VAL_NUMBER   = 0,
    VAL_STRING   = 1,
    VAL_BOOL     = 2,
    VAL_NULL     = 3,
    VAL_FUNCTION = 4,
    VAL_RETURN   = 5,
    VAL_CLASS    = 6,
    VAL_INSTANCE = 7,
    VAL_ARRAY    = 8,
    VAL_BREAK    = 9,
    VAL_CONTINUE = 10,
} ValType;

/* Forward-declare the fields that modules.c touches.  We keep the struct
 * layout byte-for-byte identical to interpreter.h's Value.               */
struct XlyVal {
    ValType   type;
    double    num;
    char     *str;
    int       boolean;
    int       local;
    void     *fn;          /* FnDef* — unused in compiled code */
    struct XlyVal *inner;  /* VAL_RETURN wrapper — unused      */
    void     *class_def;   /* ClassDef* — unused               */
    void     *instance;    /* InstanceData* — unused           */
    struct XlyVal **array; /* VAL_ARRAY elements               */
    size_t    array_len;
    size_t    array_cap;
};

/* ── constructors ───────────────────────────────────────────────────── */
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

/* ── truthiness ─────────────────────────────────────────────────────── */
int xly_truthy(XlyVal *v) {
    if (!v) return 0;
    switch (v->type) {
        case VAL_NULL:   return 0;
        case VAL_BOOL:   return v->boolean;
        case VAL_NUMBER: return v->num != 0.0;
        case VAL_STRING: return strlen(v->str) > 0;
        default:         return 1;
    }
}

/* ── string conversion ─────────────────────────────────────────────── */
char *xly_to_cstr(XlyVal *v) {
    if (!v) return strdup("null");
    char buf[256];
    switch (v->type) {
        case VAL_NUMBER:
            if (v->num == (double)(long long)v->num && fabs(v->num) < 1e15)
                snprintf(buf, sizeof(buf), "%lld", (long long)v->num);
            else
                snprintf(buf, sizeof(buf), "%g", v->num);
            return strdup(buf);
        case VAL_STRING:  return strdup(v->str);
        case VAL_BOOL:    return strdup(v->boolean ? "true" : "false");
        case VAL_NULL:    return strdup("null");
        case VAL_ARRAY: {
            /* "[e0, e1, ...]" */
            size_t cap = 64;
            char  *out = (char*)malloc(cap);
            size_t pos = 0;
            pos += snprintf(out+pos, cap-pos, "[");
            for (size_t i = 0; i < v->array_len; i++) {
                if (i) pos += snprintf(out+pos, cap-pos, ", ");
                char *es = xly_to_cstr(v->array[i]);
                size_t elen = strlen(es);
                /* quote strings inside arrays */
                int is_str = (v->array[i] && v->array[i]->type == VAL_STRING);
                size_t need = elen + (is_str ? 2 : 0);
                while (pos + need + 8 >= cap) { cap *= 2; out = (char*)realloc(out, cap); }
                if (is_str) pos += snprintf(out+pos, cap-pos, "\"%s\"", es);
                else        pos += snprintf(out+pos, cap-pos, "%s", es);
                free(es);
            }
            pos += snprintf(out+pos, cap-pos, "]");
            return out;
        }
        default:          return strdup("<object>");
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

/* ── arithmetic ─────────────────────────────────────────────────────── */
XlyVal *xly_add(XlyVal *a, XlyVal *b) {
    /* string concat if either side is a string */
    if (a->type == VAL_STRING || b->type == VAL_STRING) {
        char *sa = xly_to_cstr(a);
        char *sb = xly_to_cstr(b);
        size_t la = strlen(sa), lb = strlen(sb);
        char  *cat = (char*)malloc(la + lb + 1);
        memcpy(cat, sa, la);
        memcpy(cat+la, sb, lb);
        cat[la+lb] = '\0';
        free(sa); free(sb);
        XlyVal *r = xly_str(cat);
        free(cat);
        return r;
    }
    return xly_num(a->num + b->num);
}

XlyVal *xly_sub(XlyVal *a, XlyVal *b) { return xly_num(a->num - b->num); }
XlyVal *xly_mul(XlyVal *a, XlyVal *b) { return xly_num(a->num * b->num); }
XlyVal *xly_div(XlyVal *a, XlyVal *b) { return xly_num(a->num / b->num); }
XlyVal *xly_mod(XlyVal *a, XlyVal *b) { return xly_num(fmod(a->num, b->num)); }
XlyVal *xly_neg(XlyVal *a)            { return xly_num(-a->num); }
XlyVal *xly_not(XlyVal *a)            { return xly_bool(!xly_truthy(a)); }

/* ── comparison ─────────────────────────────────────────────────────── */
static int vals_equal(XlyVal *a, XlyVal *b) {
    if (a->type != b->type) return 0;
    switch (a->type) {
        case VAL_NUMBER: return a->num == b->num;
        case VAL_STRING: return strcmp(a->str, b->str) == 0;
        case VAL_BOOL:   return a->boolean == b->boolean;
        case VAL_NULL:   return 1;
        default:         return a == b;  /* pointer equality for objects */
    }
}

XlyVal *xly_eq(XlyVal *a, XlyVal *b)  { return xly_bool(vals_equal(a, b)); }
XlyVal *xly_neq(XlyVal *a, XlyVal *b) { return xly_bool(!vals_equal(a, b)); }
XlyVal *xly_lt(XlyVal *a, XlyVal *b)  { return xly_bool(a->num < b->num); }
XlyVal *xly_gt(XlyVal *a, XlyVal *b)  { return xly_bool(a->num > b->num); }
XlyVal *xly_lte(XlyVal *a, XlyVal *b) { return xly_bool(a->num <= b->num); }
XlyVal *xly_gte(XlyVal *a, XlyVal *b) { return xly_bool(a->num >= b->num); }

/* ── I/O ────────────────────────────────────────────────────────────── */
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
    /* strip trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return xly_str(buf);
}

/* ── array helpers ──────────────────────────────────────────────────── */
XlyVal *xly_array_create(XlyVal **elems, size_t n) {
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type      = VAL_ARRAY;
    v->array_len = n;
    v->array_cap = n ? n : 4;
    v->array     = (XlyVal**)malloc(sizeof(XlyVal*) * v->array_cap);
    if (n) memcpy(v->array, elems, sizeof(XlyVal*) * n);
    return v;
}

size_t  xly_array_len(XlyVal *arr)            { return arr->array_len; }
XlyVal *xly_array_get(XlyVal *arr, size_t i)  { return arr->array[i]; }
void    xly_array_set(XlyVal *arr, size_t i, XlyVal *val) { arr->array[i] = val; }

XlyVal *xly_array_push(XlyVal *arr, XlyVal *val) {
    if (arr->array_len >= arr->array_cap) {
        arr->array_cap = arr->array_cap ? arr->array_cap * 2 : 4;
        arr->array = (XlyVal**)realloc(arr->array, sizeof(XlyVal*)*arr->array_cap);
    }
    arr->array[arr->array_len++] = val;
    return arr;
}

/* ── module dispatch ────────────────────────────────────────────────── */
/*
 * We link against modules.c.  Its Module structs use the interpreter's
 * Value* type which is layout-compatible with XlyVal*.  We just need to
 * call modules_get() and walk the function table.
 *
 * modules_get is declared in modules.h but we don't include that header
 * here to avoid pulling in interpreter.h.  Forward-declare the minimal
 * interface we need.
 */

/* Mirror the interpreter's Module / NativeFunc structs exactly. */
typedef XlyVal* (*NativeFn)(XlyVal **args, size_t argc);
typedef struct { const char *name; NativeFn fn; } NFunc;
typedef struct { const char *name; NFunc *functions; size_t fn_count; } Mod;

/* modules_get is defined in modules.c.  Its second arg is a Module* but
 * our Mod is layout-compatible.  We cast through void*. */
extern int modules_get(const char *name, void *out);

XlyVal *xly_call_module(const char *mod, const char *fn,
                         XlyVal **args, size_t argc) {
    Mod m;
    memset(&m, 0, sizeof(m));
    if (!modules_get(mod, &m)) {
        fprintf(stderr, "[xenly runtime] unknown module '%s'\n", mod);
        return xly_null();
    }
    for (size_t i = 0; i < m.fn_count; i++) {
        if (strcmp(m.functions[i].name, fn) == 0)
            return m.functions[i].fn(args, argc);
    }
    fprintf(stderr, "[xenly runtime] '%s' not found in module '%s'\n", fn, mod);
    return xly_null();
}

/* ── exit ───────────────────────────────────────────────────────────── */
void xly_exit(int code) { exit(code); }

/* ── interpreter-API compatibility shims ────────────────────────────────
 * modules.c calls value_number(), value_string(), etc.  Those symbols live
 * in interpreter.c in the interpreter build.  For the compiled-binary
 * runtime we provide them here, each delegating to the xly_* equivalent.  */

XlyVal *value_number(double n)        { return xly_num(n); }
XlyVal *value_string(const char *s)   { return xly_str(s); }
XlyVal *value_bool(int b)             { return xly_bool(b); }
XlyVal *value_null(void)              { return xly_null(); }

XlyVal *value_array(XlyVal **items, size_t len) {
    /* identical to xly_array_create but takes ownership of items (like
       the interpreter's value_array).  modules.c passes freshly-malloc'd
       arrays, so we just adopt them directly. */
    XlyVal *v = (XlyVal*)calloc(1, sizeof(XlyVal));
    v->type      = VAL_ARRAY;
    v->array     = items;
    v->array_len = len;
    v->array_cap = items ? (len ? len : 4) : 0;
    return v;
}

void value_destroy(XlyVal *v) {
    /* In the compiled runtime we don't bother freeing individual values;
       the process exit reclaims everything.  But modules.c calls this on
       temporary values (e.g. old array elements on set/fill), so we must
       not crash.  Free scalar types that own heap strings; skip everything
       else (arrays, functions, classes are long-lived). */
    if (!v) return;
    switch (v->type) {
        case VAL_STRING:
            free(v->str);
            free(v);
            break;
        case VAL_NUMBER:
        case VAL_BOOL:
        case VAL_NULL:
            free(v);
            break;
        default:
            /* shared / long-lived — leave alone */
            break;
    }
}

char *value_to_string(XlyVal *v) { return xly_to_cstr(v); }

XlyVal *value_clone(XlyVal *v) {
    if (!v) return xly_null();
    switch (v->type) {
        case VAL_NUMBER:   return xly_num(v->num);
        case VAL_STRING:   return xly_str(v->str);
        case VAL_BOOL:     return xly_bool(v->boolean);
        case VAL_NULL:     return xly_null();
        case VAL_ARRAY: {
            // FIX: xly_array_create sets cap = len ? len : 4.
            // Allocate at least that many slots so a push() after clone() doesn't
            // write past the end of the backing array.
            size_t alloc = v->array_len > 0 ? v->array_len : 4;
            XlyVal **items = (XlyVal **)malloc(sizeof(XlyVal *) * alloc);
            for (size_t i = 0; i < v->array_len; i++) items[i] = value_clone(v->array[i]);
            return xly_array_create(items, v->array_len);
        }
        default:           return v;  // functions, classes: shared ref
    }
}

/* ── array/string indexing ─────────────────────────────────────────── */
XlyVal *xly_index(XlyVal *collection, XlyVal *index_val) {
    if (!collection || !index_val) return xly_null();
    
    if (index_val->type != VAL_NUMBER) return xly_null();
    int index = (int)index_val->num;
    
    if (collection->type == VAL_ARRAY) {
        if (index < 0 || (size_t)index >= collection->array_len) {
            return xly_null();
        }
        return collection->array[index];
    }
    
    if (collection->type == VAL_STRING) {
        int len = (int)strlen(collection->str);
        if (index < 0 || index >= len) {
            return xly_null();
        }
        char buf[2] = {collection->str[index], '\0'};
        return xly_str(buf);
    }
    
    return xly_null();
}

