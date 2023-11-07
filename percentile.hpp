#pragma once

#include <algorithm>
#include <iostream>
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
  size_t GetStatDataCount() { return stat_data_.size(); }
  bool GetPercentile(double pct, double &pctValue) {
    double x = (stat_data_.size() - 1) * pct;
    int32_t i = (int32_t)x;
    double j = x - i;
    pctValue = (1 - j) * stat_data_[i] + j * stat_data_[i + 1];
    return true;
  }
  void PrintPctData() { printData(); }

 private:
  void printData() {
    if (stat_data_.size() <= 0) return;
    std::sort(stat_data_.begin(), stat_data_.end());
    double pct50, pct95, pct99, pct999;
    GetPercentile(0.50, pct50);
    GetPercentile(0.95, pct95);
    GetPercentile(0.99, pct99);
    GetPercentile(0.999, pct999);
    cout << "per " << std::to_string(stat_data_.size()) << " request stat data" << endl;
    cout << "pct50[" << pct50 << "],pct95[" << pct95 << "],pct99[" << pct99 << "],pct999[" << pct999 << "]" << endl;
    stat_data_.clear();
  }

 private:
  std::vector<int64_t> stat_data_;  // 原始统计数据
};
}  // namespace TinyEcho