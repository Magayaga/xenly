/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "xenly.h"
#include "goxenly.h"

#define MATH_PI 3.14159265358979323846
#define MATH_TAU 6.28318530717958647692
#define MATH_E 2.71828182845904523536
#define MATH_GOLDEN_RATIO 1.61803398874989484820
#define MATH_SILVER_RATIO 2.41421356237309504880
#define MATH_SUPERGOLDEN_RATIO 1.46557123187676802665

#define XENLY_VERSION "0.1.0-preview3"

Array arrays[MAX_ARRAYS];
int num_arrays = 0;

bool evaluately_condition(const char* condition) {
    // Implement a more comprehensive logic for evaluating conditions
    // This is a simplified example
    return atoi(condition) != 0;
}

void error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

void execute_comment(const char* comment) {
    printf("// %s\n", comment);
}

double evaluate_condition(const char* condition);
double execute_sqrt(const char* arg);
void execute_for(FILE* input_file, const char* loop_variable, int start_value, int end_value, const char* loop_body);
double evaluate_arithmetic_expression(const char* expression);

void execute_print(const char* arg) {
    if (arg[0] == '"' && arg[strlen(arg) - 1] == '"') {
        printf("%.*s\n", (int)strlen(arg) - 2, arg + 1);
    }
    
    else if (strpbrk(arg, "+-*/%^")) {
        double result = evaluate_arithmetic_expression(arg);
        printf("%lf\n", result);
    }
    
    else if (strcmp(arg, "pi") == 0) {
        printf("%lf\n", MATH_PI);
    }
    
    else if (strcmp(arg, "tau") == 0) {
        printf("%lf\n", MATH_TAU);
    }
    
    else if (strcmp(arg, "e") == 0) {
        printf("%lf\n", MATH_E);
    }
    
    else if (strcmp(arg, "goldenRatio") == 0) {
        printf("%lf\n", MATH_GOLDEN_RATIO);
    }
    
    else if (strcmp(arg, "silverRatio") == 0) {
        printf("%lf\n", MATH_SILVER_RATIO);
    }
    
    else if (strcmp(arg, "supergoldenRatio") == 0) {
        printf("%lf\n", MATH_SUPERGOLDEN_RATIO);
    }
    
    else if (strncmp(arg, "sqrt(", 5) == 0 && arg[strlen(arg) - 1] == ')') {
        double result = evaluate_condition(arg + 5);
        if (result >= 0) {
            printf("%lf\n", sqrt(result));
        }
        
        else {
            error("Square root of a negative number is not supported");
        }
    }
    
    else if (strncmp(arg, "cbrt(", 5) == 0 && arg[strlen(arg) - 1] == ')') {
        double result = evaluate_condition(arg + 5);
        printf("%lf\n", cbrt(result));
    }
    
    else if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1]))) {
        // Check if it's a numeric constant or expression
        double result = evaluate_condition(arg);
        printf("%lf\n", result);
    }
    
    else {
        int is_variable = 0;
        for (int i = 0; i < num_variables; i++) {
            if (strcmp(variables[i].name, arg) == 0) {
                printf("%s\n", variables[i].value);
                is_variable = 1;
                break;
            }
        }
        if (!is_variable) {
            int result = evaluate_condition(arg);
            printf("%d\n", result);
        }
    }
}

void execute_var(const char* name, const char* value) {
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            error("Variable already declared");
        }
    }

    if (num_variables < MAX_VARIABLES) {
        strcpy(variables[num_variables].name, name);

        // Check if the value is an arithmetic expression or a string
        if (value[0] == '"' && value[strlen(value) - 1] == '"') {
            strcpy(variables[num_variables].value, value);
        }
        
        else {
            double result = evaluate_condition(value);
            snprintf(variables[num_variables].value, sizeof(variables[num_variables].value), "%lf", result);
        }

        num_variables++;
    }
    
    else {
        error("Maximum number of variables exceeded");
    }
}

