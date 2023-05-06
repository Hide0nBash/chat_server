#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "json.hpp"

#include "Group.hpp"
#include "User.hpp"
#include "public.hpp"

using namespace std;
using json = nlohmann::json;

#define PASSWOED_LENGTH 50
#define NAME_LENGTH 50
#define BUFFER_SIZE 1024

User g_current_user;
vector<User> g_current_friends_list;
vector<Group> g_current_group_list;

void ShowCurrentUserData();
void ReadTaskHandler(int client_fd);
string GetCurrentTime();

void MainMenu(int client_fd);
void Help(int fd = 0, string str = "");
void Chat(int, string);
void AddFriend(int ,string);
void CreateGroup(int ,string);
void AddGroup(int, string);
void GroupChat(int, string);
void LogOut(int, string);

bool g_is_menu_running = false;
unordered_map<string, string> command_map = {
    {"help", "显示所有支持的命令"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}
};

unordered_map<string, function<void(int, string)>> command_handler_map = {
    {"help", Help},
    {"chat", Chat},
    {"addfriend", AddFriend},
    {"creategroup", CreateGroup},
    {"addgroup", AddGroup},
    {"groupchat", GroupChat},
    {"loginout", LogOut}};

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        cerr << "command invalid example" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1)
    {
        cerr << "create socket error" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server ,0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if(-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cout << "connect error" << endl;
        close(clientfd);
        exit(-1);
    }

    for(;;)
    {
        cout << "**********welcome**********" << endl;
        cout << "           1.  login" << endl;
        cout << "           2.  register" << endl;
        cout << "           3.  quit " << endl;
        cout << "please input your choice:";

        int choice = 0;
        cin >> choice;
        cin.get();

        switch(choice)
        {
            case 1:
            {
                int id = 0;
                char pwd[PASSWOED_LENGTH] = {0};
                cout<< "give me id : ";
                cin >> id;
                cin.get();

                cout<<"give me pwd : ";
                cin.getline(pwd, PASSWOED_LENGTH);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
                if(len == -1)
                {
                    cerr << "send msg error" << endl;
                }
                else
                {
                    char buffer[BUFFER_SIZE] = {0};
                    len = recv(clientfd, buffer, BUFFER_SIZE, 0);
                    if(len == -1)
                    {
                        cerr << "recv login error ?" << endl;
                    }
                    else
                    {
                        json response_js = json::parse(buffer);
                        if(response_js["errno"].get<int>() != 0)
                        {
                            cerr << response_js["errmsg"] << endl;
                        }
                        else
                        {
                            g_current_user.set_id(response_js["id"].get<int>());
                            g_current_user.set_name(response_js["name"]);

                            if(response_js.contains("friends"))
                            {
                                vector<string> vec = response_js["friends"];
                                for(string &str : vec)
                                {
                                    json friend_js = json::parse(str);
                                    User user;
                                    user.set_id(friend_js["id"].get<int>());
                                    user.set_name(friend_js["name"]);
                                    user.set_state(friend_js["state"]);
                                    g_current_friends_list.push_back(user);
                                }
                            }

                            if(response_js.contains("groups"))
                            {
                                vector<string> vec = response_js["groups"];
                                for(string &str : vec)
                                {
                                    json group_js = json::parse(str);
                                    Group group;
                                    group.set_id(group_js["id"].get<int>());
                                    group.set_desc(group_js["desc"]);
                                    group.set_name(group_js["name"]);
                                    
                                    vector<string> vec2 = group_js["users"];
                                    for(string &str2 : vec2)
                                    {
                                        json user_js = json::parse(str2);
                                        GroupUser user;
                                        user.set_id(user_js["id"].get<int>());
                                        user.set_name(user_js["name"]);
                                        user.set_state(user_js["state"]);
                                        user.set_role(user_js["role"]);

                                        group.get_users().push_back(user);
                                    }
                                    g_current_group_list.push_back(group);
                                }
                            }

                            ShowCurrentUserData();

                            if(response_js.contains("offlinemsg"))
                            {
                                vector<string> vec = response_js["offlinemsg"];
                                for(string &str : vec)
                                {
                                    json js = json::parse(str);
                                    cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << "said: " << js["msg"].get<string>() << endl;
                                }
                            }

                            thread read_task(ReadTaskHandler, clientfd);
                            read_task.detach();

                            g_is_menu_running = true;
                            MainMenu(clientfd);
                        }
                    }
                }
            }
            break;
            case 2:
            {
                char name[NAME_LENGTH] = {0};
                char pwd[PASSWOED_LENGTH] = {0};
                cout << "user name:" << endl;
                cin.getline(name, NAME_LENGTH);
                cout<< "password:" << endl;
                cin.getline(pwd, PASSWOED_LENGTH);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
                if(len == -1)
                {
                    cerr << "send msg error" << endl;
                }
                else
                {
                    char buffer[BUFFER_SIZE] = {0};
                    len = recv(clientfd, buffer, BUFFER_SIZE, 0);
                    if(len == -1)
                    {
                        cerr << "recv register response error" << endl;
                    }
                    else
                    {
                        cout << "len: " << len << " buffer:" << buffer << endl;
                        json response_js = json::parse(buffer);
                        if(response_js["errno"].get<int>() != 0)
                        {
                            cerr << name << " is already exist, failed" << endl;
                        }
                        else
                        {
                            cout << name << " regist success, user id is " << response_js["id"] << endl;
                        }
                    }
                }
            }
            break;
            case 3:
            {
                close(clientfd);
                exit(0);
            }
            default:
            {
                cerr << "invalid input" << endl;
            }
            break;
        }
    }
}

