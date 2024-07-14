/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */

#ifndef XENLY_H
#define XENLY_H

#define MAX_LINE_LENGTH 1000
#define MAX_VARIABLES 100
#define MAX_INPUT_LENGTH 1000

typedef struct {
    char name[50];
    double value;
    char string_value[1000];
    int is_string;
} Variable;

void interpret_line(char* line);

void add_variable(const char* name, double value, const char* string_value, int is_string);

Variable* find_variable(const char* name);

char* trim(char* str);

int is_numeric(const char* str);

double evaluate_expression(const char* expr);

void execute_print(char* args);

char* execute_input(char* prompt);

#endif // XENLY_H
