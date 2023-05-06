#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

using namespace std;
using redis_handler = function<void(int, string)>;

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect();
    bool publish(int channel, string message);
    bool subscribe(int channel);
    bool unsubscribe(int channel);

    void observer_channel_message();
    void init_notify_handler(redis_handler handler);
private:
    redisContext *publish_context_;
    redisContext *subscribe_context_;
    redis_handler notify_message_handler_;
};

#endif