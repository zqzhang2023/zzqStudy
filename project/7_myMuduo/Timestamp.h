#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <iostream>
#include <string>

// 时间类
class Timestamp {
public:
    //构造
    Timestamp(); 
    //带参构造 explicit 是为了防止编译器将microSecondsSinceEpoch优化成int
    explicit Timestamp(int64_t microSecondsSinceEpoch); 
    //返回当前的Timestamp
    static Timestamp now();
    //转化为string输出 //一般是Timestamp::now().toString() 这样用
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};

#endif
