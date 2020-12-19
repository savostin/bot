#include "channel.h"
#include "channel/black_jack.h"
#include "strategy/black_jack_lay_fav.h"

condition_variable cv;
mutex m;

Channel::Channel(const ChannelType type) : BetFair(), th(), paused(true), running(true), id(type)
{
    logger = Logger::logger(Channel::getName(type).data());
    logger->set_level(spdlog::level::info);
    logger->debug("Create new channel: '{}'", Channel::getName(type).data());
    th = thread{doit, 5, this};
}

Channel::~Channel()
{
}

void Channel::finish()
{
    th.join();
}

void Channel::doit(const int rate, Channel *ths)
{
    xml_document xml;
    while (ths->running)
    {
        if (!ths->paused)
        {
            xml = ths->getSnapshot(ths->id);
            if (xml)
            {
                ths->parse(xml);
            }
        }
        this_thread::sleep_for(chrono::seconds(rate));
    }
}

void Channel::start()
{
    logger->info("Started");
    paused = false;
}

void Channel::stop()
{
    logger->info("Stopped");
    running = false;
}

void Channel::pause()
{
    logger->info("Paused");
    paused = true;
}

void Channel::resume()
{
    logger->info("Resumed");
    paused = false;
}

string Channel::getName(ChannelType type)
{
    switch (type)
    {
    case BLACK_JACK_TURBO:
        return "BLACK JACK TURBO";
    case BLACK_JACK:
        return "BLACK JACK";
    case UNKNOWN:
        return "CHANNEL";
    }
    return "";
}

string Channel::getNameSimple(ChannelType type)
{
    switch (type)
    {
    case BLACK_JACK_TURBO:
    case BLACK_JACK:
        return "BLACKJACK";
    case UNKNOWN:
        return "";
    }
    return "";
}

int Channel::favIndex(vector<Selection> &selections, const int max)
{
    int index = -1, i = 0;
    float minPrice = 1001, curPrice;
    for (vector<Selection>::iterator it = selections.begin(); it != selections.end(); it++, i++)
    {
        if (i > max)
        {
            break;
        }
        curPrice = (*it).backPrice.price;
        if ((*it).status == "IN_PLAY" && curPrice > 0 && curPrice < minPrice)
        {
            minPrice = curPrice;
            index = i;
        }
    }
    return index;
}

Channel *Channel::create(const StrategyType type)
{
    switch (type)
    {
    case ST_BJT_LF:
        return new StBlackJackLayFav(true);
        break;
    case ST_BJ_LF:
        return new StBlackJackLayFav(false);
        break;

    default:
        break;
    }
    return NULL;
}