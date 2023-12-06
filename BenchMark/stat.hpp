#pragma once
#include <mutex>

#include "percentile.hpp"

namespace BenchMark {
// pct指标统计
class PctStat {
 public:
  void InterfaceSpendTimeStat(double pct50, double pct95, double pct99) {
    std::lock_guard<std::mutex> guard(mutex_);
    interface_spend_time_stat_pct50_.push_back(pct50);
    interface_spend_time_stat_pct95_.push_back(pct95);
    interface_spend_time_stat_pct99_.push_back(pct99);
  }
  void PrintPctAvgData() {
    if (interface_spend_time_stat_pct50_.size() <= 0) return;
    double pct50 =
        std::accumulate(interface_spend_time_stat_pct50_.begin(), interface_spend_time_stat_pct50_.end(), 0.0) /
        interface_spend_time_stat_pct50_.size();
    double pct95 =
        std::accumulate(interface_spend_time_stat_pct95_.begin(), interface_spend_time_stat_pct95_.end(), 0.0) /
        interface_spend_time_stat_pct95_.size();
    double pct99 =
        std::accumulate(interface_spend_time_stat_pct99_.begin(), interface_spend_time_stat_pct99_.end(), 0.0) /
        interface_spend_time_stat_pct99_.size();
    cout << "pct avg data -> interface_pct50[" << pct50 << "us],interface_pct95[" << pct95 << "us],interface_pct99["
         << pct99 << "us]" << endl;
  }

 private:
  std::mutex mutex_;
  std::vector<double> interface_spend_time_stat_pct50_;
  std::vector<double> interface_spend_time_stat_pct95_;
  std::vector<double> interface_spend_time_stat_pct99_;
};

// 汇总统计
class SumStat {
 public:
  void DoStat(int64_t success_count, int64_t failure_count, int64_t read_failure_count, int64_t write_failure_count,
              int64_t connect_failure_count, int64_t try_connect_count) {
    std::lock_guard<std::mutex> guard(mutex_);
    success_count_ += success_count;
    failure_count_ += failure_count;
    read_failure_count_ += read_failure_count;
    write_failure_count_ += write_failure_count;
    connect_failure_count_ += connect_failure_count;
    try_connect_count_ += try_connect_count;
  }
  void PrintStatData(int64_t client_count, int64_t run_time) {
    cout << "success[" << success_count_ << "],failure[" << failure_count_ << "],try_connect[" << try_connect_count_
         << "],connect_failure[" << connect_failure_count_ << "],read_failure[" << read_failure_count_
         << "],write_failure[" << write_failure_count_ << "]" << endl;
    cout << "client_count[" << client_count << "],success_qps[" << success_count_ / run_time << "]" << endl;
    cout << "request_failure_rate[" << (double)failure_count_ * 100 / (success_count_ + failure_count_)
         << "%],connect_failure_rate[" << (double)connect_failure_count_ * 100 / try_connect_count_ << "%]" << endl;
  }

 private:
  std::mutex mutex_;
  int64_t failure_count_{0};  // 失败次数
  int64_t success_count_{0};  // 成功次数
  int64_t read_failure_count_{0};  // 细分的失败统计，读失败数
  int64_t write_failure_count_{0};  // 细分的失败统计，写失败数
  int64_t connect_failure_count_{0};  // 细分的失败统计，连接失败数
  int64_t try_connect_count_{0};  // 尝试连接的次数
};
}  // namespace BenchMark