#include "server.h"
#include "betfair.h"

Server::Server() : th()
{
    server.set_mount_point("/", "./www");

    server.Get("/statement.json", [&](const httplib::Request & /*req*/, httplib::Response &res) {
        std::time_t t = std::time(nullptr);
        json items = json::array();
        vector<Statement> st = BetFair::account()->getStatement();
        logger->debug("Statement: {:d}", st.size());
        for(vector<Statement>::iterator i=st.begin(); i!=st.end(); i++)
            {
                items.push_back((*i).toJson());
            }
        json j = {
            {"timestamp", fmt::format("{:%FT%T%z}", fmt::localtime(t))},
            {"items", items}
        };
        res.set_content(j.dump(), "application/json");
    });

    server.Get("/funds.json", [&](const httplib::Request & /*req*/, httplib::Response &res) {
        std::time_t t = std::time(nullptr);
        json items = json::array();
        Funds funds = BetFair::account()->funds();
        logger->debug("Funds: {:.2f} {}", funds.available, funds.currency);
        json j = {
            {"timestamp", fmt::format("{:%FT%T%z}", fmt::localtime(t))},
            {"funds", funds.available},
            {"currency", funds.currency}
        };
        res.set_content(j.dump(), "application/json");
    });

    logger = Logger::logger("SERVER");
    logger->set_level(spdlog::level::debug);
}

void Server::start(const unsigned int _port)
{
    port = _port;
    th = thread{doit, this};
}

void Server::stop()
{
    if (port > 0)
    {
        logger->info("Stopping...");
        server.stop();
        th.join();
    }
}

Server::~Server()
{
}

void Server::doit(Server *s)
{
    s->logger->debug("Getting ip address...");
    
    s->logger->info("Starting web-server on http://localhost:{:d}/", s->port);
    if (!s->server.listen("127.0.0.1", s->port))
    {
        s->logger->error("Unable to bind http server on port {:d} - port is busy?", s->port);
    }
}
