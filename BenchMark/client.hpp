#pragma once

#include <stdlib.h>
#include <time.h>

#include <string>

namespace BenchMark {
enum ClientStatus {
  Init = 1,  // 初始状态
  Connecting = 2,  // 连接中
  ConnectSuccess = 3,  // 连接成功
  SendRequest = 4,  // 发送请求
  RecvResponse = 5,  // 接收应答
  Success = 6,  // 成功处理完一次请求
  Failure = 7,  // 失败（每个状态都可能跳转到这个状态）
};

class EchoClient {
 public:
  EchoClient(int epoll_fd, int max_req_count = 10000) : epoll_fd_(epoll_fd), max_req_count_(max_req_count) {
    if (max_req_count_ == 0) {
      srand(time(NULL));
      max_req_count_ = (rand() % 9901) + 100;
    }
  }
  ~EchoClient() {
    if (fd_ != -1) {
      close(fd_);
    }
  }
  ClientStatus GetStatus() { return status_; }
  void SetEchoMessage(const string& message) { message_ = message; }
  bool IsValid() {
    if (Failure == status_) {
      return false;
    }
    return finish_req_count_ <= max_req_count_;
  }
  void Connecting(const std::string& ip, int64_t port) {
    // TODO
  }
  bool Deal() {  // 本函数实现状态机的流转
    if (Failure == status_) {
      return false;
    }
    bool result{false};
    if (Connecting == status_) {  // 状态转移分支 -> (ConnectSuccess, Failure)
      result = checkConnect();
    } else if (ConnectSuccess == status_) {  // 状态转移分支 -> (SendRequest, Failure)
      result = sendRequest();
    } else if (SendRequest == status_) {  // 状态转移分支 -> (SendRequest, RecvResponse, Failure)
      result = sendRequest();
    } else if (RecvResponse == status_) {  // 状态转移分支 -> (RecvResponse, Success, Failure)
      result = recvResponse();
    } else if (Success == status_) {  // 状态转移分支 -> (SendRequest, Failure)
      result = sendRequest();
    }
    return result;
  }

 private:
  bool checkConnect() {
    // TODO
    return true;
  }
  bool sendRequest() {
    // TODO
    return true;
  }
  bool recvResponse() {
    // TODO
    return true;
  }

 private:
  int fd_{-1};  // 客户端关联的fd
  int epoll_fd_{-1};  // 关联的epoll_fd
  int max_req_count_{0};  // 客户端最多执行多少次请求
  int finish_req_count_{0};  // 完成的请求数
  ClientStatus status_{Init};  // 客户端状态机
  std::string message_;  // 要发送的消息
};
}  // namespace BenchMark