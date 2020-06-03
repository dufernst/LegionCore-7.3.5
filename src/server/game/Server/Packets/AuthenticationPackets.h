/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#ifndef AuthenticationPacketsWorld_h__
#define AuthenticationPacketsWorld_h__

#include "Packet.h"
#include "Define.h"

namespace WorldPackets
{
    namespace Auth
    {
        class EarlyProcessClientPacket : public ClientPacket
        {
        public:
            EarlyProcessClientPacket(OpcodeClient opcode, WorldPacket&& packet) : ClientPacket(opcode, std::move(packet)) { }

            bool ReadNoThrow();
        };

        class Ping final : public EarlyProcessClientPacket
        {
        public:
            Ping(WorldPacket&& packet) : EarlyProcessClientPacket(CMSG_PING, std::move(packet)) { }

            uint32 Serial = 0;
            uint32 Latency = 0;

        private:
            void Read() override;
        };

        class AuthChallenge final : public ServerPacket
        {
        public:
            AuthChallenge() : ServerPacket(SMSG_AUTH_CHALLENGE, 4 + 32 + 1) { }

            WorldPacket const* Write() override;

            std::array<uint8, 16> Challenge{};
            uint32 DosChallenge[8]{};
            uint8 DosZeroBits = 0;
        };

        class AuthSession final : public EarlyProcessClientPacket
        {
        public:
            static uint32 const DigestLength = 24;

            AuthSession(WorldPacket&& packet) : EarlyProcessClientPacket(CMSG_AUTH_SESSION, std::move(packet))
            {
                LocalChallenge.fill(0);
                Digest.fill(0);
            }

            void Read() override;

            uint16 Build = 0;
            int8 BuildType = 0;
            uint32 RegionID = 0;
            uint32 BattlegroupID = 0;
            uint32 RealmID = 0;
            std::array<uint8, 16> LocalChallenge;
            std::array<uint8, DigestLength> Digest;
            uint64 DosResponse = 0;
            std::string RealmJoinTicket;
            bool UseIPv6 = false;
        };

        struct AuthWaitInfo
        {
            uint32 WaitCount = 0; ///< position of the account in the login queue
            uint32 WaitTime = 0; ///< Wait time in login queue in minutes, if sent queued and this value is 0 client displays "unknown time"
            bool HasFCM = false; ///< true if the account has a forced character migration pending. @todo implement
        };

        struct VirtualRealmNameInfo
        {
            VirtualRealmNameInfo() : IsLocal(false), IsInternalRealm(false) { }
            VirtualRealmNameInfo(bool isHomeRealm, bool isInternalRealm, std::string const& realmNameActual, std::string const& realmNameNormalized) : IsLocal(isHomeRealm), IsInternalRealm(isInternalRealm), RealmNameActual(realmNameActual), RealmNameNormalized(realmNameNormalized) { }

            bool IsLocal;                    ///< true if the realm is the same as the account's home realm
            bool IsInternalRealm;            ///< @todo research
            std::string RealmNameActual;     ///< the name of the realm
            std::string RealmNameNormalized; ///< the name of the realm without spaces
        };

        struct VirtualRealmInfo
        {
            VirtualRealmInfo(uint32 realmAddress, bool isHomeRealm, bool isInternalRealm, std::string const& realmNameActual, std::string const& realmNameNormalized) : RealmAddress(realmAddress), RealmNameInfo(isHomeRealm, isInternalRealm, realmNameActual, realmNameNormalized) { }

            uint32 RealmAddress;             ///< the virtual address of this realm, constructed as RealmHandle::Region << 24 | RealmHandle::Battlegroup << 16 | RealmHandle::Index
            VirtualRealmNameInfo RealmNameInfo;
        };

        class AuthResponse final : public ServerPacket
        {
        public:
            struct CharcterTemplateClass
            {
                CharcterTemplateClass(uint8 factionGroup, uint8 classID);

                uint8 FactionGroup;
                uint8 ClassID;
            };

            struct CharacterTemplateItemStruct
            {
                CharacterTemplateItemStruct(uint32 itemID, uint32 count, uint8 classID, uint8 factionGroup);

                uint32 ItemID;
                uint32 Count;
                uint8 ClassID;
                uint8 FactionGroup;
            };

            struct CharacterTemplateData
            {
                uint32 TemplateSetID;
                std::vector<CharcterTemplateClass> Classes;
                std::vector<CharacterTemplateItemStruct> Items;
                std::string Name;
                std::string Description;
            };

            struct AuthSuccessInfo
            {
                struct BillingInfo
                {
                    uint32 BillingPlan = 0;
                    uint32 TimeRemain = 0;
                    bool InGameRoom = false;
                };

