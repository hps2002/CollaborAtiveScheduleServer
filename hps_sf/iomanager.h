#ifndef __HPS_IOMANAGER_H__
#define __HPS_IOMANAGER_H__

#include "scheduler.h"
#include "mutex.h"
#include "fiber.h"

namespace hps_sf {

class hps_IOManager: public hps_Scheduler {
public:
  typedef std::shared_ptr<hps_IOManager> ptr;
  typedef hps_RWMutex RWMutexType;

  enum hps_Event {
    NONE    = 0x0,
    READ    = 0x1, //等于EPOLLIN
    WRITE   = 0x4, //等于EPOLLOUT
  };

private:

  struct hps_FdContext {  
  public:  
    typedef hps_Mutex MutexType;
      struct hps_EventContext {
        hps_Scheduler* scheduler = nullptr;   // 事件执行的scheduler
        hps_Fiber::ptr fiber;       // 事件协程
        std::function<void()> cb; // 事件回调函数
      };

      hps_EventContext& getContext (hps_Event event);
      void resetContext(hps_EventContext& ctx);
      void triggerEvent(hps_Event event);

                  
      hps_EventContext read;    // 读事件
      hps_EventContext write;   // 写事件
      int fd = 0;               // 事件关联句柄
      hps_Event events = NONE;  // 已经注册的事件  
      MutexType mutex; 
  };


public:
  hps_IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
  ~hps_IOManager();

  // 1 成功，0 重试， -1 错误
  int addEvent(int fd, hps_Event event, std::function<void()> cb = nullptr);
  bool delEvent(int fd, hps_Event event); 
  bool cancelEvent(int fd, hps_Event event);

  bool cancelAll(int fd);

  static hps_IOManager* GetThis(); // 获取线程当前IOmanager

protected:
  void tickle() override;
  bool stopping() override;
  void idle() override;

  void contextResize(size_t size);
private:
  int m_epfd = 0;
  int m_tickleFds[2];

  std::atomic<size_t> m_pendingEventCount = {0};
  RWMutexType m_mutex;
  std::vector<hps_FdContext*> m_fdContexts;
};
}
#endif