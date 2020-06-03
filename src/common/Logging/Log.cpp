/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Log.h"
#include "Common.h"
#include "Config.h"
#include "Util.h"
#include "AppenderConsole.h"
#include "AppenderFile.h"
#include "AppenderDB.h"
#include "LogOperation.h"
#include "Logger.h"
#include "Duration.h"

#include <cstdarg>
#include <sstream>
#include "StringFormat.h"

Log::Log() : _ioService(nullptr), _strand(nullptr)
{
    arenaLogFile2v2 = nullptr;
    arenaLogFile3v3 = nullptr;
    diffLogFile = nullptr;
    spammLogFile = nullptr;
    wardenLogFile = nullptr;
    mapInfoFile = nullptr;
    freezeFile = nullptr;
    acLogFile = nullptr;
    arenaSeasonLogFile = nullptr;
    tryCatchLogFile = nullptr;
    SetRealmID(0);
    m_logsTimestamp = "_" + GetTimestampStr();
    loggerList.resize(LOG_FILTER_MAX);
    LoadFromConfig();
    _checkLock = false;
}

Log::~Log()
{
    delete _strand;
    Close();
}

Log* Log::instance(boost::asio::io_service* ioService)
{
    static Log instance;

    if (ioService != nullptr)
    {
        instance._ioService = ioService;
        instance._strand = new boost::asio::strand(*ioService);
    }

    return &instance;
}

uint8 Log::NextAppenderId()
{
    return AppenderId++;
}

int32 GetConfigIntDefault(std::string base, const char* name, int32 value)
{
    base.append(name);
    return sConfigMgr->GetIntDefault(base, value);
}

std::string GetConfigStringDefault(std::string base, const char* name, const char* value)
{
    base.append(name);
    return sConfigMgr->GetStringDefault(base, value);
}

// Returns default logger if the requested logger is not found
Logger* Log::GetLoggerByType(LogFilterType filter)
{
    if (loggerList.size() <= filter)
        return nullptr;
    return loggerList[filter];
}

Appender* Log::GetAppenderByName(std::string const& name)
{
    auto it = appenders.begin();
    while (it != appenders.end() && it->second && it->second->getName() != name)
        ++it;

    return it == appenders.end() ? nullptr : it->second.get();
}

void Log::CreateAppenderFromConfig(const char* name)
{
    if (!name || *name == '\0')
        return;

    // Format=type,level,flags,optional1,optional2
    // if type = File. optional1 = file and option2 = mode
    // if type = Console. optional1 = Color
    std::string options = "Appender.";
    options.append(name);
    options = sConfigMgr->GetStringDefault(options, "");
    Tokenizer tokens(options, ',');
    auto iter = tokens.begin();

    if (tokens.size() < 2)
    {
        fprintf(stderr, "Log::CreateAppenderFromConfig: Wrong configuration for appender %s. Config line: %s\n", name, options.c_str());
        return;
    }

    AppenderFlags flags = APPENDER_FLAGS_NONE;
    auto type = AppenderType(atoi(*iter));
    ++iter;
    auto level = LogLevel(atoi(*iter));
    if (level > LOG_LEVEL_FATAL)
    {
        fprintf(stderr, "Log::CreateAppenderFromConfig: Wrong Log Level %u for appender %s\n", level, name);
        return;
    }

    if (++iter != tokens.end())
        flags = AppenderFlags(atoi(*iter));

    switch (type)
    {
        case APPENDER_CONSOLE:
        {
            auto appender = new AppenderConsole(NextAppenderId(), name, level, flags);
            appenders[appender->getId()].reset(appender);
            if (++iter != tokens.end())
                appender->InitColors(*iter);
            break;
        }
        case APPENDER_FILE:
        {
            std::string mode = "a";

            if (++iter == tokens.end())
            {
                fprintf(stderr, "Log::CreateAppenderFromConfig: Missing file name for appender %s\n", name);
                return;
            }

            std::string filename = *iter;

            if (++iter != tokens.end())
                mode = *iter;

            if (flags & APPENDER_FLAGS_USE_TIMESTAMP)
            {
                auto dot_pos = filename.find_last_of('.');
                if (dot_pos != std::string::npos)
                    filename.insert(dot_pos, m_logsTimestamp);
                else
                    filename += m_logsTimestamp;
            }

            auto id = NextAppenderId();
            appenders[id].reset(new AppenderFile(id, name, level, filename.c_str(), m_logsDir.c_str(), mode.c_str(), flags));
            break;
        }
        case APPENDER_DB:
        {
            auto id = NextAppenderId();
            appenders[id].reset(new AppenderDB(id, name, level, realm));
            break;
        }
        default:
            fprintf(stderr, "Log::CreateAppenderFromConfig: Unknown type %u for appender %s\n", type, name);
            break;
    }
}

