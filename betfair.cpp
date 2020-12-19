#include "betfair.h"
#include "channel.h"
#include "md5.h"

BetFairAccount *BetFairAccount::account = NULL;

map<string, float> BetFairAccount::minBetAmount{
    {"EUR", 2},
    {"DKK", 30},
    {"USD", 4},
    {"GBP", 2},
    {"AUD", 6},
    {"CAD", 6},
    {"HKD", 25},
    {"NOK", 30},
    {"SGD", 6},
    {"SEK", 30}};

BetFair::BetFair() : HTTP("https://api.games.betfair.com")
{
    logger = Logger::logger("BETFAIR");
    logger->set_level(spdlog::level::info);
}

BetFair::~BetFair()
{
}

xml_document BetFair::getSnapshot(const int id)
{
    logger->debug("getSnapshot {}", id);
    return Request(fmt::format("/rest/v1/channels/{}/snapshot", id), HTTP::nothing);
}

BetFairAccount::BetFairAccount() : BetFair(), username(""), password(""), funds(0), running(true), th()
{
    logger = Logger::logger("ACCOUNT");
    logger->set_level(spdlog::level::info);
}

BetFairAccount::~BetFairAccount()
{
    running = false;
    if (!username.empty())
    {
        th.join();
    }
}

BetFairAccount *BetFairAccount::get()
{
    if (BetFairAccount::account == NULL)
    {
        BetFairAccount::account = new BetFairAccount();
    }
    return BetFairAccount::account;
}

void BetFairAccount::del()
{
    if (BetFairAccount::account)
        delete BetFairAccount::account;
}

bool BetFairAccount::login(const string _username, const string _password)
{
    username = _username;
    password = _password;
    logger->info("Logging in as '{}'", username);
    string instance = fmt::format("{}:{}", SOFTWARE, username);
    addHeaders({{"gamexAPIPassword", password},
                {"gamexAPIAgent", SOFTWARE},
                {"gamexAPIAgentInstance", MD5String(instance)}});
    th = thread{doit, this};
    return getFunds();
}

bool BetFairAccount::getFunds()
{
    xml_document doc = Request(fmt::format("/rest/v1/account/snapshot?username={}", username), HTTP::nothing);
    xml_node root = doc.child("accountSnapshot");
    if (root)
    {
        currency = root.attribute("currency").as_string();
        funds = stof(root.child_value("availableToBetBalance"));
        logger->warn("Funds: {:.2f} {}", funds, currency);
        return true;
    }
    logger->error("No account snapshot");
    return false;
}

bool BetFairAccount::placeBet(struct Bet &bet)
{
    system("say \"attention please\"");

    string sideName = bet.side == BACK ? "BACK" : "LAY";
    bet.status = BetPlaced;
    logger->warn("Placing {} bet on '{}': {:.2f} {} @ {:.2f}...", sideName, bet.selection.name, bet.amount, currency, bet.price);
    xml_document send;
    send.load_string(fmt::format("<postBetOrder xmlns=\"urn:betfair:games:api:v1\" marketId=\"{:d}\" round=\"{:d}\" currency=\"{}\"><betPlace><bidType>{}</bidType><price>{:.2f}</price><size>{:.2f}</size><selectionId>{:d}</selectionId></betPlace></postBetOrder>",
                                 bet.market.id,
                                 bet.round,
                                 currency,
                                 sideName,
                                 /*(bet.side == BACK ? 100 : 1.02),*/ bet.price,
                                 minBet(bet.amount),
                                 bet.selection.id)
                         .c_str());
    xml_document doc = Request(fmt::format("/rest/v1/bet/order?username={}", username), send);
    xml_node betResult = doc.child("responseBetOrder").child("betPlacementResult");
    if (betResult)
    {
        bet.status = string(betResult.child_value("resultCode")) == string("OK") ? BetMatched : BetError;
        bet.id = atol(betResult.child_value("betId"));
        logger->warn("Bet result: {}, id {}", betResult.child_value("resultCode"), betResult.child_value("betId"));
        getFunds();
        return true;
    }
    else
    {
        logger->error("Xml error");
    }
    bet.status = BetError;
    return false;
}


void BetFairAccount::keepAlive()
{
    logger->debug("Keep alive");
    xml_document doc = Request(fmt::format("/rest/v1/account/snapshot?username={}", username), HTTP::nothing);
}

vector<Bet> BetFairAccount::getBetsHistory(const string &status)
{
    vector<Bet> bets;
    logger->debug("Getting bets history of {} bets", status);
    xml_document doc = Request(fmt::format("/rest/v1/bet/history?username={}&betStatus={}", username, status), HTTP::nothing);
    if (doc)
    {
        xml_node root = doc.child("betHistory");
        for (xml_node item = root.child("betHistoryItem"); item; item = item.next_sibling("betHistoryItem"))
        {
            Bet b;
            b.id = stol(item.child_value("betId"));
            b.side = string(item.child_value("bidType")) == string("BACK") ? BACK : LAY;
            b.price = stof(item.child_value("price"));
            b.amount = stof(item.child_value("size"));
            b.pl = stof(item.child_value("profitLoss"));
            b.round = stoi(item.child_value("roundNumber"));
            b.status = b.pl < 0 ? BetLost : BetWon;
            b.placed = str2time(item.child_value("placedDate"));

            xml_node sn = item.child("selectionReference");
            Selection s;
            s.id = stol(sn.child_value("selectionId"));
            s.name = sn.child_value("selectionName");

            xml_node mn = sn.child("marketReference");
            Market m;
            m.id = stol(mn.child_value("marketId"));

            b.selection = s;
            b.market = m;

            logger->debug("Got bet {:d}: '{}' {:.2f} @ {:.2f} in {:d} round placed {:%Y-%m-%d %H:%M:%S}. Result: {} with p/l {:.2f}",
                          b.id,
                          b.selection.name,
                          b.amount,
                          b.price,
                          b.round,
                          fmt::localtime(b.placed),
                          (b.status == BetWon ? "WON" : "LOST"),
                          b.pl);

            bets.push_back(b);
        }
    }
    else
    {
        logger->error("Non-xml response");
    }
    return bets;
}

