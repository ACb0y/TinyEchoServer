#pragma once

#include <string>

#include "../percentile.hpp"
#include "../timer.h"
#include "client.hpp"

namespace BenchMark {
class ClientManager {
 public:
  ClientManager(const std::string& ip, int64_t port, int epoll_fd, Timer* timer, int count, const std::string& message,
                bool is_rand_req_count, bool is_debug)
      : ip_(ip),
        port_(port),
        epoll_fd_(epoll_fd),
        timer_(timer),
        count_(count),
        message_(message),
        is_rand_req_count_(is_rand_req_count),
        is_debug_(is_debug) {
    clients_ = new EchoClient*[count];
    for (int i = 0; i < count; i++) {
      clients_[i] = newClient(message, is_debug);
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
  void GetStatData() {
    for (int i = 0; i < count_; i++) {
      if (clients_[i]->IsValid()) {
        getDealStat(clients_[i]);
      }
    }
  }
  void PrintStatData() {
    cout << "success[" << success_count_ << "],failure[" << failure_count_ << "],connect_failure["
         << connect_failure_count_ << "],read_failure[" << read_failure_count_ << "],write_failure["
         << write_failure_count_ << "]" << endl;
    percentile_.PrintPctData();
    percentile_.PrintAvgPctData();
  }
  void CheckStatus(int32_t& create_client_count) {  // 检查客户端连接状态，并模拟了客户端的随机关闭和随机创建
    for (int i = 0; i < count_; i++) {
      if (clients_[i]->IsValid()) {
        continue;
      }
      getDealStat(clients_[i]);
      delete clients_[i];
      create_client_count++;
      clients_[i] = newClient(message_, is_debug_);
      clients_[i]->Connect(ip_, port_);
    }
    if (is_debug_) {
      cout << "create_client_count=" << create_client_count << endl;
    }
  }

 private:
  void getDealStat(EchoClient* client) {
    int64_t success_count{0};
    int64_t failure_count{0};
    int64_t connect_failure_count{0};
    int64_t read_failure_count{0};
    int64_t write_failure_count{0};
    client->GetDealStat(success_count, failure_count, connect_failure_count, read_failure_count, write_failure_count);
    assert(failure_count <= 1 && connect_failure_count <= 1 && read_failure_count <= 1 && write_failure_count <= 1);
    assert(failure_count == (connect_failure_count + read_failure_count + write_failure_count));
    success_count_ += success_count;
    failure_count_ += failure_count;
    connect_failure_count_ += connect_failure_count;
    read_failure_count_ += read_failure_count;
    write_failure_count_ += write_failure_count;
  }
  EchoClient* newClient(const std::string& message, bool is_debug) {
    EchoClient* client = nullptr;
    if (is_rand_req_count_) {
      client = new EchoClient(epoll_fd_, &percentile_, is_debug, 0);
    } else {
      client = new EchoClient(epoll_fd_, &percentile_, is_debug);
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
  bool is_debug_;  // 是否调试模式
  int64_t success_count_{0};  // 成功数统计
  int64_t failure_count_{0};  // 失败数统计
  int64_t connect_failure_count_{0};  // 细分失败类型统计，连接失败
  int64_t read_failure_count_{0};  // 细分失败类型统计，读失败
  int64_t write_failure_count_{0};  // 细分失败类型统计，写失败
  TinyEcho::Percentile percentile_;  // 用于统计请求耗时的pctxx数值
};
}  // namespace BenchMark