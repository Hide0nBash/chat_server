#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>
#include "User.hpp"

using namespace std;

class FriendModel
{
public:
    bool insert(int user_id, int friend_id);
    vector<User> query(int user_id);
};

#endif