/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 */
#include <stdio.h>
#include "color.h"

void red() {
  printf("\033[1;31m");
}

void green() {
  printf("\033[1;32m");
}

void yellow() {
  printf("\033[1;33m");
}

void blue() {
  printf("\033[1;34m");
}

void purple() {
  printf("\033[1;35m");
}

void white() {
  printf("\033[1;37m");
}

void orange() {
  printf("\033[1;38;5;208m");
}

void cyan() {
  printf("\033[1;36m");
}

void resetColor() {
  printf("\033[0m");
}

void setBackgroundRed() {
  printf("\033[41m");
}

void setBackgroundGreen() {
  printf("\033[42m");
}

void setBackgroundYellow() {
  printf("\033[43m");
}

void setBackgroundBlue() {
  printf("\033[44m");
}

void setBackgroundPurple() {
  printf("\033[45m");
}

void setBackgroundWhite() {
  printf("\033[47m");
}

void resetBackgroundColor() {
  printf("\033[49m");
}

void blackAndOrange() {
  printf("\033[0;30;43m");
}