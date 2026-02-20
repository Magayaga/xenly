#ifndef MULTIPROC_H
#define MULTIPROC_H

#include "interpreter.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

// Forward declarations
struct Interpreter;
struct Value;
typedef struct Future Future;

// ─── Process Pool ─────────────────────────────────────────────────────────────
typedef struct ProcessWorker {
    pid_t pid;              // worker process ID
    int pipe_in[2];         // pipe for sending tasks TO worker
    int pipe_out[2];        // pipe for receiving results FROM worker
    int busy;               // 1 if worker is processing a task
    struct ProcessWorker *next;
} ProcessWorker;

typedef struct ProcessPool {
    size_t num_workers;
    ProcessWorker *workers;
    pthread_mutex_t lock;
} ProcessPool;

// ─── Thread Pool ──────────────────────────────────────────────────────────────
typedef struct ThreadTask {
    FnDef *fn;
    Value **args;
    size_t argc;
    Future *future;         // the future that will be resolved when task completes
    struct ThreadTask *next;
} ThreadTask;

typedef struct ThreadWorker {
    pthread_t thread;
    int active;
    struct ThreadWorker *next;
} ThreadWorker;

typedef struct ThreadPool {
    size_t num_workers;
    ThreadWorker *workers;
    ThreadTask *task_queue;
    ThreadTask *task_queue_tail;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
    int shutdown;
} ThreadPool;

// ─── Channel (for inter-task communication) ───────────────────────────────────
typedef struct ChannelMessage {
    Value *data;
    struct ChannelMessage *next;
} ChannelMessage;

typedef struct Channel {
    ChannelMessage *queue;
    ChannelMessage *queue_tail;
    size_t capacity;        // 0 = unbuffered
    size_t size;            // current items in queue
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int closed;
} Channel;

// ─── Future/Promise ───────────────────────────────────────────────────────────
typedef struct Future {
    Value *result;
    int ready;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Future;

// ─── API ──────────────────────────────────────────────────────────────────────
// Process pool
ProcessPool *process_pool_create(size_t num_workers);
void process_pool_destroy(ProcessPool *pool);
Future *process_pool_submit(ProcessPool *pool, FnDef *fn, Value **args, size_t argc);

// Thread pool
ThreadPool *thread_pool_create(size_t num_workers);
void thread_pool_destroy(ThreadPool *pool);
Future *thread_pool_submit(ThreadPool *pool, FnDef *fn, Value **args, size_t argc);

// Channel
Channel *channel_create(size_t capacity);
void channel_destroy(Channel *chan);
int channel_send(Channel *chan, Value *data);      // returns 0 on success
Value *channel_recv(Channel *chan);                // blocks until data available
int channel_try_recv(Channel *chan, Value **out);  // non-blocking
void channel_close(Channel *chan);

// Future
Future *future_create(void);
void future_destroy(Future *fut);
void future_set(Future *fut, Value *result);
Value *future_get(Future *fut);                    // blocks until ready
int future_is_ready(Future *fut);

// Built-in registration
void register_multiproc_builtins(struct Interpreter *interp);

#endif // MULTIPROC_H
