#ifndef CONST_H
#define CONST_H
#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#define SPDLOG_NO_THREAD_ID
#define SPDLOG_FMT_EXTERNAL

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <curses.h>

#include "pugixml/pugixml.hpp"
#include "json/json.hpp"

#include "fmt/core.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/chrono.h"

#define SPDLOG_SHORT_LEVEL_NAMES          \
    {                                     \
        "❗", "🔔", "💡", "👉", "❌", "💀", "O" \
    }
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "logger.h"

#define VERSION_NAME "BetFair Exchange Bot"
#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_PATCH 10

using namespace std;
using namespace pugi;
using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;

#define SOFTWARE "Individual.com.bfbj.1.0"

enum StrategyType
{
    ST_BJ_LF,
    ST_BJT_LF
};

enum ChannelType
{
    UNKNOWN = 0,
    BLACK_JACK = 1444077,
    BLACK_JACK_TURBO = 1444082,

};

NLOHMANN_JSON_SERIALIZE_ENUM(ChannelType, {
                                                {UNKNOWN, "ALL"},
                                                {BLACK_JACK, "BLACK_JACK"},
                                                {BLACK_JACK_TURBO, "BLACK_JACK_TURBO"},
                                            })

enum ChannelStatus 
{
    UNDEFINED = 0,
    STOPPED,
    RUNNING,
    PAUSED
};

NLOHMANN_JSON_SERIALIZE_ENUM(ChannelStatus, {
                                                {UNDEFINED, "UNDEFINED"},
                                                {STOPPED, "STOPPED"},
                                                {RUNNING, "RUNNING"},
                                                {PAUSED, "PAUSED"},
                                            })

struct Player
{
    string name;
    string status;
    vector<int> cards;
};

struct Price
{
    float price;
    double amount;
};

struct Selection
{
    int id;
    string name;
    string status;
    double matched;
    Price backPrice;
    Price layPrice;
};

struct Market
{
    int id;
    string status;
    float commission;
    string type;
    vector<Selection> selections;
};

struct Data
{
    unsigned long id;
    string status;
    int round;
    int second;
    int left;
    vector<Player> players;
    vector<Market> markets;
};

enum StatementType
{
    PL,
    COMMISSION,
    DEPOSIT,
    WITHDRAWAL
};

NLOHMANN_JSON_SERIALIZE_ENUM(StatementType, {
                                                {PL, "P/L"},
                                                {COMMISSION, "COMMISSION"},
                                                {DEPOSIT, "DEPOSIT"},
                                                {WITHDRAWAL, "WITHDRAWAL"},
                                            })

struct Statement
{
    unsigned long id;
    chrono::system_clock::time_point timestamp;
    StatementType type;
    string description;
    float amount = 0;
    float balance = 0;
    json toJson()
    {
        json j = {
            {"id", id},
            {"timestamp", fmt::format("{:%FT%T%z}", fmt::localtime(chrono::system_clock::to_time_t(timestamp)))},
            {"type", type},
            {"description", description},
            {"amount", amount},
            {"balance", balance}};
        return j;
    }
};

enum Side
{
    BACK,
    LAY
};

enum BetStatus
{
    BetReady,
    BetPlaced,
    BetMatched,
    BetUnmatched,
    BetError,
    BetWon,
    BetLost
};

struct Bet
{
    int round = 0;
    Market market;
    Side side;
    Selection selection;
    float price = 0;
    float amount = 0;
    unsigned long id = 0;
    BetStatus status = BetReady;
    float pl = 0;
    chrono::system_clock::time_point placed = chrono::system_clock::now();
};

struct Funds
{
    float available;
    string currency;
};

struct Proxy
{
    string server = "";
    unsigned int port = 0;
    string username = "";
    string password = "";
};

enum EventType
{
    E_NONE,
    E_EXIT,
    E_PAUSE,
    E_RESUME
};

struct Event
{
    EventType type;
    long long int data;
};


#include "events.h"


#endif
