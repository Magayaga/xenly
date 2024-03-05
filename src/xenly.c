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

void error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

void execute_comment(const char* comment) {
    printf("// %s\n", comment);
}

double evaluate_expression(const char* expr) {
    // Initialize variables
    char operators[MAX_TOKEN_SIZE];
    double operands[MAX_TOKEN_SIZE];
    int num_operators = 0;
    int num_operands = 0;

    // Tokenize the expression
    char* token = strtok((char*)expr, " ");
    while (token != NULL) {
        // Check if token is an operator
        if (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/') {
            operators[num_operators++] = token[0];
        }
            
        // Otherwise, token is an operand
        else {
            operands[num_operands++] = atof(token);
        }
        // Get the next token
        token = strtok(NULL, " ");
    }

    // Perform calculations
    double result = operands[0];
    for (int i = 0; i < num_operators; i++) {
        switch (operators[i]) {
            case '+':
                result += operands[i + 1];
                break;
            
            case '-':
                result -= operands[i + 1];
                break;
            
            case '*':
                result *= operands[i + 1];
                break;
            
            case '/':
                if (operands[i + 1] == 0) {
                    error("Division by zero");
                }
                result /= operands[i + 1];
                break;
            
            default:
                error("Invalid operator");
        }
    }

    return result;
}

#ifdef _WIN32
// WINDOWS OPERATING SYSTEM
void execute_print(const char* arg) {
    // Check if the argument is a quoted string
    if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') || 
        (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
        // Print the string without quotes
        printf("%.*s\n", (int)strlen(arg) - 2, arg + 1);
    }
    
    else {
        // Evaluate and print the expression
        double result = evaluate_expression(arg);
        printf("%lf\n", result);
    }
}

#else
// LINUX OPERATING SYSTEM
void execute_print(const char* arg) {
    // Check if the argument is a quoted string
    if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') || 
        (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
        // Print the string without quotes
        printf("%.*s\n", (int)strlen(arg) - 2, arg + 1);
    }
    
    else {
        // Evaluate and print the expression
        double result = evaluate_expression(arg);
        printf("%lf\n", result);
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
        strcpy(variables[num_variables].value, value);
        num_variables++;
    } else {
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

        else {
            error("Invalid statement");
        }
    }

    fclose(input_file);

    return 0;
}
