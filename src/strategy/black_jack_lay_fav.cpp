#include "black_jack_lay_fav.h"
#include "../channel.h"
#include "card.h"
#define V_MAJOR 1
#define V_MINOR 1

StBlackJackLayFav::StBlackJackLayFav(bool turbo) : BlackJack(turbo), placedGameId(0), checkedGameId(0), lastGameId(0)
{
    logger = Logger::logger(string(_.LoggerBfBj).data());
    logger->set_level(spdlog::level::debug);
    logger->warn(_.StrategyStarting, runningStrategy(), V_MAJOR, V_MINOR);
}

StBlackJackLayFav::~StBlackJackLayFav()
{
}

void StBlackJackLayFav::run(struct Data data)
{
    if (checkedGameId != data.id)
    {
        const int mi = BlackJack::MarketIndex::Main;
        logger->debug("----------------------");
        logger->debug("Status: {}", data.markets[mi].status);
        if (data.markets[mi].status == "ACTIVE")
        {
            //logger->debug("Round: {:d}", data.round);
            if (data.round == 1 && placedGameId)
            {
                vector<Statement> stmt = BetFair::account()->getStatement(BLACK_JACK_TURBO, 3);
                float pl = BetFair::account()->getMarketPL(placedMarketId);
                logger->warn(_.StrategyLastPL, pl);
                BetFair::account()->getFunds();
                Funds f = BetFair::account()->funds();
                logger->warn(_.AccountFunds, f.available, f.currency);
                placedGameId = 0;
                placedMarketId = 0;
            }
            else if (data.round == 2)
            {
                checkedGameId = data.id;
                int favIndex = Channel::favIndex(data.markets[mi].selections, 4);
                if (favIndex >= 0)
                {
                    logger->debug(_.StrategyChecking, data.id);
                    Player player = data.players[favIndex];
                    Selection selection = data.markets[mi].selections[favIndex];
                    logger->debug(_.StrategyFavorite, player.name);
                    logger->debug(_.StrategyPrice, selection.layPrice.price);
                    if (selection.layPrice.price > 1 && selection.layPrice.price < 4)
                    {
                        if (player.cards.size() > 0)
                        {
                            if (player.cards.size() >= 2)
                            {
                                logger->debug(_.StrategyCards, Card::sym(player.cards[0]), Card::sym(player.cards[1]));
                            }
                            else
                            {
                                logger->debug(_.StrategyCards, player.cards[0]);
                            }
                            if (Card::rank(player.cards[0]) != "A" && (player.cards.size() == 1 || Card::rank(player.cards[1]) != "A"))
                            {
                                unsigned int points = BlackJack::points(player.cards);
                                logger->debug(_.StrategyPoints, points);
                                if (points >= 12 && points <= 16)
                                {
                                    placedGameId = data.id;
                                    placedMarketId = data.markets[mi].id;
                                    Bet bet;
                                    bet.round = data.round;
                                    bet.market = data.markets[mi];
                                    bet.side = LAY;
                                    bet.selection = selection;
                                    bet.price = selection.layPrice.price;
                                    bet.amount = 4 / (bet.price - 1);
                                    logger->info(_.StrategyBet, data.id, selection.name, bet.amount, BetFair::account()->currency, selection.layPrice.price);
                                    BetFair::account()->placeBet(bet);
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
                logger->info(_.StrategySkipped, lastGameId);
            }
            lastGameId = data.id;
        }
    }
}

const string StBlackJackLayFav::runningStrategy() const
{
    return "Black Jack Lay Favorite";
}