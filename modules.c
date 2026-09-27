/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
/* _GNU_SOURCE: enables GNU/Linux extensions (memfd_create, getline, etc.).
 * On macOS, this macro has no effect — Darwin uses _DARWIN_C_SOURCE instead.
 * Guard it so static analysers don't warn about cross-platform redefinitions. */
#if !defined(__APPLE__)
#define _GNU_SOURCE
#endif
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
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// ─── Forward declarations for modules defined later in this file ─────────────
static Module module_fs(void);

// ─── Registry ────────────────────────────────────────────────────────────────
// Forward declarations for new compute modules
static Module module_numeric(void);
static Module module_matrix(void);
static Module module_diff(void);

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
    if (strcmp(name, "fs")        == 0) { *out = module_fs();        return 1; }
    if (strcmp(name, "numeric")   == 0) { *out = module_numeric();   return 1; }
    if (strcmp(name, "matrix")    == 0) { *out = module_matrix();    return 1; }
    if (strcmp(name, "diff")      == 0) { *out = module_diff();      return 1; }
    if (strcmp(name, "http")      == 0) { extern Module module_http(void);    *out = module_http();    return 1; }
    if (strcmp(name, "reflect")   == 0) { extern Module module_reflect(void); *out = module_reflect(); return 1; }
    if (strcmp(name, "iter")      == 0) { extern Module module_iter(void);    *out = module_iter();    return 1; }
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

static Value *str_toString(Value **args, size_t argc) {
    if (argc < 1) return value_string("null");
    Value *v = args[0];
    if (v->type == VAL_STRING) return value_string(v->str);
    if (v->type == VAL_NULL)   return value_string("null");
    if (v->type == VAL_BOOL)   return value_string(v->boolean ? "true" : "false");
    if (v->type == VAL_NUMBER) {
        char buf[64];
        if (v->num == (long long)v->num) snprintf(buf, sizeof(buf), "%lld", (long long)v->num);
        else snprintf(buf, sizeof(buf), "%g", v->num);
        return value_string(buf);
    }
    return value_string("[object]");
}

static Value *str_toNumber(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    Value *v = args[0];
    if (v->type == VAL_NUMBER) return value_number(v->num);
    if (v->type == VAL_STRING) return value_number(strtod(v->str, NULL));
    if (v->type == VAL_BOOL)   return value_number(v->boolean ? 1.0 : 0.0);
    return value_number(0);
}

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
    { "toString",      str_toString },
    { "toNumber",      str_toNumber },
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
// FS MODULE  — file-system read / write / append / exists / remove / list
// ═════════════════════════════════════════════════════════════════════════════

// fs.readFile(path) → string | null
static Value *fs_readFile(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    FILE *f = fopen(args[0]->str, "rb");
    if (!f) return value_null();
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return value_null(); }
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return value_null(); }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// fs.writeFile(path, content) → bool
static Value *fs_writeFile(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    FILE *f = fopen(args[0]->str, "wb");
    if (!f) return value_bool(0);
    size_t len = strlen(args[1]->str);
    size_t wr  = fwrite(args[1]->str, 1, len, f);
    fclose(f);
    return value_bool(wr == len);
}

// fs.appendFile(path, content) → bool
static Value *fs_appendFile(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    FILE *f = fopen(args[0]->str, "ab");
    if (!f) return value_bool(0);
    size_t len = strlen(args[1]->str);
    size_t wr  = fwrite(args[1]->str, 1, len, f);
    fclose(f);
    return value_bool(wr == len);
}

// fs.exists(path) → bool
static Value *fs_exists(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    FILE *f = fopen(args[0]->str, "rb");
    if (!f) return value_bool(0);
    fclose(f);
    return value_bool(1);
}

// fs.remove(path) → bool
static Value *fs_remove(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    return value_bool(remove(args[0]->str) == 0);
}

// fs.size(path) → number | null
static Value *fs_size(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    FILE *f = fopen(args[0]->str, "rb");
    if (!f) return value_null();
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return value_number((double)sz);
}

// fs.readLines(path) → array<string> | null
static Value *fs_readLines(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    FILE *f = fopen(args[0]->str, "rb");
    if (!f) return value_null();
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return value_null(); }
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return value_null(); }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';

    // Count lines first
    size_t cap = 64, count = 0;
    Value **lines = (Value **)malloc(cap * sizeof(Value *));
    char *p = buf, *start = buf;
    while (*p) {
        if (*p == '\n') {
            size_t linelen = (size_t)(p - start);
            if (linelen > 0 && start[linelen-1] == '\r') linelen--;
            char *line = (char *)malloc(linelen + 1);
            memcpy(line, start, linelen);
            line[linelen] = '\0';
            if (count >= cap) { cap *= 2; lines = (Value **)realloc(lines, cap * sizeof(Value *)); }
            lines[count++] = value_string(line);
            free(line);
            start = p + 1;
        }
        p++;
    }
    // last line (no trailing newline)
    if (p > start) {
        size_t linelen = (size_t)(p - start);
        char *line = (char *)malloc(linelen + 1);
        memcpy(line, start, linelen);
        line[linelen] = '\0';
        if (count >= cap) { cap *= 2; lines = (Value **)realloc(lines, cap * sizeof(Value *)); }
        lines[count++] = value_string(line);
        free(line);
    }
    free(buf);
    // value_array takes ownership of lines pointer
    return value_array(lines, count);
}

// fs.writeLines(path, array) → bool
static Value *fs_writeLines(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_ARRAY)
        return value_bool(0);
    FILE *f = fopen(args[0]->str, "wb");
    if (!f) return value_bool(0);
    for (size_t i = 0; i < args[1]->array_len; i++) {
        Value *item = args[1]->array[i];
        if (item && item->type == VAL_STRING)
            fprintf(f, "%s\n", item->str);
    }
    fclose(f);
    return value_bool(1);
}

// fs.rename(src, dst) → bool
static Value *fs_rename(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING)
        return value_bool(0);
    return value_bool(rename(args[0]->str, args[1]->str) == 0);
}

// fs.mkdir(path) → bool
static Value *fs_mkdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
#if defined(_WIN32)
    return value_bool(_mkdir(args[0]->str) == 0);
#else
    return value_bool(mkdir(args[0]->str, 0755) == 0);
#endif
}

// fs.listDir(path) → array<string> | null
static Value *fs_listDir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
#if defined(_WIN32)
    return value_null();
#else
    DIR *d = opendir(args[0]->str);
    if (!d) return value_null();
    size_t cap = 32, count = 0;
    Value **entries = (Value **)malloc(cap * sizeof(Value *));
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        if (count >= cap) { cap *= 2; entries = (Value **)realloc(entries, cap * sizeof(Value *)); }
        entries[count++] = value_string(ent->d_name);
    }
    closedir(d);
    Value *arr = value_array(entries, count);
    return arr;
