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

#include "WorldSocket.h"
#include "AuthenticationPackets.h"
#include "BattlenetRpcErrorCodes.h"
#include "BigNumber.h"
#include "CharacterPackets.h"
#include "HmacHash.h"
#include "Opcodes.h"
#include "PacketLog.h"
#include "ScriptMgr.h"
#include "SessionKeyGeneration.h"
#include "SHA256.h"
#include "World.h"
#include "Warden.h"
#include "Duration.h"

#include <zlib.h>
#include <memory>
#include "DatabaseEnv.h"
#include "RealmList.h"

#pragma pack(push, 1)

struct CompressedWorldPacket
{
    uint32 UncompressedSize;
    uint32 UncompressedAdler;
    uint32 CompressedAdler;
};

#pragma pack(pop)

using boost::asio::ip::tcp;

std::string const WorldSocket::ServerConnectionInitialize("WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT");
std::string const WorldSocket::ClientConnectionInitialize("WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER");
uint32 const WorldSocket::MinSizeForCompression = 0x400;
uint32 const SizeOfHeader = sizeof(uint32) + sizeof(uint16);
uint8 const WorldSocket::AuthCheckSeed[16] = { 0xC5, 0xC6, 0x98, 0x95, 0x76, 0x3F, 0x1D, 0xCD, 0xB6, 0xA1, 0x37, 0x28, 0xB3, 0x12, 0xFF, 0x8A };
uint8 const WorldSocket::SessionKeySeed[16] = { 0x58, 0xCB, 0xCF, 0x40, 0xFE, 0x2E, 0xCE, 0xA6, 0x5A, 0x90, 0xB8, 0x01, 0x68, 0x6C, 0x28, 0x0B };
uint8 const WorldSocket::ContinuedSessionSeed[16] = { 0x16, 0xAD, 0x0C, 0xD4, 0x46, 0xF9, 0x4F, 0xB2, 0xEF, 0x7D, 0xEA, 0x2A, 0x17, 0x66, 0x4D, 0x2F };

WorldSocket::WorldSocket(tcp::socket&& socket) : Socket(std::move(socket))
{
    _type = CONNECTION_TYPE_REALM;
    _key = 0;

    _worldSession = nullptr;
    _compressionStream = nullptr;
    _OverSpeedPings = 0;
    _accountId = 0;
    _authed = false;

    _serverChallenge.SetRand(8 * 16);
    _headerBuffer.Resize(SizeOfHeader);
}

WorldSocket::~WorldSocket()
{
    if (_compressionStream)
    {
        deflateEnd(_compressionStream);
        delete _compressionStream;
    }
}

void WorldSocket::Start()
{
    std::string ip_address = GetRemoteIpAddress().to_string();
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_IP_INFO);
    stmt->setString(0, ip_address);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (result)
    {
        TC_LOG_INFO(LOG_FILTER_NETWORKIO, "WorldSocket::CheckIpCallback: Sent Auth Response (IP %s banned).", GetRemoteIpAddress().to_string().c_str());
        DelayedCloseSocket();
        return;
    }

    _packetBuffer.Resize(ClientConnectionInitialize.length() + 1);

    AsyncReadWithCallback(&WorldSocket::InitializeHandler);

    MessageBuffer initializer;
    initializer.Write(ServerConnectionInitialize.c_str(), ServerConnectionInitialize.length());
    initializer.Write("\n", 1);

    QueuePacket(std::move(initializer));
    //_queryProcessor.AddQuery(LoginDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSocket::CheckIpCallback, this, std::placeholders::_1)));
}

void WorldSocket::CheckIpCallback(PreparedQueryResult result)
{
    if (result)
    {
        bool banned = false;
        do
        {
            Field* fields = result->Fetch();
            if (fields[0].GetUInt64() != 0)
                banned = true;

            if (!fields[1].GetString().empty())
                _ipCountry = fields[1].GetString();

        } while (result->NextRow());

        if (banned)
        {
            TC_LOG_INFO(LOG_FILTER_NETWORKIO, "WorldSocket::CheckIpCallback: Sent Auth Response (IP %s banned).", GetRemoteIpAddress().to_string().c_str());
            DelayedCloseSocket();
            return;
        }
    }

    _packetBuffer.Resize(ClientConnectionInitialize.length() + 1);

    AsyncReadWithCallback(&WorldSocket::InitializeHandler);

    MessageBuffer initializer;
    initializer.Write(ServerConnectionInitialize.c_str(), ServerConnectionInitialize.length());
    initializer.Write("\n", 1);

    QueuePacket(std::move(initializer));
}

