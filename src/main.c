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
#include "error.h"
#include "print_info.h"
#include "project.h"
#include "data_structures.h"
#include "math_functions.h"
#include "binary_math_functions.h"
#include "graphics_functions.h"
#include "utility.h"

// Main function
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
    }

    fclose(input_file);
    return 0;
}