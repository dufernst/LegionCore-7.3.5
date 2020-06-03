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

#ifndef SpellPackets_h__
#define SpellPackets_h__

#include "Packet.h"
#include "Player.h"
#include "Spell.h"
#include "Packets/ItemPackets.h"
#include "AreaTrigger.h"
#include "CombatLogPackets.h"
#include "MovementInfo.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "SharedDefines.h"
#include <array>

namespace WorldPackets
{
    namespace Spells
    {
        class CancelAura final : public ClientPacket
        {
        public:
            CancelAura(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_AURA, std::move(packet)) { }

            void Read() override;

            ObjectGuid CasterGUID;
            int32 SpellID = 0;
        };

        class CancelAutoRepeatSpell final : public ClientPacket
        {
        public:
            CancelAutoRepeatSpell(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_AUTO_REPEAT_SPELL, std::move(packet)) { }

            void Read() override { }
        };

        class CancelChannelling final : public ClientPacket
        {
        public:
            CancelChannelling(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_CHANNELLING, std::move(packet)) { }

            void Read() override;

            int32 ChannelSpell = 0;
        };

        class CancelGrowthAura final : public ClientPacket
        {
        public:
            CancelGrowthAura(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_GROWTH_AURA, std::move(packet)) { }

            void Read() override { }
        };

        class CancelMountAura final : public ClientPacket
        {
        public:
            CancelMountAura(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_MOUNT_AURA, std::move(packet)) { }

            void Read() override { }
        };

        class RequestCategoryCooldowns final : public ClientPacket
        {
        public:
            RequestCategoryCooldowns(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_CATEGORY_COOLDOWNS, std::move(packet)) { }

            void Read() override { }
        };

        class CategoryCooldown final : public ServerPacket
        {
        public:
            struct CategoryCooldownInfo
            {
                CategoryCooldownInfo(uint32 category, int32 cooldown);

                uint32 Category = 0;
                int32 ModCooldown = 0;
            };

            CategoryCooldown() : ServerPacket(SMSG_CATEGORY_COOLDOWN, 4) { }

            WorldPacket const* Write() override;

            std::vector<CategoryCooldownInfo> CategoryCooldowns;
        };

        class SendKnownSpells final : public ServerPacket
        {
        public:
            SendKnownSpells() : ServerPacket(SMSG_SEND_KNOWN_SPELLS, 9) { }

            WorldPacket const* Write() override;

            bool InitialLogin = false;
            std::vector<uint32> KnownSpells;
            std::vector<uint32> FavoriteSpells;
        };

        class UpdateActionButtons final : public ServerPacket
        {
        public:
            UpdateActionButtons() : ServerPacket(SMSG_UPDATE_ACTION_BUTTONS, MAX_ACTION_BUTTONS * 8 + 1)
            {
                std::memset(ActionButtons, 0, sizeof ActionButtons);
            }

            WorldPacket const* Write() override;

            uint64 ActionButtons[MAX_ACTION_BUTTONS];
            uint8 Reason = 0;
        };

        class SetActionButton final : public ClientPacket
        {
        public:
            SetActionButton(WorldPacket&& packet) : ClientPacket(CMSG_SET_ACTION_BUTTON, std::move(packet)) { }

            void Read() override;

            uint32 Action = 0;
            uint32 Type = 0;
            uint8 Index = 0;
        };

        class SendUnlearnSpells final : public ServerPacket
        {
        public:
            SendUnlearnSpells() : ServerPacket(SMSG_SEND_UNLEARN_SPELLS, 4) { }

            WorldPacket const* Write() override;

            std::vector<uint32> Spells;
        };

        struct AuraDataInfo
        {
            std::vector<float> Points;
            std::vector<float> EstimatedPoints;
            Optional<SandboxScalingData> SandboxScaling;
            Optional<ObjectGuid> CastUnit;
            Optional<int32> Duration;
            Optional<int32> Remaining;
            Optional<float> TimeMod;
            ObjectGuid CastGuid;
            uint32 SpellXSpellVisualID = 0;
            uint32 ActiveFlags = 0;
            int32 SpellID = 0;
            uint16 CastLevel = 1;
            uint8 Flags = 0;
            uint8 Applications = 1;
        };

        struct AuraInfo
        {
            uint8 Slot = 0;
            Optional<AuraDataInfo> AuraData;
        };

        class AuraUpdate final : public ServerPacket
        {
        public:
            AuraUpdate() : ServerPacket(SMSG_AURA_UPDATE, 16 + 1 + 4) { }

            WorldPacket const* Write() override;

