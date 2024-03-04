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

#ifdef _WIN32
void execute_print(const char* arg) {
    int is_variable = 0;
    char clean_arg[MAX_TOKEN_SIZE]; // New variable to store the cleaned argument

    // Remove quotes if present
    if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') || 
        (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
        strncpy(clean_arg, arg + 1, strlen(arg) - 2);
        clean_arg[strlen(arg) - 2] = '\0';
        arg = clean_arg;
    }

    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, arg) == 0) {
            printf("%s\n", variables[i].value);
            is_variable = 1;
            break;
        }
    }
    if (!is_variable) {
        // If the argument is not a variable, print it directly
        printf("%s\n", arg);
    }
}
#else
void execute_print(const char* arg) {
    int is_variable = 0;
    char clean_arg[MAX_TOKEN_SIZE]; // New variable to store the cleaned argument

    // Remove quotes if present
    if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') || 
        (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
        strncpy(clean_arg, arg + 1, strlen(arg) - 2);
        clean_arg[strlen(arg) - 2] = '\0';
        arg = clean_arg;
    }

    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, arg) == 0) {
            printf("%s\n", variables[i].value);
            is_variable = 1;
            break;
        }
    }
    if (!is_variable) {
        // If the argument is not a variable, print it directly
        printf("%s\n", arg);
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
