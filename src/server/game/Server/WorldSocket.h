/*
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#ifndef __WORLDSOCKET_H__
#define __WORLDSOCKET_H__

#include "WorldPacketCrypt.h"
#include "Socket.h"
#include "Util.h"
#include "WorldSession.h"
#include "DatabaseEnvFwd.h"
#include <chrono>
#include <safe_ptr.h>

struct z_stream_s;

class EncryptablePacket : public WorldPacket
{
public:
    EncryptablePacket(WorldPacket const& packet, bool encrypt) : WorldPacket(packet), _encrypt(encrypt) { }

    bool NeedsEncryption() const { return _encrypt; }

private:
    bool _encrypt;
};

namespace WorldPackets
{
    class ServerPacket;
    namespace Auth
    {
        class AuthSession;
        class AuthContinuedSession;
        class ConnectToFailed;
        class LogDisconnect;
        class Ping;
        class WardenData;
    }
}

#pragma pack(push, 1)

struct PacketHeader
{
    uint32 Size;
    uint16 Command;

    bool IsValidSize() { return Size < 0x10000; }
    bool IsValidOpcode() { return Command < NUM_OPCODE_HANDLERS; }
};

#pragma pack(pop)

class WorldSocket : public Socket<WorldSocket>
{
    static std::string const ServerConnectionInitialize;
    static std::string const ClientConnectionInitialize;
    static uint32 const MinSizeForCompression;

    static uint8 const AuthCheckSeed[16];
    static uint8 const SessionKeySeed[16];
    static uint8 const ContinuedSessionSeed[16];

    typedef Socket<WorldSocket> BaseSocket;

public:
    WorldSocket(boost::asio::ip::tcp::socket&& socket);
    ~WorldSocket();

    WorldSocket(WorldSocket const& right) = delete;
    WorldSocket& operator=(WorldSocket const& right) = delete;

    void Start() override;
    bool Update() override;

    void SendPacket(WorldPacket const& packet);

    ConnectionType GetConnectionType() const { return _type; }

    void SendAuthResponseError(uint32 code);
    void HandleEnableEncryptionAck();
    void SetWorldSession(WorldSessionPtr session);

protected:
    void OnClose() override;
    void ReadHandler() override;
    bool ReadHeaderHandler();

    enum class ReadDataHandlerResult
    {
        Ok = 0,
        Error = 1,
        WaitingForQuery = 2
    };

    ReadDataHandlerResult ReadDataHandler();
private:
    void CheckIpCallback(PreparedQueryResult result);
    void InitializeHandler(boost::system::error_code error, std::size_t transferedBytes);
    void LogOpcodeText(OpcodeClient opcode, std::unique_lock<std::mutex> const& guard) const;
    void WritePacketToBuffer(EncryptablePacket const& packet, MessageBuffer& buffer);
    uint32 CompressPacket(uint8* buffer, WorldPacket const& packet);

    void HandleSendAuthSession();
    void HandleAuthSession(std::shared_ptr<WorldPackets::Auth::AuthSession> authSession);
    void HandleAuthSessionCallback(std::shared_ptr<WorldPackets::Auth::AuthSession> authSession, PreparedQueryResult result);
    void HandleAuthContinuedSession(std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession);
    void HandleAuthContinuedSessionCallback(std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession, PreparedQueryResult result);
    void HandleConnectToFailed(WorldPackets::Auth::ConnectToFailed& connectToFailed);

    bool HandlePing(WorldPackets::Auth::Ping& ping);
    bool HandleWardenData(WorldPackets::Auth::WardenData& packet);

    ConnectionType _type;
    uint64 _key;

    BigNumber _serverChallenge;
    WorldPacketCrypt _authCrypt;
    BigNumber _encryptSeed;
    BigNumber _decryptSeed;
    BigNumber _sessionKey;

    std::chrono::steady_clock::time_point _LastPingTime;
    uint32 _OverSpeedPings;
    uint32 _accountId;

    std::mutex _worldSessionLock;
    WorldSessionPtr _worldSession;
    bool _authed;

    MessageBuffer _headerBuffer;
    MessageBuffer _packetBuffer;
    std::queue<EncryptablePacket> _bufferQueue;
    sf::contention_free_shared_mutex< > _bufferQueueLock;

    z_stream_s* _compressionStream;

    QueryCallbackProcessor _queryProcessor;
    std::string _ipCountry;
};

#endif
