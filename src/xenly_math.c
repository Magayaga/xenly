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
#include <math.h>
#include "xenly_math.h"

#define MATH_PI 3.14159265358979323846
#define MATH_TAU 6.28318530717958647692
#define MATH_E 2.71828182845904523536
#define MATH_GOLDEN_RATIO 1.61803398874989484820
#define MATH_SILVER_RATIO 2.41421356237309504880
#define MATH_SUPERGOLDEN_RATIO 1.46557123187676802665

// Define a function to calculate the square root
double xenly_sqrt(double x) {
    return sqrt(x);
}

// Define a function to calculate the power
double xenly_pow(double base, double exp) {
    return pow(base, exp);
}

// Define a function to calculate the sine
double xenly_sin(double x) {
    return sin(x);
}

// Define a function to calculate the cosine
double xenly_cos(double x) {
    return cos(x);
}

// Define a function to calculate the tangent
double xenly_tan(double x) {
    return tan(x);
}
