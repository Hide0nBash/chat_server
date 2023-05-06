#include "GroupModel.hpp"
#include "MySQL.hpp"
#include <iostream>

using namespace std;

bool GroupModel::create_group(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) value('%s','%s');",
                  group.get_name().c_str(), group.get_desc().c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.set_id(mysql_insert_id(mysql.get_connection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::add_group(int user_id, int group_id, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser value(%d,%d,'%s')",
                  group_id, user_id, role.c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

vector<Group> GroupModel::query_group(int user_id)
{
    char sql[1024] = {0};
    sprintf(sql, "select  a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on b.groupid=a.id  where b.userid=%d;", user_id);
    
    vector<Group> group_vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.set_id(atoi(row[0]));
                group.set_name(row[1]);
                group.set_desc(row[2]);
                group_vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    
    for(Group &temp : group_vec)
    {
        sprintf(sql, "select  a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid=a.id  where b.groupid=%d;");

        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser group_user;
                group_user.set_id(atoi(row[0]));
                group_user.set_name(row[1]);
                group_user.set_state(row[2]);
                group_user.set_role(row[3]);
                temp.get_users().push_back(group_user);
            }
            mysql_free_result(res);
        }
    }
    return group_vec;
}

vector<int> GroupModel::query_group_users(int user_id, int group_id)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", group_id, user_id);
    
    vector<int> id_vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                id_vec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return id_vec;
}