void WorldSocket::InitializeHandler(boost::system::error_code error, std::size_t transferedBytes)
{
    TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "WorldSocket::InitializeHandler: called");

    if (error)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::InitializeHandler: closed with error %s (address: %s)", error.message().c_str(), GetRemoteIpAddress().to_string().c_str());
        CloseSocket();
        return;
    }

    GetReadBuffer().WriteCompleted(transferedBytes);

    MessageBuffer& packet = GetReadBuffer();
    if (packet.GetActiveSize() > 0)
    {
        if (_packetBuffer.GetRemainingSpace() > 0)
        {
            std::size_t readHeaderSize = std::min(packet.GetActiveSize(), _packetBuffer.GetRemainingSpace());
            _packetBuffer.Write(packet.GetReadPointer(), readHeaderSize);
            packet.ReadCompleted(readHeaderSize);

            if (_packetBuffer.GetRemainingSpace() > 0)
            {
                TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "WorldSocket::InitializeHandler: Couldn't receive the whole header this time (address: %s)", GetRemoteIpAddress().to_string().c_str());

                ASSERT(packet.GetActiveSize() == 0);
                AsyncReadWithCallback(&WorldSocket::InitializeHandler);
                return;
            }

            ByteBuffer buffer(std::move(_packetBuffer));
            try
            {
                if (/*initializer*/buffer.ReadString(ClientConnectionInitialize.length()) != ClientConnectionInitialize)
                {
                    TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "WorldSocket::InitializeHandler: initializer (address: %s)", GetRemoteIpAddress().to_string().c_str());
                    CloseSocket();
                    return;
                }
            }
            catch (ByteBufferException const&)
            {
                TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "WorldSocket::InitializeHandler ByteBufferException occured while parsing a socket initialization packet from address %s. Skipped packet.",
                    GetRemoteIpAddress().to_string().c_str());
            }

            if (/*terminator*/buffer.read<uint8>() != '\n')
            {
                TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "WorldSocket::InitializeHandler: terminator (address: %s)", GetRemoteIpAddress().to_string().c_str());
                CloseSocket();
                return;
            }

            _compressionStream = new z_stream();
            _compressionStream->zalloc = alloc_func(nullptr);
            _compressionStream->zfree = free_func(nullptr);
            _compressionStream->opaque = voidpf(nullptr);
            _compressionStream->avail_in = 0;
            _compressionStream->next_in = nullptr;
            int32 z_res = deflateInit2(_compressionStream, sWorld->getIntConfig(CONFIG_COMPRESSION), Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
            if (z_res != Z_OK)
            {
                TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Can't initialize packet compression (zlib: deflateInit) Error code: %i (%s)", z_res, zError(z_res));
                CloseSocket();
                return;
            }

            _packetBuffer.Reset();
            HandleSendAuthSession();
            AsyncRead();
            return;
        }
    }

    AsyncReadWithCallback(&WorldSocket::InitializeHandler);
}

bool WorldSocket::Update()
{
    uint32 _s = getMSTime();
    uint32 bufferSize = 0;
    std::queue<EncryptablePacket> _aBufferQueue;
    if (!_bufferQueue.empty())
    {
        _bufferQueueLock.lock();
        bufferSize = _bufferQueue.size();
        std::swap(_aBufferQueue, _bufferQueue);
        _bufferQueueLock.unlock();
    }

    MessageBuffer buffer;
    while (!_aBufferQueue.empty())
    {
        auto queued = std::move(_aBufferQueue.front());
        _aBufferQueue.pop();

        uint32 packetSize = queued.size();
        if (packetSize > MinSizeForCompression && queued.NeedsEncryption())
            packetSize = compressBound(packetSize) + sizeof(CompressedWorldPacket);

        if (buffer.GetRemainingSpace() < packetSize + SizeOfHeader)
        {
            QueuePacket(std::move(buffer));
            buffer.Resize(4096);  // NOLINT
        }

        if (buffer.GetRemainingSpace() >= packetSize + SizeOfHeader)
            WritePacketToBuffer(queued, buffer);
        else    // single packet larger than 4096 bytes
        {
            MessageBuffer packetBuffer(packetSize + SizeOfHeader);
            WritePacketToBuffer(queued, packetBuffer);
            QueuePacket(std::move(packetBuffer));
        }
    }

    if (buffer.GetActiveSize() > 0)
        QueuePacket(std::move(buffer));

    uint32 _ms = GetMSTimeDiffToNow(_s);
    if (_ms > 200)
        sLog->outDiff("WorldSocket::Update Update time - %ums bufferSize %u", _ms, bufferSize);

    if (!BaseSocket::Update())
        return false;

    _queryProcessor.ProcessReadyQueries();

    return true;
}

void WorldSocket::HandleSendAuthSession()
{
    _encryptSeed.SetRand(16 * 8);
    _decryptSeed.SetRand(16 * 8);

    WorldPackets::Auth::AuthChallenge challenge;
    memcpy(challenge.Challenge.data(), _serverChallenge.AsByteArray(16).get(), 16);
    memcpy(&challenge.DosChallenge[0], _encryptSeed.AsByteArray(16).get(), 16);
    memcpy(&challenge.DosChallenge[4], _decryptSeed.AsByteArray(16).get(), 16);
    challenge.DosZeroBits = 1;

    SendPacket(*challenge.Write());
}

void WorldSocket::OnClose()
{
    if (_worldSession)
        _worldSession->SetforceExit();

    std::lock_guard<std::mutex> sessionGuard(_worldSessionLock);
    _worldSession = nullptr;
}

void WorldSocket::ReadHandler()
{
    if (!IsOpen())
        return;

    MessageBuffer& packet = GetReadBuffer();
    while (packet.GetActiveSize() > 0)
    {
        if (_headerBuffer.GetRemainingSpace() > 0)
        {
            // need to receive the header
            std::size_t readHeaderSize = std::min(packet.GetActiveSize(), _headerBuffer.GetRemainingSpace());
            _headerBuffer.Write(packet.GetReadPointer(), readHeaderSize);
            packet.ReadCompleted(readHeaderSize);

            if (_headerBuffer.GetRemainingSpace() > 0)
            {
                // Couldn't receive the whole header this time.
                ASSERT(packet.GetActiveSize() == 0);
                break;
            }

            // We just received nice new header
            if (!ReadHeaderHandler())
            {
                CloseSocket();
                return;
            }
        }

        // We have full read header, now check the data payload
        if (_packetBuffer.GetRemainingSpace() > 0)
        {
            // need more data in the payload
            std::size_t readDataSize = std::min(packet.GetActiveSize(), _packetBuffer.GetRemainingSpace());
            _packetBuffer.Write(packet.GetReadPointer(), readDataSize);
            packet.ReadCompleted(readDataSize);

            if (_packetBuffer.GetRemainingSpace() > 0)
            {
                // Couldn't receive the whole data this time.
                ASSERT(packet.GetActiveSize() == 0);
                break;
            }
        }

        // just received fresh new payload
        ReadDataHandlerResult result = ReadDataHandler();
        _headerBuffer.Reset();
        if (result != ReadDataHandlerResult::Ok)
        {
            if (result != ReadDataHandlerResult::WaitingForQuery)
                CloseSocket();

            return;
        }
    }

    AsyncRead();
}

