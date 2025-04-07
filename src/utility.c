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
#include "utility.h"
#include "error.h"
#include "data_structures.h"
#include "math_functions.h"
#include "binary_math_functions.h"
#include "graphics_functions.h"

void execute_print(const char* arg) {
    // Check if the argument is a variable reference
    if (arg[0] == '$') {
        // Look for the variable referenced by arg
        char var_name[MAX_TOKEN_SIZE];
        sscanf(arg, "$%s", var_name);
        int found = 0;
        for (int i = 0; i < result_variables; i++) {
            if (strcmp(variables[i].name, var_name) == 0) {
                switch (variables[i].type) {
                    case VAR_TYPE_INT:
                        printf("%d\n", variables[i].value.intValue);
                        break;
                    case VAR_TYPE_FLOAT:
                        printf("%lf\n", variables[i].value.floatValue);
                        break;
                    case VAR_TYPE_STRING:
                        printf("%s\n", variables[i].value.stringValue);
                        break;
                    case VAR_TYPE_BOOL:
                        printf("%s\n", variables[i].value.boolValue ? "true" : "false");
                        break;
                }
                found = 1;
                break;
            }
        }

        if (!found) {
            error("Referenced variable not found");
        }
    } else {
        // Check if the argument is a quoted string
        if ((arg[0] == '"' && arg[strlen(arg) - 1] == '"') ||
            (arg[0] == '\'' && arg[strlen(arg) - 1] == '\'')) {
            // Print the string without quotes
            printf("%.*s\n", (int)strlen(arg) - 2, arg + 1);
        } else {
            // Evaluate and print the expression
            const char* expression = arg;
            double result = evaluate_arithmetic_expression(&expression);
            printf("%lf\n", result);
        }
    }
}

double evaluate_factor(const char** expression) {
    double result = 0.0;
    int negate = 1;
    if (**expression == '-') {
        negate = -1;
        (*expression)++;
    }

    if (**expression == '(') {
        (*expression)++;
        result = evaluate_arithmetic_expression(expression);
        if (**expression == ')') {
            (*expression)++;
        } else {
            error("Mismatched parentheses");
        }
    } else if (isdigit(**expression) || **expression == '.') {
        result = atof(*expression) * negate;
        while (isdigit(**expression) || **expression == '.') {
            (*expression)++;
        }
    } else {
        if (strncmp(*expression, "pi", 2) == 0) {
            result = pi() * negate;
            *expression += 2;
        } else if (strncmp(*expression, "tau", 3) == 0) {
            result = tau() * negate;
            *expression += 3;
        } else if (strncmp(*expression, "e", 1) == 0) {
            result = e() * negate;
            (*expression)++;
        } else if (strncmp(*expression, "goldenRatio", 11) == 0) {
            result = goldenRatio() * negate;
            *expression += 11;
        } else if (strncmp(*expression, "silverRatio", 11) == 0) {
            result = silverRatio() * negate;
            *expression += 11;
        } else if (strncmp(*expression, "superGoldenRatio", 16) == 0) {
            result = superGoldenRatio() * negate;
            *expression += 16;
        } else {
            error("Unknown constant or function");
        }
    }

    return result;
}

double evaluate_power(const char** expression) {
    double base = evaluate_factor(expression);

    while (**expression == '*' && *(*expression + 1) == '*') {
        (*expression) += 2;
        double exponent = evaluate_power(expression);
        base = xenly_pow(base, exponent);
    }
    return base;
}

double evaluate_term(const char** expression) {
    double result = evaluate_power(expression);

    while (**expression == '*' || **expression == '/' || **expression == '%') {
        char operator = **expression;
        (*expression)++;

        double factor = evaluate_power(expression);
        switch (operator) {
            case '*':
                result *= factor;
                break;
            case '/':
                if (factor == 0.0) {
                    error("Division by zero");
                }
                result /= factor;
                break;
            case '%':
                if (factor == 0.0) {
                    error("Modulo by zero");
                }
                result = fmod(result, factor);
                break;
        }
    }
    return result;
}

