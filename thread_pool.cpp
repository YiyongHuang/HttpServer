#include "thread_pool.h"

pthread_mutex_t ThreadPool::lock_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify_ = PTHREAD_COND_INITIALIZER;
std::vector<pthread_t> ThreadPool::threads_;
std::vector<ThreadPoolTask> ThreadPool::queue_;
int ThreadPool::thread_count_ = 0;
int ThreadPool::queue_size_ = 0;
int ThreadPool::head_ = 0;
int ThreadPool::tail_ = 0;
int ThreadPool::count_ = 0;
int ThreadPool::shutdown_ = 0;
int ThreadPool::started_ = 0;

// 创建线程池，初始化线程
int ThreadPool::threadpoolCreate(int thread_count, int queue_size)
{
    do
    {
        if (thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE)
        {
            thread_count = 4;
            queue_size = 1024;
        }
        queue_size_ = queue_size;
        threads_.resize(thread_count);
        queue_.resize(queue_size);

        // 创建线程
        for (int i = 0; i < thread_count; ++i)
        {
            if (pthread_create(&threads_[i], NULL, threadpoolWorker, (void *)(0)) != 0)
            {
                return -1;
            }
            ++thread_count_;
            ++started_;
        }
    } while (false);

    return 0;
}

int ThreadPool::threadpoolAdd(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun)
{
    int next, err = 0;
    if (pthread_mutex_lock(&lock_) != 0)
        return THREADPOOL_LOCK_FAILURE;
    do
    {
        next = (tail_ + 1) % queue_size_;
        // 队列满
        if (count_ == queue_size_)
        {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        // 已关闭
        if (shutdown_)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        queue_[tail_].fun = fun;
        queue_[tail_].args = args;
        tail_ = next;
        ++count_;

        if (pthread_cond_signal(&notify_) != 0)
        {
            err = THREADPOOL_SIGNAL_FAILURE;
            break;
        }
    } while (false);

    if (pthread_mutex_unlock(&lock_) != 0)
        err = THREADPOOL_LOCK_FAILURE;
    return err;
}

int ThreadPool::threadpoolDestroy(ShutDownOption shutdown_option)
{
    std::cout<<"Thread pool destroy !"<<std::endl;
    int i, err = 0;

    if (pthread_mutex_lock(&lock_) != 0)
    {
        return THREADPOOL_LOCK_FAILURE;
    }
    do
    {
        if (shutdown_)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        shutdown_ = shutdown_option;

        if ((pthread_cond_broadcast(&notify_) != 0) ||
            (pthread_mutex_unlock(&lock_) != 0))
        {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }

        for (i = 0; i < thread_count_; ++i)
        {
            if (pthread_join(threads_[i], NULL) != 0)
            {
                err = THREADPOOL_THREAD_FAILURE;
            }
        }
    } while (false);

    if (!err)
    {
        threadpoolFree();
    }
    return err;
}

int ThreadPool::threadpoolFree()
{
    if (started_ > 0)
        return -1;
    pthread_mutex_lock(&lock_);
    pthread_mutex_destroy(&lock_);
    pthread_cond_destroy(&notify_);
    return 0;
}

void *ThreadPool::threadpoolWorker(void *args)
{
    while (true)
    {
        ThreadPoolTask task;
        pthread_mutex_lock(&lock_);
        while ((count_ == 0) && (!shutdown_))
        {
            pthread_cond_wait(&notify_, &lock_);
        }
        if ((shutdown_ == IMMEDIATE_SHUTDOWN) ||
            ((shutdown_ == GRACEFUL_SHUTDOWN) && (count_ == 0)))
        {
            break;
        }
        task.fun = queue_[head_].fun;
        task.args = queue_[head_].args;
        queue_[head_].fun = NULL;
        queue_[head_].args.reset();
        head_ = (head_ + 1) % queue_size_;
        --count_;
        pthread_mutex_unlock(&lock_);
        (task.fun)(task.args);
    }
    --started_;
    pthread_mutex_unlock(&lock_);
    std::cout<<"This threadpool thread finishs!"<<std::endl;
    pthread_exit(NULL);
    return (NULL);
}
