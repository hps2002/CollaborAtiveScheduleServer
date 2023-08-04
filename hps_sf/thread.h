#ifndef __HPS_THREAD_H__
#define __HPS_THREAD_H__

#include <thread>
#include <functional>
#include <pthread.h>
#include <memory>
#include <semaphore.h>
#include <atomic>

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

class hps_Mutex {
public:
  typedef hps_ScopedLockImpl<hps_Mutex> Lock;
  hps_Mutex() {
    pthread_mutex_init(&m_mutex, nullptr);
  }
  ~hps_Mutex() {
    pthread_mutex_destroy(&m_mutex);
  }

  void lock() {
    pthread_mutex_lock(&m_mutex);
  }

  void unlock() {
    pthread_mutex_unlock(&m_mutex);
  }
private:
  pthread_mutex_t m_mutex;
};

class hps_NullMutex {
public:
  typedef hps_ScopedLockImpl<hps_NullMutex> Lock;
  hps_NullMutex() {}
  ~hps_NullMutex() {}
  void lock() {}
  void unlock() {}
  
};


// 使用自旋锁，让其在cpu中原地等待，减少用户态和内核态的切换次数，提高性能
class hps_Spinlock {
public:
  typedef hps_ScopedLockImpl<hps_Spinlock> Lock;
  hps_Spinlock() {
    pthread_spin_init(&m_mutex, 0);
  }

  ~hps_Spinlock() {
    pthread_spin_destroy(&m_mutex);
  }

  void lock() {
    pthread_spin_lock(&m_mutex);
  }

  void unlock() {
    pthread_spin_unlock(&m_mutex);
  }

private:
  pthread_spinlock_t m_mutex;
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

class hps_NullRWMutex {
  typedef hps_ReadScopedLockImpl<hps_NullRWMutex> ReadLock;
  typedef hps_WriteScopedLockImpl<hps_NullRWMutex> WriteLock;

  hps_NullRWMutex() { }
  ~hps_NullRWMutex() { }
  void rdlock() {}
  void wrunlock() {}
  void unlock() {}
};

class hps_CASLock {
public:
  typedef hps_ScopedLockImpl<hps_CASLock> Lock;
  hps_CASLock() {
    m_mutex.clear();
  }

  ~hps_CASLock() {

  }

  void lock() {
    while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
  }

  void unlock() {
    std::atomic_flag_clear_explicit(& m_mutex, std::memory_order_release);
  }
private:
  volatile std::atomic_flag m_mutex;

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