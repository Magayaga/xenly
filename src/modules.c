/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#include "modules.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

// ─── Registry ────────────────────────────────────────────────────────────────
int modules_get(const char *name, Module *out) {
    if (strcmp(name, "math")   == 0) { *out = module_math();   return 1; }
    if (strcmp(name, "string") == 0) { *out = module_string(); return 1; }
    if (strcmp(name, "io")     == 0) { *out = module_io();     return 1; }
    return 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// MATH MODULE
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

static Value *math_log(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(log(args[0]->num));
}

static Value *math_random(Value **args, size_t argc) {
    (void)args; (void)argc;
    // Seed on first call
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    return value_number((double)rand() / (double)RAND_MAX);
}

static NativeFunc math_fns[] = {
    { "abs",    math_abs },
    { "sqrt",   math_sqrt },
    { "pow",    math_pow },
    { "floor",  math_floor },
    { "ceil",   math_ceil },
    { "round",  math_round },
    { "max",    math_max },
    { "min",    math_min },
    { "sin",    math_sin },
    { "cos",    math_cos },
    { "log",    math_log },
    { "random", math_random },
    { NULL, NULL }
};

Module module_math(void) {
    Module m;
    m.name      = NULL;  // set by interpreter on load
    m.functions = math_fns;
    m.fn_count  = 12;
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// STRING MODULE
// ═════════════════════════════════════════════════════════════════════════════

static Value *str_len(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    return value_number((double)strlen(args[0]->str));
}

static Value *str_upper(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *copy = strdup(args[0]->str);
    for (char *p = copy; *p; p++) *p = toupper((unsigned char)*p);
    Value *r = value_string(copy);
    free(copy);
    return r;
}

static Value *str_lower(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *copy = strdup(args[0]->str);
    for (char *p = copy; *p; p++) *p = tolower((unsigned char)*p);
    Value *r = value_string(copy);
    free(copy);
    return r;
}

static Value *str_contains(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    return value_bool(strstr(args[0]->str, args[1]->str) != NULL);
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
    Value *r = value_string(buf);
    free(buf);
    return r;
}

static Value *str_reverse(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    size_t len = strlen(args[0]->str);
    char *buf  = (char *)malloc(len + 1);
    for (size_t i = 0; i < len; i++)
        buf[i] = args[0]->str[len - 1 - i];
    buf[len] = '\0';
    Value *r = value_string(buf);
    free(buf);
    return r;
}

static Value *str_trim(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    char *s = strdup(args[0]->str);
    // Trim leading
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    // Trim trailing
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end-1))) end--;
    *end = '\0';
    Value *r = value_string(start);
    free(s);
    return r;
}

static Value *str_replace(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_STRING ||
        args[1]->type != VAL_STRING || args[2]->type != VAL_STRING)
        return value_string("");
    const char *src = args[0]->str;
    const char *old = args[1]->str;
    const char *new = args[2]->str;
    size_t old_len = strlen(old);
    if (old_len == 0) return value_string(src);

    // Count occurrences
    size_t count = 0;
    const char *p = src;
    while ((p = strstr(p, old)) != NULL) { count++; p += old_len; }

    size_t new_len = strlen(new);
    size_t src_len = strlen(src);
    size_t buf_size = src_len - count * old_len + count * new_len + 1;
    char *buf = (char *)malloc(buf_size);
    char *out = buf;
    p = src;
    while (*p) {
        if (strstr(p, old) == p) {
            memcpy(out, new, new_len); out += new_len;
            p += old_len;
        } else {
            *out++ = *p++;
        }
    }
    *out = '\0';
    Value *r = value_string(buf);
    free(buf);
    return r;
}

static Value *str_substr(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    const char *s = args[0]->str;
    int start = (int)args[1]->num;
    int len = (int)strlen(s);
    if (start < 0) start = 0;
    if (start >= len) return value_string("");
    int count = (argc >= 3) ? (int)args[2]->num : (len - start);
    if (start + count > len) count = len - start;
    char buf[4096];
    strncpy(buf, s + start, count);
    buf[count] = '\0';
    return value_string(buf);
}

static NativeFunc string_fns[] = {
    { "len",      str_len },
    { "upper",    str_upper },
    { "lower",    str_lower },
    { "contains", str_contains },
    { "repeat",   str_repeat },
    { "reverse",  str_reverse },
    { "trim",     str_trim },
    { "replace",  str_replace },
    { "substr",   str_substr },
    { NULL, NULL }
};

Module module_string(void) {
    Module m;
    m.name      = NULL;
    m.functions = string_fns;
    m.fn_count  = 9;
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// IO MODULE
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
        char *s = value_to_string(args[i]);
        if (i > 0) printf(" ");
        printf("%s", s);
        free(s);
    }
    printf("\n");
    return value_null();
}

static Value *io_read(Value **args, size_t argc) {
    if (argc > 0) {
        char *s = value_to_string(args[0]);
        printf("%s", s);
        fflush(stdout);
        free(s);
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
    m.fn_count  = 3;
    return m;
}
