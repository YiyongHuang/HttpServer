#include "timer.h"

pthread_mutex_t TimerMananger::timerLock_ = PTHREAD_MUTEX_INITIALIZER;
std::priority_queue<std::shared_ptr<TimerNode>, std::deque<std::shared_ptr<TimerNode>>, cmpExpTime> TimerMananger::timerQueue_;

TimerNode::TimerNode(std::shared_ptr<HttpRequest> request, int timeout)
    : deleted_(false),
      request_(request)

{
  struct timeval now;
  gettimeofday(&now, NULL);
  expiredTime_ = ((now.tv_sec % 10000) * 1000 + now.tv_usec / 1000) + timeout;
}

bool TimerNode::isValid()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  size_t tmp = ((now.tv_sec % 10000) * 1000 + now.tv_usec / 1000);
  if (tmp < expiredTime_)
    return true;
  else
  {
    this->setDeleted();
    return false;
  }
}

void TimerNode::seperateRequest()
{
  request_.reset();
  this->setDeleted();
}

TimerNode::~TimerNode()
{
  // std::cout << "~timernode" << std::endl;
  if (request_)
  {
    Epoll::epollDel(request_->getFd(), (EPOLLIN | EPOLLET | EPOLLONESHOT));
  }
}

void TimerMananger::addTimer(std::shared_ptr<HttpRequest> request, int timeout)
{
  std::shared_ptr<TimerNode> new_timer(new TimerNode(request, timeout));
  request->setTimer(new_timer);
  pthread_mutex_lock(&timerLock_);
  timerQueue_.push(new_timer);
  pthread_mutex_unlock(&timerLock_);
}

// 删除被分离的和超时计时器
void TimerMananger::handleExpiredEvents()
{
  pthread_mutex_lock(&timerLock_);
  // std::cout << "timerqueue size = " << timerQueue_.size() << std::endl;
  while (!timerQueue_.empty())
  {
    std::shared_ptr<TimerNode> tmp = timerQueue_.top();
    if (tmp->isDeleted())
      timerQueue_.pop();
    else if (!tmp->isValid())
      timerQueue_.pop();
    else
      break;
  }
  pthread_mutex_unlock(&timerLock_);
}