void WorldSocket::SetWorldSession(WorldSessionPtr session)
{
    std::lock_guard<std::mutex> sessionGuard(_worldSessionLock);
    _worldSession = session;
    _accountId = session->GetAccountId();
    _authed = true;
}

bool WorldSocket::ReadHeaderHandler()
{
    ASSERT(_headerBuffer.GetActiveSize() == SizeOfHeader, "Header size " SZFMTD " different than expected %u", _headerBuffer.GetActiveSize(), SizeOfHeader);

    _authCrypt.DecryptRecv(_headerBuffer.GetReadPointer(), 4);

    auto header = reinterpret_cast<PacketHeader*>(_headerBuffer.GetReadPointer());
    header->Size -= 2;

    if (!header->IsValidSize() || !header->IsValidOpcode())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::ReadHeaderHandler(): client %s sent malformed packet (size: %u, cmd: %u)", GetRemoteIpAddress().to_string().c_str(), header->Size - 2, header->Command);
        return false;
    }

    _packetBuffer.Resize(header->Size);
    return true;
}

WorldSocket::ReadDataHandlerResult WorldSocket::ReadDataHandler()
{
    auto opcode = static_cast<OpcodeClient>(reinterpret_cast<PacketHeader*>(_headerBuffer.GetReadPointer())->Command);

    WorldPacket packet(opcode, std::move(_packetBuffer), GetConnectionType());

    sPacketLog->LogPacket(packet, CLIENT_TO_SERVER, GetRemoteIpAddress(), GetRemotePort(), GetConnectionType());

    std::unique_lock<std::mutex> sessionGuard(_worldSessionLock, std::defer_lock);

    switch (opcode)
    {
        case CMSG_PING:
        {
            LogOpcodeText(opcode, sessionGuard);
            WorldPackets::Auth::Ping ping(std::move(packet));
            if (!ping.ReadNoThrow())
            {
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "WorldSocket::ReadDataHandler(): client %s sent malformed CMSG_PING", GetRemoteIpAddress().to_string().c_str());
                return ReadDataHandlerResult::Error;
            }
            if (!HandlePing(ping))
                return ReadDataHandlerResult::Error;
            break;
        }
        case CMSG_AUTH_SESSION:
        {
            LogOpcodeText(opcode, sessionGuard);
            if (_authed)
            {
                // locking just to safely log offending user is probably overkill but we are disconnecting him anyway
                if (sessionGuard.try_lock())
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::ProcessIncoming: received duplicate CMSG_AUTH_SESSION from %s", _worldSession->GetPlayerName().c_str());
                return ReadDataHandlerResult::Error;
            }

            std::shared_ptr<WorldPackets::Auth::AuthSession> authSession = std::make_shared<WorldPackets::Auth::AuthSession>(std::move(packet));
            if (!authSession->ReadNoThrow())
            {
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "WorldSocket::ReadDataHandler(): client %s sent malformed CMSG_AUTH_SESSION", GetRemoteIpAddress().to_string().c_str());
                return ReadDataHandlerResult::Error;
            }
            HandleAuthSession(authSession);
            return ReadDataHandlerResult::WaitingForQuery;
        }
        case CMSG_AUTH_CONTINUED_SESSION:
        {
            LogOpcodeText(opcode, sessionGuard);
            if (_authed)
            {
                if (sessionGuard.try_lock())
                    TC_LOG_ERROR(LOG_FILTER_OPCODES, "WorldSocket::ProcessIncoming: received duplicate CMSG_AUTH_CONTINUED_SESSION from %s", _worldSession->GetPlayerName().c_str());
                return ReadDataHandlerResult::Error;
            }

            std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession = std::make_shared<WorldPackets::Auth::AuthContinuedSession>(std::move(packet));
            if (!authSession->ReadNoThrow())
            {
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "WorldSocket::ReadDataHandler(): client %s sent malformed CMSG_AUTH_CONTINUED_SESSION", GetRemoteIpAddress().to_string().c_str());
                return ReadDataHandlerResult::Error;
            }
            HandleAuthContinuedSession(authSession);
            return ReadDataHandlerResult::WaitingForQuery;
        }
        case CMSG_KEEP_ALIVE:
            LogOpcodeText(opcode, sessionGuard);
            break;
        case CMSG_LOG_DISCONNECT:
        {
            LogOpcodeText(opcode, sessionGuard);
            std::shared_ptr<WorldPackets::Auth::LogDisconnect> log = std::make_shared<WorldPackets::Auth::LogDisconnect>(std::move(packet));
            log->Read();
            if (_worldSession)
            {
                TC_LOG_INFO(LOG_FILTER_NETWORKIO, "WorldPackets::Auth::LogDisconnect: %s was disconnected due to reason %u", _worldSession->GetPlayerName().c_str(), log->Reason);
                if (WorldSessionPtr sess__ = sWorld->FindSession(_accountId))
                    sess__->SetforceExit();
            }
            break;
        }
        case CMSG_ENABLE_NAGLE:
            LogOpcodeText(opcode, sessionGuard);
            SetNoDelay(false);
            break;
        case CMSG_CONNECT_TO_FAILED:
        {
            sessionGuard.lock();

            LogOpcodeText(opcode, sessionGuard);
            WorldPackets::Auth::ConnectToFailed connectToFailed(std::move(packet));
            if (!connectToFailed.ReadNoThrow())
            {
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "WorldSocket::ReadDataHandler(): client %s sent malformed CMSG_CONNECT_TO_FAILED", GetRemoteIpAddress().to_string().c_str());
                return ReadDataHandlerResult::Error;
            }
            HandleConnectToFailed(connectToFailed);
            break;
        }
        case CMSG_ENABLE_ENCRYPTION_ACK:
            LogOpcodeText(opcode, sessionGuard);
            HandleEnableEncryptionAck();
            break;
        case CMSG_WARDEN_DATA:
        {
            LogOpcodeText(opcode, sessionGuard);
            WorldPackets::Auth::WardenData wardenPacket(std::move(packet));
            if (!wardenPacket.ReadNoThrow())
            {
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "WorldSocket::ReadDataHandler(): client %s sent malformed CMSG_WARDEN_DATA", GetRemoteIpAddress().to_string().c_str());
                return ReadDataHandlerResult::Error;
            }
            if (!HandleWardenData(wardenPacket))
                return ReadDataHandlerResult::Error;
            break;
        }
        default:
        {
            sessionGuard.lock();

            LogOpcodeText(opcode, sessionGuard);

            if (!_worldSession)
            {
                #ifdef WIN32
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "ProcessIncoming: Client not authed opcode = %u", uint32(opcode));
                #endif
                return ReadDataHandlerResult::Error;
            }

            OpcodeHandler const* handler = opcodeTable[opcode];
            if (!handler)
            {
                #ifdef WIN32
                TC_LOG_ERROR(LOG_FILTER_OPCODES, "No defined handler for opcode %s sent by %s", GetOpcodeNameForLogging(static_cast<OpcodeClient>(packet.GetOpcode())).c_str(), _worldSession->GetPlayerName().c_str());
                #endif
                break;
            }

            // Our Idle timer will reset on any non PING opcodes.
            // Catches people idling on the login screen and any lingering ingame connections.
            if (opcode != CMSG_UI_TIME_REQUEST)
                _worldSession->ResetTimeOutTime();

            // Copy the packet to the heap before enqueuing
            _worldSession->QueuePacket(new WorldPacket(std::move(packet)));
            break;
        }
    }

    return ReadDataHandlerResult::Ok;
}

