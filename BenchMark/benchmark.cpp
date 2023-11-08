#include <iostream>
#include <string>

#include "../cmdline.h"
#include "../epollctl.hpp"
#include "../timer.h"
#include "clientmanager.hpp"

using namespace std;
using namespace BenchMark;

void usage() {
  // 新增一个debug的选项
  cout << "BenchMark -ip 0.0.0.0 -port 1688 -pkt_size 1024 -client_count 200 -run_time 60" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help                      print usage" << endl;
  cout << "    -ip,--ip                       service listen ip" << endl;
  cout << "    -port,--port                   service listen port" << endl;
  cout << "    -pkt_size,--pkt_size           size of send packet, unit is byte" << endl;
  cout << "    -client_count,--client_count   count of client" << endl;
  cout << "    -run_time,--run_time           run time, unit is second" << endl;
  cout << "    -debug,--debug                 debug mode, more info print" << endl;
  cout << endl;
}

void finish(void *data) {
  ClientManager *client_manager = (ClientManager *)data;
  client_manager->GetStatData();
  client_manager->PrintStatData();
  exit(0);
}

void clientManagerCheck(void *data) {
  ClientManager *client_manager = (ClientManager *)data;
  int32_t create_client_count = 0;
  client_manager->CheckStatus(create_client_count);
  // 重新注册定时器
  client_manager->GetTimer()->Register(clientManagerCheck, data, 1);
}

int main(int argc, char *argv[]) {
  string ip;
  int64_t port;
  int64_t pkt_size;
  int64_t client_count;
  int64_t run_time;
  bool is_debug{false};
  bool is_rand_req_count{false};
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::Int64OptRequired(&pkt_size, "pkt_size");
  CmdLine::Int64OptRequired(&client_count, "client_count");
  CmdLine::Int64OptRequired(&run_time, "run_time");
  CmdLine::BoolOpt(&is_debug, "debug");
  CmdLine::BoolOpt(&is_rand_req_count, "rand_req_count");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);

  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return -1;
  }
  Timer timer;
  std::string message(pkt_size + 1, 'a');
  ClientManager client_manager(ip, port, epoll_fd, &timer, client_count, message, is_rand_req_count, is_debug);
  timer.Register(finish, &client_manager, run_time * 1000);
  timer.Register(clientManagerCheck, &client_manager, 1);
  while (true) {
    int64_t msec = 0;
    bool oneTimer = false;
    TimerData timer_data;
    oneTimer = timer.GetLastTimer(timer_data);
    if (oneTimer) {
      msec = timer.TimeOutMs(timer_data);
    }
    int num = epoll_wait(epoll_fd, events, 2048, msec);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    } else if (num == 0) {
      sleep(0);  // 这里直接sleep(0)让出cpu。大概率被挂起，这里主动让出cpu，可以减少一次epoll_wait的调用
      msec = -1;  // 大概率被挂起，故这里超时时间设置为-1
    } else {
      msec = 0;  // 下次大概率还有事件，故msec设置为0
    }
    for (int i = 0; i < num; i++) {
      EchoClient *client = (EchoClient *)events[i].data.ptr;
      client->Deal();
    }
    if (oneTimer) timer.Run(timer_data);  // 处理定时器
  }
  return 0;
}
