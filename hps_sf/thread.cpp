#include "thread.h"
#include "log.h"
#include "util.h"
#include <iostream>

namespace hps_sf {

static thread_local hps_Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static hps_sf::hps_Logger::ptr g_logger = HPS_LOG_NAME("system");


hps_Semaphore::hps_Semaphore(uint32_t count) {
  if (sem_init(&m_semaphore, 0, count)) {
    throw std::logic_error("sem_init error");
  }
}

hps_Semaphore::~hps_Semaphore() {
  sem_destroy(&m_semaphore);
}

void hps_Semaphore::wait() {
  if (sem_wait(&m_semaphore)) {
    throw std::logic_error("sem_wait error"); 
  }
}

void hps_Semaphore::notify() {
  if (sem_post(&m_semaphore)) {
    throw std::logic_error("sem_post error");
  }
}

hps_Thread* hps_Thread::GetThis() {
  return t_thread;
}

const std::string& hps_Thread::GetName() {
  return t_thread_name;
}

void hps_Thread::SetName (const std::string& name) {
  if (name.empty()) 
    return;
  if (t_thread) {
    t_thread -> m_name = name;
  }
  t_thread_name = name;
}

hps_Thread::hps_Thread(std::function<void()> cb, const std::string& name):m_cb(cb), m_name(name) {
  if (name.empty()) {
    m_name = "UNKNOW";
  }
  int rt = pthread_create(&m_thread, nullptr,  &hps_Thread::run, this);
  if(rt) {
    HPS_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt << " name=" << m_name;
    throw std::logic_error("pthread_create error");
  }
  m_semaphore.wait();
}

hps_Thread::~hps_Thread() {
  if (m_thread) {
    pthread_detach(m_thread);
  }
}

void hps_Thread::join() {
  if (m_thread) {
    int rt = pthread_join(m_thread, nullptr);
    if (rt) {
      HPS_LOG_ERROR(g_logger) << "pthread_join thread fail, rt" << rt << " name=" << m_name;
      throw std::logic_error("pthread_join error");
    }
    m_thread = 0;
  }
}


void* hps_Thread::run(void* arg) {
  hps_Thread* thread = (hps_Thread*) arg;
  t_thread = thread;
  t_thread_name = thread -> getName();
  thread -> m_id = hps_sf::GetThreadId();
  pthread_setname_np(pthread_self(), thread -> m_name.substr(0, 15).c_str());

  std::function<void()> cb;  
  cb.swap(thread -> m_cb);

  thread -> m_semaphore.notify(); 
  
  cb();
  return 0; 
}

}