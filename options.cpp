#include "options.h"
#include "db.h"

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

void setStdinEcho(bool enable = true) {
#ifdef WIN32
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode;
  GetConsoleMode(hStdin, &mode);

  if (!enable)
    mode &= ~ENABLE_ECHO_INPUT;
  else
    mode |= ENABLE_ECHO_INPUT;

  SetConsoleMode(hStdin, mode);

#else
  struct termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (!enable)
    tty.c_lflag &= ~ECHO;
  else
    tty.c_lflag |= ECHO;

  (void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

Options::Options() {
  DB::o() << "create table if not exists data ("
             " n text primary key not null, "
             " v text);";
  data = {};
  load();
}

Options::~Options() { save(); }

bool Options::load() {
  DB::o() << "select v from data where n = 'options';" >> [&](string v) {
    try {
      data = json::parse(v);
      cout << "Load options:  " << v << endl << data << endl;
    } catch (...) {
      cerr << "Options load error" << endl;
    }
  };
  return data != NULL;
}

void Options::save() {
  cout << "Save options: " << data.dump() << endl;
  try {
    DB::o() << "delete from data where n = 'options';";
    cout << "!" << endl;
    DB::o() << "insert into data (n, v) values ('options', ?);" << data.dump();
    cout << "!" << endl;
  } catch (...) {
    cerr << "WTF" << endl;
  }
}

int Options::get(std::string name, int def) {
  try {
    return data.at(name).get<int>();
  } catch (...) {
    return def;
  }
}

std::string Options::get(std::string name, std::string def) {
  try {
    return data.at(name).get<std::string>();
  } catch (...) {
    return def;
  }
}

void Options::set(std::string name, int val) { data[name] = val; }

void Options::set(std::string name, std::string val) { data[name] = val; }

std::string Options::ask(const std::string &q, bool hide) {
  std::string ret;
  cout << q;
  if (hide) {
    setStdinEcho(false);
  }
  cin >> ret;
  if (hide) {
    setStdinEcho(true);
    cout << endl;
  }
  return ret;
}
