#include "OfflineMessageModel.hpp"
#include "MySQL.hpp"

bool OfflineMessageModel::insert(int id, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d,'%s')", id, msg.c_str());

    
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

bool OfflineMessageModel::remove(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", id);

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

vector<string> OfflineMessageModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select msg from OfflineMessage where userid=%d", id);

    vector<string> msg_vec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                msg_vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return msg_vec;
}