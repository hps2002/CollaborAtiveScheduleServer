#ifndef __HPS_THREAD_H__
#define __HPS_THREAD_H__

#include <thread>
#include <functional>
#include <pthread.h>
#include <memory>
#include <semaphore.h>
#include <atomic>

#include "mutex.h"

namespace hps_sf {

class hps_Thread {
public:
  typedef std::shared_ptr<hps_Thread> ptr;
  hps_Thread(std::function<void()> cb, const std::string& name);
  ~hps_Thread();

  pid_t getId() const {return m_id;}
  const std::string getName() const {return m_name;}
  
  void join();

  static hps_Thread* GetThis(); 
  static const std::string& GetName(); 
  static void SetName (const std::string& name);
private:
  hps_Thread(const hps_Thread&) = delete;
  hps_Thread(const hps_Thread&&) = delete;
  hps_Thread& operator=(const hps_Thread&) = delete;

  static void* run(void* arg);
private:
  pid_t m_id = -1; //线程id
  pthread_t m_thread = 0;
  std::function<void()> m_cb;
  std::string m_name;

  hps_Semaphore m_semaphore; 
};



}

#endif