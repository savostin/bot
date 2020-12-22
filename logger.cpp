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
    myst ps;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        try
        {
            ps << msg.level << msg.logger_name.data() << db.encrypt(fmt::to_string(formatted));
            ps++;
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
                                                                       ps(db, "insert into log (level, section, message) values (?, ?, ?);")
    {
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
    l->flush_on(spdlog::level::info);
    return l;
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
