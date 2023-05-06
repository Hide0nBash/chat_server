#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <mutex>
#include <mymuduo/TcpConnection.h>
#include "json.hpp"
#include "UserModel.hpp"
#include "OfflineMessageModel.hpp"
#include "FriendModel.hpp"
#include "GroupModel.hpp"
#include "Redis.hpp"

using namespace std;
using namespace placeholders;

using json = nlohmann::json;
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

class ChatService
{
public:     
    static ChatService* instance();

    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void regist(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void one_chat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    bool add_friend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    bool create_group(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void redis_subscribe_message_handler(int channel, string msg);
    bool add_group(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void group_chat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    MsgHandler get_handler(int msgid);

    void client_close_exception(const TcpConnectionPtr &conn);
    void reset();

private:
    ChatService();

    unordered_map<int, MsgHandler> msg_handler_map_;

    unordered_map<int, TcpConnectionPtr> user_connection_map_;
    mutex conn_mutex_;

    Redis redis_;
    
    UserModel user_model_;
    OfflineMessageModel offline_message_model_;
    FriendModel friend_model_;
    GroupModel group_model_;
};

#endif

