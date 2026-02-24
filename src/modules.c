/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
#define _GNU_SOURCE
#include "modules.h"
#include "unicode.h"
#include "platform.h"
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
    if (strcmp(name, "math")      == 0) { *out = module_math();      return 1; }
    if (strcmp(name, "string")    == 0) { *out = module_string();    return 1; }
    if (strcmp(name, "io")        == 0) { *out = module_io();        return 1; }
    if (strcmp(name, "array")     == 0) { *out = module_array();     return 1; }
    if (strcmp(name, "os")        == 0) { *out = module_os();        return 1; }
    if (strcmp(name, "type")      == 0) { *out = module_type();      return 1; }
    if (strcmp(name, "crypto")    == 0) { *out = module_crypto();    return 1; }
    if (strcmp(name, "path")      == 0) { *out = module_path();      return 1; }
    if (strcmp(name, "multiproc") == 0) { *out = module_multiproc(); return 1; }
    if (strcmp(name, "sys")       == 0) { *out = module_sys();       return 1; }
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

// ─── Complex Numbers ──────────────────────────────────────────────────────────
// Complex numbers are represented as arrays [real, imag]

static Value *math_complex(Value **args, size_t argc) {
    // Create complex number from real and imaginary parts
    double real = (argc >= 1) ? args[0]->num : 0.0;
    double imag = (argc >= 2) ? args[1]->num : 0.0;
    
    // Return as array [real, imag] — use value_array() so it's registered correctly
    Value **elems = (Value **)malloc(sizeof(Value *) * 2);
    elems[0] = value_number(real);
    elems[1] = value_number(imag);
    return value_array(elems, 2);
}

static Value *math_complexAdd(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    if (args[0]->type != VAL_ARRAY || args[1]->type != VAL_ARRAY) return value_null();
    if (args[0]->array_len < 2 || args[1]->array_len < 2) return value_null();
    
    double r1 = args[0]->array[0]->num;
    double i1 = args[0]->array[1]->num;
    double r2 = args[1]->array[0]->num;
    double i2 = args[1]->array[1]->num;
    
    Value **elems = (Value **)malloc(sizeof(Value *) * 2);
    elems[0] = value_number(r1 + r2);
    elems[1] = value_number(i1 + i2);
    return value_array(elems, 2);
}

static Value *math_complexMul(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    if (args[0]->type != VAL_ARRAY || args[1]->type != VAL_ARRAY) return value_null();
    if (args[0]->array_len < 2 || args[1]->array_len < 2) return value_null();
    
    double r1 = args[0]->array[0]->num;
    double i1 = args[0]->array[1]->num;
    double r2 = args[1]->array[0]->num;
    double i2 = args[1]->array[1]->num;
    
    // (r1 + i1*i) * (r2 + i2*i) = (r1*r2 - i1*i2) + (r1*i2 + i1*r2)*i
    Value **elems = (Value **)malloc(sizeof(Value *) * 2);
    elems[0] = value_number(r1 * r2 - i1 * i2);
    elems[1] = value_number(r1 * i2 + i1 * r2);
    return value_array(elems, 2);
}

static Value *math_complexAbs(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    if (args[0]->type != VAL_ARRAY || args[0]->array_len < 2) return value_number(0);
    
    double real = args[0]->array[0]->num;
    double imag = args[0]->array[1]->num;
    return value_number(sqrt(real * real + imag * imag));
}

static Value *math_complexConj(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    if (args[0]->type != VAL_ARRAY || args[0]->array_len < 2) return value_null();
    
    double real = args[0]->array[0]->num;
    double imag = args[0]->array[1]->num;
    
    Value **elems = (Value **)malloc(sizeof(Value *) * 2);
    elems[0] = value_number(real);
    elems[1] = value_number(-imag);
    return value_array(elems, 2);
}

static Value *math_complexPhase(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    if (args[0]->type != VAL_ARRAY || args[0]->array_len < 2) return value_number(0);
    
    double real = args[0]->array[0]->num;
    double imag = args[0]->array[1]->num;
    return value_number(atan2(imag, real));
}

// ─── Type-Generic Math Functions ──────────────────────────────────────────────

static Value *math_sum(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    
    // If array, sum all elements
    if (args[0]->type == VAL_ARRAY) {
        double total = 0;
        for (size_t i = 0; i < args[0]->array_len; i++) {
            if (args[0]->array[i]->type == VAL_NUMBER) {
                total += args[0]->array[i]->num;
            }
        }
        return value_number(total);
    }
    
    // If multiple arguments, sum them
    double total = 0;
    for (size_t i = 0; i < argc; i++) {
        if (args[i]->type == VAL_NUMBER) {
            total += args[i]->num;
        }
    }
    return value_number(total);
}

static Value *math_product(Value **args, size_t argc) {
    if (argc < 1) return value_number(1);
    
    // If array, multiply all elements
    if (args[0]->type == VAL_ARRAY) {
        double result = 1;
        for (size_t i = 0; i < args[0]->array_len; i++) {
            if (args[0]->array[i]->type == VAL_NUMBER) {
                result *= args[0]->array[i]->num;
            }
        }
        return value_number(result);
    }
    
    // If multiple arguments, multiply them
    double result = 1;
    for (size_t i = 0; i < argc; i++) {
        if (args[i]->type == VAL_NUMBER) {
            result *= args[i]->num;
        }
    }
    return value_number(result);
}

static Value *math_mean(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    
    // If array, calculate mean
    if (args[0]->type == VAL_ARRAY) {
        if (args[0]->array_len == 0) return value_number(0);
        double total = 0;
        size_t count = 0;
        for (size_t i = 0; i < args[0]->array_len; i++) {
            if (args[0]->array[i]->type == VAL_NUMBER) {
                total += args[0]->array[i]->num;
                count++;
            }
        }
        return value_number(count > 0 ? total / count : 0);
    }
    
    // If multiple arguments, calculate mean
    if (argc == 0) return value_number(0);
    double total = 0;
    for (size_t i = 0; i < argc; i++) {
        if (args[i]->type == VAL_NUMBER) {
            total += args[i]->num;
        }
    }
    return value_number(total / argc);
}

static Value *math_median(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    
    Value *arr = args[0];
    if (arr->type != VAL_ARRAY || arr->array_len == 0) return value_number(0);
    
    // Copy numbers to temp array
    double *nums = (double *)malloc(sizeof(double) * arr->array_len);
    size_t count = 0;
    for (size_t i = 0; i < arr->array_len; i++) {
        if (arr->array[i]->type == VAL_NUMBER) {
            nums[count++] = arr->array[i]->num;
        }
    }
    
    if (count == 0) {
        free(nums);
        return value_number(0);
    }
    
    // Simple bubble sort
    for (size_t i = 0; i < count - 1; i++) {
        for (size_t j = 0; j < count - i - 1; j++) {
            if (nums[j] > nums[j + 1]) {
                double temp = nums[j];
                nums[j] = nums[j + 1];
                nums[j + 1] = temp;
            }
        }
    }
    
    double result;
    if (count % 2 == 0) {
        result = (nums[count / 2 - 1] + nums[count / 2]) / 2.0;
    } else {
        result = nums[count / 2];
    }
    
    free(nums);
    return value_number(result);
}

static Value *math_variance(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    
    Value *arr = args[0];
    if (arr->type != VAL_ARRAY || arr->array_len == 0) return value_number(0);
    
    // Calculate mean
    double total = 0;
    size_t count = 0;
    for (size_t i = 0; i < arr->array_len; i++) {
        if (arr->array[i]->type == VAL_NUMBER) {
            total += arr->array[i]->num;
            count++;
        }
    }
    
    if (count == 0) return value_number(0);
    double mean = total / count;
    
    // Calculate variance
    double variance = 0;
    for (size_t i = 0; i < arr->array_len; i++) {
        if (arr->array[i]->type == VAL_NUMBER) {
            double diff = arr->array[i]->num - mean;
            variance += diff * diff;
        }
    }
    
    return value_number(variance / count);
}

static Value *math_stddev(Value **args, size_t argc) {
    Value *var = math_variance(args, argc);
    double result = sqrt(var->num);
    value_destroy(var);
    return value_number(result);
}

// ─── Advanced Math Functions ───────────────────────────────────────────────────

static Value *math_gcd(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    
    long long a = (long long)args[0]->num;
    long long b = (long long)args[1]->num;
    
    a = llabs(a);
    b = llabs(b);
    
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    
    return value_number((double)a);
}

static Value *math_lcm(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    
    long long a = (long long)args[0]->num;
    long long b = (long long)args[1]->num;
    
    if (a == 0 || b == 0) return value_number(0);
    
    Value *gcd_val = math_gcd(args, argc);
    long long gcd = (long long)gcd_val->num;
    value_destroy(gcd_val);
    
    return value_number((double)((llabs(a) / gcd) * llabs(b)));
}

static Value *math_factorial(Value **args, size_t argc) {
    if (argc < 1) return value_number(1);
    
    int n = (int)args[0]->num;
    if (n < 0) return value_number(NAN);
    if (n > 170) return value_number(INFINITY);  // Overflow
    
    double result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    
    return value_number(result);
}

static Value *math_combinations(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    
    int n = (int)args[0]->num;
    int k = (int)args[1]->num;
    
    if (k > n || k < 0 || n < 0) return value_number(0);
    if (k == 0 || k == n) return value_number(1);
    
    // Use iterative method to avoid overflow
    k = (k > n - k) ? n - k : k;  // Take advantage of symmetry
    
    double result = 1;
    for (int i = 0; i < k; i++) {
        result *= (n - i);
        result /= (i + 1);
    }
    
    return value_number(result);
}

static Value *math_permutations(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    
    int n = (int)args[0]->num;
    int k = (int)args[1]->num;
    
    if (k > n || k < 0 || n < 0) return value_number(0);
    
    double result = 1;
    for (int i = 0; i < k; i++) {
        result *= (n - i);
    }
    
    return value_number(result);
}

static Value *math_lerp(Value **args, size_t argc) {
    if (argc < 3) return value_number(0);
    
    double a = args[0]->num;
    double b = args[1]->num;
    double t = args[2]->num;
    
    return value_number(a + (b - a) * t);
}

static Value *math_degrees(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(args[0]->num * 180.0 / M_PI);
}