void execute_for(FILE* input_file, const char* loop_variable, int start_value, int end_value, const char* loop_body) {
    for (int i = start_value; i <= end_value; i++) {
        // Set loop variable
        char loop_variable_value[MAX_TOKEN_SIZE];
        snprintf(loop_variable_value, sizeof(loop_variable_value), "%d", i);
        execute_var(loop_variable, loop_variable_value);

        // Execute loop body
        char line[MAX_TOKEN_SIZE + 10]; // Assuming the loop body can be up to MAX_TOKEN_SIZE characters
        snprintf(line, sizeof(line), "%s", loop_body);
        while (fgets(line, sizeof(line), input_file)) {
            line[strcspn(line, "\n")] = '\0';

            // End of the loop body
            if (strncmp(line, "}", 1) == 0) {
                break;
            }

            // Execute the line within the loop body
            // Add relevant logic here
        }
    }
}

void execute_int(const char* name, const char* value) {
    for (int i = 0; i < num_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            error("Variable already declared");
        }
    }

    if (num_variables < MAX_VARIABLES) {
        strcpy(variables[num_variables].name, name);
        strcpy(variables[num_variables].value, value);
        num_variables++;
    }

    else {
        error("Maximum number of variables exceeded");
    }
}

double execute_sqrt(const char* arg) {
    double result = evaluate_condition(arg);
    if (result >= 0) {
        return sqrt(result);
    }

    else {
        error("Square root of a negative number is not supported");
    }
    return 0;
}

double execute_cbrt(const char* arg) {
    double result = evaluate_condition(arg);
    return cbrt(result);
}

double execute_pow(const char* arg) {
    double base, exponent;
    if (sscanf(arg, "%lf, %lf", &base, &exponent) == 2) {
        return pow(base, exponent);
    }
    else {
        error("Invalid arguments for 'pow'");
    }
    return 0.0;
}

double execute_sin(const char* arg) {
    double result = evaluate_condition(arg);
    return sin(result);
}

double execute_cos(const char* arg) {
    double result = evaluate_condition(arg);
    return cos(result);
}

double execute_tan(const char* arg) {
    double result = evaluate_condition(arg);
    return tan(result);
}

double execute_gamma(const char* arg) {
    double result = evaluate_condition(arg);
    return tgamma(result);
}

double execute_max(const char* arg1, const char* arg2) {
    double x = evaluate_condition(arg1);
    double y = evaluate_condition(arg2);
    return (x > y) ? x : y;
}

double execute_min(const char* arg1, const char* arg2) {
    double x = evaluate_condition(arg1);
    double y = evaluate_condition(arg2);
    return (x < y) ? x : y;
}

double execute_abs(const char* arg) {
    double x = atoi(arg);
    return abs(x);
}

double ffrt(double x) {
    if (x >= 0) {
        return pow(x, 1.0 / 5);
    } else {
        error("Fifth root of a negative number is not supported");
        return 0.0; // You can choose to return a default value here
    }
}

