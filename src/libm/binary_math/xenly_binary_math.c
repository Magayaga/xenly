/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * `xenly_binary_math.c` is the similar to the `xenly_binary_math.go` in Go programming language and and
 * `xenly_binary_math.rs` in Rust programming language.
 *
 * It is available for Linux and Windows operating systems.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Convert binary string to decimal
double xenly_bindec(const char* binary) {
    return strtol(binary, NULL, 2);
}

// Convert decimal to binary string
char* xenly_decbin(int decimal) {
    static char binary[33];
    binary[32] = '\0';
    char* ptr = &binary[31];
    if (decimal == 0) {
        *ptr = '0';
        return ptr;
    }
    while (decimal > 0) {
        *ptr-- = (decimal % 2) + '0';
        decimal /= 2;
    }
    return ptr + 1;
}
