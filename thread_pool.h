#pragma once
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include "http_request.h"
#include "epoll.h"

const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_SIGNAL_FAILURE = -6;
const int THREADPOOL_GRACEFUL = 1;

const int MAX_THREADS = 1024;
const int MAX_QUEUE = 65535;

typedef enum
{
    immediate_shutdown = 1,
    graceful_shutdown = 2
} ShutDownOption;

struct ThreadPoolTask
{
    std::function<void(std::shared_ptr<void>)> fun;
    std::shared_ptr<void> args;
};


class ThreadPool
{
private:
    static pthread_mutex_t lock;
    static pthread_cond_t notify;

    static std::vector<pthread_t> threads;
    static std::vector<ThreadPoolTask> queue;
    static int thread_count;
    static int queue_size;
    static int head;
    static int tail;
    static int count; // 任务队列当前任务数量
    static int shutdown;
    static int started;

public:
    static int threadpoolCreate(int _thread_count, int _queue_size);
    static int threadpoolAdd(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun);
    static int threadpoolDestroy(ShutDownOption shutdown_option = graceful_shutdown);
    static int threadpoolFree();
    static void *threadpoolWorker(void *args);
};
