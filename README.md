# hps_sf

## 简介
这是一个高性能服务器的框架

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