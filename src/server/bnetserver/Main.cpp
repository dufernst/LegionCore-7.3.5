/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

/**
* @file main.cpp
* @brief Authentication Server main program
*
* This file contains the main program for the
* authentication server
*/

#include <boost/asio/io_service.hpp>
#include <boost/bind/bind.hpp>
#include <boost/program_options.hpp>
#include <cds/gc/hp.h>
#include <cds/init.h>
#include <google/protobuf/stubs/common.h>
#include <iostream>
#include <openssl/crypto.h>

#include "AppenderDB.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "GitRevision.h"
#include "LoginRESTService.h"
#include "ProcessPriority.h"
#include "RealmList.h"
#include "SessionManager.h"
#include "SslContext.h"
#include "ThreadPoolMgr.hpp"
#include "Util.h"
#include "Banner.h"

using boost::asio::ip::tcp;
using namespace boost::program_options;

#ifndef _TRINITY_BNET_CONFIG
# define _TRINITY_BNET_CONFIG  "bnetserver.conf"
#endif

#if PLATFORM == TC_PLATFORM_WINDOWS
#include "ServiceWin32.h"
char serviceName[] = "bnetserver";
char serviceLongName[] = "LegionCore bnet service";
char serviceDescription[] = "LegionCore Battle.net emulator authentication service";
/*
* -1 - not in service mode
*  0 - stopped
*  1 - running
*  2 - paused
*/
int m_ServiceStatus = -1;

static boost::asio::deadline_timer* _serviceStatusWatchTimer;
void ServiceStatusWatcher(boost::system::error_code const& error);
#endif

bool StartDB();
void StopDB();
void SignalHandler(boost::system::error_code const& error, int signalNumber);
void KeepDatabaseAliveHandler(boost::system::error_code const& error);
void BanExpiryHandler(boost::system::error_code const& error);
variables_map GetConsoleArguments(int argc, char** argv, std::string& configFile, std::string& configService);

boost::asio::io_service _ioService;
static boost::asio::deadline_timer* _dbPingTimer;
static uint32 _dbPingInterval;
static boost::asio::deadline_timer* _banExpiryCheckTimer;
static uint32 _banExpiryCheckInterval;
void ShutdownThreadPool(std::vector<std::thread>& threadPool);

