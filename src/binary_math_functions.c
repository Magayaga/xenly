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
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "binary_math_functions.h"
#include "error.h"
#include "data_structures.h"

xenly_bindec_t xenly_bindec;
xenly_decbin_t xenly_decbin;

#if defined(_WIN32) || defined(_WIN64)
void load_binary_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    sprintf(filename, "%s.%s", module_name, "dll");
    HMODULE handle = LoadLibrary(filename);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'\n", filename);
        return;
    }

    xenly_bindec = (xenly_bindec_t)(void*)GetProcAddress(handle, "xenly_bindec");
    xenly_decbin = (xenly_decbin_t)(void*)GetProcAddress(handle, "xenly_decbin");

    if (!xenly_bindec || !xenly_decbin) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'\n", filename);
        FreeLibrary(handle);
        return;
    }
}
#else
void load_binary_math_module(const char* module_name) {
    char filename[MAX_TOKEN_SIZE];
    snprintf(filename, sizeof(filename), "%s.%s", module_name, "so");

    void* handle = dlopen(filename, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open module file '%s'; %s\n", filename, dlerror());
        exit(EXIT_FAILURE);
    }

    xenly_bindec = (xenly_bindec_t)dlsym(handle, "xenly_bindec");
    xenly_decbin = (xenly_decbin_t)dlsym(handle, "xenly_decbin");

    if (!xenly_bindec || !xenly_decbin) {
        fprintf(stderr, "Error: Unable to load functions from module '%s'; %s\n", filename, dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }
}
#endif