static Value *math_radians(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    return value_number(args[0]->num * M_PI / 180.0);
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
    /* complex     */ { "complex",       math_complex },
                      { "complexAdd",    math_complexAdd },
                      { "complexMul",    math_complexMul },
                      { "complexAbs",    math_complexAbs },
                      { "complexConj",   math_complexConj },
                      { "complexPhase",  math_complexPhase },
    /* generic     */ { "sum",        math_sum },
                      { "product",    math_product },
                      { "mean",       math_mean },
                      { "median",     math_median },
                      { "variance",   math_variance },
                      { "stddev",     math_stddev },
    /* advanced    */ { "gcd",        math_gcd },
                      { "lcm",        math_lcm },
                      { "factorial",  math_factorial },
                      { "combinations", math_combinations },
                      { "permutations", math_permutations },
                      { "lerp",       math_lerp },
                      { "degrees",    math_degrees },
                      { "radians",    math_radians },
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

// Unicode-aware string functions
static Value *str_unicodeLength(Value **args, size_t argc) {
    // Get length in Unicode characters (not bytes)
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    size_t len = utf8_strlen(args[0]->str);
    return value_number((double)len);
}

static Value *str_unicodeCharAt(Value **args, size_t argc) {
    // Get Unicode character at index
    if (argc < 2 || args[0]->type != VAL_STRING) return value_string("");
    const char *str = args[0]->str;
    size_t index = (size_t)args[1]->num;
    
    const char *pos = utf8_char_at(str, index);
    if (!pos || *pos == '\0') return value_string("");
    
    size_t bytes;
    utf8_decode(pos, &bytes);
    
    char buf[5] = {0};
    for (size_t i = 0; i < bytes && i < 4; i++) {
        buf[i] = pos[i];
    }
    return value_string(buf);
}

static Value *str_codePointAt(Value **args, size_t argc) {
    // Get Unicode codepoint at index
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(0);
    const char *str = args[0]->str;
    size_t index = (size_t)args[1]->num;
    
    const char *pos = utf8_char_at(str, index);
    if (!pos || *pos == '\0') return value_number(0);
    
    size_t bytes;
    uint32_t cp = utf8_decode(pos, &bytes);
    return value_number((double)cp);
}

static Value *str_fromCodePoint(Value **args, size_t argc) {
    // Create string from Unicode codepoint
    if (argc < 1) return value_string("");
    uint32_t cp = (uint32_t)args[0]->num;
    
    char buf[5] = {0};
    size_t bytes = utf8_encode(cp, buf);
    if (bytes == 0) return value_string("");
    
    return value_string(buf);
}

static Value *str_normalize(Value **args, size_t argc) {
    // Simple normalization: just return the string (full NFD/NFC would require ICU)
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    return value_string(args[0]->str);
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
    // Unicode-aware functions
    { "unicodeLength",  str_unicodeLength },
    { "unicodeCharAt",  str_unicodeCharAt },
    { "codePointAt",    str_codePointAt },
    { "fromCodePoint",  str_fromCodePoint },
    { "normalize",      str_normalize },
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
            // FIX: value_array() sets array_cap = len ? len : 4.
            // Must allocate at least that many slots so future push() calls
            // don't write past the end of the backing store.
            size_t alloc = (v->array_len > 0) ? v->array_len : 4;
            Value **items = (Value **)malloc(sizeof(Value *) * alloc);
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
    // FIX: value_array() sets array_cap = len ? len : 4.
    // We must allocate AT LEAST that many slots so arr_push()
    // doesn't write past the end of the backing store.
    int alloc = (n > 0) ? n : 4;
    Value **items = (Value **)malloc(sizeof(Value *) * (size_t)alloc);
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
// OS MODULE  — process, environment, time, filesystem
// ═════════════════════════════════════════════════════════════════════════════

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

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

// New filesystem functions
static Value *os_exists(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(stat(args[0]->str, &st) == 0);
}

static Value *os_isFile(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    if (stat(args[0]->str, &st) != 0) return value_bool(0);
    return value_bool(S_ISREG(st.st_mode));
}

static Value *os_isDir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    if (stat(args[0]->str, &st) != 0) return value_bool(0);
    return value_bool(S_ISDIR(st.st_mode));
}

static Value *os_mkdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    int mode = (argc >= 2) ? (int)args[1]->num : 0755;
    return value_bool(mkdir(args[0]->str, (mode_t)mode) == 0);
}

static Value *os_rmdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    return value_bool(rmdir(args[0]->str) == 0);
}

static Value *os_remove(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    return value_bool(remove(args[0]->str) == 0);
}

static Value *os_rename(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) 
        return value_bool(0);
    return value_bool(rename(args[0]->str, args[1]->str) == 0);
}

static Value *os_listdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_array(NULL, 0);
    
    DIR *dir = opendir(args[0]->str);
    if (!dir) return value_array(NULL, 0);
    
    Value **items = NULL;
    size_t count = 0, cap = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        if (count >= cap) {
            cap = cap ? cap * 2 : 8;
            items = realloc(items, sizeof(Value*) * cap);
        }
        items[count++] = value_string(entry->d_name);
    }
    
    closedir(dir);
    // FIX: value_array() takes ownership of `items` — do NOT free it here.
    // The old code called free(items) after passing it to value_array(), which
    // freed memory still referenced by the returned Value's ->array field.
    return value_array(items, count);
}

static Value *os_filesize(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    struct stat st;
    if (stat(args[0]->str, &st) != 0) return value_number(-1);
    return value_number((double)st.st_size);
}

static Value *os_getpid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getpid());
}

static Value *os_sleep(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    double seconds = args[0]->num;
    if (seconds > 0) {
        usleep((useconds_t)(seconds * 1000000));
    }
    return value_null();
}

static Value *os_platform(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_string(XLY_PLATFORM_NAME);
}

static NativeFunc os_fns[] = {
    // Process
    { "exit",     os_exit },
    { "getpid",   os_getpid },
    { "sleep",    os_sleep },
    { "platform", os_platform },
    // Time
    { "time",     os_time },
    { "clock",    os_clock },
    // Environment
    { "env",      os_env },
    { "cwd",      os_cwd },
    // Filesystem - query
    { "exists",   os_exists },
    { "isFile",   os_isFile },
    { "isDir",    os_isDir },
    { "filesize", os_filesize },
    { "listdir",  os_listdir },
    // Filesystem - modify
    { "mkdir",    os_mkdir },
    { "rmdir",    os_rmdir },
    { "remove",   os_remove },
    { "rename",   os_rename },
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
// CRYPTO MODULE  — cryptographic constants and hashing
// ═════════════════════════════════════════════════════════════════════════════

// Simple hash functions for basic cryptographic needs
static Value *crypto_hash(Value **args, size_t argc) {
    // Simple DJB2 hash algorithm
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    const char *str = args[0]->str;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + (unsigned long)c; // hash * 33 + c
    return value_number((double)hash);
}

static Value *crypto_fnv1a(Value **args, size_t argc) {
    // FNV-1a hash algorithm
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    const char *str = args[0]->str;
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint32_t)(unsigned char)(*str++);
        hash *= 16777619u;
    }
    return value_number((double)hash);
}

static Value *crypto_murmur3(Value **args, size_t argc) {
    // Simplified MurmurHash3 for 32-bit
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    const char *key = args[0]->str;
    uint32_t seed = (argc >= 2) ? (uint32_t)args[1]->num : 0;
    
    size_t len = strlen(key);
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    
    uint32_t h1 = seed;
    const uint32_t *blocks = (const uint32_t *)(key);
    size_t nblocks = len / 4;
    
    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k1 = blocks[i];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64;
    }
    
    // Handle tail
    const uint8_t *tail = (const uint8_t *)(key + nblocks * 4);
    uint32_t k1 = 0;
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16; // fallthrough
        case 2: k1 ^= tail[1] << 8;  // fallthrough
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
    }
    
    // Finalization
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return value_number((double)h1);
}

static Value *crypto_checksum(Value **args, size_t argc) {
    // Simple checksum (sum of bytes mod 256)
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(0);
    const char *str = args[0]->str;
    unsigned int sum = 0;
    while (*str) sum += (unsigned char)(*str++);
    return value_number((double)(sum & 0xFF));
}

static Value *crypto_randomBytes(Value **args, size_t argc) {
    // Generate random bytes as hex string
    size_t count = (argc >= 1) ? (size_t)args[0]->num : 16;
    if (count > 1024) count = 1024; // Safety limit
    
    char *hex = malloc(count * 2 + 1);
    for (size_t i = 0; i < count; i++) {
        unsigned char byte = (unsigned char)(rand() % 256);
        snprintf(hex + i * 2, 3, "%02x", byte);
    }
    hex[count * 2] = '\0';
    
    Value *result = value_string(hex);
    free(hex);
    return result;
}

static Value *crypto_uuid(Value **args, size_t argc) {
    (void)args; (void)argc;
    // Generate UUID v4 (random)
    char uuid[37];
    snprintf(uuid, sizeof(uuid),
        "%08x-%04x-4%03x-%04x-%012lx",
        (unsigned int)rand(),
        (unsigned int)(rand() & 0xFFFF),
        (unsigned int)(rand() & 0xFFF),
        (unsigned int)((rand() & 0x3FFF) | 0x8000),
        (unsigned long)(((unsigned long)rand() << 32) | (unsigned long)rand()) & 0xFFFFFFFFFFFFUL
    );
    return value_string(uuid);
}

// Cryptographic constants
static Value *crypto_MD5_SIZE(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(16);
}

static Value *crypto_SHA1_SIZE(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(20);
}

static Value *crypto_SHA256_SIZE(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number(32);
}

static NativeFunc crypto_fns[] = {
    // Hash functions
    { "hash",      crypto_hash },
    { "fnv1a",     crypto_fnv1a },
    { "murmur3",   crypto_murmur3 },
    { "checksum",  crypto_checksum },
    // Random
    { "randomBytes", crypto_randomBytes },
    { "uuid",      crypto_uuid },
    // Constants
    { "MD5_SIZE",    crypto_MD5_SIZE },
    { "SHA1_SIZE",   crypto_SHA1_SIZE },
    { "SHA256_SIZE", crypto_SHA256_SIZE },
    { NULL, NULL }
};

Module module_crypto(void) {
    Module m;
    m.name      = NULL;
    m.functions = crypto_fns;
    m.fn_count  = sizeof(crypto_fns)/sizeof(crypto_fns[0]) - 1;
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// PATH MODULE  — path manipulation utilities
// ═════════════════════════════════════════════════════════════════════════════

static Value *path_join(Value **args, size_t argc) {
    // Join path components with /
    if (argc == 0) return value_string("");
    
    size_t total_len = 0;
    for (size_t i = 0; i < argc; i++) {
        if (args[i]->type == VAL_STRING)
            total_len += strlen(args[i]->str) + 1; // +1 for separator
    }
    
    char *result = malloc(total_len + 1);
    result[0] = '\0';
    
    for (size_t i = 0; i < argc; i++) {
        if (args[i]->type != VAL_STRING) continue;
        const char *part = args[i]->str;
        if (!part || !*part) continue;
        
        // Add separator if needed
        size_t len = strlen(result);
        if (len > 0 && result[len-1] != '/') {
            strcat(result, "/");
        }
        
        // Skip leading / from part if result already has content
        if (len > 0 && *part == '/') part++;
        
        strcat(result, part);
    }
    
    Value *v = value_string(result);
    free(result);
    return v;
}

static Value *path_basename(Value **args, size_t argc) {
    // Get the final component of a path
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    const char *path = args[0]->str;
    
    // Remove trailing slashes
    size_t len = strlen(path);
    while (len > 0 && path[len-1] == '/') len--;
    if (len == 0) return value_string("/");
    
    // Find last /
    const char *last_slash = NULL;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/') last_slash = path + i;
    }
    
    if (!last_slash) {
        char *result = malloc(len + 1);
        memcpy(result, path, len);
        result[len] = '\0';
        Value *v = value_string(result);
        free(result);
        return v;
    }
    
    const char *base = last_slash + 1;
    size_t base_len = len - (size_t)(base - path);
    char *result = malloc(base_len + 1);
    memcpy(result, base, base_len);
    result[base_len] = '\0';
    Value *v = value_string(result);
    free(result);
    return v;
}

