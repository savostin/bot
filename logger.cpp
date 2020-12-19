#include "logger.h"
#include <regex>

#include "telegram.h"

Logger *Logger::l = 0;
bool Logger::created = false;
string Logger::dir = "./logs/";
string Logger::telegramKey = "";
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

Logger::Logger()
{
}

void Logger::add_sinks()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("%T %L%^[%n] %v%$");
    console_sink->set_level(spdlog::level::debug);
    sinks.push_back(console_sink);

    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fmt::format("{}/actions.log", Logger::dir), 23, 59);
    file_sink->set_pattern("%T\t%n\t%L\t%v");
    file_sink->set_level(spdlog::level::debug);
    sinks.push_back(file_sink);

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

void Logger::setDir(const string _dir)
{
    dir = _dir;
}

void Logger::setTelegram(const string chat, const string key)
{
    telegramKey = key;
    telegramChat = chat;
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