int factorial(int n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int execute_factorial(const char* arg) {
    int result = evaluate_condition(arg);
    if (result < 0) {
        error("Factorial of a negative number is not supported");
    }
    return factorial(result);
}

int fibonacci(int n) {
    if (n <= 0) {
        return 0;
    } else if (n == 1) {
        return 1;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int execute_fibonacci(const char* arg) {
    int result = evaluate_condition(arg);
    if (result < 0) {
        error("Fibonacci of a negative number is not supported");
    }
    return fibonacci(result);
}

int convert_binary_to_decimal(const char* binary) {
    int length = strlen(binary);
    int decimal = 0;

    for (int i = length - 1; i >= 0; i--) {
        if (binary[i] == '1') {
            decimal += pow(2, length - 1 - i);
        }
    }

    return decimal;
}

char* convert_decimal_to_binary(int decimal) {
    char binary[32]; // Assuming 32-bit integers
    int index = 0;

    while (decimal > 0) {
        binary[index++] = (decimal % 2) + '0';
        decimal /= 2;
    }
    binary[index] = '\0';

    // Reverse the binary string
    int left = 0;
    int right = index - 1;
    while (left < right) {
        char temp = binary[left];
        binary[left] = binary[right];
        binary[right] = temp;
        left++;
        right--;
    }

    return strdup(binary);
}

double evaluate_factor(const char** expression) {
    // Evaluate a factor in an arithmetic expression
    double result;

    if (**expression == '(') {
        (*expression)++; // Move past the opening parenthesis
        result = evaluate_arithmetic_expression(*expression);
        if (**expression == ')') {
            (*expression)++; // Move past the closing parenthesis
        }
        
        else {
            error("Mismatched parentheses");
        }
    }
    
    else {
        result = atof(*expression);
        while (isdigit(**expression) || **expression == '.') {
            (*expression)++; // Move past digits and the decimal point
        }
    }

    return result;
}

double evaluate_term(const char** expression) {
    // Evaluate a term (factor) in an arithmetic expression
    double result = evaluate_factor(expression);
    while (**expression) {
        char operator = **expression;
        if (operator == '*' || operator == '/' || operator == '%') {
            (*expression)++; // Move past the operator
            double factor = evaluate_factor(expression);
            if (operator == '*') {
                result *= factor;
            }
            
            else if (operator == '/') {
                if (factor != 0) {
                    result /= factor;
                }
                
                else {
                    error("Division by zero");
                }
            }
            
            else if (operator == '%') {
                if (factor != 0) {
                    result = fmod(result, factor);
                }
                
                else {
                    error("Modulo by zero");
                }
            }
        }
        
        else {
            break; // Not an operator, exit the loop
        }
    }
    return result;
}

double evaluate_arithmetic_expression(const char* expression) {
    // Use a recursive descent parser to evaluate arithmetic expressions
    double result = evaluate_term(&expression);
    while (*expression) {
        char operator = *expression;
        if (operator == '+' || operator == '-') {
            expression++; // Move past the operator
            double term = evaluate_term(&expression);
            if (operator == '+') {
                result += term;
            }
            
            else {
                result -= term;
            }
        }
        
        else {
            break; // Not an operator, exit the loop
        }
    }
    return result;
}

double evaluate_condition(const char* condition) {
    if (strpbrk(condition, "+-*/%^")) {
        // Use a simple recursive descent parser to evaluate arithmetic expressions
        return evaluate_arithmetic_expression(condition);
    }

    if (strncmp(condition, "pow(", 4) == 0 && condition[strlen(condition) - 1] == ')') {
        // Extract the base and exponent values from the argument
        char arguments[1000];
        strncpy(arguments, condition + 4, strlen(condition) - 5);
        arguments[strlen(condition) - 5] = '\0';

        char* base_arg = strtok(arguments, ",");
        char* exponent_arg = strtok(NULL, ",");

        double base = evaluate_condition(base_arg);
        double exponent = evaluate_condition(exponent_arg);

        return pow(base, exponent);
    }

    if (strncmp(condition, "sin(", 4) == 0 && condition[strlen(condition) - 1] == ')') {
        double inner_result = evaluate_condition(condition + 4);
        return sin(inner_result);
    }

    if (strncmp(condition, "cos(", 4) == 0 && condition[strlen(condition) - 1] == ')') {
        double inner_result = evaluate_condition(condition + 4);
        return cos(inner_result);
    }

    if (strncmp(condition, "tan(", 4) == 0 && condition[strlen(condition) - 1] == ')') {
        double inner_result = evaluate_condition(condition + 4);
        return tan(inner_result);
    }

    if (strncmp(condition, "gamma(", 6) == 0 && condition[strlen(condition) - 1] == ')') {
        double inner_result = evaluate_condition(condition + 6);
        return tgamma(inner_result);
    }

    if (strncmp(condition, "fibonacci(", 10) == 0 && condition[strlen(condition) - 1] == ')') {
        int inner_result = evaluate_condition(condition + 10);
        if (inner_result < 0) {
            error("Fibonacci of a negative number is not supported");
        }
        return fibonacci(inner_result);
    }

    if (strncmp(condition, "factorial(", 10) == 0 && condition[strlen(condition) - 1] == ')') {
        int inner_result = evaluate_condition(condition + 10);
        if (inner_result < 0) {
            error("Factorial of a negative number is not supported");
        }
        return factorial(inner_result);
    }

    if (condition[0] == '(' && condition[strlen(condition) - 1] == ')') {
        char expression[1000];
        strncpy(expression, condition + 1, strlen(condition) - 2);
        expression[strlen(condition) - 2] = '\0';
        return evaluate_condition(expression);
    }

    int left_value, right_value;
    char operator;

    if (sscanf(condition, "%d %c %d", &left_value, &operator, &right_value) == 3) {
        switch (operator) {
            case '<':
                return left_value < right_value;
            case '>':
                return left_value > right_value;
            case '=':
                return left_value == right_value;
            case '+':
                return left_value + right_value;
            case '-':
                return left_value - right_value;
            case '*':
                return left_value * right_value;
            case '/':
                if (right_value != 0) {
                    return left_value / right_value;
                } else {
                    error("Division by zero");
                }
            default:
                error("Invalid operator in condition");
        }
    } else {
        for (int i = 0; i < num_variables; i++) {
            if (strcmp(variables[i].name, condition) == 0) {
                return strcmp(variables[i].value, "true") == 0 ? 1 : 0;
            }
        }
    }

    if (strncmp(condition, "sqrt(", 5) == 0 && condition[strlen(condition) - 1] == ')') {
        double inner_result = evaluate_condition(condition + 5);
        if (inner_result >= 0) {
            return sqrt(inner_result);
        }

        else {
            error("Square root of a negative number is not supported");
        }
    }

    if (strncmp(condition, "cbrt(", 5) == 0 && condition[strlen(condition) - 1] == ')') {
        double inner_result = evaluate_condition(condition + 5);
        return cbrt(inner_result);
    }

    return atoi(condition); // Convert string to integer
}

void execute_object(const char* name, const char* properties) {
    // Create a new object with given properties
    if (num_data < MAX_OBJECTS) {
        strcpy(data_storage[num_data].name, name);
        data_storage[num_data].type = 1; // 1 for object
        strcpy(data_storage[num_data].value, properties);
        num_data++;
    } else {
        error("Maximum number of objects exceeded");
    }
}

void execute_array(const char* name, int size) {
    if (num_arrays < MAX_ARRAYS) {
        arrays[num_arrays].type = 2; // 2 for array
        arrays[num_arrays].size = size;
        arrays[num_arrays].elements = (double*)malloc(size * sizeof(double));

        strcpy(data_storage[num_data].name, name);
        data_storage[num_data].type = 2; // 2 for array
        num_data++;

        num_arrays++;
    }
    
    else {
        error("Maximum number of arrays exceeded");
    }
}

void execute_set(const char* array_name, int index, double value) {
    for (int i = 0; i < num_arrays; i++) {
        if (strcmp(arrays[i].name, array_name) == 0) {
            if (index >= 0 && index < arrays[i].size) {
                arrays[i].elements[index] = value;
            } else {
                error("Array index out of bounds");
            }
            return;
        }
    }
    error("Array not found");
}

double execute_get(const char* array_name, int index) {
    for (int i = 0; i < num_arrays; i++) {
        if (strcmp(arrays[i].name, array_name) == 0) {
            if (index >= 0 && index < arrays[i].size) {
                return arrays[i].elements[index];
            } else {
                error("Array index out of bounds");
            }
        }
    }
    error("Array not found");
    return 0; // You can choose to return a default value here
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error("Usage: xenly [input file]");
    }

    if (argc == 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
        print_version();
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help();
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--operatingsystem" ) == 0 || strcmp(argv[1], "-os") == 0)) {
        // Print the compiler's operating system
        #if defined(_WIN32)
            printf("Windows\n");
        #elif defined(__linux__)
            printf("Linux\n");
        #elif defined(__unix__) || defined(__unix)
            printf("Unix\n");
        #elif defined(__APPLE__) || defined(__MACH__)
            printf("macOS\n");
        #else
            printf("Unknown/Segmentation fault\n");
        #endif
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--dumpmachine" ) == 0 || strcmp(argv[1], "-dm") == 0)) {
        // Print the compiler's target processor
        printf("%s\n", getenv("PROCESSOR_ARCHITECTURE"));
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--dumpversion") == 0 || strcmp(argv[1], "-dv") == 0)) {
        print_dumpversion();
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "--path") == 0 || strcmp(argv[1], "-p") == 0)) {
        // Print the path to the Xenly compiler executable
        // argv[0] contains the path to the executable
        printf("%s\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--author") == 0) {
        print_author();
        return 0;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        error("Unable to open input file");
    }

    char line[1000];
    bool in_condition_block = false;
    bool condition_met = false;
    while (fgets(line, sizeof(line), input_file)) {
        line[strcspn(line, "\n")] = '\0';
        
        if (strncmp(line, "if", 2) == 0 || strncmp(line, "elif", 4) == 0 || strncmp(line, "else", 4) == 0) {
            if (in_condition_block) {
            // Handle nested conditions
            // ...
            }
            
            char condition[MAX_TOKEN_SIZE];
            strncpy(condition, line + (strncmp(line, "else", 4) == 0 ? 4 : 2), strlen(line) - (strncmp(line, "else", 4) == 0 ? 4 : 2));
            condition[strlen(line) - (strncmp(line, "else", 4) == 0 ? 4 : 2)] = '\0';
            
            in_condition_block = true;
            if ((strncmp(line, "if", 2) == 0 && evaluately_condition(condition)) ||
            (strncmp(line, "elif", 4) == 0 && !condition_met && evaluately_condition(condition))) {
                condition_met = true;
            }
            
            else {
                condition_met = false;
                continue;
            }
        }
        
        else if (strncmp(line, "{", 1) == 0) {
            // Start of the code block
            if (condition_met) {
                while (fgets(line, sizeof(line), input_file)) {
                    line[strcspn(line, "\n")] = '\0';
                    if (strncmp(line, "}", 1) == 0) {
                        // End of the code block
                        in_condition_block = false;
                        break;
                    }
                    
                    // Execute the line within the code block
                    // Add relevant logic here
                }
             }
        }
        
        else if (strncmp(line, "}", 1) == 0) {
            // End of the code block
            in_condition_block = false;
        }

        else if (strncmp(line, "for", 3) == 0) {
            char loop_variable[MAX_TOKEN_SIZE];
            int start_value, end_value;
            char loop_body[MAX_TOKEN_SIZE];

            // Extract loop variable, start value, and end value from the line
            if (sscanf(line + 3, "%s = %d to %d {", loop_variable, &start_value, &end_value) == 3) {
                // Extract the loop body until the corresponding '}'
                char temp_line[MAX_TOKEN_SIZE + 10]; // Assuming the loop body can be up to MAX_TOKEN_SIZE characters
                strcpy(loop_body, "");
                while (fgets(temp_line, sizeof(temp_line), input_file)) {
                    temp_line[strcspn(temp_line, "\n")] = '\0';
                    if (strncmp(temp_line, "}", 1) == 0) {
                        break;
                    }
                    strcat(loop_body, temp_line);
                }

                // Execute the 'for' loop
                execute_for(input_file, loop_variable, start_value, end_value, loop_body);
            } else {
                error("Invalid 'for' loop");
            }
        }

        else if (strncmp(line, "print(", 6) == 0 && line[strlen(line) - 1] == ')') {
            char argument[1000];
            strncpy(argument, line + 6, strlen(line) - 7);
            argument[strlen(line) - 7] = '\0';

            if (strncmp(argument, "pow(", 4) == 0 && argument[strlen(argument) - 1] == ')') {
                double result = evaluate_condition(argument);
                printf("%lf\n", result);
            }

            else if (strncmp(argument, "factorial(", 10) == 0 && argument[strlen(argument) - 1] == ')') {
                double result = evaluate_condition(argument);
                printf("%lf\n", result);
            }

            else if (strncmp(argument, "get(", 4) == 0 && argument[strlen(argument) - 1] == ')') {
                char array_name[MAX_TOKEN_SIZE];
                int index;
                
                // Extract array name and index from the line
                if (sscanf(argument + 4, "%[^,],%d)", array_name, &index) == 2) {
                    double result = execute_get(array_name, index);
                    printf("%lf\n", result);
                }
                
                else {
                    error("Invalid arguments for 'get'");
                }
            }

            else {
                execute_print(argument);
            }
        }

        else if (strncmp(line, "var", 3) == 0) {
            char name[MAX_TOKEN_SIZE];
            char value[MAX_TOKEN_SIZE];
            if (sscanf(line, "var %s = %[^\n]", name, value) != 2) {
                error("Invalid 'var' line");
            }

            execute_var(name, value);
        }

        else if (strncmp(line, "int", 3) == 0) {
            char name[MAX_TOKEN_SIZE];
            char value[MAX_TOKEN_SIZE];
            if (sscanf(line, "int %s = %[^\n]", name, value) != 2) {
                error("Invalid 'int' line");
            }

            execute_int(name, value);
        }

        else if (strncmp(line, "binary(", 6) == 0 && line[strlen(line) - 1] == ')') {
            char binary_argument[1000];
            strncpy(binary_argument, line + 6, strlen(line) - 7);
            binary_argument[strlen(line) - 7] = '\0';

            int decimal_result = convert_binary_to_decimal(binary_argument);
            printf("%d\n", decimal_result);
        }

        else if (strncmp(line, "decimal(", 8) == 0 && line[strlen(line) - 1] == ')') {
            int decimal_argument;
            if (sscanf(line + 8, "%d", &decimal_argument) == 1) {
                char* binary_result = convert_decimal_to_binary(decimal_argument);
                printf("%s\n", binary_result);
                free(binary_result); // Remember to free allocated memory
            } else {
                error("Invalid argument for convert_decimal");
            }
        }

        else if (strncmp(line, "sqrt", 4) == 0 && line[4] == '(' && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line + 5);
            if (result >= 0) {
                printf("%lf\n", sqrt(result));
            }

            else {
                error("Square root of a negative number is not supported");
            }
        }

        else if (strncmp(line, "cbrt", 4) == 0 && line[4] == '(' && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line + 5);
            if (result >= 0) {
                printf("%lf\n", cbrt(result));
            }
            
            else {
                error("Cube root of a negative number is not supported");
            }
        }

        else if (strncmp(line, "ffrt", 4) == 0 && line[4] == '(' && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line + 5);
            if (result >= 0) {
                printf("%lf\n", ffrt(result));
            }

            else {
                error("Fifth root of a negative number is not supported");
            }
        }

        else if (strncmp(line, "max(", 4) == 0 && line[strlen(line) - 1] == ')') {
            char arg1[MAX_TOKEN_SIZE];
            char arg2[MAX_TOKEN_SIZE];

            // Extract arguments from the line
            if (sscanf(line + 4, "%[^,],%[^)]", arg1, arg2) == 2) {
                double result = execute_max(arg1, arg2);
                printf("%lf\n", result);
            }

            else {
                error("Invalid arguments for 'max'");
            }
        }

        else if (strncmp(line, "min(", 4) == 0 && line[strlen(line) - 1] == ')') {
            char arg1[MAX_TOKEN_SIZE];
            char arg2[MAX_TOKEN_SIZE];

            // Extract arguments from the line
            if (sscanf(line + 4, "%[^,],%[^)]", arg1, arg2) == 2) {
                double result = execute_min(arg1, arg2);
                printf("%lf\n", result);
            }

            else {
                error("Invalid arguments for 'min'");
            }
        }

        else if (strncmp(line, "abs(", 4) == 0 && line[strlen(line) - 1] == ')') {
            char arg[MAX_TOKEN_SIZE];

            // Extract argument from the line
            if (sscanf(line + 4, "%[^)]", arg) == 1) {
                int result = execute_abs(arg);
                printf("%d\n", result);
            } else {
                error("Invalid argument for 'abs'");
            }
        }
        
        else if (strncmp(line, "gamma(", 6) == 0 && line[strlen(line) - 1] == ')') {
            char argument[MAX_TOKEN_SIZE];
            strncpy(argument, line + 6, strlen(line) - 7);
            argument[strlen(line) - 7] = '\0';
            
            double result = execute_gamma(argument);
            printf("%lf\n", result);
        }

        else if (strncmp(line, "fibonacci(", 10) == 0 && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line);
            printf("%lf\n", result);
        }

        else if (strncmp(line, "array", 5) == 0) {
            char name[MAX_TOKEN_SIZE];
            int size;

            // Extract array name and size from the line
            if (sscanf(line + 5, "%s %d", name, &size) == 2) {
                execute_array(name, size);
            }
            
            else {
                error("Invalid 'array' line");
            }
        }

        else if (strncmp(line, "set(", 4) == 0 && line[strlen(line) - 1] == ')') {
            char array_name[MAX_TOKEN_SIZE];
            int index;
            double value;

            // Extract array name, index, and value from the line
            if (sscanf(line + 4, "%[^,],%d,%lf)", array_name, &index, &value) == 3) {
                execute_set(array_name, index, value);
            }
            
            else {
                error("Invalid arguments for 'set'");
            }
        }

        else if (strncmp(line, "sin(", 4) == 0 && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line);
            printf("%lf\n", result);
        }

        else if (strncmp(line, "cos(", 4) == 0 && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line);
            printf("%lf\n", result);
        }

        else if (strncmp(line, "tan(", 4) == 0 && line[strlen(line) - 1] == ')') {
            double result = evaluate_condition(line);
            printf("%lf\n", result);
        }

        else if (strncmp(line, "//", 2) == 0) {
            continue;
        }

        else {
            error("Invalid sttatement");
        }
    }

    fclose(input_file);

    return 0;
}