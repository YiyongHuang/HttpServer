#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include "thread_pool.h"
#include "http_request.h"
#include "utils.h"

#define MAXEVENTS 1024  // 监听的最大事件


class HttpRequest;

class Epoll
{
private:
    static int epoll_fd_;
    static struct epoll_event *events_;
    static std::unordered_map<int, std::shared_ptr<HttpRequest>> fd2req;

public:
    static void epoll_init(int flags);
    static int epoll_add(int fd, std::shared_ptr<HttpRequest> request, int events);
    static int epoll_mod(int fd, std::shared_ptr<HttpRequest> request, int events);
    static int epoll_del(int fd, int events);
    static int eopll_wait_(int listen_fd, int max_events, int timeout);
    static std::vector<std::shared_ptr<HttpRequest>> getEventsRequest(int listen_fd, int events_num);
    static void acceptConnection(int listen_fd);
    static void handle_events(int listen_fd, int events_count);
};

