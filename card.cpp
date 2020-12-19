#include "card.h"

const vector<string> Card::ranks({"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"});

CardSuit Card::suit(int val)
{
    return (CardSuit)(val < 0 ? 4 : (floor(float(val) / 13.)));
}

string Card::sym_color(int val)
{
    if (val < 0)
        return "";
    string s;
    switch (Card::suit(val))
    {
    case Clubs:
        s = "\u001b[30m♣";
        break;
    case Diamonds:
        s = "\u001b[31m♦";
        break;
    case Hearts:
        s = "\u001b[31m♥";
        break;
    case Spades:
        s = "\u001b[30m♠";
        break;
    default:
        s = "?";
    }
    return Card::rank(val) + "\u001b[47m" + s + "\u001b[0m";
}

string Card::sym(int val)
{
    if (val < 0)
        return "";
    string s;
    switch (Card::suit(val))
    {
    case Clubs:
        s = "♣";
        break;
    case Diamonds:
        s = "♦";
        break;
    case Hearts:
        s = "♥";
        break;
    case Spades:
        s = "♠";
        break;
    default:
        s = "?";
    }
    return Card::rank(val) + s;
}

string Card::rank(int val)
{
    return val < 0 ? "?" : Card::ranks.at(val % 13);
}