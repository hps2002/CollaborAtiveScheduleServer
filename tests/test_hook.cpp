#include "hps_sf/hook.h"
#include "hps_sf/fiber.h"
#include "hps_sf/iomanager.h"
#include "hps_sf/log.h"

static hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

void test_hook() {
  hps_sf::hps_IOManager iom(1);
  iom.schedule([](){
    sleep(3);
    HPS_LOG_DEBUG(g_logger) << "test_hook 1";
  });
  iom.schedule([](){
    sleep(2);
    HPS_LOG_DEBUG(g_logger) << "test_hook 2";
  });

  HPS_LOG_DEBUG(g_logger) << "test_hook";
}

int main() {
  test_hook();
  return 0;
}