#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <unordered_set>

#include "../cmdline.h"
#include "../common.hpp"

using namespace std;
using namespace TinyEcho;

void updateReadSet(std::unordered_set<int> &client_fds, int &max_fd, int sock_fd, fd_set &read_set) {
  max_fd = sock_fd;
  FD_ZERO(&read_set);
  FD_SET(sock_fd, &read_set);
  for (const auto &client_fd : client_fds) {
    if (client_fd > max_fd) {
      max_fd = client_fd;
    }
    FD_SET(client_fd, &read_set);
  }
}

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
  cout << "Select -ip 0.0.0.0 -port 1688" << endl;
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
  SetNotBlock(sock_fd);
  std::unordered_set<int> client_fds;
  while (true) {
    updateReadSet(client_fds, max_fd, sock_fd, read_set);
    int ret = select(max_fd + 1, &read_set, NULL, NULL, NULL);
    if (ret <= 0) {
      if (ret < 0) perror("select failed");
      continue;
    }
    for (int i = 0; i <= max_fd; i++) {
      if (not FD_ISSET(i, &read_set)) {
        continue;
      }
      if (i == sock_fd) {  // 监听的sock_fd可读，则表示有新的链接
        LoopAccept(sock_fd, 1024, [&client_fds](int client_fd) {
          client_fds.insert(client_fd);  // 新增到要监听的fd集合中
        });
        continue;
      }
      handlerClient(i);
      client_fds.erase(i);
      close(i);
    }
  }
  return 0;
}