void Log::CreateLoggerFromConfig(const char* name)
{
    if (!name || *name == '\0')
        return;

    std::string options = "Logger.";
    options.append(name);
    options = sConfigMgr->GetStringDefault(options, "");
    if (options.empty())
    {
        fprintf(stderr, "Log::CreateLoggerFromConfig: Missing config option Logger.%s\n", name);
        return;
    }

    Tokenizer tokens(options, ',');
    auto iter = tokens.begin();

    if (tokens.size() != 3)
    {
        fprintf(stderr, "Log::CreateLoggerFromConfig: Wrong config option Logger.%s=%s\n", name, options.c_str());
        return;
    }

    uint32 type = atoi(*iter);
    if (type > LOG_FILTER_MAX)
    {
        fprintf(stderr, "Log::CreateLoggerFromConfig: Wrong type %u for logger %s\n", type, name);
        return;
    }

    if (loggerList.size() <= type)
        return;

    Logger& logger = loggers[type];
    if (!logger.getName().empty())
    {
        fprintf(stderr, "Error while configuring Logger %s. Already defined\n", name);
        return;
    }

    ++iter;

    auto level = LogLevel(atoi(*iter));
    if (level > LOG_LEVEL_FATAL)
    {
        fprintf(stderr, "Log::CreateLoggerFromConfig: Wrong Log Level %u for logger %s\n", type, name);
        return;
    }

    if (level < lowestLogLevel)
        lowestLogLevel = level;

    loggerList[type] = &logger;
    logger.Create(name, LogFilterType(type), level);

    ++iter;
    std::istringstream ss(*iter);
    std::string str;

    ss >> str;
    while (ss)
    {
        if (auto appender = GetAppenderByName(str))
            logger.addAppender(appender->getId(), appender);
        else
            fprintf(stderr, "Error while configuring Appender %s in Logger %s. Appender does not exist", str.c_str(), name);
        ss >> str;
    }
}

void Log::ReadAppendersFromConfig()
{
    std::istringstream ss(sConfigMgr->GetStringDefault("Appenders", ""));
    std::string name;

    do
    {
        ss >> name;
        CreateAppenderFromConfig(name.c_str());
        name = "";
    } while (ss);
}

void Log::ReadLoggersFromConfig()
{
    std::istringstream ss(sConfigMgr->GetStringDefault("Loggers", ""));
    std::string name;

    do
    {
        ss >> name;
        CreateLoggerFromConfig(name.c_str());
        name = "";
    } while (ss);

    auto it = loggers.cbegin();
    while (it != loggers.end() && it->first)
        ++it;

    // root logger must exist. Marking as disabled as its not configured
    if (it == loggers.end())
        loggers[0].Create("root", LOG_FILTER_GENERAL, LOG_LEVEL_DISABLED);
}

void Log::EnableDBAppenders()
{
    for (auto& appender : appenders)
        if (appender.second && appender.second->getType() == APPENDER_DB)
            static_cast<AppenderDB*>(appender.second.get())->setEnable(true);
}

