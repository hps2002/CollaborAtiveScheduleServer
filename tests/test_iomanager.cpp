#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "hps_sf/hps_sf.h"
#include "hps_sf/iomanager.h"

hps_sf::hps_Logger::ptr g_logger = HPS_LOG_ROOT();

int sock = 0;

void test_fiber() {
  HPS_LOG_INFO(g_logger) << "test_fiber begin()";
  sock = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(sock, FD_SETSIZE, O_NONBLOCK);

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET, "157.148.69.74", &addr.sin_addr.s_addr);
  int state = connect(sock, (const sockaddr*)&addr, sizeof(addr));
  if (!state) {
  //   HPS_LOG_DEBUG(g_logger) << "true == connect(sock, (const sockaddr*)&addr, sizeof(addr)):" << state;
  // } else if (errno == EINPROGRESS) {
  //   HPS_LOG_DEBUG(g_logger) << "add event errno = " << errno << " " << strerror(errno);

    hps_sf::hps_IOManager::GetThis() -> addEvent(sock, hps_sf::hps_IOManager::READ, [](){
      HPS_LOG_INFO(g_logger) << "read callback";
    });

    hps_sf::hps_IOManager::GetThis() -> addEvent(sock, hps_sf::hps_IOManager::WRITE, [](){
      HPS_LOG_INFO(g_logger) << "write callback";
      hps_sf::hps_IOManager::GetThis() -> cancelEvent(sock, hps_sf::hps_IOManager::READ);
      close(sock);
    });
  } else {
      HPS_LOG_DEBUG(g_logger) << "else" << errno << strerror(errno);
  }
  HPS_LOG_INFO(g_logger) << "test_fiber end()";
}

void test1() {
  HPS_LOG_DEBUG(g_logger) << "enter test1()";
  hps_sf::hps_IOManager iom(2, false);
  iom.schedule(&test_fiber);
  HPS_LOG_DEBUG(g_logger) << "test1() end";
}

int main(int argc, char** argv) {
  test1();
  // int p[2];
  // int rt = pipe(p);
  // std::cout << p[0] << " " << p[1];
  // if (rt) std::cout << "false";
  return 0;
}

// 不使用hps_sf::hps_IOManager::GetThis() -> addEvent(...)，而是直接调用hps_IOManager对象的addEvent()进行事件操作的的原因是：