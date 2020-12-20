#include "server.h"

Server::Server() : th()
{
    server.set_mount_point("/", "./www");
    server.Get("/log", [&](const httplib::Request & /*req*/, httplib::Response &res) {
        res.set_content("hello", "text/html");
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
