/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#define _GNU_SOURCE
#include "modules.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <float.h>
#include <unistd.h>

// ─── Registry ────────────────────────────────────────────────────────────────
int modules_get(const char *name, Module *out) {
    if (strcmp(name, "math")   == 0) { *out = module_math();   return 1; }
    if (strcmp(name, "string") == 0) { *out = module_string(); return 1; }
    if (strcmp(name, "io")     == 0) { *out = module_io();     return 1; }
    if (strcmp(name, "array")  == 0) { *out = module_array();  return 1; }
    if (strcmp(name, "os")     == 0) { *out = module_os();     return 1; }
    if (strcmp(name, "type")   == 0) { *out = module_type();   return 1; }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// MATH MODULE  — arithmetic, trig, rounding, constants, predicates
// ═════════════════════════════════════════════════════════════════════════════

static Value *math_abs(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(fabs(args[0]->num));
}
static Value *math_sqrt(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(sqrt(args[0]->num));
}
static Value *math_pow(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    return value_number(pow(args[0]->num, args[1]->num));
}
static Value *math_floor(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(floor(args[0]->num));
}
static Value *math_ceil(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(ceil(args[0]->num));
}
static Value *math_round(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(round(args[0]->num));
}
static Value *math_max(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    return value_number(args[0]->num > args[1]->num ? args[0]->num : args[1]->num);
}
static Value *math_min(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    return value_number(args[0]->num < args[1]->num ? args[0]->num : args[1]->num);
}
static Value *math_sin(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(sin(args[0]->num));
}
static Value *math_cos(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(cos(args[0]->num));
}
static Value *math_tan(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(tan(args[0]->num));
}
static Value *math_asin(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(asin(args[0]->num));
}
static Value *math_acos(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(acos(args[0]->num));
}
static Value *math_atan(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(atan(args[0]->num));
}
static Value *math_atan2(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    return value_number(atan2(args[0]->num, args[1]->num));
}
static Value *math_log(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(log(args[0]->num));
}
static Value *math_log2(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(log2(args[0]->num));
}
static Value *math_log10(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(log10(args[0]->num));
}
static Value *math_exp(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(exp(args[0]->num));
}
static Value *math_sign(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    double n = args[0]->num;
    return value_number(n > 0 ? 1.0 : (n < 0 ? -1.0 : 0.0));
}
static Value *math_fmod(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    return value_number(fmod(args[0]->num, args[1]->num));
}
static Value *math_clamp(Value **args, size_t argc) {
    if (argc < 3) return value_number(0);
    double val = args[0]->num, lo = args[1]->num, hi = args[2]->num;
    if (val < lo) val = lo;
    if (val > hi) val = hi;
    return value_number(val);
}
static Value *math_random(Value **args, size_t argc) {
    (void)args; (void)argc;
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    return value_number((double)rand() / (double)RAND_MAX);
}
static Value *math_random_int(Value **args, size_t argc) {
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    if (argc < 2) return value_number(0);
    int lo = (int)args[0]->num;
    int hi = (int)args[1]->num;
    if (hi <= lo) return value_number((double)lo);
    return value_number((double)(lo + rand() % (hi - lo)));
}
// Constants returned as functions: math.PI(), math.E(), math.INF(), math.NAN()
static Value *math_pi(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(M_PI);
}
static Value *math_e(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(M_E);
}
static Value *math_inf(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(HUGE_VAL);
}
static Value *math_nan(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(NAN);
}
static Value *math_isnan(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    return value_bool(isnan(args[0]->num));
}
static Value *math_isinf(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    return value_bool(isinf(args[0]->num));
}
static Value *math_isfinite(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    return value_bool(isfinite(args[0]->num));
}
static Value *math_hypot(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    return value_number(hypot(args[0]->num, args[1]->num));
}
static Value *math_cbrt(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(cbrt(args[0]->num));
}
static Value *math_trunc(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(trunc(args[0]->num));
}

static NativeFunc math_fns[] = {
    /* arithmetic  */ { "abs",       math_abs },
                      { "sqrt",      math_sqrt },
                      { "pow",       math_pow },
                      { "cbrt",      math_cbrt },
                      { "hypot",     math_hypot },
                      { "sign",      math_sign },
                      { "fmod",      math_fmod },
                      { "clamp",     math_clamp },
                      { "exp",       math_exp },
    /* rounding    */ { "floor",     math_floor },
                      { "ceil",      math_ceil },
                      { "round",     math_round },
                      { "trunc",     math_trunc },
    /* min / max   */ { "max",       math_max },
                      { "min",       math_min },
    /* trig        */ { "sin",       math_sin },
                      { "cos",       math_cos },
                      { "tan",       math_tan },
                      { "asin",      math_asin },
                      { "acos",      math_acos },
                      { "atan",      math_atan },
                      { "atan2",     math_atan2 },
    /* log         */ { "log",       math_log },
                      { "log2",      math_log2 },
                      { "log10",     math_log10 },
    /* random      */ { "random",    math_random },
                      { "randomInt", math_random_int },
    /* constants   */ { "PI",        math_pi },
                      { "E",         math_e },
                      { "INF",       math_inf },
                      { "NAN",       math_nan },
    /* predicates  */ { "isNaN",     math_isnan },
                      { "isInf",     math_isinf },
                      { "isFinite",  math_isfinite },
    { NULL, NULL }
};

Module module_math(void) {
    Module m;
    m.name      = NULL;
    m.functions = math_fns;
    m.fn_count  = sizeof(math_fns)/sizeof(math_fns[0]) - 1;  // auto-count
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// STRING MODULE  — length, case, search, slice, pad, split, join, trim, …
// ═════════════════════════════════════════════════════════════════════════════

static Value *str_len(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    return value_number((double)strlen(args[0]->str));
}
static Value *str_upper(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *copy = strdup(args[0]->str);
    for (char *p = copy; *p; p++) *p = toupper((unsigned char)*p);
    Value *r = value_string(copy); free(copy); return r;
}
static Value *str_lower(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *copy = strdup(args[0]->str);
    for (char *p = copy; *p; p++) *p = tolower((unsigned char)*p);
    Value *r = value_string(copy); free(copy); return r;
}
static Value *str_contains(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    return value_bool(strstr(args[0]->str, args[1]->str) != NULL);
}
static Value *str_startsWith(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    return value_bool(strncmp(args[0]->str, args[1]->str, strlen(args[1]->str)) == 0);
}
static Value *str_endsWith(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    size_t slen = strlen(args[0]->str);
    size_t plen = strlen(args[1]->str);
    if (plen > slen) return value_bool(0);
    return value_bool(strcmp(args[0]->str + slen - plen, args[1]->str) == 0);
}
static Value *str_indexOf(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_number(-1);
    const char *p = strstr(args[0]->str, args[1]->str);
    return value_number(p ? (double)(p - args[0]->str) : -1.0);
}
static Value *str_lastIndexOf(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_number(-1);
    const char *haystack = args[0]->str;
    const char *needle   = args[1]->str;
    size_t nlen = strlen(needle);
    const char *last = NULL;
    const char *p = haystack;
    while ((p = strstr(p, needle)) != NULL) { last = p; p += nlen; }
    return value_number(last ? (double)(last - haystack) : -1.0);
}
static Value *str_charAt(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    int idx = (int)args[1]->num;
    int len = (int)strlen(args[0]->str);
    if (idx < 0 || idx >= len) return value_string("");
    char buf[2] = { args[0]->str[idx], '\0' };
    return value_string(buf);
}
static Value *str_charCodeAt(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    int idx = (int)args[1]->num;
    int len = (int)strlen(args[0]->str);
    if (idx < 0 || idx >= len) return value_number(-1);
    return value_number((double)(unsigned char)args[0]->str[idx]);
}
static Value *str_fromCharCode(Value **args, size_t argc) {
    if (argc < 1) return value_string("");
    char buf[2] = { (char)(int)args[0]->num, '\0' };
    return value_string(buf);
}
static Value *str_repeat(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    int n = (int)args[1]->num;
    if (n <= 0) return value_string("");
    size_t slen = strlen(args[0]->str);
    size_t total = slen * (size_t)n + 1;
    char *buf = (char *)malloc(total);
    buf[0] = '\0';
    for (int i = 0; i < n; i++) strcat(buf, args[0]->str);
    Value *r = value_string(buf); free(buf); return r;
}
static Value *str_reverse(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    size_t len = strlen(args[0]->str);
    char *buf  = (char *)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = args[0]->str[len - 1 - i];
    buf[len] = '\0';
    Value *r = value_string(buf); free(buf); return r;
}
static Value *str_trim(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *s = strdup(args[0]->str);
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end-1))) end--;
    *end = '\0';
    Value *r = value_string(start); free(s); return r;
}
static Value *str_trimStart(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    const char *s = args[0]->str;
    while (*s && isspace((unsigned char)*s)) s++;
    return value_string(s);
}
static Value *str_trimEnd(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *s = strdup(args[0]->str);
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end-1))) end--;
    *end = '\0';
    Value *r = value_string(s); free(s); return r;
}
static Value *str_replace(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_STRING ||
        args[1]->type != VAL_STRING || args[2]->type != VAL_STRING)
        return value_string("");
    const char *src = args[0]->str;
    const char *old = args[1]->str;
    const char *neu = args[2]->str;
    size_t old_len = strlen(old);
    if (old_len == 0) return value_string(src);
    size_t count = 0;
    const char *p = src;
    while ((p = strstr(p, old)) != NULL) { count++; p += old_len; }
    size_t new_len = strlen(neu);
    size_t src_len = strlen(src);
    size_t buf_size = src_len - count * old_len + count * new_len + 1;
    char *buf = (char *)malloc(buf_size);
    char *out = buf;
    p = src;
    while (*p) {
        if (strstr(p, old) == p) {
            memcpy(out, neu, new_len); out += new_len;
            p += old_len;
        } else { *out++ = *p++; }
    }
    *out = '\0';
    Value *r = value_string(buf); free(buf); return r;
}
static Value *str_substr(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    const char *s = args[0]->str;
    int start = (int)args[1]->num;
    int len   = (int)strlen(s);
    if (start < 0) start = 0;
    if (start >= len) return value_string("");
    int count = (argc >= 3) ? (int)args[2]->num : (len - start);
    if (count < 0) count = 0;
    if (start + count > len) count = len - start;
    char *buf = (char *)malloc((size_t)count + 1);
    memcpy(buf, s + start, (size_t)count);
    buf[count] = '\0';
    Value *r = value_string(buf); free(buf); return r;
}
static Value *str_slice(Value **args, size_t argc) {
    // slice(str, start, end)  — end is exclusive, negative indices wrap
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    const char *s = args[0]->str;
    int len   = (int)strlen(s);
    int start = (int)args[1]->num;
    int end   = (argc >= 3) ? (int)args[2]->num : len;
    if (start < 0) start += len;
    if (end   < 0) end   += len;
    if (start < 0) start = 0;
    if (end   > len) end = len;
    if (start >= end) return value_string("");
    int count = end - start;
    char *buf = (char *)malloc((size_t)count + 1);
    memcpy(buf, s + start, (size_t)count);
    buf[count] = '\0';
    Value *r = value_string(buf); free(buf); return r;
}
static Value *str_padStart(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    const char *s   = args[0]->str;
    int         pad = (int)args[1]->num;
    const char *ch  = (argc >= 3 && args[2]->type == VAL_STRING && args[2]->str[0])
                      ? args[2]->str : " ";
    int slen = (int)strlen(s);
    if (slen >= pad) return value_string(s);
    int need = pad - slen;
    char *buf = (char *)malloc((size_t)pad + 1);
    int chlen = (int)strlen(ch);
    for (int i = 0; i < need; i++) buf[i] = ch[i % chlen];
    memcpy(buf + need, s, (size_t)slen);
    buf[pad] = '\0';
    Value *r = value_string(buf); free(buf); return r;
}
static Value *str_padEnd(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    const char *s   = args[0]->str;
    int         pad = (int)args[1]->num;
    const char *ch  = (argc >= 3 && args[2]->type == VAL_STRING && args[2]->str[0])
                      ? args[2]->str : " ";
    int slen = (int)strlen(s);
    if (slen >= pad) return value_string(s);
    char *buf = (char *)malloc((size_t)pad + 1);
    memcpy(buf, s, (size_t)slen);
    int chlen = (int)strlen(ch);
    for (int i = slen; i < pad; i++) buf[i] = ch[(i - slen) % chlen];
    buf[pad] = '\0';
    Value *r = value_string(buf); free(buf); return r;
}
// split(str, delimiter) → array of strings
static Value *str_split(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) {
        return value_array(NULL, 0);
    }
    const char *s   = args[0]->str;
    const char *sep = (argc >= 2 && args[1]->type == VAL_STRING) ? args[1]->str : " ";
    size_t sep_len  = strlen(sep);

    // Count parts first
    size_t parts = 1;
    if (sep_len > 0) {
        const char *p = s;
        while ((p = strstr(p, sep)) != NULL) { parts++; p += sep_len; }
    }

    Value **items = (Value **)malloc(sizeof(Value *) * parts);
    size_t idx = 0;

    if (sep_len == 0) {
        // Split into individual characters
        size_t slen = strlen(s);
        items = (Value **)realloc(items, sizeof(Value *) * (slen ? slen : 1));
        if (slen == 0) { items[0] = value_string(""); idx = 1; }
        else { for (size_t i = 0; i < slen; i++) { char c[2] = {s[i], 0}; items[i] = value_string(c); } idx = slen; }
    } else {
        const char *p = s;
        while (1) {
            const char *found = strstr(p, sep);
            if (!found) { items[idx++] = value_string(p); break; }
            size_t chunk = (size_t)(found - p);
            char *buf = (char *)malloc(chunk + 1);
            memcpy(buf, p, chunk);
            buf[chunk] = '\0';
            items[idx++] = value_string(buf);
            free(buf);
            p = found + sep_len;
        }
    }
    return value_array(items, idx);
}
// join(array, separator) → string
static Value *str_join(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_string("");
    const char *sep = (argc >= 2 && args[1]->type == VAL_STRING) ? args[1]->str : ",";
    Value *arr = args[0];
    size_t cap = 128, pos = 0;
    char *buf = (char *)malloc(cap);
    for (size_t i = 0; i < arr->array_len; i++) {
        if (i > 0) {
            size_t sl = strlen(sep);
            while (pos + sl + 1 >= cap) { cap *= 2; buf = (char *)realloc(buf, cap); }
            memcpy(buf + pos, sep, sl); pos += sl;
        }
        char *elem = value_to_string(arr->array[i]);
        size_t el  = strlen(elem);
        while (pos + el + 1 >= cap) { cap *= 2; buf = (char *)realloc(buf, cap); }
        memcpy(buf + pos, elem, el); pos += el;
        free(elem);
    }
    buf[pos] = '\0';
    Value *r = value_string(buf); free(buf); return r;
}

