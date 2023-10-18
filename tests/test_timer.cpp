#include "hps_sf/timer.h"
#include "hps_sf/iomanager.h"
#include "hps_sf/fiber.h"
#include "hps_sf/log.h"
#include <unistd.h>
#include <set>

static hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

void test_sleep() {
  sleep(2);
  HPS_LOG_DEBUG(g_logger) << "test_timer1";
}

void test_usleep() {
  usleep(3);
  HPS_LOG_DEBUG(g_logger) << "test_timer2";
}

void test_timer() {
  hps_sf::hps_IOManager iom(1);
  iom.addTimer(3000, [](){
    HPS_LOG_DEBUG(g_logger) << "timer1";
  });
  iom.addTimer(1000, [](){
    HPS_LOG_DEBUG(g_logger) << "timer2";
  });

  iom.addTimer(2000, [](){
    HPS_LOG_DEBUG(g_logger) << "timer3";
   });
  HPS_LOG_DEBUG(g_logger) << "test_timer"; 
}

int main(int arg, char** args) {
  test_timer();
  return 0;
}