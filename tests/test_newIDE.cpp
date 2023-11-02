#include "hps_sf/hook.h"
#include "hps_sf/fiber.h"
#include "hps_sf/iomanager.h"
#include "hps_sf/log.h"

static hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

void test() {
//    int a = 0;
    std::cout << "Hello World!" << std::endl;
}

int main() {
    test();
    return 0;
}