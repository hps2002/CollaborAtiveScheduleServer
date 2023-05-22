#include <iostream>
#include <thread>
#include "../hps_sf/log.h"
#include "../hps_sf/util.h"

int main()
{
    hps_sf::hps_Logger::ptr logger(new hps_sf::hps_Logger);
    logger -> addAppender(hps_sf::hps_LogAppender::ptr(new hps_sf::hps_StdoutLogAppender));
    
    hps_sf::hps_FileLogAppender::ptr file_appdener(new hps_sf::hps_FileLogAppender("./log.txt"));

    hps_sf::hps_LogFormatter::ptr fmt(new hps_sf::hps_LogFormatter("%d%T%p%T%m%n"));
    file_appdener -> setFormatter(fmt);
    file_appdener -> setLevel(hps_sf::hps_LogLevel::ERROR);

    logger -> addAppender(file_appdener);

    HPS_LOG_FMT_ERROR(logger, "test win11 fmt error %d", 20062001);
    HPS_LOG_INFO(logger) << "test win11 info";
    HPS_LOG_ERROR(logger) << "test win11 error";  

    auto l = hps_sf::hps_LoggerMgr::GetInstance() -> getLogger("xx");
    HPS_LOG_INFO(l) << "xxx";
    
    //插入一个logger
    return 0;
}