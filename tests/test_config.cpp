#include "../hps_sf/config.h"
#include "../hps_sf/log.h"
#include <yaml-cpp/yaml.h>

hps_sf::hps_ConfigVar<int>::ptr g_int_value_config = hps_sf::hps_Config::Lookup("system.port", (int)8080, "system port");
hps_sf::hps_ConfigVar<float>::ptr g_float_value_config = hps_sf::hps_Config::Lookup("system.value", (float)10.2f, "system value");

void test_yaml()
{
    YAML::Node root = YAML::LoadFile("/home/ubuntu/hps_sf/bin/conf/log.yml");

    HPS_LOG_INFO(HPS_LOG_ROOT()) << root;
}

int main(int arg, char** argv)
{
    HPS_LOG_INFO(HPS_LOG_ROOT()) << g_int_value_config -> getValue();
    HPS_LOG_INFO(HPS_LOG_ROOT()) << g_int_value_config -> toString();

    HPS_LOG_INFO(HPS_LOG_ROOT()) << g_float_value_config -> getValue();
    HPS_LOG_INFO(HPS_LOG_ROOT()) << g_float_value_config -> toString();

    test_yaml();
    return 0;
}