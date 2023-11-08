#pragma once

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

namespace TinyEcho {
class Percentile {
 public:
  Percentile() = default;
  void Stat(int64_t value) {
    stat_data_.push_back(value);
    total_++;
    if (stat_data_.size() >= 100000) {
      printData();
      already_print_ = true;
    }
  }
  size_t GetStatDataCount() { return stat_data_.size(); }
  bool GetPercentile(double pct, double &pctValue) {
    double x = (stat_data_.size() - 1) * pct;
    int32_t i = (int32_t)x;
    double j = x - i;
    pctValue = (1 - j) * stat_data_[i] + j * stat_data_[i + 1];
    return true;
  }
  void PrintPctData() { printData(); }
  void PrintAvgPctData() { printAvgData(); }

 private:
  void printData() {
    if (stat_data_.size() <= 0) return;
    // 小于10w且已经输出过统计数据，则不再输出
    if (stat_data_.size() < 100000 && already_print_) return;
    std::sort(stat_data_.begin(), stat_data_.end());
    double pct50, pct95, pct99;
    GetPercentile(0.50, pct50);
    GetPercentile(0.95, pct95);
    GetPercentile(0.99, pct99);
    cout << "per " << std::to_string(stat_data_.size()) << " request stat data -> ";
    cout << "pct50[" << pct50 << "],pct95[" << pct95 << "],pct99[" << pct99 << "]" << endl;
    if (stat_data_.size() >= 100000) {
      pct50_data_.push_back(pct50);
      pct95_data_.push_back(pct95);
      pct99_data_.push_back(pct99);
    }
    stat_data_.clear();
  }
  void printAvgData() {
    if (pct50_data_.size() <= 0) return;
    double pct50 = std::accumulate(pct50_data_.begin(), pct50_data_.end(), 0.0) / pct50_data_.size();
    double pct95 = std::accumulate(pct95_data_.begin(), pct95_data_.end(), 0.0) / pct95_data_.size();
    double pct99 = std::accumulate(pct99_data_.begin(), pct99_data_.end(), 0.0) / pct99_data_.size();
    cout << "pct avg data -> pct50[" << pct50 << "],pct95[" << pct95 << "],pct99[" << pct99 << "]" << endl;
  }

 private:
  bool already_print_{false};
  std::vector<int64_t> stat_data_;
  std::vector<double> pct50_data_;
  std::vector<double> pct95_data_;
  std::vector<double> pct99_data_;
};
}  // namespace TinyEcho