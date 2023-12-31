#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace hps_sf {

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_NAME("system");

static thread_local hps_Scheduler* t_scheduler = nullptr;
static thread_local hps_Fiber* t_scheduler_fiber = nullptr;

hps_Scheduler::hps_Scheduler(size_t threads, bool use_caller, const std::string& name):m_name(name){
  HPS_ASSERT(threads > 0);
  if (use_caller) {
    hps_sf::hps_Fiber::GetThis(); // 如果没有协程的话会初始化一个主协程
     -- threads;

    HPS_ASSERT(GetThis() == nullptr);
    t_scheduler = this;

    m_rootFiber.reset(new hps_Fiber(std::bind(&hps_Scheduler::run, this), 0, true));
    hps_sf::hps_Thread::SetName(m_name);

    t_scheduler_fiber = m_rootFiber.get();
    m_rootThread = hps_sf::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }
  m_threadCount = threads;
}

hps_Scheduler::~hps_Scheduler() {
  // HPS_LOG_DEBUG(g_logger) << "m_stopping = " << m_stopping;
  HPS_ASSERT(m_stopping);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

hps_Scheduler* hps_Scheduler::GetThis() {
  return t_scheduler;
}

hps_Fiber* hps_Scheduler::GetMainFiber() {
  return t_scheduler_fiber;
}

void hps_Scheduler::start() {
  // 启动线程
  MutexType::Lock lock(m_mutex);

  if (!m_stopping) {// 启动失败
    return;
  } 
  m_stopping = false;

  HPS_ASSERT(m_threads.empty());

  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; i ++) {
    m_threads[i].reset(new hps_Thread(std::bind(&hps_Scheduler::run, this), m_name + "_" + std::to_string(i)));
    m_threadIds.push_back(m_threads[i] -> getId());
  }
  
  lock.unlock();

  // if (m_rootFiber) {
  //   // m_rootFiber -> swapIn();
  //   m_rootFiber -> call();
  //   HPS_LOG_INFO(g_logger) << "call out" << m_rootFiber -> getState();
  // }
}

void hps_Scheduler::stop() {
  m_autoStop = true;
  
  // 当Scheduler的run执行的rootFiber的状态时结束（TERN）或者是初始化状态的时候就是stop
  if (m_rootFiber && m_threadCount == 0 && (m_rootFiber -> getState() == hps_Fiber::TERM || m_rootFiber -> getState() == hps_Fiber::INIT)) {
    HPS_LOG_INFO(g_logger) << this << " stopped";
    m_stopping = true;

    if (stopping()) {
      return;
    }
  }

  // bool exit_on_this_fiber = false;
  if (m_rootThread != -1) {
    HPS_ASSERT(GetThis() == this);
  } else {
    HPS_ASSERT(GetThis() != this);
  }

  m_stopping = true;
  for (size_t i = 0; i < m_threadCount; i ++) {
    tickle(); //唤醒线程，把自己结束
  }

  if (m_rootFiber) {
    tickle();
  }

  if (m_rootFiber) {
    // while(!stopping()) {
    //   if (m_rootFiber -> getState() == hps_Fiber::TERM || m_rootFiber -> getState() == hps_Fiber::EXCEPT) {
    //     m_rootFiber.reset(new hps_Fiber(std::bind(&hps_Scheduler::run, this), 0, true));
    //     HPS_LOG_INFO(g_logger) << "root fiber is term, reset";
    //     t_fiber = m_rootFiber.get();
    //   }
    //   m_rootFiber -> call();
    // }

    if (!stopping()) { 
      m_rootFiber -> call();
    }
  }

  std::vector<hps_Thread::ptr> thrs;
  {
    MutexType::Lock lock(m_mutex);
    thrs.swap(m_threads);
  }

  for (auto& i : thrs) {
    i -> join();
  }

  // if (exit_on_this_fiber) {

  // }

}

void hps_Scheduler::setThis() {
  t_scheduler = this;
}

