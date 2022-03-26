#include "server.h"
#include "channel.h"
#include "crypt.h"

#define XKEY "X-Key"

Server::Server() : th() {
  logger = Logger::logger(string(_.LoggerServer).data());
  logger->set_level(spdlog::level::info);

  server.set_mount_point("/", "./www");
  server.Get("/statement.json", [&](const httplib::Request &req,
                                    httplib::Response &res) {
    json j = jinit();
    if (req.has_header(XKEY) && checkKey(req.get_header_value(XKEY))) {
      json items = json::array();
      vector<Statement> st = BetFair::account()->getStatement();
      logger->debug("Statement: {:d}", st.size());
      json jj;
      for (vector<Statement>::iterator i = st.begin(); i != st.end(); i++) {
        jj = (*i).toJson();
        jj["secret"] = Crypt::crypt->encrypt((string)jj["secret"]);
        items.push_back(jj);
      }
      j["items"] = items;
    } else {
      j["error"] = "AUTH";
    }
    res.set_content(j.dump(), "application/json");
  });

  server.Get("/channels.json", [&](const httplib::Request &req,
                                   httplib::Response &res) {
    json j = jinit();
    if (req.has_header(XKEY) && checkKey(req.get_header_value(XKEY))) {
      json channels = json::array();
      for (map<const ChannelType, Channel *>::iterator i =
               Channel::channels.begin();
           i != Channel::channels.end(); i++) {
        Channel *c = i->second;
        channels.push_back({
            {"id", (int)i->first},
            {"name", i->second->name()},
            {"strategy", i->second->runningStrategy()},
            {"status", c->status()},
        });
      }
      j["channels"] = channels;
    } else {
      j["error"] = "AUTH";
    }
    res.set_content(j.dump(), "application/json");
  });

  server.Get(
      "/log.json", [&](const httplib::Request &req, httplib::Response &res) {
        json j = jinit();
        if (req.has_header(XKEY) && checkKey(req.get_header_value(XKEY))) {
          unsigned long last_id =
              req.has_param("from") ? stol(req.get_param_value("from")) : 0;
          logger->debug("Get logs from {:d}", last_id);
          j["lines"] = Logger::last(last_id);
          Funds funds = BetFair::account()->funds();
          j["funds"] = Crypt::crypt->encrypt(to_string(funds.available));
          j["currency"] = funds.currency;
        } else {
          j["error"] = "AUTH";
        }
        res.set_content(j.dump(), "application/json");
      });

  server.Get("/control/(pause|resume)_([A-Z_]+).json",
             [&](const httplib::Request &req, httplib::Response &res) {
               json j = jinit();
               if (req.has_header(XKEY) &&
                   checkKey(req.get_header_value(XKEY))) {
                 string m = req.matches[1].str();
                 string channel = req.matches[2].str();
                 json tmp = channel;
                 ChannelType cht = tmp.get<ChannelType>();
                 logger->debug("Control: {} {} ({:d})", m, channel, cht);
                 EventType tp = E_NONE;
                 if (m == "pause") {
                   tp = E_PAUSE;
                 } else if (m == "resume") {
                   tp = E_RESUME;
                 }
                 Events::loop()->notify(tp, cht);
                 j["result"] = "OK";
               } else {
                 j["error"] = "AUTH";
               }
               res.set_content(j.dump(), "application/json");
             });

  server.Get("/control/exit.json", [&](const httplib::Request &req,
                                       httplib::Response &res) {
    json j = jinit();
    if (req.has_header(XKEY) && checkKey(req.get_header_value(XKEY))) {
      logger->debug("Control: exit");
      Events::loop()->notify(E_EXIT);
      j["result"] = "OK";
    } else {
      j["error"] = "AUTH";
    }
    res.set_content(j.dump(), "application/json");
  });
}

void Server::start(const unsigned int _port) {
  port = _port;
  th = thread{doit, this};
}

void Server::stop() {
  if (port > 0) {
    logger->info(_.ServerStopping);
    server.stop();
    th.join();
  }
}

Server::~Server() {}

void Server::doit(Server *s) {
  s->logger->info(_.ServerStarting, s->port);
  if (!s->server.listen("127.0.0.1", s->port)) {
    s->logger->error(_.ServerStartingError, s->port);
  }
}

json Server::jinit() {
  std::time_t t = std::time(nullptr);
  json j = {{"timestamp", fmt::format("{:%FT%T%z}", fmt::localtime(t))},
            {
                "software",
                {
                    {"name", VERSION_NAME},
                    {"version", fmt::format("{}.{}.{}", VERSION_MAJOR,
                                            VERSION_MINOR, VERSION_PATCH)},
                },
            }};
  return j;
}

bool Server::checkKey(const string &key) {
  string ds = Crypt::crypt->decrypt(key);
  if (ds.empty()) {
    return false;
  }
  tm timeinfo;
  if (fmt::detail::strftime((char *)ds.c_str(), ds.length(),
                            "%Y-%m-%dT%H:%M:%S", &timeinfo) > 0) {
    std::time_t tt = std::mktime(&timeinfo);
    chrono::system_clock::time_point tp = chrono::system_clock::from_time_t(tt);
    return chrono::system_clock::now() - tp > chrono::minutes(5);
  }
  return false;
}
