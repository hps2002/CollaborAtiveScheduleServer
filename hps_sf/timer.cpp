#include "timer.h"
#include "util.h"
namespace hps_sf {

hps_Timer::hps_Timer(uint64_t ms, std::function<void()> cb, bool recurring, hps_TimerManager* manager):m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager) {
  m_next = hps_sf::GetCurrentMS() + m_ms;
} 

hps_Timer::hps_Timer(uint64_t next):m_next(next) {

}

bool hps_Timer::Comparator::operator() (const hps_Timer::ptr& lhs, const hps_Timer::ptr& rhs) const {
  if (!lhs && !rhs) return false;
  if (!lhs) return true;
  if (!rhs) return false;
  if (lhs -> m_next < rhs -> m_next) return true;
  if (rhs -> m_next < rhs -> m_next) return false;
  return lhs.get() < rhs.get();
}

bool hps_Timer::cancel() {
  hps_TimerManager::RWMutexType::WriteLock lock(m_manager -> m_mutex);
  if (m_cb) {
    m_cb = nullptr;
    auto it = m_manager -> m_timers.find(shared_from_this());
    m_manager -> m_timers.erase(it);
    return true;
  }
  return false;
}

bool hps_Timer::refresh() {
  hps_TimerManager::RWMutexType::WriteLock lock(m_manager -> m_mutex);
  if (!m_cb) {
    return false;
  }

  auto it = m_manager -> m_timers.find(shared_from_this());
  if (it == m_manager -> m_timers.end())
    return false;


  // 先移除，修改再添加的原因是set不能修改键值
  m_manager -> m_timers.erase(it);
  m_next = hps_sf::GetCurrentMS() + m_ms;
  m_manager -> m_timers.insert(shared_from_this());
  return true;
}

bool hps_Timer::reset(uint64_t ms, bool from_now) {
  if (ms == m_ms && !from_now)
    return true;
  hps_TimerManager::RWMutexType::WriteLock lock(m_manager -> m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager -> m_timers.find(shared_from_this());
  if (it == m_manager -> m_timers.end())
    return false;
  // 先移除，修改再添加的原因是set不能修改键值
  m_manager -> m_timers.erase(it);
  uint64_t start = 0;
  if (from_now) {
    start = hps_sf::GetCurrentMS();
  } else {
    start = m_next - m_ms;
  }
  m_ms = ms;
  m_next = start + m_ms;
  m_manager -> addTimer(shared_from_this(), lock);
  return true;
}

hps_TimerManager::hps_TimerManager() {
  m_previousTime = hps_sf::GetCurrentMS();
}

hps_TimerManager::~hps_TimerManager() {

}

hps_Timer::ptr hps_TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
  hps_Timer::ptr timer(new hps_Timer(ms, cb, recurring, this));
  RWMutexType::WriteLock lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}

hps_Timer::ptr hps_TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring) {
  return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
} 

uint64_t hps_TimerManager::getNextTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  m_tickled = false;
  if (m_timers.empty()) {
    return ~0ull;
  }

  const hps_Timer::ptr& next = *m_timers.begin();
  uint64_t now_ms = hps_sf::GetCurrentMS();
  if(now_ms >= next -> m_next) { //如果到达或者过了执行事件就直接返回0
    return 0;
  } else {
    return next -> m_next - now_ms;
  }
}

void hps_TimerManager::listExpiredCb(std::vector<std::function<void()> >& cbs) {
  uint64_t now_ms = hps_sf::GetCurrentMS();
  std::vector<hps_Timer::ptr> expried;
  {
    RWMutexType::ReadLock lock(m_mutex);
    if (m_timers.empty()) {
      return;
    }
  }
  
  RWMutexType::WriteLock lock(m_mutex);

  bool rollover = detectClockRollver(now_ms);
  if (!rollover && ((*m_timers.begin()) -> m_next) > now_ms) {
    return ;
  } 

  hps_Timer::ptr now_timer(new hps_Timer(now_ms));

  auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);

  while (it != m_timers.end() && (*it) -> m_next == now_ms) {
    it ++;
  }

  expried.insert(expried.begin(), m_timers.begin(), it);

  m_timers.erase(m_timers.begin(), it);
  cbs.reserve(expried.size());

  for (auto& timer : expried) {
    cbs.push_back(timer -> m_cb);
    if (timer -> m_recurring) {
      timer -> m_next = now_ms + timer -> m_ms;
      m_timers.insert(timer);
    } else {
      timer -> m_cb = nullptr;
    }
  }
  
}

void hps_TimerManager::addTimer(hps_Timer::ptr val, RWMutexType::WriteLock& lock) {
  auto it = m_timers.insert(val).first;
  bool at_front = (it == m_timers.begin() && !m_tickled);
  if (at_front) {
    m_tickled = true;
  }
  lock.unlock();

  if (at_front) {
    onTimerInsertedAtFront();
  }
}

bool hps_TimerManager::detectClockRollver(uint64_t now_ms) {
  bool rollover = false;
  if (now_ms < m_previousTime && m_previousTime - 60 * 60 * 1000) {
    rollover = true;
  }
  m_previousTime = now_ms;
  return rollover;
}

bool hps_TimerManager::hasTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  return !m_timers.empty();
}

}