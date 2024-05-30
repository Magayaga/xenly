/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "math_binary.h"

// Binary numbers to decimal
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

// Decimal to Binary number
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