/*
 * multiproc.c  —  High-performance multiprocessing for Xenly
 *
 * Features:
 * - Process pools for CPU-bound tasks (true parallelism)
 * - Thread pools for I/O-bound tasks
 * - Channels for inter-task communication
 * - Futures/Promises for async results
 * - Integrated with interpreter's spawn/await
 */

#include "multiproc.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

// Forward declare eval from interpreter.c
extern Value *eval(struct Interpreter *interp, ASTNode *node, Environment *env);

// ─── Future Implementation ────────────────────────────────────────────────────
Future *future_create(void) {
    Future *fut = (Future *)calloc(1, sizeof(Future));
    pthread_mutex_init(&fut->lock, NULL);
    pthread_cond_init(&fut->cond, NULL);
    fut->ready = 0;
    fut->result = NULL;
    return fut;
}

void future_destroy(Future *fut) {
    if (!fut) return;
    pthread_mutex_destroy(&fut->lock);
    pthread_cond_destroy(&fut->cond);
    // Don't destroy result — caller owns it
    free(fut);
}

void future_set(Future *fut, Value *result) {
    pthread_mutex_lock(&fut->lock);
    fut->result = result;
    fut->ready = 1;
    pthread_cond_broadcast(&fut->cond);
    pthread_mutex_unlock(&fut->lock);
}

Value *future_get(Future *fut) {
    pthread_mutex_lock(&fut->lock);
    while (!fut->ready)
        pthread_cond_wait(&fut->cond, &fut->lock);
    Value *result = fut->result;
    pthread_mutex_unlock(&fut->lock);
    return result;
}

int future_is_ready(Future *fut) {
    pthread_mutex_lock(&fut->lock);
    int ready = fut->ready;
    pthread_mutex_unlock(&fut->lock);
    return ready;
}

// ─── Channel Implementation ───────────────────────────────────────────────────
Channel *channel_create(size_t capacity) {
    Channel *chan = (Channel *)calloc(1, sizeof(Channel));
    chan->capacity = capacity;
    pthread_mutex_init(&chan->lock, NULL);
    pthread_cond_init(&chan->not_empty, NULL);
    pthread_cond_init(&chan->not_full, NULL);
    return chan;
}

void channel_destroy(Channel *chan) {
    if (!chan) return;
    // Free queued messages
    ChannelMessage *msg = chan->queue;
    while (msg) {
        ChannelMessage *next = msg->next;
        value_destroy(msg->data);
        free(msg);
        msg = next;
    }
    pthread_mutex_destroy(&chan->lock);
    pthread_cond_destroy(&chan->not_empty);
    pthread_cond_destroy(&chan->not_full);
    free(chan);
}

int channel_send(Channel *chan, Value *data) {
    pthread_mutex_lock(&chan->lock);
    
    if (chan->closed) {
        pthread_mutex_unlock(&chan->lock);
        return -1;  // channel closed
    }
    
    // Wait if channel is full (buffered channels only)
    while (chan->capacity > 0 && chan->size >= chan->capacity) {
        if (chan->closed) {
            pthread_mutex_unlock(&chan->lock);
            return -1;
        }
        pthread_cond_wait(&chan->not_full, &chan->lock);
    }
    
    // Enqueue message
    ChannelMessage *msg = (ChannelMessage *)malloc(sizeof(ChannelMessage));
    msg->data = data;
    msg->next = NULL;
    
    if (chan->queue_tail) {
        chan->queue_tail->next = msg;
        chan->queue_tail = msg;
    } else {
        chan->queue = chan->queue_tail = msg;
    }
    chan->size++;
    
    pthread_cond_signal(&chan->not_empty);
    pthread_mutex_unlock(&chan->lock);
    return 0;
}

