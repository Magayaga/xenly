/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#define MAX_TOKEN_SIZE 1000
#define MAX_VARIABLES 1000
#define MAX_NUMBERS 1000
#define MAX_OBJECTS 1000
#define MAX_ARRAYS 100

typedef struct {
    char name[MAX_TOKEN_SIZE];
    char value[MAX_TOKEN_SIZE];
} Variable;

extern Variable variables[MAX_VARIABLES];
extern int result_variables;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    int type; // 0 for variable, 1 for object, 2 for array
    char value[MAX_TOKEN_SIZE];
} Data;

extern Data data_storage[MAX_VARIABLES + MAX_OBJECTS + MAX_ARRAYS];
extern int result_data;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    int type; // 0 for variable, 1 for object, 2 for array
    int size;
    double* elements;
} Array;

extern Array arrays[MAX_ARRAYS];
extern int result_arrays;
extern int multiline_comment;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    char version[MAX_TOKEN_SIZE];
} Module;

#endif // DATA_STRUCTURES_H