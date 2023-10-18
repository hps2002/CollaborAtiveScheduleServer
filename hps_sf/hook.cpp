#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include <dlfcn.h>
#include <iostream>

namespace hps_sf {

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep)

void hook_init() {
  static bool is_inited = false;
  if (is_inited) 
    return ;

#define XX(name) name ## _f = (name ## _fun) dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

struct _HookIniter {
  _HookIniter() {
    hook_init();
  }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
  return t_hook_enable;
}

void set_hook_enable(bool flag) {
  t_hook_enable = flag;
}

}

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX
}

unsigned int sleep(unsigned int seconds) {
  if (!hps_sf::t_hook_enable) {
    return sleep_f(seconds);
  }

  hps_sf::hps_Fiber::ptr fiber = hps_sf::hps_Fiber::GetThis();
  hps_sf::hps_IOManager* iom = hps_sf::hps_IOManager::GetThis();
  iom -> addTimer(seconds * 1000, [iom, fiber, seconds](){
    HPS_LOG_DEBUG(HPS_LOG_ROOT()) << "增加的是" << seconds << "秒的定时器";
    iom -> schedule(fiber);
  });
  hps_sf::hps_Fiber::YieldToHold();
  return 0;
}

int usleep(useconds_t usec) {   
  if (!hps_sf::t_hook_enable) {
    return usleep_f(usec);
  }

  hps_sf::hps_Fiber::ptr fiber = hps_sf::hps_Fiber::GetThis();
  hps_sf::hps_IOManager* iom = hps_sf::hps_IOManager::GetThis();
  iom -> addTimer(usec / 1000, [iom, fiber]() {
    iom -> schedule(fiber);
  });
  hps_sf::hps_Fiber::YieldToHold();
  return 0;
}