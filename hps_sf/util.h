#ifndef __HPS_UTIL_H__
#define __HPS_UTIL_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include <string>

namespace hps_sf{

pid_t GetThreadId();
uint32_t GetFiberId();

void Backtrace (std::vector<std::string>& bt, int size = 64, int skip = 1) ;

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

// 时间相关
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

}
#endif