void ShowCurrentUserData()
{
    cout << "--------------------login user--------------------" << endl;
    cout << "current login uer => id: " << g_current_user.get_id() << " name: " << g_current_user.get_name() << endl;
    cout << "-------------------friend  list-------------------" << endl;
    if(!g_current_friends_list.empty())
    {
        for(User &user : g_current_friends_list)
        {
            cout << user.get_id() << " " << user.get_name() << " " << user.get_state() << endl;
        }
    }
    cout << "--------------------group list--------------------" << endl;
    if(!g_current_group_list.empty())
    {
        for(Group &group : g_current_group_list)
        {
            cout << group.get_id() << " " << group.get_name() << " " << group.get_desc() << endl;

            cout << "========group user========" << endl;
            for(GroupUser &group_user : group.get_users())
            {
                cout << group_user.get_id() << " " << group_user.get_name() << " " << group_user.get_state() << " " << group_user.get_role() << endl;
            }
        }
    }
    cout << "--------------------------------------------------" << endl;
}

void ReadTaskHandler(int client_fd)
{
    for(;;)
    {
        char buffer[BUFFER_SIZE] = {0};
        int len = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if(len == -1 || len == 0)
        {
            close(client_fd);
            exit(-1);
        }

        json js = json::parse(buffer);
        if(js["msgid"].get<int>() == ONE_CHAT_MSG)
        {
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"] << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        else if(js["msgid"].get<int>() == GROUP_CHAT_MSG)
        {
            cout << "group msg: [" << js["groupid"] << "]";
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

string GetCurrentTime()
{
    auto tt = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return string(date);
}

void MainMenu(int clientfd)
{
    Help();
    char buffer[BUFFER_SIZE] = {0};
    while(g_is_menu_running)
    {
        cin.getline(buffer, BUFFER_SIZE);
        string command_buf(buffer);
        string command;
        int index = command_buf.find(":");
        if(index == -1)
        {
            command = command_buf;
        }
        else
        {
            command = command_buf.substr(0, index);
        }

        auto it = command_handler_map.find(command);
        if(it == command_handler_map.end())
        {
            cerr << "invaild input command" << endl;
            continue; 
        }

        it->second(clientfd, command_buf.substr(index+1, command_buf.size() - index));
    }
}

void Help(int, string)
{
    cout << "--------command list--------" << endl;
    for (auto &it : command_map)
    {
        cout << it.first << " : " << it.second << endl;
    }
    cout << endl;
}

void Chat(int clientfd, string str)
{
    int index = str.find(":");
    if(index == -1)
    {
        cerr << "chat command invalid" << endl;
    }
    int friend_id = atoi(str.substr(0, index).c_str());
    string message = str.substr(index+1, str.size()-index);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_current_user.get_id();
    js["name"] = g_current_user.get_name();
    js["to"] = friend_id;
    js["msg"] = message;
    js["time"] = GetCurrentTime();

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "send chat msg error" << endl;
    }
}

void AddFriend(int clientfd, string str)
{
    int friend_id = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_current_user.get_id();
    js["friendid"] = friend_id;

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "send addfriend msg error" << endl;
    }
}

void CreateGroup(int clientfd, string str)
{
    int index = str.find(":");
    if(index == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string group_name = str.substr(0, index);
    string group_desc = str.substr(index+1, str.size()-index);
    
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_current_user.get_id();
    js["groupname"] = group_name;
    js["groupdesc"] = group_desc;
    
    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "crreate group error" << endl;
    }
}

void AddGroup(int clientfd, string str)
{
    int group_id = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_current_user.get_id();
    js["groupid"] = group_id;

    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "add group error" << endl;
    }
}

void GroupChat(int clientfd, string str)
{
    int index = str.find(":");
    if(index == -1)
    {
        cerr << "invalid command" << endl;
    }
    int group_id = atoi(str.substr(0, index).c_str());
    string message = str.substr(index+1, str.size()-index);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_current_user.get_id();
    js["name"] = g_current_user.get_name();
    js["groupid"] = group_id;
    js["msg"] = message;
    js["time"] = GetCurrentTime();

    string request = js.dump();
    
    int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "group chat error" << endl;
    }
}

void LogOut(int clientfd, string)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_current_user.get_id();
    
    string buffer = js.dump();
    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
    if(len == -1)
    {
        cerr << "logout error" << endl;
    }
    else
    {
        g_is_menu_running = false;
        g_current_friends_list.clear();
        g_current_group_list.clear();
    }
}
