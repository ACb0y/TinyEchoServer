#include <iostream>
#include <string>

#include "../cmdline.h"
#include "../epollctl.hpp"
#include "../timer.h"

using namespace std;

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
  Timer timer;
  timer.Register(finish, nullptr, run_time * 1000);
  while (true) {
    // TODO
    TimerData timer_data;
    timer.GetLastTimer(timer_data);
  }
  return 0;
}