void Log::vlog(LogFilterType filter, LogLevel level, char const* str, va_list argptr)
{
    char text[MAX_QUERY_LEN];
    vsnprintf(text, MAX_QUERY_LEN, str, argptr);

    std::unique_ptr<LogMessage> msg(new LogMessage(level, filter, text));
    write(std::move(msg));
}

void Log::write(std::unique_ptr<LogMessage>&& msg)
{
    auto logger = GetLoggerByType(msg->type);
    msg->text.append("\n");

    if (_ioService)
    {
        auto logOperation = std::make_shared<LogOperation>(logger, std::move(msg));
        _ioService->post(_strand->wrap([logOperation]() { logOperation->call(); }));
    }
    else
        logger->write(msg.get());

    // initialize stderr and stdout, without this launcher wont have realtime output
    std::cout << "";
    std::cerr << "";
}

std::string Log::GetTimestampStr()
{
    auto tt = SystemClock::to_time_t(SystemClock::now());

    std::tm aTm{};
    localtime_r(&tt, &aTm);

    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)

    try
    {
        return Trinity::StringFormat("%04d-%02d-%02d", aTm.tm_year + 1900, aTm.tm_mon + 1, aTm.tm_mday);
    }
    catch (std::exception const& ex)
    {
        fprintf(stderr, "Failed to initialize timestamp part of log filename! %s", ex.what());
        fflush(stderr);
        ABORT();
    }
}

bool Log::SetLogLevel(std::string const& name, const char* newLevelc, bool isLogger /* = true */)
{
    auto newLevel = LogLevel(atoi(newLevelc));
    if (newLevel < 0)
        return false;

    if (isLogger)
    {
        auto it = loggers.begin();
        while (it != loggers.end() && it->second.getName() != name)
            ++it;

        if (it == loggers.end())
            return false;

        if (newLevel != LOG_LEVEL_DISABLED && newLevel < lowestLogLevel)
            lowestLogLevel = newLevel;

        it->second.setLogLevel(newLevel);
    }
    else
    {
        auto appender = GetAppenderByName(name);
        if (!appender)
            return false;

        appender->setLogLevel(newLevel);
    }
    return true;
}

bool Log::ShouldLog(LogFilterType type, LogLevel level) const
{
    // Don't even look for a logger if the LogLevel is lower than lowest log levels across all loggers
    if (level < lowestLogLevel)
        return false;

    if (_checkLock)
        return false;

    if (loggerList.size() <= type)
        return false;

    auto logger = loggerList[type];
    if (!logger)
        return false;

    auto loggerLevel = logger->getLogLevel();
    return loggerLevel && loggerLevel <= level;
}
void Log::outCharDump(char const* str, uint32 accountId, uint64 guid, char const* name)
{
    if (!str || !ShouldLog(LOG_FILTER_PLAYER_DUMP, LOG_LEVEL_INFO))
        return;

    std::ostringstream ss;
    ss << "== START DUMP == (account: " << accountId << " guid: " << guid << " name: " << name
        << ")\n" << str << "\n== END DUMP ==\n";

    std::unique_ptr<LogMessage> msg(new LogMessage(LOG_LEVEL_INFO, LOG_FILTER_PLAYER_DUMP, ss.str()));
    std::ostringstream param;
    param << guid << '_' << name;

    msg->param1 = param.str();

    write(std::move(msg));
}

void Log::outCommand(uint32 account, const char * str, ...)
{
    if (!str || !ShouldLog(LOG_FILTER_GMCOMMAND, LOG_LEVEL_INFO))
        return;

    va_list ap;
    va_start(ap, str);
    char text[MAX_QUERY_LEN];
    vsnprintf(text, MAX_QUERY_LEN, str, ap);
    va_end(ap);

    std::unique_ptr<LogMessage> msg(new LogMessage(LOG_LEVEL_INFO, LOG_FILTER_GMCOMMAND, text));

    std::ostringstream ss;
    ss << account;
    msg->param1 = ss.str();

    write(std::move(msg));
}

void Log::SetRealmID(uint32 id)
{
    realm = id;
}

