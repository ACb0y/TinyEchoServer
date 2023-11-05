#pragma once

#include <stdlib.h>
#include <time.h>

#include <string>

#include "../epollctl.hpp"

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
  int Fd() { return fd_; }
  ClientStatus GetStatus() { return status_; }
  void SetEchoMessage(const std::string& message) { message_ = message; }
  bool IsValid() {
    if (Failure == status_) {
      return false;
    }
    bool is_valid = true;
    // 发送完请求之后，2秒还没收完应答，请求超时
    if (RecvResponse == status_ && time(nullptr) - last_req_send_time_ >= 2) {
      is_valid = false;
    }
    // 完成的请求数，已经达到最大设置的数
    if (finish_req_count_ > max_req_count_) {
      is_valid = false;
    }
    ClearEvent(epoll_fd_, fd_);
    return is_valid;
  }
  void Connect(const std::string& ip, int64_t port) {
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
      status_ = Failure;
      return;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(int16_t(port));
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    int ret = connect(fd_, (struct sockaddr*)&addr, sizeof(addr));
    if (0 == ret) {
      status_ = ConnectSuccess;
      AddWriteEvent(epoll_fd_, fd_, this);  // 监控可写事件
      return;
    }
    if (errno == EINPROGRESS) {
      status_ = Connecting;
      AddWriteEvent(epoll_fd_, fd_, this);  // 监控可写事件
      return;
    }
    status_ = Failure;
    return;
  }
  bool Deal() {  // 本函数实现状态机的流转
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
    if (not result) {
      status_ = Failure;
      ClearEvent(epoll_fd_, fd_);
    }
    return result;
  }

 private:
  bool checkConnect() {
    // TODO
    // return Common::SockOpt::GetSocketError(fd);  // 检查sockFd上是否有错误发生
    return true;
  }
  bool sendRequest() {
    // TODO
    last_req_send_time_ = time(nullptr);
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
  int64_t last_req_send_time_;  // 最后一个请求的发送时间（单位秒）
};
}  // namespace BenchMark