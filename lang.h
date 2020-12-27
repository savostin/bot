#ifndef __LANG_H__
#define __LANG_H__

#include "const.h"

class L
{
private:
    std::map<Lang, std::string> v;

public:
    L(const std::string &_en = "???", const std::string &_ru = "???");
    operator std::string() const;
    operator spdlog::string_view_t() const;
};

template <>
struct fmt::formatter<L> : formatter<string_view>
{
    template <typename FormatContext>
    auto format(L c, FormatContext &ctx)
    {
        return formatter<string_view>::format((string)c, ctx);
    }
};

struct Language
{
    static Lang current;
    static Language l;
    L LoggerApp = {
        "APP",
        "БОТ"};
    L LoggerServer = {
        "SERVER",
        "СЕРВЕР"};
    L LoggerAccount = {
        "ACCOUNT",
        "СЧЁТ"};
    L LoggerBfBj = {
        "📍",
        "📍"};
    L Usage = {
        "Usage: {} [options]",
        "Запуск: {} [параметры]"};
    L UsageVersion = {
        "Print version and exit",
        "Вывести номер версии и выйти"};
    L UsageLanguage = {
        "Language / Язык [ en|ru ]",
        "Язык / Language [ en|ru ]"};
    L UsageWrongLang = {
        "The only en or ru is currently supported",
        "Поддерживаются только en или ru"};
    L UsageHelp = {
        "Shows this help message",
        "Показать эту помощь"};
    L UsageLogs = {
        "Location of logs dir [./logs/]",
        "Расположение директории для логов [./logs/]"};
    L UsageKeep = {
        "Keep logs for X hours [24]",
        "Хранить логи X часов [24]"};
    L UsageBetFairUsername = {
        "BetFair username",
        "Имя пользователя BetFair"};
    L UsageBetFairPassword = {
        "BetFair password",
        "Пароль аккаунта BetFair"};
    L UsageEncrypt = {
        "Encryption password",
        "Пароль шифрования"};
    L UsagePort = {
        "Web-interface port",
        "Порт web-интерфейса"};
    L UsageProxyServer = {
        "Proxy server",
        "Прокси сервер"};
    L UsageProxyPort = {
        "Proxy port",
        "Порт прокси"};
    L UsageProxyUsername = {
        "Proxy username",
        "Имя пользователя прокси"};
    L UsageProxyPassword = {
        "Proxy password",
        "Пароль прокси"};
    L UsageTelegramChat = {
        "Telegram chat id to notify to",
        "ID чата Telegram для оповещений"};
    L UsageTelegramNoChat = {
        "Telegram chat id is not supplied!",
        "ID чата Telegram для оповещений не указан!"};
    L UsageTelegramKey = {
        "Telegram bot key to notify from",
        "Секретный ключ бота Telegram, от которого слать оповещения"};
    L UsageEnterCrypt = {
        "🔐 Enter data encryption password: ",
        "🔐 Введите пароль шифрования данных: "};
    L UsageEnterBetFairUsernane = {
        "👤 Enter BetFair username: ",
        "👤 Введите имя пользователя BetFair: "};
    L UsageEnterBetFairPassword = {
        "Enter BetFair password: 🔑",
        "Введите пароль аккаунта BetFair: 🔑"};
    L Welcome = {
        "✨ Welcome to {} v.{}.{}.{}!",
        "✨ Добро пожаловать в {} v.{}.{}.{}!"};
    L TelegramWarning = {
        "Telegram key is empty! Your Telegram messages will be sent through a proxy.",
        "Секретный ключ Telegram не указан! Ваши сообщения будут отправляться через прокси."};
    L Stopping = {
        "Stopping...",
        "Останавливаем..."};
    L LoginFailed = {
        "Login failed",
        "Вход неудачный"};
    L Bye = {
        "Bye! 👋",
        "До свидания! 👋"};
    L BACK = {
        "BACK",
        "ЗА"};
    L LAY = {
        "LAY",
        "ПРОТИВ"};
    L StrategyStarting = {
        "Starting strategy: {} v.{}.{}",
        "Запускается стратегия: {} v.{}.{}"};
    L StrategyLastPL = {
        "🏆 Last game p/l: {:+.2f}",
        "🏆 П/У последней игры: {:+.2f}"};
    L StrategyFavorite = {
        "Favorite: {}",
        "Фаворит: {}"};
    L StrategyPrice = {
        "Price: {:.2f}",
        "Коэффициент: {:.2f}"};
    L StrategyCards = {
        "Cards: {} {}",
        "Карты: {} {}"};
    L StrategyPoints = {
        "Points: {:d}",
        "Очки: {:d}"};
    L StrategyBet = {
        "{:d}: LAY {} - {:.2f} {} @ {:.2f}",
        "{:d}: ПРОТИВ {} - {:.2f} {} @ {:.2f}"};
    L StrategySkipped = {
        "Game {:d} skipped",
        "Игра {:d} пропущена"};
    L StrategyChecking = {
        "------------------ Checking game {:d}",
        "------------------ Проверяем игру {:d}"};
    L ChannelNoSelection = {
        "No selections!",
        "Нет исходов!"};
    L ChannelNoMarket = {
        "No market",
        "Нет рынка"};
    L ChannelNoXml = {
        "No xml game data",
        "Нет xml данных"};
    L ChannelNoXmlRoot = {
        "No xml root",
        "Нет корня xml"};
    L ChannelStatusRunning = {
        "▶️ RUNNING",
        "▶️ ЗАПУЩЕН"};
    L ChannelStatusPaused = {
        "⏸️ PAUSED",
        "⏸️ ПРИОСТАНОВЛЕН"};
    L ChannelStatusStopped = {
        "⏹️ STOPPED",
        "⏹️ ОСТАНОВЛЕН"};
    L AccountLogin = {
        "Logging in as 👤{}",
        "Авторизуемся как 👤{}"};
    L AccountFunds = {
        "💰 Funds: {:.2f} {}",
        "💰 Доступно: {:.2f} {}"};
    L AccountNoSnapshot = {
        "No account snapshot",
        "Нет информации об аккаунте"};
    L AccountNoAuth = {
        "Wrong username/password?",
        "Неверный логин/пароль?"};
    L AccountPlacingBet = {
        "🔆 Placing {} bet on '{}': {:.2f} {} @ {:.2f}...",
        "🔆 Отправляем ставку {} на '{}': {:.2f} {} @ {:.2f}..."};
    L AccountBetResult = {
        "🔵 Bet result: {}, id {}",
        "🔵 Результат ставки: {}, id {}"};
    L HttpStatusError = {
        "{}: Status: {} Error: {}",
        "{}: Статус: {} Ошибка: {}"};
    L HttpXmlError = {
        "XML: {}",
        "XML: {}"};
    L HttpJsonError = {
        "JSON parse error: {}",
        "Ошибка парсинга JSON: {}"};
    L HttpErrorUnknown = {
        "Unknown error",
        "Неизвестная ошибка"};
    L HttpErrorConnect = {
        "Unable to connect",
        "Невозможно подключиться"};
    L HttpErrorBind = {
        "Unable to bind ip address",
        "Невозможно открыть порт на этом ip"};
    L HttpErrorSocketRead = {
        "Socket read error",
        "Ошибка чтения сокета"};
    L HttpErrorSocketWrite = {
        "Socket write error",
        "Ошибка записи в сокет"};
    L HttpErrorRedirect = {
        "Exceed Redirect Count",
        "Превышен лимит перенаправлений"};
    L HttpErrorCancelled = {
        "Request cancelled",
        "Запрос отменен"};
    L HttpErrorSSLFailed = {
        "SSL connection failed",
        "Ошибка соединения SSL"};
    L HttpErrorSSLLoad = {
        "SSL loading certs failed",
        "Ошибка чтения сертификата SSL"};
    L HttpErrorSSLVerification = {
        "SSL server verification failed",
        "Ошибка проверки подлинности SSL"};
    L HttpErrorMultipart = {
        "Unsupported multipart boundary chars",
        "Неподдерживаемые символы"};
    L HttpErrorCompression = {
        "Compression failed",
        "Ошибка распаковки"};
    L ServerStopping = {
        "Stopping...",
        "Останавливаем..."};
    L ServerStarting = {
        "🌎 Starting web-server on http://localhost:{:d}/",
        "🌎 Запускается web-сервер на http://localhost:{:d}/"};
    L ServerStartingError = {
        "Unable to bind http server on port {:d} - port is busy?",
        "Невозможно открыть порт {:d} для web-сервера - порт занят?"};
};

#endif // __LANG_H__