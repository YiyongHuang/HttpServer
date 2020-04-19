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
    // 设置SIGPIPE信号默认处理函数为SIG_IGN，避免客户端意外断开连接时内核发给程序的SIGPIPE信号使程序关闭
    // SIGPIPE默认处理为退出进程
    handleSigpipe();
    // 初始化HTTP文件类型
    HttpRequest::httpRequestInit();
    if (Epoll::epollInit(0) < 0)
    {
        std::cout << "Epoll create failed" << std::endl;
        return 1;
    }
    if (ThreadPool::threadpoolCreate(threadNum, QUEUE_SIZE) < 0)
    {
        std::cout << "Threadpool create failed" << std::endl;
        return 1;
    }
    int listen_fd = socketBindListen(port);
    if (listen_fd < 0)
        return 1;
    if (setNonBlocking(listen_fd) < 0)
    {
        std::cout << "set socket non block failed" << std::endl;
        return 1;
    }
    std::shared_ptr<HttpRequest> request(new HttpRequest(listen_fd));
    if (Epoll::epollAdd(listen_fd, request, EPOLLIN | EPOLLET) < 0)
    {
        return 1;
    }
    while (true)
    {
        int event_count = Epoll::eopllWait(listen_fd, MAX_EVENTS, -1);
        // std::cout<<event_count;
        if (event_count > 0)
            Epoll::handleEvents(listen_fd, event_count);
        TimerManager::handleExpiredEvents();
    }
    ThreadPool::threadpoolDestroy();
    return 0;
}
