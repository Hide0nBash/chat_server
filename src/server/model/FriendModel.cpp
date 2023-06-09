#include "FriendModel.hpp"
#include "MySQL.hpp"

bool FriendModel::insert(int user_id, int friend_id)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d,%d);", user_id, friend_id);
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.query(sql))
        {
            return true;
        }
    }
    return false;
}

vector<User> FriendModel::query(int user_id)
{
    char sql[1024] = {0};
    sprintf(sql, "select  a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id  where b.userid=%d;", user_id);

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.set_id(atoi(row[0]));
                user.set_name(row[1]);
                user.set_state(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}