Value *channel_recv(Channel *chan) {
    pthread_mutex_lock(&chan->lock);
    
    // Wait for data
    while (!chan->queue && !chan->closed)
        pthread_cond_wait(&chan->not_empty, &chan->lock);
    
    if (!chan->queue) {
        // Channel closed and empty
        pthread_mutex_unlock(&chan->lock);
        return value_null();
    }
    
    // Dequeue message
    ChannelMessage *msg = chan->queue;
    chan->queue = msg->next;
    if (!chan->queue)
        chan->queue_tail = NULL;
    chan->size--;
    
    Value *data = msg->data;
    free(msg);
    
    pthread_cond_signal(&chan->not_full);
    pthread_mutex_unlock(&chan->lock);
    return data;
}

int channel_try_recv(Channel *chan, Value **out) {
    pthread_mutex_lock(&chan->lock);
    
    if (!chan->queue) {
        pthread_mutex_unlock(&chan->lock);
        return -1;  // no data available
    }
    
    ChannelMessage *msg = chan->queue;
    chan->queue = msg->next;
    if (!chan->queue)
        chan->queue_tail = NULL;
    chan->size--;
    
    *out = msg->data;
    free(msg);
    
    pthread_cond_signal(&chan->not_full);
    pthread_mutex_unlock(&chan->lock);
    return 0;
}

void channel_close(Channel *chan) {
    pthread_mutex_lock(&chan->lock);
    chan->closed = 1;
    pthread_cond_broadcast(&chan->not_empty);
    pthread_cond_broadcast(&chan->not_full);
    pthread_mutex_unlock(&chan->lock);
}

// ─── Thread Pool Implementation ───────────────────────────────────────────────
static void *thread_worker_func(void *arg);

ThreadPool *thread_pool_create(size_t num_workers) {
    ThreadPool *pool = (ThreadPool *)calloc(1, sizeof(ThreadPool));
    pool->num_workers = num_workers;
    pthread_mutex_init(&pool->queue_lock, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    
    // Spawn worker threads
    ThreadWorker *prev = NULL;
    for (size_t i = 0; i < num_workers; i++) {
        ThreadWorker *worker = (ThreadWorker *)calloc(1, sizeof(ThreadWorker));
        worker->active = 1;
        pthread_create(&worker->thread, NULL, thread_worker_func, pool);
        
        if (prev) {
            prev->next = worker;
        } else {
            pool->workers = worker;
        }
        prev = worker;
    }
    
    return pool;
}

void thread_pool_destroy(ThreadPool *pool) {
    if (!pool) return;
    
    // Signal shutdown
    pthread_mutex_lock(&pool->queue_lock);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_lock);
    
    // Wait for workers
    ThreadWorker *worker = pool->workers;
    while (worker) {
        pthread_join(worker->thread, NULL);
        ThreadWorker *next = worker->next;
        free(worker);
        worker = next;
    }
    
    // Clean up remaining tasks
    ThreadTask *task = pool->task_queue;
    while (task) {
        ThreadTask *next = task->next;
        // Set future to null if not already resolved
        if (task->future && !task->future->ready) {
            future_set(task->future, value_null());
        }
        free(task);
        task = next;
    }
    
    pthread_mutex_destroy(&pool->queue_lock);
    pthread_cond_destroy(&pool->queue_cond);
    free(pool);
}

static void *thread_worker_func(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    
    // Each thread needs its own interpreter instance for thread safety
    // For now, we'll create a minimal interpreter per thread
    Interpreter *thread_interp = interpreter_create();
    
    while (1) {
        pthread_mutex_lock(&pool->queue_lock);
        
        // Wait for task or shutdown
        while (!pool->task_queue && !pool->shutdown)
            pthread_cond_wait(&pool->queue_cond, &pool->queue_lock);
        
        if (pool->shutdown && !pool->task_queue) {
            pthread_mutex_unlock(&pool->queue_lock);
            break;
        }
        
        // Dequeue task
        ThreadTask *task = pool->task_queue;
        pool->task_queue = task->next;
        if (!pool->task_queue)
            pool->task_queue_tail = NULL;
        
        pthread_mutex_unlock(&pool->queue_lock);
        
        // Execute task
        FnDef *fn = task->fn;
        Value **args = task->args;
        size_t argc = task->argc;
        
        Value *result = value_null();  // default result
        
        if (fn && fn->body) {
            // Create function environment
            Environment *fn_env = env_create(fn->closure);
            
            // Bind arguments to parameters
            for (size_t i = 0; i < argc && i < fn->param_count; i++) {
                env_set(fn_env, fn->params[i].name, args[i]);
            }
            
            // Execute function body
            result = eval(thread_interp, fn->body, fn_env);
            
            // Unwrap return sentinel
            if (result && result->type == VAL_RETURN) {
                Value *inner = result->inner;
                result->inner = NULL;
                value_destroy(result);
                result = inner;
            }
            
            env_destroy(fn_env);
        }
        
        // Resolve the future with the result
        future_set(task->future, result);
        
        // Clean up task (but not args - they're still referenced)
        free(task);
    }
    
    interpreter_destroy(thread_interp);
    return NULL;
}

