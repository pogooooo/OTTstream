#include "threadPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h> // SleepConditionVariableCS, InitializeConditionVariable 등을 위해 필요

unsigned __stdcall worker_thread(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    Task task;

    while (1) {
        EnterCriticalSection(&pool->lock);

        while (pool->count == 0 && !pool->shutdown) {
            SleepConditionVariableCS(&pool->cond, &pool->lock, INFINITE);
        }

        if (pool->shutdown) {
            LeaveCriticalSection(&pool->lock);
            break;
        }

        task = pool->task_queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count--;

        LeaveCriticalSection(&pool->lock);

        if (task.func != NULL && task.arg != NULL) {
            task.func(task.arg);
        }
        else {
            // NULL task나 argument가 큐에 들어왔을 경우 (매우 희귀)
            printf("Error: Received NULL task or argument in worker thread.\n");
        }
    }
    return 0;
}

void thread_pool_init(ThreadPool* pool, int thread_count, int queue_size) {
    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->shutdown = 0;

    pool->task_queue = (Task*)malloc(sizeof(Task) * queue_size);
    pool->threads = (HANDLE*)malloc(sizeof(HANDLE) * thread_count);

    if (pool->task_queue == NULL || pool->threads == NULL) {
        perror("Failed to allocate memory for thread pool");
        exit(EXIT_FAILURE);
    }

    InitializeCriticalSection(&pool->lock);
    InitializeConditionVariable(&pool->cond);

    for (int i = 0; i < thread_count; i++) {
        pool->threads[i] = (HANDLE)_beginthreadex(NULL, 0, worker_thread, pool, 0, NULL);
    }
    printf("[ThreadPool] Created %d worker threads.\n", thread_count);
}

void thread_pool_add_task(ThreadPool* pool, unsigned(__stdcall* func)(void*), void* arg) {
    if (pool == NULL || pool->shutdown == 1) {
        printf("[ThreadPool] Error: Pool is NULL or shut down. Dropping arg.\n");
        if (arg != NULL) {
            free(arg);
        }
        return;
    }
    
    EnterCriticalSection(&pool->lock);

    if (pool->count == pool->queue_size) {
        printf("[ThreadPool] Queue is full! Dropping request.\n");
        if (arg != NULL) {
            free(arg);
        }
    }
    else {
        pool->task_queue[pool->tail].func = func;
        pool->task_queue[pool->tail].arg = arg;
        pool->tail = (pool->tail + 1) % pool->queue_size;
        pool->count++;

        WakeConditionVariable(&pool->cond);
    }

    LeaveCriticalSection(&pool->lock);
}

void thread_pool_shutdown(ThreadPool* pool) {
    EnterCriticalSection(&pool->lock);
    pool->shutdown = 1;
    WakeAllConditionVariable(&pool->cond);
    LeaveCriticalSection(&pool->lock);

    WaitForMultipleObjects(pool->thread_count, pool->threads, TRUE, INFINITE);

    free(pool->task_queue);
    free(pool->threads);
    DeleteCriticalSection(&pool->lock);
}