#endif
}

static NativeFunc fs_fns[] = {
    { "readFile",   fs_readFile  },
    { "writeFile",  fs_writeFile },
    { "appendFile", fs_appendFile},
    { "exists",     fs_exists    },
    { "remove",     fs_remove    },
    { "size",       fs_size      },
    { "readLines",  fs_readLines },
    { "writeLines", fs_writeLines},
    { "rename",     fs_rename    },
    { "mkdir",      fs_mkdir     },
    { "listDir",    fs_listDir   },
    { NULL, NULL }
};

static Module module_fs(void) {
    Module m;
    m.name      = NULL;
    m.functions = fs_fns;
    m.fn_count  = sizeof(fs_fns)/sizeof(fs_fns[0]) - 1;
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
static Value *type_isObject(Value **args, size_t argc) {
    return value_bool(argc >= 1 && args[0]->type == VAL_INSTANCE);
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
    { "isBoolean",  type_isBool },    // alias
    { "isNull",     type_isNull },
    { "isArray",    type_isArray },
    { "isFunction", type_isFunction },
    { "isObject",   type_isObject },
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
// sys module implementation: shared with the native compiler runtime.
// See src/sys_module.inc for the full implementation and rationale.
#include "sys_module.inc"

// ═════════════════════════════════════════════════════════════════════════════
// REFLECT MODULE — Dynamic reflection and alteration of objects
//
// All reflect operations work on VAL_INSTANCE objects (Xenly object/class
// instances).  The module exposes the same semantics as the built-in
// NODE_REFLECT_* AST nodes, making reflection available to higher-order
// code that receives objects at runtime without knowing their shape at
// parse time.
//
// Available functions:
//   reflect.keys(obj)              → string[]  all enumerable property names
//   reflect.ownKeys(obj)           → string[]  own keys (non-inherited)
//   reflect.get(obj, key)          → value     property value or null
//   reflect.set(obj, key, val)     → bool      true on success
//   reflect.has(obj, key)          → bool      true if property exists
//   reflect.delete(obj, key)       → bool      true if property removed
//   reflect.freeze(obj)            → obj       make object immutable
//   reflect.isFrozen(obj)          → bool      true if frozen
//   reflect.define(obj, key, desc) → obj       define with descriptor
//   reflect.apply(fn, thisArg, args) → any     call fn with args array
//   reflect.construct(Cls, args)   → instance  new Cls(...args)
//   reflect.getPrototype(obj)      → string    class name or "object"
//   reflect.ownPropertyDescriptor(obj,key) → {value,writable,enumerable}
//   reflect.typeOf(obj)            → string    "number","string","bool",…
//   reflect.isExtensible(obj)      → bool      true if not frozen
// ═════════════════════════════════════════════════════════════════════════════

/* Internal helper: get instance fields env from a Value* */
static Environment *reflect_fields(Value *obj) {
    if (!obj || obj->type != VAL_INSTANCE || !obj->instance) return NULL;
    return obj->instance->fields;
}

/* Collect all keys from an Environment into a new VAL_ARRAY */
static Value *env_keys_to_array(Environment *env) {
    if (!env) return value_array(NULL, 0);
    size_t cap = 8, cnt = 0;
    Value **keys = (Value **)malloc(sizeof(Value *) * cap);
    for (EnvEntry *e = env->entries; e; e = e->next) {
        if (cnt >= cap) { cap *= 2; keys = (Value **)realloc(keys, sizeof(Value *)*cap); }
        keys[cnt++] = value_string(e->name);
    }
    Value *arr = value_array(keys, cnt);
    free(keys);
    return arr;
}

static Value *reflect_keys_fn(Value **args, size_t argc) {
    if (argc < 1) return value_array(NULL, 0);
    return env_keys_to_array(reflect_fields(args[0]));
}

static Value *reflect_own_keys_fn(Value **args, size_t argc) {
    /* In Xenly's flat-env model own keys == all keys on instance */
    return reflect_keys_fn(args, argc);
}

static Value *reflect_get_fn(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    Environment *fields = reflect_fields(args[0]);
    if (!fields) return value_null();
    char *k = value_to_string(args[1]);
    Value *v = env_get(fields, k);
    free(k);
    return v ? v : value_null();
}

static Value *reflect_set_fn(Value **args, size_t argc) {
    if (argc < 3) return value_bool(0);
    if (!args[0] || args[0]->local == 99) return value_bool(0); /* frozen */
    Environment *fields = reflect_fields(args[0]);
    if (!fields) return value_bool(0);
    char *k = value_to_string(args[1]);
    env_set(fields, k, args[2]);
    free(k);
    return value_bool(1);
}

static Value *reflect_has_fn(Value **args, size_t argc) {
    if (argc < 2) return value_bool(0);
    Environment *fields = reflect_fields(args[0]);
    if (!fields) return value_bool(0);
    char *k = value_to_string(args[1]);
    Value *found = env_get(fields, k);
    free(k);
    return value_bool(found != NULL);
}

static Value *reflect_delete_fn(Value **args, size_t argc) {
    if (argc < 2) return value_bool(0);
    if (args[0] && args[0]->local == 99) return value_bool(0); /* frozen */
    Environment *fields = reflect_fields(args[0]);
    if (!fields) return value_bool(0);
    char *k = value_to_string(args[1]);
    EnvEntry **prev = &fields->entries;
    for (EnvEntry *e = *prev; e; prev = &e->next, e = e->next) {
        if (strcmp(e->name, k) == 0) {
            *prev = e->next;
            free(e->name); free(e); free(k);
            return value_bool(1);
        }
    }
    free(k);
    return value_bool(0);
}

static Value *reflect_freeze_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0]) return value_null();
    args[0]->local = 99; /* frozen sentinel */
    return args[0];
}

static Value *reflect_is_frozen_fn(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    return value_bool(args[0] && args[0]->local == 99);
}

static Value *reflect_is_extensible_fn(Value **args, size_t argc) {
    if (argc < 1) return value_bool(1);
    return value_bool(!args[0] || args[0]->local != 99);
}

static Value *reflect_define_fn(Value **args, size_t argc) {
    if (argc < 3) return value_null();
    if (!args[0] || args[0]->local == 99) return args[0] ? args[0] : value_null();
    Environment *fields = reflect_fields(args[0]);
    if (!fields) return value_null();
    char *k = value_to_string(args[1]);
    /* descriptor: { value, writable, enumerable } — extract .value */
    Value *val = NULL;
    Environment *desc_fields = reflect_fields(args[2]);
    if (desc_fields) val = env_get(desc_fields, "value");
    if (!val) val = args[2]; /* if not an object, treat descriptor as the value */
    env_set(fields, k, val);
    free(k);
    return args[0];
}

