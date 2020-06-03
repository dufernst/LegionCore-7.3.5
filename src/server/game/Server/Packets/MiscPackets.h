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

#ifndef MiscPackets_h__
#define MiscPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"
#include "WorldSession.h"
#include "Unit.h"
#include "Weather.h"
#include "CollectionMgr.h"
#include "ItemPackets.h"
#include "CUFProfile.h"
#include "EnumClassFlag.h"

namespace WorldPackets
{
    namespace Misc
    {
        class BindPointUpdate final : public ServerPacket
        {
        public:
            BindPointUpdate() : ServerPacket(SMSG_BIND_POINT_UPDATE, 20) { }

            WorldPacket const* Write() override;

            TaggedPosition<Position::XYZ> BindPosition;
            uint32 BindMapID = MAPID_INVALID;
            uint32 BindAreaID = 0;
        };

        class PlayerBound final : public ServerPacket
        {
        public:
            PlayerBound(ObjectGuid binderId, uint32 areaId) : ServerPacket(SMSG_PLAYER_BOUND, 16 + 4), BinderID(binderId), AreaID(areaId) { }

            WorldPacket const* Write() override;

            ObjectGuid BinderID;
            uint32 AreaID = 0;
        };

        class BinderConfirm final : public ServerPacket
        {
        public:
            BinderConfirm(ObjectGuid unit) : ServerPacket(SMSG_BINDER_CONFIRM, 16), Unit(unit) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
        };

        class InvalidatePlayer final : public ServerPacket
        {
        public:
            InvalidatePlayer(ObjectGuid guid) : ServerPacket(SMSG_INVALIDATE_PLAYER, 16), Guid(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
        };

        class LoginSetTimeSpeed final : public ServerPacket
        {
        public:
            LoginSetTimeSpeed() : ServerPacket(SMSG_LOGIN_SET_TIME_SPEED, 20) { }

            WorldPacket const* Write() override;

            float NewSpeed = 0.0f;
            int32 ServerTimeHolidayOffset = 0;
            uint32 GameTime = 0;
            uint32 ServerTime = 0;
            int32 GameTimeHolidayOffset = 0;
        };

        class SetSelection final : public ClientPacket
        {
        public:
            SetSelection(WorldPacket&& packet) : ClientPacket(CMSG_SET_SELECTION, std::move(packet)) { }

            void Read() override;

            ObjectGuid Selection;
        };

        class SetupCurrency final : public ServerPacket
        {
        public:
            SetupCurrency() : ServerPacket(SMSG_SETUP_CURRENCY, 13) { }

            WorldPacket const* Write() override;

            struct Record
            {
                Optional<int32> WeeklyQuantity;
                Optional<int32> MaxWeeklyQuantity;
                Optional<int32> TrackedQuantity;
                Optional<int32> MaxQuantity;
                int32 Type = 0;
                int32 Quantity = 0;
                uint8 Flags = 0;
            };

            std::vector<Record> Data;
        };

        class ViolenceLevel final : public ClientPacket
        {
        public:
            ViolenceLevel(WorldPacket&& packet) : ClientPacket(CMSG_VIOLENCE_LEVEL, std::move(packet)) { }

            void Read() override;

            int8 ViolenceLvl = -1;
        };

        class TriggerCinematic final : public ServerPacket
        {
        public:
            TriggerCinematic() : ServerPacket(SMSG_TRIGGER_CINEMATIC, 4) { }

            WorldPacket const* Write() override;

            uint32 CinematicID = 0;
        };

        class TriggerMovie final : public ServerPacket
        {
        public:
            TriggerMovie() : ServerPacket(SMSG_TRIGGER_MOVIE, 4) { }

            WorldPacket const* Write() override;

            uint32 MovieID = 0;
        };

        class UITimeRequest final : public ClientPacket
        {
        public:
            UITimeRequest(WorldPacket&& packet) : ClientPacket(CMSG_UI_TIME_REQUEST, std::move(packet)) { }

            void Read() override { }
        };

        class UITime final : public ServerPacket
        {
        public:
            UITime() : ServerPacket(SMSG_UI_TIME, 4) { }

            WorldPacket const* Write() override;

            uint32 Time = 0;
        };

