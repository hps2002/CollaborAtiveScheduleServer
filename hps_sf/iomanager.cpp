#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h> // 修改句柄的一些属性
#include <error.h>
#include <string.h>

#include "log.h"
#include "iomanager.h"
#include "macro.h"
#include "fiber.h"


namespace hps_sf {
  
static hps_sf::hps_Logger::ptr g_logger = HPS_LOG_NAME("system");

hps_IOManager::hps_FdContext::hps_EventContext& hps_IOManager::hps_FdContext::getContext (hps_IOManager::hps_Event event) {
  switch(event) {
    case hps_IOManager::READ:
        return read;
    case hps_IOManager::WRITE:
        return write;
    default:
        HPS_ASSERT2(false, "getContext");
  }
}

void hps_IOManager::hps_FdContext::resetContext(hps_EventContext& ctx) {
  ctx.scheduler = nullptr;
  ctx.fiber.reset();
  ctx.cb = nullptr;
}

void hps_IOManager::hps_FdContext::triggerEvent(hps_IOManager::hps_Event event) {
  HPS_ASSERT(events & event);
  events = (hps_Event)(events & ~event);
  hps_EventContext& ctx = getContext(event);
  if (ctx.cb) {
    ctx.scheduler -> schedule(&ctx.cb);
  } else {
    ctx.scheduler -> schedule(&ctx.fiber);
  }
  ctx.scheduler = nullptr;
  return ;
}

hps_IOManager::hps_IOManager(size_t threads, bool use_caller, const std::string& name):hps_Scheduler(threads, use_caller, name) {
  m_epfd = epoll_create(5000);
  HPS_ASSERT(m_epfd > 0);

  int rt = pipe(m_tickleFds);
  HPS_ASSERT(!rt);
  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  
  event.events = EPOLLIN | EPOLLET; // 使用ET触发模式，只提醒一次
  event.data.fd = m_tickleFds[0];
  
  rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK); // 修改句柄读的属性, 设置成异步
  HPS_ASSERT(!rt);

  rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
  HPS_ASSERT(!rt);

  contextResize(32);
  // m_fdContexts.resize(64); // 大小设置为64

  // 启动调度器
  start();
}

hps_IOManager::~hps_IOManager() {
  stop();
  close(m_epfd);
  close(m_tickleFds[0]);
  close(m_tickleFds[1]);

  for (size_t i = 0; i < m_fdContexts.size(); i ++) {
    if (m_fdContexts[i]) {
      delete m_fdContexts[i];
    }
  }
}
void hps_IOManager::contextResize(size_t size) {
  m_fdContexts.resize((int)size);

  for (size_t i = 0; i < m_fdContexts.size(); i ++)
    if (!m_fdContexts[i]) {
      m_fdContexts[i] = new hps_FdContext;
      m_fdContexts[i] -> fd = i;
      // m_fdContexts[i] -> write.scheduler = new hps_Scheduler;
      // m_fdContexts[i] -> read.scheduler = new hps_Scheduler;
    }
}

int hps_IOManager::addEvent(int fd, hps_Event event, std::function<void()> cb) {
  hps_FdContext* fd_ctx = nullptr;
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() > fd) { 
      fd_ctx = m_fdContexts[fd];
      lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteLock lock2(m_mutex);
    contextResize(fd * 1.5);
    fd_ctx = m_fdContexts[fd];
  }

  hps_FdContext::MutexType::Lock lock2(fd_ctx -> mutex);
  if (fd_ctx -> events & event) {
    HPS_LOG_ERROR(g_logger) << "addEvent assert fd = " << fd <<
                                "event = " << event << 
                                " fd_ctx.evetn = " << fd_ctx -> events;
    HPS_ASSERT(!(fd_ctx -> events & event));
  }

  int op = fd_ctx -> events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx -> events | event; // 新的event
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HPS_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << 
                                op << ", " << fd << ", " << epevent.events << "):"
                                << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return -1; 
  }

  m_pendingEventCount ++;
  fd_ctx -> events = (hps_Event) (fd_ctx  -> events | event);
  hps_FdContext::hps_EventContext& event_ctx = fd_ctx -> getContext(event);
  HPS_ASSERT(!event_ctx.scheduler 
            && !event_ctx.fiber 
            && !event_ctx.cb);

  event_ctx.scheduler = hps_Scheduler::GetThis();
  if (cb) {
    event_ctx.cb.swap(cb);
  } else {
    event_ctx.fiber = hps_Fiber::GetThis();
    HPS_ASSERT(event_ctx.fiber -> getState() == hps_Fiber::EXEC);
  }
  return 0;
}

bool hps_IOManager::delEvent(int fd, hps_Event event) {
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
      return false;
  } 
  hps_FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();
  hps_FdContext::MutexType::Lock lock2(fd_ctx -> mutex);
  if (!(fd_ctx -> events & event)) {
    return false;
  }

  hps_Event new_events = (hps_Event)(fd_ctx -> events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HPS_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << 
                                op << ", " << fd << ", " << epevent.events << "):"
                                << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }
  m_pendingEventCount --;
  fd_ctx -> events = new_events;
  hps_FdContext::hps_EventContext& event_ctx = fd_ctx -> getContext(event);
  fd_ctx -> resetContext(event_ctx); 
  return true;
}
bool hps_IOManager::cancelEvent(int fd, hps_Event event) {
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
      return false;
  } 
  hps_FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();
  hps_FdContext::MutexType::Lock lock2(fd_ctx -> mutex);
  if (!(fd_ctx -> events & event)) {
    return false;
  }

  hps_Event new_events = (hps_Event)(fd_ctx -> events & ~event);
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HPS_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << 
                                op << ", " << fd << ", " << epevent.events << "):"
                                << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }
  fd_ctx -> triggerEvent(event);
  m_pendingEventCount --;
  return true;
}

