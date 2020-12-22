#ifndef LOGGER_H
#define LOGGER_H
#include "const.h"
#include "sqlite.h"
//using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;

using logger_p = std::shared_ptr<spdlog::logger>;
template <typename Mutex>
class sqlite_sink;

class Logger
{
private:
    std::vector<spdlog::sink_ptr> sinks;
    static std::shared_ptr<sqlite_sink<std::mutex>> db_sink;
    static Logger *l;
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
    static logger_p logger(const char *name);
    logger_p get(const char *name);
    static nlohmann::json last(const unsigned long last_id);
};

#endif