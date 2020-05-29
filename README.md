# C++ Http WebServer

## Introduction
使用C++11写的Http服务器，采用Reactor模式，可解析GET、HEAD请求，支持长连接(keep-alive)操作。

## Requirements
* Linux (kernel 2.6+)
* g++ 4.8

## Usage
```
make
./myserver -p port -t threadnum
```
## Features
* 采用Reactor模式，主线程只负责接收请求建立连接，使用线程池处理请求和执行I/O操作
* 采用I/O多路复用EPOLL边缘触发、非阻塞I/O
* 使用线程池处理请求，提高并发量并避免频繁创建和销毁线程开销
* 采用STL中基于小根堆的优先级队列存储计时器，计时器用于关闭超时请求，计时器处理完事件后统一删除，实现惰性删除
* 使用智能指针管理内存避免内存泄漏

## 压测工具
[WebBench](https://github.com/EZLippi/WebBench)
