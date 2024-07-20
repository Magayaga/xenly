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
#include <ctype.h>
#include <stdbool.h>
#include "xenly.h"

// Array to store all declared variables
Variable variables[MAX_VARIABLES];
int variable_count = 0;
bool in_multi_line_comment = false;

void process_line(char* line) {
    char* processed_line = remove_comments(line);
    if (processed_line && *processed_line) {
        interpret_line(processed_line);
    }
}

char* remove_comments(char* line) {
    static char cleaned_line[MAX_LINE_LENGTH];
    char* write = cleaned_line;
    char* read = line;
    
    while (*read) {
        if (in_multi_line_comment) {
            if (read[0] == '*' && read[1] == '/') {
                in_multi_line_comment = false;
                read += 2;
            }
            
            else {
                read++;
            }
        }
        
        else if (read[0] == '/' && read[1] == '/') {
            break;  // Ignore rest of the line
        }
        
        else if (read[0] == '/' && read[1] == '*') {
            in_multi_line_comment = true;
            read += 2;
        }
        
        else {
            *write++ = *read++;
        }
    }
    *write = '\0';
    
    return cleaned_line[0] ? cleaned_line : NULL;
}

void interpret_line(char* line) {
    char* trimmed_line = trim(line);
    
    if (strncmp(trimmed_line, "nota(", 5) == 0) {
        char* args = trimmed_line + 5;
        char* end = strrchr(args, ')');
        if (end) *end = '\0';
        print_function(args);
    }
    
    else if (strncmp(trimmed_line, "var ", 4) == 0) {
        var_declaration(trimmed_line + 4);
    }
    
    else if (strncmp(trimmed_line, "bool ", 5) == 0) {
        bool_declaration(trimmed_line + 5);
    }
}

void print_function(char* args) {
    char* trimmed_args = trim(args);
    
    if (trimmed_args[0] == '"' && trimmed_args[strlen(trimmed_args)-1] == '"') {
        // String literal
        trimmed_args[strlen(trimmed_args)-1] = '\0';
        printf("%s\n", trimmed_args + 1);
    }
    
    else {
        // Variable or expression
        char* value = get_variable_value(trimmed_args);
        if (value != NULL) {
            printf("%s\n", value);
        }
        
        else {
            // Assume it's a numeric expression
            double result = evaluate_expression(trimmed_args);
            printf("%g\n", result);
        }
    }
}

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
    }
    
    else if (strchr(value, '.') != NULL) {
        variables[variable_count].type = 'f';
        variables[variable_count].value.f = atof(value);
    }
    
    else {
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

double evaluate_expression(const char* expr) {
    char expr_copy[MAX_LINE_LENGTH];
    strcpy(expr_copy, expr);
    
    char* token = strtok(expr_copy, " ");
    double result = atof(token);
    
    while ((token = strtok(NULL, " ")) != NULL) {
        char op = token[0];
        token = strtok(NULL, " ");
        double num = atof(token);
        
        switch (op) {
            case '+': result += num; break;
            case '-': result -= num; break;
            case '*': result *= num; break;
            case '/': result /= num; break;
        }
    }
    
    return result;
}

char* trim(char* str) {
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

bool parse_bool(const char* value) {
    if (strcmp(value, "true") == 0) {
        return true;
    }
    
    else if (strcmp(value, "false") == 0) {
        return false;
    }
    
    else {
        printf("Error: Invalid boolean value '%s'. Defaulting to false.\n", value);
        return false;
    }
}
