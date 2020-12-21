#ifndef __EVENTS_H__
#define __EVENTS_H__
#include "const.h"

#include <queue>
#include <mutex>
#include <condition_variable>

class Events
{
private:
    std::queue<Event> q;
    mutable std::mutex m;
    std::condition_variable c;
    static Events *_loop;

public:
    Events();
    ~Events();
    void push(Event e);
    Event pop(void);

    static Events *loop();

    template <class T>
    static void notify(EventType type, T data)
    {
        Event e = {type, (long long int)data};
        _loop->push(e);
    }
    static void notify(EventType type)
    {
        Event e = {type, 0};
        _loop->push(e);
    }
};

#endif // __EVENTS_H__