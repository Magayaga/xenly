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

// Define a function to calculate the square root
void xenly_sqrt(double x) {
    printf("%lf\n", sqrt(x));
}

// Define a function to calculate the power
void xenly_pow(double base, double exponent) {
    printf("%lf\n", pow(base, exponent));
}

// Define a function to calculate the sine
void xenly_sin(double x) {
    printf("%lf\n", sin(x));
}

// Define a function to calculate the cosine
void xenly_cos(double x) {
    printf("%lf\n", cos(x));
}

// Define a function to calculate the tangent
void xenly_tan(double x) {
    printf("%lf\n", tan(x));
}
