#ifndef __SERVER_H__
#define __SERVER_H__
#include "const.h"
#include "httplib/httplib.h"
#include "logger.h"
#include <thread>

class Server
{
private:
    httplib::Server server;
    void getLog(const httplib::Request& req, httplib::Response& res);
    logger_p logger;
    thread th;
    unsigned int port;
    static void doit(Server *s);
    json jinit();

public:
    Server();
    void start(const unsigned int port);
    void stop();
    ~Server();
};

#endif // __SERVER_H__