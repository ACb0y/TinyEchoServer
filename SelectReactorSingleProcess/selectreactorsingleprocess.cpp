#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "../cmdline.h"
#include "../conn.hpp"

using namespace std;
using namespace TinyEcho;

void updateSet(unordered_set<int> &read_fds, unordered_set<int> &write_fds, int &max_fd, int sock_fd, fd_set &read_set,
               fd_set &write_set) {
  max_fd = sock_fd;
  FD_ZERO(&read_set);
  FD_ZERO(&write_set);
  FD_SET(sock_fd, &read_set);
  for (const auto &read_fd : read_fds) {
    if (read_fd > max_fd) {
      max_fd = read_fd;
    }
    FD_SET(read_fd, &read_set);
  }
  for (const auto &write_fd : write_fds) {
    if (write_fd > max_fd) {
      max_fd = write_fd;
    }
    FD_SET(write_fd, &write_set);
  }
}

void usage() {
  cout << "SelectReactorSingleProcess -ip 0.0.0.0 -port 1688" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help      print usage" << endl;
  cout << "    -ip,--ip       listen ip" << endl;
  cout << "    -port,--port   listen port" << endl;
  cout << endl;
}

int main(int argc, char *argv[]) {
  string ip;
  int64_t port;
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);
  int sock_fd = CreateListenSocket(ip, port, false);
  if (sock_fd < 0) {
    return -1;
  }
  int max_fd;
  fd_set read_set;
  fd_set write_set;
  SetNotBlock(sock_fd);
  unordered_set<int> read_fds;
  unordered_set<int> write_fds;
  unordered_map<int, Conn *> conns;
  while (true) {
    updateSet(read_fds, write_fds, max_fd, sock_fd, read_set, write_set);
    int ret = select(max_fd + 1, &read_set, &write_set, NULL, NULL);
    if (ret <= 0) {
      if (ret < 0) perror("select failed");
      continue;
    }
    for (int i = 0; i <= max_fd; i++) {
      if (FD_ISSET(i, &read_set)) {
        if (i == sock_fd) {  // 监听的sock_fd可读，则表示有新的链接
          LoopAccept(sock_fd, 1024, [&read_fds, &conns](int client_fd) {
            read_fds.insert(client_fd);  // 新增到要监听的fd集合中
            conns[client_fd] = new Conn(client_fd, true);
          });
          continue;
        }
        // 执行到这里，表明可读
        Conn *conn = conns[i];
        if (not conn->Read()) {  // 执行读失败
          delete conn;
          conns.erase(i);
          read_fds.erase(i);
          close(i);
          continue;
        }
        if (conn->OneMessage()) {  // 判断是否要触发写事件
          conn->EnCode();
          read_fds.erase(i);
          write_fds.insert(i);
        }
      }
      if (FD_ISSET(i, &write_set)) {  // 可写
        Conn *conn = conns[i];
        if (not conn->Write()) {  // 执行写失败
          delete conn;
          conns.erase(i);
          write_fds.erase(i);
          close(i);
          continue;
        }
        if (conn->FinishWrite()) {  // 完成了请求的应答写
          conn->Reset();
          write_fds.erase(i);
          read_fds.insert(i);
        }
      }
    }
  }
  return 0;
}