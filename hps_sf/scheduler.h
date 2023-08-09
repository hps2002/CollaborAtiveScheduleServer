#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <memory>
#include <vector>

#include "mutex.h"
#include "fiber.h"
#include "thread.h"

namespace hps_sf {

class hps_Scheduler {
public:
  typedef std::shared_ptr<hps_Scheduler> ptr;
  typedef hps_Mutex MutexType;

  hps_Scheduler(size_t threads = 1, bool = use_caller = true, const std::string& name = "");
  virtual ~hps_Scheduler();

  const std::string& getName() const {return m_name;}
  static hps_Fiber* GetMainFiber();

  void start();
  void stop();

  template<class FiberOrCb>
  void schedule (FiberOrCb fc, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduleNoLock(fc, thread);
    }

    if (need_tickle) 
      tickle();
    
  }

  template<calss InputIterator>
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
private:
  template<class FiberOrCb>
  bool scheduleNoLock(FiberOrCb fc, int thread) {
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
    
    hps_FiberAndThread(hps_Fiber::ptr f, int thr):fiber(f), thread(thr) {}

    hps_FiberAndThread(hps_Fiber::ptr* f, int thr):thread(thr) {
      fiber.swap(*f);
    } 

    hps_FiberAndThread(std::function<void()> f, int thr):thread(thr) {
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
  std::vector<hps_Thread> m_threads;
  std::list<hps_FiberAndThread> m_fibers;
  std::string m_name;
};

}

#endif