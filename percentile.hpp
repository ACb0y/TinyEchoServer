#pragma once

#include <algorithm>
#include <iostream>
#include <mutex>
#include <numeric>
#include <vector>

using namespace std;

namespace TinyEcho {
class Percentile {
 public:
  Percentile() = default;
  void ConnectSpendTimeStat(int64_t value) { connect_spend_time_stat_data_.push_back(value); }
  void InterfaceSpendTimeStat(int64_t value) {
    interface_spend_time_stat_data_.push_back(value);
    printSpendTimePctData();
  }

  static void PrintPctAvgData() {
    if (interface_spend_time_stat_pct50_data_.size() <= 0) return;
    double pct50 = std::accumulate(interface_spend_time_stat_pct50_data_.begin(),
                                   interface_spend_time_stat_pct50_data_.end(), 0.0) /
                   interface_spend_time_stat_pct50_data_.size();
    double pct95 = std::accumulate(interface_spend_time_stat_pct95_data_.begin(),
                                   interface_spend_time_stat_pct95_data_.end(), 0.0) /
                   interface_spend_time_stat_pct95_data_.size();
    double pct99 = std::accumulate(interface_spend_time_stat_pct99_data_.begin(),
                                   interface_spend_time_stat_pct99_data_.end(), 0.0) /
                   interface_spend_time_stat_pct99_data_.size();
    cout << "pct avg data -> interface_pct50[" << pct50 << "us],interface_pct95[" << pct95 << "us],interface_pct99["
         << pct99 << "us]" << endl;
  }

 private:
  bool getInterfacePctValue(double pct, double &pctValue) {
    if (interface_spend_time_stat_data_.size() <= 0) {
      return false;
    }
    double x = (interface_spend_time_stat_data_.size() - 1) * pct;
    int32_t i = (int32_t)x;
    double j = x - i;
    pctValue = (1 - j) * interface_spend_time_stat_data_[i] + j * interface_spend_time_stat_data_[i + 1];
    return true;
  }
  bool getConnectPctValue(double pct, double &pctValue) {
    if (connect_spend_time_stat_data_.size() <= 0) {
      return false;
    }
    double x = (connect_spend_time_stat_data_.size() - 1) * pct;
    int32_t i = (int32_t)x;
    double j = x - i;
    pctValue = (1 - j) * connect_spend_time_stat_data_[i] + j * connect_spend_time_stat_data_[i + 1];
    return true;
  }
  void printSpendTimePctData() {
    if (interface_spend_time_stat_data_.size() < 100000) return;
    std::sort(connect_spend_time_stat_data_.begin(), connect_spend_time_stat_data_.end());
    std::sort(interface_spend_time_stat_data_.begin(), interface_spend_time_stat_data_.end());
    double interface_pct50, interface_pct95, interface_pct99;
    getInterfacePctValue(0.50, interface_pct50);
    getInterfacePctValue(0.95, interface_pct95);
    getInterfacePctValue(0.99, interface_pct99);
    double connect_pct50, connect_pct95, connect_pct99;
    {
      std::lock_guard<std::mutex> guard(mutex_);  // 对终端的输出和统计pctxx数据都是临界区
      cout << "per " << std::to_string(interface_spend_time_stat_data_.size()) << " req stat data -> ";
      cout << "interface[pct50=" << interface_pct50 << "us,pct95=" << interface_pct95 << "us,pct99=" << interface_pct99
           << "us]";
      if (getConnectPctValue(0.50, connect_pct50) && getConnectPctValue(0.95, connect_pct95) &&
          getConnectPctValue(0.99, connect_pct99)) {
        cout << ",connect[pct50=" << connect_pct50 << "us,pct95=" << connect_pct95 << "us,pct99=" << connect_pct99
             << "]" << endl;
      } else {
        cout << endl;
      }
      interface_spend_time_stat_pct50_data_.push_back(interface_pct50);
      interface_spend_time_stat_pct95_data_.push_back(interface_pct95);
      interface_spend_time_stat_pct99_data_.push_back(interface_pct99);
    }
    connect_spend_time_stat_data_.clear();
    interface_spend_time_stat_data_.clear();
  }

 private:
  std::vector<int64_t> connect_spend_time_stat_data_;
  std::vector<int64_t> interface_spend_time_stat_data_;
  static std::mutex mutex_;
  static std::vector<double> interface_spend_time_stat_pct50_data_;
  static std::vector<double> interface_spend_time_stat_pct95_data_;
  static std::vector<double> interface_spend_time_stat_pct99_data_;
};

std::mutex Percentile::mutex_;
std::vector<double> Percentile::interface_spend_time_stat_pct50_data_;
std::vector<double> Percentile::interface_spend_time_stat_pct95_data_;
std::vector<double> Percentile::interface_spend_time_stat_pct99_data_;

}  // namespace TinyEcho