        class TutorialFlags final : public ServerPacket
        {
        public:
            TutorialFlags() : ServerPacket(SMSG_TUTORIAL_FLAGS, 32)
            {
                std::memset(TutorialData, 0, sizeof TutorialData);
            }

            WorldPacket const* Write() override;

            uint32 TutorialData[MAX_ACCOUNT_TUTORIAL_VALUES];
        };

        class TutorialSetFlag final : public ClientPacket
        {
        public:
            TutorialSetFlag(WorldPacket&& packet) : ClientPacket(CMSG_TUTORIAL_FLAG, std::move(packet)) { }

            void Read() override;

            uint8 Action = 0;
            uint32 TutorialBit = 0;
        };

        class WorldServerInfo final : public ServerPacket
        {
        public:
            WorldServerInfo() : ServerPacket(SMSG_WORLD_SERVER_INFO, 26) { }

            WorldPacket const* Write() override;

            uint32 DifficultyID = 0;
            bool XRealmPvpAlert = false;
            Optional<uint32> RestrictedAccountMaxLevel;
            Optional<uint32> RestrictedAccountMaxMoney;
            uint8 IsTournamentRealm = 0;
            Optional<uint32> InstanceGroupSize;
        };

        class AreaTrigger final : public ClientPacket
        {
        public:
            AreaTrigger(WorldPacket&& packet) : ClientPacket(CMSG_AREA_TRIGGER, std::move(packet)) { }

            void Read() override;

            int32 AreaTriggerID = 0;
            bool Entered = false;
            bool FromClient = false;
        };

        class SetDungeonDifficulty final : public ClientPacket
        {
        public:
            SetDungeonDifficulty(WorldPacket&& packet) : ClientPacket(CMSG_SET_DUNGEON_DIFFICULTY, std::move(packet)) { }

            void Read() override;

            int32 DifficultyID = 0;
        };

        class SetRaidDifficulty final : public ClientPacket
        {
        public:
            SetRaidDifficulty(WorldPacket&& packet) : ClientPacket(CMSG_SET_RAID_DIFFICULTY, std::move(packet)) { }

            void Read() override;

            int32 DifficultyID = 0;
            uint8 Legacy = 0;
        };

        class DungeonDifficultySet final : public ServerPacket
        {
        public:
            DungeonDifficultySet() : ServerPacket(SMSG_SET_DUNGEON_DIFFICULTY, 4) { }

            WorldPacket const* Write() override;

            int32 DifficultyID = 0;
        };

        class RaidDifficultySet final : public ServerPacket
        {
        public:
            RaidDifficultySet() : ServerPacket(SMSG_RAID_DIFFICULTY_SET, 4 + 1) { }

            WorldPacket const* Write() override;

            int32 DifficultyID = 0;
            uint8 Legacy = 0;
        };

        class ArchaeologySurveryCast final : public ServerPacket
        {
        public:
            ArchaeologySurveryCast() : ServerPacket(SMSG_ARCHAEOLOGY_SURVERY_CAST, 13) { }

            WorldPacket const* Write() override;

            uint32 ResearchBranchID = 0;
            uint32 TotalFinds = 0;
            uint32 NumFindsCompleted = 0;
            bool SuccessfulFind = false;
        };

        class CorpseReclaimDelay final : public ServerPacket
        {
        public:
            CorpseReclaimDelay() : ServerPacket(SMSG_CORPSE_RECLAIM_DELAY, 4) { }

            WorldPacket const* Write() override;

            uint32 Remaining = 0;
        };

        class DeathReleaseLoc final : public ServerPacket
        {
        public:
            DeathReleaseLoc() : ServerPacket(SMSG_DEATH_RELEASE_LOC, 4 + 3 * 4) { }

            WorldPacket const* Write() override;

            int32 MapID = 0;
            TaggedPosition<Position::XYZ> Loc;
        };

        class PortGraveyard final : public ClientPacket
        {
        public:
            PortGraveyard(WorldPacket&& packet) : ClientPacket(CMSG_CLIENT_PORT_GRAVEYARD, std::move(packet)) { }

            void Read() override { }
        };

        class PreRessurect final : public ServerPacket
        {
        public:
            PreRessurect() : ServerPacket(SMSG_PRE_RESSURECT, 16) { }

            WorldPacket const* Write() override;

