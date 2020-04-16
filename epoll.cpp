#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include "thread_pool.h"
#include "http_request.h"
#include "utils.h"
#include "timer.h"

#define MAX_EVENTS 1024   // 监听的最大事件
#define TIMER_TIMEOUT 500 // 计时器最大等待时间 (ms)

class HttpRequest;

void myHandler(std::shared_ptr<void> request);

class Epoll
{
private:
    static int epoll_fd_;
    static struct epoll_event *events_;
    static std::unordered_map<int, std::shared_ptr<HttpRequest>> fd2req_;

public:
    static int epollInit(int flags);
    static int epollAdd(int fd, std::shared_ptr<HttpRequest> request, int events);
    static int epollMod(int fd, std::shared_ptr<HttpRequest> request, int events);
    static int epollDel(int fd, int events);
    static int eopllWait(int listen_fd, int max_events, int timeout);
    static std::vector<std::shared_ptr<HttpRequest>> getEventsRequest(int listen_fd, int events_num);
    static void acceptConnection(int listen_fd);
    static void handleEvents(int listen_fd, int events_count);
};
