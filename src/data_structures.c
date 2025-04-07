/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structures.h"

Variable variables[MAX_VARIABLES];
int result_variables = 0;

Data data_storage[MAX_VARIABLES + MAX_OBJECTS + MAX_ARRAYS];
int result_data = 0;

Array arrays[MAX_ARRAYS];
int result_arrays = 0;
int multiline_comment = 0;

void add_variable(const char* name, VarType type, const char* value) {
    if (result_variables >= MAX_VARIABLES) {
        fprintf(stderr, "Error: Maximum number of variables exceeded\n");
        return;
    }

    strcpy(variables[result_variables].name, name);
    variables[result_variables].type = type;

    switch (type) {
        case VAR_TYPE_INT:
            variables[result_variables].value.intValue = atoi(value);
            break;
        case VAR_TYPE_FLOAT:
            variables[result_variables].value.floatValue = atof(value);
            break;
        case VAR_TYPE_STRING:
            strcpy(variables[result_variables].value.stringValue, value);
            break;
        case VAR_TYPE_BOOL:
            if (strcmp(value, "true") == 0) {
                variables[result_variables].value.boolValue = 1;
            } else if (strcmp(value, "false") == 0) {
                variables[result_variables].value.boolValue = 0;
            } else {
                fprintf(stderr, "Error: Invalid boolean value '%s'\n", value);
                return;
            }
            break;
    }

    result_variables++;
}
