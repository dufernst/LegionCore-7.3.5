/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef LOG_H
#define LOG_H

#include "Define.h"
#include "LogCommon.h"
#include "Appender.h"
#include "Logger.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <unordered_map>
#include <string>
#include <stdarg.h>
#include <safe_ptr.h>
#include "StringFormat.h"
#include "Common.h"

typedef std::unordered_map<uint8, Logger> LoggerMap;
typedef std::vector<Logger*> LoggerList;

class Log
{
    Log();
    ~Log();

public:
    Log(Log const&) = delete;
    Log(Log&&) = delete;
    Log& operator=(Log const&) = delete;
    Log& operator=(Log&&) = delete;

    static Log* instance(boost::asio::io_service* ioService = nullptr);

    void LoadFromConfig();
    void Close();
    bool ShouldLog(LogFilterType type, LogLevel level) const;
    bool SetLogLevel(std::string const& name, char const* newLevelc, bool isLogger = true);

    void outMessage(LogFilterType filter, LogLevel level, std::string&& message)
    {
        std::unique_ptr<LogMessage> msg(new LogMessage(level, filter, std::move(message)));
        write(std::move(msg));
    }

    template<typename Format, typename... Args>
    void outMessage(LogFilterType filter, LogLevel level, Format&& fmt, Args&&... args)
    {
        outMessage(filter, level, Trinity::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...));
    }

    void outArena(uint8 jointype, const char * str, ...); // ATTR_PRINTF(3, 4);
    void OutPveEncounter(char const* str, ...);
    void outSpamm(const char * str, ...)               ATTR_PRINTF(2, 3);
    void outDiff(const char * str, ...)                ATTR_PRINTF(2, 3);
    void outWarden(const char * str, ...)               ATTR_PRINTF(2, 3);
    void outCommand(uint32 account, const char * str, ...) ATTR_PRINTF(3, 4);
    void outCharDump(char const* str, uint32 accountId, uint64 guid, char const* name);
    void outMapInfo(const char * str, ...)                ATTR_PRINTF(2, 3);
    void outFreeze(const char * str, ...)                ATTR_PRINTF(2, 3);
    void outAnticheat(const char * str, ...)                ATTR_PRINTF(2, 3);
    void outArenaSeason(const char * str, ...)                ATTR_PRINTF(2, 3);
    void outTryCatch(const char * str, ...)                ATTR_PRINTF(2, 3);

    void EnableDBAppenders();
    static std::string GetTimestampStr();

    void SetRealmID(uint32 id);
    static void outTimestamp(FILE* file);
    uint32 GetRealmID() const { return realm; }


    std::atomic<bool> _checkLock{};

private:
    void vlog(LogFilterType f, LogLevel level, char const* str, va_list argptr);
    void write(std::unique_ptr<LogMessage>&& msg);

    Logger* GetLoggerByType(LogFilterType filter);
    Appender* GetAppenderByName(std::string const& name);
    uint8 NextAppenderId();
    void CreateAppenderFromConfig(const char* name);
    void CreateLoggerFromConfig(const char* name);
    void ReadAppendersFromConfig();
    void ReadLoggersFromConfig();

    std::unordered_map<uint8, std::unique_ptr<Appender>> appenders;
    LoggerMap loggers;
    LoggerList loggerList;
    uint8 AppenderId{};
    LogLevel lowestLogLevel = LOG_LEVEL_DISABLED;
    FILE* openLogFile(char const* configFileName, char const* configTimeStampFlag, char const* mode);
    FILE* arenaLogFile2v2;
    FILE* arenaLogFile3v3;
    FILE* spammLogFile;
    FILE* diffLogFile;
    FILE* wardenLogFile;
    FILE* mapInfoFile;
    FILE* _pveEncounterLogFile{};
    FILE* freezeFile;
    FILE* acLogFile;
    FILE* arenaSeasonLogFile;
    FILE* tryCatchLogFile;

    std::string m_logsDir;
    std::string m_logsTimestamp;

    uint32 realm{};
    boost::asio::io_service* _ioService;
    boost::asio::strand* _strand;
};

#define sLog Log::instance()

#define TC_LOG_MESSAGE_BODY(filterType__, level__, ...) \
    if (sLog->ShouldLog(filterType__, level__)) \
        sLog->outMessage(filterType__, level__, __VA_ARGS__);

#define TC_LOG_TRACE(filterType__, ...)                     \
    do {                                                    \
        TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_TRACE,  __VA_ARGS__) \
    } while(0)

#define TC_LOG_DEBUG(filterType__, ...) \
    do {                                                    \
        TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_DEBUG,  __VA_ARGS__) \
    } while(0)

#define TC_LOG_INFO(filterType__, ...)  \
    do {                                                    \
        TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_INFO,  __VA_ARGS__) \
    } while(0)

#define TC_LOG_WARN(filterType__, ...)  \
    do {                                                    \
        TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_WARN,  __VA_ARGS__) \
    } while(0)

#define TC_LOG_ERROR(filterType__, ...) \
    do {                                                    \
        TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_ERROR,  __VA_ARGS__) \
    } while(0)

#define TC_LOG_FATAL(filterType__, ...) \
    do {                                                    \
        TC_LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_FATAL,  __VA_ARGS__) \
    } while(0)

#endif
