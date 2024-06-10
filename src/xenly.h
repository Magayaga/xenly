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

#define MAX_TOKEN_SIZE 1000
#define MAX_VARIABLES 1000
#define MAX_OBJECTS 1000
#define MAX_VALUE_LENGTH 256
#define MAX_ARRAYS 100
#define MAX_NUM_ARGS 10

typedef struct {
    char name[MAX_TOKEN_SIZE];
    char value[MAX_TOKEN_SIZE];
} Variable;

Variable variables[MAX_VARIABLES];
int num_variables = 0;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    int type; // 0 for variable, 1 for object, 2 for array
    char value[MAX_TOKEN_SIZE];
} Data;

Data data_storage[MAX_VARIABLES + MAX_OBJECTS + MAX_ARRAYS];
int num_data = 0;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    int type; // 0 for variable, 1 for object, 2 for array
    int size;
    double* elements;
} Array;

Array arrays[MAX_ARRAYS];
int num_arrays = 0;
int multiline_comment = 0;

// Evaluately condition
bool evaluately_condition(const char* condition);

// Print
void execute_print(const char* arg);

// Input
void execute_input(const char* message, char* buffer, int buffer_size);

// Parse numeric value
int parse_numeric_value(const char* value, double* result);

// Bool
void execute_bool(const char* name, bool value);
bool evaluate_bool(const char* value);

// Var
void execute_var(const char* name, const char* val);

// Parse and Execute Arithmetic operation
int parse_and_execute_arithmetic_operation(const char* operation, int* result);

// For
void execute_for(FILE* input_file, const char* loop_variable, int start_value, int end_value, const char* loop_body);

// Int
void execute_let(const char* name, const char* value);

// Square root function
double xe_sqrt(double x);

// Cube root function
double xe_cbrt(double x);

// Fifth root function
double ffrt(double x);

// Square root
double execute_sqrt(const char* arg);

// Cube root
double execute_cbrt(const char* arg);

// Pow
double execute_pow(const char* arg);

// Sin
double execute_sin(const char* arg);

// Cos
double execute_cos(const char* arg);

// Tan
double execute_tan(const char* arg);

// Gamma
double execute_gamma(const char* arg);

// Max
double xe_max(double x, double y);

// Min
double xe_min(double x, double y);

// Max function
double execute_max(const double* numbers, int count);

// Min function
double execute_min(const double* numbers, int count);

// Abs
double xe_abs(double x);

// Abs function
double execute_abs(const char* arg);

// Factorial function
int factorial(int n);

// Factorial numbers
int execute_factorial(const char* arg);

// Fibonacci function
int fibonacci(int n);

// Fibonacci numbers
int execute_fibonacci(const char* arg);

// Factor
double evaluate_factor(const char** expression);

// Term
double evaluate_term(const char** expression);

// Evaluate arithmetic expression
double evaluate_arithmetic_expression(const char* expression);

// Evaluate condition
double evaluate_condition(const char* condition);

// Object
void execute_object(const char* name, const char* properties);

// Array
void execute_array(const char* name, int size);

// Set
void execute_set(const char* array_name, int index, double value);

// Get
double execute_get(const char* array_name, int index);

#endif