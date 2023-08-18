#include "hps_sf/hps_sf.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

int main() {
  HPS_LOG_INFO(g_logger) << "main";
  hps_sf::hps_Scheduler sc;
  sc.start();
  sc.stop();
  HPS_LOG_ERROR(g_logger) << "over";
  return 0;
}