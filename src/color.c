/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */
#include <stdio.h>
#include "color.h"

// Red text
void red() {
  printf("\033[1;31m");
}

// Green text
void green() {
  printf("\033[1;32m");
}

// Yellow text
void yellow() {
  printf("\033[1;33m");
}

// Blue text
void blue() {
  printf("\033[1;34m");
}

// Purple text
void purple() {
  printf("\033[1;35m");
}

// White text
void white() {
  printf("\033[1;37m");
}

// Orange text
void orange() {
  printf("\033[1;38;5;208m");
}

// Cyan text
void cyan() {
  printf("\033[1;36m");
}

// Reset color
void resetColor() {
  printf("\033[0m");
}

// Red background
void setBackgroundRed() {
  printf("\033[41m");
}

// Green background
void setBackgroundGreen() {
  printf("\033[42m");
}

// Yellow background
void setBackgroundYellow() {
  printf("\033[43m");
}

// Blue background
void setBackgroundBlue() {
  printf("\033[44m");
}

// Purple background
void setBackgroundPurple() {
  printf("\033[45m");
}

// White background
void setBackgroundWhite() {
  printf("\033[47m");
}

// Reset background color
void resetBackgroundColor() {
  printf("\033[49m");
}

// Black text and Orange background
void blackAndOrange() {
  printf("\033[0;30;48;5;208m");
}
