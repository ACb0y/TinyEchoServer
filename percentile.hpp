#pragma once

#include <algorithm>
#include <vector>

namespace TinyEcho {
class Percentile {
 public:
  Percentile() = default;
  void Stat(int64_t value) { stat_data_.push_back(value); }
  size_t GetStatDataCount() { return stat_data_.size(); }
  bool GetPercentile(double pct, double &pctValue) {
    std::sort(stat_data_.begin(), stat_data_.end());
    double x = (stat_data_.size() - 1) * pct;
    int32_t i = (int32_t)x;
    double j = x - i;
    pctValue = (1 - j) * stat_data_[i] + j * stat_data_[i + 1];
    return true;
  }

 private:
  std::vector<int64_t> stat_data_;  // 原始统计数据
};
}  // namespace TinyEcho