            ObjectGuid PlayerGUID;
        };

        class ReclaimCorpse final : public ClientPacket
        {
        public:
            ReclaimCorpse(WorldPacket&& packet) : ClientPacket(CMSG_RECLAIM_CORPSE, std::move(packet)) { }

            void Read() override;

            ObjectGuid CorpseGUID;
        };

        class RepopRequest final : public ClientPacket
        {
        public:
            RepopRequest(WorldPacket&& packet) : ClientPacket(CMSG_REPOP_REQUEST, std::move(packet)) { }

            void Read() override;

            bool CheckInstance = false;
        };

        class RequestCemeteryList final : public ClientPacket
        {
        public:
            RequestCemeteryList(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_CEMETERY_LIST, std::move(packet)) { }

            void Read() override { }
        };

        class RequestCemeteryListResponse final : public ServerPacket
        {
        public:
            RequestCemeteryListResponse() : ServerPacket(SMSG_REQUEST_CEMETERY_LIST_RESPONSE, 1 + 4) { }

            WorldPacket const* Write() override;

            bool IsGossipTriggered = false;
            std::vector<uint32> CemeteryID;
        };

        class ResurrectResponse final : public ClientPacket
        {
        public:
            ResurrectResponse(WorldPacket&& packet) : ClientPacket(CMSG_RESURRECT_RESPONSE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Resurrecter;
            uint32 Response = 0;
        };

