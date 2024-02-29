/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */
#ifndef XENLY_H
#define XENLY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

bool evaluately_condition(const char* condition);
void error(const char* message);
void execute_comment(const char* comment);
double evaluate_condition(const char* condition);
double execute_sqrt(const char* arg);
void execute_for(FILE* input_file, const char* loop_variable, int start_value, int end_value, const char* loop_body);
double evaluate_arithmetic_expression(const char* expression);
void execute_print(const char* arg);
int parse_numeric_value(const char* value, double* result);
void execute_bool(const char* name, bool value);
bool evaluate_bool(const char* value);
void execute_var(const char* name, const char* value);
void execute_int(const char* name, const char* value);
double execute_cbrt(const char* arg);
double execute_pow(const char* arg);
double execute_sin(const char* arg);
double execute_cos(const char* arg);
double execute_tan(const char* arg);
double execute_gamma(const char* arg);
double max(double x, double y);
double min(double x, double y);
double execute_max(const double* numbers, int count);
double execute_min(const double* numbers, int count);
double execute_abs(const char* arg);
double ffrt(double x);
int factorial(int n);
int execute_factorial(const char* arg);
int fibonacci(int n);
int execute_fibonacci(const char* arg);
int convert_binary_to_decimal(const char* binary);
char* convert_decimal_to_binary(int decimal);
double evaluate_factor(const char** expression);
double evaluate_term(const char** expression);
void execute_object(const char* name, const char* properties);
void execute_array(const char* name, int size);
void execute_set(const char* array_name, int index, double value);
double execute_get(const char* array_name, int index);

#endif // XENLY_H
