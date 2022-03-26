#include "const.h"
#include "channel.h"
#include "server.h"
#include <csignal>
#include "argagg.hpp"
#include "events.h"
#include "db.h"
#include "crypt.h"
#include "options.h"

void signal_handler(int)
{
	Events::loop()->notify(E_EXIT);
}

int main(int argc, char **argv)
{
	int exit_code = EXIT_SUCCESS;
	argagg::parser argparser;
	argagg::parser_results args;
	short i = 0;
	while (i++ < 2) // WTF?! Not more than 2 times?
	{
		argparser = {{
			{"lang", {"--lang"}, _.UsageLanguage, 1},
			{"password", {"-p", "--password"}, _.UsageEncrypt, 1},
			{"file", {"-f", "--file"}, _.UsageFile, 1},
			{"wizard", {"-w", "--wizard"}, _.UsageWizard, 0},
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

		string lang = args["lang"].as<string>("en");
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
	bool wizard = args["wizard"] ? true : false;
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

	string file = args["file"].as<string>("");
	do
	{
		if ((file.empty()))
		{
			file = Options::ask(_.UsageEnterFile);
		}
		if (!DB::init(file))
		{
			cerr << (string)_.UsageFileError << endl;
			file = "";
		}
	} while (file.empty());

	string password = args["password"].as<string>("");
	int tried = 0;
	do
	{
		if (password.empty())
		{
			password = Options::ask(_.UsageEnterCrypt, true);
		}
		if (!DB::o().set_key(password))
		{
			cerr << (string)_.UsageWrongPassword << endl;
			password = "";
			tried++;
			this_thread::sleep_for(chrono::seconds(5 ^ tried));
		}
	} while (password.empty() && tried < 3);

	if (password.empty())
	{
		return EXIT_FAILURE;
	}

	Crypt::init(password);

	Options o;
	if (o.get("proxy_port", 0) > 0 && !o.get("proxy_server", "").empty())
	{
		HTTP::setProxy(o.get("proxy_server", ""), o.get("proxy_port", 0), o.get("proxy_username", ""), o.get("proxy_password", ""));
	}

	string t_chat = o.get("telegram_chat", "");
	if (!t_chat.empty())
	{
		string t_key = o.get("telegram_key", "");
		Logger::telegramChat = t_chat;
		Logger::telegramKey = t_key;
	}

	while (o.get("username", "").empty())
	{
		o.set("username", Options::ask(_.UsageEnterBetFairUsernane));
	}

	string pass = o.get("password", "");
	if (pass.empty())
	{
		while (pass.empty())
		{
			pass = Options::ask(_.UsageEnterBetFairPassword, true);
		}

		string s = Options::ask(_.UsageSaveBetFairPassword);
		if (s == "Y" || s == "y" || s == "Д" || s == "д" || s == "")
		{
			o.set("password", pass);
		}
	}

	o.save();
	Logger::init(o.get("keep", 24));
	logger_p logger = Logger::logger(string(_.LoggerApp).data());
	logger->set_level(spdlog::level::info);
	logger->info(_.Welcome, VERSION_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
	logger->info("{}", OPENSSL_VERSION_TEXT);
	if (t_chat.empty() && args["telegram_key"])
	{
		logger->warn(_.UsageTelegramNoChat);
	}
	if (!t_chat.empty() && !args["telegram_key"])
	{
		logger->warn(_.TelegramWarning);
	}

	Server server;

	try
	{
		if (BetFair::account()->login(o.get("username", ""), pass))
		{
			signal(SIGINT, signal_handler);
			unsigned int port = o.get("port", 0);
			if (port > 0)
			{
				server.start(port);
			}
			Channel *s = Channel::create(ST_BJT_LF);
			//s->status(RUNNING);
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
