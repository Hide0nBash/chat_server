#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

using namespace std;

class OfflineMessageModel
{
public:
    //store user offline msg
    bool insert(int id, string msg);
    
    //delete user offline msg
    bool remove(int id);

    //search user msg
    vector<string> query(int id);
};

#endif