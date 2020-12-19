#include "const.h"
#include "channel.h"
#include <csignal>
#include "argagg.hpp"

vector<Channel *> running;
logger_p logger;

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

void setStdinEcho(bool enable = true)
{
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

void signal_handler(int)
{
	logger->info("Stopping...");
	for (vector<Channel *>::iterator it = running.begin(); it != running.end(); it++)
	{
		(*it)->stop();
	}
}

int main(int argc, char **argv)
{

	argagg::parser argparser{{
		{"version", {"-V", "--version"}, "Print version and exit", 0},
		{"help", {"-h", "--help"}, "Shows this help message", 0},
		{"logs", {"-l", "--logs"}, "Location of logs dir [./logs/]", 1},
		{"username", {"-u", "--user"}, "BetFair username", 1},
		{"password", {"-p", "--password"}, "BetFair password", 1},
		{"telegram_chat", {"-c", "--chat"}, "Telegram chat id", 1},
		{"telegram_key", {"-k", "--key"}, "Telegram bot key", 1},
	}};

	argagg::parser_results args;
	try
	{
		args = argparser.parse(argc, argv);
	}
	catch (const std::exception &e)
	{
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	if (args["help"])
	{
		cerr << fmt::format("Usage: {} [options]", argv[0]) << endl
			 << argparser;
		return EXIT_SUCCESS;
	}

	if (args["version"])
	{
		cerr << VERSION_NAME << " v." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << endl;
		return EXIT_SUCCESS;
	}

	string logs = args["logs"].as<string>("./logs/");
	Logger::setDir(logs);

	string username = args["username"].as<string>("");
	while (username.empty())
	{
		cout << "Enter BetFair username: ";
		cin >> username;
	}

	string password = args["password"].as<string>("");
	while (password.empty())
	{
		cout << "Enter BetFair password: ";
		setStdinEcho(false);
		cin >> password;
		setStdinEcho(true);
		cout << endl;
	}

	string t_chat = args["telegram_chat"].as<string>("");
	if (!t_chat.empty())
	{
		string t_key = args["telegram_key"].as<string>("");
		Logger::setTelegram(t_chat, t_key);
	}

	signal(SIGINT, signal_handler);
	logger = Logger::logger("APP");
	logger->set_level(spdlog::level::info);
	logger->info("Welcome to {} v.{}.{}.{}!", VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	if (!t_chat.empty() && !args["telegram_key"])
	{
		logger->warn("Telegram key is empty! Your Telegram messages will be sent through a proxy.");
	}

	try
	{
		if (BetFairAccount::get()->login(username, password))
		{
			Channel *s = Channel::create(ST_BJT_LF);
			s->start();
			running.push_back(s);
			for (vector<Channel *>::iterator it = running.begin(); it != running.end(); it++)
			{
				(*it)->finish();
				delete *it;
			}
		}
	}
	catch (const std::bad_alloc &c)
	{
		system("say oops");
		logger->error(c.what());
		BetFairAccount::del();
		return EXIT_FAILURE;
	}
	catch (char *e)
	{
		system("say oops");
		logger->error(e);
		BetFairAccount::del();
		return EXIT_FAILURE;
	}
	catch (...)
	{
		system("say oops");
		BetFairAccount::del();
		return EXIT_FAILURE;
	}
	logger->info("Bye!");
	BetFairAccount::del();

	return EXIT_SUCCESS;
}
