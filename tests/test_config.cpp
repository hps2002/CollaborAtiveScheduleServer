#include "../hps_sf/config.h"
#include "../hps_sf/log.h"
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <list>

hps_sf::hps_ConfigVar<int>::ptr g_int_value_config = hps_sf::hps_Config::Lookup("system.port", (int)8080, "system port");
hps_sf::hps_ConfigVar<float>::ptr g_float_value_config = hps_sf::hps_Config::Lookup("huang.value", (float)10.2f, "huang value");

hps_sf::hps_ConfigVar<std::vector<int> >::ptr g_vector_value_config = hps_sf::hps_Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "huang int vec");

hps_sf::hps_ConfigVar<std::list<int> >::ptr g_list_value_config = hps_sf::hps_Config::Lookup("system.int_list", std::list<int>{1, 2}, "huang int list");

hps_sf::hps_ConfigVar<std::set<int> >::ptr g_set_value_config = hps_sf::hps_Config::Lookup("system.int_set", std::set<int>{1, 2}, "huang int set");

hps_sf::hps_ConfigVar<std::unordered_set<int> >::ptr g_unordered_set_value_config = hps_sf::hps_Config::Lookup("system.int_unordered_set", std::unordered_set<int>{1, 2}, "huang int unordered_set");

hps_sf::hps_ConfigVar<std::map<std::string, int> >::ptr g_string_int_map_value_config = hps_sf::hps_Config::Lookup("system.str_int_map", std::map<std::string, int>{{"huang", 1}, {"pei", 2}, {"shen", 3}}, "huang int map");

hps_sf::hps_ConfigVar<std::unordered_map<std::string, int> >::ptr g_string_int_umap_value_config = hps_sf::hps_Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"huang", 1}, {"pei", 2}, {"shen", 3}}, "huang int umap");

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
    // HPS_LOG_INFO(HPS_LOG_ROOT()) << "before: " << g_int_value_config -> getValue();
    // HPS_LOG_INFO(HPS_LOG_ROOT()) << "before: " << g_float_value_config -> toString();
#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var -> getValue(); \
        for (auto& p : v) \
            HPS_LOG_INFO(HPS_LOG_ROOT()) << #prefix " " #name ":" << p; \
        HPS_LOG_INFO(HPS_LOG_ROOT()) << #prefix " " #name ":" << g_var -> toString(); \
    } 

#define XX_MAP(g_var, name, prefix) \
    {                               \
        auto& v = g_var -> getValue(); \
        for (auto& i : v)           \
            HPS_LOG_INFO(HPS_LOG_ROOT()) << #prefix " " << #name ":{" << i.first << ":" << i.second << "}"; \
        HPS_LOG_INFO(HPS_LOG_ROOT()) << #prefix " " #name ":" << g_var -> toString(); \
    }

    XX(g_vector_value_config, int_vec, before);
    XX(g_list_value_config, int_list, before);
    XX(g_set_value_config, int_set, before);
    XX(g_unordered_set_value_config, int_unordered_set, before);
    XX_MAP(g_string_int_map_value_config, int_str_int_map, before);
    XX_MAP(g_string_int_umap_value_config, int_str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/ubuntu/hps_sf/bin/conf/log.yml");
    
    hps_sf::hps_Config::LoadFromYaml(root);

    // HPS_LOG_INFO(HPS_LOG_ROOT()) << "after: " << g_int_value_config -> getValue();
    // HPS_LOG_INFO(HPS_LOG_ROOT()) << "after: " << g_float_value_config -> toString();

    XX(g_vector_value_config, int_vec, after);
    XX(g_list_value_config, int_list, after);
    XX(g_set_value_config, int_set, after);
    XX(g_unordered_set_value_config, int_unordered_set, after);
    XX_MAP(g_string_int_map_value_config, int_str_int_map, after);
    XX_MAP(g_string_int_umap_value_config, int_str_int_umap, after);

}

int main(int arg, char** argv)
{
    // test_yaml();
    test_config();
    return 0;
}