            bool UpdateAll = false;
            ObjectGuid UnitGUID;
            std::vector<AuraInfo> Auras;
        };

        struct TargetLocation
        {
            ObjectGuid Transport;
            Position Location;
        };

        struct SpellTargetData
        {
            uint32 Flags = 0;
            ObjectGuid Unit;
            ObjectGuid Item;
            Optional<TargetLocation> SrcLocation;
            Optional<TargetLocation> DstLocation;
            Optional<uint32> MapID;
            Optional<float> Orientation;
            std::string Name;
        };

        struct MissileTrajectoryRequest
        {
            float Pitch = 0.0f;
            float Speed = 0.0f;
        };

        struct SpellWeight
        {
            uint32 Type = 0;
            int32 ID = 0;
            uint32 Quantity = 0;
        };

        struct SpellCastRequest
        {
            ObjectGuid SpellGuid;
            ObjectGuid Charmer;
            SpellTargetData Target;
            MissileTrajectoryRequest MissileTrajectory;
            std::vector<SpellWeight> Weight;
            int32 SpellID = 0;
            int32 SpellXSpellVisualID = 0;
            int32 Misc[2] = { };
            uint8 SendCastFlags = 0;
            Optional<MovementInfo> MoveUpdate;
        };

        class CastSpell final : public ClientPacket
        {
        public:
            CastSpell(WorldPacket&& packet) : ClientPacket(CMSG_CAST_SPELL, std::move(packet)) { }

            void Read() override;

            SpellCastRequest Cast;
        };

        class PetCastSpell final : public ClientPacket
        {
        public:
            PetCastSpell(WorldPacket&& packet) : ClientPacket(CMSG_PET_CAST_SPELL, std::move(packet)) { }

            void Read() override;

            ObjectGuid PetGUID;
            SpellCastRequest Cast;
        };

        class ItemUse final : public ClientPacket
        {
        public:

            ItemUse(WorldPacket&& packet) : ClientPacket(CMSG_USE_ITEM, std::move(packet)) { }

            void Read() override;

            ObjectGuid itemGUID;
            SpellCastRequest Cast;
            uint8 bagIndex = 0;
            uint8 slot = 0;
        };

        struct SpellMissStatus
        {
            SpellMissStatus(uint8 reason, uint8 status) : Reason(reason), ReflectStatus(status) { }

            uint8 Reason = 0;
            uint8 ReflectStatus = 0;
        };

        struct SpellPowerData
        {
            SpellPowerData(int32 remain, uint8 power) : Remain(remain), Power(power) { }

            int32 Remain = 0;
            uint8 Power = 0;
        };

        struct RuneData
        {
            uint8 Start = 0;
            uint8 Count = 0;
            std::vector<uint8> Cooldowns;
        };

        struct MissileTrajectoryResult
        {
            uint32 TravelTime = 0;
            float Pitch = 0.0f;
        };

        struct SpellAmmo
        {
            int32 DisplayID = 0;
        };

        struct CreatureImmunities
        {
            uint32 School = 0;
            uint32 Value = 0;
        };

        struct SpellHealPrediction
        {
            ObjectGuid BeaconGUID;
            uint32 Points = 0;
            uint8 Type = 0;
        };

        struct SpellCastData
        {
            Array<ObjectGuid, 255> HitTargets;
            Array<SpellMissStatus, 255> MissStatus;
            Array<ObjectGuid, 255> MissTargets;
            std::vector<SpellPowerData> RemainingPower;
            std::vector<TargetLocation> TargetPoints;
            MissileTrajectoryResult MissileTrajectory;
            CreatureImmunities Immunities;
            SpellHealPrediction Predict;
            SpellTargetData Target;
            Optional<RuneData> RemainingRunes;
            ObjectGuid CasterGUID;
            ObjectGuid CasterUnit;
            ObjectGuid CastGuid;
            ObjectGuid CastGuid2;
            int32 SpellID = 0;
            SpellAmmo Ammo;
            uint32 SpellXSpellVisualID = 0;
            uint32 CastFlags = 0;
            uint32 CastFlagsEx = 0;
            uint32 CastTime = 0;
            uint8 DestLocSpellCastIndex = 0;
        };

        class SpellGo final : public CombatLog::CombatLogServerPacket
        {
        public:
            SpellGo() : CombatLog::CombatLogServerPacket(SMSG_SPELL_GO) { }

            WorldPacket const* Write() override;

            SpellCastData Cast;
        };

        class SpellStart final : public ServerPacket
        {
        public:
            SpellStart() : ServerPacket(SMSG_SPELL_START) { }

            WorldPacket const* Write() override;

            SpellCastData Cast;
        };