static NativeFunc string_fns[] = {
    { "len",           str_len },
    { "upper",         str_upper },
    { "lower",         str_lower },
    { "contains",      str_contains },
    { "startsWith",    str_startsWith },
    { "endsWith",      str_endsWith },
    { "indexOf",       str_indexOf },
    { "lastIndexOf",   str_lastIndexOf },
    { "charAt",        str_charAt },
    { "charCodeAt",    str_charCodeAt },
    { "fromCharCode",  str_fromCharCode },
    { "repeat",        str_repeat },
    { "reverse",       str_reverse },
    { "trim",          str_trim },
    { "trimStart",     str_trimStart },
    { "trimEnd",       str_trimEnd },
    { "replace",       str_replace },
    { "substr",        str_substr },
    { "slice",         str_slice },
    { "padStart",      str_padStart },
    { "padEnd",        str_padEnd },
    { "split",         str_split },
    { "join",          str_join },
    { NULL, NULL }
};

Module module_string(void) {
    Module m;
    m.name      = NULL;
    m.functions = string_fns;
    m.fn_count  = sizeof(string_fns)/sizeof(string_fns[0]) - 1;  // auto-count
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// IO MODULE  — console I/O
// ═════════════════════════════════════════════════════════════════════════════

static Value *io_write(Value **args, size_t argc) {
    for (size_t i = 0; i < argc; i++) {
        char *s = value_to_string(args[i]);
        printf("%s", s);
        free(s);
    }
    fflush(stdout);
    return value_null();
}
static Value *io_writeln(Value **args, size_t argc) {
    for (size_t i = 0; i < argc; i++) {
        if (i > 0) printf(" ");
        char *s = value_to_string(args[i]);
        printf("%s", s);
        free(s);
    }
    printf("\n");
    return value_null();
}
static Value *io_read(Value **args, size_t argc) {
    if (argc > 0) {
        char *s = value_to_string(args[0]);
        printf("%s", s); fflush(stdout); free(s);
    }
    char buf[4096];
    if (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
        return value_string(buf);
    }
    return value_string("");
}

static NativeFunc io_fns[] = {
    { "write",   io_write },
    { "writeln", io_writeln },
    { "read",    io_read },
    { NULL, NULL }
};

Module module_io(void) {
    Module m;
    m.name      = NULL;
    m.functions = io_fns;
    m.fn_count  = sizeof(io_fns)/sizeof(io_fns[0]) - 1;  // auto-count
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// ARRAY MODULE  — create, mutate, query, transform, aggregate
// ═════════════════════════════════════════════════════════════════════════════

// Helper: deep-copy a single value (shallow: numbers/strings/bools are value types)
static Value *value_clone(Value *v) {
    if (!v) return value_null();
    switch (v->type) {
        case VAL_NUMBER:   return value_number(v->num);
        case VAL_STRING:   return value_string(v->str);
        case VAL_BOOL:     return value_bool(v->boolean);
        case VAL_NULL:     return value_null();
        case VAL_ARRAY: {
            Value **items = (Value **)malloc(sizeof(Value *) * v->array_len);
            for (size_t i = 0; i < v->array_len; i++) items[i] = value_clone(v->array[i]);
            return value_array(items, v->array_len);
        }
        default:           return v;  // functions, classes, instances: shared ref
    }
}

// array.create(len, fill)  — create array of length len filled with fill (default 0)
static Value *arr_create(Value **args, size_t argc) {
    int n = (argc >= 1) ? (int)args[0]->num : 0;
    if (n < 0) n = 0;
    Value **items = (Value **)malloc(sizeof(Value *) * (size_t)(n ? n : 1));
    Value *fill   = (argc >= 2) ? args[1] : NULL;
    for (int i = 0; i < n; i++)
        items[i] = fill ? value_clone(fill) : value_number(0);
    return value_array(items, (size_t)n);
}

// array.of(a, b, c, …)  — create array from arguments
static Value *arr_of(Value **args, size_t argc) {
    Value **items = (Value **)malloc(sizeof(Value *) * (argc ? argc : 1));
    for (size_t i = 0; i < argc; i++) items[i] = value_clone(args[i]);
    return value_array(items, argc);
}

// array.len(arr)
static Value *arr_len(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_number(0);
    return value_number((double)args[0]->array_len);
}

// array.get(arr, index)
static Value *arr_get(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_ARRAY) return value_null();
    int idx = (int)args[1]->num;
    int len = (int)args[0]->array_len;
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) return value_null();
    return value_clone(args[0]->array[idx]);
}