static Value *path_dirname(Value **args, size_t argc) {
    // Get directory part of a path
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string(".");
    const char *path = args[0]->str;
    
    // Remove trailing slashes
    size_t len = strlen(path);
    while (len > 0 && path[len-1] == '/') len--;
    if (len == 0) return value_string("/");
    
    // Find last /
    const char *last_slash = NULL;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/') last_slash = path + i;
    }
    
    if (!last_slash) return value_string(".");
    if (last_slash == path) return value_string("/");
    
    size_t dir_len = (size_t)(last_slash - path);
    char *result = malloc(dir_len + 1);
    memcpy(result, path, dir_len);
    result[dir_len] = '\0';
    Value *v = value_string(result);
    free(result);
    return v;
}

static Value *path_ext(Value **args, size_t argc) {
    // Get file extension
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string("");
    const char *path = args[0]->str;
    
    // Find last . after last /
    const char *last_slash = strrchr(path, '/');
    const char *last_dot = strrchr(path, '.');
    
    if (!last_dot || (last_slash && last_dot < last_slash))
        return value_string("");
    
    return value_string(last_dot);
}

static Value *path_isAbs(Value **args, size_t argc) {
    // Check if path is absolute
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    const char *path = args[0]->str;
    return value_bool(path[0] == '/');
}

static Value *path_clean(Value **args, size_t argc) {
    // Clean up path (remove .., ., etc.)
    if (argc < 1 || args[0]->type != VAL_STRING) return value_string(".");
    const char *path = args[0]->str;
    
    if (!*path) return value_string(".");
    
    // Simple cleaning: remove redundant slashes and trailing slashes
    size_t len = strlen(path);
    char *result = malloc(len + 2);
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/') {
            // Skip multiple slashes
            if (j == 0 || result[j-1] != '/') {
                result[j++] = '/';
            }
        } else {
            result[j++] = path[i];
        }
    }
    
    // Remove trailing slash (unless it's root)
    if (j > 1 && result[j-1] == '/') j--;
    
    result[j] = '\0';
    if (j == 0) strcpy(result, ".");
    
    Value *v = value_string(result);
    free(result);
    return v;
}

static Value *path_split(Value **args, size_t argc) {
    // Split path into [dir, base]
    // NOTE: value_array() takes OWNERSHIP of both the parts[] array pointer and
    // the Value* pointers inside it — it does NOT clone them.  Do not free parts[]
    // or the Values after calling value_array().
    if (argc < 1 || args[0]->type != VAL_STRING) {
        Value **parts = malloc(sizeof(Value*) * 2);
        parts[0] = value_string(".");
        parts[1] = value_string("");
        return value_array(parts, 2);   // takes full ownership of parts and its elements
    }

    Value *dir  = path_dirname(args, argc);    // freshly allocated
    Value *base = path_basename(args, argc);   // freshly allocated

    Value **parts = malloc(sizeof(Value*) * 2);
    parts[0] = dir;    // ownership transferred to parts[]
    parts[1] = base;   // ownership transferred to parts[]
    return value_array(parts, 2);   // takes full ownership of parts[] and dir/base
}

static NativeFunc path_fns[] = {
    { "join",     path_join },
    { "basename", path_basename },
    { "dirname",  path_dirname },
    { "ext",      path_ext },
    { "isAbs",    path_isAbs },
    { "clean",    path_clean },
    { "split",    path_split },
    { NULL, NULL }
};

Module module_path(void) {
    Module m;
    m.name      = NULL;
    m.functions = path_fns;
    m.fn_count  = sizeof(path_fns)/sizeof(path_fns[0]) - 1;
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
        case VAL_ENUM_VARIANT: return value_string(args[0]->variant.tag);
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

// ═════════════════════════════════════════════════════════════════════════════
// MULTIPROC MODULE  — high-performance multiprocessing & concurrency
// ═════════════════════════════════════════════════════════════════════════════
#include "multiproc.h"

static Value *mp_cpu_count(Value **args, size_t argc) {
    (void)args; (void)argc;
    long nproc = xly_cpu_count();
    return value_number((double)nproc);
}

#ifndef XENLY_NO_MULTIPROC
// ─── Multiprocessing Functions (require interpreter, not in compiled code) ───

static Value *mp_channel_create(Value **args, size_t argc) {
    size_t capacity = 0;
    if (argc > 0 && args[0]->type == VAL_NUMBER) {
        capacity = (size_t)args[0]->num;
    }
    Channel *chan = channel_create(capacity);
    return value_number((double)(uintptr_t)chan);
}

static Value *mp_channel_send(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    // Make a copy of the value to send (channel takes ownership)
    Value *original = args[1];
    Value *copy = NULL;
    
    switch (original->type) {
        case VAL_NUMBER:
            copy = value_number(original->num);
            break;
        case VAL_STRING:
            copy = value_string(original->str);
            break;
        case VAL_BOOL:
            copy = value_bool(original->boolean);
            break;
        case VAL_NULL:
            copy = value_null();
            break;
        default:
            // Complex types (functions, objects) - store reference
            copy = original;
            break;
    }
    
    int result = channel_send(chan, copy);
    return value_number((double)result);
}

static Value *mp_channel_recv(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    return channel_recv(chan);
}

static Value *mp_channel_close(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    channel_close(chan);
    return value_null();
}

static Value *mp_channel_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    channel_destroy(chan);
    return value_null();
}

static Value *mp_thread_pool_create(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    size_t num_workers = (size_t)args[0]->num;
    ThreadPool *pool = thread_pool_create(num_workers);
    return value_number((double)(uintptr_t)pool);
}

static Value *mp_thread_pool_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    ThreadPool *pool = (ThreadPool *)(uintptr_t)args[0]->num;
    thread_pool_destroy(pool);
    return value_null();
}

static Value *mp_process_pool_create(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    size_t num_workers = (size_t)args[0]->num;
    ProcessPool *pool = process_pool_create(num_workers);
    return value_number((double)(uintptr_t)pool);
}

static Value *mp_process_pool_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    ProcessPool *pool = (ProcessPool *)(uintptr_t)args[0]->num;
    process_pool_destroy(pool);
    return value_null();
}

static Value *mp_future_get(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Future *fut = (Future *)(uintptr_t)args[0]->num;
    return future_get(fut);
}

static Value *mp_future_is_ready(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    Future *fut = (Future *)(uintptr_t)args[0]->num;
    return value_bool(future_is_ready(fut));
}

static Value *mp_future_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Future *fut = (Future *)(uintptr_t)args[0]->num;
    future_destroy(fut);
    return value_null();
}

#endif // XENLY_NO_MULTIPROC

static NativeFunc multiproc_fns[] = {
    { "cpu_count",          mp_cpu_count },
#ifndef XENLY_NO_MULTIPROC
    { "channel_create",     mp_channel_create },
    { "channel_send",       mp_channel_send },
    { "channel_recv",       mp_channel_recv },
    { "channel_close",      mp_channel_close },
    { "channel_destroy",    mp_channel_destroy },
    { "thread_pool_create", mp_thread_pool_create },
    { "thread_pool_destroy", mp_thread_pool_destroy },
    { "process_pool_create", mp_process_pool_create },
    { "process_pool_destroy", mp_process_pool_destroy },
    { "future_get",         mp_future_get },
    { "future_is_ready",    mp_future_is_ready },
    { "future_destroy",     mp_future_destroy },
#endif // XENLY_NO_MULTIPROC
    { NULL, NULL }
};

Module module_multiproc(void) {
    Module m;
    m.name      = NULL;
    m.functions = multiproc_fns;
    m.fn_count  = sizeof(multiproc_fns)/sizeof(multiproc_fns[0]) - 1;
    return m;
}


// ═════════════════════════════════════════════════════════════════════════════
// SYS MODULE  — Full-spectrum C system programming interface for Xenly
//
// This module brings the power of C system programming directly into the
// Xenly language. It covers every layer a systems engineer needs:
//
//  LEVEL 1 — Process & Control:  fork, exec, wait, kill, signals, rlimits
//  LEVEL 2 — File & I/O:         open/read/write/close, seek, mmap, poll/select
//  LEVEL 3 — Filesystem:         stat, chmod, chown, symlink, readlink, fsync
//  LEVEL 4 — Networking:         socket, bind, connect, listen, accept, send/recv
//  LEVEL 5 — Time & Clocks:      clock_gettime (monotonic, realtime), nanosleep
//  LEVEL 6 — System Info:        uname, sysconf, rlimit, /proc/self/status
//  LEVEL 7 — Bit & Memory:       band/bor/bxor/bnot/shl/shr, popcnt/clz/ctz
//  LEVEL 8 — IPC:                pipe, mkfifo, syslog
//  LEVEL 9 — File Locking:       fcntl advisory locks (POSIX)
//  LEVEL 10 — Constants:         STDIN/STDOUT/STDERR, INT_MAX, signal numbers
//
// Inspired by C — the language behind Linux, macOS, BSD, embedded firmware,
// compilers (GCC, LLVM), databases (SQLite), and the Xenly runtime itself.
//
// Usage in Xenly:
//   import "sys"
//   sys.exit(0)
//   sys.write(sys.STDOUT(), "hello\n")
//   const fd = sys.open("/etc/hosts", sys.O_RDONLY())
//   const t  = sys.clock_monotonic()   // [seconds, nanoseconds]
//   sys.syslog(sys.LOG_INFO(), "started")
// ═════════════════════════════════════════════════════════════════════════════
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <syslog.h>
#include <time.h>

/* ── macOS portability shims ─────────────────────────────────────────────────
 * Problems under -D_POSIX_C_SOURCE=200809L on macOS (Xcode clang):
 *
 *  1. MAP_ANON / MAP_ANONYMOUS:
 *     <sys/mman.h> only exposes MAP_ANON under _DARWIN_C_SOURCE, which our
 *     strict POSIX flag suppresses.  Hard-code the well-known value 0x1000.
 *
 *  2. sys/sysctl.h conflict:
 *     Including <sys/sysctl.h> under strict _POSIX_C_SOURCE pulls in
 *     <sys/ucred.h> and <sys/proc.h>, which use BSD-only typedefs (u_int,
 *     u_char, u_short) that are also stripped by the strict mode → cascade
 *     of "unknown type name" errors.  Solution: do NOT include <sys/sysctl.h>;
 *     instead forward-declare only the one function we need (sysctlbyname)
 *     with plain C types — this matches its actual ABI on all macOS versions.
 *
 *  3. _SC_NPROCESSORS_ONLN / _CONF, _SC_PHYS_PAGES, _SC_AVPHYS_PAGES:
 *     These GNU/Linux sysconf constants don't exist on macOS.  We replace
 *     them with sysctlbyname("hw.logicalcpu") for CPU counts, and
 *     sysctlbyname("hw.memsize") + sysconf(_SC_PAGE_SIZE) for RAM pages.
 *     Free pages come from /usr/lib/libSystem (host_statistics64 via the
 *     forward-declared Mach call — no <mach/mach.h> needed).
 * ─────────────────────────────────────────────────────────────────────────── */
#if defined(PLATFORM_MACOS)

   /* ── MAP_ANONYMOUS ─────────────────────────────────────────────────────── */
   /* Hard-code 0x1000 — MAP_ANON has had this value on every macOS / Darwin
    * release since 10.0.  We avoid referencing MAP_ANON by name because it
    * isn't visible under strict _POSIX_C_SOURCE. */
