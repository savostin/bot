#include "lang.h"
#include <fmt/format.h>

Lang Language::current = ENGLISH;
Language Language::l;

L::operator std::string() const {
#ifndef DEBUG
  if (v.at(Language::current) == string("???")) {
    cerr << "No translation" << endl; // TODO
  }
#endif
  return v.at(Language::current);
}

L::operator spdlog::string_view_t() const {
#ifndef DEBUG
  if (v.at(Language::current) == string("???")) {
    cerr << "No translation" << endl; // TODO
  }
#endif
  return v.at(Language::current);
}

L::L(const std::string &en, const std::string &ru)
    : v({{ENGLISH, en}, {RUSSIAN, ru}}) {}