int main(int argc, char** argv)
{
    signal(SIGABRT, &Trinity::AbortHandler);

    std::string configFile = _TRINITY_BNET_CONFIG;
    std::string configService;
    auto vm = GetConsoleArguments(argc, argv, configFile, configService);
    // exit if help or version is enabled
    if (vm.count("help") || vm.count("version"))
        return 0;

    GOOGLE_PROTOBUF_VERIFY_VERSION;

#if PLATFORM == TC_PLATFORM_WINDOWS
    if (configService.compare("install") == 0)
        return WinServiceInstall() ? 0 : 1;
    if (configService.compare("uninstall") == 0)
        return WinServiceUninstall() ? 0 : 1;
    if (configService.compare("run") == 0)
        return WinServiceRun() ? 0 : 1;
#endif

    std::string configError;
    if (!sConfigMgr->LoadInitial(configFile, configError))
    {
        printf("Error in config file: %s\n", configError.c_str());
        return 1;
    }

    Trinity::Banner::Show("bnetserver", [](char const* text)
    {
        TC_LOG_INFO(LOG_FILTER_BATTLENET, "%s", text);
    }, []()
    {
        TC_LOG_INFO(LOG_FILTER_BATTLENET, "Using configuration file %s.", sConfigMgr->GetFilename().c_str());
        TC_LOG_INFO(LOG_FILTER_BATTLENET, "Using SSL version: %s (library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
        TC_LOG_INFO(LOG_FILTER_BATTLENET, "Using Boost version: %i.%i.%i", BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
    }
    );

    cds::Initialize();
    cds::gc::HP hpGC;
    cds::threading::Manager::attachThread();

    // Seed the OpenSSL's PRNG here.
    // That way it won't auto-seed when calling BigNumber::SetRand and slow down the first world login
    BigNumber seed;
    seed.SetRand(16 * 8);

    // bnetserver PID file creation
    std::string pidFile = sConfigMgr->GetStringDefault("PidFile", "");
    if (!pidFile.empty())
    {
        if (uint32 pid = CreatePIDFile(pidFile))
            TC_LOG_INFO(LOG_FILTER_BATTLENET, "Daemon PID: %u\n", pid);
        else
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Cannot create PID file %s.\n", pidFile.c_str());
            return 1;
        }
    }

    if (!Battlenet::SslContext::Initialize())
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Failed to initialize SSL context");
        return 1;
    }

    boost::asio::signal_set signals(_ioService, SIGINT, SIGTERM);
#if PLATFORM == TC_PLATFORM_WINDOWS
    signals.add(SIGBREAK);
#endif
    signals.async_wait(SignalHandler);

    // Start the Boost based thread pool
    int numThreads = sConfigMgr->GetIntDefault("ThreadPool", 1);
    std::vector<std::thread> threadPool;

    if (numThreads < 1)
        numThreads = 1;

    for (int i = 0; i < numThreads; ++i)
        threadPool.emplace_back(boost::bind(&boost::asio::io_service::run, &_ioService));

    // Initialize the database connection
    if (!StartDB())
    {
        ShutdownThreadPool(threadPool);
        return 1;
    }

    // _ioService = new boost::asio::io_service();

    // Start the listening port (acceptor) for auth connections
    int32 bnport = sConfigMgr->GetIntDefault("BattlenetPort", 1119);
    if (bnport < 0 || bnport > 0xFFFF)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Specified battle.net port (%d) out of allowed range (1-65535)", bnport);
        StopDB();
        // delete _ioService;
        ShutdownThreadPool(threadPool);
        return 1;
    }

    if (!sLoginService.Start(_ioService))
    {
        StopDB();
        // delete _ioService;
        ShutdownThreadPool(threadPool);
        TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Failed to initialize login service");
        return 1;
    }

    // Get the list of realms for the server
    sRealmList->Initialize(_ioService, sConfigMgr->GetIntDefault("RealmsStateUpdateDelay", 10));

    std::string bindIp = sConfigMgr->GetStringDefault("BindIP", "0.0.0.0");

    int networkThreads = sConfigMgr->GetIntDefault("Network.Threads", 1);
    if (networkThreads <= 0)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Network.Threads must be greater than 0");
        return 0;
    }

    sSessionMgr.StartNetwork(_ioService, bindIp, bnport, networkThreads);
    // sSessionMgr.StartNetwork(*_ioService, bindIp, bnport);

    // Set process priority according to configuration settings
    SetProcessPriority("server.bnetserver");

    // Enabled a timed callback for handling the database keep alive ping
    _dbPingInterval = sConfigMgr->GetIntDefault("MaxPingTime", 30);
    _dbPingTimer = new boost::asio::deadline_timer(_ioService);
    _dbPingTimer->expires_from_now(boost::posix_time::minutes(_dbPingInterval));
    _dbPingTimer->async_wait(KeepDatabaseAliveHandler);

    _banExpiryCheckInterval = sConfigMgr->GetIntDefault("BanExpiryCheckInterval", 60);
    _banExpiryCheckTimer = new boost::asio::deadline_timer(_ioService);
    _banExpiryCheckTimer->expires_from_now(boost::posix_time::seconds(_banExpiryCheckInterval));
    _banExpiryCheckTimer->async_wait(BanExpiryHandler);

    TC_LOG_INFO(LOG_FILTER_BATTLENET, "%s (bnetserver-daemon) ready...", GitRevision::GetFullVersion());

#if PLATFORM == TC_PLATFORM_WINDOWS
    if (m_ServiceStatus != -1)
    {
        _serviceStatusWatchTimer = new boost::asio::deadline_timer(_ioService);
        _serviceStatusWatchTimer->expires_from_now(boost::posix_time::seconds(1));
        _serviceStatusWatchTimer->async_wait(ServiceStatusWatcher);
    }
#endif

    // Start the io service worker loop
    _ioService.run();

    ShutdownThreadPool(threadPool);

    _banExpiryCheckTimer->cancel();
    _dbPingTimer->cancel();

    sLoginService.Stop();

    sSessionMgr.StopNetwork();

    sRealmList->Close();

    // Close the Database Pool and library
    StopDB();

    cds::threading::Manager::detachThread();
    cds::Terminate();

    TC_LOG_INFO(LOG_FILTER_BATTLENET, "Halting process...");

    signals.cancel();

    delete _banExpiryCheckTimer;
    delete _dbPingTimer;
    // delete _ioService;
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}