vector<Bet> BetFairAccount::getBets(const unsigned long channel, const string &status)
{
    vector<Bet> bets;
    logger->debug("Getting current {} bets for channel {}", status, channel);
    xml_document doc = Request(fmt::format("/rest/v1/bet/snapshot?username={}&betStatus={}&channelId={:d}", username, status, channel), HTTP::nothing);
    if (doc)
    {
        xml_node root = doc.child("betSnapshot");
        for (xml_node item = root.child("betSnapshotItem"); item; item = item.next_sibling("betSnapshotItem"))
        {
            Bet b;
            b.id = stol(item.child_value("betId"));
            b.side = string(item.child_value("bidType")) == string("BACK") ? BACK : LAY;
            b.price = stof(item.child_value("price"));
            b.amount = stof(item.child_value("size"));
            b.pl = 0;
            b.round = stoi(item.child_value("roundNumber"));
            b.status = string(item.child_value("priceMatched")).empty() ? BetUnmatched : BetMatched;
            b.placed = str2time(item.child_value("placedDate"));

            xml_node sn = item.child("selectionReference");
            Selection s;
            s.id = stol(sn.child_value("selectionId"));
            s.name = sn.child_value("selectionName");

            xml_node mn = sn.child("marketReference");
            Market m;
            m.id = stol(mn.child_value("marketId"));

            b.selection = s;
            b.market = m;

            logger->debug("Got bet {:d}: '{}' {:.2f} @ {:.2f} in {:d} round placed {:%Y-%m-%d %H:%M:%S}. Status: {}",
                          b.id,
                          b.selection.name,
                          b.amount,
                          b.price,
                          b.round,
                          fmt::localtime(b.placed),
                          (b.status == BetMatched ? "MATCHED" : "UNMATCHED"));

            bets.push_back(b);
        }
    }
    else
    {
        logger->error("Non-xml response");
    }
    return bets;
}

vector<Statement> BetFairAccount::getStatement(const ChannelType channel, const int count, const int from)
{
    vector<Statement> statement;
    string channelName = Channel::getNameSimple(channel);
    string typeName = "";
    logger->debug("Getting account statement for channel {} {} / {}", channelName, count, from);
    xml_document doc = Request(fmt::format("/rest/v1/account/statement?username={}&recordCount={:d}&startRecord={:d}{}", username, count, from, (channel == UNKNOWN ? "" : fmt::format("&account={}", channelName))), HTTP::nothing);
    if (doc)
    {
        xml_node root = doc.child("accountStatement");
        for (xml_node item = root.child("item"); item; item = item.next_sibling("item"))
        {
            Statement s;
            s.id = item.attribute("refNo").as_ullong();
            s.timestamp = str2time(item.child_value("settledDate"));
            s.amount = stof(item.child_value("amount"));
            s.balance = stof(item.child_value("balance"));
            xml_node asdb = item.child("accountStatementDescription").child("accountStatementBetDescription");
            if (asdb)
            {
                s.type = PL;
                typeName = "P/L";
                s.description = fmt::format("{} {:.2f} {} @ {:.2f} '{}': {}",
                                            asdb.child_value("bidType"),
                                            stof(asdb.child_value("size")),
                                            currency,
                                            stof(asdb.child_value("size")),
                                            asdb.child("selectionReference").child_value("selectionName"),
                                            asdb.child_value("winLose"));
            }
            xml_node asdd = item.child("accountStatementDescription").child("accountStatementTransferDescription");
            if (asdd)
            {
                s.type = string(asdd.child_value("paymentType")) == string("DEPOSIT") ? DEPOSIT : WITHDRAWAL;
                typeName = asdd.child_value("paymentType");
                s.description = "";
            }
            xml_node asdc = item.child("accountStatementDescription").child("accountStatementCommissionDescription");
            if (asdc)
            {
                s.type = COMMISSION;
                typeName = "COMMISSION";
                s.description = fmt::format("{}%", asdc.child_value("commissionRate"));
            }
            logger->debug("Got record {:d}: {:%Y-%m-%d %H:%M:%S} {} {:+.2f} {:.2f} {}",
                          s.id,
                          fmt::localtime(s.timestamp),
                          typeName,
                          s.amount,
                          s.balance,
                          s.description);
            statement.push_back(s);
        }
    }
    else
    {
        logger->error("Non-xml response");
    }
    return statement;
}

float BetFairAccount::minBet(float amount)
{
    return max(amount, BetFairAccount::minBetAmount[currency]);
}

chrono::system_clock::time_point BetFairAccount::str2time(const string &str)
{
    tm tm = {};
    strptime(str.c_str(), "%Y-%m-%dT%H:%M:%Sz", &tm);
    return chrono::system_clock::from_time_t(mktime(&tm));
}

void BetFairAccount::doit(BetFairAccount *acc)
{
    auto start = std::chrono::system_clock::now();
    while (acc->running)
    {
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        if (elapsed_seconds.count() > 60)
        {
            start = std::chrono::system_clock::now();
            acc->keepAlive();
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}
