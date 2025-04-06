/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#ifndef BINARY_MATH_FUNCTIONS_H
#define BINARY_MATH_FUNCTIONS_H

typedef double (*xenly_bindec_t)(const char*);
typedef char* (*xenly_decbin_t)(int);

extern xenly_bindec_t xenly_bindec;
extern xenly_decbin_t xenly_decbin;

void load_binary_math_module(const char* module_name);

#endif // BINARY_MATH_FUNCTIONS_H