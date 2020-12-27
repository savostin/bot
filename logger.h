#ifndef LOGGER_H
#define LOGGER_H
#include "const.h"

using logger_p = std::shared_ptr<spdlog::logger>;

class Logger
{
private:
    static std::vector<spdlog::sink_ptr> sinks;
    static unsigned int keep_hours;

public:
    static std::string telegramKey;
    static std::string telegramChat;

public:
    static void init(unsigned int keep_hours);
    static logger_p logger(const char *name);
    static nlohmann::json last(const unsigned long last_id);
};

#endif