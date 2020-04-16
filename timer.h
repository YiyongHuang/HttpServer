#pragma once
#include <sys/time.h>
#include <queue>
#include "http_request.h"

class HttpRequest;

// 计时器节点
class TimerNode
{
private:
    size_t expiredTime_;
    bool deleted_;
    std::shared_ptr<HttpRequest> request_;

public:
    TimerNode(std::shared_ptr<HttpRequest> request, int timeout);
    size_t getExpTime() { return expiredTime_; }
    bool isValid();
    void seperateRequest();
    bool isDeleted() { return deleted_; }
    void setDeleted() { deleted_ = true; }
    ~TimerNode();
};

// 仿函数，用于实现小根堆的优先级队列
struct cmpExpTime
{
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) { return a->getExpTime() > b->getExpTime(); }
};

// 计时器管理器
class TimerMananger
{
private:
    static pthread_mutex_t timerLock_;
    static std::priority_queue<std::shared_ptr<TimerNode>, std::deque<std::shared_ptr<TimerNode>>, cmpExpTime> timerQueue_;

public:
    static void addTimer(std::shared_ptr<HttpRequest> request, int timeout);
    static void handleExpiredEvents();
};
