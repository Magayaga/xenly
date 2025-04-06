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

#define MAX_VARIABLES 100
#define MAX_LINE_LENGTH 1024

typedef struct {
    char name[MAX_LINE_LENGTH];
    char type; // 's' for string, 'i' for int, 'f' for float, 'b' for bool
    union {
        char s[MAX_LINE_LENGTH];
        int i;
        float f;
        bool b;
    } value;
} Variable;

extern Variable variables[MAX_VARIABLES];
extern int variable_count;
extern bool in_multi_line_comment;

void process_line(char* line);
char* remove_comments(char* line);
void interpret_line(char* line);
void print_function(char* args);
void var_declaration(char* args);
void bool_declaration(char* args);
char* get_variable_value(const char* name);
double evaluate_expression(const char* expr);
char* trim(char* str);
bool parse_bool(const char* value);

#endif // XENLY_H