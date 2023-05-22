#include "../hps_sf/config.h"
#include "../hps_sf/log.h"
#include <yaml-cpp/yaml.h>
#include <string>

hps_sf::hps_ConfigVar<int>::ptr g_int_value_config = hps_sf::hps_Config::Lookup("huang.port", (int)8080, "huang port");
hps_sf::hps_ConfigVar<float>::ptr g_float_value_config = hps_sf::hps_Config::Lookup("huang.value", (float)10.2f, "huang value");

void print_yaml (const YAML::Node& node, int level)
{
    if (node.IsScalar())
    {
        HPS_LOG_INFO(HPS_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull()) 
    {
        HPS_LOG_INFO(HPS_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); it ++)
        {
            HPS_LOG_INFO(HPS_LOG_ROOT()) << std::string(level * 4, ' ') << it -> first << " - " << it -> second.Type() << " - " << level;
            print_yaml(it -> second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); i ++)
        {
            HPS_LOG_INFO(HPS_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml()
{
    YAML::Node root = YAML::LoadFile("/home/ubuntu/hps_sf/bin/conf/log.yml");
    print_yaml(root, 0);
}

void test_config()
{
    HPS_LOG_INFO(HPS_LOG_ROOT()) << "before: " << g_int_value_config -> getValue();
    HPS_LOG_INFO(HPS_LOG_ROOT()) << "before: " << g_float_value_config -> toString();

    YAML::Node root = YAML::LoadFile("/home/ubuntu/hps_sf/bin/conf/log.yml");
    
    hps_sf::hps_Config::LoadFromYaml(root);


    HPS_LOG_INFO(HPS_LOG_ROOT()) << "after: " << g_int_value_config -> getValue();
    HPS_LOG_INFO(HPS_LOG_ROOT()) << "after: " << g_float_value_config -> toString();
}

int main(int arg, char** argv)
{
    // test_yaml();
    test_config();
    return 0;
}