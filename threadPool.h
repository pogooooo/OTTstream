#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <windows.h>

typedef struct {
    unsigned(__stdcall* func)(void*);
    void* arg;
} Task;

typedef struct {
    HANDLE* threads;
    int thread_count;

    Task* task_queue;
    int queue_size;
    int head;
    int tail;
    int count;

    CRITICAL_SECTION lock;
    CONDITION_VARIABLE cond;
    int shutdown;
} ThreadPool;

void thread_pool_init(ThreadPool* pool, int thread_count, int queue_size);
void thread_pool_add_task(ThreadPool* pool, unsigned(__stdcall* func)(void*), void* arg);
void thread_pool_shutdown(ThreadPool* pool);

#endif