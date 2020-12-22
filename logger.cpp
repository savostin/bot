#include "logger.h"
#include <regex>

#include "telegram.h"
#include "sqlite.h"

Logger *Logger::l = 0;
bool Logger::created = false;
string Logger::dir = "./logs/";
string Logger::telegramKey = "";
string Logger::password = "";
string Logger::telegramChat = "";
unsigned int Logger::keep_hours = 24;
std::shared_ptr<sqlite_sink<std::mutex>> Logger::db_sink;

template <typename Mutex>
class telegram_sink : public spdlog::sinks::base_sink<Mutex>
{
private:
    Telegram tg;
    const string chat;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        tg.send(chat, fmt::to_string(formatted));
    }

    void flush_() override
    {
    }

public:
    telegram_sink(const string &token, const string &_chat) : spdlog::sinks::base_sink<Mutex>(), tg(token), chat(_chat)
    {
    }
};
using telegram_sink_mt = telegram_sink<std::mutex>;

template <typename Mutex>
class sqlite_sink : public spdlog::sinks::base_sink<Mutex>
{
private:
    mydb db;
    myst pinsert;
    myst pselect;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        try
        {
            pinsert << msg.level << msg.logger_name.data() << db.encrypt(fmt::to_string(formatted));
            pinsert++;
        }
        catch (sqlite::sqlite_exception &e)
        {
            cerr << e.get_code() << ": " << e.what() << " during "
                 << e.get_sql() << endl;
        }
        catch (...)
        {
        }
    }

    void flush_() override
    {
        std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point tt = t - std::chrono::hours(Logger::keep_hours);
        db << "delete from log where ts < ?;" << std::chrono::duration_cast<std::chrono::seconds>(tt.time_since_epoch()).count();
        db << "VACUUM;";
    }

public:
    ~sqlite_sink() noexcept override {}
    sqlite_sink(const string &file, const string &password) noexcept : spdlog::sinks::base_sink<Mutex>(),
                                                                       db(file, password, "create table if not exists log ("
                                                                                          " id integer primary key autoincrement not null, "
                                                                                          " ts timestamp default (strftime('%s', 'now')), "
                                                                                          " level int, "
                                                                                          " section text, "
                                                                                          " message text "
                                                                                          ");"),
                                                                       pinsert(db, "insert into log (level, section, message) values (?, ?, ?);"),
                                                                       pselect(db, "select id, ts, level, section, message from log where id > ? order by id desc limit 1000;")
    {
    }
    nlohmann::json last(const unsigned long last_id)
    {
        json lines = json::array();
        pselect << last_id >>
            [&](unsigned long id, int ts, int level, string section, string message) {
                lines.push_back({
                    {"id", id},
                    {"ts", ts},
                    {"level", level},
                    {"section", section},
                    {"message", db.decrypt(message)},
                });
            };
        return lines;
    }
};

using sqlite_sink_mt = sqlite_sink<std::mutex>;

Logger::Logger()
{
}

void Logger::add_sinks()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("%T %L%^[%n] %v%$");
    console_sink->set_level(spdlog::level::debug);
    sinks.push_back(console_sink);

    /*   
    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fmt::format("{}/actions.log", Logger::dir), 23, 59);
    file_sink->set_pattern("%T\t%n\t%L\t%v");
    file_sink->set_level(spdlog::level::debug);
    sinks.push_back(file_sink);
*/

    auto sqlite_sink = std::make_shared<sqlite_sink_mt>(fmt::format("{}/log.db", dir), password);
    sqlite_sink->set_pattern("%v");
    sqlite_sink->set_level(spdlog::level::debug);
    auto formatter = std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, "");
    sqlite_sink->set_formatter(std::move(formatter));

    sinks.push_back(sqlite_sink);
    db_sink = sqlite_sink;

    if (!telegramChat.empty())
    {
        auto telegram_sink = std::make_shared<telegram_sink_mt>(telegramKey, telegramChat);
        telegram_sink->set_pattern("[%n] %L %v");
        telegram_sink->set_level(spdlog::level::warn);
        sinks.push_back(telegram_sink);
    }
}

Logger::~Logger()
{
}

logger_p Logger::get(const char *name)
{
    std::shared_ptr<spdlog::logger> l = std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
    l->flush_on(spdlog::level::warn);
    return l;
}

nlohmann::json Logger::last(const unsigned long last_id)
{
    return db_sink->last(last_id);
}

logger_p Logger::logger(const char *name)
{
    if (!created)
    {
        created = true;
        l = new Logger();
        l->add_sinks();
    }
    return l->get(name);
}
