#include "logger.h"
#include <regex>

#include "db.h"
#include "md5.h"
#include "telegram.h"

string Logger::telegramKey = "";
string Logger::telegramChat = "";
unsigned int Logger::keep_hours = 24;
std::vector<spdlog::sink_ptr> Logger::sinks;

template <typename Mutex>
class telegram_sink : public spdlog::sinks::base_sink<Mutex> {
private:
  Telegram tg;
  const string chat;

protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    tg.send(chat, fmt::to_string(formatted));
  }

  void flush_() override {}

public:
  telegram_sink(const string &token, const string &_chat)
      : spdlog::sinks::base_sink<Mutex>(), tg(token), chat(_chat) {}
};
using telegram_sink_mt = telegram_sink<std::mutex>;

template <typename Mutex>
class sqlite_sink : public spdlog::sinks::base_sink<Mutex> {
private:
  sqlite::database_binder tmp;
  sqlite::database_binder pinsert;

protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    try {
      pinsert << msg.level << msg.logger_name.data()
              << fmt::to_string(formatted);
      pinsert++;
    } catch (sqlite::sqlite_exception &e) {
      cerr << e.get_code() << ": " << e.what() << ", " << e.get_sql() << endl;
    } catch (...) {
    }
  }

  void flush_() override {
    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tt =
        t - std::chrono::hours(Logger::keep_hours);
    DB::o() << "delete from log where ts < ?;"
            << std::chrono::duration_cast<std::chrono::seconds>(
                   tt.time_since_epoch())
                   .count();
    DB::o() << "VACUUM;";
  }

public:
  ~sqlite_sink() noexcept override {}
  sqlite_sink() noexcept
      : spdlog::sinks::base_sink<Mutex>(),
        tmp(DB::o() << "create table if not exists log ("
                       " id integer primary key autoincrement not null, "
                       " ts timestamp default (strftime('%s', 'now')), "
                       " level int, "
                       " section text, "
                       " message text "
                       ");"),
        pinsert(
            DB::o()
            << "insert into log (level, section, message) values (?, ?, ?);") {
    cout << "@@@" << endl;
  }

  void rekey(const string new_key) {
    flush_();
    DB::o().rekey(new_key);
  }
};

using sqlite_sink_mt = sqlite_sink<std::mutex>;

void Logger::init(unsigned int _keep_hours) {
  keep_hours = _keep_hours;
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern("%T %L%^%n → %v%$");
  console_sink->set_level(spdlog::level::debug);
  sinks.push_back(console_sink);
  try {
    cout << "!1!!" << endl;
    auto sqlite_sink = std::make_shared<sqlite_sink_mt>();
    cout << "!!!" << endl;
    sqlite_sink->set_pattern("%v");
    sqlite_sink->set_level(spdlog::level::debug);
    auto formatter = std::make_unique<spdlog::pattern_formatter>(
        "%v", spdlog::pattern_time_type::local, "");
    sqlite_sink->set_formatter(std::move(formatter));

    sinks.push_back(sqlite_sink);
  } catch (...) {
    cerr << "???" << endl;
  }

  if (!telegramChat.empty()) {
    auto telegram_sink =
        std::make_shared<telegram_sink_mt>(telegramKey, telegramChat);
    telegram_sink->set_pattern("[%n] %L %v");
    telegram_sink->set_level(spdlog::level::warn);
    sinks.push_back(telegram_sink);
  }
}

nlohmann::json Logger::last(const unsigned long last_id) {
  json lines = json::array();
  DB::o() << "select id, ts, level, section, message from log where id > ? "
             "order by id desc limit 1000;"
          << last_id >>
      [&](unsigned long id, int ts, int level, string section, string message) {
        lines.push_back({
            {"id", id},
            {"ts", ts},
            {"level", level},
            {"section", section},
            {"message", message},
        });
      };
  return lines;
}

logger_p Logger::logger(const char *name) {
  std::shared_ptr<spdlog::logger> ret =
      std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
  ret->flush_on(spdlog::level::warn);
  return ret;
}
