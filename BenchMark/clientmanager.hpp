#pragma once

#include <string>

#include "../percentile.hpp"
#include "../timer.h"
#include "client.hpp"

namespace BenchMark {
class ClientManager {
 public:
  ClientManager(const std::string& ip, int64_t port, int epoll_fd, Timer* timer, int count, const std::string& message,
                bool is_rand_req_count)
      : ip_(ip),
        port_(port),
        epoll_fd_(epoll_fd),
        timer_(timer),
        count_(count),
        message_(message),
        is_rand_req_count_(is_rand_req_count) {
    clients_ = new EchoClient*[count];
    for (int i = 0; i < count; i++) {
      clients_[i] = newClient(message);
      clients_[i]->Connect(ip_, port_);
    }
  }
  ~ClientManager() {
    for (int i = 0; i < count_; i++) {
      if (clients_[i]) delete clients_[i];
    }
    delete[] clients_;
  }
  Timer* GetTimer() { return timer_; }
  void PrintStatData() {
    cout << "success_count[" << success_count_ << "],failure_count[" << failure_count_ << "]" << endl;
    percentile_.PrintPctData();
    percentile_.PrintAvgPctData();
  }
  void CheckStatus(int32_t& create_client_count) {  // 检查客户端连接状态，并模拟了客户端的随机关闭和随机创建
    for (int i = 0; i < count_; i++) {
      if (clients_[i]->IsValid()) {
        continue;
      }
      int64_t success_count{0};
      int64_t failure_count{0};
      // TODO 能区分不同的失败类型，connect超时，读写超时，还是读写失败
      clients_[i]->GetDealStat(success_count, failure_count);
      assert(failure_count <= 1);
      success_count_ += success_count;
      failure_count_ += failure_count;
      delete clients_[i];
      create_client_count++;
      clients_[i] = newClient(message_);
      clients_[i]->Connect(ip_, port_);
    }
  }

 private:
  EchoClient* newClient(const std::string& message) {
    EchoClient* client = nullptr;
    if (is_rand_req_count_) {
      client = new EchoClient(epoll_fd_, &percentile_, 0);
    } else {
      client = new EchoClient(epoll_fd_, &percentile_);
    }
    client->SetEchoMessage(message);
    return client;
  }

 private:
  std::string ip_;  // 要压测服务监听的ip
  int64_t port_;  // 要压测服务监听的端口
  int epoll_fd_;  // epoll_fd
  Timer* timer_;  // 定时器
  EchoClient** clients_;  // 客户端连接池
  int count_;  // 客户端连接池大小
  std::string message_;  // 要发送的消息
  bool is_rand_req_count_;  // 是否随机请求次数
  int64_t success_count_{0};  // 成功数统计
  int64_t failure_count_{0};  // 失败数统计
  TinyEcho::Percentile percentile_;  // 用于统计请求耗时的pctxx数值
};
}  // namespace BenchMark