// array.set(arr, index, value) — mutates arr in place, returns arr
static Value *arr_set(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_ARRAY) return value_null();
    int idx = (int)args[1]->num;
    int len = (int)args[0]->array_len;
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) return args[0];
    value_destroy(args[0]->array[idx]);
    args[0]->array[idx] = value_clone(args[2]);
    return args[0];   // return same array (shared ref, caller keeps it)
}

// array.push(arr, value) — mutates arr, returns new length
static Value *arr_push(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_ARRAY) return value_number(0);
    Value *arr = args[0];
    if (arr->array_len >= arr->array_cap) {
        arr->array_cap = arr->array_cap ? arr->array_cap * 2 : 4;
        arr->array = (Value **)realloc(arr->array, sizeof(Value *) * arr->array_cap);
    }
    arr->array[arr->array_len++] = value_clone(args[1]);
    return arr;   // return the array itself (not a number) for chaining
}

// array.pop(arr) — removes and returns last element
static Value *arr_pop(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY || args[0]->array_len == 0)
        return value_null();
    Value *arr = args[0];
    Value *last = arr->array[--arr->array_len];
    arr->array[arr->array_len] = NULL;
    return last;   // ownership transferred to caller
}

// array.shift(arr) — removes and returns first element
static Value *arr_shift(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY || args[0]->array_len == 0)
        return value_null();
    Value *arr   = args[0];
    Value *first = arr->array[0];
    memmove(arr->array, arr->array + 1, sizeof(Value *) * (arr->array_len - 1));
    arr->array_len--;
    return first;
}