/// Initialize connection to the database
bool StartDB()
{
    MySQL::Library_Init();

    std::string dbstring = sConfigMgr->GetStringDefault("LoginDatabaseInfo", "");
    if (dbstring.empty())
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Database not specified");
        return false;
    }

    int32 worker_threads = sConfigMgr->GetIntDefault("LoginDatabase.WorkerThreads", 1);
    if (worker_threads < 1 || worker_threads > 32)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Improper value specified for LoginDatabase.WorkerThreads, defaulting to 1.");
        worker_threads = 1;
    }

    int32 synch_threads = sConfigMgr->GetIntDefault("LoginDatabase.SynchThreads", 1);
    if (synch_threads < 1 || synch_threads > 32)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLENET, "Improper value specified for LoginDatabase.SynchThreads, defaulting to 1.");
        synch_threads = 1;
    }

    if (!LoginDatabase.Open(dbstring, uint8(worker_threads), uint8(synch_threads)))
        return false;

    TC_LOG_INFO(LOG_FILTER_BATTLENET, "Started auth database connection pool.");
    sLog->SetRealmID(0); // Enables DB appenders when realm is set.
    return true;
}

/// Close the connection to the database
void StopDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}

void SignalHandler(boost::system::error_code const& error, int /*signalNumber*/)
{
    if (!error)
        _ioService.stop();
}

void KeepDatabaseAliveHandler(boost::system::error_code const& error)
{
    if (!error)
    {
        TC_LOG_DEBUG(LOG_FILTER_BATTLENET, "Ping MySQL to keep connection alive");
        LoginDatabase.KeepAlive();

        _dbPingTimer->expires_from_now(boost::posix_time::minutes(_dbPingInterval));
        _dbPingTimer->async_wait(KeepDatabaseAliveHandler);
    }
}

void BanExpiryHandler(boost::system::error_code const& error)
{
    if (!error)
    {
        LoginDatabase.Execute(LoginDatabase.GetPreparedStatement(LOGIN_DEL_EXPIRED_IP_BANS));
        LoginDatabase.Execute(LoginDatabase.GetPreparedStatement(LOGIN_UPD_EXPIRED_ACCOUNT_BANS));

        _banExpiryCheckTimer->expires_from_now(boost::posix_time::seconds(_banExpiryCheckInterval));
        _banExpiryCheckTimer->async_wait(BanExpiryHandler);
    }
}

#if PLATFORM == TC_PLATFORM_WINDOWS
void ServiceStatusWatcher(boost::system::error_code const& error)
{
    if (!error)
    {
        if (m_ServiceStatus == 0)
        {
            _ioService.stop();
            delete _serviceStatusWatchTimer;
        }
        else
        {
            _serviceStatusWatchTimer->expires_from_now(boost::posix_time::seconds(1));
            _serviceStatusWatchTimer->async_wait(ServiceStatusWatcher);
        }
    }
}
#endif

variables_map GetConsoleArguments(int argc, char** argv, std::string& configFile, std::string& configService)
{
    (void)configService;

    options_description all("Allowed options");
    all.add_options()
        ("help,h", "print usage message")
        ("version,v", "print version build info")
        ("config,c", value<std::string>(&configFile)->default_value(_TRINITY_BNET_CONFIG), "use <arg> as configuration file")
        ;
#if PLATFORM == TC_PLATFORM_WINDOWS
    options_description win("Windows platform specific options");
    win.add_options()
        ("service,s", value<std::string>(&configService)->default_value(""), "Windows service options: [install | uninstall]")
        ;

    all.add(win);
#endif
    variables_map variablesMap;
    try
    {
        store(command_line_parser(argc, argv).options(all).allow_unregistered().run(), variablesMap);
        notify(variablesMap);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }

    if (variablesMap.count("help"))
    {
        std::cout << all << "\n";
    }
    else if (variablesMap.count("version"))
    {
        std::cout << GitRevision::GetFullVersion() << "\n";
    }

    return variablesMap;
}

void ShutdownThreadPool(std::vector<std::thread>& threadPool)
{
    _ioService.stop();

    for (auto& thread : threadPool)
    {
        thread.join();
    }
}
