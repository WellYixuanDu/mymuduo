#pragma once

#include "tools/noncopyable.h"
#include <functional>
#include "tools/Timestamp.h"
#include <memory>
// 头文件中没有使用到具体的方法，只是声明，因此前置声明一下即可，不需包含头文件
class EventLoop;

/*
    理清楚 EventLoop、Channel、Poller之间的关系，Reactor模型上对应 Demultiplex
    channel 理解为通道，封装了sockfd和感兴趣的Event，如EPOLLIN、EPOLLOUT事件，还绑定了poller返回的具体事件
    打包fd为channel，下发给poller
    一共有两种channel，分别打包listenfd（acceptorChannel）与connfd（connectionChannel）
    channel和poller间不能互相访问，是通过EventLoop进行的
*/
class Channel : noncopyable
{
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();
    // fd得到poller通知以后，调用相应回调处理事件
    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }
    // 防止channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    int set_revents(int revt) { revents_ = revt; }
    

    // 设置fd相应的事件状态
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }
    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;  // 事件循环
    const int fd_;  //fd，poller监听的对象

    int events_; // 注册fd感兴趣的事件
    int revents_; // poller返回具体发生的事件

    int index_; 

    std::weak_ptr<void> tie_;
    bool tied_;

    // channel 通道里面能够获知fd最终发生的具体的事件revents，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};