// array.unshift(arr, value) — inserts at front, returns new length
static Value *arr_unshift(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_ARRAY) return value_number(0);
    Value *arr = args[0];
    if (arr->array_len >= arr->array_cap) {
        arr->array_cap = arr->array_cap ? arr->array_cap * 2 : 4;
        arr->array = (Value **)realloc(arr->array, sizeof(Value *) * arr->array_cap);
    }
    memmove(arr->array + 1, arr->array, sizeof(Value *) * arr->array_len);
    arr->array[0] = value_clone(args[1]);
    arr->array_len++;
    return arr;   // return the array itself
}

// array.contains(arr, val) — linear search by string representation
static Value *arr_contains(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_ARRAY) return value_bool(0);
    Value *arr = args[0];
    Value *needle = args[1];
    for (size_t i = 0; i < arr->array_len; i++) {
        Value *elem = arr->array[i];
        // Compare by type then value
        if (elem->type != needle->type) continue;
        switch (elem->type) {
            case VAL_NUMBER: if (elem->num == needle->num) return value_bool(1); break;
            case VAL_STRING: if (strcmp(elem->str, needle->str) == 0) return value_bool(1); break;
            case VAL_BOOL:   if (elem->boolean == needle->boolean) return value_bool(1); break;
            case VAL_NULL:   return value_bool(1);
            default: break;
        }
    }
    return value_bool(0);
}

