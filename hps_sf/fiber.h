#ifndef __FIBER_H__
#define __FIBER_H__

#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"

namespace hps_sf {

class hps_Scheduler;
class hps_Fiber : public std::enable_shared_from_this<hps_Fiber> {
friend class hps_Scheduler;
public:
  typedef std::shared_ptr<hps_Fiber> ptr;
	
	// 协程的状态
  enum State {
    INIT,	// 初始化
    HOLD, // 暂停
    EXEC, // 运行
    TERM, // 结束
    READY, // 准备
    EXCEPT //异常
  };

private:
  hps_Fiber();

public:
  hps_Fiber(std::function<void()> cb, size_t statcksize = 0, bool use_caller = false);
  ~hps_Fiber();

  void reset(std::function<void()> cb); //重置协程函数，并重置状态
  void swapIn(); //切换到当前协程执行
  void swapOut(); //当前协程切换到后台

  void call();
  void back();

  uint64_t getId() const {return m_id;}
  State getState() const {return m_state;}
public:
  static void SetThis(hps_Fiber* f);
  static hps_Fiber::ptr GetThis(); //返回当前协程
  static void YieldToReady(); //协程切换到后台，且设置为Ready状态
  static void YieldToHold(); //协程切换到后台，且设置为Hold状态

  static uint64_t TotalFibel(); //计算总协程数
  static void MainFunc();
  static void CallerMainFunc();
  static uint64_t GetFiberId();
private:
  uint64_t m_id = 0;
  uint32_t m_statcksize = 0;
  State m_state = INIT;

  ucontext_t m_ctx; // 当前协程的上下文
  void* m_stack = nullptr;

  std::function<void()> m_cb;
};

}

#endif
