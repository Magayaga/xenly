/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#ifndef GRAPHICS_FUNCTIONS_H
#define GRAPHICS_FUNCTIONS_H

typedef void (*draw_circle_t)(int, int, int);

extern draw_circle_t draw_circle;

void load_2d_graphics_module(const char* module_name);

#endif // GRAPHICS_FUNCTIONS_H