// array.indexOf(arr, val) — returns first index or -1
static Value *arr_indexOf(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_ARRAY) return value_number(-1);
    Value *arr = args[0];
    Value *needle = args[1];
    for (size_t i = 0; i < arr->array_len; i++) {
        Value *elem = arr->array[i];
        if (elem->type != needle->type) continue;
        switch (elem->type) {
            case VAL_NUMBER: if (elem->num == needle->num) return value_number((double)i); break;
            case VAL_STRING: if (strcmp(elem->str, needle->str) == 0) return value_number((double)i); break;
            case VAL_BOOL:   if (elem->boolean == needle->boolean) return value_number((double)i); break;
            case VAL_NULL:   return value_number((double)i);
            default: break;
        }
    }
    return value_number(-1);
}

// array.reverse(arr) — returns new reversed array
static Value *arr_reverse(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_array(NULL, 0);
    Value *arr = args[0];
    size_t len = arr->array_len;
    Value **items = (Value **)malloc(sizeof(Value *) * (len ? len : 1));
    for (size_t i = 0; i < len; i++)
        items[i] = value_clone(arr->array[len - 1 - i]);
    return value_array(items, len);
}

// array.slice(arr, start, end) — returns new sub-array [start, end)
static Value *arr_slice(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_array(NULL, 0);
    Value *arr = args[0];
    int len   = (int)arr->array_len;
    int start = (argc >= 2) ? (int)args[1]->num : 0;
    int end   = (argc >= 3) ? (int)args[2]->num : len;
    if (start < 0) start += len;
    if (end   < 0) end   += len;
    if (start < 0) start = 0;
    if (end   > len) end = len;
    if (start >= end) return value_array(NULL, 0);
    size_t count = (size_t)(end - start);
    Value **items = (Value **)malloc(sizeof(Value *) * count);
    for (size_t i = 0; i < count; i++)
        items[i] = value_clone(arr->array[start + (int)i]);
    return value_array(items, count);
}

