#include <iostream>
#include <string>

#include "../cmdline.h"
#include "../epollctl.hpp"
#include "../timer.h"
#include "clientmanager.hpp"

using namespace std;
using namespace BenchMark;

void usage() {
  cout << "BenchMark -ip 0.0.0.0 -port 1688 -pkt_size 1024 -client_count 200 -run_time 60 -short_conn" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help                      print usage" << endl;
  cout << "    -ip,--ip                       service listen ip" << endl;
  cout << "    -port,--port                   service listen port" << endl;
  cout << "    -pkt_size,--pkt_size           size of send packet, unit is byte" << endl;
  cout << "    -client_count,--client_count   count of client" << endl;
  cout << "    -run_time,--run_time           run time, unit is second" << endl;
  cout << "    -short_conn                    use short connect" << endl;
  cout << endl;
}

void finish(void *data) {
  // TODO
}

void clientManagerCheck(void *data) {
  ClientManager *client_manager = (ClientManager *)data;
  int32_t create_client_cnt = 0;
  client_manager->CheckStatus(create_client_cnt);
  cout << "create client count = " << create_client_cnt << endl;
  // 重新注册定时器
  client_manager->GetTimer()->Register(clientManagerCheck, data, 1000);
}

int main(int argc, char *argv[]) {
  string ip;
  int64_t port;
  int64_t pkt_size;
  int64_t client_count;
  int64_t run_time;
  bool short_conn{false};
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::Int64OptRequired(&pkt_size, "pkt_size");
  CmdLine::Int64OptRequired(&client_count, "client_count");
  CmdLine::Int64OptRequired(&run_time, "run_time");
  CmdLine::BoolOpt(&short_conn, "short");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);

  epoll_event events[2048];
  int epoll_fd = epoll_create(1);
  if (epoll_fd < 0) {
    perror("epoll_create failed");
    return -1;
  }
  Timer timer;
  ClientManager client_manager(ip, port, epoll_fd, &timer, client_count, true);
  timer.Register(finish, nullptr, run_time * 1000);
  timer.Register(clientManagerCheck, &client_manager, 1000);
  while (true) {
    int64_t msec = 0;
    bool oneTimer = false;
    TimerData timer_data;
    oneTimer = timer.GetLastTimer(timer_data);
    if (oneTimer) {
      msec = timer.GetLastTimer(timer_data);
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

/*
 * while (true) {
      oneTimer = idle_connection_timer_.GetLastTimer(timerData);
      if (oneTimer) {
        msec = idle_connection_timer_.TimeOutMs(timerData);
      }
      int num = epoll_wait(main_epoll_fd_, events, 2048, msec);
      if (num < 0) {
        ERROR("epoll_wait failed, errMsg[%s]", strerror(errno));
        continue;
      } else if (num == 0) {  // 没有事件了，下次调用epoll_wait大概率被挂起
        sleep(0);  // 这里直接sleep(0)让出cpu。大概率被挂起，这里主动让出cpu，可以减少一次epoll_wait的调用
        msec = -1;  // 大概率被挂起，故这里超时时间设置为-1
      } else {
        msec = 0;  // 下次大概率还有事件，故msec设置为0
      }
      for (int i = 0; i < num; i++) {
        EventData *data = (EventData *)events[i].data.ptr;
        data->events_ = events[i].events;
        mainEventHandler(data);
      }
      if (oneTimer) idle_connection_timer_.Run(timerData);  // 处理定时器
    }
 * */

/*

  bool isMultiIo = (std::string(argv[3]) == "1");
  EchoServer::Conn conn(sockFd, epollFd, isMultiIo);
  EchoServer::SetNotBlock(sockFd);
  EchoServer::AddReadEvent(&conn);
  while (true) {
    int num = epoll_wait(epollFd, events, 2048, -1);
    if (num < 0) {
      perror("epoll_wait failed");
      continue;
    }
    for (int i = 0; i < num; i++) {
      EchoServer::Conn *conn = (EchoServer::Conn *)events[i].data.ptr;
      if (conn->Fd() == sockFd) {
        EchoServer::LoopAccept(sockFd, 2048, [epollFd, isMultiIo](int clientFd) {
          EchoServer::Conn *conn = new EchoServer::Conn(clientFd, epollFd, isMultiIo);
          EchoServer::SetNotBlock(clientFd);
          EchoServer::AddReadEvent(conn);  // 监听可读事件
        });
        continue;
      }
      auto releaseConn = [&conn]() {
        EchoServer::ClearEvent(conn);
        delete conn;
      };
      if (events[i].events & EPOLLIN) {  // 可读
        if (not conn->Read()) {          // 执行读失败
          releaseConn();
          continue;
        }
        if (conn->OneMessage()) {             // 判断是否要触发写事件
          EchoServer::ModToWriteEvent(conn);  // 修改成只监控可写事件
        }
      }
      if (events[i].events & EPOLLOUT) {  // 可写
        if (not conn->Write()) {          // 执行写失败
          releaseConn();
          continue;
        }
        if (conn->FinishWrite()) {  // 完成了请求的应答写，则可以释放连接
          releaseConn();
        }
      }
    }
  }
 */
