#include "hps_sf/hps_sf.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

void run_in_fiber () {
  HPS_LOG_INFO(g_logger) << "run_in_fiber begin";
  hps_sf::hps_Fiber::YieldToHold();
  HPS_LOG_INFO(g_logger) << "run_in_fiber end";
  hps_sf::hps_Fiber::YieldToHold();
}

void test_fiber() {
  HPS_LOG_INFO(g_logger) << "main begin -1";
  
  {
    hps_sf::hps_Fiber::GetThis();
    HPS_LOG_INFO(g_logger) << "main begin";
    hps_sf::hps_Fiber::ptr fiber(new hps_sf::hps_Fiber(run_in_fiber));
    fiber -> swapIn();
    HPS_LOG_INFO(g_logger) << "main after swapIn";
    fiber -> swapIn();
    HPS_LOG_INFO(g_logger) << "main after end";
  }
  HPS_LOG_INFO(g_logger) << "main after end2"; 
}

void func() {
  std::cout << "\nhuang pei shen\n";
}

int main() {
  hps_sf::hps_Thread::SetName("main");
  std::vector<hps_sf::hps_Thread::ptr> thrs;
  for (int i = 0; i < 3; i ++) {
    thrs.push_back(hps_sf::hps_Thread::ptr(
                    new hps_sf::hps_Thread(&test_fiber, "name_" + std::to_string(i))));
  }
  for (auto i : thrs) {
    i -> join();
  }
  return 0;
}