        class LearnedSpells final : public ServerPacket
        {
        public:
            LearnedSpells() : ServerPacket(SMSG_LEARNED_SPELLS, 9) { }

            WorldPacket const* Write() override;

            std::vector<int32> SpellID;
            std::vector<int32> FavoriteSpellID;
            bool SuppressMessaging = false;
        };

        class SpellFailure final : public ServerPacket
        {
        public:
            SpellFailure() : ServerPacket(SMSG_SPELL_FAILURE, 32 + 8 + 2) { }

            WorldPacket const* Write() override;

            ObjectGuid CasterUnit;
            ObjectGuid CastGUID;
            uint32 SpellXSpellVisualID = 0;
            uint32 SpellID = 0;
            uint16 Reason = 0;
        };

        class SpellFailedOther final : public ServerPacket
        {
        public:
            SpellFailedOther() : ServerPacket(SMSG_SPELL_FAILED_OTHER, 32 + 8 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid CasterUnit;
            ObjectGuid CastGUID;
            uint32 SpellID  = 0;
            uint32 SpellXSpellVisualID = 0;
            uint8 Reason = 0;
        };

        //< SMSG_CAST_FAILED
        //< SMSG_PET_CAST_FAILED
        class CastFailed final : public ServerPacket
        {
        public:
            CastFailed(OpcodeServer opcode) : ServerPacket(opcode, 4 + 4 + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid SpellGuid;
            uint32 SpellXSpellVisualID = 0;
            int32 Reason = 0;
            int32 FailedArg1 = -1;
            int32 FailedArg2 = -1;
            int32 SpellID = 0;
        };

        struct SpellModifierData
        {
            float ModifierValue = 0.0f;
            uint8 ClassIndex = 0;
        };

        struct SpellModifier
        {
            uint8 ModIndex = 0;
            std::vector<SpellModifierData> ModifierData;
        };

        class SetSpellModifier final : public ServerPacket
        {
        public:
            SetSpellModifier(OpcodeServer opcode) : ServerPacket(opcode, 20) { }

            WorldPacket const* Write() override;

            std::vector<SpellModifier> Modifiers;
        };

        class UnlearnedSpells final : public ServerPacket
        {
        public:
            UnlearnedSpells() : ServerPacket(SMSG_UNLEARNED_SPELLS, 4) { }

            WorldPacket const* Write() override;

            std::vector<uint32> SpellID;
            bool SuppressMessaging = false;
        };

        class SupercededSpells final : public ServerPacket
        {
        public:
            SupercededSpells() : ServerPacket(SMSG_SUPERCEDED_SPELLS, 12) { }

            WorldPacket const* Write() override;

            std::vector<int32> SpellID;
            std::vector<int32> Superceded;
            std::vector<int32> FavoriteSpellID;
        };

        class CooldownEvent final : public ServerPacket
        {
        public:
            CooldownEvent(bool isPet, int32 spellId) : ServerPacket(SMSG_COOLDOWN_EVENT, 16 + 4), IsPet(isPet), SpellID(spellId) { }

            WorldPacket const* Write() override;

            bool IsPet = false;
            int32 SpellID;
        };

        class ClearCooldowns final : public ServerPacket
        {
        public:
            ClearCooldowns() : ServerPacket(SMSG_CLEAR_COOLDOWNS, 4 + 1) { }

            WorldPacket const* Write() override;

            std::vector<int32> SpellID;
            bool IsPet = false;
        };

        class ClearCooldown final : public ServerPacket
        {
        public:
            ClearCooldown() : ServerPacket(SMSG_CLEAR_COOLDOWN, 1 + 4 + 1) { }

            WorldPacket const* Write() override;

            bool IsPet = false;
            int32 SpellID = 0;
            bool ClearOnHold = false;
        };

        class ModifyCooldown final : public ServerPacket
        {
        public:
            ModifyCooldown() : ServerPacket(SMSG_MODIFY_COOLDOWN, 1 + 4 + 4) { }

            WorldPacket const* Write() override;

            bool IsPet = false;
            int32 DeltaTime = 0;
            int32 SpellID = 0;
        };

        class ModifyCooldownRecoverySpeed final : public ServerPacket
        {
        public:
            ModifyCooldownRecoverySpeed() : ServerPacket(SMSG_MODIFY_COOLDOWN_RECOVERY_SPEED, 4 + 4) { }

            WorldPacket const* Write() override;

            int32 SpellID = 0;
            float Multiplier = 0.0f;
            float Multiplier2 = 0.0f;
        };

