#ifndef MYSQL_H
#define MYSQL_H

#include <string>
#include <mysql/mysql.h>
#include <mymuduo/Logger.h>

using namespace std;

#define SERVER "localhost"
#define USER "lgh"
#define PASSWORD "123456"
#define DBNAME "chat"

class MySQL
{
public:
    MySQL()
    {
        conn_ = mysql_init(nullptr);
    }

    ~MySQL()
    {
        if(conn_ != nullptr)
        {
            mysql_close(conn_);
        }
    }

    bool connect()
    {
        MYSQL *p = mysql_real_connect(conn_, "localhost",
             "lgh", "123456", "chat", 3306, nullptr, 0);
        if(p != nullptr)
        {
            mysql_query(conn_, "set names gbk");
            LOG_INFO("connect mysql success!!!");
        }
        else
        {
            LOG_INFO("connect mysql failed???");
        }
        return p;
    }

    bool update(string sql)
    {
        if(mysql_query(conn_, sql.c_str()))
        {
            LOG_INFO("update error");
            //something i don t know
            return false;
        }
        return true;
    }

    MYSQL_RES* query(string sql)
    {
        if(mysql_query(conn_, sql.c_str()))
        {
            LOG_INFO("select error???");
            //something unknown
            return nullptr;
        }
        return mysql_use_result(conn_);
    }

    MYSQL* get_connection()
    {
        return conn_;
    }

private:
    MYSQL* conn_;
};

#endif