// array.concat(arr1, arr2) — returns new merged array
static Value *arr_concat(Value **args, size_t argc) {
    if (argc < 2) return value_array(NULL, 0);
    Value *a = (args[0]->type == VAL_ARRAY) ? args[0] : NULL;
    Value *b = (args[1]->type == VAL_ARRAY) ? args[1] : NULL;
    size_t alen = a ? a->array_len : 0;
    size_t blen = b ? b->array_len : 0;
    size_t total = alen + blen;
    Value **items = (Value **)malloc(sizeof(Value *) * (total ? total : 1));
    for (size_t i = 0; i < alen; i++) items[i]       = value_clone(a->array[i]);
    for (size_t i = 0; i < blen; i++) items[alen + i] = value_clone(b->array[i]);
    return value_array(items, total);
}

// array.fill(arr, value) — fills entire array in place
static Value *arr_fill(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_ARRAY) return value_null();
    Value *arr  = args[0];
    Value *fill = args[1];
    for (size_t i = 0; i < arr->array_len; i++) {
        value_destroy(arr->array[i]);
        arr->array[i] = value_clone(fill);
    }
    return args[0];
}

// array.empty() — returns empty array
static Value *arr_empty(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_array(NULL, 0);
}

// array.sum(arr) — sums all numeric elements
static Value *arr_sum(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_number(0);
    double sum = 0;
    for (size_t i = 0; i < args[0]->array_len; i++)
        if (args[0]->array[i]->type == VAL_NUMBER)
            sum += args[0]->array[i]->num;
    return value_number(sum);
}

// array.min(arr) — minimum numeric element
static Value *arr_min(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY || args[0]->array_len == 0)
        return value_null();
    double m = DBL_MAX;
    int found = 0;
    for (size_t i = 0; i < args[0]->array_len; i++)
        if (args[0]->array[i]->type == VAL_NUMBER) {
            if (!found || args[0]->array[i]->num < m) m = args[0]->array[i]->num;
            found = 1;
        }
    return found ? value_number(m) : value_null();
}

// array.max(arr) — maximum numeric element
static Value *arr_max(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY || args[0]->array_len == 0)
        return value_null();
    double m = -DBL_MAX;
    int found = 0;
    for (size_t i = 0; i < args[0]->array_len; i++)
        if (args[0]->array[i]->type == VAL_NUMBER) {
            if (!found || args[0]->array[i]->num > m) m = args[0]->array[i]->num;
            found = 1;
        }
    return found ? value_number(m) : value_null();
}

// array.join(arr, sep) — join elements into string (delegates to string.join)
static Value *arr_join(Value **args, size_t argc) {
    return str_join(args, argc);   // reuse string module impl
}

// array.sort(arr) — numeric sort, returns new sorted array
static int cmp_numeric(const void *a, const void *b) {
    Value *va = *(Value * const *)a;
    Value *vb = *(Value * const *)b;
    double da = (va->type == VAL_NUMBER) ? va->num : 0;
    double db = (vb->type == VAL_NUMBER) ? vb->num : 0;
    return (da > db) - (da < db);
}
static int cmp_string(const void *a, const void *b) {
    Value *va = *(Value * const *)a;
    Value *vb = *(Value * const *)b;
    const char *sa = (va->type == VAL_STRING) ? va->str : "";
    const char *sb = (vb->type == VAL_STRING) ? vb->str : "";
    return strcmp(sa, sb);
}
static Value *arr_sort(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_array(NULL, 0);
    Value *arr = args[0];
    size_t len = arr->array_len;
    Value **items = (Value **)malloc(sizeof(Value *) * (len ? len : 1));
    for (size_t i = 0; i < len; i++) items[i] = value_clone(arr->array[i]);
    // Detect type: if first element is string, sort as strings
    if (len > 0 && items[0]->type == VAL_STRING)
        qsort(items, len, sizeof(Value *), cmp_string);
    else
        qsort(items, len, sizeof(Value *), cmp_numeric);
    return value_array(items, len);
}