void WorldSocket::LogOpcodeText(OpcodeClient opcode, std::unique_lock<std::mutex> const& guard) const
{
    #ifdef WIN32
    if (!guard)
        TC_LOG_INFO(LOG_FILTER_OPCODES, "C->S: %s %s conn %i", GetOpcodeNameForLogging(opcode).c_str(), GetRemoteIpAddress().to_string().c_str(), GetConnectionType());
    else
        TC_LOG_INFO(LOG_FILTER_OPCODES, "C->S: %s %s conn %i", GetOpcodeNameForLogging(opcode).c_str(), (_worldSession ? _worldSession->GetPlayerName() : GetRemoteIpAddress().to_string()).c_str(), GetConnectionType());
    #endif
}

void WorldSocket::SendPacket(WorldPacket const& packet)
{
    if (!IsOpen())
        return;

    // uint32 opcode = packet.GetOpcode();
    uint32 packetSize = packet.size();
    if (packetSize > 0x7FFFFFF) // If packet size bugget, don`t send it http://pastebin.com/Q0xG8aGp
        return;

    sPacketLog->LogPacket(packet, SERVER_TO_CLIENT, GetRemoteIpAddress(), GetRemotePort(), GetConnectionType());

    #ifdef WIN32
    if (SMSG_ON_MONSTER_MOVE != static_cast<OpcodeServer>(packet.GetOpcode()))
        TC_LOG_INFO(LOG_FILTER_OPCODES, "S->C: %s Size %u %s connection %i, connectionType %i", GetOpcodeNameForLogging(static_cast<OpcodeServer>(packet.GetOpcode())).c_str(), packetSize, GetRemoteIpAddress().to_string().c_str(), packet.GetConnection(), GetConnectionType());
    #endif

    _bufferQueueLock.lock();
    _bufferQueue.emplace(EncryptablePacket(packet, _authCrypt.IsInitialized()));
    _bufferQueueLock.unlock();
}

void WorldSocket::WritePacketToBuffer(EncryptablePacket const& packet, MessageBuffer& buffer)
{
    uint32 opcode = packet.GetOpcode();
    uint32 packetSize = packet.size();

    // Reserve space for buffer
    uint8* headerPos = buffer.GetWritePointer();
    buffer.WriteCompleted(SizeOfHeader);

    if (packetSize > MinSizeForCompression && packet.NeedsEncryption())
    {
        CompressedWorldPacket cmp;
        cmp.UncompressedSize = packetSize + 2;
        cmp.UncompressedAdler = adler32(adler32(0x9827D8F1, reinterpret_cast<Bytef*>(&opcode), 2), packet.contents(), packetSize);

        // Reserve space for compression info - uncompressed size and checksums
        uint8* compressionInfo = buffer.GetWritePointer();
        buffer.WriteCompleted(sizeof(CompressedWorldPacket));

        uint32 compressedSize = CompressPacket(buffer.GetWritePointer(), packet);

        cmp.CompressedAdler = adler32(0x9827D8F1, buffer.GetWritePointer(), compressedSize);

        memcpy(compressionInfo, &cmp, sizeof(CompressedWorldPacket));
        buffer.WriteCompleted(compressedSize);
        packetSize = compressedSize + sizeof(CompressedWorldPacket);

        opcode = SMSG_COMPRESSED_PACKET;
    }
    else if (!packet.empty())
        buffer.Write(packet.contents(), packet.size());

    packetSize += 2 /*opcode*/;

    PacketHeader header;
    header.Size = packetSize;
    header.Command = opcode;
    _authCrypt.EncryptSend(reinterpret_cast<uint8*>(&header), 4);

    memcpy(headerPos, &header, SizeOfHeader);
}