#  ifndef MAP_ANONYMOUS
#    define MAP_ANONYMOUS 0x1000
#  endif

   /* ── sysctlbyname forward declaration ──────────────────────────────────── */
   /* We declare it ourselves instead of including <sys/sysctl.h> to dodge the
    * BSD-typedef cascade.  The prototype matches the real libSystem symbol. */
   extern int sysctlbyname(const char *name,
                           void *oldp, size_t *oldlenp,
                           void *newp, size_t newlen);

   /* ── host_statistics64 forward declaration ─────────────────────────────── */
   /* Sufficient to call it without pulling in all of <mach/mach.h>. */
   typedef unsigned int  xly_mach_port_t;
   typedef int           xly_kern_return_t;
   typedef unsigned int  xly_mach_msg_type_number_t;
   typedef int *         xly_host_info64_t;
#  define XLY_HOST_VM_INFO64       ((int)4)
#  define XLY_HOST_VM_INFO64_COUNT ((xly_mach_msg_type_number_t)(sizeof(struct xly_vm_statistics64) / sizeof(int)))
#  define XLY_KERN_SUCCESS         0
   struct xly_vm_statistics64 {
       unsigned int free_count;
       unsigned int active_count;
       unsigned int inactive_count;
       unsigned int wire_count;
       /* remaining fields unused — we only read free/inactive */
       unsigned int zero_fill_count;
       unsigned int reactivations;
       unsigned int pageins;
       unsigned int pageouts;
       unsigned int faults;
       unsigned int cow_faults;
       unsigned int lookups;
       unsigned int hits;
       unsigned int purges;
       unsigned int purgeable_count;
       unsigned int speculative_count;
       unsigned int decompressions;
       unsigned int compressions;
       unsigned int swapins;
       unsigned int swapouts;
       unsigned int compressor_page_count;
       unsigned int throttled_count;
       unsigned int external_page_count;
       unsigned int internal_page_count;
       unsigned long long total_uncompressed_pages_in_compressor;
   };
   extern xly_kern_return_t host_statistics64(xly_mach_port_t host,
                                              int flavor,
                                              xly_host_info64_t info,
                                              xly_mach_msg_type_number_t *count);
   extern xly_mach_port_t   mach_host_self(void);

   /* ── helper implementations ────────────────────────────────────────────── */
   static inline long xly_nprocessors_onln(void) {
       int n = 0; size_t sz = sizeof(n);
       return (sysctlbyname("hw.logicalcpu", &n, &sz, NULL, 0) == 0 && n > 0)
              ? (long)n : 1L;
   }
   static inline long xly_nprocessors_conf(void) {
       int n = 0; size_t sz = sizeof(n);
       return (sysctlbyname("hw.logicalcpu_max", &n, &sz, NULL, 0) == 0 && n > 0)
              ? (long)n : 1L;
   }
#  ifndef _SC_PAGE_SIZE
#    define _SC_PAGE_SIZE 29
#  endif
   static inline long xly_phys_pages(void) {
       unsigned long long mem = 0; size_t sz = sizeof(mem);
       if (sysctlbyname("hw.memsize", &mem, &sz, NULL, 0) != 0) return 0;
       long ps = sysconf(_SC_PAGE_SIZE);
       if (ps <= 0) ps = 4096;
       return (long)(mem / (unsigned long long)ps);
   }
   static inline long xly_avphys_pages(void) {
       xly_mach_port_t host = mach_host_self();
       struct xly_vm_statistics64 vms;
       xly_mach_msg_type_number_t cnt = XLY_HOST_VM_INFO64_COUNT;
       if (host_statistics64(host, XLY_HOST_VM_INFO64,
                             (xly_host_info64_t)&vms, &cnt) != XLY_KERN_SUCCESS)
           return 0;
       return (long)(vms.free_count + vms.inactive_count);
   }

#else  /* Linux / BSD */

   static inline long xly_nprocessors_onln(void) {
       long n = sysconf(_SC_NPROCESSORS_ONLN); return n > 0 ? n : 1;
   }
   static inline long xly_nprocessors_conf(void) {
       long n = sysconf(_SC_NPROCESSORS_CONF); return n > 0 ? n : 1;
   }
   static inline long xly_phys_pages(void) {
       long p = sysconf(_SC_PHYS_PAGES); return p > 0 ? p : 0;
   }
   static inline long xly_avphys_pages(void) {
       long p = sysconf(_SC_AVPHYS_PAGES); return p > 0 ? p : 0;
   }

#endif /* PLATFORM_MACOS */

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 1 — PROCESS CONTROL
// ═════════════════════════════════════════════════════════════════════════════

// sys.exit(code) — terminate process with exit code (C: exit())
static Value *sys_exit(Value **args, size_t argc) {
    int code = (argc >= 1 && args[0]->type == VAL_NUMBER) ? (int)args[0]->num : 0;
    exit(code);
    return value_null();
}

// sys.abort() — abnormal termination, generates SIGABRT / core dump (C: abort())
static Value *sys_abort(Value **args, size_t argc) {
    (void)args; (void)argc;
    abort();
    return value_null();
}

// sys.getpid() — current process ID (C: getpid())
static Value *sys_getpid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getpid());
}

// sys.getppid() — parent process ID (C: getppid())
static Value *sys_getppid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getppid());
}

// sys.fork() — fork process; returns child PID in parent, 0 in child (C: fork())
static Value *sys_fork(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)fork());
}

// sys.exec(path, args_array) — replace process image (C: execv())
static Value *sys_exec(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    const char *path = args[0]->str;
    char **argv_list;
    size_t argv_count;
    if (argc >= 2 && args[1]->type == VAL_ARRAY) {
        argv_count = args[1]->array_len + 2;
        argv_list  = (char **)malloc(sizeof(char *) * argv_count);
        argv_list[0] = (char *)path;
        for (size_t i = 0; i < args[1]->array_len; i++)
            argv_list[i + 1] = (args[1]->array[i]->type == VAL_STRING) ? args[1]->array[i]->str : "";
        argv_list[argv_count - 1] = NULL;
    } else {
        argv_list = (char **)malloc(sizeof(char *) * 2);
        argv_list[0] = (char *)path;
        argv_list[1] = NULL;
    }
    execv(path, argv_list);
    int err = errno;
    free(argv_list);
    return value_number((double)(-err));
}

// sys.wait() — wait for any child, returns its PID (C: wait())
static Value *sys_wait(Value **args, size_t argc) {
    (void)args; (void)argc;
    int status = 0;
    return value_number((double)wait(&status));
}

// sys.waitpid(pid) — wait for specific child PID (C: waitpid())
static Value *sys_waitpid(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    int status = 0;
    return value_number((double)waitpid((pid_t)args[0]->num, &status, 0));
}

// sys.kill(pid, sig) — send signal to process (C: kill())
// Common: SIGHUP=1, SIGINT=2, SIGKILL=9, SIGTERM=15, SIGUSR1=10, SIGUSR2=12
static Value *sys_kill(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    return value_number((double)kill((pid_t)args[0]->num, (int)args[1]->num));
}

// sys.raise(sig) — send signal to current process (C: raise())
static Value *sys_raise(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)raise((int)args[0]->num));
}

// sys.getrlimit(resource) — get resource limit (C: getrlimit())
// resource: 0=CPU, 1=FSIZE, 2=DATA, 3=STACK, 4=CORE, 5=RSS, 7=NOFILE, 9=AS
// Returns [soft_limit, hard_limit] (-1 = unlimited)
static Value *sys_getrlimit(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    struct rlimit rl;
    if (getrlimit((int)args[0]->num, &rl) < 0) return value_null();
    Value **elems = (Value **)malloc(sizeof(Value *) * 2);
    elems[0] = value_number(rl.rlim_cur == RLIM_INFINITY ? -1.0 : (double)rl.rlim_cur);
    elems[1] = value_number(rl.rlim_max == RLIM_INFINITY ? -1.0 : (double)rl.rlim_max);
    return value_array(elems, 2);
}

// sys.setrlimit(resource, soft, hard) — set resource limit (C: setrlimit())
// Pass -1 for unlimited. Returns 0 on success, -1 on error.
static Value *sys_setrlimit(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_number(-1);
    struct rlimit rl;
    double soft = args[1]->num, hard = args[2]->num;
    rl.rlim_cur = (soft < 0) ? RLIM_INFINITY : (rlim_t)soft;
    rl.rlim_max = (hard < 0) ? RLIM_INFINITY : (rlim_t)hard;
    return value_number((double)setrlimit((int)args[0]->num, &rl));
}

// sys.getuid() — current user ID (C: getuid())
static Value *sys_getuid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getuid());
}

// sys.getgid() — current group ID (C: getgid())
static Value *sys_getgid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getgid());
}

// sys.geteuid() — effective user ID (C: geteuid())
static Value *sys_geteuid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)geteuid());
}

// sys.getegid() — effective group ID (C: getegid())
static Value *sys_getegid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getegid());
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 2 — FILE DESCRIPTOR I/O & MEMORY-MAPPED FILES
// ═════════════════════════════════════════════════════════════════════════════

// sys.open(path, flags, mode) — open file descriptor (C: open())
// flags: use sys.O_RDONLY(), sys.O_WRONLY(), sys.O_RDWR(), sys.O_CREAT(), sys.O_TRUNC()
static Value *sys_open(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    int    flags = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : O_RDONLY;
    mode_t mode  = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (mode_t)args[2]->num : 0644;
    return value_number((double)open(args[0]->str, flags, mode));
}

// sys.close(fd) — close file descriptor (C: close())
static Value *sys_close(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)close((int)args[0]->num));
}

// sys.read(fd, count) — read up to count bytes (C: read()), returns string or null
static Value *sys_read(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_NUMBER) return value_null();
    size_t count = (size_t)args[1]->num;
    if (count == 0 || count > 67108864) return value_null(); // cap 64 MB
    char *buf = (char *)malloc(count + 1);
    if (!buf) return value_null();
    ssize_t n = read((int)args[0]->num, buf, count);
    if (n < 0) { free(buf); return value_null(); }
    buf[n] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// sys.write(fd, data) — write string to fd (C: write()), returns bytes written
static Value *sys_write(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    int fd = (int)args[0]->num;
    if (args[1]->type == VAL_STRING) {
        const char *s = args[1]->str;
        return value_number((double)write(fd, s, strlen(s)));
    }
    char *s = value_to_string(args[1]);
    ssize_t n = write(fd, s, strlen(s));
    free(s);
    return value_number((double)n);
}

// sys.seek(fd, offset, whence) — reposition file offset (C: lseek())
// whence: 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END
static Value *sys_seek(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)lseek((int)args[0]->num, (off_t)args[1]->num, (int)args[2]->num));
}

// sys.dup(fd) — duplicate fd (C: dup())
static Value *sys_dup(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)dup((int)args[0]->num));
}

// sys.dup2(old, new) — duplicate old fd as new fd (C: dup2())
static Value *sys_dup2(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    return value_number((double)dup2((int)args[0]->num, (int)args[1]->num));
}

// sys.pipe() — create pipe, returns [read_fd, write_fd] (C: pipe())
static Value *sys_pipe(Value **args, size_t argc) {
    (void)args; (void)argc;
    int fds[2];
    if (pipe(fds) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)fds[0]);
    e[1] = value_number((double)fds[1]);
    return value_array(e, 2);
}

