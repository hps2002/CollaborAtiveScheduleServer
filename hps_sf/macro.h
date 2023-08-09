#ifndef __HPS_MACRO_H__
#define __HPS_MACRO_H__

#include <string.h>
#include <assert.h>
#include "util.h"

// 返回每一层栈在做什么
#define HPS_ASSERT(x) \
    if (!(x)) { \
      HPS_LOG_ERROR(HPS_LOG_ROOT()) << "ASSERTION: " #x \
                        << "\nbacktrace:\n" \
                        << hps_sf::BacktraceToString(100, 2, "    "); \
      assert(x); \
    } 

#define HPS_ASSERT2(x, w) \
    if (!(x)) { \
      HPS_LOG_ERROR(HPS_LOG_ROOT()) << "ASSERTION: " #x \
                        << "\n" << w \
                        << "\nbacktrace:\n" \
                        << hps_sf::BacktraceToString(100, 2, "    "); \
      assert(x); \
    } 


#endif