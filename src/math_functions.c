/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "math_functions.h"
#include "error.h"
#include "data_structures.h"

xenly_constant_t pi;
xenly_constant_t tau;
xenly_constant_t e;
xenly_constant_t goldenRatio;
xenly_constant_t silverRatio;
xenly_constant_t superGoldenRatio;
xenly_sqrt_t xenly_sqrt;
xenly_cbrt_t xenly_cbrt;
xenly_ffrt_t xenly_ffrt;
xenly_pow_t xenly_pow;
xenly_sin_t xenly_sin;
xenly_cos_t xenly_cos;
xenly_tan_t xenly_tan;
xenly_csc_t xenly_csc;
xenly_sec_t xenly_sec;
xenly_cot_t xenly_cot;
xe_min_t xe_min;
xe_max_t xe_max;
xenly_min_t xenly_min;
xenly_max_t xenly_max;
xenly_abs_t xenly_abs;

#if defined(_WIN32) || defined(_WIN64)
void load_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    sprintf(filename, "%s.%s", module_name, "dll");
    HMODULE handle = LoadLibrary(filename);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'\n", filename);
        return;
    }

    pi = (xenly_constant_t)(void*)GetProcAddress(handle, "pi");
    tau = (xenly_constant_t)(void*)GetProcAddress(handle, "tau");
    e = (xenly_constant_t)(void*)GetProcAddress(handle, "e");
    goldenRatio = (xenly_constant_t)(void*)GetProcAddress(handle, "goldenRatio");
    silverRatio = (xenly_constant_t)(void*)GetProcAddress(handle, "silverRatio");
    superGoldenRatio = (xenly_constant_t)(void*)GetProcAddress(handle, "superGoldenRatio");
    xenly_sqrt = (xenly_sqrt_t)(void*)GetProcAddress(handle, "xenly_sqrt");
    xenly_cbrt = (xenly_cbrt_t)(void*)GetProcAddress(handle, "xenly_cbrt");
    xenly_ffrt = (xenly_ffrt_t)(void*)GetProcAddress(handle, "xenly_ffrt");
    xenly_pow = (xenly_pow_t)(void*)GetProcAddress(handle, "xenly_pow");
    xenly_sin = (xenly_sin_t)(void*)GetProcAddress(handle, "xenly_sin");
    xenly_cos = (xenly_cos_t)(void*)GetProcAddress(handle, "xenly_cos");
    xenly_tan = (xenly_tan_t)(void*)GetProcAddress(handle, "xenly_tan");
    xenly_csc = (xenly_csc_t)(void*)GetProcAddress(handle, "xenly_csc");
    xenly_sec = (xenly_sec_t)(void*)GetProcAddress(handle, "xenly_sec");
    xenly_cot = (xenly_cot_t)(void*)GetProcAddress(handle, "xenly_cot");
    xe_min = (xe_min_t)(void*)GetProcAddress(handle, "xe_min");
    xe_max = (xe_max_t)(void*)GetProcAddress(handle, "xe_max");
    xenly_min = (xenly_min_t)(void*)GetProcAddress(handle, "xenly_min");
    xenly_max = (xenly_max_t)(void*)GetProcAddress(handle, "xenly_max");
    xenly_abs = (xenly_abs_t)(void*)GetProcAddress(handle, "xenly_abs");

    if (!pi || !tau || !e || !goldenRatio || !silverRatio || !superGoldenRatio || !xenly_sqrt || !xenly_cbrt || !xenly_ffrt || !xenly_pow || !xenly_sin || !xenly_cos || !xenly_tan || !xenly_csc || !xenly_sec || !xenly_cot || !xe_min || !xe_max || !xenly_min || !xenly_max || !xenly_abs) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }
}
#else
void load_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    snprintf(filename, sizeof(filename), "%s.%s", module_name, "so");

    void* handle = dlopen(filename, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'; %s\n", filename, dlerror());
        exit(EXIT_FAILURE);
    }

    pi = (xenly_constant_t)dlsym(handle, "pi");
    tau = (xenly_constant_t)dlsym(handle, "tau");
    e = (xenly_constant_t)dlsym(handle, "e");
    goldenRatio = (xenly_constant_t)dlsym(handle, "goldenRatio");
    silverRatio = (xenly_constant_t)dlsym(handle, "silverRatio");
    superGoldenRatio = (xenly_constant_t)dlsym(handle, "superGoldenRatio");
    xenly_sqrt = (xenly_sqrt_t)dlsym(handle, "xenly_sqrt");
    xenly_cbrt = (xenly_cbrt_t)dlsym(handle, "xenly_cbrt");
    xenly_ffrt = (xenly_ffrt_t)dlsym(handle, "xenly_ffrt");
    xenly_pow = (xenly_pow_t)dlsym(handle, "xenly_pow");
    xenly_sin = (xenly_sin_t)dlsym(handle, "xenly_sin");
    xenly_cos = (xenly_cos_t)dlsym(handle, "xenly_cos");
    xenly_tan = (xenly_tan_t)dlsym(handle, "xenly_tan");
    xenly_csc = (xenly_csc_t)dlsym(handle, "xenly_csc");
    xenly_sec = (xenly_sec_t)dlsym(handle, "xenly_sec");
    xenly_cot = (xenly_cot_t)dlsym(handle, "xenly_cot");
    xe_min = (xe_min_t)dlsym(handle, "xe_min");
    xe_max = (xe_max_t)dlsym(handle, "xe_max");
    xenly_min = (xenly_min_t)dlsym(handle, "xenly_min");
    xenly_max = (xenly_max_t)dlsym(handle, "xenly_max");
    xenly_abs = (xenly_abs_t)dlsym(handle, "xenly_abs");

    if (!pi || !tau || !e || !goldenRatio || !silverRatio || !superGoldenRatio || !xenly_sqrt || !xenly_cbrt || !xenly_ffrt || !xenly_pow || !xenly_sin || !xenly_cos || !xenly_tan || !xenly_csc || !xenly_sec || !xenly_cot || !xe_min || !xe_max || !xenly_min || !xenly_max || !xenly_abs) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }
}
#endif