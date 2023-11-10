#include <iostream>
#include <string>
#include <thread>

#include "../cmdline.h"
#include "../epollctl.hpp"
#include "../timer.h"
#include "clientmanager.hpp"

using namespace std;
using namespace BenchMark;

string ip;
int64_t port;
int64_t thread_count;
int64_t pkt_size;
int64_t client_count;
int64_t run_time;
int64_t max_req_count;
bool is_debug;

void usage() {
  cout << "BenchMark -ip 0.0.0.0 -port 1688 -thread_count 1 -max_req_count 100000 -pkt_size 1024 -client_count 200 "
          "-run_time 60 -debug"
       << endl;
  cout << "options:" << endl;
  cout << "    -h,--help                      print usage" << endl;
  cout << "    -ip,--ip                       service listen ip" << endl;
  cout << "    -port,--port                   service listen port" << endl;
  cout << "    -thread_count,--thread_count   run thread count" << endl;
  cout << "    -max_req_count,--max_req_count one connection max req count" << endl;
  cout << "    -pkt_size,--pkt_size           size of send packet, unit is byte" << endl;
  cout << "    -client_count,--client_count   count of client" << endl;
  cout << "    -run_time,--run_time           run time, unit is second" << endl;
  cout << "    -debug,--debug                 debug mode, more info print" << endl;
  cout << endl;
}

void threadExit(void *data) {
  ClientManager *client_manager = (ClientManager *)data;
  client_manager->GetStatData();
  client_manager->SetExit();
}

void clientManagerCheck(void *data) {
  ClientManager *client_manager = (ClientManager *)data;
  client_manager->CheckStatus();
  // 重新注册定时器
  client_manager->GetTimer()->Register(clientManagerCheck, data, 1);
}

void handler() {
  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return;
  }
  Timer timer;
  bool is_running = true;
  std::string message(pkt_size + 1, 'a');
  ClientManager client_manager(ip, port, epoll_fd, &timer, client_count, message, max_req_count, is_debug, run_time,
                               &is_running);
  timer.Register(threadExit, &client_manager, run_time * 1000);
  timer.Register(clientManagerCheck, &client_manager, 1);
  while (is_running) {
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
}

int main(int argc, char *argv[]) {
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::Int64OptRequired(&thread_count, "thread_count");
  CmdLine::Int64OptRequired(&max_req_count, "max_req_count");
  CmdLine::Int64OptRequired(&pkt_size, "pkt_size");
  CmdLine::Int64OptRequired(&client_count, "client_count");
  CmdLine::Int64OptRequired(&run_time, "run_time");
  CmdLine::BoolOpt(&is_debug, "debug");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);
  thread_count = thread_count > 10 ? 10 : thread_count;
  std::thread threads[10];
  for (int64_t i = 0; i < thread_count; i++) {
    threads[i] = std::thread(handler);
  }
  for (int64_t i = 0; i < thread_count; i++) {
    threads[i].join();
  }
  ClientManager::PrintStatData(thread_count * client_count, run_time);
  return 0;
}
