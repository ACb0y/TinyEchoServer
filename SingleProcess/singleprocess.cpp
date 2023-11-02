#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "../cmdline.h"
#include "../common.hpp"

using namespace std;
using namespace TinyEcho;

void handlerClient(int client_fd) {
  string msg;
  if (not RecvMsg(client_fd, msg)) {
    return;
  }
  SendMsg(client_fd, msg);
  close(client_fd);
}

void usage() {
  cout << "SingleProcess -ip 0.0.0.0 -port 1688" << endl;
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
  while (true) {
    int client_fd = accept(sock_fd, NULL, 0);
    if (client_fd < 0) {
      perror("accept failed");
      continue;
    }
    handlerClient(client_fd);
    close(client_fd);
  }
  return 0;
}