#include "hps_sf/hps_sf.h"


hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

int count = 0;
// hps_sf::hps_RWMutex s_mutex;
hps_sf::hps_Mutex s_mutex;
void fun1() {
  
  HPS_LOG_INFO(g_logger) << "name: " << hps_sf::hps_Thread::GetName() << 
                            " this.name: "  << hps_sf::hps_Thread::GetThis() -> getName() << 
                            " id: " << hps_sf::GetThreadId() << 
                            " this.id: " << hps_sf::hps_Thread::GetThis() -> getId();
  for (int i = 0;  i < 10000000; i ++) {
    hps_sf::hps_Mutex::Lock lock(s_mutex);
     ++ count;
  }
}

void fun2() {
  while (true) {
    HPS_LOG_INFO(g_logger) << "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh";
  }
}

void fun3 () {
  while (true) {
    HPS_LOG_INFO(g_logger) << "======================================";
  }
}

int main(int arg, char** argv) {
  HPS_LOG_INFO(g_logger) << "thread test begin";
  YAML::Node root = YAML::LoadFile("/home/ubuntu/hps_sf/bin/conf/log2.yml");
  hps_sf::hps_Config::LoadFromYaml(root);

  std::vector<hps_sf::hps_Thread::ptr> thrs;
  for (int i = 0; i < 2; i ++) {
    hps_sf::hps_Thread::ptr thr1(new hps_sf::hps_Thread(&fun2, "name_fun2_" + std::to_string(i)));
    hps_sf::hps_Thread::ptr thr2(new hps_sf::hps_Thread(&fun3, "name_fun3_" + std::to_string(i)));
    thrs.push_back(thr1);
    thrs.push_back(thr2);
  }
  for (int i = 0; i < (int)thrs.size(); i ++) {
    thrs[i] -> join();
  }
  HPS_LOG_INFO(g_logger) << "Thread test end";
  HPS_LOG_INFO(g_logger) << "count = " << count;
  return 0;
}