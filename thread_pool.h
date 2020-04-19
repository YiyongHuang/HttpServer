#pragma once
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include "http_request.h"
#include "epoll.h"

#define THREADPOOL_SIGNAL_FAILURE -1
#define THREADPOOL_LOCK_FAILURE -2
#define THREADPOOL_QUEUE_FULL -3
#define THREADPOOL_SHUTDOWN -4
#define THREADPOOL_THREAD_FAILURE -5

#define MAX_THREADS 1024
#define MAX_QUEUE 65535

typedef enum
{
    IMMEDIATE_SHUTDOWN = 1,
    GRACEFUL_SHUTDOWN = 2
} ShutDownOption;

struct ThreadPoolTask
{
    std::function<void(std::shared_ptr<void>)> fun;
    std::shared_ptr<void> args;
};


class ThreadPool
{
private:
    static pthread_mutex_t lock_;
    static pthread_cond_t notify_;

    static std::vector<pthread_t> threads_;
    static std::vector<ThreadPoolTask> queue_;
    static int thread_count_;
    static int queue_size_;
    static int head_;
    static int tail_;
    static int count_; // 任务队列当前任务数量
    static int shutdown_;
    static int started_;

public:
    static int threadpoolCreate(int thread_count, int queue_size);
    static int threadpoolAdd(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun);
    static int threadpoolDestroy(ShutDownOption shutdown_option = GRACEFUL_SHUTDOWN);
    static int threadpoolFree();
    static void *threadpoolWorker(void *args);
};