void hps_Scheduler::run() {
  HPS_LOG_INFO(g_logger) << "run";
  setThis();
  hps_sf::set_hook_enable(true);

  if (hps_sf::GetThreadId() != m_rootThread) {
    t_scheduler_fiber = hps_Fiber::GetThis().get();
  }

  hps_Fiber::ptr idle_fiber(new hps_Fiber(std::bind(&hps_Scheduler::idle, this)));
  hps_Fiber::ptr cb_fiber;

  hps_FiberAndThread ft;
  // HPS_LOG_DEBUG(g_logger) << "m_fiber.size() = " << m_fibers.size();
  while (1) {
    // HPS_LOG_DEBUG(g_logger) << "enter pick fibers queue";
    ft.reset();
    bool tickle_me = false;
    bool is_active = false;
    // 取出一个需要执行的fiber
    {
      MutexType::Lock lock(m_mutex);
      auto it = m_fibers.begin();
      while(it != m_fibers.end()) {
        if (it -> thread != -1 && it -> thread != hps_sf::GetThreadId()) {
          it ++;
          tickle_me = true;
          continue;
        }
        HPS_ASSERT(it -> fiber || it -> cb);
        if (it -> fiber && it -> fiber -> getState() == hps_Fiber::EXEC) {
          it ++;
          continue;
        }

        ft = *it;
        m_fibers.erase(it);
        m_activeThreadCount ++;
        is_active = true;
        break;
      }
    }

    if (tickle_me) {
      tickle();
    }

    // HPS_LOG_DEBUG(g_logger) << "ft.fiber = " << ft.fiber << ", fiber.state = " << ft.fiber -> getState();
    if (ft.fiber && (ft.fiber -> getState() != hps_Fiber::TERM || ft.fiber -> getState() != hps_Fiber::EXCEPT)) {
      ft.fiber -> swapIn();
      m_activeThreadCount --;

      if (ft.fiber -> getState() == hps_Fiber::READY) {
        schedule(ft.fiber);
      } else if (ft.fiber -> getState() != hps_Fiber::TERM && ft.fiber -> getState() != hps_Fiber::EXCEPT) {
        ft.fiber -> m_state = hps_Fiber::HOLD;
      }
      ft.reset();
    } else if (ft.cb) {
      
      if (cb_fiber) {
        cb_fiber -> reset(ft.cb);
      } else  {
        cb_fiber.reset(new hps_Fiber(ft.cb));
      }

      ft.reset();
      cb_fiber -> swapIn();
      m_activeThreadCount --;

      if (cb_fiber -> getState() == hps_Fiber::READY) {
        schedule(cb_fiber);
        cb_fiber.reset();
      } else if (cb_fiber -> getState() == hps_Fiber::EXCEPT || cb_fiber -> getState() == hps_Fiber::TERM) {
        cb_fiber -> reset(nullptr);
      } else {
        cb_fiber -> m_state = hps_Fiber::HOLD;
        cb_fiber.reset();
      }
    } else {
      if (is_active) {
        m_activeThreadCount --;
        continue;
      }
      if (idle_fiber -> getState() == hps_Fiber::TERM) {
        // HPS_LOG_INFO(g_logger) << "idle fiber term";
        break;
      }

      m_idleThreadCount ++;
      idle_fiber -> swapIn();
      m_idleThreadCount --;
      if (idle_fiber -> getState() != hps_Fiber::TERM && idle_fiber -> getState() != hps_Fiber::EXCEPT) {
        idle_fiber -> m_state = hps_Fiber::HOLD;
      }  
    }
  }
}

void hps_Scheduler::tickle() {
  HPS_LOG_INFO(g_logger) << "tickle";
}

bool hps_Scheduler::stopping() {
  MutexType::Lock lock(m_mutex);
  return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
}

void hps_Scheduler::idle() {
  HPS_LOG_INFO(g_logger) << "idle";
  while (!stopping()) {
    hps_sf::hps_Fiber::YieldToHold();
  }
}

}