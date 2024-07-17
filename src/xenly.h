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

#include <stdbool.h>

#define MAX_LINE_LENGTH 1000
#define MAX_VARIABLES 100

typedef struct {
    char name[50];
    char type;  // 's' for string, 'i' for integer, 'f' for float, 'b' for boolean
    union {
        char s[100];
        int i;
        float f;
        bool b;
    } value;
} Variable;

// Function prototypes
void process_line(char* line);
void interpret_line(char* line);
void print_function(char* args);
void var_declaration(char* args);
void bool_declaration(char* args);
char* get_variable_value(const char* name);
double evaluate_expression(const char* expr);
char* trim(char* str);
bool parse_bool(const char* value);
char* remove_comments(char* line);

#endif // XENLY_H
