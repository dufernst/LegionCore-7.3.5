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

#ifndef InstancePackets_h__
#define InstancePackets_h__

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Instance
    {
        class UpdateLastInstance final : public ServerPacket
        {
        public:
            UpdateLastInstance(uint32 ID) : ServerPacket(SMSG_UPDATE_LAST_INSTANCE, 4), MapID(ID) { }

            WorldPacket const* Write() override;

            uint32 MapID = 0;
        };

        class UpdateInstanceOwnership final : public ServerPacket
        {
        public:
            UpdateInstanceOwnership() : ServerPacket(SMSG_UPDATE_INSTANCE_OWNERSHIP, 4) { }

            WorldPacket const* Write() override;

            int32 IOwnInstance = 0;
        };

        struct InstanceLockInfos
        {
            uint64 InstanceID = 0u;
            uint32 MapID = 0u;
            uint32 DifficultyID = 0u;
            int32 TimeRemaining = 0;
            uint32 CompletedMask = 0u;

            bool Locked = false;
            bool Extended = false;
        };

        class InstanceInfo final : public ServerPacket
        {
        public:
            InstanceInfo() : ServerPacket(SMSG_INSTANCE_INFO, 4) { }

            WorldPacket const* Write() override;

            std::vector<InstanceLockInfos> LockList;
        };

        class ResetInstances final : public ClientPacket
        {
        public:
            ResetInstances(WorldPacket&& packet) : ClientPacket(CMSG_RESET_INSTANCES, std::move(packet)) { }

            void Read() override { }
        };

        class InstanceReset final : public ServerPacket
        {
        public:
            InstanceReset(uint32 ID) : ServerPacket(SMSG_INSTANCE_RESET, 4), MapID(ID) { }

            WorldPacket const* Write() override;

            uint32 MapID = 0;
        };

        class InstanceResetFailed final : public ServerPacket
        {
        public:
            InstanceResetFailed() : ServerPacket(SMSG_INSTANCE_RESET_FAILED, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 MapID = 0;
            uint8 ResetFailedReason = 0;
        };

        class ResetFailedNotify final : public ServerPacket
        {
        public:
            ResetFailedNotify() : ServerPacket(SMSG_RESET_FAILED_NOTIFY, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class InstanceSaveCreated final : public ServerPacket
        {
        public:
            InstanceSaveCreated(bool gm) : ServerPacket(SMSG_INSTANCE_SAVE_CREATED, 1), Gm(gm) { }

            WorldPacket const* Write() override;

            bool Gm = false;
        };

        class InstanceLockResponse final : public ClientPacket
        {
        public:
            InstanceLockResponse(WorldPacket&& packet) : ClientPacket(CMSG_INSTANCE_LOCK_RESPONSE, std::move(packet)) { }

            void Read() override;

            bool AcceptLock = false;
        };

        class RaidGroupOnly final : public ServerPacket
        {
        public:
            RaidGroupOnly() : ServerPacket(SMSG_RAID_GROUP_ONLY, 4 + 4) { }

            WorldPacket const* Write() override;

            int32 Delay = 0;
            uint32 Reason = 0;
        };

        class BossKillCredit final : public ServerPacket
        {
        public:
            BossKillCredit(int32 ID) : ServerPacket(SMSG_BOSS_KILL_CREDIT, 4), encounterID(ID) { }

            WorldPacket const* Write() override;

            int32 encounterID = 0;
        };

        //< SMSG_INSTANCE_ENCOUNTER_END
        //< SMSG_INSTANCE_ENCOUNTER_IN_COMBAT_RESURRECTION
        //< SMSG_INSTANCE_ENCOUNTER_PHASE_SHIFT_CHANGED
        //< SMSG_CLEAR_BOSS_EMOTES
        //< SMSG_SUMMON_CANCEL
        class NullSmsg final : public ServerPacket
        {
        public:
            NullSmsg(OpcodeServer opcode) : ServerPacket(opcode, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class RaidInstanceMessage final : public ServerPacket
        {
        public:
            RaidInstanceMessage() : ServerPacket(SMSG_RAID_INSTANCE_MESSAGE, 1 + 4 + 4 + 1 + 1) { }

            WorldPacket const* Write() override;

            int32 MapID = 0;
            int32 DifficultyID = 0;
            uint8 Type = 0;
            bool Locked = false;
            bool Extended = false;
        };

        class ChangePlayerDifficultyResult final : public ServerPacket
        {
        public:
            ChangePlayerDifficultyResult() : ServerPacket(SMSG_CHANGE_PLAYER_DIFFICULTY_RESULT, 1 + 1 + 4 + 4 + 4 + 4 + 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            uint32 CooldownReason = 0;
            uint32 InstanceMapID = 0;
            uint32 DifficultyRecID = 0;
            uint32 MapID = 0;
            uint8 Result = 0;
            bool Cooldown = false;
        };

        class InstanceGroupSizeChanged final : public ServerPacket
        {
        public:
            InstanceGroupSizeChanged(uint32 groupSize) : ServerPacket(SMSG_INSTANCE_GROUP_SIZE_CHANGED, 4), GroupSize(groupSize){ }

            WorldPacket const* Write() override;

            uint32 GroupSize = 0;
        };

        class InstanceEncounterStart final : public ServerPacket
        {
        public:
            InstanceEncounterStart() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_START, 4 + 4 + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            uint32 InCombatResCount = 0;
            uint32 MaxInCombatResCount = 0;
            uint32 CombatResChargeRecovery = 0;
            uint32 NextCombatResChargeTime = 0;
            bool InProgress = true;
        };

        class InstanceEncounterEngageUnit final : public ServerPacket
        {
        public:
            InstanceEncounterEngageUnit() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_ENGAGE_UNIT, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint8 TargetFramePriority = 0;
        };

        class InstanceEncounterChangePriority final : public ServerPacket
        {
        public:
            InstanceEncounterChangePriority() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_CHANGE_PRIORITY, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint8 TargetFramePriority = 0;
        };

        class InstanceEncounterDisengageUnit final : public ServerPacket
        {
        public:
            InstanceEncounterDisengageUnit() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_DISENGAGE_UNIT, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
        };

        class InstanceEncounterGainCombatResurrectionCharge final : public ServerPacket
        {
        public:
            InstanceEncounterGainCombatResurrectionCharge() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_GAIN_COMBAT_RESURRECTION_CHARGE, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 InCombatResCount = 0;
            uint32 CombatResChargeRecovery = 0;
        };

        class EncounterStart final : public ServerPacket
        {
        public:
            EncounterStart() : ServerPacket(SMSG_ENCOUNTER_START, 4) { }

            WorldPacket const* Write() override;

            uint32 EncounterID = 0;
            uint32 DifficultyID = 0;
            uint32 GroupSize = 0;
            uint32 UnkEncounterDataSize = 0;
        };

        class EncounterEnd final : public ServerPacket
        {
        public:
            EncounterEnd() : ServerPacket(SMSG_ENCOUNTER_END, 13) { }

            WorldPacket const* Write() override;

            uint32 EncounterID = 0;
            uint32 DifficultyID = 0;
            uint32 GroupSize = 0;
            bool Success = false;
        };

        class PendingRaidLock final : public ServerPacket
        {
        public:
            PendingRaidLock() : ServerPacket(SMSG_PENDING_RAID_LOCK, 10) { }

            WorldPacket const* Write() override;

            int32 TimeUntilLock = 0;
            uint32 CompletedMask = 0;
            bool Extending = false;
            bool WarningOnly = false;
        };

        class StartTimer final : public ServerPacket
        {
        public:
            StartTimer() : ServerPacket(SMSG_START_TIMER, 12) { }

            WorldPacket const* Write() override;

            Seconds TimeRemaining = Seconds(0);
            Seconds TotalTime = Seconds(0);
            TimerType Type = WORLD_TIMER_TYPE_PVP;
        };

        class QueryWorldCountwodnTimer final : public ClientPacket
        {
        public:
            QueryWorldCountwodnTimer(WorldPacket&& packet) : ClientPacket(CMSG_QUERY_COUNTDOWN_TIMER, std::move(packet)) { }

            void Read() override;

            TimerType Type = WORLD_TIMER_TYPE_PVP;
        };

        class SummonRaidMemberValidateFailed final : public ServerPacket
        {
        public:
            SummonRaidMemberValidateFailed() : ServerPacket(SMSG_SUMMON_RAID_MEMBER_VALIDATE_FAILED, 4) { }

            WorldPacket const* Write() override;

            struct ClientSummonRaidMemberValidateReason
            {
                ObjectGuid Member;
                int32 ReasonCode = 0;
            };

            std::vector<ClientSummonRaidMemberValidateReason> Members;
        };

        class InstanceEncounterSetAllowingRelease final : public ServerPacket
        {
        public:
            InstanceEncounterSetAllowingRelease() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_SET_ALLOWING_RELEASE, 1) { }

            WorldPacket const* Write() override;

            bool ReleaseAllowed = false;
        };

        class InstanceEncounterSetSuppressingRelease final : public ServerPacket
        {
        public:
            InstanceEncounterSetSuppressingRelease() : ServerPacket(SMSG_INSTANCE_ENCOUNTER_SET_SUPPRESSING_RELEASE, 1) { }

            WorldPacket const* Write() override;

            bool SupressinDisabled = false;
        };
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Instance::InstanceLockInfos const& lockInfos);

#endif // InstancePackets_h__
