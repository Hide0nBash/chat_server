#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MSG = 1,    //login msg
    LOGIN_MSG_ACK,    //login reply
    REG_MSG,          //regist msg
    REG_MSG_ACK,      //regist reply
    ONE_CHAT_MSG,     //one chat one
    ADD_FRIEND_MSG,

    CREATE_GROUP_MSG, //create group msg
    ADD_GROUP_MSG,    //join group msg
    GROUP_CHAT_MSG,   //group msg

    LOGINOUT_MSG,     //logout msg
};

#endif