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
  Finish = 8,  // 客户端read返回了0
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
    if (Failure == status_ || Finish == status_) {
      return false;
    }
    bool is_valid = true;
    // connect超时
    if (Connecting == status_) {
      if ((GetCurrentTimeUs() - last_connect_time_us_) / 1000 >= 2000) {
        is_valid = false;
      }
    }
    // 写超时
    if (SendRequest == status_) {
      if ((GetCurrentTimeUs() - last_send_req_time_us_) / 1000 >= 2000) {
        is_valid = false;
      }
    }
    // 读超时
    if (RecvResponse == status_) {
      if ((GetCurrentTimeUs() - last_recv_resp_time_us_) / 1000 >= 2000) {
        is_valid = false;
      }
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
      last_connect_time_us_ = GetCurrentTimeUs();
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
    int err = 0;
    socklen_t errLen = sizeof(err);
    assert(0 == getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &errLen));
    return 0 == err;
  }
  bool sendRequest() {
    last_send_req_time_us_ = GetCurrentTimeUs();

    // TODO
    return true;
  }
  bool recvResponse() {
    // TODO
    return true;
  }
  /*
   *  bool Read() {
    do {
      ssize_t ret = read(fd_, codec_.Data(), codec_.Len());
      if (ret == 0) {
        perror("peer close connection");
        return false;
      }
      if (ret < 0) {
        if (EINTR == errno) continue;
        if (EAGAIN == errno or EWOULDBLOCK == errno) return true;
        perror("read failed");
        return false;
      }
      codec_.DeCode((size_t)ret);
    } while (is_multi_io_);
    return true;
  }
  bool Write() {
    do {
      if (send_len_ == pkt_.UseLen()) return true;
      ssize_t ret = write(fd_, pkt_.Data() + send_len_, pkt_.UseLen() - send_len_);
      if (ret < 0) {
        if (EINTR == errno) continue;
        if (EAGAIN == errno && EWOULDBLOCK == errno) return true;
        perror("write failed");
        return false;
      }
      send_len_ += ret;
    } while (is_multi_io_);
    return true;
  }
   */
  int64_t GetCurrentTimeUs() {
    struct timeval current;
    gettimeofday(&current, NULL);
    return current.tv_sec * 1000000 + current.tv_usec;  //计算运行的时间，单位微秒
  }

 private:
  int fd_{-1};  // 客户端关联的fd
  int epoll_fd_{-1};  // 关联的epoll_fd
  int max_req_count_{0};  // 客户端最多执行多少次请求
  int finish_req_count_{0};  // 完成的请求数
  ClientStatus status_{Init};  // 客户端状态机
  std::string message_;  // 要发送的消息
  int64_t last_connect_time_us_{0};  // 最近一次connect时间，单元us
  int64_t last_send_req_time_us_{0};  // 最近一次发送请求时间，单元us
  int64_t last_recv_resp_time_us_{0};  // 最近一次接受应答时间，单位us
};
}  // namespace BenchMark