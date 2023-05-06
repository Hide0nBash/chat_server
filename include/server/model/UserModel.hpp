#ifndef USERMODEL_H
#define USERMODEL_H

#include "User.hpp"

class UserModel
{
public:
    bool insert(User &user);
    User query(int id);
    bool update_state(User user);
    bool reset_state();
};

#endif