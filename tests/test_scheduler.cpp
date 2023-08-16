#include "hps_sf/hps_sf.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

int main() {
  hps_sf::hps_Scheduler sc;
  sc.start();
  sc.stop();
  return 0;
}