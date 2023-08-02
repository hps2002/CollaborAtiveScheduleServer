#include "hps_sf/hps_sf.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

int count = 0;
hps_sf::hps_RWMutex s_mutex;
void fun1() {
  
  HPS_LOG_INFO(g_logger) << "name: " << hps_sf::hps_Thread::GetName() << 
                            " this.name: "  << hps_sf::hps_Thread::GetThis() -> getName() << 
                            " id: " << hps_sf::GetThreadId() << 
                            " this.id: " << hps_sf::hps_Thread::GetThis() -> getId();
  for (int i = 0;  i < 10000000; i ++) {
    hps_sf::hps_RWMutex::WriteLock lock(s_mutex);
     ++ count;
  }
}

void fun2() {

}

int main(int arg, char** argv) {
  HPS_LOG_INFO(g_logger) << "thread test begin";
  std::vector<hps_sf::hps_Thread::ptr> thrs;
  for (int i = 0; i < 5; i ++) {
    hps_sf::hps_Thread::ptr thr(new hps_sf::hps_Thread(&fun1, "name_" + std::to_string(i)));
    thrs.push_back(thr);
  }
  for (int i = 0; i < 5; i ++) {
    thrs[i] -> join();
  }
  HPS_LOG_INFO(g_logger) << "Thread test end";
  HPS_LOG_INFO(g_logger) << "count = " << count;
  return 0;
}