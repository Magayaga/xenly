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
#include "project.h"
#include "print_info.h"
#include "xenly.h"

// Main function
int main(int argc, char* argv[]) {
    if (argc == 3 && (strcmp(argv[1], "--create-project") == 0)) {
        // Create initialize project
        create_initialize_project(argv[2]);
    }

    if (argc != 2) {
        printf("Usage: xenly [input file]\n");
        return 1;
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

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        interpret_line(line);
    }

    fclose(file);
    return 0;
}
