#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../cmdline.h"
#include "../common.hpp"

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

void handler(int sock_fd) {
  while (true) {
    int client_fd = accept(sock_fd, NULL, 0);
    if (client_fd < 0) {
      perror("accept failed");
      continue;
    }
    handlerClient(client_fd);
    close(client_fd);
  }
}

void usage() {
  cout << "ProcessPool1 -ip 0.0.0.0 -port 1688" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help      print usage" << endl;
  cout << "    -ip,--ip       listen ip" << endl;
  cout << "    -port,--port   listen port" << endl;
  cout << endl;
}

int main(int argc, char* argv[]) {
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
  for (int i = 0; i < GetNProcs(); i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      continue;
    }
    if (0 == pid) {
      handler(sock_fd);  // 子进程陷入死循环，处理客户端请求
      exit(0);
    }
  }
  while (true) sleep(1);  // 父进程陷入死循环
  return 0;
}