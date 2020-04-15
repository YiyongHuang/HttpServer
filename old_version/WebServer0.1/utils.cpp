#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

const int MAX_BUFF = 4096;
const int BACK_LOG = 2048;

ssize_t readn(int fd, void *buff, size_t n) {
  char *buf = static_cast<char *>(buff);
  ssize_t total_size = 0;
  size_t nleft = n;
  while (nleft > 0) {
    ssize_t nread = read(fd, buf + total_size, nleft);
    if (nread < 0) {
      if (errno == EINTR) {
        return total_size;
      } else if (errno == EAGAIN) {
        return total_size;
      } else {
        perror("read error");
        return -1;
      }
    } else if (nread == 0) {
      break;
    }
    total_size += nread;
    nleft -= nread;
  }
  //   buf[total_size] = 0;
  return total_size;
}

ssize_t readn(int fd, std::string &buff) {
  ssize_t nread = 0;
  ssize_t total_size = 0;
  while (true) {
    char buf[MAX_BUFF];
    if ((nread = read(fd, buf, MAX_BUFF)) < 0) {
      if (errno == EINTR)
        continue;
      else if (errno == EAGAIN) {
        return total_size;
      } else {
        perror("read error");
        return -1;
      }
    } else if (nread == 0) {
      break;
    }
    total_size += nread;
    buff += std::string(buf, buf + nread);
  }
  return total_size;
}

ssize_t writen(int fd, void *buff, size_t n) {
  size_t nleft = n;
  ssize_t nwritten = 0;
  ssize_t write_size = 0;
  char *buf = static_cast<char *>(buff);
  while (nleft > 0) {
    if ((nwritten = write(fd, buf + write_size, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
          continue;
        } else if (errno == EAGAIN) {
          return write_size;
        } else {
          perror("write error");
          return -1;
        }
      }
    }
    write_size += nwritten;
    nleft -= nwritten;
  }
  return write_size;
}

ssize_t writen(int fd, std::string &buff) {
  size_t nleft = buff.size();
  ssize_t nwritten = 0;
  ssize_t writeSum = 0;
  const char *buf = buff.c_str();
  while (nleft > 0) {
    if ((nwritten = write(fd, buf + writeSum, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
          continue;
        } else if (errno == EAGAIN)
          break;
        else {
          perror("write error");
          return -1;
        }
      }
    }
    writeSum += nwritten;
    nleft -= nwritten;
  }
  if (writeSum == static_cast<int>(buff.size()))
    buff.clear();
  else
    buff = buff.substr(writeSum);
  return writeSum;
}

void handleSigpipe() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL) < 0) perror("sigaction error");
}

int setNonBlocking(int fd) {
  int fl = fcntl(fd, F_GETFL);
  if (fl < 0) {
    perror("setNonBlock");
    return -1;
  }
  fcntl(fd, F_SETFL, fl | O_NONBLOCK);
  return 0;
}


int socketBindListen(int port) {
  if (port < 0 || port > 65535) {
    std::cout << "port error!" << std::endl;
    return -1;
  }
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    perror("socket error");
    close(listen_fd);
    return -1;
  }
  int optval = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval,
                 sizeof(optval)) == -1) {
    perror("set REUSEADDR error");
    close(listen_fd);
    return -1;
  }
  struct sockaddr_in addr_in;
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons((unsigned int)port);
  addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listen_fd, (sockaddr *)&addr_in, sizeof(addr_in)) == -1) {
    perror("bind error");
    close(listen_fd);
    return -1;
  }
  if (listen(listen_fd, BACK_LOG) == -1) {
    perror("listen error");
    close(listen_fd);
    return -1;
  }
  return listen_fd;
}