static Value *reflect_get_prototype_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0]) return value_string("null");
    if (args[0]->type == VAL_INSTANCE && args[0]->instance &&
        args[0]->instance->class_def)
        return value_string(args[0]->instance->class_def->name);
    return value_string("object");
}

static Value *reflect_own_prop_descriptor_fn(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    Environment *fields = reflect_fields(args[0]);
    if (!fields) return value_null();
    char *k = value_to_string(args[1]);
    Value *val = env_get(fields, k);
    free(k);
    if (!val) return value_null();
    /* Return { value: V, writable: true, enumerable: true, configurable: true } */
    Value **items = (Value **)malloc(sizeof(Value *) * 4);
    /* Build a simple instance to represent the descriptor */
    InstanceData *inst = (InstanceData *)calloc(1, sizeof(InstanceData));
    inst->fields = env_create(NULL);
    env_set(inst->fields, "value",        val);
    env_set(inst->fields, "writable",     value_bool(args[0]->local != 99));
    env_set(inst->fields, "enumerable",   value_bool(1));
    env_set(inst->fields, "configurable", value_bool(args[0]->local != 99));
    Value *desc = (Value *)calloc(1, sizeof(Value));
    desc->type     = VAL_INSTANCE;
    desc->local    = 1;
    desc->instance = inst;
    free(items);
    return desc;
}

static Value *reflect_typeof_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0]) return value_string("null");
    switch (args[0]->type) {
        case VAL_NUMBER:   return value_string("number");
        case VAL_STRING:   return value_string("string");
        case VAL_BOOL:     return value_string("bool");
        case VAL_NULL:     return value_string("null");
        case VAL_FUNCTION: return value_string("function");
        case VAL_ARRAY:    return value_string("array");
        case VAL_INSTANCE: return value_string("object");
        case VAL_CLASS:    return value_string("class");
        default:           return value_string("unknown");
    }
}

static NativeFunc reflect_fns[] = {
    { "keys",                  reflect_keys_fn              },
    { "ownKeys",               reflect_own_keys_fn          },
    { "get",                   reflect_get_fn               },
    { "set",                   reflect_set_fn               },
    { "has",                   reflect_has_fn               },
    { "delete",                reflect_delete_fn            },
    { "freeze",                reflect_freeze_fn            },
    { "isFrozen",              reflect_is_frozen_fn         },
    { "isExtensible",          reflect_is_extensible_fn     },
    { "define",                reflect_define_fn            },
    { "getPrototype",          reflect_get_prototype_fn     },
    { "ownPropertyDescriptor", reflect_own_prop_descriptor_fn },
    { "typeOf",                reflect_typeof_fn            },
    { NULL, NULL }
};

Module module_reflect(void) {
    Module m;
    m.name      = NULL;
    m.functions = reflect_fns;
    m.fn_count  = sizeof(reflect_fns)/sizeof(reflect_fns[0]) - 1;
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// ITER MODULE — Iterator and generator utilities
//
// Provides helper functions for working with generators and iterators:
//   iter.collect(gen)         → array    drain a generator into an array
//   iter.take(gen, n)         → array    first n values from a generator
//   iter.map(gen, fn)         → array    transform each yielded value
//   iter.filter(gen, fn)      → array    keep values where fn(v) is truthy
//   iter.reduce(gen, fn, acc) → any      fold generator values
//   iter.forEach(gen, fn)     → null     side-effect over generator
//   iter.zip(gen1, gen2)      → array    [[a1,b1],[a2,b2],…] until either done
//   iter.enumerate(gen)       → array    [[0,v0],[1,v1],…]
//   iter.range(start,end,step)→ array    numeric range (like Python range())
//   iter.count(start, step)   → object   infinite counter generator object
// ═════════════════════════════════════════════════════════════════════════════

/* Note: iter_next, iter_result_done, iter_result_value were removed.
 * Iteration over generator objects is handled by NODE_FOR_OF in interpreter.c
 * which calls .next() directly. These helpers were dead code. */

/* iter.range(start, end, step=1) — numeric range array */
static Value *iter_range_fn(Value **args, size_t argc) {
    if (argc < 2) return value_array(NULL, 0);
    double start = args[0]->type == VAL_NUMBER ? args[0]->num : 0;
    double end   = args[1]->type == VAL_NUMBER ? args[1]->num : 0;
    double step  = (argc >= 3 && args[2]->type == VAL_NUMBER) ? args[2]->num : 1;
    if (step == 0) step = 1;
    size_t cap = 16, cnt = 0;
    Value **arr = (Value **)malloc(sizeof(Value *) * cap);
    for (double v = start;
         (step > 0) ? (v < end) : (v > end);
         v += step) {
        if (cnt >= cap) { cap *= 2; arr = (Value **)realloc(arr, sizeof(Value *)*cap); }
        arr[cnt++] = value_number(v);
    }
    Value *result = value_array(arr, cnt);
    free(arr);
    return result;
}

/* iter.collect — drain array or generator into an array */
static Value *iter_collect_fn(Value **args, size_t argc) {
    if (argc < 1) return value_array(NULL, 0);
    Value *src = args[0];
    /* If already an array, return copy */
    if (src->type == VAL_ARRAY) {
        Value **copy = (Value **)malloc(sizeof(Value *) * src->array_len);
        for (size_t i = 0; i < src->array_len; i++) copy[i] = src->array[i];
        Value *r = value_array(copy, src->array_len);
        free(copy);
        return r;
    }
    /* Generator: drain by calling .next() repeatedly */
    /* We can't call Xenly fns without interp, so return empty for generators
     * — full support requires the interpreter calling this inline.
     * Return the iterator unchanged so for-of can consume it. */
    return src;
}

/* iter.take(arr_or_gen, n) — first n elements */
static Value *iter_take_fn(Value **args, size_t argc) {
    if (argc < 2) return value_array(NULL, 0);
    Value *src = args[0];
    size_t n   = (args[1]->type == VAL_NUMBER) ? (size_t)args[1]->num : 0;
    if (src->type == VAL_ARRAY) {
        size_t take = src->array_len < n ? src->array_len : n;
        Value **out = (Value **)malloc(sizeof(Value *) * take);
        for (size_t i = 0; i < take; i++) out[i] = src->array[i];
        Value *r = value_array(out, take);
        free(out);
        return r;
    }
    return value_array(NULL, 0);
}

/* iter.enumerate(arr) — [[0,v0],[1,v1],…] */
static Value *iter_enumerate_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY)
        return value_array(NULL, 0);
    Value *src = args[0];
    Value **out = (Value **)malloc(sizeof(Value *) * src->array_len);
    for (size_t i = 0; i < src->array_len; i++) {
        Value *pair[2] = { value_number((double)i), src->array[i] };
        out[i] = value_array(pair, 2);
    }
    Value *r = value_array(out, src->array_len);
    free(out);
    return r;
}