// sys.fsync(fd) — flush fd's kernel buffer to hardware (C: fsync())
// Essential for databases and transactional writes
static Value *sys_fsync(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fsync((int)args[0]->num));
}

// sys.fdatasync(fd) — flush data only, no metadata (C: fdatasync())
static Value *sys_fdatasync(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fdatasync((int)args[0]->num));
}

// sys.truncate(path, length) — truncate file to length bytes (C: truncate())
static Value *sys_truncate(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)truncate(args[0]->str, (off_t)args[1]->num));
}

// sys.ftruncate(fd, length) — truncate open file to length bytes (C: ftruncate())
static Value *sys_ftruncate(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)ftruncate((int)args[0]->num, (off_t)args[1]->num));
}

// sys.mmap(fd, size, prot, flags, offset) — memory-map a file or anonymous region
// Returns an opaque handle (pointer as number) or -1 on error.
// prot:  1=PROT_READ, 2=PROT_WRITE, 3=PROT_READ|PROT_WRITE
// flags: 1=MAP_SHARED, 2=MAP_PRIVATE, 34=MAP_PRIVATE|MAP_ANONYMOUS (fd=-1 for anon)
// Essential for shared memory, zero-copy I/O, fast file access.
static Value *sys_mmap(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    size_t size  = (size_t)args[0]->num;
    int    prot  = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : PROT_READ | PROT_WRITE;
    int    flags = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : MAP_PRIVATE | MAP_ANONYMOUS;  /* MAP_ANONYMOUS shim covers macOS MAP_ANON */
    int    fd    = (argc >= 4 && args[3]->type == VAL_NUMBER) ? (int)args[3]->num : -1;
    off_t  off   = (argc >= 5 && args[4]->type == VAL_NUMBER) ? (off_t)args[4]->num : 0;
    void  *ptr   = mmap(NULL, size, prot, flags, fd, off);
    if (ptr == MAP_FAILED) return value_number(-1);
    return value_number((double)(uintptr_t)ptr);
}

// sys.munmap(ptr, size) — unmap a memory-mapped region (C: munmap())
static Value *sys_munmap(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    void  *ptr  = (void *)(uintptr_t)(long long)args[0]->num;
    size_t size = (size_t)args[1]->num;
    return value_number((double)munmap(ptr, size));
}

// sys.mmap_read(ptr, offset, count) — read bytes from mmap region into string
static Value *sys_mmap_read(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_null();
    uint8_t *base   = (uint8_t *)(uintptr_t)(long long)args[0]->num;
    size_t   offset = (size_t)args[1]->num;
    size_t   count  = (size_t)args[2]->num;
    if (count == 0 || count > 67108864) return value_null();
    char *buf = (char *)malloc(count + 1);
    if (!buf) return value_null();
    memcpy(buf, base + offset, count);
    buf[count] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// sys.mmap_write(ptr, offset, data) — write string into mmap region
static Value *sys_mmap_write(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER || args[2]->type != VAL_STRING) return value_number(-1);
    uint8_t    *base   = (uint8_t *)(uintptr_t)(long long)args[0]->num;
    size_t      offset = (size_t)args[1]->num;
    const char *data   = args[2]->str;
    size_t      len    = strlen(data);
    memcpy(base + offset, data, len);
    return value_number((double)len);
}

// sys.poll(fds_array, timeout_ms) — wait for I/O events (C: poll())
// fds_array: array of [fd, events] pairs. events: 1=POLLIN, 4=POLLOUT
// Returns number of ready fds, or -1 on error. Timeout -1 = block forever.
static Value *sys_poll(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_number(-1);
    int timeout = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : -1;
    size_t nfds = args[0]->array_len;
    struct pollfd *pfds = (struct pollfd *)calloc(nfds, sizeof(struct pollfd));
    if (!pfds) return value_number(-1);
    for (size_t i = 0; i < nfds; i++) {
        Value *pair = args[0]->array[i];
        if (pair->type == VAL_ARRAY && pair->array_len >= 2) {
            pfds[i].fd     = (int)pair->array[0]->num;
            pfds[i].events = (short)pair->array[1]->num;
        }
    }
    int ret = poll(pfds, (nfds_t)nfds, timeout);
    free(pfds);
    return value_number((double)ret);
}

// sys.fcntl_lock(fd, type) — advisory file lock (C: fcntl() F_SETLK)
// type: 0=F_RDLCK (shared), 1=F_WRLCK (exclusive), 2=F_UNLCK (release)
// Used by databases (SQLite, PostgreSQL) to coordinate concurrent access.
static Value *sys_fcntl_lock(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    int t = (int)args[1]->num;
    fl.l_type   = (t == 0) ? F_RDLCK : (t == 1) ? F_WRLCK : F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0; // entire file
    return value_number((double)fcntl((int)args[0]->num, F_SETLK, &fl));
}

// sys.fcntl_setfl(fd, flags) — set file status flags (C: fcntl() F_SETFL)
// e.g. set O_NONBLOCK: sys.fcntl_setfl(fd, sys.O_NONBLOCK())
static Value *sys_fcntl_setfl(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fcntl((int)args[0]->num, F_SETFL, (int)args[1]->num));
}

// sys.fcntl_getfl(fd) — get file status flags (C: fcntl() F_GETFL)
static Value *sys_fcntl_getfl(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fcntl((int)args[0]->num, F_GETFL, 0));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 3 — FILESYSTEM
// ═════════════════════════════════════════════════════════════════════════════

// sys.stat(path) — file metadata (C: stat())
// Returns [size, mode, uid, gid, mtime, atime, nlinks] or null
static Value *sys_stat(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    struct stat st;
    if (stat(args[0]->str, &st) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 7);
    e[0] = value_number((double)st.st_size);
    e[1] = value_number((double)st.st_mode);
    e[2] = value_number((double)st.st_uid);
    e[3] = value_number((double)st.st_gid);
    e[4] = value_number((double)st.st_mtime);
    e[5] = value_number((double)st.st_atime);
    e[6] = value_number((double)st.st_nlink);
    return value_array(e, 7);
}

// sys.lstat(path) — like stat() but for symlinks themselves (C: lstat())
static Value *sys_lstat(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    struct stat st;
    if (lstat(args[0]->str, &st) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 7);
    e[0] = value_number((double)st.st_size);
    e[1] = value_number((double)st.st_mode);
    e[2] = value_number((double)st.st_uid);
    e[3] = value_number((double)st.st_gid);
    e[4] = value_number((double)st.st_mtime);
    e[5] = value_number((double)st.st_atime);
    e[6] = value_number((double)st.st_nlink);
    return value_array(e, 7);
}

// sys.isfile(path) — true if path is a regular file
static Value *sys_isfile(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(stat(args[0]->str, &st) == 0 && S_ISREG(st.st_mode));
}

// sys.isdir(path) — true if path is a directory
static Value *sys_isdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(stat(args[0]->str, &st) == 0 && S_ISDIR(st.st_mode));
}

// sys.islnk(path) — true if path is a symbolic link
static Value *sys_islnk(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(lstat(args[0]->str, &st) == 0 && S_ISLNK(st.st_mode));
}

// sys.access(path, mode) — check file accessibility (C: access())
// mode: 0=F_OK(exists), 1=X_OK(exec), 2=W_OK(write), 4=R_OK(read)
static Value *sys_access(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)access(args[0]->str, (int)args[1]->num));
}

// sys.mkdir(path, mode) — create directory (C: mkdir())
static Value *sys_mkdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    mode_t m = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (mode_t)args[1]->num : 0755;
    return value_number((double)mkdir(args[0]->str, m));
}

// sys.rmdir(path) — remove empty directory (C: rmdir())
static Value *sys_rmdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)rmdir(args[0]->str));
}

// sys.unlink(path) — delete a file (C: unlink())
static Value *sys_unlink(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)unlink(args[0]->str));
}

// sys.rename(old, new) — rename/move file (C: rename())
static Value *sys_rename(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)rename(args[0]->str, args[1]->str));
}

// sys.link(old, new) — create hard link (C: link())
// Both names refer to the same inode — used by version control, atomic replace.
static Value *sys_link(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)link(args[0]->str, args[1]->str));
}

// sys.symlink(target, linkname) — create symbolic link (C: symlink())
static Value *sys_symlink(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)symlink(args[0]->str, args[1]->str));
}

// sys.readlink(path) — read symbolic link target (C: readlink())
static Value *sys_readlink(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    char buf[4096];
    ssize_t n = readlink(args[0]->str, buf, sizeof(buf) - 1);
    if (n < 0) return value_null();
    buf[n] = '\0';
    return value_string(buf);
}

// sys.chmod(path, mode) — change file permissions (C: chmod())
// mode is octal e.g. 0644=420, 0755=493
static Value *sys_chmod(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)chmod(args[0]->str, (mode_t)args[1]->num));
}

// sys.chown(path, uid, gid) — change file owner/group (C: chown())
// Pass -1 for uid or gid to leave unchanged.
static Value *sys_chown(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_STRING) return value_number(-1);
    uid_t uid = (uid_t)args[1]->num;
    gid_t gid = (gid_t)args[2]->num;
    return value_number((double)chown(args[0]->str, uid, gid));
}

// sys.getcwd() — current working directory (C: getcwd())
static Value *sys_getcwd(Value **args, size_t argc) {
    (void)args; (void)argc;
    char buf[4096];
    return getcwd(buf, sizeof(buf)) ? value_string(buf) : value_null();
}

// sys.chdir(path) — change working directory (C: chdir())
static Value *sys_chdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)chdir(args[0]->str));
}

// sys.mkfifo(path, mode) — create named pipe / FIFO (C: mkfifo())
// Named pipes allow unrelated processes to communicate via the filesystem.
static Value *sys_mkfifo(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    mode_t m = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (mode_t)args[1]->num : 0644;
    return value_number((double)mkfifo(args[0]->str, m));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 4 — NETWORKING (POSIX SOCKETS)
// ═════════════════════════════════════════════════════════════════════════════

// sys.socket(domain, type, proto) — create socket (C: socket())
// domain: 2=AF_INET (IPv4), 10=AF_INET6 (IPv6), 1=AF_UNIX
// type:   1=SOCK_STREAM (TCP), 2=SOCK_DGRAM (UDP)
// proto:  0=default
static Value *sys_socket(Value **args, size_t argc) {
    int domain = (argc >= 1 && args[0]->type == VAL_NUMBER) ? (int)args[0]->num : AF_INET;
    int type   = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : SOCK_STREAM;
    int proto  = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : 0;
    return value_number((double)socket(domain, type, proto));
}

// sys.connect(fd, ip, port) — connect TCP socket to ip:port (C: connect())
static Value *sys_connect(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_STRING) return value_number(-1);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((uint16_t)args[2]->num);
    if (inet_pton(AF_INET, args[1]->str, &addr.sin_addr) <= 0) return value_number(-1);
    return value_number((double)connect((int)args[0]->num, (struct sockaddr *)&addr, sizeof(addr)));
}

