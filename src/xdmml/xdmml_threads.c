/*
 * xdmml_threads.c - Cross-platform Threading Implementation
 */

#include "xdmml_extended.h"
#include <stdlib.h>
#include <string.h>

#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    #include <pthread.h>
    #include <errno.h>
    #include <time.h>
    #define XDMML_HAS_PTHREADS 1
#endif

// ─── Thread Structure ────────────────────────────────────────────────────────

struct XDMML_Thread {
#ifdef XDMML_HAS_PTHREADS
    pthread_t handle;
#endif
    XDMML_ThreadFunc func;
    void* data;
    char* name;
    int return_value;
};

struct XDMML_Mutex {
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_t handle;
#endif
};

struct XDMML_Condition {
#ifdef XDMML_HAS_PTHREADS
    pthread_cond_t handle;
#endif
};

struct XDMML_Semaphore {
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint32_t value;
#endif
};

// ─── Thread Functions ────────────────────────────────────────────────────────

#ifdef XDMML_HAS_PTHREADS
static void* thread_wrapper(void* arg) {
    XDMML_Thread* thread = (XDMML_Thread*)arg;
    thread->return_value = thread->func(thread->data);
    return NULL;
}
#endif

XDMML_Thread* xdmml_create_thread(XDMML_ThreadFunc fn, const char* name, void* data) {
    if (!fn) return NULL;
    
    XDMML_Thread* thread = (XDMML_Thread*)calloc(1, sizeof(XDMML_Thread));
    if (!thread) return NULL;
    
    thread->func = fn;
    thread->data = data;
    thread->name = name ? strdup(name) : NULL;
    
#ifdef XDMML_HAS_PTHREADS
    if (pthread_create(&thread->handle, NULL, thread_wrapper, thread) != 0) {
        free(thread->name);
        free(thread);
        return NULL;
    }
#endif
    
    return thread;
}

void xdmml_wait_thread(XDMML_Thread* thread, int* status) {
    if (!thread) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_join(thread->handle, NULL);
#endif
    
    if (status) {
        *status = thread->return_value;
    }
    
    free(thread->name);
    free(thread);
}

void xdmml_detach_thread(XDMML_Thread* thread) {
    if (!thread) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_detach(thread->handle);
#endif
    
    free(thread->name);
    free(thread);
}

uint64_t xdmml_thread_id(void) {
#ifdef XDMML_HAS_PTHREADS
    return (uint64_t)pthread_self();
#else
    return 0;
#endif
}

// ─── Mutex Functions ─────────────────────────────────────────────────────────

XDMML_Mutex* xdmml_create_mutex(void) {
    XDMML_Mutex* mutex = (XDMML_Mutex*)malloc(sizeof(XDMML_Mutex));
    if (!mutex) return NULL;
    
#ifdef XDMML_HAS_PTHREADS
    if (pthread_mutex_init(&mutex->handle, NULL) != 0) {
        free(mutex);
        return NULL;
    }
#endif
    
    return mutex;
}

void xdmml_destroy_mutex(XDMML_Mutex* mutex) {
    if (!mutex) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_destroy(&mutex->handle);
#endif
    
    free(mutex);
}

void xdmml_lock_mutex(XDMML_Mutex* mutex) {
    if (!mutex) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_lock(&mutex->handle);
#endif
}

bool xdmml_try_lock_mutex(XDMML_Mutex* mutex) {
    if (!mutex) return false;
    
#ifdef XDMML_HAS_PTHREADS
    return pthread_mutex_trylock(&mutex->handle) == 0;
#else
    return false;
#endif
}

void xdmml_unlock_mutex(XDMML_Mutex* mutex) {
    if (!mutex) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_unlock(&mutex->handle);
#endif
}

// ─── Condition Variable Functions ────────────────────────────────────────────

XDMML_Condition* xdmml_create_condition(void) {
    XDMML_Condition* cond = (XDMML_Condition*)malloc(sizeof(XDMML_Condition));
    if (!cond) return NULL;
    
#ifdef XDMML_HAS_PTHREADS
    if (pthread_cond_init(&cond->handle, NULL) != 0) {
        free(cond);
        return NULL;
    }
#endif
    
    return cond;
}

void xdmml_destroy_condition(XDMML_Condition* cond) {
    if (!cond) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_cond_destroy(&cond->handle);
#endif
    
    free(cond);
}

void xdmml_wait_condition(XDMML_Condition* cond, XDMML_Mutex* mutex) {
    if (!cond || !mutex) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_cond_wait(&cond->handle, &mutex->handle);
#endif
}

bool xdmml_wait_condition_timeout(XDMML_Condition* cond, XDMML_Mutex* mutex, uint32_t ms) {
    if (!cond || !mutex) return false;
    
#ifdef XDMML_HAS_PTHREADS
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    
    int result = pthread_cond_timedwait(&cond->handle, &mutex->handle, &ts);
    return (result == 0);
#else
    return false;
#endif
}

void xdmml_signal_condition(XDMML_Condition* cond) {
    if (!cond) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_cond_signal(&cond->handle);
#endif
}

void xdmml_broadcast_condition(XDMML_Condition* cond) {
    if (!cond) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_cond_broadcast(&cond->handle);
#endif
}

// ─── Semaphore Functions ─────────────────────────────────────────────────────

XDMML_Semaphore* xdmml_create_semaphore(uint32_t initial_value) {
    XDMML_Semaphore* sem = (XDMML_Semaphore*)malloc(sizeof(XDMML_Semaphore));
    if (!sem) return NULL;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->cond, NULL);
    sem->value = initial_value;
#endif
    
    return sem;
}

void xdmml_destroy_semaphore(XDMML_Semaphore* sem) {
    if (!sem) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
#endif
    
    free(sem);
}

void xdmml_wait_semaphore(XDMML_Semaphore* sem) {
    if (!sem) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_lock(&sem->mutex);
    while (sem->value == 0) {
        pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    sem->value--;
    pthread_mutex_unlock(&sem->mutex);
#endif
}

bool xdmml_try_wait_semaphore(XDMML_Semaphore* sem) {
    if (!sem) return false;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_lock(&sem->mutex);
    bool success = (sem->value > 0);
    if (success) {
        sem->value--;
    }
    pthread_mutex_unlock(&sem->mutex);
    return success;
#else
    return false;
#endif
}

void xdmml_post_semaphore(XDMML_Semaphore* sem) {
    if (!sem) return;
    
#ifdef XDMML_HAS_PTHREADS
    pthread_mutex_lock(&sem->mutex);
    sem->value++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
#endif
}
