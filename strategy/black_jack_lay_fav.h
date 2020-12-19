#ifndef STRATEGY_BLACK_JACK_LAY_FAV_H
#define STRATEGY_BLACK_JACK_LAY_FAV_H

#include "../const.h"
#include "../channel/black_jack.h"

class StBlackJackLayFav : public BlackJack
{
private:
    unsigned long placedGameId;
    unsigned long checkedGameId;
    unsigned long lastGameId;
    logger_p logger;

protected:

public:
    StBlackJackLayFav(bool turbo);
    ~StBlackJackLayFav();
    void run(struct Data data);
};

#endif