// sys.bind(fd, ip, port) — bind socket to address/port (C: bind())
// Use ip="" or "0.0.0.0" to bind to all interfaces.
static Value *sys_bind(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_number(-1);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((uint16_t)args[2]->num);
    addr.sin_addr.s_addr = (argc >= 2 && args[1]->type == VAL_STRING && strlen(args[1]->str) > 0)
                         ? inet_addr(args[1]->str) : INADDR_ANY;
    int optval = 1;
    setsockopt((int)args[0]->num, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return value_number((double)bind((int)args[0]->num, (struct sockaddr *)&addr, sizeof(addr)));
}

// sys.listen(fd, backlog) — mark socket as passive (server) (C: listen())
static Value *sys_listen(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)listen((int)args[0]->num, (int)args[1]->num));
}

// sys.accept(fd) — accept incoming connection, returns [client_fd, client_ip] (C: accept())
static Value *sys_accept(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);
    int cfd = accept((int)args[0]->num, (struct sockaddr *)&caddr, &clen);
    if (cfd < 0) return value_null();
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
    Value **e = (Value **)malloc(sizeof(Value *) * 3);
    e[0] = value_number((double)cfd);
    e[1] = value_string(ip);
    e[2] = value_number((double)ntohs(caddr.sin_port));
    return value_array(e, 3);
}

// sys.send(fd, data, flags) — send data on socket (C: send())
// flags: 0=none, 1=MSG_OOB, 2=MSG_DONTROUTE
static Value *sys_send(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_STRING) return value_number(-1);
    int   flags = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : 0;
    const char *s = args[1]->str;
    return value_number((double)send((int)args[0]->num, s, strlen(s), flags));
}

// sys.recv(fd, count, flags) — receive data from socket (C: recv())
static Value *sys_recv(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_NUMBER) return value_null();
    size_t count = (size_t)args[1]->num;
    if (count == 0 || count > 67108864) return value_null();
    int flags = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : 0;
    char *buf = (char *)malloc(count + 1);
    if (!buf) return value_null();
    ssize_t n = recv((int)args[0]->num, buf, count, flags);
    if (n < 0) { free(buf); return value_null(); }
    buf[n] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// sys.setsockopt(fd, level, optname, value) — set socket option (C: setsockopt())
// level: 1=SOL_SOCKET, 6=IPPROTO_TCP
// optname (SOL_SOCKET): 2=SO_REUSEADDR, 7=SO_KEEPALIVE, 8=SO_DONTROUTE
static Value *sys_setsockopt(Value **args, size_t argc) {
    if (argc < 4 || args[0]->type != VAL_NUMBER) return value_number(-1);
    int optval = (int)args[3]->num;
    return value_number((double)setsockopt((int)args[0]->num, (int)args[1]->num,
                                           (int)args[2]->num, &optval, sizeof(optval)));
}

// sys.inet_aton(ip) — convert IPv4 string to 32-bit integer (C: inet_addr())
static Value *sys_inet_aton(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)ntohl(inet_addr(args[0]->str)));
}

// sys.inet_ntoa(n) — convert 32-bit integer to IPv4 string (C: inet_ntoa())
static Value *sys_inet_ntoa(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    struct in_addr addr;
    addr.s_addr = htonl((uint32_t)args[0]->num);
    return value_string(inet_ntoa(addr));
}

// sys.htons(n) — host-to-network short (byte-swap if needed) (C: htons())
static Value *sys_htons(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)htons((uint16_t)args[0]->num));
}

// sys.ntohs(n) — network-to-host short (C: ntohs())
static Value *sys_ntohs(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)ntohs((uint16_t)args[0]->num));
}

// sys.htonl(n) — host-to-network long (C: htonl())
static Value *sys_htonl(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)htonl((uint32_t)args[0]->num));
}

// sys.ntohl(n) — network-to-host long (C: ntohl())
static Value *sys_ntohl(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)ntohl((uint32_t)args[0]->num));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 5 — HIGH-RESOLUTION TIME & CLOCKS
// ═════════════════════════════════════════════════════════════════════════════

// sys.clock_realtime() — wall clock time as [seconds, nanoseconds] (C: clock_gettime(CLOCK_REALTIME))
// Affected by NTP adjustments; use for timestamps.
static Value *sys_clock_realtime(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)ts.tv_sec);
    e[1] = value_number((double)ts.tv_nsec);
    return value_array(e, 2);
}

// sys.clock_monotonic() — monotonic clock [seconds, nanoseconds] (C: clock_gettime(CLOCK_MONOTONIC))
// Never goes backwards; use for measuring elapsed time / benchmarking.
static Value *sys_clock_monotonic(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)ts.tv_sec);
    e[1] = value_number((double)ts.tv_nsec);
    return value_array(e, 2);
}

// sys.clock_process() — CPU time used by this process [sec, nsec] (C: CLOCK_PROCESS_CPUTIME_ID)
static Value *sys_clock_process(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)ts.tv_sec);
    e[1] = value_number((double)ts.tv_nsec);
    return value_array(e, 2);
}

// sys.nanosleep(sec, nsec) — sleep with nanosecond precision (C: nanosleep())
// Far more precise than usleep(). Essential for real-time / embedded code.
static Value *sys_nanosleep(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    struct timespec req;
    req.tv_sec  = (time_t)args[0]->num;
    req.tv_nsec = (long)args[1]->num;
    return value_number((double)nanosleep(&req, NULL));
}

// sys.time() — Unix timestamp as integer seconds (C: time())
static Value *sys_time(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)time(NULL));
}

// sys.clock() — CPU time consumed (C: clock()), returns seconds as float
static Value *sys_clock_fn(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)clock() / (double)CLOCKS_PER_SEC);
}

// sys.gettimeofday() — microsecond-precision wall time [sec, usec] (C: gettimeofday())
static Value *sys_gettimeofday(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)tv.tv_sec);
    e[1] = value_number((double)tv.tv_usec);
    return value_array(e, 2);
}

// sys.sleep(sec) — sleep N whole seconds (C: sleep())
static Value *sys_sleep_sec(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)sleep((unsigned int)args[0]->num));
}

// sys.usleep(usec) — sleep N microseconds (C: usleep())
static Value *sys_usleep(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)usleep((useconds_t)args[0]->num));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 6 — SYSTEM INFORMATION & ENVIRONMENT
// ═════════════════════════════════════════════════════════════════════════════

// sys.uname() — OS/kernel identity [sysname, nodename, release, version, machine] (C: uname())
static Value *sys_uname(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct utsname u;
    if (uname(&u) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 5);
    e[0] = value_string(u.sysname);
    e[1] = value_string(u.nodename);
    e[2] = value_string(u.release);
    e[3] = value_string(u.version);
    e[4] = value_string(u.machine);
    return value_array(e, 5);
}

// sys.hostname() — machine hostname (C: gethostname())
static Value *sys_hostname(Value **args, size_t argc) {
    (void)args; (void)argc;
    char buf[256];
    return (gethostname(buf, sizeof(buf)) == 0) ? value_string(buf) : value_null();
}

// sys.nproc() — online CPU count (portable: sysctlbyname on macOS, sysconf on Linux)
static Value *sys_nproc(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_nprocessors_onln());
}

// sys.nproc_conf() — configured CPU count (may be higher than online)
static Value *sys_nproc_conf(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_nprocessors_conf());
}

// sys.phys_pages() — total physical RAM pages (portable)
static Value *sys_phys_pages(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_phys_pages());
}

// sys.avphys_pages() — available (free) RAM pages (portable)
static Value *sys_avphys_pages(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_avphys_pages());
}

// sys.PAGE_SIZE() — OS memory page size in bytes (typically 4096)
static Value *sys_PAGE_SIZE(Value **args, size_t argc) {
    (void)args; (void)argc;
    long ps = sysconf(_SC_PAGE_SIZE);
    return value_number((double)(ps > 0 ? ps : 4096));
}

// sys.endian() — byte order of this CPU: "little" or "big"
static Value *sys_endian(Value **args, size_t argc) {
    (void)args; (void)argc;
    uint16_t t = 0x0001;
    return value_string((*(uint8_t *)&t) ? "little" : "big");
}

// sys.sizeof_ptr() — pointer width: 4 (32-bit) or 8 (64-bit)
static Value *sys_sizeof_ptr(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)sizeof(void *));
}

// sys.getenv(name) — read environment variable (C: getenv())
static Value *sys_getenv(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    const char *v = getenv(args[0]->str);
    return v ? value_string(v) : value_null();
}

// sys.setenv(name, value) — set environment variable (C: setenv())
static Value *sys_setenv(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)setenv(args[0]->str, args[1]->str, 1));
}

// sys.unsetenv(name) — remove environment variable (C: unsetenv())
static Value *sys_unsetenv(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)unsetenv(args[0]->str));
}

// sys.errno() — current errno value (C: errno)
static Value *sys_errno_fn(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)errno);
}

// sys.strerror(code) — human-readable errno description (C: strerror())
static Value *sys_strerror(Value **args, size_t argc) {
    int code = (argc >= 1 && args[0]->type == VAL_NUMBER) ? (int)args[0]->num : errno;
    return value_string(strerror(code));
}

// sys.system(cmd) — run shell command, returns exit status (C: system())
static Value *sys_system(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)system(args[0]->str));
}

// sys.proc_status() — read /proc/self/status into a string (Linux)
// Returns kernel view of the current process: VmRSS, threads, state, etc.
static Value *sys_proc_status(Value **args, size_t argc) {
    (void)args; (void)argc;
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0) return value_null();
    char buf[8192];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0) return value_null();
    buf[n] = '\0';
    return value_string(buf);
}

// sys.proc_maps() — read /proc/self/maps (virtual memory layout of process)
// Shows every mapped region: code, stack, heap, shared libs, mmap regions.
static Value *sys_proc_maps(Value **args, size_t argc) {
    (void)args; (void)argc;
    int fd = open("/proc/self/maps", O_RDONLY);
    if (fd < 0) return value_null();
    char buf[65536];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0) return value_null();
    buf[n] = '\0';
    return value_string(buf);
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 7 — BIT & MEMORY OPERATIONS
// ═════════════════════════════════════════════════════════════════════════════

