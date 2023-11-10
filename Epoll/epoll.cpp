#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../cmdline.h"
#include "../epollctl.hpp"

using namespace std;
using namespace TinyEcho;

void handlerClient(int client_fd) {
  string msg;
  while (true) {
    if (not RecvMsg(client_fd, msg)) {
      return;
    }
    if (not SendMsg(client_fd, msg)) {
      return;
    }
  }
}

void usage() {
  cout << "Epoll -ip 0.0.0.0 -port 1688 -la" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help      print usage" << endl;
  cout << "    -ip,--ip       listen ip" << endl;
  cout << "    -port,--port   listen port" << endl;
  cout << "    -la,--la       loop accept" << endl;
  cout << endl;
}

int main(int argc, char *argv[]) {
  string ip;
  int64_t port;
  bool loop_accept;
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::BoolOpt(&loop_accept, "la");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);
  int sock_fd = CreateListenSocket(ip, port, false);
  if (sock_fd < 0) {
    return -1;
  }
  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return -1;
  }
  Conn conn(sock_fd, epoll_fd, false);
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
      if (conn->Fd() == sock_fd) {
        if (loop_accept) {
          LoopAccept(sock_fd, 2048, [epoll_fd](int client_fd) {
            Conn *conn = new Conn(client_fd, epoll_fd, false);
            AddReadEvent(conn);  // 监听可读事件，保持fd为阻塞IO
          });
        } else {
          int client_fd = accept(sock_fd, NULL, 0);
          if (client_fd > 0) {
            Conn *conn = new Conn(client_fd, epoll_fd, false);
            AddReadEvent(conn);  // 监听可读事件，保持fd为阻塞IO
            continue;
          }
          perror("accept failed");
        }
        continue;
      }
      handlerClient(conn->Fd());
      ClearEvent(conn);
      delete conn;
    }
  }
  return 0;
}
