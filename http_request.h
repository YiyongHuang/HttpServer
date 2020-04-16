#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <map>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "utils.h"
#include "epoll.h"
#include "timer.h"

#define MAX_READ_TIMES 100

enum URIState
{
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS,
};

enum HeaderState
{
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};

enum ResponseState
{
    RESPONSE_SUCCESS = 1,
    RESPONSE_ERROR
};

enum ParseState
{
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACES_AFTER_COLON,
    H_VALUE,
    H_CR,
    H_LF,
    H_END_CR,
    H_END_LF
};

enum HttpMethod
{
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};

enum HttpVersion
{
    HTTP_10 = 1,
    HTTP_11,
    HTTP_20
};

class TimerNode;

class HttpRequest : public std::enable_shared_from_this<HttpRequest>
{
private:
    int fd_;
    int method_;
    size_t readTimes_;   // 读取不到数据时重复读取的次数，肯数据未到达或请求终止
    HttpVersion HttpVersion_;
    bool keep_alive_;
    std::map<std::string, std::string> headers_;
    std::string fileName_;
    std::string inBuffer_;
    std::string outBuffer_;
    std::weak_ptr<TimerNode> timer_;
    static std::unordered_map<std::string, std::string> mime_;

public:
    static void httpRequestInit();
    HttpRequest(int listen_fd);
    URIState parseURI();
    HeaderState parseHeader();
    ResponseState responseRequest();
    void resetRequest();
    void handleRequest();
    void sepereteTimer();
    void handleError(int error_no, std::string msg);
    void setTimer(std::shared_ptr<TimerNode> timer) { timer_ = timer; }
    int getFd() { return fd_; }
    ~HttpRequest();
};