static Value *sys_band(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num, b = (uint64_t)(long long)args[1]->num;
    return value_number((double)(long long)(a & b));
}
static Value *sys_bor(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num, b = (uint64_t)(long long)args[1]->num;
    return value_number((double)(long long)(a | b));
}
static Value *sys_bxor(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num, b = (uint64_t)(long long)args[1]->num;
    return value_number((double)(long long)(a ^ b));
}
static Value *sys_bnot(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)(long long)(~a));
}
static Value *sys_shl(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(n >= 0 ? a << n : a >> (-n)));
}
static Value *sys_shr(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(n >= 0 ? a >> n : a << (-n)));
}
static Value *sys_sar(Value **args, size_t argc) {
    // Arithmetic (signed) right shift — preserves sign bit
    if (argc < 2) return value_number(0);
    long long a = (long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(a >> n));
}
static Value *sys_rol(Value **args, size_t argc) {
    // Rotate left (32-bit)
    if (argc < 2) return value_number(0);
    uint32_t a = (uint32_t)(long long)args[0]->num; int n = ((int)args[1]->num) & 31;
    return value_number((double)(long long)((a << n) | (a >> (32 - n))));
}
static Value *sys_ror(Value **args, size_t argc) {
    // Rotate right (32-bit) — used in SHA, AES, CRC algorithms
    if (argc < 2) return value_number(0);
    uint32_t a = (uint32_t)(long long)args[0]->num; int n = ((int)args[1]->num) & 31;
    return value_number((double)(long long)((a >> n) | (a << (32 - n))));
}
static Value *sys_popcnt(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)__builtin_popcountll(a));
}
static Value *sys_clz(Value **args, size_t argc) {
    if (argc < 1) return value_number(64);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number(a == 0 ? 64.0 : (double)__builtin_clzll(a));
}
static Value *sys_ctz(Value **args, size_t argc) {
    if (argc < 1) return value_number(64);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number(a == 0 ? 64.0 : (double)__builtin_ctzll(a));
}
static Value *sys_bswap32(Value **args, size_t argc) {
    // Byte-swap 32-bit — used for endian conversion in network/file code
    if (argc < 1) return value_number(0);
    uint32_t a = (uint32_t)(long long)args[0]->num;
    return value_number((double)(long long)__builtin_bswap32(a));
}
static Value *sys_bswap64(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)(long long)__builtin_bswap64(a));
}
static Value *sys_parity(Value **args, size_t argc) {
    // Returns 1 if number of set bits is odd, 0 if even (parity check)
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)(__builtin_parityll(a)));
}
static Value *sys_bit_get(Value **args, size_t argc) {
    // Get bit at position n: sys.bit_get(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)((a >> n) & 1));
}
static Value *sys_bit_set(Value **args, size_t argc) {
    // Set bit at position n: sys.bit_set(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(a | ((uint64_t)1 << n)));
}
static Value *sys_bit_clear(Value **args, size_t argc) {
    // Clear bit at position n: sys.bit_clear(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(a & ~((uint64_t)1 << n)));
}
static Value *sys_bit_toggle(Value **args, size_t argc) {
    // Toggle bit at position n: sys.bit_toggle(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(a ^ ((uint64_t)1 << n)));
}
static Value *sys_align_up(Value **args, size_t argc) {
    // Round n up to nearest multiple of align (must be power of 2)
    // Fundamental in memory allocators and struct layout
    if (argc < 2) return value_number(0);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    uint64_t a = (uint64_t)(long long)args[1]->num;
    if (a == 0) return value_number((double)(long long)n);
    return value_number((double)(long long)((n + a - 1) & ~(a - 1)));
}
static Value *sys_align_down(Value **args, size_t argc) {
    // Round n down to nearest multiple of align (must be power of 2)
    if (argc < 2) return value_number(0);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    uint64_t a = (uint64_t)(long long)args[1]->num;
    if (a == 0) return value_number((double)(long long)n);
    return value_number((double)(long long)(n & ~(a - 1)));
}
static Value *sys_is_pow2(Value **args, size_t argc) {
    // Returns true if n is a power of 2 (n > 0 && (n & (n-1)) == 0)
    if (argc < 1) return value_bool(0);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    return value_bool(n > 0 && (n & (n - 1)) == 0);
}
static Value *sys_next_pow2(Value **args, size_t argc) {
    // Next power of 2 >= n — used in buffer sizing, hash tables
    if (argc < 1) return value_number(1);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    if (n == 0) return value_number(1);
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8;
    n |= n >> 16; n |= n >> 32;
    return value_number((double)(long long)(n + 1));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 8 — IPC & SYSTEM LOGGING
// ═════════════════════════════════════════════════════════════════════════════

// sys.openlog(ident, option, facility) — open syslog connection (C: openlog())
// Used by system daemons to write structured logs to the OS log daemon.
// option: 0=default, 1=LOG_PID, 2=LOG_CONS
// facility: 0=LOG_KERN, 1=LOG_USER, 3=LOG_DAEMON, 4=LOG_AUTH
static Value *sys_openlog(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    int option   = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : LOG_PID;
    int facility = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : LOG_USER;
    openlog(args[0]->str, option, facility);
    return value_null();
}

// sys.syslog(priority, message) — write to system log (C: syslog())
// priority: 0=EMERG,1=ALERT,2=CRIT,3=ERR,4=WARNING,5=NOTICE,6=INFO,7=DEBUG
static Value *sys_syslog(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_null();
    int priority = (int)args[0]->num;
    char *msg = (args[1]->type == VAL_STRING) ? args[1]->str : "";
    syslog(priority, "%s", msg);
    return value_null();
}

// sys.closelog() — close syslog (C: closelog())
static Value *sys_closelog(Value **args, size_t argc) {
    (void)args; (void)argc;
    closelog();
    return value_null();
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 9 — FILE OPEN FLAGS & SIGNAL CONSTANTS
// ═════════════════════════════════════════════════════════════════════════════

// Open flags — use these with sys.open()
static Value *sys_O_RDONLY(Value **a, size_t c)   { (void)a;(void)c; return value_number(O_RDONLY); }
static Value *sys_O_WRONLY(Value **a, size_t c)   { (void)a;(void)c; return value_number(O_WRONLY); }
static Value *sys_O_RDWR(Value **a, size_t c)     { (void)a;(void)c; return value_number(O_RDWR);   }
static Value *sys_O_CREAT(Value **a, size_t c)    { (void)a;(void)c; return value_number(O_CREAT);  }
static Value *sys_O_TRUNC(Value **a, size_t c)    { (void)a;(void)c; return value_number(O_TRUNC);  }
static Value *sys_O_APPEND(Value **a, size_t c)   { (void)a;(void)c; return value_number(O_APPEND); }
static Value *sys_O_NONBLOCK(Value **a, size_t c) { (void)a;(void)c; return value_number(O_NONBLOCK); }
static Value *sys_O_SYNC(Value **a, size_t c)     { (void)a;(void)c; return value_number(O_SYNC);   }
static Value *sys_O_EXCL(Value **a, size_t c)     { (void)a;(void)c; return value_number(O_EXCL);   }

// Signal numbers — use with sys.kill() and sys.raise()
static Value *sys_SIGHUP(Value **a, size_t c)     { (void)a;(void)c; return value_number(SIGHUP);   }
static Value *sys_SIGINT(Value **a, size_t c)     { (void)a;(void)c; return value_number(SIGINT);   }
static Value *sys_SIGQUIT(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGQUIT);  }
static Value *sys_SIGKILL(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGKILL);  }
static Value *sys_SIGTERM(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGTERM);  }
static Value *sys_SIGUSR1(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGUSR1);  }
static Value *sys_SIGUSR2(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGUSR2);  }
static Value *sys_SIGCHLD(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGCHLD);  }
static Value *sys_SIGPIPE(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGPIPE);  }
static Value *sys_SIGALRM(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGALRM);  }
static Value *sys_SIGSEGV(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGSEGV);  }

// Syslog priority constants
static Value *sys_LOG_EMERG(Value **a, size_t c)   { (void)a;(void)c; return value_number(LOG_EMERG);   }
static Value *sys_LOG_ALERT(Value **a, size_t c)   { (void)a;(void)c; return value_number(LOG_ALERT);   }
static Value *sys_LOG_CRIT(Value **a, size_t c)    { (void)a;(void)c; return value_number(LOG_CRIT);    }
static Value *sys_LOG_ERR(Value **a, size_t c)     { (void)a;(void)c; return value_number(LOG_ERR);     }
static Value *sys_LOG_WARNING(Value **a, size_t c) { (void)a;(void)c; return value_number(LOG_WARNING); }
static Value *sys_LOG_NOTICE(Value **a, size_t c)  { (void)a;(void)c; return value_number(LOG_NOTICE);  }
static Value *sys_LOG_INFO(Value **a, size_t c)    { (void)a;(void)c; return value_number(LOG_INFO);    }
static Value *sys_LOG_DEBUG(Value **a, size_t c)   { (void)a;(void)c; return value_number(LOG_DEBUG);   }

// Poll event flags
static Value *sys_POLLIN(Value **a, size_t c)    { (void)a;(void)c; return value_number(POLLIN);    }
static Value *sys_POLLOUT(Value **a, size_t c)   { (void)a;(void)c; return value_number(POLLOUT);   }
static Value *sys_POLLERR(Value **a, size_t c)   { (void)a;(void)c; return value_number(POLLERR);   }
static Value *sys_POLLHUP(Value **a, size_t c)   { (void)a;(void)c; return value_number(POLLHUP);   }

// mmap prot flags
static Value *sys_PROT_READ(Value **a, size_t c)  { (void)a;(void)c; return value_number(PROT_READ);  }
static Value *sys_PROT_WRITE(Value **a, size_t c) { (void)a;(void)c; return value_number(PROT_WRITE); }
static Value *sys_PROT_EXEC(Value **a, size_t c)  { (void)a;(void)c; return value_number(PROT_EXEC);  }
static Value *sys_PROT_NONE(Value **a, size_t c)  { (void)a;(void)c; return value_number(PROT_NONE);  }
static Value *sys_MAP_SHARED(Value **a, size_t c)    { (void)a;(void)c; return value_number(MAP_SHARED);    }
static Value *sys_MAP_PRIVATE(Value **a, size_t c)   { (void)a;(void)c; return value_number(MAP_PRIVATE);   }
static Value *sys_MAP_ANONYMOUS(Value **a, size_t c) { (void)a;(void)c; return value_number(MAP_ANONYMOUS); }

// Socket domain/type constants
static Value *sys_AF_INET(Value **a, size_t c)    { (void)a;(void)c; return value_number(AF_INET);    }
static Value *sys_AF_INET6(Value **a, size_t c)   { (void)a;(void)c; return value_number(AF_INET6);   }
static Value *sys_AF_UNIX(Value **a, size_t c)    { (void)a;(void)c; return value_number(AF_UNIX);    }
static Value *sys_SOCK_STREAM(Value **a, size_t c){ (void)a;(void)c; return value_number(SOCK_STREAM); }
static Value *sys_SOCK_DGRAM(Value **a, size_t c) { (void)a;(void)c; return value_number(SOCK_DGRAM);  }

// Resource limit resource IDs
static Value *sys_RLIMIT_CPU(Value **a, size_t c)    { (void)a;(void)c; return value_number(RLIMIT_CPU);    }
static Value *sys_RLIMIT_FSIZE(Value **a, size_t c)  { (void)a;(void)c; return value_number(RLIMIT_FSIZE);  }
static Value *sys_RLIMIT_STACK(Value **a, size_t c)  { (void)a;(void)c; return value_number(RLIMIT_STACK);  }
static Value *sys_RLIMIT_NOFILE(Value **a, size_t c) { (void)a;(void)c; return value_number(RLIMIT_NOFILE); }
static Value *sys_RLIMIT_AS(Value **a, size_t c)     { (void)a;(void)c; return value_number(RLIMIT_AS);     }

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 10 — NUMERIC CONSTANTS (from C limits.h / stdint.h)
// ═════════════════════════════════════════════════════════════════════════════
static Value *sys_STDIN(Value **a, size_t c)    { (void)a;(void)c; return value_number(0); }
static Value *sys_STDOUT(Value **a, size_t c)   { (void)a;(void)c; return value_number(1); }
static Value *sys_STDERR(Value **a, size_t c)   { (void)a;(void)c; return value_number(2); }
static Value *sys_INT_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT_MAX);  }
static Value *sys_INT_MIN_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT_MIN);  }
static Value *sys_UINT_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)UINT_MAX); }
static Value *sys_LONG_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)LONG_MAX); }
static Value *sys_INT8_MAX_fn(Value **a, size_t c)   { (void)a;(void)c; return value_number((double)INT8_MAX);   }
static Value *sys_INT8_MIN_fn(Value **a, size_t c)   { (void)a;(void)c; return value_number((double)INT8_MIN);   }
static Value *sys_INT16_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT16_MAX);  }
static Value *sys_INT16_MIN_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT16_MIN);  }
static Value *sys_INT32_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT32_MAX);  }
static Value *sys_INT32_MIN_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT32_MIN);  }
static Value *sys_UINT8_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)UINT8_MAX);  }
static Value *sys_UINT16_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)UINT16_MAX); }
static Value *sys_UINT32_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)UINT32_MAX); }