        class ModifyChargeRecoverySpeed final : public ServerPacket
        {
        public:
            ModifyChargeRecoverySpeed() : ServerPacket(SMSG_MODIFY_CHARGE_RECOVERY_SPEED, 4 + 4) { }

            WorldPacket const* Write() override;

            int32 SpellID = 0;
            float Multiplier = 0.0f;
            float Multiplier2 = 0.0f;
            bool IsPet = false;
        };

        struct SpellCooldownStruct
        {
            SpellCooldownStruct() = default;
            SpellCooldownStruct(uint32 spellId, uint32 forcedCooldown);
            SpellCooldownStruct(uint32 spellId, uint32 forcedCooldown, float modRate);

            uint32 SpellID = 0;
            uint32 ForcedCooldown = 0;
            float ModRate = 1.0f;
        };

        class SpellCooldown final : public ServerPacket
        {
        public:
            SpellCooldown() : ServerPacket(SMSG_SPELL_COOLDOWN, 4 + 16 + 1) { }

            WorldPacket const* Write() override;

            std::vector<SpellCooldownStruct> SpellCooldowns;
            ObjectGuid Caster;
            uint8 Flags = 0;
        };

        struct SpellHistoryEntry
        {
            Optional<uint32> unused622_1;
            Optional<uint32> unused622_2;
            uint32 SpellID = 0;
            uint32 ItemID = 0;
            uint32 Category = 0;
            int32 RecoveryTime = 0;
            int32 CategoryRecoveryTime = 0;
            float ModRate = 1.0f;
            bool OnHold = false;
        };

        class SendSpellHistory final : public ServerPacket
        {
        public:
            SendSpellHistory() : ServerPacket(SMSG_SEND_SPELL_HISTORY, 4) { }

            WorldPacket const* Write() override;

            std::vector<SpellHistoryEntry> Entries;
        };

        class RefreshSpellHistory final : public ServerPacket
        {
        public:
            RefreshSpellHistory() : ServerPacket(SMSG_REFRESH_SPELL_HISTORY, 4) { }

            WorldPacket const* Write() override;

            std::vector<SpellHistoryEntry> Entries;
        };

        class ClearAllSpellCharges final : public ServerPacket
        {
        public:
            ClearAllSpellCharges() : ServerPacket(SMSG_CLEAR_ALL_SPELL_CHARGES, 1) { }

            WorldPacket const* Write() override;

            bool IsPet = false;
        };

        class ClearSpellCharges final : public ServerPacket
        {
        public:
            ClearSpellCharges() : ServerPacket(SMSG_CLEAR_SPELL_CHARGES, 1 + 4) { }

            WorldPacket const* Write() override;

            bool IsPet = false;
            int32 Category = 0;
        };

        class SetSpellCharges final : public ServerPacket
        {
        public:
            SetSpellCharges() : ServerPacket(SMSG_SET_SPELL_CHARGES, 1 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 Category = 0;
            uint32 NextRecoveryTime = 0;
            float ChargeModRate = 1.0f;
            uint8 ConsumedCharges = 0;
            bool IsPet = false;
        };

        struct SpellChargeEntry
        {
            uint32 Category = 0;
            uint32 NextRecoveryTime = 0;
            float ChargeModRate = 1.0f;
            uint8 ConsumedCharges = 0;
        };

        class SendSpellCharges final : public ServerPacket
        {
        public:
            SendSpellCharges() : ServerPacket(SMSG_SEND_SPELL_CHARGES, 4) { }

            WorldPacket const* Write() override;

            std::vector<SpellChargeEntry> Entries;
        };

        class ClearTarget final : public ServerPacket
        {
        public:
            ClearTarget() : ServerPacket(SMSG_CLEAR_TARGET, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
        };

        class CancelOrphanSpellVisual final : public ServerPacket
        {
        public:
            CancelOrphanSpellVisual(int32 id) : ServerPacket(SMSG_CANCEL_ORPHAN_SPELL_VISUAL, 4), SpellVisualID(id) { }

            WorldPacket const* Write() override;

            int32 SpellVisualID = 0;
        };

        class CancelSpellVisual final : public ServerPacket
        {
        public:
            CancelSpellVisual() : ServerPacket(SMSG_CANCEL_SPELL_VISUAL, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid Source;
            int32 SpellVisualID = 0;
        };

        class CancelCast final : public ClientPacket
        {
        public:
            CancelCast(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_CAST, std::move(packet)) { }

            void Read() override;

            ObjectGuid CastGuid;
            uint32 SpellID = 0;
        };

        class OpenItem final : public ClientPacket
        {
        public:
            OpenItem(WorldPacket&& packet) : ClientPacket(CMSG_OPEN_ITEM, std::move(packet)) { }

