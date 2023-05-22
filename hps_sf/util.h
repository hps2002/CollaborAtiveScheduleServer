#ifndef __HPS_UTIL_H__
#define __HPS_UTIL_H__

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdint>

namespace hps_sf{

pid_t GetThreadId();
uint32_t GetFiberId();

}
#endif
