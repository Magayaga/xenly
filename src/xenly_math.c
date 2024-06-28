/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * `xenly_math.c` is the similar to the `xenly_math.go` in Go programming language and `xenly_math.rs` in Rust
 * programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "xenly_math.h"

// Define and export mathematical constants and universal constants
#define MATH_PI 3.14159265358979323846
#define MATH_TAU 6.28318530717958647692
#define MATH_E 2.71828182845904523536
#define MATH_GOLDEN_RATIO 1.61803398874989484820
#define MATH_SILVER_RATIO 2.41421356237309504880
#define MATH_SUPERGOLDEN_RATIO 1.46557123187676802665
#define PHYSICAL_SPEED_OF_LIGHT_MS 299792458
#define PHYSICAL_SPEED_OF_LIGHT_KMH 1080000000
#define PHYSICAL_SPEED_OF_LIGHT_MileS 186000
#define PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2 0.0000000000667430
#define PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2 0.0000000667430

// Mathematical constants using functions
double pi() {
    return MATH_PI;
}

double tau() {
    return MATH_TAU;
}

double e() {
    return MATH_E;
}

double goldenRatio() {
    return MATH_GOLDEN_RATIO;
}

double silverRatio() {
    return MATH_SILVER_RATIO;
}

double superGoldenRatio() {
    return MATH_SUPERGOLDEN_RATIO;
}

// Universal constants using functions
double speedOfLight() {
    return PHYSICAL_SPEED_OF_LIGHT_MS;
}

double speedOfLight_kmh() {
    return PHYSICAL_SPEED_OF_LIGHT_KMH;
}

double speedOfLight_MileS() {
    return PHYSICAL_SPEED_OF_LIGHT_MileS;
}

double gravitationalConstant() {
    return PHYSICAL_GRAVTIATIONAL_CONSTANT_N_M2__KG2;
}

double gravitationalConstant_dyncm2g2() {
    return PHYSICAL_GRAVTIATIONAL_CONSTANT_DYN_CM2__G2;
}

// Define a function to calculate the square root
double xenly_sqrt(double x) {
    return sqrt(x);
}

// Define a function to calculate the cube root
double xenly_cbrt(double x) {
    return cbrt(x);
}

// Define a function to calculate the fifth root
double xenly_ffrt(double x) {
    return pow(x, 1.0 / 5.0);
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

// Define a function to calculate the cosecant
double xenly_csc(double x) {
    return 1 / sin(x);
}

// Define a function to calculate the secant
double xenly_sec(double x) {
    return 1 / cos(x);
}

// Define a function to calculate the cotangent
double xenly_cot(double x) {
    return 1 / tan(x);
}

// Define a function to calculate the minimum function (xe_min and xenly_min)
double xe_min(double x, double y) {
    return (x < y) ? x : y;
}

double xenly_min(const double* numbers, int count) {
    double min_value = numbers[0];
    for (int i = 1; i < count; i++) {
        min_value = xe_min(min_value, numbers[i]);
    }
    return min_value;
}

// Define a function to calculate the maximum function (xe_max and xenly_max)
double xe_max(double x, double y) {
    return (x > y) ? x : y;
}

double xenly_max(const double* numbers, int count) {
    double max_value = numbers[0];
    for (int i = 1; i < count; i++) {
        max_value = xe_max(max_value, numbers[i]);
    }
    return max_value;
}

// Define a function to calculate the absolute value function (xe_abs and xenly_abs)
double xe_abs(double x) {
    return x < 0 ? -x : x;
}

double xenly_abs(const char* arg) {
    double x = atoi(arg);
    return xe_abs(x);
}