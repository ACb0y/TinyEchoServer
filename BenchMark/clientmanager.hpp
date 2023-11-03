#pragma once

#include <string>

#include "../timer.h"
#include "client.hpp"

namespace BenchMark {
class ClientManager {
 public:
  ClientManager(const std::string& ip, int64_t port, int epoll_fd, Timer* timer, int count, bool is_rand_req_cnt)
      : ip_(ip), port_(port), epoll_fd_(epoll_fd), timer_(timer), count_(count), is_rand_req_cnt_(is_rand_req_cnt) {
    clients_ = new EchoClient*[count];
    for (int i = 0; i < count; i++) {
      clients_[i] = newClient();
      clients_[i]->Connecting(ip_, port_);
    }
  }
  ~ClientManager() {
    for (int i = 0; i < count_; i++) {
      if (clients_[i]) delete clients_[i];
    }
    delete[] clients_;
  }
  Timer* GetTimer() { return timer_; }
  void CheckStatus() {  // 检查客户端连接状态，并模拟了客户端的随机关闭和随机创建
    for (int i = 0; i < count_; i++) {
      if (clients_[i]->IsValid()) {
        continue;
      }
      delete clients_[i];
      clients_[i] = newClient();
      clients_[i]->Connecting(ip_, port_);
    }
  }

 private:
  EchoClient* newClient() {
    if (is_rand_req_cnt_) {
      return new EchoClient(epoll_fd_, 0);
    }
    return new EchoClient(epoll_fd_);
  }

 private:
  std::string ip_;  // 要压测服务监听的ip
  int64_t port_;  // 要压测服务监听的端口
  int epoll_fd_;  // epoll_fd
  Timer* timer_;  // 定时器
  EchoClient** clients_;  // 客户端连接池
  int count_;  // 客户端连接池大小
  bool is_rand_req_count_;  // 是否随机请求次数
};
}  // namespace BenchMark