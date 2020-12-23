#include "betfair.h"
#include "channel.h"
#include "md5.h"

BetFairAccount *BetFair::_account = NULL;

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
    setKeepAlive(true);
    logger = Logger::logger("BETFAIR");
    logger->set_level(spdlog::level::info);
}

BetFair::~BetFair()
{
}

xml_document BetFair::getSnapshot(const int id)
{
    logger->debug("getSnapshot {}", id);
    return Request(fmt::format("/rest/v1/channels/{}/snapshot", id), HTTP::no_xml);
}

BetFairAccount::BetFairAccount() : BetFair(), username(""), password(""), available(0), running(true), th()
{
    logger = Logger::logger(string(_.LoggerAccount).data());
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

BetFairAccount *BetFair::account()
{
    if (BetFair::_account == NULL)
    {
        BetFair::_account = new BetFairAccount();
    }
    return BetFair::_account;
}

bool BetFairAccount::login(const string _username, const string _password)
{
    username = _username;
    password = _password;
    logger->info(_.AccountLogin, username);
    string instance = fmt::format("{}:{}", SOFTWARE, username);
    addHeaders({{"gamexAPIPassword", password},
                {"gamexAPIAgent", SOFTWARE},
                {"gamexAPIAgentInstance", MD5String(instance)}});
    th = thread{doit, this};
    if (getFunds())
    {
        logger->warn(_.AccountFunds, available, currency);
        return true;
    }
    return false;
}

bool BetFairAccount::getFunds()
{
    xml_document doc = Request(fmt::format("/rest/v1/account/snapshot?username={}", username), HTTP::no_xml);
    if (lastStatus == 200)
    {
        xml_node root = doc.child("accountSnapshot");
        if (root)
        {
            currency = root.attribute("currency").as_string();
            available = stof(root.child_value("availableToBetBalance"));
            return true;
        }
        logger->error(_.AccountNoSnapshot);
    }
    else if (lastStatus == 401)
    {
        logger->error(_.AccountNoAuth);
    }
    return false;
}

Funds BetFairAccount::funds()
{
    Funds f;
    f.available = available;
    f.currency = currency;
    return f;
}

bool BetFairAccount::placeBet(struct Bet &bet)
{
    string sideName = bet.side == BACK ? "BACK" : "LAY";
    string sideNameInt = bet.side == BACK ? _.BACK : _.LAY;
    bet.status = BetPlaced;
    logger->warn(_.AccountPlacingBet, sideNameInt, bet.selection.name, bet.amount, currency, bet.price);
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
        logger->warn(_.AccountBetResult, betResult.child_value("resultCode"), betResult.child_value("betId"));
        getFunds();
        logger->warn(_.AccountFunds, available, currency);
        return true;
    }
    else
    {
        logger->error(_.ChannelNoXml);
    }
    bet.status = BetError;
    return false;
}

void BetFairAccount::keepAlive()
{
    logger->debug("Keep alive");
    getFunds();
}

vector<Bet> BetFairAccount::getBetsHistory(const string &status)
{
    vector<Bet> bets;
    logger->debug("Getting bets history of {} bets", status);
    xml_document doc = Request(fmt::format("/rest/v1/bet/history?username={}&betStatus={}", username, status), HTTP::no_xml);
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
        logger->error(_.ChannelNoXml);
    }
    return bets;
}

float BetFairAccount::getMarketPL(const int marketId)
{
    float pl = 0;
    vector<Bet> bh = getBetsHistory();
    for (vector<Bet>::iterator i = bh.begin(); i != bh.end(); i++)
    {
        if ((*i).market.id == marketId)
        {
            pl += (*i).pl;
        }
    }
    return pl;
}

vector<Bet> BetFairAccount::getBets(const unsigned long channel, const string &status)
{
    vector<Bet> bets;
    logger->debug("Getting current {} bets for channel {}", status, channel);
    xml_document doc = Request(fmt::format("/rest/v1/bet/snapshot?username={}&betStatus={}&channelId={:d}", username, status, channel), HTTP::no_xml);
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
        logger->error(_.ChannelNoXml);
    }
    return bets;
}

vector<Statement> BetFairAccount::getStatement(const ChannelType channel, const int count, const int from)
{
    vector<Statement> statement;
    string channelName = Channel::getNameSimple(channel);
    string typeName = "";
    logger->debug("Getting account statement for channel {} {} / {}", channelName, count, from);
    xml_document doc = Request(fmt::format("/rest/v1/account/statement?username={}&recordCount={:d}&startRecord={:d}{}", username, count, from, (channel == UNKNOWN ? "" : fmt::format("&account={}", channelName))), HTTP::no_xml);
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
                s.description = fmt::format("{} - {} {:.2f} {} @ {:.2f} '{}': {}",
                                            asdb.child("selectionReference").child("marketReference").child_value("channelName"),
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
                s.description = fmt::format("{}% - {}", asdc.child_value("commissionRate"), asdc.child("marketReference").child_value("channelName"));
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
        logger->error(_.ChannelNoXml);
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