            void Read() override;

            uint8 Slot = 0;
            uint8 PackSlot = 0;
        };

        struct SpellChannelStartInterruptImmunities
        {
            int32 SchoolImmunities = 0;
            int32 Immunities = 0;
        };

        struct SpellTargetedHealPrediction
        {
            ObjectGuid TargetGUID;
            SpellHealPrediction Predict;
        };

        class SpellChannelStart final : public ServerPacket
        {
        public:
            SpellChannelStart() : ServerPacket(SMSG_SPELL_CHANNEL_START, 4 + 16 + 4) { }

            WorldPacket const* Write() override;

            Optional<SpellChannelStartInterruptImmunities> InterruptImmunities;
            Optional<SpellTargetedHealPrediction> HealPrediction;
            ObjectGuid CasterGUID;
            int32 SpellID = 0;
            int32 SpellXSpellVisualID = 0;
            uint32 ChannelDuration = 0;
        };

        class SpellChannelUpdate final : public ServerPacket
        {
        public:
            SpellChannelUpdate() : ServerPacket(SMSG_SPELL_CHANNEL_UPDATE, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid CasterGUID;
            int32 TimeRemaining = 0;
        };

        class ResurrectRequest final : public ServerPacket
        {
        public:
            ResurrectRequest() : ServerPacket(SMSG_RESURRECT_REQUEST, 16 + 4 + 4 + 4 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid ResurrectOffererGUID;
            uint32 ResurrectOffererVirtualRealmAddress = 0;
            uint32 PetNumber = 0;
            int32 SpellID = 0;
            bool UseTimer = false;
            bool Sickness = false;
            std::string Name;
        };

        class SelfRes final : public ClientPacket
        {
        public:
            SelfRes(WorldPacket&& packet) : ClientPacket(CMSG_SELF_RES, std::move(packet)) { }

            void Read() override;
            
            int32 SpellID = 0;
        };

        class UnlearnSkill final : public ClientPacket
        {
        public:
            UnlearnSkill(WorldPacket&& packet) : ClientPacket(CMSG_UNLEARN_SKILL, std::move(packet)) { }

            void Read() override;

            uint32 SkillID = 0;
        };

        class UnlearnSpecialization final : public ClientPacket
        {
        public:
            UnlearnSpecialization(WorldPacket&& packet) : ClientPacket(CMSG_UNLEARN_SPECIALIZATION, std::move(packet)) { }

            void Read() override;

            uint8 SpecializationIndex = 0;
        };

        class GetMirrorImageData final : public ClientPacket
        {
        public:
            GetMirrorImageData(WorldPacket&& packet) : ClientPacket(CMSG_GET_MIRROR_IMAGE_DATA, std::move(packet)) { }

            void Read() override;

            ObjectGuid UnitGUID;
            uint32 DisplayID = 0;
        };

        class MirrorImageComponentedData final : public ServerPacket
        {
        public:
            MirrorImageComponentedData() : ServerPacket(SMSG_MIRROR_IMAGE_COMPONENTED_DATA, 16 * 2 + 9 + 11), CustomDisplay{} { }

            WorldPacket const* Write() override;

            ObjectGuid UnitGUID;
            ObjectGuid GuildGUID;
            std::vector<uint32> ItemDisplayID;
            uint32 DisplayID = 0;
            uint8 RaceID = 0;
            uint8 Gender = 0;
            uint8 ClassID = 0;
            uint8 SkinColor = 0;
            uint8 FaceVariation = 0;
            uint8 HairVariation = 0;
            uint8 HairColor = 0;
            uint8 BeardVariation = 0;
            std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> CustomDisplay;
        };

        class MirrorImageCreatureData final : public ServerPacket
        {
        public:
            MirrorImageCreatureData() : ServerPacket(SMSG_MIRROR_IMAGE_CREATURE_DATA, 8 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid UnitGUID;
            uint32 DisplayID = 0;
        };

        class SpellClick final : public ClientPacket
        {
        public:
            SpellClick(WorldPacket&& packet) : ClientPacket(CMSG_SPELL_CLICK, std::move(packet)) { }

            void Read() override;

            ObjectGuid SpellClickUnitGuid;
            bool TryAutoDismount = false;
        };

        class ResyncRunes final : public ServerPacket
        {
        public:
            ResyncRunes(size_t size) : ServerPacket(SMSG_RESYNC_RUNES, 4 + 2 * size) { }

            WorldPacket const* Write() override;

            std::vector<uint8> Cooldowns;
            uint8 Start = 0;
            uint8 Count = 0;
        };

