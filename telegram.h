#ifndef __TELEGRAM_H__
#define __TELEGRAM_H__

#include "const.h"
#include "http.h"

class Telegram : public HTTP
{
private:
    const string token;

public:
    Telegram(const string _token);
    ~Telegram();
    bool send(const string &chat, const string &message);
};

#endif // __TELEGRAM_H__