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
  void Stat(int64_t value) {
    stat_data_.push_back(value);
    if (stat_data_.size() >= 100000) {
      printData();
    }
  }
  bool GetPercentile(double pct, double &pctValue) {
    double x = (stat_data_.size() - 1) * pct;
    int32_t i = (int32_t)x;
    double j = x - i;
    pctValue = (1 - j) * stat_data_[i] + j * stat_data_[i + 1];
    return true;
  }
  static void PrintAvgData() {
    if (pct50_data_.size() <= 0) return;
    double pct50 = std::accumulate(pct50_data_.begin(), pct50_data_.end(), 0.0) / pct50_data_.size();
    double pct95 = std::accumulate(pct95_data_.begin(), pct95_data_.end(), 0.0) / pct95_data_.size();
    double pct99 = std::accumulate(pct99_data_.begin(), pct99_data_.end(), 0.0) / pct99_data_.size();
    cout << "pct avg data -> pct50[" << pct50 << "us],pct95[" << pct95 << "us],pct99[" << pct99 << "us]" << endl;
  }

 private:
  void printData() {
    if (stat_data_.size() <= 0) return;
    std::sort(stat_data_.begin(), stat_data_.end());
    double pct50, pct95, pct99;
    GetPercentile(0.50, pct50);
    GetPercentile(0.95, pct95);
    GetPercentile(0.99, pct99);
    {
      std::lock_guard<std::mutex> guard(mutex_);  // 对终端的输出和统计pctxx数据都是临界区
      cout << "per " << std::to_string(stat_data_.size()) << " request stat data -> ";
      cout << "pct50[" << pct50 << "us],pct95[" << pct95 << "us],pct99[" << pct99 << "us]" << endl;
      if (stat_data_.size() >= 100000) {
        pct50_data_.push_back(pct50);
        pct95_data_.push_back(pct95);
        pct99_data_.push_back(pct99);
      }
    }
    stat_data_.clear();
  }

 private:
  std::vector<int64_t> stat_data_;
  static std::mutex mutex_;
  static std::vector<double> pct50_data_;
  static std::vector<double> pct95_data_;
  static std::vector<double> pct99_data_;
};

std::mutex Percentile::mutex_;
std::vector<double> Percentile::pct50_data_;
std::vector<double> Percentile::pct95_data_;
std::vector<double> Percentile::pct99_data_;
}  // namespace TinyEcho