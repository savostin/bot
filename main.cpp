#include "const.h"
#include "channel.h"
#include "server.h"
#include <csignal>
#include "argagg.hpp"
#include "events.h"

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
	Events::loop()->notify(E_EXIT);
}

int main(int argc, char **argv)
{
	int exit_code = EXIT_SUCCESS;
	argagg::parser_results args;
	argagg::parser argparser;
	string lang = "";
	short i = 0;
	while (i++ < 2) // WTF?! Not more than 2 times?
	{
		argparser = {{
			{"lang", {"--lang"}, _.UsageLanguage, 1},
			{"logs", {"-l", "--logs"}, _.UsageLogs, 1},
			{"keep", {"--keep"}, _.UsageKeep, 1},
			{"username", {"-u", "--user"}, _.UsageBetFairUsername, 1},
			{"password", {"-p", "--password"}, _.UsageBetFairPassword, 1},
			{"port", {"--port"}, _.UsagePort, 1},
			{"proxy_server", {"--proxy-server"}, _.UsageProxyServer, 1},
			{"proxy_port", {"--proxy-port"}, _.UsageProxyPort, 1},
			{"proxy_username", {"--proxy-username"}, _.UsageProxyUsername, 1},
			{"proxy_password", {"--proxy-password"}, _.UsageProxyPassword, 1},
			{"telegram_chat", {"-c", "--chat"}, _.UsageTelegramChat, 1},
			{"telegram_key", {"-k", "--key"}, _.UsageTelegramKey, 1},
			{"version", {"-V", "--version"}, _.UsageVersion, 0},
			{"help", {"-h", "--help"}, _.UsageHelp, 0},
		}};

		try
		{
			args = argparser.parse(argc, argv);
		}
		catch (const std::exception &e)
		{
			cerr << e.what() << '\n';
			return EXIT_FAILURE;
		}

		lang = args["lang"].as<string>("en");
		if (lang == "en")
		{
			Language::current = ENGLISH;
		}
		else if (lang == "ru")
		{
			Language::current = RUSSIAN;
		}
		else
		{
			cerr << string(_.UsageWrongLang) << endl;
			return EXIT_FAILURE;
		}
	}

	if (args["help"])
	{
		cerr << fmt::format((string)_.Usage, argv[0]) << endl
			 << argparser;
		return EXIT_SUCCESS;
	}

	if (args["version"])
	{
		cout << VERSION_NAME << " v." << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << endl;
		return EXIT_SUCCESS;
	}

	string logs = args["logs"].as<string>("./logs/");
	Logger::dir = logs;

	unsigned int keep_hours = args["keep"].as<unsigned int>(24);
	Logger::keep_hours = keep_hours;

	if (args["proxy_port"] && args["proxy_server"])
	{
		HTTP::setProxy(args["proxy_server"].as<string>(""), args["proxy_port"].as<int>(0), args["proxy_username"].as<string>(""), args["proxy_password"].as<string>(""));
	}

	string username = args["username"].as<string>("");
	while (username.empty())
	{
		cout << (string)_.UsageEnterBetFairUsernane;
		cin >> username;
	}
	Logger::password = username;

	string password = args["password"].as<string>("");
	while (password.empty())
	{
		cout << (string)_.UsageEnterBetFairPassword;
		setStdinEcho(false);
		cin >> password;
		setStdinEcho(true);
		cout << endl;
	}

	string t_chat = args["telegram_chat"].as<string>("");
	if (!t_chat.empty())
	{
		string t_key = args["telegram_key"].as<string>("");
		Logger::telegramChat = t_chat;
		Logger::telegramKey = t_key;
	}
	else if (args["telegram_key"])
	{
		logger->warn(_.UsageTelegramNoChat);
	}

	Server server;
	unsigned int port = args["port"].as<unsigned int>(0);
	if (port > 0)
	{
		server.start(port);
	}

	signal(SIGINT, signal_handler);
	logger = Logger::logger(string(_.LoggerApp).data());
	logger->set_level(spdlog::level::info);
	logger->info(_.Welcome, VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	logger->info("{}", OPENSSL_VERSION_TEXT);
	if (!t_chat.empty() && !args["telegram_key"])
	{
		logger->warn(_.TelegramWarning);
	}

	try
	{
		if (BetFair::account()->login(username, password))
		{
			Channel *s = Channel::create(ST_BJT_LF);
			s->status(RUNNING);
			Event e;
			bool is_running = true;
			while (is_running)
			{
				e = Events::loop()->pop();
				ChannelType t = UNKNOWN;
				switch (e.type)
				{
				case E_EXIT:
					logger->info(_.Stopping);
					is_running = false;
					break;
				case E_PAUSE:
					t = (ChannelType)e.data;
					Channel::status(t, PAUSED);
					break;
				case E_RESUME:
					t = (ChannelType)e.data;
					Channel::status(t, RUNNING);
					break;
				case E_NONE:
					break;
				}
			}
			Channel::status(UNKNOWN, STOPPED);
			Channel::finish(UNKNOWN);
		}
		else
		{
			logger->error(_.LoginFailed);
		}
	}
	catch (const std::bad_alloc &c)
	{
		logger->error(c.what());
		exit_code = EXIT_FAILURE;
	}
	catch (char *e)
	{
		logger->error(e);
		exit_code = EXIT_FAILURE;
	}
	catch (...)
	{
		exit_code = EXIT_FAILURE;
	}
	server.stop();
	logger->info(_.Bye);
	delete BetFair::account();

	return exit_code;
}
