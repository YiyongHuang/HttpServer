#pragma once
#include <cstdlib>
#include <string>


ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &buff);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &buff);

void handleSigpipe();
int setNonBlocking(int fd);
int socketBindListen(int port);