/* iter.zip(a, b) — [[a0,b0],[a1,b1],…] stop at shorter */
static Value *iter_zip_fn(Value **args, size_t argc) {
    if (argc < 2 || !args[0] || !args[1]) return value_array(NULL, 0);
    Value *a = args[0], *b = args[1];
    if (a->type != VAL_ARRAY || b->type != VAL_ARRAY) return value_array(NULL, 0);
    size_t len = a->array_len < b->array_len ? a->array_len : b->array_len;
    Value **out = (Value **)malloc(sizeof(Value *) * len);
    for (size_t i = 0; i < len; i++) {
        Value *pair[2] = { a->array[i], b->array[i] };
        out[i] = value_array(pair, 2);
    }
    Value *r = value_array(out, len);
    free(out);
    return r;
}

/* iter.flatten(arr_of_arrs) — one level of nesting removed */
static Value *iter_flatten_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY)
        return value_array(NULL, 0);
    Value *src = args[0];
    size_t cap = 16, cnt = 0;
    Value **out = (Value **)malloc(sizeof(Value *) * cap);
    for (size_t i = 0; i < src->array_len; i++) {
        Value *item = src->array[i];
        if (item && item->type == VAL_ARRAY) {
            for (size_t j = 0; j < item->array_len; j++) {
                if (cnt >= cap) { cap*=2; out=(Value**)realloc(out,sizeof(Value*)*cap); }
                out[cnt++] = item->array[j];
            }
        } else {
            if (cnt >= cap) { cap*=2; out=(Value**)realloc(out,sizeof(Value*)*cap); }
            out[cnt++] = item;
        }
    }
    Value *r = value_array(out, cnt);
    free(out);
    return r;
}

/* iter.reverse(arr) — reversed copy */
static Value *iter_reverse_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY)
        return value_array(NULL, 0);
    Value *src = args[0];
    size_t n = src->array_len;
    Value **out = (Value **)malloc(sizeof(Value *) * n);
    for (size_t i = 0; i < n; i++) out[i] = src->array[n - 1 - i];
    Value *r = value_array(out, n);
    free(out);
    return r;
}

/* iter.unique(arr) — deduplicated copy (O(n²) — fine for small arrays) */
static Value *iter_unique_fn(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY)
        return value_array(NULL, 0);
    Value *src = args[0];
    size_t cap = 8, cnt = 0;
    Value **out = (Value **)malloc(sizeof(Value *) * cap);
    for (size_t i = 0; i < src->array_len; i++) {
        Value *v = src->array[i];
        int dup = 0;
        for (size_t j = 0; j < cnt; j++) {
            Value *u = out[j];
            if (!v && !u) { dup=1; break; }
            if (!v || !u) continue;
            if (v->type != u->type) continue;
            if (v->type == VAL_NUMBER && v->num == u->num) { dup=1; break; }
            if (v->type == VAL_STRING && v->str && u->str &&
                strcmp(v->str, u->str)==0) { dup=1; break; }
            if (v->type == VAL_BOOL && v->boolean == u->boolean) { dup=1; break; }
        }
        if (!dup) {
            if (cnt >= cap) { cap*=2; out=(Value**)realloc(out,sizeof(Value*)*cap); }
            out[cnt++] = v;
        }
    }
    Value *r = value_array(out, cnt);
    free(out);
    return r;
}

/* iter.chunk(arr, n) — split arr into sub-arrays of size n */
static Value *iter_chunk_fn(Value **args, size_t argc) {
    if (argc < 2 || !args[0] || args[0]->type != VAL_ARRAY)
        return value_array(NULL, 0);
    Value *src = args[0];
    size_t n   = (args[1]->type == VAL_NUMBER && args[1]->num >= 1)
               ? (size_t)args[1]->num : 1;
    size_t chunks = (src->array_len + n - 1) / n;
    Value **out  = (Value **)malloc(sizeof(Value *) * chunks);
    for (size_t c = 0; c < chunks; c++) {
        size_t start = c * n;
        size_t end   = start + n < src->array_len ? start + n : src->array_len;
        size_t len   = end - start;
        Value **sub  = (Value **)malloc(sizeof(Value *) * len);
        for (size_t i = 0; i < len; i++) sub[i] = src->array[start + i];
        out[c] = value_array(sub, len);
        free(sub);
    }
    Value *r = value_array(out, chunks);
    free(out);
    return r;
}

static NativeFunc iter_fns[] = {
    { "range",     iter_range_fn     },
    { "collect",   iter_collect_fn   },
    { "take",      iter_take_fn      },
    { "enumerate", iter_enumerate_fn },
    { "zip",       iter_zip_fn       },
    { "flatten",   iter_flatten_fn   },
    { "reverse",   iter_reverse_fn   },
    { "unique",    iter_unique_fn    },
    { "chunk",     iter_chunk_fn     },
    { NULL, NULL }
};

Module module_iter(void) {
    Module m;
    m.name      = NULL;
    m.functions = iter_fns;
    m.fn_count  = sizeof(iter_fns)/sizeof(iter_fns[0]) - 1;
    return m;
}

// ═════════════════════════════════════════════════════════════════════════════
// HTTP MODULE  — wraps xly_http.c for use via  import "http"
//
// The http module is intentionally NOT auto-imported: starting a server
// thread in every program would be wrong.  Users must explicitly:
//
//   import "http"
//   const srv = http.create()
//   http.get(srv, "/", fn(ctx) { http.send_text(ctx, 200, "Hello") })
//   http.listen(srv, "0.0.0.0", 8080)
//   http.run(srv)
//
// All XlyVal* wrappers delegate to the xly_http_*_val functions defined in
// xly_http.c which are already compiled into libxly_rt.a.
// ═════════════════════════════════════════════════════════════════════════════

/* xly_http.h is now safe to include after interpreter.h because it detects
 * INTERPRETER_H and uses typedef Value XlyVal instead of including xly_rt.h.
 * All xly_http_*_val functions are compiled into the interpreter binary via
 * INTERP_SRCS (src/xly_http.c). No extern declarations needed. */
#include "xly_http.h"

/* Module function wrappers — bridge NativeFunc (Value**,size_t) to xly_http_*_val */