uint32 WorldSocket::CompressPacket(uint8* buffer, WorldPacket const& packet)
{
    uint32 opcode = packet.GetOpcode();
    uint32 bufferSize = deflateBound(_compressionStream, packet.size() + sizeof(uint16));

    _compressionStream->next_out = buffer;
    _compressionStream->avail_out = bufferSize;
    _compressionStream->next_in = reinterpret_cast<Bytef*>(&opcode);
    _compressionStream->avail_in = sizeof(uint16);

    int32 z_res = deflate(_compressionStream, Z_NO_FLUSH);
    if (z_res != Z_OK)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Can't compress packet opcode (zlib: deflate) Error code: %i (%s, msg: %s)", z_res, zError(z_res), _compressionStream->msg);
        return 0;
    }

    _compressionStream->next_in = const_cast<Bytef*>(packet.contents());
    _compressionStream->avail_in = packet.size();

    z_res = deflate(_compressionStream, Z_SYNC_FLUSH);
    if (z_res != Z_OK)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Can't compress packet data (zlib: deflate) Error code: %i (%s, msg: %s)", z_res, zError(z_res), _compressionStream->msg);
        return 0;
    }

    return bufferSize - _compressionStream->avail_out;
}

struct AccountInfo
{
    struct
    {
        bool IsLockedToIP;
        std::string LastIP;
        std::string LockCountry;
        LocaleConstant Locale;

    } BattleNet;

    struct
    {
        std::array<uint8, 64> KeyData;
        int64 MuteTime;
        uint64 Hwid;
        uint32 Id;
        uint32 Recruiter;
        uint16 AtAuthFlag;
        uint32 BPayBalance;
        AccountTypes Security;
        std::string OS;
        uint8 Expansion;
        bool IsRectuiter;
        bool IsBanned;
    } Game;

    bool IsBanned() const
    {
        return Game.IsBanned;
    }

    explicit AccountInfo(Field* fields)
    {
        //          0          1            2          3               4           5           6          7           8     9           10          11         12
        // SELECT a.id, a.sessionkey, a.last_ip, a.locked, a.lock_country, a.expansion, a.mutetime, a.locale, a.recruiter, a.os, aa.gmLevel, a.AtAuthFlag, a.balans
        //                                                           13   14       15
        // ab.unbandate > UNIX_TIMESTAMP() OR ab.unbandate = ab.bandate, r.id, a.hwid
        // FROM account a LEFT JOIN account r ON a.id = r.recruiter LEFT JOIN account_access aa ON a.id = aa.id AND aa.RealmID IN (-1, ?) LEFT JOIN account_banned ab ON a.id = ab.id AND ab.active = 1
        // WHERE a.username = ? ORDER BY aa.RealmID DESC LIMIT 1
        Game.Id = fields[0].GetUInt32();
        HexStrToByteArray(fields[1].GetString(), Game.KeyData.data());
        BattleNet.LastIP = fields[2].GetString();
        BattleNet.IsLockedToIP = fields[3].GetBool();
        BattleNet.LockCountry = fields[4].GetString();
        Game.Expansion = /*std::max(fields[5].GetUInt8(), */CURRENT_EXPANSION;//);
        Game.MuteTime = fields[6].GetInt64();
        BattleNet.Locale = LocaleConstant(fields[7].GetUInt8());
        Game.Recruiter = fields[8].GetUInt32();
        Game.OS = fields[9].GetString();
        Game.Security = AccountTypes(fields[10].GetUInt8());
        Game.IsBanned = fields[13].GetUInt64() != 0;
        Game.IsRectuiter = fields[14].GetUInt32() != 0;
        Game.AtAuthFlag = AuthFlags(fields[11].GetUInt16());
        Game.BPayBalance = fields[12].GetUInt32();
        Game.Hwid = fields[15].GetUInt64();
        if (BattleNet.Locale >= MAX_LOCALES)
            BattleNet.Locale = LOCALE_enUS;
    }
};

void WorldSocket::HandleAuthSession(std::shared_ptr<WorldPackets::Auth::AuthSession> authSession)
{
    // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession");

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_INFO_BY_NAME);
    stmt->setInt32(0, int32(realm.Id.Realm));
    stmt->setString(1, authSession->RealmJoinTicket);

    _queryProcessor.AddQuery(LoginDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSocket::HandleAuthSessionCallback, this, authSession, std::placeholders::_1)));
}