void Log::Close()
{
    if (arenaLogFile2v2 != nullptr)
        fclose(arenaLogFile2v2);
    arenaLogFile2v2 = nullptr;
    if (arenaLogFile3v3 != nullptr)
        fclose(arenaLogFile3v3);
    arenaLogFile3v3 = nullptr;
    if (spammLogFile != nullptr)
        fclose(spammLogFile);
    spammLogFile = nullptr;
    if (diffLogFile != nullptr)
        fclose(diffLogFile);
    diffLogFile = nullptr;
    if (wardenLogFile != nullptr)
        fclose(wardenLogFile);
    wardenLogFile = nullptr;

    if (mapInfoFile != nullptr)
        fclose(mapInfoFile);
    mapInfoFile = nullptr;

    if (freezeFile != nullptr)
        fclose(freezeFile);
    freezeFile = nullptr;

    if (acLogFile != nullptr)
        fclose(acLogFile);
    acLogFile = nullptr;

    if (arenaSeasonLogFile != nullptr)
        fclose(arenaSeasonLogFile);
    arenaSeasonLogFile = nullptr;

    if (tryCatchLogFile != nullptr)
        fclose(tryCatchLogFile);
    tryCatchLogFile = nullptr;

    loggers.clear();
    appenders.clear();
}

