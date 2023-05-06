#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "Group.hpp"
#include <string>
#include <vector>

using namespace std;

class GroupModel
{
public:
    bool add_group(int user_id, int group_id, string role);
    bool create_group(Group &group);

    vector<Group> query_group(int user_id);
    vector<int> query_group_users(int user_id, int group_id);
};

#endif
