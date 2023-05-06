#include <functional>
#include <string>
#include <iostream>

#include "ChatService.hpp"
#include "ChatServer.hpp"
#include "json.hpp"

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& addr, const std::string name)
    : loop_(loop)
    , server_(loop, addr, name)
{
    server_.setConnectionCallback(bind(&ChatServer::on_connection, this, _1));
    server_.setMessageCallback(bind(&ChatServer::on_message, this, _1, _2, _3));
    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

void ChatServer::on_connection(const TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        ChatService::instance()->client_close_exception(conn);
        conn->shutdown();
    }
}

void ChatServer::on_message(const TcpConnectionPtr& conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    cout<<"exute: "<<buf<<endl;

    json js = json::parse(buf);
    auto msg_handler = ChatService::instance()->get_handler(js["msgid"].get<int>());
    msg_handler(conn, js, time);
}