        class ScriptCast final : public ServerPacket
        {
        public:
            ScriptCast(int32 spellID) : ServerPacket(SMSG_SCRIPT_CAST, 4), SpellID(spellID) { }

            WorldPacket const* Write() override;

            int32 SpellID = 0;
        };

        class PlaySpellVisual final : public ServerPacket
        {
        public:
            PlaySpellVisual() : ServerPacket(SMSG_PLAY_SPELL_VISUAL, 32 + 16 + 4 + 1 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Source;
            ObjectGuid Target;
            Position TargetPosition;
            int32 SpellVisualID = 0;
            uint16 ReflectStatus = 0;
            uint16 MissReason = 0;
            bool SpeedAsTime = false;
            float TravelSpeed = 0.0f;
        };

        class PlaySpellVisualKit final : public ServerPacket
        {
        public:
            PlaySpellVisualKit() : ServerPacket(SMSG_PLAY_SPELL_VISUAL_KIT, 16 + 12) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Unit;
            int32 KitType = 0;
            uint32 Duration = 0;
            int32 KitRecID = 0;
        };

        class PlayOrphanSpellVisual final : public ServerPacket
        {
        public:
            PlayOrphanSpellVisual() : ServerPacket(SMSG_PLAY_ORPHAN_SPELL_VISUAL, 16 + 12 * 3 + 4 + 4 + 4 + 1) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Target;
            TaggedPosition<Position::XYZ> SourceLocation;
            TaggedPosition<Position::XYZ> SourceRotation;
            TaggedPosition<Position::XYZ> TargetLocation;
            int32 SpellVisualID = 0;
            float TravelSpeed = 0.0f;
            float UnkFloat = 0.0f;
            bool SpeedAsTime = false;
        };

        class WeeklySpellUsage final : public ServerPacket
        {
        public:
            WeeklySpellUsage() : ServerPacket(SMSG_WEEKLY_SPELL_USAGE, 4) { }
            
            WorldPacket const* Write() override;

            std::vector<int32> Category;
        };

        class ResumeCastBar final : public ServerPacket
        {
        public:
            ResumeCastBar() : ServerPacket(SMSG_RESUME_CAST_BAR, 16 + 16 + 4 + 4 + 4 + 4 + 1) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Guid;
            ObjectGuid Target;
            uint32 SpellID = 0;
            uint32 SpellXSpellVisualID = 0;
            uint32 TimeRemaining = 0;
            uint32 TotalTime = 0;
            Optional<SpellChannelStartInterruptImmunities> Immunities;
        };

        class MissileTrajectoryCollision final : public ClientPacket
        {
        public:
            MissileTrajectoryCollision(WorldPacket&& packet) : ClientPacket(CMSG_MISSILE_TRAJECTORY_COLLISION, std::move(packet)) { }
        
            void Read() override;

            ObjectGuid Target;
            ObjectGuid CastGUID;
            int32 SpellID = 0;
            TaggedPosition<Position::XYZ> CollisionPos;
        };

        class NotifyMissileTrajectoryCollision final : public ServerPacket
        {
        public:
            NotifyMissileTrajectoryCollision() : ServerPacket(SMSG_NOTIFY_MISSILE_TRAJECTORY_COLLISION, 16 + 1 + 12) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Caster;
            ObjectGuid CastGUID;
            TaggedPosition<Position::XYZ> CollisionPos;
        };

        class NotifyDestLocSpellCast final : public ServerPacket
        {
        public:
            NotifyDestLocSpellCast() : ServerPacket(SMSG_NOTIFY_DEST_LOC_SPELL_CAST, 16 + 16 + 4 + 12 + 12 + 4 + 4 + 4 + 1 + 1) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Caster;
            ObjectGuid DestTransport;
            ObjectGuid CastID;
            uint32 SpellID = 0;
            TaggedPosition<Position::XYZ> SourceLoc;
            TaggedPosition<Position::XYZ> DestLoc;
            float MissileTrajectoryPitch = 0.0f;
            float MissileTrajectorySpeed = 0.0f;
            int32 TravelTime = 0;
            uint8 DestLocSpellCastIndex = 0;
        };

        struct LossOfControlInfo
        {
            Mechanics Mechanic = MECHANIC_NONE;
            uint32 Type = 0; //@TODO make controlEffect enum
            uint8 AuraSlot = 0;
            uint8 EffectIndex = 0;
        };

        class LossOfControlAuraUpdate final : public ServerPacket
        {
        public:
            LossOfControlAuraUpdate() : ServerPacket(SMSG_LOSS_OF_CONTROL_AURA_UPDATE, 4) { }
            
