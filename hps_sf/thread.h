#ifndef __HPS_THREAD_H__
#define __HPS_THREAD_H__

#include <thread>
#include <functional>
#include <pthread.h>
#include <memory>
#include <semaphore.h>

namespace hps_sf {

class hps_Semaphore {
public:
  hps_Semaphore(uint32_t count = 1);
  ~hps_Semaphore();

  void wait();
  void notify();

private:
  hps_Semaphore(const hps_Semaphore&) = delete;
  hps_Semaphore(const hps_Semaphore&&) = delete;
  hps_Semaphore& operator=(const hps_Semaphore&) = delete;

private:
  sem_t m_semaphore;
};

template <class T>
struct hps_ScopedLockImpl {
public:
  hps_ScopedLockImpl(T& mutex):m_mutex(mutex) {
    m_mutex.lock();
    m_locked = true;
  }
  
  ~hps_ScopedLockImpl() {
    unlock();
    m_locked = false;
  }

  void lock () {
    if (!m_locked) {
      m_mutex.lock();
      m_locked = true;
    }
  }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }
private:
  T& m_mutex;
  bool m_locked;
};

template<class T>
 struct hps_ReadScopedLockImpl {
public:
  hps_ReadScopedLockImpl(T& mutex):m_mutex(mutex) {
    m_mutex.rdlock();
    m_locked = true;
  }
  
  ~hps_ReadScopedLockImpl() {
    unlock();
    m_locked = false;
  }

  void lock () {
    if (!m_locked) {
      m_mutex.rdlock();
      m_locked = true;
    }
  }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }
private:
  T& m_mutex;
  bool m_locked;
};

template<class T>
struct hps_WriteScopedLockImpl {
public:
  hps_WriteScopedLockImpl(T& mutex):m_mutex(mutex) {
    m_mutex.wrlock();
    m_locked = true;
  }
  
  ~hps_WriteScopedLockImpl() {
    unlock();
    m_locked = false;
  }

  void lock () {
    if (!m_locked) {
      m_mutex.wrlock();
      m_locked = true;
    }
  }

  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }
private:
  T& m_mutex;
  bool m_locked;
};

class hps_RWMutex {
public:
  typedef hps_ReadScopedLockImpl<hps_RWMutex> ReadLock;
  typedef hps_WriteScopedLockImpl<hps_RWMutex> WriteLock;
  hps_RWMutex() {
    pthread_rwlock_init(&m_lock, nullptr);
  }

  ~hps_RWMutex () {
    pthread_rwlock_destroy(&m_lock);
  }

  void rdlock() {
    pthread_rwlock_rdlock(&m_lock);
  }

  void wrlock() {
    pthread_rwlock_wrlock(&m_lock);
  }

  void unlock() {
    pthread_rwlock_unlock(&m_lock);
  }
private:
  pthread_rwlock_t m_lock;
};

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