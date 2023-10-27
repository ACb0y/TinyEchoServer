#include "coroutine.h"
#include "unittestcore.hpp"

using namespace MyCoroutine;

void print(void* arg) {
  Schedule* schedule = (Schedule*)arg;
  std::cout << "print begin" << std::endl;
  CoroutineYield(*schedule);
  std::cout << "print end" << std::endl;
}

TEST_CASE(Schedule_ALL) {
  Schedule schedule;
  ScheduleInit(schedule, 1024);
  ASSERT_FALSE(ScheduleRunning(schedule));
  ScheduleClean(schedule);
}

TEST_CASE(Coroutine_Success) {
  Schedule schedule;
  ScheduleInit(schedule, 1024);
  ASSERT_FALSE(ScheduleRunning(schedule));
  int id = CoroutineCreate(schedule, print, &schedule);
  ASSERT_EQ(id, 0);
  while (ScheduleRunning(schedule)) {
    ASSERT_EQ(CoroutineResume(schedule), Success);
    ASSERT_EQ(CoroutineResumeById(schedule, id), Success);
  }
  ScheduleClean(schedule);
}
/*
 * // 创建协程
int CoroutineCreate(Schedule& schedule, Entry entry, void* arg, uint32_t priority = 0);
// 让出执行权，只能在从协程中调用
void CoroutineYield(Schedule& schedule);
// 恢复从协程的调用，只能在主协程中调用
int CoroutineResume(Schedule& schedule);
// 恢复指定从协程的调用，只能在主协程中调用
int CoroutineResumeById(Schedule& schedule, int id);

// 协程调度结构体初始化
int ScheduleInit(Schedule& schedule, int coroutineCnt, int stackSize = 8 * 1024);
// 判断是否还有协程在运行
bool ScheduleRunning(Schedule& schedule);
// 释放调度器
void ScheduleClean(Schedule& schedule);
 */