            WorldPacket const* Write() override;

            std::vector<LossOfControlInfo> Infos;
            ObjectGuid TargetGuid;
        };

        class CancelSpellVisualKit final : public ServerPacket
        {
        public:
            CancelSpellVisualKit() : ServerPacket(SMSG_CANCEL_SPELL_VISUAL_KIT, 16 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Source;
            int32 SpellVisualKitID = 0;
        };

        class AuraPointsDepleted final : public ServerPacket
        {
        public:
            AuraPointsDepleted() : ServerPacket(SMSG_AURA_POINTS_DEPLETED, 16 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint8 Slot = 0;
            uint8 EffectIndex = 0;
        };

        class AreaTriggerMoveScale final : public ServerPacket
        {
        public:
            AreaTriggerMoveScale() : ServerPacket(SMSG_AREA_TRIGGER_MOVE_SCALE) { }
            
            WorldPacket const* Write() override;

            ObjectGuid TriggerGUID;
            AreaTriggerSpline Spline;
        };

        class AreaTriggerSplineUnk2 final : public ServerPacket
        {
        public:
            AreaTriggerSplineUnk2() : ServerPacket(SMSG_AREA_TRIGGER_SPLINE_UNK_2, 12 + 16 + 16) { }
            
            WorldPacket const* Write() override;

            ObjectGuid TriggerGUID;
            TaggedPosition<Position::XYZO> UnkPosition;
            AreaTriggerSpline Spline;
        };

        class AreaTriggerReShape final : public ServerPacket
        {
            public:
            AreaTriggerReShape() : ServerPacket(SMSG_AREA_TRIGGER_RE_SHAPE) { }

            WorldPacket const* Write() override;

            Optional<AreaTriggerSpline> Spline;
            Optional<AreaTriggerCircle> Circle;
            ObjectGuid TriggerGUID;
        };

        class AreaTriggerSequence final : public ServerPacket
        {
            public:
            AreaTriggerSequence() : ServerPacket(SMSG_AREA_TRIGGER_SEQUENCE) { }

            WorldPacket const* Write() override;

            ObjectGuid TriggerGUID;
            int32 SequenceAnimationID = 0;
            bool SequenceEntered = false;
        };

        class AreaTriggerDenied final : public ServerPacket
        {
        public:
            AreaTriggerDenied() : ServerPacket(SMSG_AREA_TRIGGER_DENIED, 4 + 1) { }
            
            WorldPacket const* Write() override;

            int32 SequenceAnimationID = 0;
            bool SequenceEntered = false;
        };

        class SpellDelayed final : public ServerPacket
        {
        public:
            SpellDelayed() : ServerPacket(SMSG_SPELL_DELAYED, 16 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Caster;
            int32 ActualDelay = 0;
        };

        class MissileCancel final : public ServerPacket
        {
        public:
            MissileCancel() : ServerPacket(SMSG_MISSILE_CANCEL, 16 + 4 + 1) { }
            
            WorldPacket const* Write() override;

            ObjectGuid OwnerGUID;
            uint32 SpellID = 0;
            bool Reverse = false;
        };

        class DispelFailed final : public ServerPacket
        {
        public:
            DispelFailed() : ServerPacket(SMSG_DISPEL_FAILED, 32 + 4 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid VictimGUID;
            ObjectGuid CasterGUID;
            uint32 SpellID = 0;
            std::list<uint32> FailedSpellIDs;
        };

        struct DispellData
        {
            uint32 SpellID = 0;
            Optional<uint32> Rolled;
            Optional<uint32> Needed;
            bool IsHarmful = false;
        };

        class SpellDispellLog final : public ServerPacket
        {
        public:
            SpellDispellLog() : ServerPacket(SMSG_SPELL_DISPELL_LOG, 32 + 4 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid TargetGUID;
            ObjectGuid CasterGUID;
            std::vector<DispellData> Dispell;
            uint32 SpellID = 0;
            bool IsSteal = false;
            bool IsBreak = false;
        };

        class SpellAbsorbLog final : public ServerPacket
        {
        public:
            SpellAbsorbLog() : ServerPacket(SMSG_SPELL_ABSORB_LOG, 48 + 12 + 1) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Victim;
            ObjectGuid Caster;
            ObjectGuid ShieldTargetGUID;
            uint32 InterruptedSpellID = 0;
            uint32 SpellID = 0;
            int32 Absorbed = 0;
            Optional<SpellCastLogData> LogData;
        };

        class SpellCategoryCooldown final : public ServerPacket
        {
        public:
            SpellCategoryCooldown() : ServerPacket(SMSG_SPELL_CATEGORY_COOLDOWN, 21) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Caster;
            uint32 SpellID = 0;
            bool UnkBit = false;
        };

