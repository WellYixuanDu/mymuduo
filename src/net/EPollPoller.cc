#include "net/EPollPoller.h"
#include "tools/Logger.h"
#include <errno.h>
#include "net/Channel.h"
#include <unistd.h>
#include <strings.h>
// channel 未添加到poller中
const int kNew = -1;
// channel 已添加到poller中
const int kAdded = 1;
// channel 从 poller中删除
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d\n", errno)
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}



Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;

    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop拿到了它的poller给它返回的所有发生事件的channel列表
    }
}

/*
           EventLoop 
    channelList   Poller
                 (Channel Map<fd, channel *>)
*/
void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else //channel 已经在poller中注册过
    {
        int fd =  channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
    
}
// 从poller中删除channel
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    
    channels_.erase(fd);
    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, channel->fd());
    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 更新 channel 通道
void EPollPoller::update(int operaton, Channel *channel)
{
    epoll_event event;
    bzero(&event, sizeof event);
    int fd = channel->fd();
    event.data.fd = fd;
    event.events = channel->events();
    event.data.ptr = channel;
    

    if (::epoll_ctl(epollfd_, operaton, fd, &event) < 0)
    {
        if (operaton == EPOLL_CTL_DEL)
        {
            LOG_ERROR("eopll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("eopll_ctl add | mod error:%d\n", errno);
        }
    }
}