#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <memory>
#include <vector>
#include <list>

#include "mutex.h"
#include "log.h"
#include "fiber.h"
#include "thread.h"


namespace hps_sf {

class hps_Scheduler:public std::enable_shared_from_this<hps_Scheduler>
{
public:
  typedef std::shared_ptr<hps_Scheduler> ptr;
  typedef hps_Mutex MutexType;

  hps_Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
  virtual ~hps_Scheduler();

  const std::string& getName() const {return m_name;}
  static hps_Fiber* GetMainFiber();
  static hps_Scheduler* GetThis();

  void start();
  void stop();

  template<class FiberOrCb>
  void schedule (FiberOrCb fc, int thread = -1) {
    hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduleNoLock(fc, thread);
    }

    if (need_tickle) 
      tickle();
    
  }

  template<class InputIterator>
  void schedule (InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      while(begin != end) {
        need_tickle = scheduleNoLock(& *begin) || need_tickle;
      }
    }
    if (need_tickle) {
      tickle();
    }
  }
protected:
  virtual void tickle();
  // 真正执行协程调度方法
  void run(); 
  virtual bool stopping();
  virtual void idle();

  void setThis();
  bool hasIdleThreads() { return m_idleThreadCount > 0;}

private:
  template<class FiberOrCb>
  bool scheduleNoLock(FiberOrCb fc, int thread) {
    hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();
    bool need_tickle = m_fibers.empty();
    hps_FiberAndThread ft(fc, thread);
    if (ft.fiber || ft.cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }
private:
  struct hps_FiberAndThread
  {
    hps_Fiber::ptr fiber;
    std::function<void()> cb;
    int thread;
    
    // 确定协程在哪个线程上跑
    hps_FiberAndThread(hps_Fiber::ptr f, int thr):fiber(f), thread(thr) {}

    // 交换fiber
    hps_FiberAndThread(hps_Fiber::ptr* f, int thr):thread(thr) {
      fiber.swap(*f);
    } 

    // 交换cb
    hps_FiberAndThread(std::function<void()> f, int thr):cb(f), thread(thr) {
    }

    hps_FiberAndThread(std::function<void()>* f, int thr):thread(thr) {
      cb.swap(*f);
    }

    hps_FiberAndThread():thread(-1) {}

    void reset() {
      fiber = nullptr;
      cb = nullptr;
      thread = -1;
    }
  };
  
private:
  MutexType m_mutex;
  std::vector<hps_Thread::ptr> m_threads;
  std::list<hps_FiberAndThread> m_fibers;
  hps_Fiber::ptr m_rootFiber; // 主协程
  std::string m_name;

protected:
  std::vector<int> m_threadIds;
  size_t m_threadCount = 0;
  std::atomic<size_t> m_activeThreadCount = {0};
  std::atomic<size_t> m_idleThreadCount = {0};
  bool m_stopping = true;
  bool m_autoStop = false;
  int m_rootThread = 0;
};

}

#endif