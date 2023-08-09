#include "hps_sf/hps_sf.h"
#include <assert.h>

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

void test_assert() {
  HPS_LOG_INFO(g_logger) << hps_sf::BacktraceToString(10);
  HPS_ASSERT2(0 == 1, "abcdef xx");

}

int main(int argc, char** argv) {
  test_assert();
  return 0;
}