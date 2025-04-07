/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#include "xenly.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int variable_count = 0;

void var_declaration(char* args) {
    char* name = strtok(args, "=");
    char* value = strtok(NULL, "");
    
    if (!name || !value) {
        printf("Error: Invalid variable declaration\n");
        return;
    }
    
    name = trim(name);
    value = trim(value);

    if (variable_count >= MAX_VARIABLES) {
        printf("Error: Maximum number of variables reached.\n");
        return;
    }

    strcpy(variables[variable_count].name, name);

    if (value[0] == '"' && value[strlen(value)-1] == '"') {
        variables[variable_count].type = 's';
        strncpy(variables[variable_count].value.s, value + 1, strlen(value) - 2);
        variables[variable_count].value.s[strlen(value) - 2] = '\0';
    } else if (strchr(value, '.') != NULL) {
        variables[variable_count].type = 'f';
        variables[variable_count].value.f = atof(value);
    } else {
        variables[variable_count].type = 'i';
        variables[variable_count].value.i = atoi(value);
    }

    variable_count++;
}

void bool_declaration(char* args) {
    char* name = strtok(args, "=");
    char* value = strtok(NULL, "");
    
    if (!name || !value) {
        printf("Error: Invalid boolean declaration\n");
        return;
    }
    
    name = trim(name);
    value = trim(value);

    if (variable_count >= MAX_VARIABLES) {
        printf("Error: Maximum number of variables reached.\n");
        return;
    }

    strcpy(variables[variable_count].name, name);
    variables[variable_count].type = 'b';
    variables[variable_count].value.b = parse_bool(value);

    variable_count++;
}

char* get_variable_value(const char* name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            static char buffer[100];
            switch (variables[i].type) {
                case 's':
                    return variables[i].value.s;
                
                case 'i':
                    sprintf(buffer, "%d", variables[i].value.i);
                    return buffer;
                
                case 'f':
                    sprintf(buffer, "%g", variables[i].value.f);
                    return buffer;
                
                case 'b':
                    sprintf(buffer, "%d", variables[i].value.b);
                    return buffer;
            }
        }
    }
    return NULL;
}