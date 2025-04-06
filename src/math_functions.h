/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

typedef double (*xenly_constant_t)();
typedef double (*xenly_sqrt_t)(double);
typedef double (*xenly_cbrt_t)(double);
typedef double (*xenly_ffrt_t)(double);
typedef double (*xenly_pow_t)(double, double);
typedef double (*xenly_sin_t)(double);
typedef double (*xenly_cos_t)(double);
typedef double (*xenly_tan_t)(double);
typedef double (*xenly_csc_t)(double);
typedef double (*xenly_sec_t)(double);
typedef double (*xenly_cot_t)(double);
typedef double (*xe_min_t)(int, ...);
typedef double (*xe_max_t)(int, ...);
typedef double (*xenly_min_t)(int, ...);
typedef double (*xenly_max_t)(int, ...);
typedef double (*xenly_abs_t)(double);

extern xenly_constant_t pi;
extern xenly_constant_t tau;
extern xenly_constant_t e;
extern xenly_constant_t goldenRatio;
extern xenly_constant_t silverRatio;
extern xenly_constant_t superGoldenRatio;
extern xenly_sqrt_t xenly_sqrt;
extern xenly_cbrt_t xenly_cbrt;
extern xenly_ffrt_t xenly_ffrt;
extern xenly_pow_t xenly_pow;
extern xenly_sin_t xenly_sin;
extern xenly_cos_t xenly_cos;
extern xenly_tan_t xenly_tan;
extern xenly_csc_t xenly_csc;
extern xenly_sec_t xenly_sec;
extern xenly_cot_t xenly_cot;
extern xe_min_t xe_min;
extern xe_max_t xe_max;
extern xenly_min_t xenly_min;
extern xenly_max_t xenly_max;
extern xenly_abs_t xenly_abs;

void load_math_module(const char* module_name);

#endif // MATH_FUNCTIONS_H