void WorldSocket::HandleAuthSessionCallback(std::shared_ptr<WorldPackets::Auth::AuthSession> authSession, PreparedQueryResult result)
{
    // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSessionCallback");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Sent Auth Response (unknown account).");
        DelayedCloseSocket();
        return;
    }

	RealmBuildInfo const* buildInfo = sRealmList->GetBuildInfo(realm.Build);
	if (!buildInfo)
	{
		SendAuthResponseError(ERROR_BAD_VERSION);
		TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Missing auth seed for realm build %u (%s).", realm.Build, GetRemoteIpAddress().to_string().c_str());
		DelayedCloseSocket();
		return;
	}


    AccountInfo account(result->Fetch());

    // For hook purposes, we get Remoteaddress at this point.
    auto address = GetRemoteIpAddress().to_string();

    SHA256Hash digestKeyHash;
    digestKeyHash.UpdateData(account.Game.KeyData.data(), account.Game.KeyData.size());
    if (account.Game.OS == "Win")
        digestKeyHash.UpdateData(buildInfo->WinAuthSeed.data(), buildInfo->WinAuthSeed.size());
    else if (account.Game.OS == "Wn64")
        digestKeyHash.UpdateData(buildInfo->Win64AuthSeed.data(), buildInfo->Win64AuthSeed.size());
    else if (account.Game.OS == "Mc64")
        digestKeyHash.UpdateData(buildInfo->Mac64AuthSeed.data(), buildInfo->Mac64AuthSeed.size());
    digestKeyHash.Finalize();

    HmacSha256 hmac(digestKeyHash.GetLength(), digestKeyHash.GetDigest());
    hmac.UpdateData(authSession->LocalChallenge.data(), authSession->LocalChallenge.size());
    hmac.UpdateData(_serverChallenge.AsByteArray(16).get(), 16);
    hmac.UpdateData(AuthCheckSeed, 16);
    hmac.Finalize();

    // Check that Key and account name are the same on client and server
    if (memcmp(hmac.GetDigest(), authSession->Digest.data(), authSession->Digest.size()) != 0)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Authentication failed for account: %u ('%s') address: %s", account.Game.Id, authSession->RealmJoinTicket.c_str(), address.c_str());
        DelayedCloseSocket();
        return;
    }

    SHA256Hash keyData;
    keyData.UpdateData(account.Game.KeyData.data(), account.Game.KeyData.size());
    keyData.Finalize();
    HmacSha256 sessionKeyHmac(keyData.GetLength(), keyData.GetDigest());
    sessionKeyHmac.UpdateData(_serverChallenge.AsByteArray(16).get(), 16);
    sessionKeyHmac.UpdateData(authSession->LocalChallenge.data(), authSession->LocalChallenge.size());
    sessionKeyHmac.UpdateData(SessionKeySeed, 16);
    sessionKeyHmac.Finalize();

    uint8 sessionKey[40];
    SessionKeyGenerator<SHA256Hash> sessionKeyGenerator(sessionKeyHmac.GetDigest(), sessionKeyHmac.GetLength());
    sessionKeyGenerator.Generate(sessionKey, 40);
    _sessionKey.SetBinary(sessionKey, 40);

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_INFO_CONTINUED_SESSION);
    stmt->setString(0, _sessionKey.AsHexStr());
    stmt->setUInt32(1, account.Game.Id);
    LoginDatabase.Execute(stmt);

    // First reject the connection if packet contains invalid data or realm state doesn't allow logging in
    if (sWorld->IsClosed())
    {
        SendAuthResponseError(ERROR_BAD_SERVER);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: World closed, denying client (%s).", GetRemoteIpAddress().to_string().c_str());
        DelayedCloseSocket();
        return;
    }

    // if (authSession->RealmID != realm.Id.Realm)
    // {
        // SendAuthResponseError(ERROR_DENIED);
        // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Client %s requested connecting with realm id %u but this realm has id %u set in config.",
            // GetRemoteIpAddress().to_string().c_str(), authSession->RealmID, realm.Id.Realm);
        // DelayedCloseSocket();
        // return;
    // }

    // Must be done before WorldSession is created
    bool wardenActive = sWorld->getBoolConfig(CONFIG_WARDEN_ENABLED);
    if (wardenActive && account.Game.OS != "Win" && account.Game.OS != "Wn64" && account.Game.OS != "Mc64")
    {
        SendAuthResponseError(ERROR_BAD_VERSION);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Client %s attempted to log in using invalid client OS (%s).", address.c_str(), account.Game.OS.c_str());
        DelayedCloseSocket();
        return;
    }

    ///- Re-check ip locking (same check as in auth).
    if (account.BattleNet.IsLockedToIP)
    {
        if (account.BattleNet.LastIP != address)
        {
            SendAuthResponseError(ERROR_RISK_ACCOUNT_LOCKED);
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Client %s attempted to log in using IsLockedToIP.", address.c_str());
            DelayedCloseSocket();
            return;
        }
    }
    else if (!account.BattleNet.LockCountry.empty() && account.BattleNet.LockCountry != "00" && !_ipCountry.empty())
    {
        if (account.BattleNet.LockCountry != _ipCountry)
        {
            SendAuthResponseError(ERROR_RISK_ACCOUNT_LOCKED);
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Client %s LockCountry != _ipCountry", address.c_str());
            DelayedCloseSocket();
            return;
        }
    }

    int64 mutetime = account.Game.MuteTime;
    //! Negative mutetime indicates amount of seconds to be muted effective on next login - which is now.
    if (mutetime < 0)
    {
        mutetime = time(nullptr) + llabs(mutetime);

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_MUTE_TIME);
        stmt->setInt64(0, mutetime);
        stmt->setUInt32(1, account.Game.Id);
        LoginDatabase.Execute(stmt);
    }

    if (account.IsBanned())
    {
        SendAuthResponseError(ERROR_GAME_ACCOUNT_BANNED);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Client %s IsBanned()", address.c_str());
        DelayedCloseSocket();
        return;
    }

    // Check locked state for server
    AccountTypes allowedAccountType = sWorld->GetPlayerSecurityLimit();
    if (allowedAccountType > SEC_PLAYER && account.Game.Security < allowedAccountType)
    {
        SendAuthResponseError(ERROR_SERVER_IS_PRIVATE);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Client %s allowedAccountType %u Security %u", address.c_str(), allowedAccountType, account.Game.Security);
        DelayedCloseSocket();
        return;
    }

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_LAST_IP);

    stmt->setString(0, address);
    stmt->setUInt32(1, account.Game.Id);

    LoginDatabase.Execute(stmt);

    _authed = true;

    _worldSession = std::make_shared<WorldSession>(account.Game.Id, std::move(authSession->RealmJoinTicket), shared_from_this(), account.Game.Security,
        account.Game.Expansion, mutetime, account.Game.OS, account.BattleNet.Locale, account.Game.Recruiter, account.Game.IsRectuiter, AuthFlags(account.Game.AtAuthFlag), account.Game.BPayBalance);

    _worldSession->_realmID = authSession->RealmID;
    _worldSession->_hwid = account.Game.Hwid;

    if (_worldSession->_hwid != 0)
    {
        if (auto result2 = LoginDatabase.PQuery("SELECT penalties, last_reason from hwid_penalties where hwid = " UI64FMTD, _worldSession->_hwid))
        {
            Field* fields = result2->Fetch();
            _worldSession->_countPenaltiesHwid = fields[0].GetInt32();

            if ((sWorld->getIntConfig(CONFIG_ANTI_FLOOD_HWID_BANS_COUNT) && _worldSession->_countPenaltiesHwid >= sWorld->getIntConfig(CONFIG_ANTI_FLOOD_HWID_BANS_COUNT))
                || _worldSession->_countPenaltiesHwid < 0)
            {
                std::stringstream ss;
                ss << (fields[1].GetString().empty() ? "Antiflood unknwn" :  fields[1].GetCString()) << "*";
                if (sWorld->getBoolConfig(CONFIG_ANTI_FLOOD_HWID_BANS_ALLOW))
                {
                    sWorld->BanAccount(BAN_ACCOUNT, _worldSession->GetAccountName(), "-1", ss.str(), "Server");
                    SendAuthResponseError(ERROR_GAME_ACCOUNT_BANNED);
                    DelayedCloseSocket();
                    return;
                }
                
                if (sWorld->getBoolConfig(CONFIG_ANTI_FLOOD_HWID_MUTE_ALLOW))
                {
                    sWorld->MuteAccount(_worldSession->GetAccountId(), -1, ss.str(), "Server", _worldSession.get());
                }
                else if (sWorld->getBoolConfig(CONFIG_ANTI_FLOOD_HWID_KICK_ALLOW))
                {
                    SendAuthResponseError(ERROR_GAME_ACCOUNT_BANNED);
                    DelayedCloseSocket();
                    return;
                }
            }
        }
    }

    if (wardenActive)
    {
        if (!_worldSession->InitializeWarden(&_sessionKey, account.Game.OS))
        {
            SendAuthResponseError(ERROR_DENIED);
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthSession: Warden initializtion failed for account: %u ('%s') address: %s", account.Game.Id, authSession->RealmJoinTicket.c_str(), address.c_str());
            DelayedCloseSocket();
            return;
        }
    }

    _accountId = _worldSession->GetAccountId();

    AsyncRead();

    SendPacket(*WorldPackets::Auth::EnableEncryption().Write());
}

