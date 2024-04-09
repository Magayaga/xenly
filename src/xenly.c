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

#define MAX_TOKEN_SIZE 1000
#define MAX_VARIABLES 1000
#define MAX_OBJECTS 1000
#define MAX_ARRAYS 100

#define MATH_PI 3.14159265358979323846
#define MATH_TAU 6.28318530717958647692
#define MATH_E 2.71828182845904523536
#define MATH_GOLDEN_RATIO 1.61803398874989484820
#define MATH_SILVER_RATIO 2.41421356237309504880
#define MATH_SUPERGOLDEN_RATIO 1.46557123187676802665

#define XENLY_VERSION "0.1.0-nanopreview1"

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

void error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

void execute_comment(const char* comment) {
    printf("// %s\n", comment);
}

double evaluate_factor(const char** expression);
double evaluate_arithmetic_expression(const char** expression);

// Factor
double evaluate_factor(const char** expression) {
    // Evaluate a factor in an arithmetic expression
    double result;

    // Check for unary minus
    int negate = 1;
    if (**expression == '-') {
        negate = -1;
        (*expression)++; // Move past the unary minus
    }

    if (**expression == '(') {
        (*expression)++; // Move past the opening parenthesis
        result = evaluate_arithmetic_expression(expression);
        if (**expression == ')') {
            (*expression)++; // Move past the closing parenthesis
        }
        
        else {
            error("Mismatched parentheses");
        }
    }
    
    else if (isdigit(**expression) || **expression == '.') {
        result = atof(*expression) * negate;
        while (isdigit(**expression) || **expression == '.') {
            (*expression)++; // Move past digits and the decimal point
        }
    }
    
    else {
        // Handle mathematical constants
        if (strncmp(*expression, "pi", 2) == 0) {
            result = MATH_PI * negate;
            *expression += 2; // Move past the constant
        }
        
        else if (strncmp(*expression, "tau", 3) == 0) {
            result = MATH_TAU * negate;
            *expression += 3; // Move past the constant
        }
        
        else if (strncmp(*expression, "e", 1) == 0) {
            result = MATH_E * negate;
            (*expression)++; // Move past the constant
        }
        
        else if (strncmp(*expression, "goldenRatio", 12) == 0) {
            result = MATH_GOLDEN_RATIO * negate;
            *expression += 12; // Move past the constant
        }
        
        else if (strncmp(*expression, "silverRatio", 12) == 0) {
            result = MATH_SILVER_RATIO * negate;
            *expression += 12; // Move past the constant
        }
        
        else if (strncmp(*expression, "supergoldenRatio", 17) == 0) {
            result = MATH_SUPERGOLDEN_RATIO * negate;
            *expression += 17; // Move past the constant
        }
        
        else {
            error("Invalid factor");
        }
    }

    return result;
}

// Term
double evaluate_term(const char** expression) {
    // Evaluate a term (factor) in an arithmetic expression
    double result = evaluate_factor(expression);
    while (**expression && (**expression == '*' || **expression == '/' || **expression == '%')) {
        char op = **expression;
        (*expression)++; // Move past the operator
        double factor = evaluate_factor(expression);
        switch (op) {
            case '*':
                result *= factor;
                break;
            
            case '/':
                if (factor != 0) {
                    result /= factor;
                }
                
                else {
                    error("Division by zero");
                }
                break;
            
            case '%':
                if (factor != 0) {
                    result = fmod(result, factor);
                }
                
                else {
                    error("Modulo by zero");
                }
                break;
        }
    }
    return result;
}

// Evaluate arithmetic expression
double evaluate_arithmetic_expression(const char** expression) {
    // Use a recursive descent parser to evaluate arithmetic expressions
    double result = evaluate_term(expression);
    while (**expression && (**expression == '+' || **expression == '-')) {
        char op = **expression;
        (*expression)++; // Move past the operator
        double term = evaluate_term(expression);
        switch (op) {
            case '+':
                result += term;
                break;
            case '-':
                result -= term;
                break;
        }
    }
    return result;
}

