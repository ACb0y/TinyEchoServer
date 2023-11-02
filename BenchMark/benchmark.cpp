#include <iostream>
#include <string>

#include "../cmdline.h"
#include "../epollctl.hpp"

using namespace std;

void usage() {
  cout << "BenchMark -ip 0.0.0.0 -port 1688 -pkt_size 1024 -client_count 200 -run_time 60" << endl;
  cout << "options:" << endl;
  cout << "    -h,--help                      print usage" << endl;
  cout << "    -ip,--ip                       service listen ip" << endl;
  cout << "    -port,--port                   service listen port" << endl;
  cout << "    -pkt_size,--pkt_size           size of send packet, unit is byte" << endl;
  cout << "    -client_count,--client_count   count of client" << endl;
  cout << "    -run_time,--run_time           run time, unit is second" << endl;
  cout << endl;
}

int main(int argc, char *argv[]) {
  string ip;
  int64_t port;
  int64_t pkt_size;
  int64_t client_count;
  int64_t run_time;
  CmdLine::StrOptRequired(&ip, "ip");
  CmdLine::Int64OptRequired(&port, "port");
  CmdLine::Int64OptRequired(&pkt_size, "pkt_size");
  CmdLine::Int64OptRequired(&client_count, "client_count");
  CmdLine::Int64OptRequired(&run_time, "run_time");
  CmdLine::SetUsage(usage);
  CmdLine::Parse(argc, argv);
  while (true) {
    // TODO
  }
  return 0;
}
