#include "scheduler.h"
#include "log.h"

namespace hps_sf {

static hps_sf::hps_Logger::ptr g_logger  HPS_LOG_NAME("system");

  hps_Scheduler::hps_Scheduler(size_t threads, bool = use_caller, const std::string& name) {

  }

  hps_Scheduler::~hps_Scheduler() {

  }

  hps_Fiber* hps_Scheduler::GetMainFiber() {

  }

  void hps_Scheduler::start() {

  }

  void hps_Scheduler::stop() {
    
  }


}