        class AddLossOfControl final : public ServerPacket
        {
        public:
            AddLossOfControl() : ServerPacket(SMSG_ADD_LOSS_OF_CONTROL, 16 + 4 + 4 + 4 + 4 + 4 + 4) { }
            
            WorldPacket const* Write() override;

            ObjectGuid Caster;
            ObjectGuid Target;
            Mechanics Mechanic = MECHANIC_NONE;
            SpellSchoolMask LockoutSchoolMask = SPELL_SCHOOL_MASK_NONE;
            LossOfControlType Type = LOC_NONE;
            uint32 SpellID = 0;
            int32 Duration = 0;
            int32 DurationRemaining = 0;
        };

        class RemoveLossOfControl final : public ServerPacket
        {
        public:
            RemoveLossOfControl() : ServerPacket(SMSG_REMOVE_LOSS_OF_CONTROL, 16 + 4 + 4) { }
            
            WorldPacket const* Write() override;
            
            ObjectGuid Caster;
            int32 SpellID = 0;
            LossOfControlType Type = LOC_NONE;
        };

        class UpdateMissileTrajectory final : public ClientPacket
        {
        public:
            UpdateMissileTrajectory(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_MISSILE_TRAJECTORY, std::move(packet)) { }
        
            void Read() override;

            Optional<MovementInfo> Status;
            ObjectGuid Guid;
            TaggedPosition<Position::XYZ> FirePos;
            TaggedPosition<Position::XYZ> ImpactPos;
            int32 SpellID = 0;
            float Pitch = 0.0f;
            float Speed = 0.0f;
            uint16 MoveMsgID = 0;
        };

        class SpellPrepare final : public ServerPacket
        {
        public:
            SpellPrepare() : ServerPacket(SMSG_SPELL_PREPARE, 32) { }
            
            WorldPacket const* Write() override;

            ObjectGuid ClientCastID;
            ObjectGuid ServerCastID;
        };

        class PushSpellToActionBar final : public ServerPacket
        {
        public:
            PushSpellToActionBar(int32 spellID) : ServerPacket(SMSG_PUSH_SPELL_TO_ACTION_BAR, 4), SpellID(spellID) { }

            WorldPacket const* Write() override;

            int32 SpellID = 0;
        };

        class SpellFailureMessage final : public ServerPacket
        {
        public:
            SpellFailureMessage(int32 spellID) : ServerPacket(SMSG_SPELL_FAILURE_MESSAGE, 4), SpellID(spellID) { }

            WorldPacket const* Write() override;

            int32 SpellID = 0;
        };

        class SpecializationChanged final : public ServerPacket
        {
        public:
            SpecializationChanged(int16 specID) : ServerPacket(SMSG_SPECIALIZATION_CHANGED, 2), SpecID(specID) { }

            WorldPacket const* Write() override;

            int16 SpecID = 0;
        }; 
        
        class DontAutoPushSpellsToActionBar final : public ServerPacket
        {
        public:
            DontAutoPushSpellsToActionBar() : ServerPacket(SMSG_DONT_AUTO_PUSH_SPELLS_TO_ACTION_BAR, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class CancelModSpeedNoControlAuras final : public ClientPacket
        {
        public:
            CancelModSpeedNoControlAuras(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_MOD_SPEED_NO_CONTROL_AURAS, std::move(packet)) { }
        
            void Read() override;

            ObjectGuid TargetGUID;
        };

        class CancelQueuedSpell final : public ClientPacket
        {
        public:
            CancelQueuedSpell(WorldPacket&& packet) : ClientPacket(CMSG_CANCEL_QUEUED_SPELL, std::move(packet)) { }

            void Read() override { }
        };

        class UpdateSpellVisual final : public ClientPacket
        {
        public:
            UpdateSpellVisual(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_SPELL_VISUAL, std::move(packet)) { }

            void Read() override;

            int32 SpellID = 0;
            int32 SpellXSpellVisualId = 0;
            ObjectGuid TargetGUID;
        };
    }
}

ByteBuffer& operator>>(ByteBuffer& buffer, WorldPackets::Spells::SpellCastRequest& request);
ByteBuffer& operator<<(ByteBuffer& data, AreaTriggerSpline const& triggerSplineData);
ByteBuffer& operator<<(ByteBuffer& data, AreaTriggerCircle const& triggerCircleData);
ByteBuffer& operator<<(ByteBuffer& data, AreaTriggerPolygon const& polygon);

#endif // SpellPackets_h__