void WorldSocket::HandleAuthContinuedSession(std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession)
{
    // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthContinuedSession");

    WorldSession::ConnectToKey key;
    key.Raw = authSession->Key;

    _type = ConnectionType(key.Fields.ConnectionType);
    if (_type != CONNECTION_TYPE_INSTANCE)
    {
        SendAuthResponseError(ERROR_DENIED);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthContinuedSession: Client %s !_type != CONNECTION_TYPE_INSTANCE", GetRemoteIpAddress().to_string().c_str());
        DelayedCloseSocket();
        return;
    }

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_INFO_CONTINUED_SESSION);
    stmt->setUInt32(0, uint32(key.Fields.AccountId));

    _queryProcessor.AddQuery(LoginDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSocket::HandleAuthContinuedSessionCallback, this, authSession, std::placeholders::_1)));
}

void WorldSocket::HandleAuthContinuedSessionCallback(std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession, PreparedQueryResult result)
{
    // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthContinuedSessionCallback _worldSession %u", bool(_worldSession));

    if (!result)
    {
        SendAuthResponseError(ERROR_DENIED);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthContinuedSessionCallback: Client %s !result", GetRemoteIpAddress().to_string().c_str());
        DelayedCloseSocket();
        return;
    }

    WorldSession::ConnectToKey key;
    _key = key.Raw = authSession->Key;

    Field* fields = result->Fetch();
    _sessionKey.SetHexStr(fields[1].GetCString());

    HmacSha256 hmac(40, _sessionKey.AsByteArray(40).get());
    hmac.UpdateData(reinterpret_cast<uint8 const*>(&authSession->Key), sizeof(authSession->Key));
    hmac.UpdateData(authSession->LocalChallenge.data(), authSession->LocalChallenge.size());
    hmac.UpdateData(_serverChallenge.AsByteArray(16).get(), 16);
    hmac.UpdateData(ContinuedSessionSeed, 16);
    hmac.Finalize();

    if (memcmp(hmac.GetDigest(), authSession->Digest.data(), authSession->Digest.size()) != 0)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleAuthContinuedSession: Authentication failed for account: %u ('%s') address: %s",
            uint32(key.Fields.AccountId), fields[0].GetString().c_str(), GetRemoteIpAddress().to_string().c_str());
        DelayedCloseSocket();
        return;
    }

    SendPacket(*WorldPackets::Auth::EnableEncryption().Write());
    AsyncRead();
}

void WorldSocket::HandleConnectToFailed(WorldPackets::Auth::ConnectToFailed& connectToFailed)
{
    // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleConnectToFailed");

    if (_worldSession)
    {
        if (_worldSession->PlayerLoading())
        {
            switch (connectToFailed.Serial)
            {
                case WorldPackets::Auth::ConnectToSerial::WorldAttempt1:
                    _worldSession->SendConnectToInstance(WorldPackets::Auth::ConnectToSerial::WorldAttempt2);
                    break;
                case WorldPackets::Auth::ConnectToSerial::WorldAttempt2:
                    _worldSession->SendConnectToInstance(WorldPackets::Auth::ConnectToSerial::WorldAttempt3);
                    break;
                case WorldPackets::Auth::ConnectToSerial::WorldAttempt3:
                    _worldSession->SendConnectToInstance(WorldPackets::Auth::ConnectToSerial::WorldAttempt4);
                    break;
                case WorldPackets::Auth::ConnectToSerial::WorldAttempt4:
                    _worldSession->SendConnectToInstance(WorldPackets::Auth::ConnectToSerial::WorldAttempt5);
                    break;
                case WorldPackets::Auth::ConnectToSerial::WorldAttempt5:
                {
                    TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "%s failed to connect 5 times to world socket, aborting login", _worldSession->GetPlayerName().c_str());
                    _worldSession->AbortLogin(WorldPackets::Character::LoginFailureReason::NoWorld);
                    break;
                }
                default:
                    return;
            }
        }
        //else
        //{
        //    transfer_aborted when/if we get map node redirection
        //    SendPacket(*WorldPackets::Auth::ResumeComms().Write());
        //}
    }
}

