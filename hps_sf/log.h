#ifndef __HPS_LOG_H__
#define __HPS_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdarg>
#include <map>
#include "util.h"
#include "singleton.h"

#define HPS_LOG_LEVEL(logger, level) \
    if (logger -> getLevel() <= level) \
        hps_sf::hps_LogEventWarp(hps_sf::hps_LogEvent::ptr(new hps_sf::hps_LogEvent(logger, level, __FILE__, __LINE__, 0, hps_sf::GetThreadId(), hps_sf::GetFiberId(), time(0)))).getSS()

#define HPS_LOG_DEBUG(logger)  HPS_LOG_LEVEL(logger, hps_sf::hps_LogLevel::DEBUG)
#define HPS_LOG_INFO(logger)  HPS_LOG_LEVEL(logger, hps_sf::hps_LogLevel::INFO)
#define HPS_LOG_WARN(logger)  HPS_LOG_LEVEL(logger, hps_sf::hps_LogLevel::WARN)
#define HPS_LOG_ERROR(logger)  HPS_LOG_LEVEL(logger, hps_sf::hps_LogLevel::ERROR)
#define HPS_LOG_FATAL(logger)  HPS_LOG_LEVEL(logger, hps_sf::hps_LogLevel::FATAL)

#define HPS_LOG_FMT_LEVEL(looger, level, fmt, ...) \
    if (looger -> getLevel() <= level) \
        hps_sf::hps_LogEventWarp(hps_sf::hps_LogEvent::ptr(new hps_sf::hps_LogEvent(logger, level, __FILE__, \
                                    __LINE__, 0, hps_sf::GetThreadId(), hps_sf::GetFiberId(), time(0)))).getEvent() -> format(fmt, __VA_ARGS__)

#define HPS_LOG_FMT_DEBUG(logger, fmt, ...) HPS_LOG_FMT_LEVEL(logger, hps_sf::hps_LogLevel::DEBUG, fmt, __VA_ARGS__)
#define HPS_LOG_FMT_INFO(logger, fmt, ...)  HPS_LOG_FMT_LEVEL(logger, hps_sf::hps_LogLevel::INFO, fmt, __VA_ARGS__)
#define HPS_LOG_FMT_WARN(logger, fmt, ...)  HPS_LOG_FMT_LEVEL(logger, hps_sf::hps_LogLevel::WARN, fmt, __VA_ARGS__)
#define HPS_LOG_FMT_ERROR(logger, fmt, ...) HPS_LOG_FMT_LEVEL(logger, hps_sf::hps_LogLevel::ERROR, fmt, __VA_ARGS__)
#define HPS_LOG_FMT_FATAL(logger, fmt, ...) HPS_LOG_FMT_LEVEL(logger, hps_sf::hps_LogLevel::FATAL, fmt, __VA_ARGS__)

#define HPS_LOG_ROOT() hps_sf::hps_LoggerMgr::GetInstance() -> getRoot()

namespace hps_sf
{
class hps_Logger;

// 日志级别
class hps_LogLevel {
public:

    enum Level 
    {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char* ToString(hps_LogLevel::Level level);
};

// 日志事件
class hps_LogEvent{
public:
    typedef std::shared_ptr<hps_LogEvent> ptr;
    hps_LogEvent(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, const char* file, int32_t line, uint32_t elapse
                , uint32_t threadId, uint32_t fiberId, uint64_t time);
    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const {return m_elapse; }
    uint32_t getThreadId() const {return m_threadId; }
    uint32_t getFiberId() const {return m_fiberId; }
    uint64_t getTime() const {return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<hps_Logger> getLogger() const { return m_logger; }
    hps_LogLevel::Level getLevel() const {return m_level;}

    std::stringstream& getSS(){ return m_ss; }
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);
private:
    const char* m_file = nullptr; // 文件名
    int32_t m_line = 0; //行号
    uint32_t m_elapse = 0;  //程序启动开始到现在毫秒数
    uint32_t m_threadId = 0; // 线程号
    uint32_t m_fiberId = 0; //  协程号
    uint64_t m_time;    //时间
    std::stringstream m_ss;  //内容

