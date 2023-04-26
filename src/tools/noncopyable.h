#pragma once

/**
 * @brief noncopyable 被继承后，派生类对象可以进行正常的构造和析构，但派生类对象无法进行拷贝构造和赋值操作
 * 
 */
class noncopyable
{
    public:
        noncopyable(const noncopyable&)=delete;
        void operator=(const noncopyable&)=delete;

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
};