#ifndef CHANNEL_BLACK_JACK_H
#define CHANNEL_BLACK_JACK_H

#include "../const.h"
#include "../channel.h"

class BlackJack : public Channel
{
public:
    enum MarketIndex { Main = 0 };
    BlackJack(bool turbo);
    ~BlackJack();
    virtual bool parse(xml_document &doc);
    static unsigned int points(vector<int> &cards);

};

#endif