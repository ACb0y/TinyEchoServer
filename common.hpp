#pragma once

#include <assert.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/un.h>

#include <functional>
#include <iostream>

using namespace std;

#include "codec.hpp"

namespace TinyEcho {
// 获取系统有多少个可用的cpu
inline int GetNProcs() { return get_nprocs(); }

// 用于阻塞IO模式下发送应答消息
inline bool SendMsg(int fd, const std::string message) {
  Packet pkt;
  Codec codec;
  codec.EnCode(message, pkt);
  size_t sendLen = 0;
  while (sendLen != pkt.UseLen()) {
    ssize_t ret = write(fd, pkt.Data() + sendLen, pkt.UseLen() - sendLen);
    if (ret < 0) {
      if (errno == EINTR) continue;  // 中断的情况可以重试
      perror("write failed");
      return false;
    }
    sendLen += ret;
  }
  return true;
}

// 用于阻塞IO模式下接收请求消息
inline bool RecvMsg(int fd, std::string &message) {
  Codec codec;
  std::string *temp = nullptr;
  while ((temp = codec.GetMessage()) == nullptr) {  // 只要还没获取到一个完整的消息，则一直循环
    ssize_t ret = read(fd, codec.Data(), codec.Len());
    if (0 == ret) {
      cout << "peer close connection" << endl;
      return false;
    }
    if (ret < 0) {
      if (errno == EINTR) continue;  // 中断的情况可以重试
      perror("read failed");
      return false;
    }
    codec.DeCode(ret);
  }
  message = *temp;
  delete temp;
  return true;
}

inline void SetNotBlock(int fd) {
  int oldOpt = fcntl(fd, F_GETFL);
  assert(oldOpt != -1);
  assert(fcntl(fd, F_SETFL, oldOpt | O_NONBLOCK) != -1);
}

inline void SetTimeOut(int fd, int64_t sec, int64_t usec) {
  struct timeval tv;
  tv.tv_sec = sec;  //秒
  tv.tv_usec = usec;  //微秒，1秒等于10的6次方微秒
  assert(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != -1);
  assert(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != -1);
}

inline int CreateListenUnixSocket(std::string path) {
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("socket failed");
    return -1;
  }
  if (bind(sock_fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
    perror("bind failed");
    return -1;
  }
  if (listen(sock_fd, 10240) != 0) {
    perror("listen failed");
    return -1;
  }
  return sock_fd;
}

inline int CreateListenSocket(std::string ip, int port, bool reuse_port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("socket failed");
    return -1;
  }
  int reuse = 1;
  int opt = SO_REUSEADDR;
  if (reuse_port) opt = SO_REUSEPORT;
  if (setsockopt(sock_fd, SOL_SOCKET, opt, &reuse, sizeof(reuse)) != 0) {
    perror("setsockopt failed");
    return -1;
  }
  if (bind(sock_fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
    perror("bind failed");
    return -1;
  }
  if (listen(sock_fd, 10240) != 0) {
    perror("listen failed");
    return -1;
  }
  return sock_fd;
}

// 调用本函数之前需要把sock_fd设置成非阻塞的
inline void LoopAccept(int sock_fd, int max_conn, std::function<void(int clientFd)> client_accept_call_back) {
  while (max_conn--) {
    int client_fd = accept(sock_fd, NULL, 0);
    if (client_fd > 0) {
      client_accept_call_back(client_fd);
      continue;
    }
    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
      perror("accept failed");
    }
    break;
  }
}
}  // namespace TinyEcho
