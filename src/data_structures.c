/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#include "data_structures.h"

Variable variables[MAX_VARIABLES];
int result_variables = 0;

Data data_storage[MAX_VARIABLES + MAX_OBJECTS + MAX_ARRAYS];
int result_data = 0;

Array arrays[MAX_ARRAYS];
int result_arrays = 0;
int multiline_comment = 0;