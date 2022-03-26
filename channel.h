#ifndef CHANNEL_H
#define CHANNEL_H

#include "betfair.h"
#include "const.h"
#include <thread>

class Channel : public BetFair {
private:
  thread th;

protected:
  ChannelStatus _status;
  const ChannelType id;
  static void doit(const int rate, Channel *th);
  logger_p logger;

public:
  Channel(const ChannelType id);
  virtual ~Channel();
  void status(ChannelStatus s);
  ChannelStatus status();
  void finish();
  virtual bool parse(xml_document &str) = 0;
  virtual void run(struct Data data) = 0;
  virtual const string runningStrategy() const = 0;
  string name();

  static Channel *create(const StrategyType type);
  static int favIndex(vector<Selection> &selections, const int max = 100);

  static map<const ChannelType, Channel *> channels;
  static void status(const ChannelType t, ChannelStatus s);
  static ChannelStatus status(const ChannelType t);
  static void finish(const ChannelType t);
};

#endif