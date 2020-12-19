#include "black_jack.h"

BlackJack::BlackJack(bool turbo) : Channel(turbo ? BLACK_JACK_TURBO : BLACK_JACK)
{
}

BlackJack::~BlackJack()
{
}

bool BlackJack::parse(xml_document &doc)
{
    struct Data data;
    xml_node root = doc.child("channelSnapshot").child("channel");
    if (root)
    {
        data.status = root.child("status").child_value();
        xml_node game = root.child("game");
        if (game)
        {
            data.id = game.attribute("id").as_ullong();
            data.round = stoi(game.child("round").child_value());
            logger->debug("{}: round {}", data.status, data.round);
            data.second = stoi(game.child("bettingWindowTime").child_value()) * 0.01 * stoi(game.child("bettingWindowPercentageComplete").child_value());
            data.left = stoi(game.child("bettingWindowTime").child_value()) * 0.01 * (100 - stoi(game.child("bettingWindowPercentageComplete").child_value()));
            xml_node gameData = game.child("gameData");
            if (gameData)
            {
                for (xml_node object = gameData.child("object"); object; object = object.next_sibling("object"))
                {
                    vector<int> cards;
                    for (xml_node card = object.child("property"); card; card = card.next_sibling("property"))
                    {
                        cards.push_back(string(card.attribute("value").as_string()) == string("NOT AVAILABLE") ? -1 : card.attribute("value").as_int());
                    }
                    Player player;
                    player.name = object.attribute("name").as_string();
                    player.status = object.child("status").child_value();
                    player.cards = cards;
                    data.players.push_back(player);
                }
                xml_node market = game.child("markets").child("market");
                if (market)
                {
                    Market mainMarket;
                    mainMarket.id = market.attribute("id").as_int();
                    mainMarket.commission = stof(market.child("commissionRate").child_value());
                    mainMarket.status = market.child("status").child_value();
                    mainMarket.type = market.child("marketType").child_value();
                    xml_node selections = market.child("selections");
                    if (selections)
                    {
                        for (xml_node selection = selections.child("selection"); selection; selection = selection.next_sibling("selection"))
                        {
                            Selection sel;
                            sel.id = selection.attribute("id").as_int();
                            sel.name = selection.child("name").child_value();
                            sel.status = selection.child("status").child_value();
                            sel.matched = stod(selection.child("amountMatched").child_value());

                            xml_node bps = selection.child("bestAvailableToBackPrices");
                            if (bps)
                            {
                                xml_node bpr = bps.child("price");
                                if (bpr)
                                {
                                    Price pr;
                                    pr.price = stof(bpr.child_value());
                                    pr.amount = bpr.attribute("amountUnmatched").as_double();
                                    sel.backPrice = pr;
                                }
                            }

                            xml_node lps = selection.child("bestAvailableToLayPrices");
                            if (lps)
                            {
                                xml_node lpr = lps.child("price");
                                if (lpr)
                                {
                                    Price pr;
                                    pr.price = stof(lpr.child_value());
                                    pr.amount = lpr.attribute("amountUnmatched").as_double();
                                    sel.layPrice = pr;
                                }
                            }
                            mainMarket.selections.push_back(sel);
                        }
                    }
                    else
                    {
                        logger->error("No selections");
                    }
                    data.markets.push_back(mainMarket);
                }
                else
                {
                    logger->error("No market");
                }
                run(data);
                return true;
            }
            else
            {
                logger->error("No xml game data");
            }
        }
        else
        {
            logger->error("No xml game");
        }
    }
    else
    {
        logger->error("No xml root");
    }
    return false;
}

unsigned int BlackJack::points(vector<int> &cards)
{
    unsigned int pts = 0;
    unsigned int cds = 0;
    unsigned int aces = 0;
    unsigned int val;
    int card;
    for (vector<int>::iterator it = cards.begin(); it != cards.end(); it++)
    {
        card = (*it);
        if (card < 0)
            continue;
        if (card >= 0)
        {
            val = card % 13;
            if (val == 0)
            {
                aces++;
                pts += 11;
            }
            else if (val >= 10)
            {
                pts += 10;
            }
            else
            {
                pts += val + 1;
            }
            cds++;
        }
    }
    if (pts > 21)
    {
        if (aces > 0)
            pts -= aces * 10;
    }

    return pts;
}