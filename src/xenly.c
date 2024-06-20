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
#include "error.h"
#include "print_info.h"
#include "project.h"

#if defined(_WIN32) || defined(_WIN64)
// WINDOWS OPERATING SYSTEM
#include <windows.h>

// DYNAMIC-LINK LIBRARY - is a shared library in the Windows operating system.
#define IMPORT_SUFFIX "dll"

#else
// LINUX OPERATING SYSTEM
#include <dlfcn.h>

/*
 * SHARED OBJECT - is a shared library in the Linux operating system because
 * Linux distributions (Examples: Ubuntu, Fedora, Debian, Kali Linux, and more)
 * and Mobile operating systems based on Linux kernel (Examples: Android, and more)
 *
 */
#define IMPORT_SUFFIX "so"
#endif

#define MAX_TOKEN_SIZE 1000
#define MAX_VARIABLES 1000
#define MAX_OBJECTS 1000
#define MAX_ARRAYS 100

typedef struct {
    char name[MAX_TOKEN_SIZE];
    char value[MAX_TOKEN_SIZE];
} Variable;

Variable variables[MAX_VARIABLES];
int result_variables = 0;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    int type; // 0 for variable, 1 for object, 2 for array
    char value[MAX_TOKEN_SIZE];
} Data;

Data data_storage[MAX_VARIABLES + MAX_OBJECTS + MAX_ARRAYS];
int result_data = 0;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    int type; // 0 for variable, 1 for object, 2 for array
    int size;
    double* elements;
} Array;

Array arrays[MAX_ARRAYS];
int result_arrays = 0;
int multiline_comment = 0;

typedef struct {
    char name[MAX_TOKEN_SIZE];
    char version[MAX_TOKEN_SIZE];
    // Add more fields as needed
} Module;

// Declare the function pointers
typedef double (*xenly_constant_t)();
typedef double (*xenly_sqrt_t)(double);
typedef double (*xenly_cbrt_t)(double);
typedef double (*xenly_pow_t)(double, double);
typedef double (*xenly_sin_t)(double);
typedef double (*xenly_cos_t)(double);
typedef double (*xenly_tan_t)(double);
typedef double (*xenly_csc_t)(double);
typedef double (*xenly_sec_t)(double);
typedef double (*xenly_cot_t)(double);
typedef double (*xenly_bindec_t)(const char*);
typedef char* (*xenly_decbin_t)(int);
typedef void (*draw_circle_t)(int, int, int);

// Define the function pointers for math
xenly_constant_t pi;
xenly_constant_t tau;
xenly_constant_t e;
xenly_constant_t goldenRatio;
xenly_constant_t silverRatio;
xenly_constant_t superGoldenRatio;
xenly_sqrt_t xenly_sqrt;
xenly_cbrt_t xenly_cbrt;
xenly_pow_t xenly_pow;
xenly_sin_t xenly_sin;
xenly_cos_t xenly_cos;
xenly_tan_t xenly_tan;
xenly_csc_t xenly_csc;
xenly_sec_t xenly_sec;
xenly_cot_t xenly_cot;

// Define the function pointers for binary math
xenly_bindec_t xenly_bindec;
xenly_decbin_t xenly_decbin;

// Define the function pointers for 2D graphics
draw_circle_t draw_circle;

// Comment
void execute_comment(const char* comment) {
    printf("// %s\n", comment);
}

double evaluate_factor(const char** expression);
double evaluate_arithmetic_expression(const char** expression);

// Load module
#if defined(_WIN32) || defined(_WIN64)
// WINDOWS OPERATING SYSTEM