// array.sortDesc(arr) — descending sort, returns new sorted array
static int cmp_numeric_desc(const void *a, const void *b) {
    return -cmp_numeric(a, b);
}
static int cmp_string_desc(const void *a, const void *b) {
    return -cmp_string(a, b);
}
static Value *arr_sortDesc(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_array(NULL, 0);
    Value *arr = args[0];
    size_t len = arr->array_len;
    Value **items = (Value **)malloc(sizeof(Value *) * (len ? len : 1));
    for (size_t i = 0; i < len; i++) items[i] = value_clone(arr->array[i]);
    if (len > 0 && items[0]->type == VAL_STRING)
        qsort(items, len, sizeof(Value *), cmp_string_desc);
    else
        qsort(items, len, sizeof(Value *), cmp_numeric_desc);
    return value_array(items, len);
}

// array.unique(arr) — returns new array with duplicates removed (preserves order)
static Value *arr_unique(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_array(NULL, 0);
    Value *arr = args[0];
    size_t cap = arr->array_len ? arr->array_len : 1;
    Value **items = (Value **)malloc(sizeof(Value *) * cap);
    size_t count = 0;
    for (size_t i = 0; i < arr->array_len; i++) {
        int dup = 0;
        for (size_t j = 0; j < count; j++) {
            Value *a = arr->array[i], *b = items[j];
            if (a->type != b->type) continue;
            if (a->type == VAL_NUMBER && a->num == b->num)     { dup = 1; break; }
            if (a->type == VAL_STRING && strcmp(a->str, b->str) == 0) { dup = 1; break; }
            if (a->type == VAL_BOOL   && a->boolean == b->boolean)   { dup = 1; break; }
            if (a->type == VAL_NULL)                                  { dup = 1; break; }
        }
        if (!dup) items[count++] = value_clone(arr->array[i]);
    }
    return value_array(items, count);
}

// array.flatten(arr) — one-level flatten of nested arrays
static Value *arr_flatten(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_array(NULL, 0);
    Value *arr = args[0];
    size_t cap = 16, count = 0;
    Value **items = (Value **)malloc(sizeof(Value *) * cap);
    for (size_t i = 0; i < arr->array_len; i++) {
        Value *elem = arr->array[i];
        if (elem->type == VAL_ARRAY) {
            for (size_t j = 0; j < elem->array_len; j++) {
                if (count >= cap) { cap *= 2; items = (Value **)realloc(items, sizeof(Value *) * cap); }
                items[count++] = value_clone(elem->array[j]);
            }
        } else {
            if (count >= cap) { cap *= 2; items = (Value **)realloc(items, sizeof(Value *) * cap); }
            items[count++] = value_clone(elem);
        }
    }
    return value_array(items, count);
}

// array.range(start, end, step) — generate numeric range
static Value *arr_range(Value **args, size_t argc) {
    if (argc < 1) return value_array(NULL, 0);
    double start = 0, end_val, step = 1;
    if (argc == 1) { end_val = args[0]->num; }
    else           { start = args[0]->num; end_val = args[1]->num; }
    if (argc >= 3) step = args[2]->num;
    if (step == 0) return value_array(NULL, 0);

    // Pre-calculate count
    size_t count = 0;
    if (step > 0) { for (double v = start; v < end_val; v += step) count++; }
    else          { for (double v = start; v > end_val; v += step) count++; }

    Value **items = (Value **)malloc(sizeof(Value *) * (count ? count : 1));
    size_t idx = 0;
    if (step > 0) { for (double v = start; v < end_val; v += step) items[idx++] = value_number(v); }
    else          { for (double v = start; v > end_val; v += step) items[idx++] = value_number(v); }
    return value_array(items, idx);
}

static NativeFunc array_fns[] = {
    /* creation    */ { "create",    arr_create },
                      { "of",        arr_of },
                      { "empty",     arr_empty },
                      { "range",     arr_range },
    /* access      */ { "len",       arr_len },
                      { "get",       arr_get },
                      { "set",       arr_set },
    /* mutators    */ { "push",      arr_push },
                      { "pop",       arr_pop },
                      { "shift",     arr_shift },
                      { "unshift",   arr_unshift },
                      { "fill",      arr_fill },
    /* query       */ { "contains",  arr_contains },
                      { "indexOf",   arr_indexOf },
    /* transforms  */ { "reverse",   arr_reverse },
                      { "slice",     arr_slice },
                      { "concat",    arr_concat },
                      { "sort",      arr_sort },
                      { "sortDesc",  arr_sortDesc },
                      { "unique",    arr_unique },
                      { "flatten",   arr_flatten },
    /* aggregates  */ { "sum",       arr_sum },
                      { "min",       arr_min },
                      { "max",       arr_max },
                      { "join",      arr_join },
    { NULL, NULL }
};

