/*
 * multiproc_builtins.c  —  Built-in functions for multiprocessing
 */

#include "interpreter.h"
#include "multiproc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Global pools (initialized on first use)
static ThreadPool *g_thread_pool = NULL;
static ProcessPool *g_process_pool = NULL;

// ─── Thread Pool Functions ────────────────────────────────────────────────────
Value *builtin_thread_pool_create(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) {
        fprintf(stderr, "thread_pool_create expects (num_workers: number)\n");
        return value_null();
    }
    
    size_t num_workers = (size_t)args[0]->num;
    ThreadPool *pool = thread_pool_create(num_workers);
    
    // Wrap pool pointer as a number (hacky but works for POC)
    return value_number((double)(uintptr_t)pool);
}

Value *builtin_thread_pool_submit(Value **args, size_t argc) {
    if (argc < 2) {
        fprintf(stderr, "thread_pool_submit expects (pool, fn, ...args)\n");
        return value_null();
    }
    
    ThreadPool *pool = (ThreadPool *)(uintptr_t)args[0]->num;
    if (args[1]->type != VAL_FUNCTION) {
        fprintf(stderr, "thread_pool_submit expects a function as second argument\n");
        return value_null();
    }
    
    FnDef *fn = args[1]->fn;
    Value **fn_args = (argc > 2) ? &args[2] : NULL;
    size_t fn_argc = (argc > 2) ? argc - 2 : 0;
    
    Future *fut = thread_pool_submit(pool, fn, fn_args, fn_argc);
    return value_number((double)(uintptr_t)fut);
}

Value *builtin_thread_pool_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    ThreadPool *pool = (ThreadPool *)(uintptr_t)args[0]->num;
    thread_pool_destroy(pool);
    return value_null();
}

// ─── Process Pool Functions ───────────────────────────────────────────────────
Value *builtin_process_pool_create(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) {
        fprintf(stderr, "process_pool_create expects (num_workers: number)\n");
        return value_null();
    }
    
    size_t num_workers = (size_t)args[0]->num;
    ProcessPool *pool = process_pool_create(num_workers);
    
    return value_number((double)(uintptr_t)pool);
}

Value *builtin_process_pool_submit(Value **args, size_t argc) {
    if (argc < 2) {
        fprintf(stderr, "process_pool_submit expects (pool, fn, ...args)\n");
        return value_null();
    }
    
    ProcessPool *pool = (ProcessPool *)(uintptr_t)args[0]->num;
    if (args[1]->type != VAL_FUNCTION) {
        fprintf(stderr, "process_pool_submit expects a function as second argument\n");
        return value_null();
    }
    
    FnDef *fn = args[1]->fn;
    Value **fn_args = (argc > 2) ? &args[2] : NULL;
    size_t fn_argc = (argc > 2) ? argc - 2 : 0;
    
    Future *fut = process_pool_submit(pool, fn, fn_args, fn_argc);
    return value_number((double)(uintptr_t)fut);
}

Value *builtin_process_pool_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    ProcessPool *pool = (ProcessPool *)(uintptr_t)args[0]->num;
    process_pool_destroy(pool);
    return value_null();
}

// ─── Channel Functions ────────────────────────────────────────────────────────
Value *builtin_channel_create(Value **args, size_t argc) {
    size_t capacity = 0;
    if (argc > 0 && args[0]->type == VAL_NUMBER) {
        capacity = (size_t)args[0]->num;
    }
    
    Channel *chan = channel_create(capacity);
    return value_number((double)(uintptr_t)chan);
}

Value *builtin_channel_send(Value **args, size_t argc) {
    if (argc < 2) {
        fprintf(stderr, "channel_send expects (channel, value)\n");
        return value_null();
    }
    
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    Value *data = args[1];
    
    int result = channel_send(chan, data);
    return value_number((double)result);
}

Value *builtin_channel_recv(Value **args, size_t argc) {
    if (argc < 1) {
        fprintf(stderr, "channel_recv expects (channel)\n");
        return value_null();
    }
    
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    return channel_recv(chan);
}

Value *builtin_channel_try_recv(Value **args, size_t argc) {
    if (argc < 1) {
        fprintf(stderr, "channel_try_recv expects (channel)\n");
        return value_null();
    }
    
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    Value *out = NULL;
    int result = channel_try_recv(chan, &out);
    
    if (result == 0) {
        return out;
    } else {
        return value_null();
    }
}

Value *builtin_channel_close(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    channel_close(chan);
    return value_null();
}

Value *builtin_channel_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Channel *chan = (Channel *)(uintptr_t)args[0]->num;
    channel_destroy(chan);
    return value_null();
}

// ─── Future Functions ─────────────────────────────────────────────────────────
Value *builtin_future_get(Value **args, size_t argc) {
    if (argc < 1) {
        fprintf(stderr, "future_get expects (future)\n");
        return value_null();
    }
    
    Future *fut = (Future *)(uintptr_t)args[0]->num;
    return future_get(fut);
}

Value *builtin_future_is_ready(Value **args, size_t argc) {
    if (argc < 1) return value_bool(0);
    Future *fut = (Future *)(uintptr_t)args[0]->num;
    return value_bool(future_is_ready(fut));
}

Value *builtin_future_destroy(Value **args, size_t argc) {
    if (argc < 1) return value_null();
    Future *fut = (Future *)(uintptr_t)args[0]->num;
    future_destroy(fut);
    return value_null();
}

// ─── CPU-bound computation helpers ────────────────────────────────────────────
Value *builtin_cpu_count(Value **args, size_t argc) {
    (void)args; (void)argc;
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    return value_number((double)nproc);
}

// ─── Registration ─────────────────────────────────────────────────────────────
void register_multiproc_builtins(struct Interpreter *interp) {
    // Thread pool
    register_builtin(interp, "thread_pool_create", builtin_thread_pool_create);
    register_builtin(interp, "thread_pool_submit", builtin_thread_pool_submit);
    register_builtin(interp, "thread_pool_destroy", builtin_thread_pool_destroy);
    
    // Process pool
    register_builtin(interp, "process_pool_create", builtin_process_pool_create);
    register_builtin(interp, "process_pool_submit", builtin_process_pool_submit);
    register_builtin(interp, "process_pool_destroy", builtin_process_pool_destroy);
    
    // Channels
    register_builtin(interp, "channel_create", builtin_channel_create);
    register_builtin(interp, "channel_send", builtin_channel_send);
    register_builtin(interp, "channel_recv", builtin_channel_recv);
    register_builtin(interp, "channel_try_recv", builtin_channel_try_recv);
    register_builtin(interp, "channel_close", builtin_channel_close);
    register_builtin(interp, "channel_destroy", builtin_channel_destroy);
    
    // Futures
    register_builtin(interp, "future_get", builtin_future_get);
    register_builtin(interp, "future_is_ready", builtin_future_is_ready);
    register_builtin(interp, "future_destroy", builtin_future_destroy);
    
    // Utilities
    register_builtin(interp, "cpu_count", builtin_cpu_count);
}