                uint8 AccountExpansionLevel = 0;
                uint8 ActiveExpansionLevel = 0;
                uint32 TimeRested = 0;

                uint32 VirtualRealmAddress = 0;
                uint32 TimeSecondsUntilPCKick = 0;
                uint32 CurrencyID = 0;
                int32 Time = 0;

                BillingInfo Billing;

                std::vector<VirtualRealmInfo> VirtualRealms;
                std::vector<CharacterTemplateData> Templates;

                std::unordered_map<uint8, uint8> const* AvailableClasses = nullptr;

                bool IsExpansionTrial = false;
                bool ForceCharacterTemplate = false;
                Optional<uint16> NumPlayersHorde;
                Optional<uint16> NumPlayersAlliance;
            };

            AuthResponse();

            WorldPacket const* Write() override;

            Optional<AuthSuccessInfo> SuccessInfo;
            Optional<AuthWaitInfo> WaitInfo;
            uint32 Result = 0;
        };

        class WaitQueueUpdate final : public ServerPacket
        {
        public:
            WaitQueueUpdate() : ServerPacket(SMSG_WAIT_QUEUE_UPDATE, 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            AuthWaitInfo WaitInfo;
        };

        enum class ConnectToSerial : uint32
        {
            None = 0,
            Realm = 14,
            WorldAttempt1 = 17,
            WorldAttempt2 = 35,
            WorldAttempt3 = 53,
            WorldAttempt4 = 71,
            WorldAttempt5 = 89
        };

        class ConnectTo final : public ServerPacket
        {
            static std::string const Haiku;
            static uint8 const PiDigits[130];

        public:
            enum AddressType : uint8
            {
                IPv4 = 1,
                IPv6 = 2
            };

            struct ConnectPayload
            {
                std::array<uint8, 16> Where;
                uint32 Adler32 = 0;
                uint16 Port;
                AddressType Type;
                uint8 XorMagic = 0x2A;
                uint8 PanamaKey[32];
            };

            ConnectTo();
            static bool InitializeEncryption();

            WorldPacket const* Write() override;

            uint64 Key = 0;
            ConnectToSerial Serial = ConnectToSerial::None;
            ConnectPayload Payload;
            uint8 Con = 0;
        };

        class AuthContinuedSession final : public EarlyProcessClientPacket
        {
        public:
            static uint32 const DigestLength = 24;

            AuthContinuedSession(WorldPacket&& packet) : EarlyProcessClientPacket(CMSG_AUTH_CONTINUED_SESSION, std::move(packet))
            {
                LocalChallenge.fill(0);
                Digest.fill(0);
            }

            void Read() override;

            uint64 DosResponse = 0;
            uint64 Key = 0;
            std::array<uint8, 16> LocalChallenge;
            std::array<uint8, DigestLength> Digest;
        };

        class LogDisconnect final : public ClientPacket
        {
        public:
            LogDisconnect(WorldPacket&& packet) : ClientPacket(CMSG_LOG_DISCONNECT, std::move(packet)) { }

            void Read() override;

            uint32 Reason = 0;
        };

        class ResumeComms final : public ServerPacket
        {
        public:
            ResumeComms(ConnectionType connection) : ServerPacket(SMSG_RESUME_COMMS, 0, connection) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class ConnectToFailed final : public EarlyProcessClientPacket
        {
        public:
            ConnectToFailed(WorldPacket&& packet) : EarlyProcessClientPacket(CMSG_CONNECT_TO_FAILED, std::move(packet)) { }

            void Read() override;

            ConnectToSerial Serial = ConnectToSerial::None;
            uint8 Con = 0;
        };

        class WardenData final : public EarlyProcessClientPacket
        {
        public:
            WardenData(WorldPacket&& packet) : EarlyProcessClientPacket(CMSG_WARDEN_DATA, std::move(packet)) { }

            void Read() override;

            ByteBuffer Data;
        };

        class WaitQueueFinish final : public ServerPacket
        {
        public:
            WaitQueueFinish() : ServerPacket(SMSG_WAIT_QUEUE_FINISH, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class EnableEncryption final : public ServerPacket
        {
        public:
            EnableEncryption() : ServerPacket(SMSG_ENABLE_ENCRYPTION, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class BanReason final : public ServerPacket
        {
        public:
            BanReason() : ServerPacket(SMSG_BAN_REASON, 4) { }

            WorldPacket const* Write() override;

            uint32 Reason = 0;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Auth::VirtualRealmNameInfo const& realmInfo);

#endif // AuthenticationPacketsWorld_h__
