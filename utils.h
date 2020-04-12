#pragma once
#include <cstdlib>
#include <string>

#define PATHLEN 128


#define MIN(a, b) ((a) < (b) ? (a) : (b))

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &buff);
ssize_t readn(int fd, std::string &buff, bool &zero);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &buff);

void handleSigpipe();
int setNonBlocking(int fd);
void setNoDelay(int fd);
void setNoLinger(int fd);
void shutDownWR(int fd);
int socketBindListen(int port);