// Mathematical functions
void load_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    sprintf(filename, "%s.%s", module_name, IMPORT_SUFFIX);
    HMODULE handle = LoadLibrary(filename);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'\n", filename);
        return;
    }

    // Load function pointers using GetProcAddress
    pi = (xenly_constant_t)(void*)GetProcAddress(handle, "pi");
    if (!pi) {
        fprintf(stderr, "Error: Unable to load constant 'pi' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    tau = (xenly_constant_t)(void*)GetProcAddress(handle, "tau");
    if (!tau) {
        fprintf(stderr, "Error: Unable to load constant 'tau' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    e = (xenly_constant_t)(void*)GetProcAddress(handle, "e");
    if (!e) {
        fprintf(stderr, "Error: Unable to load constant 'e' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    goldenRatio = (xenly_constant_t)(void*)GetProcAddress(handle, "goldenRatio");
    if (!goldenRatio) {
        fprintf(stderr, "Error: Unable to load constant 'goldenRatio' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    silverRatio = (xenly_constant_t)(void*)GetProcAddress(handle, "silverRatio");
    if (!silverRatio) {
        fprintf(stderr, "Error: Unable to load constant 'silverRatio' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    superGoldenRatio = (xenly_constant_t)(void*)GetProcAddress(handle, "superGoldenRatio");
    if (!superGoldenRatio) {
        fprintf(stderr, "Error: Unable to load constant 'superGoldenRatio' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_sqrt = (xenly_sqrt_t)(void*)GetProcAddress(handle, "xenly_sqrt");
    if (!xenly_sqrt) {
        fprintf(stderr, "Error: Unable to load function 'xenly_sqrt' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_cbrt = (xenly_cbrt_t)(void*)GetProcAddress(handle, "xenly_cbrt");
    if (!xenly_cbrt) {
        fprintf(stderr, "Error: Unable to load function 'xenly_cbrt' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_pow = (xenly_pow_t)(void*)GetProcAddress(handle, "xenly_pow");
    if (!xenly_pow) {
        fprintf(stderr, "Error: Unable to load function 'xenly_pow' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_sin = (xenly_sin_t)(void*)GetProcAddress(handle, "xenly_sin");
    if (!xenly_sin) {
        fprintf(stderr, "Error: Unable to load function 'xenly_sin' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_cos = (xenly_cos_t)(void*)GetProcAddress(handle, "xenly_cos");
    if (!xenly_cos) {
        fprintf(stderr, "Error: Unable to load function 'xenly_cos' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_tan = (xenly_tan_t)(void*)GetProcAddress(handle, "xenly_tan");
    if (!xenly_tan) {
        fprintf(stderr, "Error: Unable to load function 'xenly_tan' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_csc = (xenly_csc_t)(void*)GetProcAddress(handle, "xenly_csc");
    if (!xenly_csc) {
        fprintf(stderr, "Error: Unable to load function 'xenly_csc' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_sec = (xenly_sec_t)(void*)GetProcAddress(handle, "xenly_sec");
    if (!xenly_sec) {
        fprintf(stderr, "Error: Unable to load function 'xenly_sec' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_cot = (xenly_cot_t)(void*)GetProcAddress(handle, "xenly_cot");
    if (!xenly_cot) {
        fprintf(stderr, "Error: Unable to load function 'xenly_cot' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }
}

// 2D Graphics functions
void load_2d_graphics_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    sprintf(filename, "%s.%s", module_name, IMPORT_SUFFIX);
    HMODULE handle = LoadLibrary(filename);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'\n", filename);
        return;
    }

    // Load function pointers using GetProcAddress
    draw_circle = (draw_circle_t)(void*)GetProcAddress(handle, "draw_circle");
    if (!draw_circle) {
        fprintf(stderr, "Error: Unable to load function 'draw_circle' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }
}

// Binary mathematical functions
void load_binary_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    sprintf(filename, "%s.%s", module_name, IMPORT_SUFFIX);
    HMODULE handle = LoadLibrary(filename);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'\n", filename);
        return;
    }

    xenly_bindec = (xenly_bindec_t)(void*)GetProcAddress(handle, "xenly_bindec");
    if (!xenly_bindec) {
        fprintf(stderr, "Error: Unable to load function 'xenly_bindec' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }

    xenly_decbin = (xenly_decbin_t)(void*)GetProcAddress(handle, "xenly_decbin");
    if (!xenly_decbin) {
        fprintf(stderr, "Error: Unable to load function 'xenly_decbin' from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }
}
#else
// LINUX OPERATING SYSTEM

// Mathematical functions
void load_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    snprintf(filename, sizeof(filename), "%s.%s", module_name, IMPORT_SUFFIX);

    void* handle = dlopen(filename, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'; %s\n", filename, dlerror());
        exit(1);
    }

    // Load function pointers using dlsym
    pi = (xenly_constant_t)dlsym(handle, "pi");
    if (!pi) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    tau = (xenly_constant_t)dlsym(handle, "tau");
    if (!tau) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    e = (xenly_constant_t)dlsym(handle, "e");
    if (!e) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    goldenRatio = (xenly_constant_t)dlsym(handle, "goldenRatio");
    if (!goldenRatio) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    silverRatio = (xenly_constant_t)dlsym(handle, "silverRatio");
    if (!silverRatio) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    superGoldenRatio = (xenly_constant_t)dlsym(handle, "superGoldenRatio");
    if (!superGoldenRatio) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_sqrt = (xenly_sqrt_t)dlsym(handle, "xenly_sqrt");
    if (!xenly_sqrt) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_cbrt = (xenly_cbrt_t)dlsym(handle, "xenly_cbrt");
    if (!xenly_cbrt) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_pow = (xenly_pow_t)dlsym(handle, "xenly_pow");
    if (!xenly_pow) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_sin = (xenly_sin_t)dlsym(handle, "xenly_sin");
    if (!xenly_sin) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_cos = (xenly_cos_t)dlsym(handle, "xenly_cos");
    if (!xenly_cos) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_tan = (xenly_tan_t)dlsym(handle, "xenly_tan");
    if (!xenly_tan) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_csc = (xenly_csc_t)dlsym(handle, "xenly_csc");
    if (!xenly_csc) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_sec = (xenly_sec_t)dlsym(handle, "xenly_sec");
    if (!xenly_sec) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_cot = (xenly_cot_t)dlsym(handle, "xenly_cot");
    if (!xenly_cot) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }
}

// Binary mathematical functions
void load_binary_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    snprintf(filename, sizeof(filename), "%s.%s", module_name, IMPORT_SUFFIX);

    void* handle = dlopen(filename, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'; %s\n", filename, dlerror());
        exit(1);
    }

    xenly_bindec = (xenly_bindec_t)dlsym(handle, "xenly_bindec");
    if (!xenly_bindec) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }

    xenly_decbin = (xenly_decbin_t)dlsym(handle, "xenly_decbin");
    if (!xenly_decbin) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }
}

// 2D Graphics functions
void load_2d_graphics_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    snprintf(filename, sizeof(filename), "%s.%s", module_name, IMPORT_SUFFIX);

    void* handle = dlopen(filename, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'; %s\n", filename, dlerror());
        exit(1);
    }

    draw_circle = (draw_circle_t)dlsym(handle, "draw_circle");
    if (!draw_circle) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(1);
    }
}
#endif

// Factor
double evaluate_factor(const char** expression) {
    // Evaluate a factor in an arithmetic expression
    double result = 0.0;

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
        if (strncmp(*expression, "pi", 2) == 0) {
            result = pi() * negate;
            *expression += 2;
        }
        
        else if (strncmp(*expression, "tau", 3) == 0) {
            result = tau() * negate;
            *expression += 3;
        }
        
        else if (strncmp(*expression, "e", 1) == 0) {
            result = e() * negate;
            (*expression)++;
        }
        
        else if (strncmp(*expression, "goldenRatio", 11) == 0) {
            result = goldenRatio() * negate;
            *expression += 11;
        }
        
        else if (strncmp(*expression, "silverRatio", 11) == 0) {
            result = silverRatio() * negate;
            *expression += 11;
        }
        
        else if (strncmp(*expression, "superGoldenRatio", 16) == 0) {
            result = superGoldenRatio() * negate;
            *expression += 16;
        }
        
        else {
            error("Unknown constant or function");
        }
    }

    return result;
}

// Power
double evaluate_power(const char** expression) {
    // Evaluate base factor first
    double base = evaluate_factor(expression);

    // Check for '**' operator
    while (**expression == '*' && *(*expression + 1) == '*') {
        // Skip the '**' operator
        (*expression) += 2;
        
        // Evaluate exponent recursively
        double exponent = evaluate_power(expression);
        base = xenly_pow(base, exponent);
    }
    return base;
}

// Term
double evaluate_term(const char** expression) {
    // Evaluate a term (power) in an arithmetic expression
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

#if defined(_WIN32) || defined(_WIN64)
// WINDOWS OPERATING SYSTEM
void execute_print(const char* arg) {
    // Check if the argument is a variable reference
    if (arg[0] == '$') {
        // Look for the variable referenced by arg
        char var_name[MAX_TOKEN_SIZE];
        sscanf(arg, "$%s", var_name);
        int found = 0;
        for (int i = 0; i < result_variables; i++) {
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
            const char* expression = arg;
            double result = evaluate_arithmetic_expression(&expression);
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
        for (int i = 0; i < result_variables; i++) {
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

void execute_math_function(const char* line) {
    char func[MAX_TOKEN_SIZE];
    char arg[MAX_TOKEN_SIZE];

    sscanf(line, "%[^()](%[^)])", func, arg);

    double value = 0.0;
    const char* arg_ptr = arg;
    value = evaluate_arithmetic_expression(&arg_ptr);

    if (strcmp(func, "xenly_sqrt") == 0) {
        printf("%f\n", xenly_sqrt(value));
    }
    
    else if (strcmp(func, "xenly_cbrt") == 0) {
        printf("%f\n", xenly_cbrt(value));
    }
    
    else if (strcmp(func, "xenly_pow") == 0) {
        char base[MAX_TOKEN_SIZE];
        char exp[MAX_TOKEN_SIZE];
        sscanf(arg, "%[^,],%s", base, exp);

        const char* base_ptr = base;
        const char* exp_ptr = exp;
        double base_val = evaluate_arithmetic_expression(&base_ptr);
        double exp_val = evaluate_arithmetic_expression(&exp_ptr);

        printf("%f\n", xenly_pow(base_val, exp_val));
    }
    
    else if (strcmp(func, "xenly_sin") == 0) {
        printf("%f\n", xenly_sin(value));
    }
    
    else if (strcmp(func, "xenly_cos") == 0) {
        printf("%f\n", xenly_cos(value));
    }
    
    else if (strcmp(func, "xenly_tan") == 0) {
        printf("%f\n", xenly_tan(value));
    }

    else if (strcmp(func, "xenly_csc") == 0) {
        printf("%f\n", xenly_csc(value));
    }

    else if (strcmp(func, "xenly_sec") == 0) {
        printf("%f\n", xenly_sec(value));
    }

    else if (strcmp(func, "xenly_cot") == 0) {
        printf("%f\n", xenly_cot(value));
    }

    else if (strcmp(func, "xenly_bindec") == 0) {
        printf("%f\n", xenly_bindec(arg));
    }
    
    else if (strcmp(func, "xenly_decbin") == 0) {
        const char* arg_ptr = arg;
        int value = (int)evaluate_arithmetic_expression(&arg_ptr);
        printf("%s\n", xenly_decbin(value));
    }
    
    else if (strcmp(func, "draw_circle") == 0) {
        char x[MAX_TOKEN_SIZE];
        char y[MAX_TOKEN_SIZE];
        char radius[MAX_TOKEN_SIZE];
        
        // Initialize buffers
        x[0] = y[0] = radius[0] = '\0';
    
        // Parse arguments
        int parsed = sscanf(arg, "%[^,],%[^,],%s", x, y, radius);
        if (parsed != 3) {
            fprintf(stderr, "Error: Invalid arguments for draw_circle. Expected format: x,y,radius\n");
            return;
        }
    
        // Convert to double
        const char* x_ptr = x;
        const char* y_ptr = y;
        const char* radius_ptr = radius;
    
        // Evaluate arithmetic expressions
        double x_val = evaluate_arithmetic_expression(&x_ptr);
        double y_val = evaluate_arithmetic_expression(&y_ptr);
        double radius_val = evaluate_arithmetic_expression(&radius_ptr);
    
        // Check if values are valid
        if (x_val == NAN || y_val == NAN || radius_val == NAN) {
            fprintf(stderr, "Error: Invalid numerical values for draw_circle.\n");
            return;
        }
    
        // Draw circle
        draw_circle(x_val, y_val, radius_val);
    }

    else {
        error("Unknown function");
    }
}

void execute_var(const char* line) {
    char name[MAX_TOKEN_SIZE];
    char value[MAX_TOKEN_SIZE];
    
    // Use sscanf to parse the input line
    if (sscanf(line, "var %s = %[^\n]", name, value) != 2) {
        error("Invalid 'var' line");
    }

    // Check if the variable already exists
    for (int i = 0; i < result_variables; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            error("Variable already declared");
        }
    }

    // Ensure variable count does not exceed the maximum
    if (result_variables >= MAX_VARIABLES) {
        error("Maximum number of variables exceeded");
    }

    // Evaluate the value if it's an arithmetic expression
    const char* value_ptr = value;
    double evaluated_value = evaluate_arithmetic_expression(&value_ptr);
    snprintf(variables[result_variables].value, sizeof(variables[result_variables].value), "%lf", evaluated_value);

    // Store the variable name
    strcpy(variables[result_variables].name, name);
    result_variables++;
}

double evaluate_condition(const char* condition) {
    for (int i = 0; i < result_variables; i++) {
        if (strcmp(variables[i].name, condition) == 0) {
            return strcmp(variables[i].value, "true") == 0 ? 1 : 0;
        }
    }

    return atof(condition); // Convert string to double
}

int main(int argc, char* argv[]) {
    if (argc == 3 && (strcmp(argv[1], "--create-project") == 0)) {
        // Create initialize project
        create_initialize_project(argv[2]);
    }

    if (argc != 2) {
        error("Usage: xenly [input file]");
    }

    else if (argc == 2 && (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0)) {
        print_version();
        return 0;
    }

    else if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help();
        return 0;
    }

    else if (argc == 2 && (strcmp(argv[1], "--operatingsystem" ) == 0 || strcmp(argv[1], "-os") == 0)) {
        print_operatingsystem();
        return 0;
    }

    else if (argc == 2 && (strcmp(argv[1], "--dumpmachine") == 0 || strcmp(argv[1], "-dm") == 0)) {
        print_dumpmachines();
        return 0;
    }

    /*
    else if (argc == 2 && (strcmp(argv[1], "--dumpreleasedate") == 0 || strcmp(argv[1], "-drd") == 0)) {
        print_dumpreleasedate();
        return 0;
    }
    */

    else if (argc == 2 && (strcmp(argv[1], "--dumpversion") == 0 || strcmp(argv[1], "-dv") == 0)) {
        print_dumpversion();
        return 0;
    }

    else if (argc == 2 && (strcmp(argv[1], "--new-project") == 0)) {
        initialize_project();
        return 0;
    }

    else if (strcmp(argv[1], "--author") == 0) {
        print_author();
        return 0;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        error("Unable to open input file");
    }

    char line[MAX_TOKEN_SIZE]; // Increased size to match the constant MAX_TOKEN_SIZE
    while (fgets(line, sizeof(line), input_file)) {
        line[strcspn(line, "\n")] = '\0';

        if (strncmp(line, "import ", 7) == 0) {
            char module_name[MAX_TOKEN_SIZE];
            sscanf(line + 7, "%s", module_name);

            // import math
            if (strcmp(module_name, "math") == 0) {
                load_math_module("math");
            }
            
            // import binary_math
            else if (strcmp(module_name, "binary_math") == 0) {
                load_binary_math_module("binary_math");
            }

            // import 2d_graphics
            else if (strcmp(module_name, "2d_graphics") == 0) {
                load_2d_graphics_module("2d_graphics");
            }

            // import std
            /*
            else if (strcmp(module_name, "std") == 0) {
                load_std_module("std");
            }
            */
            
            else {
                fprintf(stderr, "Error: Unknown module '%s'\n", module_name);
            }
        }
        
        else if (strncmp(line, "print(", 6) == 0 && line[strlen(line) - 1] == ')') {
            char argument[MAX_TOKEN_SIZE]; // Increased size to match the constant MAX_TOKEN_SIZE
            strncpy(argument, line + 6, strlen(line) - 7);
            argument[strlen(line) - 7] = '\0';

            execute_print(argument);
        }

        else if (strchr(line, '(') && strchr(line, ')')) {
            execute_math_function(line);
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
        // Handle other function calls similarly...
    }

    fclose(input_file);
    return 0;
}
