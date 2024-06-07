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
#include "color.h"
#include "print_info.h"

#define XENLY_VERSION "0.1.0-nanopreview2"
#define XENLY_AUTHORS "Cyril John Magayaga"

// Print version
void print_version() {
    printf("Xenly %s (Pre-alpha release)\n", XENLY_VERSION);
    printf("Copyright (c) 2023-2024 ");
    setBackgroundBlue();
    white();
    printf(" %s ", XENLY_AUTHORS);
    resetBackgroundColor();
    resetColor();
    printf("\n");
}

// Print dump version
void print_dumpversion() {
    printf("%s\n", XENLY_VERSION);
}

// Print author
void print_author() {
    printf("Copyright (c) 2023-2024 ");
    setBackgroundBlue();
    white();
    printf(" %s ", XENLY_AUTHORS);
    resetBackgroundColor();
    resetColor();
    printf("\n");
}
