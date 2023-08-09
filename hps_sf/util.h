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

void Backtrace (std::vector<std::string>& bt, int size, int skip = 1) ;

std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");


}
#endif
