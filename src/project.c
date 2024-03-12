#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "color.h"

void initialize_project() {
    // Create a new folder for the project
    system("mkdir xenly_project");

    // Change directory to the newly created folder
    chdir("xenly_project");

    // Create a new Xenly source file
    FILE *source_file = fopen("main.xe", "w");
    if (source_file == NULL) {
        perror("Unable to create source file");
    }

    // Write default "hello world" program to the source file
    fprintf(source_file, "print(\"Hello, World!\")\nprint(2*9-6/3*5)\n");
    fclose(source_file);
    
    // Inform the user that the project has been initialized
    printf("New Xenly project initialized in ");
    setBackgroundGreen();
    white();
    printf(" 'xenly_project' ");
    resetColor();
    resetBackgroundColor();
    printf(" folder.\n");
}

void create_project(const char *project_name) {
    // Create a new folder for the project
    char mkdir_command[100];
    sprintf(mkdir_command, "mkdir %s_project", project_name);
    if (system(mkdir_command) != 0) {
        perror("Unable to create project directory");
        return;
    }

    // Change directory to the newly created folder
    if (chdir(project_name) != 0) {
        perror("Unable to change directory to project folder");
        return;
    }

    // Create a new Xenly source file
    FILE *source_file = fopen("main.xe", "w");
    if (source_file == NULL) {
        perror("Unable to create source file");
        return;
    }

    // Write default "hello world" program to the source file
    fprintf(source_file, "print(\"Hello, World!\")\nprint(2*9-6/3*5)\n");

    // Close the source file
    fclose(source_file);

    // Inform the user that the project has been initialized
    printf("New Xenly project initialized in ");
    setBackgroundGreen();
    printf(" '%s_project' ", project_name);
    resetColor();
    resetBackgroundColor();
    printf(" folder.\n");
}