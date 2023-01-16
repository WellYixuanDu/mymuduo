#pragma once

#include <string>

#include "noncopyable.h"
// 定义日志级别 INFO ERROR FATAL DEBUG 
enum LogLevel
{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};


#define LOG_INFO(LogmsgFormat, ...) \
    do \
    {  \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0);

#define LOG_ERROR(LogmsgFormat, ...) \
    do \
    {  \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0);

#define LOG_FATAL(LogmsgFormat, ...) \
    do \
    {  \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0);

#ifdef MUDEBUG
#define LOG_DEBUG(LogmsgFormat, ...) \
    do \
    {  \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while (0);    
#else
    #define LOG_DEBUG(LogmsgFormat, ...)
#endif
// 输出一个日志类，单例模式

class Logger : noncopyable
{
    public:
        // 获取日志唯一的实例对象
        static Logger& instance();
        //设置日志级别
        void setLogLevel(int level);
        // 写日志
        void log(std::string msg); 
    private:
        int logLevel_;
        Logger(){}
};