    std::shared_ptr<hps_Logger> m_logger;  
    hps_LogLevel::Level m_level;
};

class hps_LogEventWarp{
public:
    hps_LogEventWarp(hps_LogEvent::ptr e);
    ~hps_LogEventWarp();
    std::stringstream& getSS();

    hps_LogEvent::ptr getEvent() const {return m_event;}
private:
    hps_LogEvent::ptr m_event;
};

// 日志格式器
class hps_LogFormatter{
public:
    typedef std::shared_ptr<hps_LogFormatter> ptr;
    hps_LogFormatter(const std::string& pattern);//根据partern里面的格式进行输出
    std::string format(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, hps_LogEvent::ptr event);
    
public:
    class hps_FormatItem
    {
    public:
        typedef std::shared_ptr<hps_FormatItem> ptr;
        virtual ~hps_FormatItem(){}
        virtual void format(std::ostream& os, std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) = 0;
    };
    void init();    //使用log4j的格式进行日志定义
private:
    std::string m_pattern;
    std::vector<hps_FormatItem::ptr> m_items;
};

//日志输出地
class hps_LogAppender{
public:
    typedef std::shared_ptr<hps_LogAppender> ptr;
    virtual ~hps_LogAppender() {}
    virtual void log(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) = 0;
    void setFormatter(hps_LogFormatter::ptr val) { m_formatter = val; }
    hps_LogFormatter::ptr getFormatter() const { return m_formatter; }

    hps_LogLevel::Level getLevel() const {return m_level;}
    void setLevel(hps_LogLevel::Level level) {m_level = level;}
protected:
    hps_LogLevel::Level m_level = hps_LogLevel::DEBUG;
    hps_LogFormatter::ptr m_formatter;
};

//日志器
class hps_Logger: public std::enable_shared_from_this<hps_Logger>
{
public:
    typedef std::shared_ptr<hps_Logger> ptr;
    hps_Logger(const std::string& name = "root");
    void log(hps_LogLevel::Level level, hps_LogEvent::ptr event);
    
    void debug(hps_LogEvent::ptr event);
    void info(hps_LogEvent::ptr event);
    void warn(hps_LogEvent::ptr event);
    void error(hps_LogEvent::ptr event);
    void fatal(hps_LogEvent::ptr event);

    void addAppender(hps_LogAppender::ptr appender);
    void delAppender(hps_LogAppender::ptr addender);
    hps_LogLevel::Level getLevel() const {return m_level;}
    void setLevel(hps_LogLevel::Level val) { m_level = val; }

    const std::string getName() const { return m_name; }
private:
    std::string m_name;             //日志名称
    hps_LogLevel::Level m_level;      //日志级别
    std::list<hps_LogAppender::ptr> m_appenders;           //Appender集合
    hps_LogFormatter::ptr m_formatter;
};

//输出到控制台的Appender
class hps_StdoutLogAppender : public hps_LogAppender
{
public:
    typedef std::shared_ptr<hps_StdoutLogAppender> ptr;
    void log(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override;
private:
};

//输出到文件的Appender
class hps_FileLogAppender : public hps_LogAppender
{
public:
    typedef std::shared_ptr<hps_FileLogAppender> ptr;
    
    hps_FileLogAppender(const std::string & filename);
    ~hps_FileLogAppender();
    void log(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override; 
    
    //重新打开文件，文件打开成功，返回true；
    bool reopen(); 
private:
    std::string m_filename;
    std::ofstream m_filestream;
};

class hps_LoggerManager 
{
public:
    hps_LoggerManager();
    hps_Logger::ptr getLogger(const std::string& name);

    void init();
    hps_Logger::ptr getRoot() const {return m_root;};
private:
    std::map<std::string, hps_Logger::ptr> m_loggers;
    hps_Logger::ptr m_root;
};

typedef hps_sf::hps_Singleton<hps_LoggerManager> hps_LoggerMgr;


}

#endif