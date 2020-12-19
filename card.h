#ifndef CARD_H
#define CARD_H
#include "const.h"
#include "math.h"

enum CardSuit
{
    Clubs = 0,
    Diamonds,
    Hearts,
    Spades,
    Unknown
};

class Card
{
private:
    static const vector<string> ranks;

public:
    static CardSuit suit(int val);
    static string sym_color(int val);
    static string sym(int val);
    static string rank(int val);
};


#endif