static Value *http_create(Value **args, size_t argc) {
    return xly_http_create_val(argc > 0 ? args[0] : NULL);
}
static Value *http_listen(Value **args, size_t argc) {
    if (argc < 3) return value_bool(0);
    return xly_http_listen_val(args[0], args[1], args[2]);
}
static Value *http_run(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    return xly_http_run_val(args[0]);
}
static Value *http_run_async(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    return xly_http_run_async_val(args[0]);
}
static Value *http_stop(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    return xly_http_stop_val(args[0]);
}
static Value *http_route(Value **args, size_t argc) {
    if (argc < 4) return value_bool(0);
    return xly_http_route_val(args[0], args[1], args[2], args[3],
                               argc > 4 ? args[4] : NULL);
}
static Value *http_get(Value **args, size_t argc) {
    if (argc < 3) return value_bool(0);
    return xly_http_get_val(args[0], args[1], args[2]);
}
static Value *http_post(Value **args, size_t argc) {
    if (argc < 3) return value_bool(0);
    return xly_http_post_val(args[0], args[1], args[2]);
}
static Value *http_put(Value **args, size_t argc) {
    if (argc < 3) return value_bool(0);
    return xly_http_put_val(args[0], args[1], args[2]);
}
static Value *http_delete(Value **args, size_t argc) {
    if (argc < 3) return value_bool(0);
    return xly_http_delete_val(args[0], args[1], args[2]);
}
static Value *http_send_text(Value **args, size_t argc) {
    if (argc < 3) return value_null();
    return xly_http_send_text_val(args[0], args[1], args[2]);
}
static Value *http_send_json(Value **args, size_t argc) {
    if (argc < 3) return value_null();
    return xly_http_send_json_val(args[0], args[1], args[2]);
}
static Value *http_param(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    return xly_http_param_val(args[0], args[1]);
}
static Value *http_query(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    return xly_http_query_val(args[0], args[1]);
}
static Value *http_header(Value **args, size_t argc) {
    if (argc < 2) return value_null();
    return xly_http_req_header_val(args[0], args[1]);
}
static Value *http_to_json(Value **args, size_t argc) {
    if (argc < 1) return value_string("null");
    return xly_http_to_json_val(args[0]);
}
static Value *http_from_json(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    return xly_http_from_json_val(args[0]);
}
static Value *http_version(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_string("xly_http/1.0.0");
}

static NativeFunc http_fns[] = {
    { "create",     http_create     },
    { "listen",     http_listen     },
    { "run",        http_run        },
    { "runAsync",   http_run_async  },
    { "stop",       http_stop       },
    { "route",      http_route      },
    { "get",        http_get        },
    { "post",       http_post       },
    { "put",        http_put        },
    { "delete",     http_delete     },
    { "sendText",   http_send_text  },
    { "sendJson",   http_send_json  },
    { "param",      http_param      },
    { "query",      http_query      },
    { "header",     http_header     },
    { "toJson",     http_to_json    },
    { "fromJson",   http_from_json  },
    { "version",    http_version    },
    { NULL, NULL }
};

Module module_http(void) {
    Module m;
    m.name      = NULL;
    m.functions = http_fns;
    m.fn_count  = sizeof(http_fns)/sizeof(http_fns[0]) - 1;
    return m;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MATRIX / NUMERIC / DIFF HELPERS  (interpreter-side Value* helpers)
 * ═══════════════════════════════════════════════════════════════════════════ */
static inline double mv_dbl(Value *v) {
    return (v && v->type == VAL_NUMBER) ? v->num : 0.0;
}
static inline double mget(Value *M, size_t i, size_t j) {
    if (!M || M->type != VAL_ARRAY || i >= M->array_len) return 0.0;
    Value *row = M->array[i];
    if (!row || row->type != VAL_ARRAY || j >= row->array_len) return 0.0;
    return mv_dbl(row->array[j]);
}
static inline void mset(Value *M, size_t i, size_t j, double v) {
    if (!M || M->type != VAL_ARRAY || i >= M->array_len) return;
    Value *row = M->array[i];
    if (!row || row->type != VAL_ARRAY || j >= row->array_len) return;
    row->array[j] = value_number(v);
}
static Value *mcreate(size_t rows, size_t cols, double val) {
    if (rows == 0 || cols == 0) return value_array(NULL, 0);
    Value **ra = (Value **)malloc(sizeof(Value *) * rows);
    for (size_t i = 0; i < rows; i++) {
        Value **ca = (Value **)malloc(sizeof(Value *) * cols);
        for (size_t j = 0; j < cols; j++) ca[j] = value_number(val);
        ra[i] = value_array(ca, cols);
        free(ca);
    }
    Value *M = value_array(ra, rows);
    free(ra);
    return M;
}
static inline size_t mrows(Value *M) { return (M && M->type == VAL_ARRAY) ? M->array_len : 0; }
static inline size_t mcols(Value *M) {
    if (!M || M->type != VAL_ARRAY || !M->array_len) return 0;
    Value *r = M->array[0];
    return (r && r->type == VAL_ARRAY) ? r->array_len : 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * NUMERIC MODULE (non-callback functions only; callbacks handled in interpreter.c)
 * ═══════════════════════════════════════════════════════════════════════════ */
static Value *num_horner(Value **args, size_t argc) {
    if (argc < 2 || !args[0] || args[0]->type != VAL_ARRAY) return value_number(0.0);
    double x = mv_dbl(args[1]), r = 0.0;
    for (size_t k = 0; k < args[0]->array_len; k++) r = r * x + mv_dbl(args[0]->array[k]);
    return value_number(r);
}
static Value *num_fastInvSqrt(Value **args, size_t argc) {
    if (argc < 1) return value_number(0.0);
    float x = (float)mv_dbl(args[0]), x2 = x * 0.5f;
    union { float f; uint32_t i; } u;
    u.f = x; u.i = 0x5f3759dfU - (u.i >> 1);
    u.f *= (1.5f - x2 * u.f * u.f);
    u.f *= (1.5f - x2 * u.f * u.f);
    return value_number((double)u.f);
}
static Value *num_dot(Value **args, size_t argc) {
    if (argc < 2 || !args[0] || !args[1] || args[0]->type != VAL_ARRAY || args[1]->type != VAL_ARRAY) return value_number(0.0);
    size_t n = args[0]->array_len < args[1]->array_len ? args[0]->array_len : args[1]->array_len;
    double s = 0.0;
    for (size_t k = 0; k < n; k++) s += mv_dbl(args[0]->array[k]) * mv_dbl(args[1]->array[k]);
    return value_number(s);
}
static Value *num_cross(Value **args, size_t argc) {
    if (argc < 2 || !args[0] || !args[1] || args[0]->array_len < 3 || args[1]->array_len < 3) return value_null();
    double ax=mv_dbl(args[0]->array[0]),ay=mv_dbl(args[0]->array[1]),az=mv_dbl(args[0]->array[2]);
    double bx=mv_dbl(args[1]->array[0]),by=mv_dbl(args[1]->array[1]),bz=mv_dbl(args[1]->array[2]);
    Value *e[3]={value_number(ay*bz-az*by),value_number(az*bx-ax*bz),value_number(ax*by-ay*bx)};
    return value_array(e, 3);
}
static Value *num_norm1(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY) return value_number(0.0);
    double s = 0.0;
    for (size_t k=0; k<args[0]->array_len; k++) s += fabs(mv_dbl(args[0]->array[k]));
    return value_number(s);
}
static Value *num_norm2(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY) return value_number(0.0);
    double s = 0.0;
    for (size_t k=0; k<args[0]->array_len; k++) { double v=mv_dbl(args[0]->array[k]); s+=v*v; }
    return value_number(sqrt(s));
}
static Value *num_normInf(Value **args, size_t argc) {
    if (argc < 1 || !args[0] || args[0]->type != VAL_ARRAY) return value_number(0.0);
    double m = 0.0;
    for (size_t k=0; k<args[0]->array_len; k++) { double v=fabs(mv_dbl(args[0]->array[k])); if(v>m) m=v; }
    return value_number(m);
}
static Value *num_lagrange(Value **args, size_t argc) {
    if (argc < 3 || !args[0] || !args[1] || args[0]->type != VAL_ARRAY || args[1]->type != VAL_ARRAY) return value_number(0.0);
    size_t nn = args[0]->array_len < args[1]->array_len ? args[0]->array_len : args[1]->array_len;
    double x = mv_dbl(args[2]), result = 0.0;
    for (size_t i=0; i<nn; i++) {
        double xi=mv_dbl(args[0]->array[i]), yi=mv_dbl(args[1]->array[i]), term=yi;
        for (size_t j=0; j<nn; j++) {
            if (j==i) continue;
            double d=xi-mv_dbl(args[0]->array[j]);
            if (fabs(d)<1e-15) continue;
            term*=(x-mv_dbl(args[0]->array[j]))/d;
        }
        result+=term;
    }
    return value_number(result);
}
static Value *num_chebyshev(Value **args, size_t argc) {
    if (argc < 2) return value_number(0.0);
    int ord=(int)mv_dbl(args[0]); double x=mv_dbl(args[1]);
    if (ord==0) return value_number(1.0);
    if (ord==1) return value_number(x);
    double t0=1.0,t1=x,t2=0.0;
    for (int i=2;i<=ord;i++){t2=2.0*x*t1-t0;t0=t1;t1=t2;}
    return value_number(t2);
}
/* Stubs for callback-based functions (real impl in interpreter.c call_module_fn) */
static Value *num_stub(Value **args, size_t argc) { (void)args;(void)argc; return value_null(); }

