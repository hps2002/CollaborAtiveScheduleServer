#include "log.h"
#include <iostream>
#include <map>
#include <functional>
#include <vector>
#include <tuple>
#include <time.h>
#include <string.h>
#include "config.h"

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

hps_LogLevel::Level hps_LogLevel::FromString(const std::string& str)
{
  #define XX(level, v) \
    if (str == #v) \
      return hps_LogLevel::level; \

    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return hps_LogLevel::UNKNOW;
  #undef XX
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

void hps_LogAppender::setFormatter(hps_LogFormatter::ptr val) {
  MutexType::Lock lock(m_mutex);
  m_formatter = val;
  if (m_formatter) {
    m_hasFormatter = true;
  }
  else {
    m_hasFormatter = false;
  }
}

hps_LogFormatter::ptr hps_LogAppender::getFormatter() {
  MutexType::Lock lock(m_mutex);
  return m_formatter;
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
        os << event -> getLogger() -> getName();
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

class hps_ThreadNameFormatItem: public hps_LogFormatter::hps_FormatItem
{
public:
    hps_ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, hps_Logger::ptr logger, hps_LogLevel::Level level, hps_LogEvent::ptr event) override
    {
        os << event -> getThreadName();
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
            , uint32_t threadId, uint32_t fiberId, uint64_t time, const std::string& thread_name)
    :m_file(file),
    m_line(line),
    m_elapse(elapse),
    m_threadId(threadId),
    m_fiberId(fiberId),
    m_time(time),
    m_threadName(thread_name),
    m_logger(logger),
    m_level(level)
    {

    }



hps_Logger::hps_Logger(const std::string& name): m_name(name), m_level(hps_LogLevel::DEBUG)
{   
    m_formatter.reset(new hps_LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void hps_Logger::setFormatter (hps_LogFormatter::ptr val)
{
    MutexType::Lock lock(m_mutex);
    m_formatter = val;

    for (auto&i : m_appenders) {
      MutexType::Lock ll(i -> m_mutex);
      if (!i -> m_hasFormatter)
        i -> m_formatter = m_formatter;
    }
}

void hps_Logger::setFormatter (const std::string& val)
{
    hps_sf::hps_LogFormatter::ptr new_val(new hps_sf::hps_LogFormatter(val));
    if (new_val -> isError())
    {
      std::cout << "Logger setFormatter name = " << m_name << "value = " << val << " incalid formatter" << std::endl;
      return ;
    }
    // m_formatter.reset(new hps_sf::hps_LogFormatter(val));
    setFormatter(new_val);
}

hps_LogFormatter::ptr hps_Logger::getFormatter()
{
  MutexType::Lock lock(m_mutex);
  return m_formatter;
}

void hps_Logger::addAppender(hps_LogAppender::ptr appender)
{
    MutexType::Lock lock(m_mutex);
    //查看appender中是否有Formater,如果没有的话将Logger的formatter给他，保证每个对象都有formatter 
    if (!appender -> getFormatter())
    {
      MutexType::Lock ll(appender -> m_mutex);
      appender -> m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}
void hps_Logger::delAppender(hps_LogAppender::ptr appender)
{
    MutexType::Lock lock(m_mutex);
    for (auto it = m_appenders.begin(); it != m_appenders.end(); it++)
        if (*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
}

void hps_Logger::clearAppenders()
{
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void hps_Logger::log(hps_LogLevel::Level level, hps_LogEvent::ptr event)
{
    if (level >= m_level)
    {
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if (!m_appenders.empty())
        {
          for (auto& i : m_appenders)
              i -> log(self, level, event);
        }
        else if (m_root)
        {
            m_root -> log(level, event);
        }
    }
}

std::string hps_Logger::toYAMLString()  {
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["name"] = m_name;
  if (m_level != hps_LogLevel::UNKNOW)
    node["level"] = hps_LogLevel::ToString(m_level);
  node["type"] = "hps_StdoutLogAppender";
  if (m_formatter) {
    node["formatter"] = m_formatter -> getPattern();;
  }
  for (auto& i : m_appenders) {
    node["appenders"].push_back(YAML::Load(i -> toYAMLString()));
  }

  std::stringstream ss;
  ss << node;
  return ss.str(); 
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
        uint64_t now = time(0);
        if (now != m_latsTime) {
          reopen();
          m_latsTime = now;
        }
        MutexType::Lock lock(m_mutex);
        if (m_filestream << m_formatter -> format(logger, level, event)) {
          std::cout << "LogFile error" << std::endl;
        }
    }
}

std::string hps_FileLogAppender::toYAMLString() {
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  if (m_level != hps_LogLevel::UNKNOW)
    node["level"] = hps_LogLevel::ToString(m_level); 
  node["file"] = m_filename;
  node["type"] = "hps_FileAppender";
  if (m_hasFormatter && m_formatter) {
    node["formatter"] = m_formatter -> getPattern();
  }
  std::stringstream ss;
  ss << node;
  // std::cout << ss.str() << std::endl;
  return ss.str(); 
}

bool hps_FileLogAppender::reopen()
{
    MutexType::Lock lock(m_mutex);
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
        MutexType::Lock lock(m_mutex);
        std::cout << m_formatter -> format(logger, level, event);
    }
}

std::string hps_StdoutLogAppender::toYAMLString() {
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["type"] = "hps_StdoutLogAppender";
  if (m_level != hps_LogLevel::UNKNOW)
    node["level"] = hps_LogLevel::ToString(m_level);
  if (m_hasFormatter && m_formatter) {
    node["formatter"] = m_formatter -> getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
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
            m_error = true;
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

            XX(m, hps_MessageFormatItem),           //消息
            XX(p, hps_LevelFormatItem),             //日志级别
            XX(r, hps_ElapseFormatItem),            //累积毫秒
            XX(c, hps_NameFormatItem),              //日志名称
            XX(t, hps_ThreadIdFormatItem),          //线程id
            XX(n, hps_NewLineFormatItem),           //换行
            XX(d, hps_DateTimeFormatItem),          //时间
            XX(f, hps_FilenameFormatItem),          //文件名
            XX(l, hps_LineFormatItem),              //行号
            XX(T, hps_TabFormatItem),               //缩进
            XX(F, hps_FiberIdFormatItem),           //协程号
            XX(N, hps_ThreadNameFormatItem),        //线程名
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
                m_error = true;
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

    m_loggers[m_root -> m_name] = m_root;
    init();
}

hps_Logger::ptr hps_LoggerManager::getLogger(const std::string& name)
{
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) 
    {
        return it -> second;
    }
    
    hps_Logger::ptr logger(new hps_Logger(name));
    logger -> m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

struct hps_LogAppenderDefine {
    int type = 0; // 1 - file; 2 - stdout
    hps_LogLevel::Level level = hps_LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const hps_LogAppenderDefine& oth) const
    {
      return type == oth.type && 
             level == oth.level && 
             formatter == oth.formatter &&
             file == oth.file;
    }
};

struct hps_LogDefine {
    std::string name;
    hps_LogLevel::Level level = hps_LogLevel::UNKNOW;
    std::string formatter;
    std::vector<hps_LogAppenderDefine> appenders;
    
    bool operator==(const hps_LogDefine& oth) const
    {
        return name == oth.name &&
               level == oth.level && 
               formatter == oth.formatter &&
               appenders == oth.appenders;
    }

    bool operator<(const hps_LogDefine& oth) const{
      return name < oth.name;
    }
};

template<>
class hps_LexicalCast<std::string, std::set<hps_LogDefine> >
{
public:
    std::set<hps_LogDefine> operator() (const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        std::set<hps_LogDefine> vec;
        for (size_t i = 0; i < node.size(); i ++)
        {
            auto n = node[i];
            if (!n["name"].IsDefined())
            {
                std::cout << "log config error: name is null" << n << std::endl;
                continue;
            } 
            
            hps_LogDefine ld;
            ld.name = n["name"].as<std::string>();
            ld.level = hps_LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if (n["formatter"].IsDefined())
            {
              ld.formatter = n["formatter"].as<std::string>();
            }

            if (n["appenders"].IsDefined())
            {
              for (size_t x = 0; x < n["appenders"].size(); x ++)
              {
                auto a = n["appenders"][x];
                if (!a["type"].IsDefined()) 
                {
                  std::cout << "log config error: appender type is null" << n << std::endl;
                  continue;
                }
                std::string type = a["type"].as<std::string>();
                hps_LogAppenderDefine lad;
                if (type == "hps_FileLogAppender")
                {
                  lad.type = 1;
                  if (!a["file"].IsDefined())
                  {
                    std::cout << "log config error: fileappender file is null, " << a << std::endl;
                    continue;
                  }
                  lad.file = a["file"].as<std::string>();
                  if (a["formatter"].IsDefined())
                  {
                    lad.formatter = a["formatter"].as<std::string>();
                  }
                }
                else if (type == "hps_StdoutLogAppender")
                  lad.type = 2;
                else
                {
                  std::cout << "log config error: appender type is invalid, " << a << std::endl;
                  continue;
                }
                ld.appenders.push_back(lad);
              }
            }
            vec.insert(ld);
        }
        return vec;
    }
};

template<>
class hps_LexicalCast<std::set<hps_LogDefine>, std::string>
{
public:
    std::string operator() (const std::set<hps_LogDefine> v)
    {
        YAML::Node node;
        for (auto& i : v)
        {
          YAML::Node n;
          n["name"] = i.name;
          if (i.level != hps_LogLevel::UNKNOW) {
            n["level"] = hps_LogLevel::ToString(i.level);
          }
          if (!i.formatter.empty()) {
            n["formatter"] = i.formatter;
          }
          for (auto& a : i.appenders) {
            YAML::Node na;
            if (a.type == 1) {
              na["type"] = "hps_FileLogAppender";
              na["file"] = a.file;
            } else {
              na["type"] = "hps_StdoutLogAppender";
            }
            if (a.level != hps_LogLevel::UNKNOW) {
                na["level"] = hps_LogLevel::ToString(a.level);
            }
            if (!a.formatter.empty()) {
              na["formatter"] = a.formatter;
            }

            n["appenders"].push_back(na);
          }
          node.push_back(n);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

hps_sf::hps_ConfigVar<std::set<hps_LogDefine> >::ptr g_log_defines = hps_sf::hps_Config::Lookup("logs", std::set<hps_LogDefine>(), "logs config");

struct hps_LogIniter {
    hps_LogIniter()
    {
        g_log_defines -> addListener([](const std::set<hps_LogDefine>& old_value, const std::set<hps_LogDefine>& new_value)
        {
            HPS_LOG_INFO(HPS_LOG_ROOT()) << "on_logger_change_conf_changed";
            for (auto& i : new_value)
            {
                auto it = old_value.find(i);
                hps_sf::hps_Logger::ptr logger;
                if (it == old_value.end())
                {
                    //新增logger
                    logger = HPS_LOG_NAME(i.name);
                }
                else 
                {
                  if (!(i == *it))
                  {
                    // 修改
                    logger = HPS_LOG_NAME(i.name);
                  }
                }
                logger -> setLevel(i.level);
                if (!i.formatter.empty())
                {
                    logger -> setFormatter(i.formatter);
                }

                logger -> clearAppenders();
                for (auto& a : i.appenders)
                {
                    hps_sf::hps_LogAppender::ptr ap;
                    if (a.type == 1)
                    {
                      ap.reset(new hps_FileLogAppender(a.file));
                    }
                    else if (a.type == 2)
                    {
                      ap.reset(new hps_StdoutLogAppender);
                    }
                    ap -> setLevel(a.level);
                    if (!a.formatter.empty()) {
                      hps_LogFormatter::ptr fmt(new hps_LogFormatter(a.formatter));
                      if (!fmt -> isError())
                        ap -> setFormatter(fmt);
                      else
                        std::cout << "log.name=" << i.name << "appender type=" << a.type << "  formatter=" << a.formatter << " is vaild"  << std::endl; 
                    }
                    logger -> addAppender(ap);
                }
            }

            for (auto& i : old_value) 
            {
                auto it = new_value.find(i);
                if (it == new_value.end())
                {
                    // 删除logger
                    auto logger = HPS_LOG_NAME(i.name);
                    logger -> setLevel((hps_LogLevel::Level)100);
                    logger -> clearAppenders();
                }
            }
        });
    }
};

static hps_LogIniter __log_init;

std::string hps_LoggerManager::toYAMLString() {
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  for (auto& i : m_loggers) {
    node.push_back(YAML::Load(i.second -> toYAMLString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}


void hps_LoggerManager::init () 
{

}
}
