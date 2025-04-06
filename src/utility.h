/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#ifndef UTILITY_H
#define UTILITY_H

void execute_print(const char* arg);
double evaluate_factor(const char** expression);
double evaluate_arithmetic_expression(const char** expression);
void execute_math_function(const char* line);
void execute_var(const char* line);
double evaluate_condition(const char* condition);

#endif // UTILITY_H