Module module_array(void) {
    Module m;
    m.name      = NULL;
    m.functions = array_fns;
    m.fn_count  = sizeof(array_fns)/sizeof(array_fns[0]) - 1;  // auto-count
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// OS MODULE  — process, environment, time
// ═════════════════════════════════════════════════════════════════════════════

static Value *os_exit(Value **args, size_t argc) {
    int code = (argc >= 1) ? (int)args[0]->num : 0;
    exit(code);
    return value_null();  // unreachable
}
static Value *os_time(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)time(NULL));
}
static Value *os_clock(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)clock() / (double)CLOCKS_PER_SEC);
}
static Value *os_env(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    const char *val = getenv(args[0]->str);
    return val ? value_string(val) : value_null();
}
static Value *os_cwd(Value **args, size_t argc) {
    (void)args; (void)argc;
    char buf[4096];
    if (getcwd(buf, sizeof(buf))) return value_string(buf);
    return value_string("");
}

static NativeFunc os_fns[] = {
    { "exit",  os_exit },
    { "time",  os_time },
    { "clock", os_clock },
    { "env",   os_env },
    { "cwd",   os_cwd },
    { NULL, NULL }
};

Module module_os(void) {
    Module m;
    m.name      = NULL;
    m.functions = os_fns;
    m.fn_count  = sizeof(os_fns)/sizeof(os_fns[0]) - 1;  // auto-count
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// TYPE MODULE  — conversion & predicate functions
// ═════════════════════════════════════════════════════════════════════════════

static Value *type_toNumber(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    Value *v = args[0];
    switch (v->type) {
        case VAL_NUMBER: return value_number(v->num);
        case VAL_BOOL:   return value_number(v->boolean ? 1.0 : 0.0);
        case VAL_STRING: {
            char *end;
            double n = strtod(v->str, &end);
            // If entire string consumed → valid number; otherwise NaN
            while (*end && isspace((unsigned char)*end)) end++;
            return value_number(*end == '\0' && v->str[0] != '\0' ? n : NAN);
        }
        case VAL_NULL:   return value_number(0);
        default:         return value_number(NAN);
    }
}
static Value *type_toString(Value **args, size_t argc) {
    if (argc < 1) return value_string("");
    char *s = value_to_string(args[0]);
    Value *r = value_string(s);
    free(s);
    return r;
}
static Value *type_toBool(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    Value *v = args[0];
    switch (v->type) {
        case VAL_BOOL:   return value_bool(v->boolean);
        case VAL_NUMBER: return value_bool(v->num != 0.0);
        case VAL_STRING: return value_bool(strlen(v->str) > 0);
        case VAL_NULL:   return value_bool(0);
        default:         return value_bool(1);
    }
}
static Value *type_isNumber(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_NUMBER);
}
static Value *type_isString(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_STRING);
}
static Value *type_isBool(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_BOOL);
}
static Value *type_isNull(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_NULL);
}
static Value *type_isArray(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_ARRAY);
}
static Value *type_isFunction(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_FUNCTION);
}
static Value *type_typeOf(Value **args, size_t argc) {
    // Mirrors the typeof keyword but callable
    if (argc < 1) return value_string("null");
    switch (args[0]->type) {
        case VAL_NUMBER:   return value_string("number");
        case VAL_STRING:   return value_string("string");
        case VAL_BOOL:     return value_string("bool");
        case VAL_NULL:     return value_string("null");
        case VAL_FUNCTION: return value_string("function");
        case VAL_CLASS:    return value_string("class");
        case VAL_ARRAY:    return value_string("array");
        case VAL_INSTANCE: {
            const char *name = (args[0]->instance && args[0]->instance->class_def)
                             ? args[0]->instance->class_def->name : "instance";
            return value_string(name);
        }
        default: return value_string("unknown");
    }
}

static NativeFunc type_fns[] = {
    { "toNumber",   type_toNumber },
    { "toString",   type_toString },
    { "toBool",     type_toBool },
    { "isNumber",   type_isNumber },
    { "isString",   type_isString },
    { "isBool",     type_isBool },
    { "isNull",     type_isNull },
    { "isArray",    type_isArray },
    { "isFunction", type_isFunction },
    { "typeOf",     type_typeOf },
    { NULL, NULL }
};

Module module_type(void) {
    Module m;
    m.name      = NULL;
    m.functions = type_fns;
    m.fn_count  = sizeof(type_fns)/sizeof(type_fns[0]) - 1;  // auto-count
    return m;
}