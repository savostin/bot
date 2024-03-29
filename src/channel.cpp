#include "channel.h"
#include "channel/black_jack.h"
#include "strategy/black_jack_lay_fav.h"

map<const ChannelType, Channel *> Channel::channels;
condition_variable cv;
mutex m;

Channel::Channel(const ChannelType type)
    : BetFair(), th(), _status(STOPPED), id(type) {
  logger = Logger::logger(name().data());
  logger->set_level(spdlog::level::info);
  logger->debug("Create new channel: '{}'", name().data());
  th = thread{doit, 5, this};
}

Channel::~Channel() {}

void Channel::finish() { th.join(); }

void Channel::doit(const int rate, Channel *ths) {
  xml_document xml;
  while (ths->status() != STOPPED) {
    if (ths->status() == RUNNING) {
      xml = ths->getSnapshot(ths->id);
      if (xml) {
        ths->parse(xml);
      }
    }
    this_thread::sleep_for(chrono::seconds(rate));
  }
}

void Channel::status(ChannelStatus s) {
  _status = s;
  string j = "?";
  switch (s) {
  case RUNNING:
    j = _.ChannelStatusRunning;
    break;
  case PAUSED:
    j = _.ChannelStatusPaused;
    break;
  case STOPPED:
    j = _.ChannelStatusStopped;
    break;
  case UNDEFINED:
    j = "?";
    break;
  }
  logger->info("{}: {}", runningStrategy(), j);
}

ChannelStatus Channel::status() { return _status; }

string Channel::name() {
  json j(id);
  return j;
}

int Channel::favIndex(vector<Selection> &selections, const int max) {
  int index = -1, i = 0;
  float minPrice = 1001, curPrice;
  for (vector<Selection>::iterator it = selections.begin();
       it != selections.end(); it++, i++) {
    if (i > max) {
      break;
    }
    curPrice = (*it).backPrice.price;
    if ((*it).status == "IN_PLAY" && curPrice > 0 && curPrice < minPrice) {
      minPrice = curPrice;
      index = i;
    }
  }
  return index;
}

Channel *Channel::create(const StrategyType type) {
  Channel *ch;
  ChannelType t;
  switch (type) {
  case ST_BJT_LF:
    ch = new StBlackJackLayFav(true);
    t = BLACK_JACK_TURBO;
    break;
  case ST_BJ_LF:
    ch = new StBlackJackLayFav(false);
    t = BLACK_JACK;
    break;

  default:
    return NULL;
    break;
  }
  channels[t] = ch;
  return ch;
}

void Channel::status(const ChannelType t, ChannelStatus s) {
  if (t == UNKNOWN) {
    for (map<const ChannelType, Channel *>::iterator it = channels.begin();
         it != channels.end(); it++) {
      it->second->status(s);
    }
  } else if (channels.find(t) != channels.end()) {
    channels[t]->status(s);
  }
}

ChannelStatus Channel::status(const ChannelType t) {
  if (channels.find(t) != channels.end()) {
    return channels[t]->status();
  }
  return UNDEFINED;
}
void Channel::finish(const ChannelType t) {
  if (t == UNKNOWN) {
    for (map<const ChannelType, Channel *>::iterator it = channels.begin();
         it != channels.end(); it++) {
      it->second->finish();
      delete it->second;
    }
    channels.clear();
  } else if (channels.find(t) != channels.end()) {
    Channel *ch = channels[t];
    ch->finish();
    delete ch;
    channels.erase(t);
  }
}