Future *thread_pool_submit(ThreadPool *pool, FnDef *fn, Value **args, size_t argc) {
    // Create future first
    Future *fut = future_create();
    
    ThreadTask *task = (ThreadTask *)calloc(1, sizeof(ThreadTask));
    task->fn = fn;
    task->args = args;
    task->argc = argc;
    task->future = fut;  // link the future to this task
    
    // Enqueue task
    pthread_mutex_lock(&pool->queue_lock);
    if (pool->task_queue_tail) {
        pool->task_queue_tail->next = task;
        pool->task_queue_tail = task;
    } else {
        pool->task_queue = pool->task_queue_tail = task;
    }
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_lock);
    
    return fut;
}

// ─── Process Pool Implementation ──────────────────────────────────────────────
ProcessPool *process_pool_create(size_t num_workers) {
    ProcessPool *pool = (ProcessPool *)calloc(1, sizeof(ProcessPool));
    pool->num_workers = num_workers;
    pthread_mutex_init(&pool->lock, NULL);
    
    ProcessWorker *prev = NULL;
    for (size_t i = 0; i < num_workers; i++) {
        ProcessWorker *worker = (ProcessWorker *)calloc(1, sizeof(ProcessWorker));
        
        // Create pipes
        if (pipe(worker->pipe_in) < 0 || pipe(worker->pipe_out) < 0) {
            perror("pipe");
            free(worker);
            continue;
        }
        
        // Fork worker process
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(worker->pipe_in[0]);
            close(worker->pipe_in[1]);
            close(worker->pipe_out[0]);
            close(worker->pipe_out[1]);
            free(worker);
            continue;
        }
        
        if (pid == 0) {
            // Child process: close unused pipe ends
            close(worker->pipe_in[1]);   // close write end of input
            close(worker->pipe_out[0]);  // close read end of output
            
            // Worker loop: read tasks, execute, write results
            while (1) {
                // TODO: Implement worker protocol
                // For now, just sleep
                sleep(1);
            }
            exit(0);
        }
        
        // Parent: close unused pipe ends
        close(worker->pipe_in[0]);   // close read end of input
        close(worker->pipe_out[1]);  // close write end of output
        
        worker->pid = pid;
        
        if (prev) {
            prev->next = worker;
        } else {
            pool->workers = worker;
        }
        prev = worker;
    }
    
    return pool;
}

void process_pool_destroy(ProcessPool *pool) {
    if (!pool) return;
    
    ProcessWorker *worker = pool->workers;
    while (worker) {
        // Close pipes
        close(worker->pipe_in[1]);
        close(worker->pipe_out[0]);
        
        // Kill worker process
        kill(worker->pid, SIGTERM);
        waitpid(worker->pid, NULL, 0);
        
        ProcessWorker *next = worker->next;
        free(worker);
        worker = next;
    }
    
    pthread_mutex_destroy(&pool->lock);
    free(pool);
}

Future *process_pool_submit(ProcessPool *pool, FnDef *fn, Value **args, size_t argc) {
    (void)pool; (void)fn; (void)args; (void)argc;
    // TODO: Serialize task to worker, deserialize result
    Future *fut = future_create();
    future_set(fut, value_null());  // placeholder
    return fut;
}