// ═════════════════════════════════════════════════════════════════════════════
// MODULE FUNCTION TABLE
// ═════════════════════════════════════════════════════════════════════════════
static NativeFunc sys_fns[] = {
    // ── LEVEL 1: Process Control ────────────────────────────────────────────
    { "exit",           sys_exit            },
    { "abort",          sys_abort           },
    { "getpid",         sys_getpid          },
    { "getppid",        sys_getppid         },
    { "fork",           sys_fork            },
    { "exec",           sys_exec            },
    { "wait",           sys_wait            },
    { "waitpid",        sys_waitpid         },
    { "kill",           sys_kill            },
    { "raise",          sys_raise           },
    { "getuid",         sys_getuid          },
    { "getgid",         sys_getgid          },
    { "geteuid",        sys_geteuid         },
    { "getegid",        sys_getegid         },
    { "getrlimit",      sys_getrlimit       },
    { "setrlimit",      sys_setrlimit       },
    // ── LEVEL 2: File Descriptor I/O & mmap ────────────────────────────────
    { "open",           sys_open            },
    { "close",          sys_close           },
    { "read",           sys_read            },
    { "write",          sys_write           },
    { "seek",           sys_seek            },
    { "dup",            sys_dup             },
    { "dup2",           sys_dup2            },
    { "pipe",           sys_pipe            },
    { "fsync",          sys_fsync           },
    { "fdatasync",      sys_fdatasync       },
    { "truncate",       sys_truncate        },
    { "ftruncate",      sys_ftruncate       },
    { "mmap",           sys_mmap            },
    { "munmap",         sys_munmap          },
    { "mmap_read",      sys_mmap_read       },
    { "mmap_write",     sys_mmap_write      },
    { "poll",           sys_poll            },
    { "fcntl_lock",     sys_fcntl_lock      },
    { "fcntl_setfl",    sys_fcntl_setfl     },
    { "fcntl_getfl",    sys_fcntl_getfl     },
    // ── LEVEL 3: Filesystem ─────────────────────────────────────────────────
    { "stat",           sys_stat            },
    { "lstat",          sys_lstat           },
    { "isfile",         sys_isfile          },
    { "isdir",          sys_isdir           },
    { "islnk",          sys_islnk           },
    { "access",         sys_access          },
    { "mkdir",          sys_mkdir           },
    { "rmdir",          sys_rmdir           },
    { "unlink",         sys_unlink          },
    { "rename",         sys_rename          },
    { "link",           sys_link            },
    { "symlink",        sys_symlink         },
    { "readlink",       sys_readlink        },
    { "chmod",          sys_chmod           },
    { "chown",          sys_chown           },
    { "getcwd",         sys_getcwd          },
    { "chdir",          sys_chdir           },
    { "mkfifo",         sys_mkfifo          },
    // ── LEVEL 4: Networking ─────────────────────────────────────────────────
    { "socket",         sys_socket          },
    { "connect",        sys_connect         },
    { "bind",           sys_bind            },
    { "listen",         sys_listen          },
    { "accept",         sys_accept          },
    { "send",           sys_send            },
    { "recv",           sys_recv            },
    { "setsockopt",     sys_setsockopt      },
    { "inet_aton",      sys_inet_aton       },
    { "inet_ntoa",      sys_inet_ntoa       },
    { "htons",          sys_htons           },
    { "ntohs",          sys_ntohs           },
    { "htonl",          sys_htonl           },
    { "ntohl",          sys_ntohl           },
    // ── LEVEL 5: Time & Clocks ──────────────────────────────────────────────
    { "clock_realtime",  sys_clock_realtime  },
    { "clock_monotonic", sys_clock_monotonic },
    { "clock_process",   sys_clock_process   },
    { "nanosleep",       sys_nanosleep       },
    { "gettimeofday",    sys_gettimeofday    },
    { "sleep",           sys_sleep_sec       },
    { "usleep",          sys_usleep          },
    { "time",            sys_time            },
    { "clock",           sys_clock_fn        },
    // ── LEVEL 6: System Info & Environment ─────────────────────────────────
    { "uname",          sys_uname           },
    { "hostname",       sys_hostname        },
    { "nproc",          sys_nproc           },
    { "nproc_conf",     sys_nproc_conf      },
    { "phys_pages",     sys_phys_pages      },
    { "avphys_pages",   sys_avphys_pages    },
    { "PAGE_SIZE",      sys_PAGE_SIZE       },
    { "endian",         sys_endian          },
    { "sizeof_ptr",     sys_sizeof_ptr      },
    { "getenv",         sys_getenv          },
    { "setenv",         sys_setenv          },
    { "unsetenv",       sys_unsetenv        },
    { "errno",          sys_errno_fn        },
    { "strerror",       sys_strerror        },
    { "system",         sys_system          },
    { "proc_status",    sys_proc_status     },
    { "proc_maps",      sys_proc_maps       },
    // ── LEVEL 7: Bit & Memory Operations ───────────────────────────────────
    { "band",           sys_band            },
    { "bor",            sys_bor             },
    { "bxor",           sys_bxor            },
    { "bnot",           sys_bnot            },
    { "shl",            sys_shl             },
    { "shr",            sys_shr             },
    { "sar",            sys_sar             },
    { "rol",            sys_rol             },
    { "ror",            sys_ror             },
    { "popcnt",         sys_popcnt          },
    { "clz",            sys_clz             },
    { "ctz",            sys_ctz             },
    { "bswap32",        sys_bswap32         },
    { "bswap64",        sys_bswap64         },
    { "parity",         sys_parity          },
    { "bit_get",        sys_bit_get         },
    { "bit_set",        sys_bit_set         },
    { "bit_clear",      sys_bit_clear       },
    { "bit_toggle",     sys_bit_toggle      },
    { "align_up",       sys_align_up        },
    { "align_down",     sys_align_down      },
    { "is_pow2",        sys_is_pow2         },
    { "next_pow2",      sys_next_pow2       },
    // ── LEVEL 8: IPC & Syslog ───────────────────────────────────────────────
    { "openlog",        sys_openlog         },
    { "syslog",         sys_syslog          },
    { "closelog",       sys_closelog        },
    // ── LEVEL 9: Open Flags & Signal Constants ──────────────────────────────
    { "O_RDONLY",       sys_O_RDONLY        },
    { "O_WRONLY",       sys_O_WRONLY        },
    { "O_RDWR",         sys_O_RDWR          },
    { "O_CREAT",        sys_O_CREAT         },
    { "O_TRUNC",        sys_O_TRUNC         },
    { "O_APPEND",       sys_O_APPEND        },
    { "O_NONBLOCK",     sys_O_NONBLOCK      },
    { "O_SYNC",         sys_O_SYNC          },
    { "O_EXCL",         sys_O_EXCL          },
    { "SIGHUP",         sys_SIGHUP          },
    { "SIGINT",         sys_SIGINT          },
    { "SIGQUIT",        sys_SIGQUIT         },
    { "SIGKILL",        sys_SIGKILL         },
    { "SIGTERM",        sys_SIGTERM         },
    { "SIGUSR1",        sys_SIGUSR1         },
    { "SIGUSR2",        sys_SIGUSR2         },
    { "SIGCHLD",        sys_SIGCHLD         },
    { "SIGPIPE",        sys_SIGPIPE         },
    { "SIGALRM",        sys_SIGALRM         },
    { "SIGSEGV",        sys_SIGSEGV         },
    { "LOG_EMERG",      sys_LOG_EMERG       },
    { "LOG_ALERT",      sys_LOG_ALERT       },
    { "LOG_CRIT",       sys_LOG_CRIT        },
    { "LOG_ERR",        sys_LOG_ERR         },
    { "LOG_WARNING",    sys_LOG_WARNING     },
    { "LOG_NOTICE",     sys_LOG_NOTICE      },
    { "LOG_INFO",       sys_LOG_INFO        },
    { "LOG_DEBUG",      sys_LOG_DEBUG       },
    { "POLLIN",         sys_POLLIN          },
    { "POLLOUT",        sys_POLLOUT         },
    { "POLLERR",        sys_POLLERR         },
    { "POLLHUP",        sys_POLLHUP         },
    { "PROT_READ",      sys_PROT_READ       },
    { "PROT_WRITE",     sys_PROT_WRITE      },
    { "PROT_EXEC",      sys_PROT_EXEC       },
    { "PROT_NONE",      sys_PROT_NONE       },
    { "MAP_SHARED",     sys_MAP_SHARED      },
    { "MAP_PRIVATE",    sys_MAP_PRIVATE     },
    { "MAP_ANONYMOUS",  sys_MAP_ANONYMOUS   },
    { "AF_INET",        sys_AF_INET         },
    { "AF_INET6",       sys_AF_INET6        },
    { "AF_UNIX",        sys_AF_UNIX         },
    { "SOCK_STREAM",    sys_SOCK_STREAM     },
    { "SOCK_DGRAM",     sys_SOCK_DGRAM      },
    { "RLIMIT_CPU",     sys_RLIMIT_CPU      },
    { "RLIMIT_FSIZE",   sys_RLIMIT_FSIZE    },
    { "RLIMIT_STACK",   sys_RLIMIT_STACK    },
    { "RLIMIT_NOFILE",  sys_RLIMIT_NOFILE   },
    { "RLIMIT_AS",      sys_RLIMIT_AS       },
    // ── LEVEL 10: Integer Constants ─────────────────────────────────────────
    { "STDIN",          sys_STDIN           },
    { "STDOUT",         sys_STDOUT          },
    { "STDERR",         sys_STDERR          },
    { "INT_MAX",        sys_INT_MAX_fn      },
    { "INT_MIN",        sys_INT_MIN_fn      },
    { "UINT_MAX",       sys_UINT_MAX_fn     },
    { "LONG_MAX",       sys_LONG_MAX_fn     },
    { "INT8_MAX",       sys_INT8_MAX_fn     },
    { "INT8_MIN",       sys_INT8_MIN_fn     },
    { "INT16_MAX",      sys_INT16_MAX_fn    },
    { "INT16_MIN",      sys_INT16_MIN_fn    },
    { "INT32_MAX",      sys_INT32_MAX_fn    },
    { "INT32_MIN",      sys_INT32_MIN_fn    },
    { "UINT8_MAX",      sys_UINT8_MAX_fn    },
    { "UINT16_MAX",     sys_UINT16_MAX_fn   },
    { "UINT32_MAX",     sys_UINT32_MAX_fn   },
    { NULL, NULL }
};

Module module_sys(void) {
    Module m;
    m.name      = NULL;
    m.functions = sys_fns;
    m.fn_count  = sizeof(sys_fns)/sizeof(sys_fns[0]) - 1;
    return m;
}