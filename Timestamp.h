#pragma once
#include <iostream>

#include <string>

// 时间类
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(const int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};