        class AreaTriggerNoCorpse final : public ServerPacket
        {
        public:
            AreaTriggerNoCorpse() : ServerPacket(SMSG_AREA_TRIGGER_NO_CORPSE, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class Weather final : public ServerPacket
        {
        public:
            Weather(WeatherState weatherID, float intensity = 0.0f, bool abrupt = false) :
                ServerPacket(SMSG_WEATHER, 4 + 4 + 1), WeatherID(weatherID), Intensity(intensity), Abrupt(abrupt) { }

            WorldPacket const* Write() override;

            WeatherState WeatherID = WEATHER_STATE_FINE;
            float Intensity = 0.0f;
            bool Abrupt = false;
        };

        class StandStateChange final : public ClientPacket
        {
        public:
            StandStateChange(WorldPacket&& packet) : ClientPacket(CMSG_STAND_STATE_CHANGE, std::move(packet)) { }

            void Read() override;

            UnitStandStateType StandState = UNIT_STAND_STATE_STAND;
        };

        class StandStateUpdate final : public ServerPacket
        {
        public:
            StandStateUpdate() : ServerPacket(SMSG_STAND_STATE_UPDATE, 4 + 1) { }

            WorldPacket const* Write() override;

            uint32 AnimKitID = 0;
            UnitStandStateType State = UNIT_STAND_STATE_STAND;
        };

        class StartMirrorTimer final : public ServerPacket
        {
        public:
            StartMirrorTimer() : ServerPacket(SMSG_START_MIRROR_TIMER, 21) { }

            WorldPacket const* Write() override;

            int32 Scale = 0;
            int32 MaxValue = 0;
            int32 Timer = 0;
            int32 SpellID = 0;
            int32 Value = 0;
            bool Paused = false;
        };

        class PauseMirrorTimer final : public ServerPacket
        {
        public:
            PauseMirrorTimer(int32 timer, bool paused) : ServerPacket(SMSG_PAUSE_MIRROR_TIMER, 5), Paused(paused), Timer(timer) { }

            WorldPacket const* Write() override;

            bool Paused = true;
            int32 Timer = 0;
        };

        class StopMirrorTimer final : public ServerPacket
        {
        public:
            StopMirrorTimer(int32 timer) : ServerPacket(SMSG_STOP_MIRROR_TIMER, 4), Timer(timer) { }

            WorldPacket const* Write() override;

            int32 Timer = 0;
        };

        class ExplorationExperience final : public ServerPacket
        {
        public:
            ExplorationExperience(int32 experience, int32 areaID) : ServerPacket(SMSG_EXPLORATION_EXPERIENCE, 8), Experience(experience), AreaID(areaID) { }

            WorldPacket const* Write() override;

            int32 Experience = 0;
            int32 AreaID = 0;
        };

        class LevelUpInfo final : public ServerPacket
        {
        public:
            LevelUpInfo() : ServerPacket(SMSG_LEVEL_UP_INFO, 56) { }

            WorldPacket const* Write() override;

            std::array<int32, MAX_POWERS_PER_CLASS> PowerDelta{};
            std::array<int32, MAX_STATS> StatDelta{};
            int32 Level = 0;
            int32 HealthDelta = 0;
            int32 Cp = 0;
        };

        class PlayMusic final : public ServerPacket
        {
        public:
            PlayMusic(uint32 soundKitID) : ServerPacket(SMSG_PLAY_MUSIC, 4), SoundKitID(soundKitID) { }

            WorldPacket const* Write() override;

            uint32 SoundKitID = 0;
        };

        class RandomRollClient final : public ClientPacket
        {
        public:
            RandomRollClient(WorldPacket&& packet) : ClientPacket(CMSG_RANDOM_ROLL, std::move(packet)) { }

            void Read() override;

            int32 Min = 0;
            int32 Max = 0;
            uint8 PartyIndex = 0;
        };

        class RandomRoll final : public ServerPacket
        {
        public:
            RandomRoll() : ServerPacket(SMSG_RANDOM_ROLL, 16 + 16 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid Roller;
            ObjectGuid RollerWowAccount;
            int32 Min = 0;
            int32 Max = 0;
            int32 Result = 0;
        };

        class EnableBarberShop final : public ServerPacket
        {
        public:
            EnableBarberShop() : ServerPacket(SMSG_ENABLE_BARBER_SHOP, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        enum class PhaseShiftFlags : uint16
        {
            None = 0x0,
            Cosmetic = 0x1,
            Personal = 0x2
        };

        struct PhaseShiftDataPhase
        {
            PhaseShiftDataPhase(uint16 id, EnumClassFlag<PhaseShiftFlags> flags);
            PhaseShiftDataPhase(uint16 id);

            EnumClassFlag<PhaseShiftFlags> PhaseFlags = PhaseShiftFlags::Cosmetic; // not default
            uint16 Id = 0;
        };

        struct PhaseShiftData
        {
            std::vector<PhaseShiftDataPhase> Phases;
            ObjectGuid PersonalGUID;
            uint32 PhaseShiftFlags = 8;
        };

        class PhaseShift final : public ServerPacket
        {
        public:
            PhaseShift() : ServerPacket(SMSG_PHASE_SHIFT_CHANGE, 16 + 4 + 4 + 16 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            std::vector<uint16> PreloadMapIDs;
            std::vector<uint16> UiWorldMapAreaIDSwaps;
            std::vector<uint16> VisibleMapIDs;
            PhaseShiftData Phaseshift;
            ObjectGuid Client;
        };

        class ZoneUnderAttack final : public ServerPacket
        {
        public:
            ZoneUnderAttack() : ServerPacket(SMSG_ZONE_UNDER_ATTACK, 4) { }

            WorldPacket const* Write() override;

            int32 AreaID = 0;
        };

        class DurabilityDamageDeath final : public ServerPacket
        {
        public:
            DurabilityDamageDeath() : ServerPacket(SMSG_DURABILITY_DAMAGE_DEATH, 4) { }

            WorldPacket const* Write() override;

            int32 Percent = 0;
        };

        class ObjectUpdateFailed final : public ClientPacket
        {
        public:
            ObjectUpdateFailed(WorldPacket&& packet) : ClientPacket(CMSG_OBJECT_UPDATE_FAILED, std::move(packet)) { }

            void Read() override;

            ObjectGuid ObjectGUID;
        };

        class ObjectUpdateRescued final : public ClientPacket
        {
        public:
            ObjectUpdateRescued(WorldPacket&& packet) : ClientPacket(CMSG_OBJECT_UPDATE_RESCUED, std::move(packet)) { }

            void Read() override;

            ObjectGuid ObjectGUID;
        };

        class PlaySound final : public ServerPacket
        {
        public:
            PlaySound(ObjectGuid sourceObjectGuid, int32 soundKitID) : ServerPacket(SMSG_PLAY_SOUND, 20), SourceObjectGuid(sourceObjectGuid), SoundKitID(soundKitID) { }
            PlaySound() : ServerPacket(SMSG_PLAY_SOUND, 20) { }

            WorldPacket const* Write() override;

            ObjectGuid SourceObjectGuid;
            int32 SoundKitID = 0;
        };

        class CompleteCinematic final : public ClientPacket
        {
        public:
            CompleteCinematic(WorldPacket&& packet) : ClientPacket(CMSG_COMPLETE_CINEMATIC, std::move(packet)) { }

            void Read() override { }
        };
        
        class CompleteMovie final : public ClientPacket
        {
        public:
            CompleteMovie(WorldPacket&& packet) : ClientPacket(CMSG_COMPLETE_MOVIE, std::move(packet)) { }

            void Read() override { }
        };

        class NextCinematicCamera final : public ClientPacket
        {
        public:
            NextCinematicCamera(WorldPacket&& packet) : ClientPacket(CMSG_NEXT_CINEMATIC_CAMERA, std::move(packet)) { }

            void Read() override { }
        };

        class FarSight final : public ClientPacket
        {
        public:
            FarSight(WorldPacket&& packet) : ClientPacket(CMSG_FAR_SIGHT, std::move(packet)) { }

            void Read() override;

            bool Enable = false;
        };

        class Dismount final : public ServerPacket
        {
        public:
            Dismount(ObjectGuid guid) : ServerPacket(SMSG_DISMOUNT, 16), Guid(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
        };

        class PlayOneShotAnimKit final : public ServerPacket
        {
        public:
            PlayOneShotAnimKit() : ServerPacket(SMSG_PLAY_ONE_SHOT_ANIM_KIT, 16 + 2) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint16 AnimKitID = 0;
        };

        class SetAIAnimKit final : public ServerPacket
        {
        public:
            SetAIAnimKit() : ServerPacket(SMSG_SET_AI_ANIM_KIT, 16 + 2) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint16 AnimKitID = 0;
        };

        class SetMovementAnimKit final : public ServerPacket
        {
        public:
            SetMovementAnimKit() : ServerPacket(SMSG_SET_MOVEMENT_ANIM_KIT, 16 + 2) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint16 AnimKitID = 0;
        };

        class SetMeleeAnimKit final : public ServerPacket
        {
        public:
            SetMeleeAnimKit() : ServerPacket(SMSG_SET_MELEE_ANIM_KIT, 16 + 2) { }

            WorldPacket const* Write() override;

            ObjectGuid Unit;
            uint16 AnimKitID = 0;
        };

        class SetPlayHoverAnim final : public ServerPacket
        {
        public:
            SetPlayHoverAnim() : ServerPacket(SMSG_SET_PLAY_HOVER_ANIM, 16 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid UnitGUID;
            bool PlayHoverAnim = false;
        };

        class OpeningCinematic final : public ClientPacket
        {
        public:
            OpeningCinematic(WorldPacket&& packet) : ClientPacket(CMSG_OPENING_CINEMATIC, std::move(packet)) { }

            void Read() override { }
        };

        class TogglePvP final : public ClientPacket
        {
        public:
            TogglePvP(WorldPacket&& packet) : ClientPacket(CMSG_TOGGLE_PVP, std::move(packet)) { }

            void Read() override { }
        };

        class SetPvP final : public ClientPacket
        {
        public:
            SetPvP(WorldPacket&& packet) : ClientPacket(CMSG_SET_PVP, std::move(packet)) { }

            void Read() override;

            bool EnablePVP = false;
        };

        class SummonRequest final : public ServerPacket
        {
        public:
            SummonRequest() : ServerPacket(SMSG_SUMMON_REQUEST, 16 + 4 + 4 + 1 + 1) { }

            enum class SummonReason : uint8
            {
                SPELL = 0,
                SCENARIO = 1
            };

            WorldPacket const* Write() override;

            ObjectGuid SummonerGUID;
            uint32 SummonerVirtualRealmAddress = 0;
            int32 AreaID = 0;
            EnumClassFlag<SummonReason> Reason = SummonReason::SPELL;
            bool SkipStartingArea = false;
        };

        class SpecialMountAnim final : public ServerPacket
        {
        public:
            SpecialMountAnim(ObjectGuid guid) : ServerPacket(SMSG_SPECIAL_MOUNT_ANIM, 16), UnitGUID(guid) { }

            WorldPacket const* Write() override;

            ObjectGuid UnitGUID;
        };

        class MountSpecialAnim final : public ClientPacket
        {
        public:
            MountSpecialAnim(WorldPacket&& packet) : ClientPacket(CMSG_MOUNT_SPECIAL_ANIM, std::move(packet)) { }

            void Read() override { }
        };

        //< SMSG_SHOW_NEUTRAL_PLAYER_FACTION_SELECT_UI
        //< SMSG_RESET_WEEKLY_CURRENCY
        class NullSMsg final : public ServerPacket
        {
        public:
            NullSMsg(OpcodeServer opcode) : ServerPacket(opcode, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        struct TaskProgress
        {
            std::vector<uint16> Counts;
            time_t FailureTime = time(nullptr);
            uint32 TaskID = 0;
            uint32 Flags = 0;
        };

        class UpdateTaskProgress final : public ServerPacket
        {
        public:
            UpdateTaskProgress() : ServerPacket(SMSG_UPDATE_TASK_PROGRESS, 4) { }

            WorldPacket const* Write() override;

            std::vector<TaskProgress> Progress;
        };

        class SetAllTaskProgress final : public ServerPacket
        {
        public:
            SetAllTaskProgress() : ServerPacket(SMSG_SET_ALL_TASK_PROGRESS, 4) { }

            WorldPacket const* Write() override;

            std::vector<TaskProgress> Progress;
        };

        class StreamingMovie final : public ServerPacket
        {
        public:
            StreamingMovie() : ServerPacket(SMSG_STREAMING_MOVIES, 4) { }

            WorldPacket const* Write() override;

            std::vector<int16> MovieIDs;
        };

        class StopElapsedTimer final : public ServerPacket
        {
        public:
            StopElapsedTimer() : ServerPacket(SMSG_STOP_ELAPSED_TIMER, 5) { }

            WorldPacket const* Write() override;

            int32 TimerID = 0;
            bool KeepTimer = false;
        };

        class ShowTradeSkillResponse final : public ServerPacket
        {
        public:
            ShowTradeSkillResponse() : ServerPacket(SMSG_SHOW_TRADE_SKILL_RESPONSE, 16 + 4 + 12) { }

            WorldPacket const* Write() override;

            ObjectGuid PlayerGUID;
            uint32 SpellId = 0;
            std::vector<int32> SkillLineIDs;
            std::vector<int32> SkillRanks;
            std::vector<int32> SkillMaxRanks;
            std::vector<int32> KnownAbilitySpellIDs;
        };

        class ShowTradeSkill final : public ClientPacket
        {
        public:
            ShowTradeSkill(WorldPacket&& packet) : ClientPacket(CMSG_SHOW_TRADE_SKILL, std::move(packet)) { }

            void Read() override;

            ObjectGuid PlayerGUID;
            uint32 SpellID = 0;
            uint32 SkillLineID = 0;
        };

        class SetTaskComplete final : public ServerPacket
        {
        public:
            SetTaskComplete() : ServerPacket(SMSG_SET_TASK_COMPLETE, 4) { }

            WorldPacket const* Write() override;

            int32 TaskID = 0;
        };

        class PlayerSkinned final : public ServerPacket
        {
        public:
            PlayerSkinned() : ServerPacket(SMSG_PLAYER_SKINNED, 1) { }

            WorldPacket const* Write() override;

            bool FreeRepop = false;
        };

        class PlaySpeakerbotSound final : public ServerPacket
        {
        public:
            PlaySpeakerbotSound() : ServerPacket(SMSG_PLAY_SPEAKERBOT_SOUND, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid SourceObjectGUID;
            uint32 SoundID = 0;
        };

        class MountResult final : public ServerPacket
        {
        public:
            MountResult() : ServerPacket(SMSG_MOUNT_RESULT, 4) { }

            WorldPacket const* Write() override;

            MountResultEnum Result = MOUNT_RESULT_INVALID_MOUNT_TREE;
        };

        class DisplayGameError final : public ServerPacket
        {
        public:
            DisplayGameError() : ServerPacket(SMSG_DISPLAY_GAME_ERROR, 6) { }

            WorldPacket const* Write() override;

            UIErrors Error = UIErrors::ERR_SYSTEM;
            Optional<uint32> Arg;
            Optional<uint32> Arg2;
        };

        class DismountResult final : public ServerPacket
        {
        public:
            DismountResult() : ServerPacket(SMSG_DISMOUNT_RESULT, 4) { }

            WorldPacket const* Write() override;

            uint32 Result = 0;
        };

        class DisenchantCredit final : public ServerPacket
        {
        public:
            DisenchantCredit() : ServerPacket(SMSG_DISENCHANT_CREDIT, 4) { }

            WorldPacket const* Write() override;

            Item::ItemInstance Item;
            ObjectGuid Disenchanter;
        };

        class CustomLoadScreen final : public ServerPacket
        {
        public:
            CustomLoadScreen(uint32 teleportSpellId, uint32 loadingScreenId) : ServerPacket(SMSG_CUSTOM_LOAD_SCREEN, 8), TeleportSpellID(teleportSpellId), LoadingScreenID(loadingScreenId) { }

            WorldPacket const* Write() override;

            uint32 TeleportSpellID = 0;
            uint32 LoadingScreenID = 0;
        };

        class RespecWipeConfirm final : public ServerPacket
        {
        public:
            RespecWipeConfirm() : ServerPacket(SMSG_RESPEC_WIPE_CONFIRM, 16 + 4 + 1) { }

            WorldPacket const* Write() override;

            ObjectGuid RespecMaster;
            uint32 Cost = 0;
            RespecType respecType = RESPEC_TYPE_TALENTS;
        };

        class ConfirmRespecWipe final : public ClientPacket
        {
        public:
            ConfirmRespecWipe(WorldPacket&& packet) : ClientPacket(CMSG_CONFIRM_RESPEC_WIPE, std::move(packet)) { }

            void Read() override;

            ObjectGuid RespecMaster;
            RespecType respecType = RESPEC_TYPE_TALENTS;
        };

        class CrossedInebriationThreshold final : public ServerPacket
        {
        public:
            CrossedInebriationThreshold() : ServerPacket(SMSG_CROSSED_INEBRIATION_THRESHOLD, 16 + 4 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid Guid;
            int32 Threshold = 0;
            uint32 ItemID = 0;
        };

        class SaveCUFProfiles final : public ClientPacket
        {
        public:
            SaveCUFProfiles(WorldPacket&& packet) : ClientPacket(CMSG_SAVE_CUF_PROFILES, std::move(packet)) { }

            void Read() override;

            Array<std::unique_ptr<CUFProfile>, MAX_CUF_PROFILES> CUFProfiles;
        };

        class LoadCUFProfiles final : public ServerPacket
        {
        public:
            LoadCUFProfiles() : ServerPacket(SMSG_LOAD_CUF_PROFILES, 4) { }

            WorldPacket const* Write() override;

            std::vector<CUFProfile const*> CUFProfiles;
        };
        
        class OverrideLight final : public ServerPacket
        {
        public:
            OverrideLight() : ServerPacket(SMSG_OVERRIDE_LIGHT, 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            int32 AreaLightID = 0;
            int32 TransitionMilliseconds = 0;
            int32 OverrideLightID = 0;
        };

        class RequestResearchHistory final : public ClientPacket
        {
        public:
            RequestResearchHistory(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_RESEARCH_HISTORY, std::move(packet)) { }

            void Read() override { }
        };

        struct ResearchHistory
        {
            int32 ProjectID = 0;
            int32 CompletionCount = 0;
            time_t FirstCompleted = time_t(0);
        };

        class SetupResearchHistory final : public ServerPacket
        {
        public:
            SetupResearchHistory() : ServerPacket(SMSG_SETUP_RESEARCH_HISTORY, 4) { }

            WorldPacket const* Write() override;

            std::vector<ResearchHistory> History;
        };

        class ResearchComplete final : public ServerPacket
        {
        public:
            ResearchComplete() : ServerPacket(SMSG_RESEARCH_COMPLETE, 12) { }

            WorldPacket const* Write() override;

            ResearchHistory Research;
        };

        class ChoiceResponse final : public ClientPacket
        {
        public:
            ChoiceResponse(WorldPacket&& packet) : ClientPacket(CMSG_CHOICE_RESPONSE, std::move(packet)) { }
        
            void Read() override;

            uint32 ChoiceID = 0;
            uint32 ResponseID = 0;
        };
        
        class SetCurrency final : public ServerPacket
        {
        public:
            SetCurrency() : ServerPacket(SMSG_SET_CURRENCY, 16) { }

            WorldPacket const* Write() override;
            
            Optional<int32> TrackedQuantity;
            Optional<int32> WeeklyQuantity;
            Optional<int32> MaxQuantity;
            uint32 Flags = 0;
            int32 Quantity = 0;
            int32 Type = 0;
            bool SuppressChatLog = false;
        };

        class SetMaxWeeklyQuantity final : public ServerPacket
        {
        public:
            SetMaxWeeklyQuantity() : ServerPacket(SMSG_SET_MAX_WEEKLY_QUANTITY, 8) { }

            WorldPacket const* Write() override;
            
            uint32 Type = 0;
            int32 MaxWeeklyQuantity = 0;
        };

        class CloseInteraction final : public ClientPacket
        {
        public:
            CloseInteraction(WorldPacket&& packet) : ClientPacket(CMSG_CLOSE_INTERACTION, std::move(packet)) { }

            void Read() override;

            ObjectGuid ObjectGUID;
        };

        class AccountMountUpdate final : public ServerPacket
        {
        public:
            AccountMountUpdate() : ServerPacket(SMSG_ACCOUNT_MOUNT_UPDATE, 5) { }

            WorldPacket const* Write() override;

            bool IsFullUpdate = false;
            MountContainer const* Mounts = nullptr;
        };

        class MountSetFavorite final : public ClientPacket
        {
        public:
            MountSetFavorite(WorldPacket&& packet) : ClientPacket(CMSG_MOUNT_SET_FAVORITE, std::move(packet)) { }

            void Read() override;

            uint32 MountSpellID = 0;
            bool IsFavorite = false;
        };

        class MultiplePackets final : public ServerPacket
        {
        public:
            MultiplePackets() : ServerPacket(SMSG_MULTIPLE_PACKETS) { }

            WorldPacket const* Write() override;
            void AddPacket(WorldPacket const* packet);

            ByteBuffer Packets;
        };

        class RequestConsumptionConversionInfo final : public ClientPacket
        {
        public:
            RequestConsumptionConversionInfo(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_CONSUMPTION_CONVERSION_INFO, std::move(packet)) { }

            void Read() override;

            uint32 ID = 0;
        };

        class ContributionGetState final : public ClientPacket
        {
        public:
            ContributionGetState(WorldPacket&& packet) : ClientPacket(CMSG_CONTRIBUTION_GET_STATE, std::move(packet)) { }

            void Read() override;

            uint32 ContributionID = 0;
            uint32 ContributionGUID = 0;
        };

        class ContributionCollectorContribute final : public ClientPacket
        {
        public:
            ContributionCollectorContribute(WorldPacket&& packet) : ClientPacket(CMSG_CONTRIBUTION_CONTRIBUTE, std::move(packet)) { }

            void Read() override;

            ObjectGuid ContributionTableNpcGuid;
            uint32 OrderIndex = 0;
        };

        class ContributionResponse final : public ServerPacket
        {
        public:
            ContributionResponse() : ServerPacket(SMSG_CONTRIBUTION_RESPONSE, 12) { }

            WorldPacket const* Write() override;

            uint32 Data = 0;
            uint32 ContributionID = 0;
            uint32 ContributionGUID = 0;
        };

        class TwitterConnect final : public ClientPacket
        {
        public:
            TwitterConnect(WorldPacket&& packet) : ClientPacket(CMSG_TWITTER_CONNECT, std::move(packet)) { }

            void Read() override { }
        };

        class TwitterDisconnect final : public ClientPacket
        {
        public:
            TwitterDisconnect(WorldPacket&& packet) : ClientPacket(CMSG_TWITTER_DISCONNECT, std::move(packet)) { }

            void Read() override { }
        };

        class ResetChallengeModeCheat final : public ClientPacket
        {
        public:
            ResetChallengeModeCheat(WorldPacket&& packet) : ClientPacket(CMSG_RESET_CHALLENGE_MODE_CHEAT, std::move(packet)) { }

            void Read() override { }
        };
    }
}

#endif // MiscPackets_h__
