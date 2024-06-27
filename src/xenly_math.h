/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#ifndef XENLY_MATH_H
#define XENLY_MATH_H

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
double pi();
double tau();
double e();
double goldenRatio();
double silverRatio();
double superGoldenRatio();

// Universal constants using functions
double speedOfLight();
double speedOfLight_kmh();
double speedOfLight_MileS();
double gravitationalConstant();
double gravitationalConstant_dyncm2g2();

// Define a function to calculate the square root
double xenly_sqrt(double x);

// Define a function to calculate the cube root
double xenly_cbrt(double x);

// Define a function to calculate the fifth root
double xenly_ffrt(double x);

// Define a function to calculate the power
double xenly_pow(double base, double exponent);

// Define a function to calculate the sine
double xenly_sin(double x);

// Define a function to calculate the cosine
double xenly_cos(double x);

// Define a function to calculate the tangent
double xenly_tan(double x);

// Define a function to calculate the cosecant
double xenly_csc(double x);

// Define a function to calculate the secant
double xenly_sec(double x);

// Define a function to calculate the cotangent
double xenly_cot(double x);

// Define a function to calculate the minimum function (xe_min and xenly_min)
double xe_min(double x, double y);
double xenly_min(const double* numbers, int count);

// Define a function to calculate the maximum function (xe_max and xenly_max)
double xe_max(double x, double y);
double xenly_max(const double* numbers, int count);

// Define a function to calculate the absolute value function (xe_abs and xenly_abs)
double xe_abs(double x);
double xenly_abs(const char* arg);

#endif
