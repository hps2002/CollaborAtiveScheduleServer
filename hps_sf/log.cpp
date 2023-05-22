#include "log.h"
#include <iostream>
#include <map>
#include <functional>
#include <vector>
#include <tuple>
#include <time.h>
#include <string.h>

namespace hps_sf{

const char* hps_LogLevel::ToString(hps_LogLevel::Level level)
{

    switch(level)
    {
#define XX(name) \
    case hps_LogLevel::Level::name: \
        return #name; \
        break;
    
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

hps_LogEventWarp::hps_LogEventWarp(hps_LogEvent::ptr e):m_event(e)
{

}
hps_LogEventWarp::~hps_LogEventWarp()
{
    m_event -> getLogger() -> log(m_event -> getLevel(), m_event);
}

void hps_LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start (al, fmt);
    format(fmt, al);
    va_end(al);
}

void hps_LogEvent::format(const char* fmt, va_list al)
{   
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1)
    {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

std::stringstream& hps_LogEventWarp::getSS()
{
    return m_event -> getSS();
}

class hps_MessageFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getContent();
    }
}; 

class hps_LevelFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_LevelFormatItem(const std::string str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << hps_LogLevel::ToString(level);
    }
};



class hps_ElapseFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_ElapseFormatItem(const std::string& = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getElapse();
    }
};

class hps_NameFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << logger -> getName();
    }
};

class hps_ThreadIdFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getThreadId();
    }
};

class hps_FiberIdFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getFiberId();
    }
};

class hps_DateTimeFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S"): m_format(format) 
    {
        if (m_format.empty())
            m_format = "%Y-%m-%d %H:%M:%S";
    }
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        struct tm tm;
        time_t time = event -> getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class hps_FilenameFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_FilenameFormatItem(const std::string str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getFile();
    }
};

class hps_LineFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_LineFormatItem(const std::string str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getLine();
    }
};

class hps_NewLineFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_NewLineFormatItem(const std::string str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << std::endl;
    }
};

class hps_StringFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_StringFormatItem(const std::string& str):m_string(str) {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << m_string;
    }
private:
    std::string m_string;
};

class hps_TabFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << "\t";
    }
private:
    std::string m_string;
};

hps_LogEvent::hps_LogEvent(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level
            , const char* file, int32_t line, uint32_t elapse
            , uint32_t threadId, uint32_t fiberId, uint64_t time)
    :m_file(file),
    m_line(line),
    m_elapse(elapse),
    m_threadId(threadId),
    m_fiberId(fiberId),
    m_time(time),
    m_logger(logger),
    m_level(level)
    {

    }