bool hps_IOManager::cancelAll(int fd) {
  RWMutexType::ReadLock lock(m_mutex);
  if ((int)m_fdContexts.size() <= fd) {
      return false;
  } 
  hps_FdContext* fd_ctx = m_fdContexts[fd];
  lock.unlock();
  hps_FdContext::MutexType::Lock lock2(fd_ctx -> mutex);
  if (!fd_ctx -> events) {
    return false;
  }

  int op = EPOLL_CTL_DEL;
  epoll_event epevent;
  epevent.events = 0;
  epevent.data.ptr = fd_ctx;

  int rt = epoll_ctl(m_epfd, op, fd, &epevent);
  if (rt) {
    HPS_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << 
                                op << ", " << fd << ", " << epevent.events << "):"
                                << rt << " (" << errno << ") (" << strerror(errno) << ")";
    return false;
  }

  if (fd_ctx -> events & READ) {
    fd_ctx -> triggerEvent(READ);
    m_pendingEventCount --;
  }
  if (fd_ctx -> events & WRITE) {
    fd_ctx -> triggerEvent(WRITE);
    m_pendingEventCount --;
  }

  HPS_ASSERT(fd_ctx -> events == 0);
  return true;
}

hps_IOManager* hps_IOManager::GetThis() {
  return dynamic_cast<hps_IOManager*>(hps_Scheduler::GetThis());
}

// 当外面有协程需要运行的时候会触发
void hps_IOManager::tickle() {
  if (!hasIdleThreads())
    return ;
  int rt = write(m_tickleFds[1], "T", 1);
  HPS_ASSERT(rt == 1);
}

bool hps_IOManager::stopping() {
  uint64_t timeout = 0;
  return stopping(timeout);
}

bool hps_IOManager::stopping(uint64_t& timeout) {
  timeout = getNextTimer();
  return timeout == ~0ull
          && m_pendingEventCount == 0
          && hps_Scheduler::stopping();
}

// 线程什么事都不干的时候会陷入idle
void hps_IOManager::idle() {
  epoll_event* events = new epoll_event[64];
  std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
    delete[] ptr;
  });

  while (true) {
    auto next_timeout = 0ull;
    if (stopping()) {
      next_timeout = getNextTimer();
      if (next_timeout == ~0ull) {
        HPS_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
        break;
      }
    }

    int rt = 0;
    do {
      static const int MAX_TIMEOUT = 5000;
      if (next_timeout != ~0ull) {
        next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
      } else {
        next_timeout = MAX_TIMEOUT;
      }
      rt = epoll_wait(m_epfd, events, 64, (int)next_timeout); 
      // HPS_LOG_DEBUG(g_logger) << "rt = " << rt;

      if (rt < 0 && errno == EINTR) {

      } else {
        break;
      }
    } while(true);

    std::vector<std::function<void()> > cbs;
    listExpiredCb(cbs);
    if (!cbs.empty()) {
      schedule(cbs.begin(), cbs.end());
      cbs.clear();
    }

    for(int i = 0; i < rt; i ++) {
      epoll_event& event = events[i];
      if (event.data.fd == m_tickleFds[0]) {
        uint8_t dummy;
        while (read(m_tickleFds[0], &dummy, 1) == 1);
        continue;
      }

      hps_FdContext* fd_ctx = (hps_FdContext*)event.data.ptr;

      hps_FdContext::MutexType::Lock lock(fd_ctx -> mutex);
      if(event.events & (EPOLLERR | EPOLLHUP)) {
        event.events |= EPOLLIN | EPOLLOUT;
      }
      int real_events = NONE;
      if (event.events & EPOLLIN) {
        real_events |= READ;
      }
      if (event.events & EPOLLOUT) {
        real_events |= WRITE;
      }

      if ((fd_ctx -> events & real_events) == NONE) {
        continue;
      }

      int left_events = (fd_ctx -> events & ~real_events);
      int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      event.events = EPOLLET | left_events;

      int rt2 = epoll_ctl(m_epfd, op, fd_ctx -> fd, &event);
      if (rt2) {
        HPS_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << 
                                  op << ", " << fd_ctx -> fd << ", " << event.events << "):"
                                  << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
        continue;
      }

      if (real_events & READ) {
        fd_ctx -> triggerEvent(READ);
        m_pendingEventCount --;
      } 
      if (real_events & WRITE) {
        fd_ctx -> triggerEvent(WRITE);
        m_pendingEventCount --;
      }
    }

    // 拿出事件之后开始执行协程
    hps_Fiber::ptr cur = hps_Fiber::GetThis();
    auto raw_ptr = cur.get();
    cur.reset();

    raw_ptr -> swapOut();
  }
}

void hps_IOManager::onTimerInsertedAtFront() {
  tickle();

}


}