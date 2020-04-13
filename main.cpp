#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <iostream>
#include "utils.h"
#include "epoll.h"
#include "http_request.h"
#include "thread_pool.h"

#define QUEUE_SIZE 1024 // 线程池任务队列大小

int main(int argc, char *argv[])
{
    int port = 8080;
    int threadNum = 4;
    const char *optstr = "p:t:";
    int opt;
    while ((opt = getopt(argc, argv, optstr)) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = atoi(optarg);
            break;
        case 't':
            threadNum = atoi(optarg);
            break;
        default:
            break;
        }
    }
    handleSigpipe();
    Epoll::epoll_init(0);
    HttpRequest::http_request_init();
    if (ThreadPool::threadpool_create(threadNum, QUEUE_SIZE) < 0)
    {
        printf("Threadpool create failed\n");
        return 1;
    }
    int listen_fd = socketBindListen(port);
    if (listen_fd < 0)
        return 1;
    if (setNonBlocking(listen_fd) < 0)
    {
        perror("set socket non block failed");
        return 1;
    }
    std::shared_ptr<HttpRequest> request(new HttpRequest(listen_fd));
    if (Epoll::epoll_add(listen_fd, request, EPOLLIN | EPOLLET) < 0)
    {
        return 1;
    }
    while (true)
    {
        int event_count = Epoll::eopll_wait_(listen_fd, MAXEVENTS, -1);
        // std::cout<<event_count;
        if (event_count > 0)
            Epoll::handle_events(listen_fd, event_count);
    }
    return 0;
}