hps_Logger::hps_Logger(const std::string& name): m_name(name), m_level(hps_LogLevel::DEBUG)
{   
    m_formatter.reset(new hps_LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void hps_Logger::addAppender(hps_LogAppender::ptr appender)
{
    //查看appender中是否有Formater,如果没有的话将Logger的formatter给他，保证每个对象都有formatter 
    if (!appender -> getFormatter())
        appender -> setFormatter(m_formatter);
    m_appenders.push_back(appender);
}
void hps_Logger::delAppender(hps_LogAppender::ptr appender)
{
    for (auto it = m_appenders.begin(); it != m_appenders.end(); it++)
        if (*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
}

void hps_Logger::log(hps_LogLevel::Level level, hps_LogEvent::ptr event)
{
    if (level >= m_level)
    {
        auto self = shared_from_this();
        for (auto& i : m_appenders)
            i -> log(self, level, event);
    }
}

void hps_Logger::debug(hps_LogEvent::ptr event)
{
    log(hps_LogLevel::DEBUG, event);
}
void hps_Logger::info(hps_LogEvent::ptr event)
{
    log(hps_LogLevel::INFO, event);
}
void hps_Logger::warn(hps_LogEvent::ptr event)
{
    log(hps_LogLevel::WARN, event);
}
void hps_Logger::error(hps_LogEvent::ptr event)
{
    log(hps_LogLevel::ERROR, event);
}
void hps_Logger::fatal(hps_LogEvent::ptr event)
{
    log(hps_LogLevel::FATAL, event);
}

hps_FileLogAppender::hps_FileLogAppender(const std::string & filename):m_filename(filename)
{
    reopen();
}

hps_FileLogAppender::~hps_FileLogAppender()
{
    m_filestream.close();
}

void hps_FileLogAppender::log(std::shared_ptr<hps_Logger> logger, hps_LogLevel::Level level, hps_LogEvent::ptr event)
{
    if (level >= m_level)
    {
        m_filestream << m_formatter -> format(logger, level, event);
    }
}

bool hps_FileLogAppender::reopen()
{
    if (m_filestream)
    {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}   

void hps_StdoutLogAppender::log(std::shared_ptr<hps_Logger> logger,  hps_LogLevel::Level level, hps_LogEvent::ptr event)
{
    if (level >= m_level)
    {
        std::cout << m_formatter -> format(logger, level, event);
    }
}
hps_LogFormatter::hps_LogFormatter(const std::string& pattern): m_pattern(pattern)
{
    init();
}
std::string hps_LogFormatter::format(std::shared_ptr<hps_Logger> logger,  hps_LogLevel::Level level, hps_LogEvent::ptr event)
{
    std::stringstream ss;
    for (auto& i : m_items)
    {
        i -> format(ss, logger, level, event);
    }
    return ss.str();
}

//%xxx %xxx{xxx} %%       只有这三种格式
void hps_LogFormatter::init()
{
    //str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr = "";
    for (size_t i = 0; i < m_pattern.size(); i ++)
    {
        if (m_pattern[i] != '%')
        {
            nstr += m_pattern[i];
            continue;
        }
        if (i + 1 < m_pattern.size() && m_pattern[i + 1] == '%')
        {
            nstr += m_pattern[i + 1];
            continue;
        }

        //当m_partten[i] == '%'
        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str = "";
        std::string fmt = "";
        while (n < m_pattern.size())
        {
            if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' 
                            && m_pattern[n] != '}'))
            {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }

            if (fmt_status == 0 && m_pattern[n] == '{')
            {
                str = m_pattern.substr(i + 1, n - i - 1);
                // std::cout << "*" << str << std::endl;
                fmt_status = 1; // 解析格式
                fmt_begin = n;
                n ++;
                continue;
            }
            else if (fmt_status == 1 && m_pattern[n] == '}')
            {
                fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                // std::cout << "#" << fmt << std::endl;
                fmt_status = 0;
                n ++;
                break;
            }
            n ++;
            if (n == m_pattern.size() && str.empty())
                str = m_pattern.substr(i + 1);
        }

        if (fmt_status == 0)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            // str = m_pattern.substr(i + 1, n - i - 1);

            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
        else if (fmt_status == 1)
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
        // else if (fmt_status == 2)
        // {
        //     if (!nstr.empty())
        //     {
        //         vec.push_back(std::make_tuple(nstr, "", 0));
        //         nstr.clear();
        //     }
        //     vec.push_back(std::make_tuple(str, fmt, 1));
        //     i = n - 1;
        // }
    }
    if (!nstr.empty())
        vec.push_back(std::make_tuple(nstr, "", 0));
    

    static std::map<std::string, std::function<hps_FormatItem::ptr(const std::string& str)> > s_format_items = 
    {

    #define XX(str, C) \
            {#str, [](const std::string& fmt) {return hps_FormatItem::ptr(new C(fmt));}}

            XX(m, hps_MessageFormatItem),
            XX(p, hps_LevelFormatItem),
            XX(r, hps_ElapseFormatItem),
            XX(c, hps_NameFormatItem),
            XX(t, hps_ThreadIdFormatItem),  
            XX(n, hps_NewLineFormatItem), 
            XX(d, hps_DateTimeFormatItem),
            XX(f, hps_FilenameFormatItem), 
            XX(l, hps_LineFormatItem),
            XX(T, hps_TabFormatItem),
            XX(F, hps_FiberIdFormatItem), 
    #undef XX
    };

    for (auto& i : vec)
    {
        if (std::get<2>(i) == 0)
        {
            m_items.push_back(hps_FormatItem::ptr(new hps_StringFormatItem(std::get<0>(i))));
        }
        else 
        {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end())
            {
                m_items.push_back(hps_FormatItem::ptr(new hps_StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            }
            else 
            {
                m_items.push_back(it -> second(std::get<1>(i)));
            }
        }
        // std::cout<< "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
}

hps_LoggerManager::hps_LoggerManager()
{
    m_root.reset(new hps_Logger);
    m_root -> addAppender(hps_LogAppender::ptr(new hps_StdoutLogAppender));
}

hps_Logger::ptr hps_LoggerManager::getLogger(const std::string& name)
{
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it -> second;
}
}
