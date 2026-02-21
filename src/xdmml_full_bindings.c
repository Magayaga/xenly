/*
 * xdmml_full_bindings.c - Complete Xenly bindings for XDMML
 */

#include "interpreter.h"
#include "xdmml/xdmml.h"
#include "xdmml/xdmml_extended.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ═══════════════════════════════════════════════════════════════════════════
// CORE FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

static Value* xdmml_init_fn(Value** args, size_t argc) {
    uint32_t flags = XDMML_INIT_EVERYTHING;
    if (argc > 0 && args[0]->type == VAL_NUMBER) {
        flags = (uint32_t)args[0]->num;
    }
    XDMML_Result result = xdmml_init(flags);
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

// ═══════════════════════════════════════════════════════════════════════════
// WINDOW FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

static Value* xdmml_create_window_fn(Value** args, size_t argc) {
    if (argc < 5) return value_null();
    
    const char* title = (args[0]->type == VAL_STRING) ? args[0]->str : "Window";
    int x = (int)args[1]->num;
    int y = (int)args[2]->num;
    int width = (int)args[3]->num;
    int height = (int)args[4]->num;
    uint32_t flags = (argc > 5) ? (uint32_t)args[5]->num : 0;
    
    XDMML_Window* window = xdmml_create_window(title, x, y, width, height, flags);
    return value_number((double)(uintptr_t)window);
}

static Value* xdmml_destroy_window_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    xdmml_destroy_window(window);
    return value_null();
}

static Value* xdmml_swap_window_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Window* window = (XDMML_Window*)(uintptr_t)args[0]->num;
    xdmml_swap_window(window);
    return value_null();
}

// ═══════════════════════════════════════════════════════════════════════════
// THREADING FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

static Value* xdmml_thread_id_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xdmml_thread_id());
}

static Value* xdmml_create_mutex_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    XDMML_Mutex* mutex = xdmml_create_mutex();
    return value_number((double)(uintptr_t)mutex);
}

static Value* xdmml_destroy_mutex_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Mutex* mutex = (XDMML_Mutex*)(uintptr_t)args[0]->num;
    xdmml_destroy_mutex(mutex);
    return value_null();
}

static Value* xdmml_lock_mutex_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Mutex* mutex = (XDMML_Mutex*)(uintptr_t)args[0]->num;
    xdmml_lock_mutex(mutex);
    return value_null();
}

static Value* xdmml_unlock_mutex_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Mutex* mutex = (XDMML_Mutex*)(uintptr_t)args[0]->num;
    xdmml_unlock_mutex(mutex);
    return value_null();
}

static Value* xdmml_create_semaphore_fn(Value** args, size_t argc) {
    uint32_t initial = (argc > 0) ? (uint32_t)args[0]->num : 0;
    XDMML_Semaphore* sem = xdmml_create_semaphore(initial);
    return value_number((double)(uintptr_t)sem);
}

static Value* xdmml_wait_semaphore_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Semaphore* sem = (XDMML_Semaphore*)(uintptr_t)args[0]->num;
    xdmml_wait_semaphore(sem);
    return value_null();
}

static Value* xdmml_post_semaphore_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Semaphore* sem = (XDMML_Semaphore*)(uintptr_t)args[0]->num;
    xdmml_post_semaphore(sem);
    return value_null();
}

// ═══════════════════════════════════════════════════════════════════════════
// NETWORKING FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

static Value* xdmml_create_socket_fn(Value** args, size_t argc) {
    XDMML_SocketType type = XDMML_SOCKET_TCP;
    if (argc > 0) {
        type = (XDMML_SocketType)(int)args[0]->num;
    }
    XDMML_Socket* socket = xdmml_create_socket(type);
    return value_number((double)(uintptr_t)socket);
}

static Value* xdmml_close_socket_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_Socket* socket = (XDMML_Socket*)(uintptr_t)args[0]->num;
    xdmml_close_socket(socket);
    return value_null();
}

static Value* xdmml_connect_fn(Value** args, size_t argc) {
    if (argc < 3) return value_number(-1);
    
    XDMML_Socket* socket = (XDMML_Socket*)(uintptr_t)args[0]->num;
    const char* host = (args[1]->type == VAL_STRING) ? args[1]->str : "";
    uint16_t port = (uint16_t)args[2]->num;
    
    XDMML_Result result = xdmml_connect(socket, host, port);
    return value_number((double)result);
}

static Value* xdmml_send_fn(Value** args, size_t argc) {
    if (argc < 2) return value_number(-1);
    
    XDMML_Socket* socket = (XDMML_Socket*)(uintptr_t)args[0]->num;
    const char* data = (args[1]->type == VAL_STRING) ? args[1]->str : "";
    
    int sent = xdmml_send(socket, data, strlen(data));
    return value_number((double)sent);
}

static Value* xdmml_recv_fn(Value** args, size_t argc) {
    if (argc < 2) return value_null();
    
    XDMML_Socket* socket = (XDMML_Socket*)(uintptr_t)args[0]->num;
    int max_len = (int)args[1]->num;
    
    char* buffer = (char*)malloc(max_len + 1);
    if (!buffer) return value_null();
    
    int received = xdmml_recv(socket, buffer, max_len);
    if (received > 0) {
        buffer[received] = '\0';
        Value* result = value_string(buffer);
        free(buffer);
        return result;
    }
    
    free(buffer);
    return value_null();
}