double evaluate_arithmetic_expression(const char** expression) {
    double result = evaluate_term(expression);
    while (**expression && (**expression == '+' || **expression == '-')) {
        char op = **expression;
        (*expression)++;
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

void execute_math_function(const char* line) {
    char func[MAX_TOKEN_SIZE];
    char arg[MAX_TOKEN_SIZE];

    sscanf(line, "%[^()](%[^)])", func, arg);

    double value = 0.0;
    const char* arg_ptr = arg;
    value = evaluate_arithmetic_expression(&arg_ptr);

    double numbers[0];
    int count = 0;
    char* token = strtok((char*)arg_ptr, ",");

    while (token != NULL && count < MAX_NUMBERS) {
        numbers[count++] = atof(token);
        token = strtok(NULL, ",");
    }

    if (strcmp(func, "xenly_sqrt") == 0) {
        printf("%f\n", xenly_sqrt(value));
    } else if (strcmp(func, "xenly_cbrt") == 0) {
        printf("%f\n", xenly_cbrt(value));
    } else if (strcmp(func, "xenly_ffrt") == 0) {
        printf("%f\n", xenly_ffrt(value));
    } else if (strcmp(func, "xenly_pow") == 0) {
        char base[MAX_TOKEN_SIZE];
        char exp[MAX_TOKEN_SIZE];
        sscanf(arg, "%[^,],%s", base, exp);

        const char* base_ptr = base;
        const char* exp_ptr = exp;
        double base_val = evaluate_arithmetic_expression(&base_ptr);
        double exp_val = evaluate_arithmetic_expression(&exp_ptr);

        printf("%f\n", xenly_pow(base_val, exp_val));
    } else if (strcmp(func, "xenly_sin") == 0) {
        printf("%f\n", xenly_sin(value));
    } else if (strcmp(func, "xenly_cos") == 0) {
        printf("%f\n", xenly_cos(value));
    } else if (strcmp(func, "xenly_tan") == 0) {
        printf("%f\n", xenly_tan(value));
    } else if (strcmp(func, "xenly_csc") == 0) {
        printf("%f\n", xenly_csc(value));
    } else if (strcmp(func, "xenly_sec") == 0) {
        printf("%f\n", xenly_sec(value));
    } else if (strcmp(func, "xenly_cot") == 0) {
        printf("%f\n", xenly_cot(value));
    } else if (strcmp(func, "xenly_abs") == 0) {
        printf("%f\n", xenly_abs(value));
    } else if (strcmp(func, "xenly_bindec") == 0) {
        printf("%f\n", xenly_bindec(arg));
    } else if (strcmp(func, "xenly_decbin") == 0) {
        const char* arg_ptr = arg;
        int value = (int)evaluate_arithmetic_expression(&arg_ptr);
        printf("%s\n", xenly_decbin(value));
    } else if (strcmp(func, "draw_circle") == 0) {
        char x[MAX_TOKEN_SIZE];
        char y[MAX_TOKEN_SIZE];
        char radius[MAX_TOKEN_SIZE];

        x[0] = y[0] = radius[0] = '\0';

        int parsed = sscanf(arg, "%[^,],%[^,],%s", x, y, radius);
        if (parsed != 3) {
            fprintf(stderr, "Error: Invalid arguments for draw_circle. Expected format: x,y,radius\n");
            return;
        }

        const char* x_ptr = x;
        const char* y_ptr = y;
        const char* radius_ptr = radius;

        double x_val = evaluate_arithmetic_expression(&x_ptr);
        double y_val = evaluate_arithmetic_expression(&y_ptr);
        double radius_val = evaluate_arithmetic_expression(&radius_ptr);

        if (x_val == NAN || y_val == NAN || radius_val == NAN) {
            fprintf(stderr, "Error: Invalid numerical values for draw_circle.\n");
            return;
        }

        draw_circle((int)x_val, (int)y_val, (int)radius_val);
    } else {
        error("Unknown function");
    }
}

void execute_var(const char* line) {
    char name[MAX_TOKEN_SIZE];
    char value[MAX_TOKEN_SIZE];
    
    if (sscanf(line, "var %s = %[^\n]", name, value) != 2) {
        error("Invalid 'var' line");
    }

    for (int i = 0; i < result_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            error("Variable already declared");
        }
    }

    if (result_variables >= MAX_VARIABLES) {
        error("Maximum number of variables exceeded");
    }

    const char* value_ptr = value;
    double evaluated_value = evaluate_arithmetic_expression(&value_ptr);
    snprintf(variables[result_variables].value.stringValue, sizeof(variables[result_variables].value.stringValue), "%lf", evaluated_value);

    strcpy(variables[result_variables].name, name);
    result_variables++;
}

double evaluate_condition(const char* condition) {
    for (int i = 0; i < result_variables; i++) {
        if (strcmp(variables[i].name, condition) == 0) {
            return strcmp(variables[i].value.stringValue, "true") == 0 ? 1 : 0;
        }
    }

    return atof(condition);
}

char* trim(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

bool parse_bool(const char* value) {
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
        return true;
    } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
        return false;
    } else {
        fprintf(stderr, "Error: Invalid boolean value '%s'\n", value);
        exit(EXIT_FAILURE);
    }
}
