#include "epoll.h"

int Epoll::epoll_fd_;
epoll_event *Epoll::events_;
std::unordered_map<int, std::shared_ptr<HttpRequest>> Epoll::fd2req;

void Epoll::epoll_init(int flags)
{
    epoll_fd_ = epoll_create1(flags);
    if (epoll_fd_ == -1)
    {
        perror("epoll create");
        abort();
    }
    events_ = new epoll_event[MAXEVENTS];
}

int Epoll::epoll_add(int fd, std::shared_ptr<HttpRequest> request, int events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1)
    {
        perror("epoll add error");
        return -1;
    }
    fd2req[fd] = request;
    return 0;
}
int Epoll::epoll_mod(int fd, std::shared_ptr<HttpRequest> request, int events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event);
    if (ret == -1)
    {
        perror("epoll mod error");
        return -1;
    }
    fd2req[fd] = request;
    return 0;
}
int Epoll::epoll_del(int fd, int events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);
    if (ret == -1)
    {
        perror("epoll del error");
        return -1;
    }
    auto fd_iter = fd2req.find(fd);
    if (fd_iter != fd2req.end())
        fd2req.erase(fd_iter);
    return 0;
}

int Epoll::eopll_wait_(int listen_fd, int max_events, int timeout)
{
    int events_count = epoll_wait(epoll_fd_, events_, max_events, timeout);
    if (events_count < 0)
        perror("epoll wait error");
    return events_count;
}

std::vector<std::shared_ptr<HttpRequest>> Epoll::getEventsRequest(int listen_fd, int events_count)
{
    std::vector<std::shared_ptr<HttpRequest>> request;
    for (int i = 0; i < events_count; ++i)
    {
        int fd = events_[i].data.fd;
        if (fd < 3)
        {
            break;
        }
        else if (fd == listen_fd)
        {
            acceptConnection(listen_fd);
        }
        else
        {
            // 排除错误事件
            if ((events_[i].events & EPOLLERR) || (events_[i].events & EPOLLHUP) || (!(events_[i].events & EPOLLIN)))
            {
                auto fd_iter = fd2req.find(fd);
                if (fd_iter != fd2req.end())
                    fd2req.erase(fd_iter);
                continue;
            }
            std::shared_ptr<HttpRequest> cur_req(fd2req[fd]);
            request.push_back(cur_req);
            auto fd_ite = fd2req.find(fd);
            if (fd_ite != fd2req.end())
                fd2req.erase(fd_ite);
        }
    }
    return request;
}

void Epoll::acceptConnection(int listen_fd)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    int accept_fd = 0;
    socklen_t client_addr_len = 0;
    while ((accept_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len)) > 0)
    {
        std::cout<<"connected ["<<inet_ntoa(client_addr.sin_addr)<<"]: "<<ntohs(client_addr.sin_port)<<std::endl;

        int ret = setNonBlocking(accept_fd);
        if(ret<0) return;
        std::shared_ptr<HttpRequest> new_req(new HttpRequest(accept_fd));
        epoll_add(accept_fd, new_req, (EPOLLIN | EPOLLET | EPOLLONESHOT));
    }
}

void Epoll::handle_events(int listen_fd, int events_count)
{
    std::vector<std::shared_ptr<HttpRequest>> request = getEventsRequest(listen_fd, events_count);
    if (request.size() > 0)
    {
        for (auto &req : request)
        {
            // 线程池满了或者其他原因，抛弃本次监听到的请求。
            if (ThreadPool::threadpool_add(req) < 0)
            {
                break;
            }
        }
    }
}

