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
#include <math.h>
#include "xenly.h"

// Array to store all declared variables
Variable variables[MAX_VARIABLES];
int variable_count = 0;

// Function to remove leading and trailing whitespace from a string
char* trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// Function to find a variable by name
Variable* find_variable(const char* name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// Function to add a new variable to the variables array
void add_variable(const char* name, double value, const char* string_value, int is_string) {
    if (variable_count < MAX_VARIABLES) {
        strcpy(variables[variable_count].name, name);
        variables[variable_count].value = value;
        if (is_string) {
            strcpy(variables[variable_count].string_value, string_value);
        }
        variables[variable_count].is_string = is_string;
        variable_count++;
    }
    
    else {
        printf("Error: Maximum number of variables reached\n");
    }
}

// Function to check if a string represents a numeric value
int is_numeric(const char* str) {
    char* endptr;
    strtod(str, &endptr);
    return *endptr == '\0';
}

// Function to evaluate a simple arithmetic expression
double evaluate_expression(const char* expr) {
    char op = '+';
    double result = 0;
    char* token = strtok((char*)expr, " ");
    
    while (token!= NULL) {
        if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 || 
            strcmp(token, "*") == 0 || strcmp(token, "/") == 0) {
            op = token[0];
        }
        
        else {
            double value;
            Variable* var = find_variable(token);
            if (var!= NULL) {
                if (var->is_string) {
                    printf("Error: Cannot perform arithmetic on string '%s'\n", var->string_value);
                    return 0;
                }
                value = var->value;
            }
            
            else {
                value = atof(token);
            }
            
            switch(op) {
                case '+': result += value; break;
                case '-': result -= value; break;
                case '*': result *= value; break;
                case '/': result /= value; break;
            }
        }
        token = strtok(NULL, " ");
    }
    
    return result;
}

// Function to execute a print statement
void execute_print(char* args) {
    char* token = strtok(args, ",");
    while (token!= NULL) {
        char* trimmed_token = trim(token);
        if (trimmed_token[0] == '"' && trimmed_token[strlen(trimmed_token)-1] == '"') {
            // Remove the quotes and print the string content
            printf("%.*s", (int)strlen(trimmed_token) - 2, trimmed_token + 1);
        }
        
        else {
            Variable* var = find_variable(trimmed_token);
            if (var!= NULL) {
                if (var->is_string) {
                    printf("%s", var->string_value);
                }
                
                else {
                    printf("%g", var->value);
                }
            }
            
            else if (is_numeric(trimmed_token)) {
                printf("%s", trimmed_token);
            }
            
            else {
                double result = evaluate_expression(trimmed_token);
                printf("%g", result);
            }
        }
        token = strtok(NULL, ",");
        if (token != NULL) {
            printf(" ");  // Add space between printed items
        }
    }
    printf("\n");
}

// Function to execute an input statement
char* execute_input(char* prompt) {
    static char input_buffer[MAX_INPUT_LENGTH];
    if (prompt!= NULL && strlen(prompt) > 0) {
        if (prompt[0] == '"' && prompt[strlen(prompt)-1] == '"') {
            printf("%.*s", (int)strlen(prompt) - 2, prompt + 1);
        }
        
        else {
            printf("%s", prompt);
        }
    }
    fgets(input_buffer, sizeof(input_buffer), stdin);
    input_buffer[strcspn(input_buffer, "\n")] = 0;  // Remove trailing newline
    return input_buffer;
}

// Function to interpret a single line of input
void interpret_line(char* line) {
    char* trimmed_line = trim(line);
    if (strlen(trimmed_line) == 0) {
        return;
    }
    
    if (strncmp(trimmed_line, "nota(", 5) == 0 && trimmed_line[strlen(trimmed_line)-1] == ')') {
        char* args = trimmed_line + 5;
        args[strlen(args)-1] = '\0';  // Remove closing parenthesis
        execute_print(args);
    }
    
    else if (strncmp(trimmed_line, "var ", 4) == 0) {
        char* var_decl = trimmed_line + 4;
        char* eq_sign = strchr(var_decl, '=');
        if (eq_sign!= NULL) {
            *eq_sign = '\0';
            char* var_name = trim(var_decl);
            char* var_value = trim(eq_sign + 1);
            
            if (strncmp(var_value, "input(", 6) == 0 && var_value[strlen(var_value)-1] == ')') {
                char* input_args = var_value + 6;
                input_args[strlen(input_args)-1] = '\0';  // Remove closing parenthesis
                char* input_result = execute_input(input_args);
                if (is_numeric(input_result)) {
                    add_variable(var_name, atof(input_result), "", 0);
                }
                
                else {
                    add_variable(var_name, 0, input_result, 1);
                }
            }
            
            else if (var_value[0] == '"' && var_value[strlen(var_value)-1] == '"') {
                // Remove the quotes and store the string content
                var_value[strlen(var_value)-1] = '\0';
                add_variable(var_name, 0, var_value + 1, 1);
            }
            
            else {
                double value = evaluate_expression(var_value);
                add_variable(var_name, value, "", 0);
            }
        }
        
        else {
            printf("Syntax error: Invalid variable declaration\n");
        }
    }
    
    else {
        printf("Syntax error: Invalid statement '%s'\n", trimmed_line);
    }
}
