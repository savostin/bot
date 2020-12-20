#include "http.h"

xml_document HTTP::nothing = xml_document();

HTTP::HTTP(const string _host) : client(_host.c_str()), lastError(""), lastStatus(0), host(_host)
{
    headers = {
        {"Cache-Control", "no-cache"},
        {"User-Agent", "BF-bj 1.0.1"},
        {"Accept-Encoding", "gzip, deflate"},
        {"Connection", "Keep-Alive"},
        {"Keep-Alive", "timeout=50, max=10000"},
        {"Accept", "*/*"}};
    logger = Logger::logger("HTTP");
    logger->set_level(spdlog::level::info);
    logger->info("{}", OPENSSL_VERSION_TEXT);
    client.set_tcp_nodelay(true);
    client.set_keep_alive(true);
    client.set_default_headers(headers);
    client.enable_server_certificate_verification(false);
}

HTTP::~HTTP()
{
}

void HTTP::addHeaders(httplib::Headers _headers)
{
    headers.insert(_headers.begin(), _headers.end());
    client.set_default_headers(headers);
}

void HTTP::removeHeader(const string &name)
{
    headers.erase(name);
}

struct xml_string_writer : xml_writer
{
    string result;
    virtual void write(const void *data, size_t size)
    {
        result.append(static_cast<const char *>(data), size);
    }
};

string HTTP::Request(const string &url, const string &data, const string &contentType)
{
    lastError = "";
    lastStatus = 0;
    httplib::Result res = data.empty() ? client.Get(url.c_str()) : client.Post(url.c_str(), data, contentType.c_str());
    if (res)
    {
        logger->debug("{}: {}, {}", url, res->status, res.error());
        lastStatus = res->status;
        if (res->status == 200)
        {
            return res->body;
        }
        logger->error("{}: Status: {} Error: {}", url, res->status, res.error());
        cout << res->body << endl;
        lastError = res.error();
    }
    else
    {
        logger->error("ERROR: {}", error(res.error()));
        lastError = fmt::format("ERROR {}", error(res.error()));
        client.stop();
    }
    return "";
}

xml_document HTTP::Request(const string &url, xml_document &data)
{
    xml_document doc;
    struct xml_string_writer xw;
    if (data != HTTP::nothing)
    {
        data.save(xw);
    }
    string d = Request(url, xw.result, "application/xml; charset=UTF-8");
    if (lastError.empty())
    {
        xml_parse_result res = doc.load_string(d.c_str());
        if (!res)
        {
            logger->error("XML: {}", res.description());
            logger->debug(d);
            lastError = res.description();
        }
    }
    return doc;
}

json HTTP::Request(const string &url, json &data)
{
    string res = Request(url, data.dump(), "application/json");
    try
    {
        return json::parse(res.c_str());
    }
    catch (json::parse_error &e)
    {
        logger->error(e.what());
    }
    return json();
}

string HTTP::error(httplib::Error code)
{
    switch (code)
    {
    case httplib::Error::Success:
        return "";
        break;
    case httplib::Error::Unknown:
        return "Unknown error";
        break;
    case httplib::Error::Connection:
        return "Unable to connect";
        break;
    case httplib::Error::BindIPAddress:
        return "Unable to bind ip address";
        break;
    case httplib::Error::Read:
        return "Socket read error";
        break;
    case httplib::Error::Write:
        return "Socket write error";
        break;
    case httplib::Error::ExceedRedirectCount:
        return "Exceed Redirect Count";
        break;
    case httplib::Error::Canceled:
        return "Cancelled";
        break;
    case httplib::Error::SSLConnection:
        return "SSL connection failed";
        break;
    case httplib::Error::SSLLoadingCerts:
        return "SSL loading certs failed";
        break;
    case httplib::Error::SSLServerVerification:
        return "SSL server verification failed";
        break;
    case httplib::Error::UnsupportedMultipartBoundaryChars:
        return "Unsupported multipart boundary chars";
        break;
    case httplib::Error::Compression:
        return "Compression failed";
        break;
    }
return "WTF";
}