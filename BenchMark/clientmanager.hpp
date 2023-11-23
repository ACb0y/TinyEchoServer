#pragma once

#include <mutex>
#include <string>

#include "../percentile.hpp"
#include "../timer.h"
#include "client.hpp"

namespace BenchMark {
class ClientManager {
 public:
  ClientManager(const std::string& ip, int64_t port, int epoll_fd, Timer* timer, int count, const std::string& message,
                int64_t max_req_count, bool is_debug, int64_t run_time, bool* is_running, int64_t rate_limit)
      : ip_(ip),
        port_(port),
        epoll_fd_(epoll_fd),
        timer_(timer),
        count_(count),
        message_(message),
        max_req_count_(max_req_count),
        is_debug_(is_debug),
        run_time_(run_time),
        is_running_(is_running),
        rate_limit_(rate_limit) {
    temp_rate_limit_ = rate_limit_;
    clients_ = new EchoClient*[count];
    for (int i = 0; i < count; i++) {
      clients_[i] = newClient(message, is_debug, max_req_count, &temp_rate_limit_);
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
  void SetExit() { *is_running_ = false; }
  void GetStatData() {
    for (int i = 0; i < count_; i++) {
      if (clients_[i]->IsValid()) {
        getDealStat(clients_[i]);
      }
    }
  }
  // 实现连接操作的耗时统计
  static void PrintStatData(int64_t client_count, int64_t run_time) {
    cout << "--- benchmark statistics ---" << endl;
    TinyEcho::Percentile::PrintPctAvgData();
    cout << "success[" << success_count_ << "],failure[" << failure_count_ << "],try_connect[" << try_connect_count_
         << "],connect_failure[" << connect_failure_count_ << "],read_failure[" << read_failure_count_
         << "],write_failure[" << write_failure_count_ << "]" << endl;
    cout << "client_count[" << client_count << "],success_qps[" << success_count_ / run_time << "]" << endl;
    cout << "request_failure_rate[" << (double)failure_count_ * 100 / (success_count_ + failure_count_)
         << "%],connect_failure_rate[" << (double)connect_failure_count_ * 100 / try_connect_count_ << "%]" << endl;
  }

  void CheckClientStatusAndDeal() {  // 检查客户端连接状态，连接失效的会重新建立连接
    int32_t create_client_count = 0;
    for (int i = 0; i < count_; i++) {
      if (clients_[i]->IsValid()) {
        clients_[i]->TryRestart();  // 尝试重启请求
        continue;
      }
      getDealStat(clients_[i]);
      delete clients_[i];
      create_client_count++;
      clients_[i] = newClient(message_, is_debug_, max_req_count_, &temp_rate_limit_);
      clients_[i]->Connect(ip_, port_);
    }
    if (is_debug_) {
      cout << "create_client_count=" << create_client_count << endl;
    }
  }
  void RateLimitRefresh() { temp_rate_limit_ = rate_limit_; }

 private:
  void getDealStat(EchoClient* client) {
    int64_t success_count{0};
    int64_t failure_count{0};
    int64_t connect_failure_count{0};
    int64_t read_failure_count{0};
    int64_t write_failure_count{0};
    int64_t try_connect_count{0};
    client->GetDealStat(success_count, failure_count, connect_failure_count, read_failure_count, write_failure_count,
                        try_connect_count);
    assert(failure_count <= 1 && connect_failure_count <= 1 && read_failure_count <= 1 && write_failure_count <= 1 &&
           try_connect_count <= 1);
    assert(failure_count == (connect_failure_count + read_failure_count + write_failure_count));
    {
      std::lock_guard<std::mutex> guard(mutex_);
      success_count_ += success_count;
      failure_count_ += failure_count;
      connect_failure_count_ += connect_failure_count;
      read_failure_count_ += read_failure_count;
      write_failure_count_ += write_failure_count;
      try_connect_count_ += try_connect_count;
    }
  }
  EchoClient* newClient(const std::string& message, bool is_debug, int64_t max_req_count, int64_t* temp_rate_limit) {
    EchoClient* client = nullptr;
    client = new EchoClient(epoll_fd_, &percentile_, is_debug, max_req_count, temp_rate_limit);
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
  int64_t max_req_count_;  // 最大请求次数
  bool is_debug_;  // 是否调试模式
  int64_t run_time_;  // 运行时间
  TinyEcho::Percentile percentile_;  // 用于统计请求耗时的pctxx数值
  bool* is_running_;  // 是否运行中
  int64_t rate_limit_;  // 请求限流
  int64_t temp_rate_limit_;  // 请求限流临时变量
  static std::mutex mutex_;  // 统计使用的互斥量
  static int64_t success_count_;  // 成功数统计
  static int64_t failure_count_;  // 失败数统计
  static int64_t try_connect_count_;  // 尝试连接的次数
  static int64_t connect_failure_count_;  // 细分失败类型统计，连接失败
  static int64_t read_failure_count_;  // 细分失败类型统计，读失败
  static int64_t write_failure_count_;  // 细分失败类型统计，写失败
};

std::mutex ClientManager::mutex_;
int64_t ClientManager::success_count_ = 0;
int64_t ClientManager::failure_count_ = 0;
int64_t ClientManager::try_connect_count_ = 0;
int64_t ClientManager::connect_failure_count_ = 0;
int64_t ClientManager::read_failure_count_ = 0;
int64_t ClientManager::write_failure_count_ = 0;
}  // namespace BenchMark