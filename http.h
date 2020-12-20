#ifndef HTTP_H
#define HTTP_H
#include "const.h"
#include "httplib/httplib.h"
#include "logger.h"

using namespace pugi;

class HTTP
{
protected:
    httplib::Client client;
    string lastError;
    int lastStatus;

private:
    httplib::Headers headers;
    logger_p logger;
    string host;
    static Proxy proxy;

public:
    HTTP(const string host);
    ~HTTP();
    void addHeaders(httplib::Headers headers);
    void removeHeader(const string &name);
    string Request(const string &url, const string &data = "", const string &contentType = "");
    xml_document Request(const string &url, xml_document &data);
    json Request(const string &url, json &data);
    static xml_document no_xml;
    static string error(httplib::Error code);
    void setKeepAlive(bool keep);
    static void setProxy(const string &server, const unsigned int port, const string username = "", const string password = "");
};

#endif