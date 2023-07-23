# hps_sf
[![Security Status](https://www.murphysec.com/platform3/v31/badge/1677911350636732416.svg)](https://www.murphysec.com/console/report/1677911350447988736/1677911350636732416)
## 构建方法
下载到本地之后使用cmake进行外部构建，在项目文件中创建一个名为`build`的目录, 进入`build`中使用cmake进行项目构建。
构建命令
```
mkdir build
cd build 
cmake ..
make
```
## 简介
这是一个高性能服务器的框架.

## 日志系统
按照log4j的模式进行日志系统的设计

类机构：
    hps_Logger (日志器)
        |
        | ---- hps_Formatter (日志格式)
        |
    hps_Appender (日志落地位置)

hps_Logger,对外使用的类，当输入日志级别大于等于hps_Loger的日志级别才能真正的写入。可以有多个不同的hps_Logger, 将框架日志和业务日志进行分离。

hps_Appender，定义日志的输出落地点，实现了hps_StdoutLogAppender（控制台日志）、hps_FileLogAppender(文件日志)。都拥有自己的日志级别和日志格式，主要用于区分日志级别，将error日志，单独输出到一个文件中。

hps_Formatter, 自定义日志的输出格式，通过字符串自定义的日志格式，仿照printf()的输出方法。

## 配置系统

配置系统的原则是：约定优于配置

```c++
template<T, FromStr, Tostr>
class CondifVar;

template<F, T>
LexicalCast;

// 容器偏特化， 目前支持vector
// list，set, map, unordered_set, unordered_map
// map、unordered_set 支持key = std::string
// Config::Lookup(key), key相同
// 类型不同，不会有报错，这个需要处理一下
```

自定义类型实现，需要实现sylar::LexcalCast, 偏特化  
实现手，就可以支持Config解析自定义类型，自定义类型可以和常规SSTL容器一起使用。