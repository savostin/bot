#ifndef LOGGER_H
#define LOGGER_H
#include "const.h"


using logger_p = std::shared_ptr<spdlog::logger>;

class Logger
{
private:
    std::vector<spdlog::sink_ptr> sinks;
    static Logger* l;
    static bool created;
public:
    static std::string dir;
    static std::string password;
    static std::string telegramKey;
    static std::string telegramChat;
public:
    Logger();
    void add_sinks();
    ~Logger();
    static logger_p logger(const char* name);
    logger_p get(const char* name);
};

#endif