void Log::LoadFromConfig()
{
    Close();
    AppenderId = 0;
    lowestLogLevel = LOG_LEVEL_FATAL;

    m_logsDir = sConfigMgr->GetStringDefault("LogsDir", "");
    if (!m_logsDir.empty() && (m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
        m_logsDir.push_back('/');

    ReadAppendersFromConfig();
    ReadLoggersFromConfig();

    arenaLogFile2v2 = openLogFile("ArenaLogFile2v2", nullptr, "a");
    arenaLogFile3v3 = openLogFile("ArenaLogFile3v3", nullptr, "a");
    spammLogFile = openLogFile("SpammLogFile", nullptr, "a");
    diffLogFile = openLogFile("diffLogFile", nullptr, "a");
    wardenLogFile = openLogFile("Warden.LogFile", nullptr, "a");
    mapInfoFile = openLogFile("MapInfo.LogFile", nullptr, "a");
    _pveEncounterLogFile = openLogFile("PveEncounters", nullptr, "a");
    freezeFile = openLogFile("freezeFile", nullptr, "a");
    acLogFile = openLogFile("AnticheatLogFile", nullptr, "a");
    arenaSeasonLogFile = openLogFile("ArenaSeasonLogFile", nullptr, "a");
    tryCatchLogFile = openLogFile("TryCatchLogFile", nullptr, "a");
}

void Log::outArena(uint8 jointype, const char * str, ...)
{
    if (!str)
        return;

    if (jointype != 2 && jointype != 3) // 2 -2v2, 3 - 3v3, 0 - both
        jointype = 3;
    else
        jointype -= 1;

    // on the output we have flags -> jointype & 1 - 2v2, jointype & 2 - 3v3

    if (jointype & 1 && arenaLogFile2v2)
    {
        va_list ap;
        outTimestamp(arenaLogFile2v2);
        va_start(ap, str);
        vfprintf(arenaLogFile2v2, str, ap);
        fprintf(arenaLogFile2v2, "\n");
        va_end(ap);
        fflush(arenaLogFile2v2);
    }

    if (jointype & 2 && arenaLogFile3v3)
    {
        va_list ap;
        outTimestamp(arenaLogFile3v3);
        va_start(ap, str);
        vfprintf(arenaLogFile3v3, str, ap);
        fprintf(arenaLogFile3v3, "\n");
        va_end(ap);
        fflush(arenaLogFile3v3);
    }
}

void Log::OutPveEncounter(char const* str, ...)
{
    if (!str || !_pveEncounterLogFile)
        return;

    va_list ap;
    outTimestamp(_pveEncounterLogFile);
    va_start(ap, str);
    vfprintf(_pveEncounterLogFile, str, ap);
    fprintf(_pveEncounterLogFile, "\n");
    va_end(ap);
    fflush(_pveEncounterLogFile);
}

void Log::outTimestamp(FILE* file)
{
    auto t = time(nullptr);
    auto aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    fprintf(file, "%-4d-%02d-%02d %02d:%02d:%02d ", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
}

FILE* Log::openLogFile(char const* configFileName, char const* configTimeStampFlag, char const* mode)
{
    auto logfn = sConfigMgr->GetStringDefault(configFileName, "");
    if (logfn.empty())
        return nullptr;

    if (configTimeStampFlag && sConfigMgr->GetBoolDefault(configTimeStampFlag, false))
    {
        auto dot_pos = logfn.find_last_of('.');
        if (dot_pos != std::string::npos)
            logfn.insert(dot_pos, m_logsTimestamp);
        else
            logfn += m_logsTimestamp;
    }

    return fopen((m_logsDir + logfn).c_str(), mode);
}

void Log::outSpamm(const char * str, ...)
{
    if (!str)
        return;

    if (spammLogFile)
    {
        va_list ap;
        outTimestamp(spammLogFile);
        va_start(ap, str);
        vfprintf(spammLogFile, str, ap);
        fprintf(spammLogFile, "\n");
        va_end(ap);
        fflush(spammLogFile);
    }
}

void Log::outDiff(const char * str, ...)
{
    if (!str)
        return;

    if (diffLogFile)
    {
        va_list ap;
        outTimestamp(diffLogFile);
        va_start(ap, str);
        vfprintf(diffLogFile, str, ap);
        fprintf(diffLogFile, "\n");
        va_end(ap);
        fflush(diffLogFile);
    }
}

void Log::outFreeze(const char * str, ...)
{
    if (!str)
        return;

    if (freezeFile)
    {
        va_list ap;
        outTimestamp(freezeFile);
        va_start(ap, str);
        vfprintf(freezeFile, str, ap);
        fprintf(freezeFile, "\n");
        va_end(ap);
        fflush(freezeFile);
    }
}

void Log::outAnticheat(const char * str, ...)
{
    if (!str)
        return;

    if (acLogFile)
    {
        va_list ap;
        outTimestamp(acLogFile);
        va_start(ap, str);
        vfprintf(acLogFile, str, ap);
        fprintf(acLogFile, "\n");
        va_end(ap);
        fflush(acLogFile);
    }
}

void Log::outMapInfo(const char * str, ...)
{
    if (!str)
        return;

    if (mapInfoFile)
    {
        va_list ap;
        outTimestamp(mapInfoFile);
        va_start(ap, str);
        vfprintf(mapInfoFile, str, ap);
        fprintf(mapInfoFile, "\n");
        va_end(ap);
        fflush(mapInfoFile);
    }
}

void Log::outWarden(const char * str, ...)
{
    if (!str)
        return;

    if (wardenLogFile)
    {
        va_list ap;
        outTimestamp(wardenLogFile);
        va_start(ap, str);
        vfprintf(wardenLogFile, str, ap);
        fprintf(wardenLogFile, "\n");
        va_end(ap);
        fflush(wardenLogFile);
    }
}

void Log::outArenaSeason(const char * str, ...)
{
    if (!str)
        return;

    if (arenaSeasonLogFile)
    {
        va_list ap;
        outTimestamp(arenaSeasonLogFile);
        va_start(ap, str);
        vfprintf(arenaSeasonLogFile, str, ap);
        fprintf(arenaSeasonLogFile, "\n");
        va_end(ap);
        fflush(arenaSeasonLogFile);
    }
}

void Log::outTryCatch(const char * str, ...)
{
    if (!str)
        return;

    if (tryCatchLogFile)
    {
        va_list ap;
        outTimestamp(tryCatchLogFile);
        va_start(ap, str);
        vfprintf(tryCatchLogFile, str, ap);
        fprintf(tryCatchLogFile, "\n");
        va_end(ap);
        fflush(tryCatchLogFile);
    }
}
