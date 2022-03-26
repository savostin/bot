#include "http.h"

xml_document HTTP::no_xml = xml_document();
Proxy HTTP::proxy = {};

HTTP::HTTP(const string _host)
    : client(_host.c_str()), lastError(""), lastStatus(0), host(_host) {
  logger = Logger::logger("HTTP");
  logger->set_level(spdlog::level::info);
  client.set_tcp_nodelay(true);
  client.enable_server_certificate_verification(false);
  addHeaders({{"Cache-Control", "no-cache"},
              {"User-Agent", "BF-bj 1.0.1"},
              {"Accept-Encoding", "gzip, deflate"},
              {"Accept", "*/*"}});
  if (HTTP::proxy.port > 0) {
    client.set_proxy(HTTP::proxy.server.c_str(), HTTP::proxy.port);
    client.set_proxy_basic_auth(HTTP::proxy.username.c_str(),
                                HTTP::proxy.password.c_str());
  }
}

HTTP::~HTTP() {}

void HTTP::addHeaders(httplib::Headers _headers) {
  headers.insert(_headers.begin(), _headers.end());
  client.set_default_headers(headers);
}

void HTTP::removeHeader(const string &name) { headers.erase(name); }

struct xml_string_writer : xml_writer {
  string result;
  virtual void write(const void *data, size_t size) {
    result.append(static_cast<const char *>(data), size);
  }
};

string HTTP::Request(const string &url, const string &data,
                     const string &contentType) {
  lastError = "";
  lastStatus = 0;
  httplib::Result res =
      data.empty() ? client.Get(url.c_str())
                   : client.Post(url.c_str(), data, contentType.c_str());
  if (res) {
    logger->debug("{} {}: {}, {}", (data.empty() ? "GET" : "POST"), url,
                  res->status, res.error());
    lastStatus = res->status;
    if (res->status == 200) {
      return res->body;
    }
    logger->error(_.HttpStatusError, url, res->status, res.error());
#ifdef DEBUG
    cout << res->body << endl;
#endif
    lastError = res.error();
  } else {
    logger->error("{}", error(res.error()));
    lastError = error(res.error());
    client.stop();
  }
  return "";
}

xml_document HTTP::Request(const string &url, xml_document &data) {
  xml_document doc;
  struct xml_string_writer xw;
  if (data != HTTP::no_xml) {
    data.save(xw);
  }
  string d = Request(url, xw.result, "application/xml; charset=UTF-8");
  if (lastError.empty()) {
    xml_parse_result res = doc.load_string(d.c_str());
    if (!res) {
      logger->error(_.HttpXmlError, res.description());
      logger->debug(d);
      lastError = res.description();
    }
  }
  return doc;
}

json HTTP::Request(const string &url, json &data) {
  string res = Request(url, data.empty() ? "" : data.dump(),
                       "application/json; charset=UTF-8");
  try {
    return json::parse(res.c_str());
  } catch (json::parse_error &e) {
    logger->error(e.what());
  } catch (...) {
    logger->error(_.HttpJsonError, res);
  }
  return json();
}

string HTTP::error(httplib::Error code) {
  switch (code) {
  case httplib::Error::Success:
    return "";
    break;
  case httplib::Error::Unknown:
    return _.HttpErrorUnknown;
    break;
  case httplib::Error::Connection:
    return _.HttpErrorConnect;
    break;
  case httplib::Error::BindIPAddress:
    return _.HttpErrorBind;
    break;
  case httplib::Error::Read:
    return _.HttpErrorSocketRead;
    break;
  case httplib::Error::Write:
    return _.HttpErrorSocketWrite;
    break;
  case httplib::Error::ExceedRedirectCount:
    return _.HttpErrorRedirect;
    break;
  case httplib::Error::Canceled:
    return _.HttpErrorCancelled;
    break;
  case httplib::Error::SSLConnection:
    return _.HttpErrorSSLFailed;
    break;
  case httplib::Error::SSLLoadingCerts:
    return _.HttpErrorSSLLoad;
    break;
  case httplib::Error::SSLServerVerification:
    return _.HttpErrorSSLVerification;
    break;
  case httplib::Error::UnsupportedMultipartBoundaryChars:
    return _.HttpErrorMultipart;
    break;
  case httplib::Error::Compression:
    return _.HttpErrorCompression;
    break;
  }
  return "WTF";
}

void HTTP::setKeepAlive(bool keep) {
  client.set_keep_alive(keep);
  removeHeader("Connection");
  removeHeader("Keep-Alive");
  if (keep) {
    addHeaders({{"Connection", "close"}});
  } else {
    addHeaders({{"Connection", "Keep-Alive"},
                {"Keep-Alive", "timeout=50, max=10000"}});
  }
}

void HTTP::setProxy(const string &server, const unsigned int port,
                    const string username, const string password) {
  proxy.server = server;
  proxy.port = port;
  proxy.username = username;
  proxy.password = password;
}