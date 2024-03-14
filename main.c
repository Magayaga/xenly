#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For chdir function
#include <string.h> // For string manipulation functions

void setBackgroundGreen() {
    printf("\033[42m"); // ANSI escape code for green background
}

void resetColor() {
    printf("\033[0m"); // ANSI escape code to reset color
}

void resetBackgroundColor() {
    printf("\033[49m"); // ANSI escape code to reset background color
}

void initialize_project(const char *project_name) {
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

int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "--create-project") == 0) {
        initialize_project(argv[2]);
    } else {
        printf("Usage: %s --create-project <project_name>\n", argv[0]);
    }
    return 0;
}
