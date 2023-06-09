#include "UserModel.hpp"
#include "User.hpp"
#include "MySQL.hpp"
#include <iostream>

using namespace std;

bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s');",
            user.get_name().c_str(), user.get_password().c_str(), user.get_state().c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            user.set_id(mysql_insert_id(mysql.get_connection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d;", id);
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);        
            if(row  != nullptr)
            {    
                User user;
                user.set_id(atoi(row[0]));
                user.set_name(row[1]);
                user.set_password(row[2]);
                user.set_state(row[3]);

                //free room
                mysql_free_result(res);

                return user;
            }
        }
    }
    return User();
}

bool UserModel::update_state(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = %d;",
                  user.get_state().c_str(), user.get_id());
    
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

bool UserModel::reset_state()
{
    char sql[1024];
    sprintf(sql, "update User set state = 'offline' where state = 'online';");
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