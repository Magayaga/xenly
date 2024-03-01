/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "color.h"
#include "print_info.h"

#define XENLY_RELEASEDATE "March 30, 2024"
#define XENLY_VERSION "0.1.0-preview5"

// Print version
void print_version() {
    printf("Xenly %s (Pre-alpha release)\n", XENLY_VERSION);
    printf("Copyright (c) 2023-2024 ");
    setBackgroundBlue();
    white();
    printf(" Cyril John Magayaga ");
    resetBackgroundColor();
    resetColor();
    printf("\n");
}

// Print dump version
void print_dumpversion() {
    printf("%s\n", XENLY_VERSION);
}

// Print dump release date
void print_dumpreleasedate() {
    printf("%s\n", XENLY_RELEASEDATE);
}

// Print help
void print_help() {
    printf("Usage: xenly [input file]\n");
    setBackgroundBlue();
    white();
    printf(" Options: ");
    resetBackgroundColor();
    resetColor();
    printf("\n");
    printf("  -h, --help                   Display this information.\n");
    printf("  -v, --version                Display compiler version information.\n");
    printf("  -dv, --dumpversion           Display the version of the compiler.\n");
    printf("  -drd, --dumpreleasedate      Display the release date of the compiler.\n");
    printf("  -dm, --dumpmachine           Display the compiler's target processor.\n");
    printf("  -os, --operatingsystem       Display the operating system.\n");
    printf("  --author                     Display the author information.\n");
    printf("  --init                       Create a new xenly package.\n");
    printf("For bug reporting instructions, please see:\n");
    printf("<https://github.com/magayaga/xenly>\n");
}

// Print author
void print_author() {
    printf("Copyright (c) 2023-2024 ");
    setBackgroundBlue();
    white();
    printf(" Cyril John Magayaga ");
    resetBackgroundColor();
    resetColor();
    printf("\n");
}

// Print operating system
void print_operatingsystem() {
    // Print the compiler's operating system
    #if defined(_WIN32)
        printf("Windows\n");
    #elif defined(__linux__)
        printf("Linux\n");
    #elif defined(__unix__) || defined(__unix)
        printf("Unix\n");
    #elif defined(__APPLE__) || defined(__MACH__)
        #include "TargetConditionals.h"
        #if TARGET_OS_MAC
            printf("macOS\n");
        #elif TARGET_OS_IOS
            printf("iOS\n");
        #elif TARGET_OS_TV
            printf("tvOS\n");
        #elif TARGET_OS_WATCH
            printf("watchOS\n");
        #endif
    #elif defined(__ANDROID__)
        printf("Android\n");
    #elif defined(__FreeBSD__)
        printf("FreeBSD\n");
    #elif defined(__DragonFly__)
        printf("DragonFlyBSD\n");
    #elif defined(__OpenBSD__)
        printf("OpenBSD\n");
    #elif defined(__NetBSD__)
        printf("NetBSD\n");
    #else
        printf("Unknown/Segmentation fault\n");
    #endif
}

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
    printf("New Xenly project initialized in 'xenly project' folder.\n");
}