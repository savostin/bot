#include "telegram.h"

Telegram::Telegram(const string _token) : HTTP(_token.empty() ? "https://bf-bj.com" : "https://api.telegram.org"), token(_token)
{
    setKeepAlive(false);
}

Telegram::~Telegram()
{
}

bool Telegram::send(const string &chat, const string &message)
{
    json j = {{"chat_id", chat}, {"text", message}, {"disable_notification", false}};
    json j2 = Request(token.empty() ? "/tg/" : fmt::format("/bot{}/sendMessage", token), j);
    return j2["ok"] == true;
}