#ifdef _WIN32
// WINDOWS OPERATING SYSTEM
void execute_print(const char* arg) {
    // Check if the argument is a variable reference
    if (arg[0] == '$') {
        // Look for the variable referenced by arg
        char var_name[MAX_TOKEN_SIZE];
        sscanf(arg, "$%s", var_name);
        int found = 0;
        for (int i = 0; i < num_variables; i++) {
            if (strcmp(variables[i].name, var_name) == 0) {
                printf("%s\n", variables[i].value);
                found = 1;
                break;
            }
        }
        
        if (!found) {
            error("Referenced variable not found");
        }
    }
    
    else {
        // Check if the argument is a quoted string
        if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') || 
            (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
            // Print the string without quotes
            printf("%.*s\n", (int)strlen(arg) - 2, arg + 1);
        }
        
        else {
            // Evaluate and print the expression
            double result = evaluate_arithmetic_expression(&arg);
            printf("%lf\n", result);
        }
    }
}

#else
// LINUX OPERATING SYSTEM
void execute_print(const char* arg) {
    // Check if the argument is a variable reference
    if (arg[0] == '$') {
        // Look for the variable referenced by arg
        char var_name[MAX_TOKEN_SIZE];
        sscanf(arg, "$%s", var_name);
        int found = 0;
        for (int i = 0; i < num_variables; i++) {
            if (strcmp(variables[i].name, var_name) == 0) {
                printf("%s\n", variables[i].value);
                found = 1;
                break;
            }
        }
        
        if (!found) {
            error("Referenced variable not found");
        }
    }
    
    else {
        // Check if the argument is a quoted string
        if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') || 
            (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
            // Print the string without quotes
            printf("%.*s\n", (int)strlen(arg) - 2, arg + 1);
        }
        
        else {
            // Evaluate and print the expression
            double result = evaluate_arithmetic_expression(&arg);
            printf("%lf\n", result);
        }
    }
}
#endif

void execute_var(const char* line) {
    char name[MAX_TOKEN_SIZE];
    char value[MAX_TOKEN_SIZE];
    // Use sscanf to parse the input line
    if (sscanf(line, "var %s = %[^\n]", name, value) != 2) {
        error("Invalid 'var' line");
    }

    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            error("Variable already declared");
        }
    }

    if (num_variables < MAX_VARIABLES) {
        strcpy(variables[num_variables].name, name);
        // Check if value is another variable
        if (value[0] == '$') {
            // Look for the variable referenced by value
            char var_name[MAX_TOKEN_SIZE];
            sscanf(value, "$%s", var_name);
            int found = 0;
            for (int i = 0; i < num_variables; i++) {
                if (strcmp(variables[i].name, var_name) == 0) {
                    strcpy(value, variables[i].value);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                error("Referenced variable not found");
            }
        }
        strcpy(variables[num_variables].value, value);
        num_variables++;
    }
    
    else {
        error("Maximum number of variables exceeded");
    }
}

double evaluate_condition(const char* condition) {
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, condition) == 0) {
            return strcmp(variables[i].value, "true") == 0 ? 1 : 0;
        }
    }

    return atof(condition); // Convert string to double
}

// Print version
void print_version() {
    printf("Xenly %s (Pre-alpha release)\n", XENLY_VERSION);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error("Usage: xenly [input file]");
    }

    if (strcmp(argv[1], "--version") == 0) {
        print_version();
        return 0;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        error("Unable to open input file");
    }

    char line[MAX_TOKEN_SIZE]; // Increased size to match the constant MAX_TOKEN_SIZE
    while (fgets(line, sizeof(line), input_file)) {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "print(", 6) == 0 && line[strlen(line) - 1] == ')') {
            char argument[MAX_TOKEN_SIZE]; // Increased size to match the constant MAX_TOKEN_SIZE
            strncpy(argument, line + 6, strlen(line) - 7);
            argument[strlen(line) - 7] = '\0';

            execute_print(argument);
        }

        else if (strncmp(line, "var", 3) == 0) {
            execute_var(line);
        }

        else if (strncmp(line, "//", 2) == 0) {
            continue;
        }

        else if (multiline_comment == 0) {
            // Check if the line starts with "/*"
            if (strncmp(line, "/*", 2) == 0) {
                multiline_comment = 0;
                // If the line contains both "/*" and "*/" on the same line
                if (strstr(line, "*/") != NULL) {
                    multiline_comment = 0;
                }
                
                continue;
            }
        }

        else if (multiline_comment == 1) {
            // Check if the line contains "*/"
            if (strstr(line, "*/") != NULL) {
                multiline_comment = 0;
                continue;
            }
            
            else {
                continue;
            }
        }

        else {
            error("Invalid statement");
        }
    }

    fclose(input_file);

    return 0;
}