void WorldSocket::SendAuthResponseError(uint32 code)
{
    WorldPackets::Auth::AuthResponse response;
    response.Result = code;
    SendPacket(*response.Write());
}

void WorldSocket::HandleEnableEncryptionAck()
{
    // TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleEnableEncryptionAck _worldSession %u _type %u", bool(_worldSession), _type);

    if (_type == CONNECTION_TYPE_REALM)
    {
        _authCrypt.Init(&_sessionKey);
        sWorld->AddSession(_worldSession);
    }
    else
    {
        _authCrypt.Init(&_sessionKey, _encryptSeed.AsByteArray().get(), _decryptSeed.AsByteArray().get());
        sWorld->AddInstanceSocket(shared_from_this(), _key);
    }
}

bool WorldSocket::HandlePing(WorldPackets::Auth::Ping& ping)
{
    if (_LastPingTime == TimePoint())
        _LastPingTime = SteadyClock::now();
    else
    {
        TimePoint now = SteadyClock::now();
        std::chrono::steady_clock::duration diff = now - _LastPingTime;

        _LastPingTime = now;

        if (diff < Seconds(27))
        {
            ++_OverSpeedPings;

            uint32 maxAllowed = sWorld->getIntConfig(CONFIG_MAX_OVERSPEED_PINGS);

            if (maxAllowed && _OverSpeedPings > maxAllowed)
            {
                std::unique_lock<std::mutex> sessionGuard(_worldSessionLock);

                if (_worldSession)
                {
                    TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandlePing: %s kicked for over-speed pings (address: %s)",
                        _worldSession->GetPlayerName().c_str(), GetRemoteIpAddress().to_string().c_str());

                    return false;
                }
            }
        }
        else
            _OverSpeedPings = 0;
    }

    std::lock_guard<std::mutex> sessionGuard(_worldSessionLock);

    if (_worldSession)
    {
        _worldSession->SetLatency(ping.Latency);
        _worldSession->ResetClientTimeDelay();

        if (Player* plr = _worldSession->GetPlayer())
            plr->SetPing(ping.Latency);
    }
    else
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandlePing: peer sent CMSG_PING, but is not authenticated or got recently kicked, address = %s", GetRemoteIpAddress().to_string().c_str());
        return false;
    }

    WorldPacket packet(SMSG_PONG, 4);
    packet << ping.Serial;
    SendPacket(packet);
    return true;
}

bool WorldSocket::HandleWardenData(WorldPackets::Auth::WardenData& packet)
{
    if (packet.Data.empty())
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleWardenData: peer sent CMSG_WARDEN_DATA, but packet length is not valid, address = %s", GetRemoteIpAddress().to_string().c_str());
        return false;
    }

    std::lock_guard<std::mutex> sessionGuard(_worldSessionLock);

    if (!_worldSession)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleWardenData: peer sent CMSG_WARDEN_DATA, but is not authenticated or got recently kicked, address = %s", GetRemoteIpAddress().to_string().c_str());
        return false;
    }

    Warden* _warden = _worldSession->GetWarden();

    if (!_warden)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSocket::HandleWardenData: peer sent CMSG_WARDEN_DATA, but server is not prepared for handle it, address = %s", GetRemoteIpAddress().to_string().c_str());
        return false;
    }

    if (_warden->GetState() == WARDEN_MODULE_SET_PLAYER_LOCK)
        return true;

    _warden->DecryptData(packet.Data.contents(), packet.Data.size());
    uint8 opcode;
    packet.Data >> opcode;
    TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: CMSG opcode %02X, size %u", opcode, uint32(packet.Data.size()));
    //sLog->outWarden("Raw Packet Decrypted Data - %s", ByteArrayToHexStr(const_cast<uint8*>(recvData.contents()), recvData.size()).c_str());

    switch (opcode)
    {
        case WARDEN_CMSG_MODULE_MISSING:
        {
            _warden->SendModuleToClient();
            break;
        }
        case WARDEN_CMSG_MODULE_OK:
        {
            _warden->ActivateModule();
            break;
        }
        case WARDEN_CMSG_CHEAT_CHECKS_RESULT:
        {
            _warden->HandleData(packet.Data);
            break;
        }
        case WARDEN_CMSG_MEM_CHECKS_RESULT:
        {
            // NYI - maybe used for extended protection
            TC_LOG_DEBUG(LOG_FILTER_WARDEN, "WARDEN: CMSG_MEM_CHECKS_RESULT received!");
            break;
        }
        case WARDEN_CMSG_HASH_RESULT:
        {
            if (_warden->GetState() == WARDEN_MODULE_CONNECTING_TO_MAIEV)
                _warden->HandleHashResultSpecial(packet.Data);
            else
                _warden->HandleHashResult(packet.Data);
            break;
        }
        case WARDEN_CMSG_MODULE_FAILED:
        {
            _warden->HandleModuleFailed();
            break;
        }
        // TODO: think about good name!
        case WARDEN_CMSG_EXTENDED_DATA:
        {
            _warden->HandleExtendedData(packet.Data);
            break;
        }
        // TODO: think about good name!
        case WARDEN_CMSG_STRING_DATA:
        {
            _warden->HandleStringData(packet.Data);
            break;
        }
        default:
        {
            sLog->outWarden("Unknown CMSG opcode %02X, size %u, account %s (%s), module state %s", opcode, uint32(packet.Data.size() - 1), _worldSession->GetAccountName().c_str(), _worldSession->GetOS().c_str(), _warden->GetStateString().c_str());
            break;
        }
    }

    return true;
}
