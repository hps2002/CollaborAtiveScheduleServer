#ifndef __HPS_SF_TIMER_H__
#define __HPS_SF_TIMER_H__

#include <vector>
#include <memory>
#include <set>
#include "thread.h"
#include "mutex.h"

namespace hps_sf {
class hps_TimerManager;

class hps_Timer: public std::enable_shared_from_this<hps_Timer> {
friend class hps_TimerManager;
public:
  typedef std::shared_ptr<hps_Timer> ptr;
private:
  hps_Timer(uint64_t ms, std::function<void()> cb, bool recurring, hps_TimerManager* manager);

  hps_Timer(uint64_t next);

public:
  bool cancel();
  bool refresh();
  bool reset(uint64_t ms, bool from_now);

private:
  bool m_recurring = false;      // 是否循环定时器
  uint64_t m_ms = 0;         // 时间间隔，间隔多长时间去执行
  uint64_t m_next = 0;    
  std::function<void()> m_cb;        // 精确的执行时间
  hps_TimerManager* m_manager = nullptr;
  
private:
  struct Comparator {
    bool operator() (const hps_Timer::ptr& lhs, const hps_Timer::ptr& rhs) const;
  };
};

class hps_TimerManager {
friend class hps_Timer;
public:
  typedef hps_RWMutex RWMutexType;

  hps_TimerManager();
  virtual ~hps_TimerManager();

  hps_Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

  hps_Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false);

  uint64_t getNextTimer();
  void listExpiredCb(std::vector<std::function<void()> >& cbs);
  bool hasTimer();
protected:
  virtual void onTimerInsertedAtFront() = 0;
  void addTimer(hps_Timer::ptr val, RWMutexType::WriteLock& lock);
private:
  bool detectClockRollver(uint64_t now_ms);

private:
  RWMutexType m_mutex;
  std::set<hps_Timer::ptr, hps_Timer::Comparator> m_timers;
  bool m_tickled = false;
  uint64_t m_previousTime;
};

}
#endif