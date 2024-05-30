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

bool evaluately_condition(const char* condition);

void execute_comment(const char* comment);

double evaluate_condition(const char* condition);

void execute_for(FILE* input_file, const char* loop_variable, int start_value, int end_value, const char* loop_body);

double evaluate_arithmetic_expression(const char* expression);

void execute_print(const char* arg);

void execute_input(const char* message, char* buffer, int buffer_size);

int parse_numeric_value(const char* value, double* result);

void execute_bool(const char* name, bool value);

bool evaluate_bool(const char* value);

void execute_var(const char* name, const char* value);

void execute_let(const char* name, const char* value);

double evaluate_factor(const char** expression);

double evaluate_term(const char** expression);

void execute_object(const char* name, const char* properties);

void execute_array(const char* name, int size);

void execute_set(const char* array_name, int index, double value);

double execute_get(const char* array_name, int index);


#endif