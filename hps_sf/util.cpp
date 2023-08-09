#include "util.h"
#include <execinfo.h>
#include "fiber.h"
#include "log.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_NAME("system");

namespace hps_sf{

pid_t GetThreadId()
{
    return syscall(SYS_gettid);
}

uint32_t GetFiberId()
{
  return hps_sf::hps_Fiber::GetFiberId();
}

void Backtrace (std::vector<std::string>& bt, int size, int skip)  {
  void** array = (void**) malloc((sizeof(void*) * size));
  size_t s = ::backtrace(array, size);

  char** strings = backtrace_symbols(array, s);
  if (strings == NULL) {
    HPS_LOG_ERROR(g_logger) << "backtrace_synbols error";
    return;
  }

  for (size_t i = skip; i < s; i ++) {
    bt.push_back(strings[i]);
  }

  free(strings);
  free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
  std::vector<std::string> bt;
  Backtrace(bt, size, skip);
  std::stringstream ss;
  for (size_t i = 0; i < bt.size(); i ++) {
    ss << prefix << bt[i] << std::endl;
  }
  return ss.str();
}

}