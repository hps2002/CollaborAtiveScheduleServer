#include "fiber.h"
#include "config.h"
#include "macro.h"
#include <atomic>
#include "sched.h"
#include "scheduler.h"

#include "log.h"

namespace hps_sf {

static hps_Logger::ptr g_logger = HPS_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local hps_Fiber* t_fiber = nullptr;
static thread_local hps_Fiber::ptr t_threadFiber = nullptr;

static hps_ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
                hps_Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class hps_MallocStackAlloctor {
public:
  static void* Alloc(size_t size) {
    return malloc(size);
  }

  static void Dealloc(void* vp, size_t size) {
    return free(vp);
  }
};

using StackAlloctor = hps_MallocStackAlloctor;

uint64_t hps_Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber -> getId();
  }
  return 0;
}

// 主协程
hps_Fiber::hps_Fiber() {
  m_state = EXEC;
  SetThis(this);

  if (getcontext(&m_ctx)) {
    HPS_ASSERT2(false, "getcontex");
  }

  s_fiber_count ++;

  HPS_LOG_DEBUG(g_logger) << "Fiber::Fiber";
}

// 新生成的协程
hps_Fiber::hps_Fiber(std::function<void()> cb, size_t statcksize, bool use_caller):m_id(++ s_fiber_id), m_cb(cb) 
{

  ++ s_fiber_count;
  m_statcksize = statcksize ? statcksize : g_fiber_stack_size -> getValue();

  m_stack = StackAlloctor::Alloc(m_statcksize);
  if (getcontext(&m_ctx)) {
    HPS_ASSERT2(false, "getcontext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_statcksize;

  if (!use_caller) {
    makecontext(&m_ctx, &hps_Fiber::MainFunc, 0);
  } else {
    makecontext(&m_ctx, &hps_Fiber::CallerMainFunc, 0);
  }

  HPS_LOG_DEBUG(g_logger) << "Fiber::Fiber Id = " << m_id;
}

hps_Fiber::~hps_Fiber() {
  -- s_fiber_count;
  if (m_stack) {
    HPS_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
    StackAlloctor::Dealloc(m_stack, m_statcksize);
  } else {
    HPS_ASSERT(!m_cb);
    HPS_ASSERT(m_state == EXEC);

    hps_Fiber* cur = t_fiber;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
  HPS_LOG_DEBUG(g_logger) << "Fiber::~Fiber id = " << m_id;
}

void hps_Fiber::reset(std::function<void()> cb) {
  HPS_ASSERT(m_stack);
  HPS_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
  m_cb = cb;
  if (getcontext(&m_ctx)) {
    HPS_ASSERT2(false, "getcontext");
  }
  
  m_ctx.uc_link = nullptr;
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_statcksize;

  makecontext(&m_ctx, &hps_Fiber::MainFunc, 0);
  m_state = INIT;
}   

// 当前协程强行切成目标执行协程
void hps_Fiber::call() {
  SetThis(this);
  m_state = EXEC;
  HPS_LOG_ERROR(g_logger) << getId();
  if (swapcontext(&t_threadFiber -> m_ctx, &m_ctx)) {
    HPS_ASSERT2(false, "swapcontext");
  }
}

void hps_Fiber::swapIn() {
  SetThis(this);
  HPS_ASSERT(m_state != EXEC);
  m_state = EXEC;
  // HPS_LOG_DEBUG(g_logger) << "GetMainFiber -> m_ctx: " << &hps_Scheduler::GetMainFiber() -> m_ctx << ", m_ctx: " << &m_ctx;
  if (swapcontext(&hps_Scheduler::GetMainFiber() -> m_ctx , &m_ctx)) {
    HPS_ASSERT2(false, "swapcontext");
  }
}

void hps_Fiber::back() {
  SetThis(t_threadFiber.get());
  if (swapcontext(&m_ctx, &t_threadFiber -> m_ctx)) {
    HPS_ASSERT2(false, "swapcontext");
  }
}


void hps_Fiber::swapOut() {
  SetThis(hps_Scheduler::GetMainFiber());
  if (swapcontext(&m_ctx, &hps_Scheduler::GetMainFiber() -> m_ctx))
    HPS_ASSERT2(false, "swapcontext");
}

void hps_Fiber::SetThis(hps_Fiber* f) {
  t_fiber = f;
}

hps_Fiber::ptr hps_Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber -> shared_from_this();
  }
  hps_Fiber::ptr main_fiber(new hps_Fiber);
  HPS_ASSERT(t_fiber == main_fiber.get());
  t_threadFiber = main_fiber;
  return t_fiber -> shared_from_this();
}

void hps_Fiber::YieldToReady() {
  hps_Fiber::ptr cur = GetThis();
  cur -> m_state = READY;
  cur -> swapOut();
}

void hps_Fiber::YieldToHold() {
  hps_Fiber::ptr cur = GetThis();
  cur -> m_state = HOLD;
  cur -> swapOut();
}

uint64_t hps_Fiber::TotalFibel() {
  return s_fiber_count;
}

void hps_Fiber::MainFunc() {
  hps_Fiber::ptr cur = GetThis();
  HPS_ASSERT(cur);
  try {
    cur -> m_cb();
    cur -> m_cb = nullptr;
    cur -> m_state = TERM;
  } catch (std::exception& e) {
    cur -> m_state = EXCEPT;
    HPS_LOG_ERROR(g_logger) << "Fiber Except: " << e.what()
                            << "fiberId = " << cur ->getId()
                            << std::endl
                            << hps_sf::BacktraceToString();
  } catch (...) {
    cur -> m_state = EXCEPT;
    HPS_LOG_ERROR(g_logger) << "Fiber Except";
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr -> swapOut();

  HPS_ASSERT2(false, "never reach, fiberId = " + std::to_string(raw_ptr -> getId()));
}

void hps_Fiber::CallerMainFunc() {
  hps_Fiber::ptr cur = GetThis();
  HPS_ASSERT(cur);
  try {
    cur -> m_cb();
    cur -> m_cb = nullptr;
    cur -> m_state = TERM;
  } catch (std::exception& e) {
    cur -> m_state = EXCEPT;
    HPS_LOG_ERROR(g_logger) << "Fiber Except: " << e.what()
                            << "fiberId = " << cur ->getId()
                            << std::endl
                            << hps_sf::BacktraceToString();
  } catch (...) {
    cur -> m_state = EXCEPT;
    HPS_LOG_ERROR(g_logger) << "Fiber Except";
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr -> back();

  HPS_ASSERT2(false, "never reach, fiberId = " + std::to_string(raw_ptr -> getId()));
}

}