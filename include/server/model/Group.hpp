#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include "GroupUser.hpp"

class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
        : id_(id)
        , name_(name)
        , desc_(desc)
    {
    }

    int get_id()
    {
        return id_;
    }

    string get_name()
    {
        return name_;
    }

    string get_desc()
    {
        return desc_;
    }

    vector<GroupUser> &get_users()
    {
        return users_;
    }

    void set_id(int id)
    {
        id_ = id;
    }

    void set_name(string name)
    {
        name_ = name;
    }

    void set_desc(string desc)
    {
        desc_ = desc;
    }

private:
    int id_;
    string name_;
    string desc_;
    vector<GroupUser> users_;
};

#endif