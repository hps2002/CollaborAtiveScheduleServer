#include "hps_sf/hps_sf.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

void test_fiber() {
  static int s_count = 5;
  HPS_LOG_INFO(g_logger) << "test in fiber s_count = " << s_count;
  sleep(1);
  if (-- s_count>= 0)
    hps_sf::hps_Scheduler::GetThis() -> schedule(&test_fiber, hps_sf::GetThreadId());
}

int main() {
  HPS_LOG_INFO(g_logger) << "main";
  hps_sf::hps_Scheduler sc(3, true, "test");
  sc.start();
  sleep(2);
  HPS_LOG_INFO(g_logger) << "schedule";
  sc.schedule(&test_fiber);
  sc.stop();
  HPS_LOG_INFO(g_logger) << "over";
  return 0;
}