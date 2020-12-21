#ifndef CHANNEL_H
#define CHANNEL_H

#include "const.h"
#include <thread>
#include "betfair.h"

class Channel : public BetFair
{
private:
    thread th;

protected:
    bool paused;
    bool running;
    const ChannelType id;
    static void doit(const int rate, Channel *th);
    logger_p logger;

public:
    Channel(const ChannelType id);
    virtual ~Channel();
    void start();
    void stop();
    void pause();
    void resume();
    void finish();
    virtual bool parse(xml_document &str) = 0;
    virtual void run(struct Data data) = 0;

    static Channel *create(const StrategyType type);
    static int favIndex(vector<Selection> &selections, const int max = 100);
    static string getName(ChannelType type);
    static string getNameSimple(ChannelType type);

    static map<const ChannelType, Channel *> channels;
    static void pause(const ChannelType t);
    static void resume(const ChannelType t);
    static void stop(const ChannelType t);
    static void start(const ChannelType t);
    static void finish(const ChannelType t);
};

#endif