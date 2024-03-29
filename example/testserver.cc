#include <mymuduo/net/TcpServer.h>
#include <mymuduo/tools/Logger.h>
#include <string>
#include <functional>
class EchoServer
{
public:
    EchoServer(EventLoop *loop,
                const InetAddress &addr,
                const std::string &name)
            : server_(loop, addr, name)
            , loop_(loop)
    {
        // 注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
        );
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );

        // 设置合适的线程数量
        server_.setThreadNum(3);
    }
    void start()
    {
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP: %s \n", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN: %s \n", conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn,
                    Buffer *buf,
                    Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        conn->shutdown(); // 关闭写端  EPOLLUP -》 closeCallback_
    }
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer-01"); //Acceptor non_blocking listenfd create bind

    server.start(); // listen loopthread listenfd=>acceptchannel =>mainloop =>
    loop.loop(); // 启动mainLoop的底层Poller

    return 0;
}