/*
 * xdmml_bindings.c - Xenly bindings for XDMML
 */

#include "interpreter.h"
#include "xdmml/xdmml.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ─── XDMML Init/Quit ─────────────────────────────────────────────────────────

static Value* xdmml_init_fn(Value** args, size_t argc) {
    uint32_t flags = XDMML_INIT_EVERYTHING;
    if (argc > 0 && args[0]->type == VAL_NUMBER) {
        flags = (uint32_t)args[0]->num;
    }
    
    XDMML_Result result = xdmml_init(flags);
    if (result != XDMML_OK) {
        fprintf(stderr, "[XDMML] Init failed: %s\n", xdmml_get_error(result));
    }
    
    return value_number((double)result);
}

static Value* xdmml_quit_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    xdmml_quit();
    return value_null();
}

static Value* xdmml_get_version_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    return value_string(xdmml_get_version());
}

// ─── Window Functions ────────────────────────────────────────────────────────

static Value* xdmml_create_window_fn(Value** args, size_t argc) {
    if (argc < 5) {
        return value_null();
    }
    
    const char* title = (args[0]->type == VAL_STRING) ? args[0]->str : "Window";
    int x = (int)args[1]->num;
    int y = (int)args[2]->num;
    int width = (int)args[3]->num;
    int height = (int)args[4]->num;
    uint32_t flags = (argc > 5) ? (uint32_t)args[5]->num : 0;
    
    XDMML_Window* window = xdmml_create_window(title, x, y, width, height, flags);
    
    // Return window handle as number (pointer cast)
    return value_number((double)(uintptr_t)window);
}

static Value* xdmml_destroy_window_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    xdmml_destroy_window(window);
    
    return value_null();
}

static Value* xdmml_set_window_title_fn(Value** args, size_t argc) {
    if (argc < 2) return value_null();
    
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    const char* title = (args[1]->type == VAL_STRING) ? args[1]->str : "";
    
    xdmml_set_window_title(window, title);
    return value_null();
}

static Value* xdmml_swap_window_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    xdmml_swap_window(window);
    
    return value_null();
}

// ─── OpenGL Functions ────────────────────────────────────────────────────────

static Value* xdmml_gl_create_context_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    XDMML_GLContext* context = xdmml_gl_create_context(window);
    
    return value_number((double)(uintptr_t)context);
}

static Value* xdmml_gl_make_current_fn(Value** args, size_t argc) {
    if (argc < 2) return value_null();
    
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    XDMML_GLContext* context = (XDMML_GLContext*)(uintptr_t)args[1]->num;
    
    xdmml_gl_make_current(window, context);
    return value_null();
}

// ─── Event Functions ─────────────────────────────────────────────────────────

static Value* xdmml_poll_event_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    
    XDMML_Event event;
    bool has_event = xdmml_poll_event(&event);
    
    if (!has_event) {
        return value_null();
    }
    
    // Return event as object (simplified - just type and timestamp)
    Value* obj = value_number((double)event.type);
    return obj;
}

// ─── Timing Functions ────────────────────────────────────────────────────────

static Value* xdmml_get_ticks_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xdmml_get_ticks());
}

static Value* xdmml_delay_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    
    uint32_t ms = (uint32_t)args[0]->num;
    xdmml_delay(ms);
    
    return value_null();
}

// ─── Constants ───────────────────────────────────────────────────────────────

static Value* get_xdmml_constant(const char* name) {
    // Window flags
    if (strcmp(name, "WINDOW_OPENGL") == 0) return value_number(XDMML_WINDOW_OPENGL);
    if (strcmp(name, "WINDOW_RESIZABLE") == 0) return value_number(XDMML_WINDOW_RESIZABLE);
    if (strcmp(name, "WINDOW_FULLSCREEN") == 0) return value_number(XDMML_WINDOW_FULLSCREEN);
    
    // Init flags
    if (strcmp(name, "INIT_VIDEO") == 0) return value_number(XDMML_INIT_VIDEO);
    if (strcmp(name, "INIT_AUDIO") == 0) return value_number(XDMML_INIT_AUDIO);
    if (strcmp(name, "INIT_EVERYTHING") == 0) return value_number(XDMML_INIT_EVERYTHING);
    
    // Window position
    if (strcmp(name, "WINDOWPOS_CENTERED") == 0) return value_number(XDMML_WINDOWPOS_CENTERED);
    
    return value_null();
}

// ─── Module Registration ─────────────────────────────────────────────────────

typedef struct {
    const char* name;
    Value* (*func)(Value**, size_t);
} XDMMLFunction;

static XDMMLFunction xdmml_functions[] = {
    // Core
    { "init", xdmml_init_fn },
    { "quit", xdmml_quit_fn },
    { "get_version", xdmml_get_version_fn },
    
    // Window
    { "create_window", xdmml_create_window_fn },
    { "destroy_window", xdmml_destroy_window_fn },
    { "set_window_title", xdmml_set_window_title_fn },
    { "swap_window", xdmml_swap_window_fn },
    
    // OpenGL
    { "gl_create_context", xdmml_gl_create_context_fn },
    { "gl_make_current", xdmml_gl_make_current_fn },
    
    // Events
    { "poll_event", xdmml_poll_event_fn },
    
    // Timing
    { "get_ticks", xdmml_get_ticks_fn },
    { "delay", xdmml_delay_fn },
    
    { NULL, NULL }
};

Module module_xdmml(void) {
    Module mod = {0};
    mod.name = "xdmml";
    
    // Count functions
    int count = 0;
    while (xdmml_functions[count].name) count++;
    
    mod.functions = malloc(sizeof(NativeFunc) * (count + 1));
    for (int i = 0; i < count; i++) {
        mod.functions[i].name = xdmml_functions[i].name;
        mod.functions[i].func = xdmml_functions[i].func;
    }
    mod.functions[count].name = NULL;
    mod.functions[count].func = NULL;
    
    return mod;
}