static Value* xdmml_http_get_fn(Value** args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) {
        return value_null();
    }
    
    XDMML_HTTPResponse* resp = xdmml_http_get(args[0]->str);
    if (!resp) return value_null();
    
    Value* result = resp->body ? value_string(resp->body) : value_null();
    xdmml_free_http_response(resp);
    
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════
// DYNAMIC LOADING FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

static Value* xdmml_load_library_fn(Value** args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) {
        return value_null();
    }
    
    XDMML_SharedObject* lib = xdmml_load_library(args[0]->str);
    return value_number((double)(uintptr_t)lib);
}

static Value* xdmml_unload_library_fn(Value** args, size_t argc) {
    if (argc < 1) return value_null();
    XDMML_SharedObject* lib = (XDMML_SharedObject*)(uintptr_t)args[0]->num;
    xdmml_unload_library(lib);
    return value_null();
}

static Value* xdmml_get_symbol_fn(Value** args, size_t argc) {
    if (argc < 2 || args[1]->type != VAL_STRING) {
        return value_null();
    }
    
    XDMML_SharedObject* lib = (XDMML_SharedObject*)(uintptr_t)args[0]->num;
    void* symbol = xdmml_get_symbol(lib, args[1]->str);
    
    return value_number((double)(uintptr_t)symbol);
}

// ═══════════════════════════════════════════════════════════════════════════
// FILE SYSTEM FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

static Value* xdmml_get_home_dir_fn(Value** args, size_t argc) {
    (void)args; (void)argc;
    const char* home = xdmml_get_home_dir();
    return home ? value_string(home) : value_null();
}

static Value* xdmml_get_pref_dir_fn(Value** args, size_t argc) {
    const char* org = (argc > 0 && args[0]->type == VAL_STRING) ? args[0]->str : "xenly";
    const char* app = (argc > 1 && args[1]->type == VAL_STRING) ? args[1]->str : "app";
    
    const char* pref = xdmml_get_pref_dir(org, app);
    return pref ? value_string(pref) : value_null();
}

static Value* xdmml_path_exists_fn(Value** args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) {
        return value_bool(false);
    }
    return value_bool(xdmml_path_exists(args[0]->str));
}

static Value* xdmml_mkdir_fn(Value** args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) {
        return value_bool(false);
    }
    return value_bool(xdmml_mkdir(args[0]->str));
}

static Value* xdmml_list_directory_fn(Value** args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) {
        return value_null();
    }
    
    int count;
    char** files = xdmml_list_directory(args[0]->str, &count);
    if (!files || count == 0) return value_null();
    
    // Create array of string values
    Value** items = (Value**)malloc(sizeof(Value*) * count);
    for (int i = 0; i < count; i++) {
        items[i] = value_string(files[i]);
    }
    
    xdmml_free_directory_list(files, count);
    return value_array(items, count);
}

// ═══════════════════════════════════════════════════════════════════════════
// TIMING FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

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

// ═══════════════════════════════════════════════════════════════════════════
// MODULE REGISTRATION
// ═══════════════════════════════════════════════════════════════════════════

typedef struct {
    const char* name;
    Value* (*func)(Value**, size_t);
} XDMMLFunction;

static XDMMLFunction xdmml_all_functions[] = {
    // Core
    { "init", xdmml_init_fn },
    { "quit", xdmml_quit_fn },
    { "get_version", xdmml_get_version_fn },
    
    // Window
    { "create_window", xdmml_create_window_fn },
    { "destroy_window", xdmml_destroy_window_fn },
    { "swap_window", xdmml_swap_window_fn },
    
    // Threading
    { "thread_id", xdmml_thread_id_fn },
    { "create_mutex", xdmml_create_mutex_fn },
    { "destroy_mutex", xdmml_destroy_mutex_fn },
    { "lock_mutex", xdmml_lock_mutex_fn },
    { "unlock_mutex", xdmml_unlock_mutex_fn },
    { "create_semaphore", xdmml_create_semaphore_fn },
    { "wait_semaphore", xdmml_wait_semaphore_fn },
    { "post_semaphore", xdmml_post_semaphore_fn },
    
    // Networking
    { "create_socket", xdmml_create_socket_fn },
    { "close_socket", xdmml_close_socket_fn },
    { "connect", xdmml_connect_fn },
    { "send", xdmml_send_fn },
    { "recv", xdmml_recv_fn },
    { "http_get", xdmml_http_get_fn },
    
    // Dynamic Loading
    { "load_library", xdmml_load_library_fn },
    { "unload_library", xdmml_unload_library_fn },
    { "get_symbol", xdmml_get_symbol_fn },
    
    // File System
    { "get_home_dir", xdmml_get_home_dir_fn },
    { "get_pref_dir", xdmml_get_pref_dir_fn },
    { "path_exists", xdmml_path_exists_fn },
    { "mkdir", xdmml_mkdir_fn },
    { "list_directory", xdmml_list_directory_fn },
    
    // Timing
    { "get_ticks", xdmml_get_ticks_fn },
    { "delay", xdmml_delay_fn },
    
    { NULL, NULL }
};

Module module_xdmml(void) {
    Module mod = {0};
    mod.name = NULL;
    
    // Count functions
    int count = 0;
    while (xdmml_all_functions[count].name) count++;
    
    mod.functions = malloc(sizeof(NativeFunc) * (count + 1));
    for (int i = 0; i < count; i++) {
        mod.functions[i].name = (char*)xdmml_all_functions[i].name;
        mod.functions[i].fn = xdmml_all_functions[i].func;
    }
    mod.functions[count].name = NULL;
    mod.functions[count].fn = NULL;
    mod.fn_count = count;
    
    return mod;
}
