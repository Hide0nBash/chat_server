#include <mymuduo/Logger.h>
#include <vector>
#include <map>
#include <iostream>
#include "ChatService.hpp"
#include "public.hpp"

using namespace std;

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    msg_handler_map_.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    msg_handler_map_.insert({LOGINOUT_MSG, bind(&ChatService::logout, this, _1, _2, _3)});
    msg_handler_map_.insert({REG_MSG, bind(&ChatService::regist, this, _1, _2, _3)});
    msg_handler_map_.insert({ONE_CHAT_MSG, bind(&ChatService::one_chat, this, _1, _2, _3)});
    msg_handler_map_.insert({ADD_FRIEND_MSG, bind(&ChatService::add_friend, this, _1, _2, _3)});
    msg_handler_map_.insert({CREATE_GROUP_MSG, bind(&ChatService::create_group, this, _1, _2, _3)});
    msg_handler_map_.insert({ADD_GROUP_MSG, bind(&ChatService::add_group, this, _1, _2, _3)});
    msg_handler_map_.insert({GROUP_CHAT_MSG, bind(&ChatService::group_chat, this, _1, _2, _3)});

    if(redis_.connect())
    {
        redis_.init_notify_handler(bind(&ChatService::redis_subscribe_message_handler, this, _1, _2));
    }
}

MsgHandler ChatService::get_handler(int msgid)
{
    auto it = msg_handler_map_.find(msgid);
    if(it == msg_handler_map_.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time){
            LOG_ERROR("msgid = %s can not find");
        };
    }
    else
    {
        return msg_handler_map_[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];
    
    User user = user_model_.query(id);

    if(user.get_id() == id && user.get_password() == password)
    {
        if(user.get_state() == "online")
        {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "id is online";
            
            conn->send(response.dump());
        }
        else
        {
            {
                lock_guard<mutex> lock(conn_mutex_);
                user_connection_map_.insert({id, conn});
            }

            redis_.subscribe(id);

            user.set_state("online");
            user_model_.update_state(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.get_id();
            response["name"] = user.get_name();

            vector<string> vec = offline_message_model_.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                offline_message_model_.remove(id);
            }

            vector<User> user_vec = friend_model_.query(id);
            if(!user_vec.empty())
            {
                vector<string> friend_vec;
                for(User &user : user_vec)
                {
                    json js;
                    js["id"] = user.get_id();
                    js["name"] = user.get_name();
                    js["state"] = user.get_state();
                    friend_vec.push_back(js.dump());
                }
                response["friends"] = friend_vec;
            }

            vector<Group> group_vec = group_model_.query_group(id);
            if(!group_vec.empty())
            {
                vector<string> groupV;
                for(Group &group : group_vec)
                {
                    json grpjs;
                    grpjs["id"] = group.get_id();
                    grpjs["groupname"] = group.get_name();
                    grpjs["groupdesc"] = group.get_desc();

                    vector<string> userV;
                    for(GroupUser &user : group.get_users())
                    {
                        json js;
                        js["id"] = user.get_id();
                        js["name"] = user.get_name();
                        js["state"] = user.get_state();
                        js["role"] = user.get_role();
                        userV.push_back(js.dump());
                    }
                    grpjs["users"] = userV;
                    groupV.push_back(grpjs.dump());
                }
                response["groups"] = groupV;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password error";
        conn->send(response.dump());
    }
}

void ChatService::client_close_exception(const TcpConnectionPtr& conn)
{
    User user;
    {
        lock_guard<mutex> lock(conn_mutex_);
        for(auto it = user_connection_map_.begin(); it != user_connection_map_.end(); it++)
        {
            if(it->second == conn)
            {
                user.set_id(it->first);
                user_connection_map_.erase(it);
                break;
            }
        }
    }

    redis_.unsubscribe(user.get_id());

    if(user.get_id() != -1)
    {
        user.set_state("offline");
        user_model_.update_state(user);
    }
}

void ChatService::regist(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.set_name(name);
    user.set_password(password);

    bool state = user_model_.insert(user);
    if (state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.get_id();

        conn->send(response.dump());
    }
    else
    {
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;

        conn->send(response.dump());
    }
}


void ChatService::one_chat(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int receive_id = js["to"].get<int>();
    {
        lock_guard<mutex> lock(conn_mutex_);
        auto it = user_connection_map_.find(receive_id);
        if(it != user_connection_map_.end())
        {
            it->second->send(js.dump());
            return;
        }
    }

    User user = user_model_.query(receive_id);
    if(user.get_state() == "online")
    {
        redis_.publish(receive_id, js.dump());
        return;
    }

    offline_message_model_.insert(receive_id, js.dump());
}

bool ChatService::add_friend(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int friend_id = js["friendid"].get<int>();

    friend_model_.insert(user_id, friend_id);
}


bool ChatService::create_group(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if(group_model_.create_group(group))
    {
        if(group_model_.add_group(user_id, group.get_id(), "creator"))
        {
            return true;
        }
    }

    return false;
}

void ChatService::group_chat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["groupid"].get<int>();

    vector<int> userid_vec = group_model_.query_group_users(user_id, group_id);
    lock_guard<mutex> lock(conn_mutex_);
    for(int id : userid_vec)
    {
        auto it = user_connection_map_.find(id);
        if(it != user_connection_map_.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            User user = user_model_.query(id);
            if(user.get_state() == "online")
            {
                redis_.publish(id, js.dump());
            }
            else
            {
                offline_message_model_.insert(id, js.dump());
            }
        }
    }
}

bool ChatService::add_group(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["groupid"].get<int>();
    if (group_model_.add_group(user_id, group_id, "normal"))
    {
        return true;
    }
    return false;
}


void ChatService::reset()
{
    user_model_.reset_state();
}

void ChatService::logout(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    {
        lock_guard<mutex> lock(mutex);
        auto it = user_connection_map_.find(user_id);
        if(it != user_connection_map_.end())
        {
            user_connection_map_.erase(it);
        }
    }

    redis_.unsubscribe(user_id);
    User user(user_id, "", "", "offline");
    user_model_.update_state(user);
}

void ChatService::redis_subscribe_message_handler(int channel, string message)
{
    lock_guard<mutex> lock(conn_mutex_);
    auto it = user_connection_map_.find(channel);
    if(it != user_connection_map_.end())
    {
        it->second->send(message);
        return;
    }

    offline_message_model_.insert(channel, message);
}
