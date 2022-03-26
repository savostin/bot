#ifndef BETFAIR_H
#define BETFAIR_H

#include "const.h"
#include "http.h"
#include <thread>

using namespace std;
using namespace pugi;

class BetFairAccount;

class BetFair : public HTTP {
private:
  logger_p logger;
  static BetFairAccount *_account;

public:
  BetFair();
  ~BetFair();
  xml_document getSnapshot(const int id);
  static BetFairAccount *account();
};

class BetFairAccount : public BetFair {
private:
  string username;
  string password;
  float available;
  static map<string, float> minBetAmount;
  static chrono::system_clock::time_point str2time(const string &str);
  logger_p logger;
  static void doit(BetFairAccount *acc);
  bool running;
  thread th;

public:
  string currency;
  BetFairAccount();
  ~BetFairAccount();
  bool login(const string username, const string password);
  bool getFunds();
  Funds funds();
  bool placeBet(struct Bet &bet);
  void keepAlive();
  vector<Bet> getBets(const unsigned long channel,
                      const string &status = "ACTIVE");
  vector<Bet> getBetsHistory(const string &status = "SETTLED");
  float getMarketPL(const int marketId);
  vector<Statement> getStatement(const int count = 100, const int from = 0);

  float minBet(float amount);
};

#endif