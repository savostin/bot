#include "black_jack_lay_fav.h"
#include "../channel.h"
#include "card.h"

StBlackJackLayFav::StBlackJackLayFav(bool turbo) : BlackJack(turbo), placedGameId(0), checkedGameId(0), lastGameId(0)
{
    logger = Logger::logger("BJ-LF");
    logger->set_level(spdlog::level::debug);
    logger->debug("Create new strategy: BJ-LF");
}

StBlackJackLayFav::~StBlackJackLayFav()
{
}

void StBlackJackLayFav::run(struct Data data)
{
    if (checkedGameId != data.id)
    {
        const int mi = BlackJack::MarketIndex::Main;
        //logger->debug("----------------------");
        //logger->debug("Status: {}", data.markets[mi].status);
        if (data.markets[mi].status == "ACTIVE")
        {
            //logger->debug("Round: {:d}", data.round);
            if (data.round == 1 && placedGameId)
            {
                vector<Statement> stmt = BetFairAccount::get()->getStatement(BLACK_JACK_TURBO, 3);
                BetFairAccount::get()->getFunds();
                placedGameId = 0;
            }
            else if (data.round == 2)
            {
                checkedGameId = data.id;
                int favIndex = Channel::favIndex(data.markets[mi].selections, 4);
                if (favIndex >= 0)
                {
                    logger->debug("---------------------- {:d}", data.id);
                    Player player = data.players[favIndex];
                    Selection selection = data.markets[mi].selections[favIndex];
                    logger->debug("Favorite: {}", player.name);
                    logger->debug("Price: {:.2f}", selection.layPrice.price);
                    if (selection.layPrice.price > 1 && selection.layPrice.price < 4)
                    {
                        if (player.cards.size() > 0)
                        {
                            if (player.cards.size() >= 2)
                            {
                                logger->debug("Cards: {} {}", Card::sym(player.cards[0]), Card::sym(player.cards[1]));
                            }
                            else
                            {
                                logger->debug("Cards: [{}]", player.cards[0]);
                            }
                            if (Card::rank(player.cards[0]) != "A" && (player.cards.size() == 1 || Card::rank(player.cards[1]) != "A"))
                            {
                                unsigned int points = BlackJack::points(player.cards);
                                logger->debug("Points: {:d}", points);
                                if (points >= 12 && points <= 16)
                                {
                                    logger->info("{:d}: LAY {} - {:.2f} EUR @ {:.2f}", data.id, selection.name, 4 / (selection.layPrice.price - 1), selection.layPrice.price);
                                    placedGameId = data.id;
                                    Bet bet;
                                    bet.round = data.round;
                                    bet.market = data.markets[mi];
                                    bet.side = LAY;
                                    bet.selection = selection;
                                    bet.price = selection.layPrice.price;
                                    bet.amount = 4 / (bet.price - 1);
                                    BetFair::account.placeBet(bet);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (data.id != lastGameId && (lastGameId == 0 || lastGameId != placedGameId) && data.round == 1)
        {
            if (lastGameId != 0)
            {
                logger->info("Game {:d} skipped", lastGameId);
            }
            lastGameId = data.id;
        }
    }
}