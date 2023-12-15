#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "../cmdline.h"
#include "../conn.hpp"
#include "../epollctl.hpp"

using namespace std;
using namespace TinyEcho;

void mainReactor(string ip, int64_t port, int64_t sub_reactor_count, bool is_main_read) {
  waitSubReactor(sub_reactor_count);  // 等待所有的subReactor线程都启动完毕
  int sock_fd = CreateListenSocket(ip, port, true);
  if (sock_fd < 0) {
    return;
  }
  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return;
  }
  int index = 0;
  Conn conn(sock_fd, epoll_fd, true);
  SetNotBlock(sock_fd);
  AddReadEvent(&conn);
  while (true) {
    int num = epoll_wait(epoll_fd, events, 2048, -1);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < num; i++) {
      Conn *conn = (Conn *)events[i].data.ptr;
      if (conn->Fd() == sock_fd) {  // 有客户端的连接到来了
        LoopAccept(sock_fd, 2048, [&index, is_main_read, epoll_fd, sub_reactor_count](int client_fd) {
          SetNotBlock(client_fd);
          if (is_main_read) {
            Conn *conn = new Conn(client_fd, epoll_fd, true);
            AddReadEvent(conn);  // 在mainReactor线程中监听可读事件
          } else {
            addToSubReactor(index, sub_reactor_count, client_fd);
          }
        });
        continue;
      }
      // 客户端有数据可读，则把连接迁移到subReactor线程中管理
      ClearEvent(conn, false);
      addToSubReactor(index, sub_reactor_count, conn->Fd());
      delete conn;
    }
  }
}

void subReactor(int thread_id) {
  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return;
  }
  EpollFd[thread_id] = epoll_fd;
  subReactorNotifyReady();
  while (true) {
    int num = epoll_wait(epoll_fd, events, 2048, -1);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < num; i++) {
      Conn *conn = (Conn *)events[i].data.ptr;
      auto releaseConn = [&conn]() {
        ClearEvent(conn);
        delete conn;
      };
      if (events[i].events & EPOLLIN) {  // 可读
        if (not conn->Read()) {  // 执行非阻塞读
          releaseConn();
          continue;
        }
        if (conn->OneMessage()) {  // 判断是否要触发写事件
          conn->EnCode();
          ModToWriteEvent(conn);  // 修改成只监控可写事件
        }
      }
      if (events[i].events & EPOLLOUT) {  // 可写
        if (not conn->Write()) {  // 执行非阻塞写
          releaseConn();
          continue;
        }
        if (conn->FinishWrite()) {  // 完成了请求的应答写，则可以释放连接
          conn->Reset();
          ModToReadEvent(conn);  // 修改成只监控可读事件
        }
      }
    }
  }
}

void usage() {
  cout << "EpollReactorProcessPoolMS -ip 0.0.0.0 -port 1688 -main 3 -sub 8" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help      print usage" << endl;
  cout << "    -ip,--ip       listen ip" << endl;
  cout << "    -port,--port   listen port" << endl;
  cout << "    -main,--main   mainReactor count" << endl;
  cout << "    -sub,--sub     subReactor count" << endl;
  cout << endl;
}

void createListenUnixSockets(std::vector<int> &unix_sockets, int64_t sub_reactor_count) {
  for (int i = 0; i < sub_reactor_count; i++) {
    int unix_socket_fd = CreateListenUnixSocket("./unix_socket_" + std::to_string(i));
    assert(unix_socket_fd > 0);
    unix_sockets.push_back(unix_sockets);
  }
}

void subReactor(std::vector<int> &unix_sockets, int index, int64_t sub_reactor_count) {
  for (int i = 0; i < sub_reactor_count; i++) {  // 先关闭多余的unix_sockets
    if (i != index) {
      close(unix_sockets[i]);
    }
  }
  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return;
  }
  int unix_socket_fd = unix_sockets[index];
  Conn conn(unix_socket_fd, epoll_fd, true);
  SetNotBlock(unix_socket_fd);
  AddReadEvent(&conn);
  while (true) {
    int num = epoll_wait(epoll_fd, events, 2048, -1);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < num; i++) {
      Conn *conn = (Conn *)events[i].data.ptr;
      auto releaseConn = [&conn]() {
        ClearEvent(conn);
        delete conn;
      };
      if (events[i].events & EPOLLIN) {  // 可读
        if (not conn->Read()) {  // 执行非阻塞读
          releaseConn();
          continue;
        }
        if (conn->OneMessage()) {  // 判断是否要触发写事件
          conn->EnCode();
          ModToWriteEvent(conn);  // 修改成只监控可写事件
        }
      }
      if (events[i].events & EPOLLOUT) {  // 可写
        if (not conn->Write()) {  // 执行非阻塞写
          releaseConn();
          continue;
        }
        if (conn->FinishWrite()) {  // 完成了请求的应答写，则可以释放连接
          conn->Reset();
          ModToReadEvent(conn);  // 修改成只监控可读事件
        }
      }
    }
  }
}

void createSubReactor(std::vector<int> &unix_sockets, int64_t sub_reactor_count) {
  for (int i = 0; i < sub_reactor_count; i++) {
    pid_t pid = fork();
    assert(pid != -1);
    if (pid == 0) {  // 子进程
      subReactor(unix_sockets, i, sub_reactor_count);
      exit(0);
    }
  }
}

void createMainReactor(std::vector<int> &unix_sockets, int64_t sub_reactor_count, int64_t main_reactor_count) {
  for (const auto &unix_socket : unix_sockets) {  // 先关闭之前创建的为SubReactor用于监听的unix_socket
    close(unix_socket);
  }

  // TODO
}

int main(int argc, char *argv[]) {
  string ip;
  int64_t port;
  int64_t main_reactor_count;
  int64_t sub_reactor_count;
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::Int64OptRequired(&main_reactor_count, "main");
  CmdLine::Int64OptRequired(&sub_reactor_count, "sub");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);
  main_reactor_count = main_reactor_count > GetNProcs() ? GetNProcs() : main_reactor_count;
  sub_reactor_count = sub_reactor_count > GetNProcs() ? GetNProcs() : sub_reactor_count;

  std::vector<int> unix_sockets;
  createListenUnixSockets(unix_sockets, sub_reactor_count);  // 在unixsocket上开启监听
  createSubReactor(unix_sockets, sub_reactor_count);  // 创建SubReactor进程
  createMainReactor(unix_sockets, sub_reactor_count, main_reactor_count);  // 创建MainRector线程
  while (true) sleep(1);  // 主进程陷入死循环
  return 0;
}