static NativeFunc numeric_fns[] = {
    { "horner",       num_horner },
    { "polyEval",     num_horner },
    { "fastInvSqrt",  num_fastInvSqrt },
    { "dot",          num_dot },
    { "cross",        num_cross },
    { "norm1",        num_norm1 },
    { "norm2",        num_norm2 },
    { "norm",         num_norm2 },
    { "normInf",      num_normInf },
    { "lagrange",     num_lagrange },
    { "chebyshev",    num_chebyshev },
    /* callback-based: implemented in interpreter.c, stubs here so import works */
    { "bisect",       num_stub },
    { "secant",       num_stub },
    { "newton",       num_stub },
    { "integrate",    num_stub },
    { "simpson",      num_stub },
    { "trapezoid",    num_stub },
    { "romberg",      num_stub },
    { NULL, NULL }
};
static Module module_numeric(void) {
    Module m; m.name=NULL;
    m.functions=numeric_fns;
    m.fn_count=sizeof(numeric_fns)/sizeof(numeric_fns[0])-1;
    return m;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MATRIX MODULE — full implementation (no callbacks needed)
 * ═══════════════════════════════════════════════════════════════════════════ */
static Value *mat_create_fn(Value **a, size_t c) {
    size_t r=(size_t)mv_dbl(c>=1?a[0]:NULL), co=(size_t)mv_dbl(c>=2?a[1]:NULL);
    return mcreate(r, co, c>=3?mv_dbl(a[2]):0.0);
}
static Value *mat_zeros_fn(Value **a, size_t c) {
    size_t r=(size_t)mv_dbl(c>=1?a[0]:NULL);
    size_t co=(c>=2)?((size_t)mv_dbl(a[1])):r;
    return mcreate(r, co, 0.0);
}
static Value *mat_eye_fn(Value **a, size_t c) {
    size_t n=(size_t)mv_dbl(c>=1?a[0]:NULL);
    Value *M=mcreate(n,n,0.0);
    for (size_t i=0;i<n;i++) mset(M,i,i,1.0);
    return M;
}
static Value *mat_rows_fn(Value **a, size_t c) { return value_number((double)mrows(c>=1?a[0]:NULL)); }
static Value *mat_cols_fn(Value **a, size_t c) { return value_number((double)mcols(c>=1?a[0]:NULL)); }
static Value *mat_get_fn(Value **a, size_t c) {
    if (c<3) return value_number(0.0);
    return value_number(mget(a[0],(size_t)mv_dbl(a[1]),(size_t)mv_dbl(a[2])));
}
static Value *mat_set_fn(Value **a, size_t c) {
    if (c<4||!a[0]) return value_null();
    mset(a[0],(size_t)mv_dbl(a[1]),(size_t)mv_dbl(a[2]),mv_dbl(a[3]));
    return a[0];
}
static Value *mat_transpose_fn(Value **a, size_t c) {
    if (c<1||!a[0]) return value_null();
    size_t r=mrows(a[0]), co=mcols(a[0]);
    Value *T=mcreate(co,r,0.0);
    for (size_t i=0;i<r;i++) for (size_t j=0;j<co;j++) mset(T,j,i,mget(a[0],i,j));
    return T;
}
static Value *mat_add_fn(Value **a, size_t c) {
    if (c<2||!a[0]||!a[1]) return value_null();
    size_t r=mrows(a[0]), co=mcols(a[0]);
    Value *C=mcreate(r,co,0.0);
    for (size_t i=0;i<r;i++) for (size_t j=0;j<co;j++) mset(C,i,j,mget(a[0],i,j)+mget(a[1],i,j));
    return C;
}
static Value *mat_sub_fn(Value **a, size_t c) {
    if (c<2||!a[0]||!a[1]) return value_null();
    size_t r=mrows(a[0]), co=mcols(a[0]);
    Value *C=mcreate(r,co,0.0);
    for (size_t i=0;i<r;i++) for (size_t j=0;j<co;j++) mset(C,i,j,mget(a[0],i,j)-mget(a[1],i,j));
    return C;
}
static Value *mat_scale_fn(Value **a, size_t c) {
    if (c<2||!a[0]) return value_null();
    size_t r=mrows(a[0]), co=mcols(a[0]); double s=mv_dbl(a[1]);
    Value *C=mcreate(r,co,0.0);
    for (size_t i=0;i<r;i++) for (size_t j=0;j<co;j++) mset(C,i,j,mget(a[0],i,j)*s);
    return C;
}
static Value *mat_mul_fn(Value **a, size_t c) {
    if (c<2||!a[0]||!a[1]) return value_null();
    size_t ra=mrows(a[0]),ca=mcols(a[0]),cb=mcols(a[1]);
    Value *C=mcreate(ra,cb,0.0);
    for (size_t i=0;i<ra;i++) for (size_t j=0;j<cb;j++){
        double s=0.0;
        for (size_t k=0;k<ca;k++) s+=mget(a[0],i,k)*mget(a[1],k,j);
        mset(C,i,j,s);
    }
    return C;
}
static Value *mat_matvec_fn(Value **a, size_t c) {
    if (c<2||!a[0]||!a[1]||a[1]->type!=VAL_ARRAY) return value_null();
    size_t r=mrows(a[0]),co=a[1]->array_len;
    Value **out=(Value**)malloc(sizeof(Value*)*r);
    for (size_t i=0;i<r;i++){
        double s=0.0;
        for (size_t j=0;j<co;j++) s+=mget(a[0],i,j)*mv_dbl(a[1]->array[j]);
        out[i]=value_number(s);
    }
    Value *res=value_array(out,r); free(out); return res;
}
static Value *mat_dot_fn(Value **a, size_t c) {
    if (c<2||!a[0]||!a[1]||a[0]->type!=VAL_ARRAY||a[1]->type!=VAL_ARRAY) return value_number(0.0);
    size_t n=a[0]->array_len<a[1]->array_len?a[0]->array_len:a[1]->array_len;
    double s=0.0;
    for (size_t k=0;k<n;k++) s+=mv_dbl(a[0]->array[k])*mv_dbl(a[1]->array[k]);
    return value_number(s);
}
static Value *mat_trace_fn(Value **a, size_t c) {
    if (c<1||!a[0]) return value_number(0.0);
    size_t n=mrows(a[0]); double s=0.0;
    for (size_t i=0;i<n;i++) s+=mget(a[0],i,i);
    return value_number(s);
}
static Value *mat_norm_fn(Value **a, size_t c) {
    if (c<1||!a[0]||a[0]->type!=VAL_ARRAY) return value_number(0.0);
    double s=0.0;
    for (size_t i=0;i<a[0]->array_len;i++){
        Value *row=a[0]->array[i];
        if (!row||row->type!=VAL_ARRAY) continue;
        for (size_t j=0;j<row->array_len;j++){double v=mv_dbl(row->array[j]);s+=v*v;}
    }
    return value_number(sqrt(s));
}
static Value *mat_det_fn(Value **a, size_t c) {
    if (c<1||!a[0]||a[0]->type!=VAL_ARRAY) return value_number(0.0);
    size_t n=a[0]->array_len;
    if (n==0) return value_number(1.0);
    double *M=(double*)malloc(sizeof(double)*n*n);
    for (size_t i=0;i<n;i++) for (size_t j=0;j<n;j++) M[i*n+j]=mget(a[0],i,j);
    double det=1.0; int sign=1;
    for (size_t col=0;col<n;col++){
        size_t piv=col; double mx=fabs(M[col*n+col]);
        for (size_t r=col+1;r<n;r++) if(fabs(M[r*n+col])>mx){mx=fabs(M[r*n+col]);piv=r;}
        if (fabs(M[piv*n+col])<1e-12){free(M);return value_number(0.0);}
        if (piv!=col){for(size_t j=0;j<n;j++){double t=M[col*n+j];M[col*n+j]=M[piv*n+j];M[piv*n+j]=t;}sign=-sign;}
        det*=M[col*n+col];
        for (size_t r=col+1;r<n;r++){double f=M[r*n+col]/M[col*n+col];for(size_t j=col;j<n;j++) M[r*n+j]-=f*M[col*n+j];}
    }
    free(M); return value_number(det*sign);
}
static Value *mat_inv_fn(Value **a, size_t c) {
    if (c<1||!a[0]||a[0]->type!=VAL_ARRAY) return value_null();
    size_t n=a[0]->array_len;
    double *F=(double*)malloc(sizeof(double)*n*2*n);
    for (size_t i=0;i<n;i++){
        for (size_t j=0;j<n;j++) F[i*(2*n)+j]=mget(a[0],i,j);
        for (size_t j=0;j<n;j++) F[i*(2*n)+n+j]=(i==j)?1.0:0.0;
    }
    for (size_t col=0;col<n;col++){
        size_t piv=col; double mx=fabs(F[col*(2*n)+col]);
        for (size_t r=col+1;r<n;r++) if(fabs(F[r*(2*n)+col])>mx){mx=fabs(F[r*(2*n)+col]);piv=r;}
        if (piv!=col) for(size_t j=0;j<2*n;j++){double t=F[col*(2*n)+j];F[col*(2*n)+j]=F[piv*(2*n)+j];F[piv*(2*n)+j]=t;}
        double d=F[col*(2*n)+col];
        if (fabs(d)<1e-12){free(F);return value_null();}
        for(size_t j=0;j<2*n;j++) F[col*(2*n)+j]/=d;
        for(size_t r=0;r<n;r++){if(r==col)continue;double f=F[r*(2*n)+col];for(size_t j=0;j<2*n;j++) F[r*(2*n)+j]-=f*F[col*(2*n)+j];}
    }
    Value *inv=mcreate(n,n,0.0);
    for (size_t i=0;i<n;i++) for (size_t j=0;j<n;j++) mset(inv,i,j,F[i*(2*n)+n+j]);
    free(F); return inv;
}
static Value *mat_solve_fn(Value **a, size_t c) {
    if (c<2||!a[0]||!a[1]||a[0]->type!=VAL_ARRAY||a[1]->type!=VAL_ARRAY) return value_null();
    size_t n=a[0]->array_len;
    double *M=(double*)malloc(sizeof(double)*n*(n+1));
    for (size_t i=0;i<n;i++){
        for (size_t j=0;j<n;j++) M[i*(n+1)+j]=mget(a[0],i,j);
        M[i*(n+1)+n]=(i<a[1]->array_len)?mv_dbl(a[1]->array[i]):0.0;
    }
    for (size_t col=0;col<n;col++){
        size_t piv=col; double mx=fabs(M[col*(n+1)+col]);
        for (size_t r=col+1;r<n;r++) if(fabs(M[r*(n+1)+col])>mx){mx=fabs(M[r*(n+1)+col]);piv=r;}
        if (piv!=col) for(size_t j=0;j<=n;j++){double t=M[col*(n+1)+j];M[col*(n+1)+j]=M[piv*(n+1)+j];M[piv*(n+1)+j]=t;}
        if (fabs(M[col*(n+1)+col])<1e-12){free(M);return value_null();}
        for(size_t r=col+1;r<n;r++){double f=M[r*(n+1)+col]/M[col*(n+1)+col];for(size_t j=col;j<=n;j++) M[r*(n+1)+j]-=f*M[col*(n+1)+j];}
    }
    Value **xarr=(Value**)malloc(sizeof(Value*)*n);
    for (int i=(int)n-1;i>=0;i--){
        double s=M[i*(n+1)+n];
        for (size_t j=(size_t)i+1;j<n;j++) s-=M[i*(n+1)+j]*xarr[j]->num;
        xarr[i]=value_number(s/M[i*(n+1)+i]);
    }
    Value *x=value_array(xarr,n); free(M); free(xarr); return x;
}
static Value *mat_lu_fn(Value **a, size_t c) {
    if (c<1||!a[0]||a[0]->type!=VAL_ARRAY) return value_null();
    size_t n=a[0]->array_len;
    Value *L=mcreate(n,n,0.0), *U=mcreate(n,n,0.0);
    for (size_t i=0;i<n;i++) for (size_t j=0;j<n;j++) mset(U,i,j,mget(a[0],i,j));
    for (size_t i=0;i<n;i++) mset(L,i,i,1.0);
    for (size_t k=0;k<n;k++) for (size_t i=k+1;i<n;i++){
        double ukk=mget(U,k,k);
        if (fabs(ukk)<1e-15) continue;
        double f=mget(U,i,k)/ukk;
        mset(L,i,k,f);
        for (size_t j=k;j<n;j++) mset(U,i,j,mget(U,i,j)-f*mget(U,k,j));
    }
    Value *res[2]={L,U}; return value_array(res,2);
}
static Value *mat_rank_fn(Value **a, size_t c) {
    if (c<1||!a[0]||a[0]->type!=VAL_ARRAY) return value_number(0);
    size_t rows=mrows(a[0]),cols=mcols(a[0]);
    double *M=(double*)malloc(sizeof(double)*rows*cols);
    for (size_t i=0;i<rows;i++) for (size_t j=0;j<cols;j++) M[i*cols+j]=mget(a[0],i,j);
    size_t rank=0,row=0;
    for (size_t col=0;col<cols&&row<rows;col++){
        size_t piv=row; double mx=fabs(M[row*cols+col]);
        for (size_t r=row+1;r<rows;r++) if(fabs(M[r*cols+col])>mx){mx=fabs(M[r*cols+col]);piv=r;}
        if (mx<1e-10) continue;
        if (piv!=row) for(size_t j=0;j<cols;j++){double t=M[row*cols+j];M[row*cols+j]=M[piv*cols+j];M[piv*cols+j]=t;}
        for(size_t r=row+1;r<rows;r++){double f=M[r*cols+col]/M[row*cols+col];for(size_t j=col;j<cols;j++) M[r*cols+j]-=f*M[row*cols+j];}
        rank++;row++;
    }
    free(M); return value_number((double)rank);
}
static Value *mat_powerIter_fn(Value **a, size_t c) {
    if (c<1||!a[0]||a[0]->type!=VAL_ARRAY) return value_null();
    size_t n=mrows(a[0]); int iters=(c>=2&&a[1])?(int)mv_dbl(a[1]):100;
    double *v=(double*)calloc(n,sizeof(double)),*w=(double*)malloc(n*sizeof(double));
    v[0]=1.0; double eigenval=0.0;
    for (int it=0;it<iters;it++){
        for (size_t i=0;i<n;i++){w[i]=0.0;for(size_t j=0;j<n;j++) w[i]+=mget(a[0],i,j)*v[j];}
        eigenval=w[0]; double mx=fabs(w[0]);
        for (size_t i=1;i<n;i++) if(fabs(w[i])>mx){mx=fabs(w[i]);eigenval=w[i];}
        if (mx<1e-15) break;
        for (size_t i=0;i<n;i++) v[i]=w[i]/eigenval;
    }
    free(v);free(w); return value_number(eigenval);
}

static NativeFunc matrix_fns[] = {
    { "create",      mat_create_fn },
    { "zeros",       mat_zeros_fn },
    { "eye",         mat_eye_fn },
    { "identity",    mat_eye_fn },
    { "rows",        mat_rows_fn },
    { "cols",        mat_cols_fn },
    { "get",         mat_get_fn },
    { "set",         mat_set_fn },
    { "transpose",   mat_transpose_fn },
    { "add",         mat_add_fn },
    { "sub",         mat_sub_fn },
    { "scale",       mat_scale_fn },
    { "mul",         mat_mul_fn },
    { "matvec",      mat_matvec_fn },
    { "dot",         mat_dot_fn },
    { "trace",       mat_trace_fn },
    { "norm",        mat_norm_fn },
    { "fnorm",       mat_norm_fn },
    { "det",         mat_det_fn },
    { "inv",         mat_inv_fn },
    { "inverse",     mat_inv_fn },
    { "solve",       mat_solve_fn },
    { "lu",          mat_lu_fn },
    { "rank",        mat_rank_fn },
    { "powerIter",   mat_powerIter_fn },
    { NULL, NULL }
};
static Module module_matrix(void) {
    Module m; m.name=NULL;
    m.functions=matrix_fns;
    m.fn_count=sizeof(matrix_fns)/sizeof(matrix_fns[0])-1;
    return m;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DIFF MODULE — stubs only; all diff functions use callbacks → interpreter.c
 * ═══════════════════════════════════════════════════════════════════════════ */
static Value *diff_stub(Value **args, size_t argc) { (void)args;(void)argc; return value_null(); }

static NativeFunc diff_fns[] = {
    { "forward",    diff_stub },
    { "backward",   diff_stub },
    { "central",    diff_stub },
    { "deriv",      diff_stub },
    { "d",          diff_stub },
    { "second",     diff_stub },
    { "d2",         diff_stub },
    { "nth",        diff_stub },
    { "partial",    diff_stub },
    { "gradient",   diff_stub },
    { "grad",       diff_stub },
    { "hessian",    diff_stub },
    { "jacobian",   diff_stub },
    { "taylor",     diff_stub },
    { NULL, NULL }
};
static Module module_diff(void) {
    Module m; m.name=NULL;
    m.functions=diff_fns;
    m.fn_count=sizeof(diff_fns)/sizeof(diff_fns[0])-1;
    return m;
}
