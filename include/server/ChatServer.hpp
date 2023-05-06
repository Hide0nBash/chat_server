#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <mymuduo/TcpServer.h>
#include <string>

class ChatServer
{
public:
    ChatServer(EventLoop* loop, const InetAddress& addr, const std::string name);
    void start();
private:
    void on_connection(const TcpConnectionPtr &);
    void on_message(const TcpConnectionPtr &, Buffer *, Timestamp);

    TcpServer server_;
    EventLoop* loop_;
};

#endif