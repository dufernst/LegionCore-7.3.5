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

#ifndef CharacterPackets_h__
#define CharacterPackets_h__

#include "Packet.h"
#include "Player.h"

namespace WorldPackets
{
    namespace Character
    {
        //CMSG_ENUM_CHARACTERS
        //CMSG_ENUM_CHARACTERS_DELETED_BY_CLIENT
        class EnumCharacters final : public ClientPacket
        {
        public:
            EnumCharacters(WorldPacket&& packet) : ClientPacket(std::move(packet)) { }

            void Read() override { }
        };

        struct CharacterCreateInfo
        {
            uint8 Race = RACE_NONE;
            uint8 Class = CLASS_NONE;
            uint8 Sex = GENDER_NONE;
            uint8 Skin = 0;
            uint8 Face = 0;
            uint8 HairStyle = 0;
            uint8 HairColor = 0;
            uint8 FacialHairStyle = 0;
            uint8 OutfitId = 0;
            std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> CustomDisplay;
            Optional<int32> TemplateSet;
            std::string Name;

            /// Server side data
            uint8 CharCount = 0;
            ObjectGuid guid;
        };

        struct CharacterRenameInfo
        {
            std::string NewName;
            ObjectGuid Guid;
        };

        struct CharCustomizeInfo
        {
            ObjectGuid CharGUID;
            std::string CharName;
            uint8 HairStyleID = 0;
            uint8 FaceID = 0;
            uint8 SexID = GENDER_NONE;
            uint8 HairColorID = 0;
            uint8 FacialHairStyleID = 0;
            uint8 SkinID = 0;
            std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> CustomDisplay;
        };

        struct CharRaceOrFactionChangeInfo : public CharCustomizeInfo
        {
            uint8 RaceID = RACE_NONE;
            ObjectGuid Guid;
            bool FactionChange = false;
            std::string Name;
        };

        struct CharacterUndeleteInfo
        {
            ObjectGuid CharacterGuid;
            int32 ClientToken = 0;

            /// Server side data
            std::string Name;
        };

        class EnumCharactersResult final : public ServerPacket
        {
        public:
            struct CharacterInfo
            {
                CharacterInfo(Field* fields, CharEnumMap& charInfo, uint32 accountId);

                ObjectGuid Guid;
                std::string Name;
                uint32 LastPlayedTime = 0;
                uint16 SpecializationID = 0;
                uint8 ListPosition = 0; ///< Order of the characters in list
                uint8 Race = 0;
                uint8 Class = 0;
                uint8 Sex = 0;
                uint8 Skin = 0;
                uint8 Face = 0;
                uint8 HairStyle = 0;
                uint8 HairColor = 0;
                uint8 FacialHair = 0;
                std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> CustomDisplay;
                uint8 Level = 0;
                int32 ZoneId = 0;
                int32 MapId = 0;
                TaggedPosition<Position::XYZ> PreLoadPosition;
                ObjectGuid GuildGuid;
                uint32 Flags = 0;
                uint32 Flags2 = 0;
                uint32 Flags3 = 0;
                uint32 Flags4 = 0;
                bool FirstLogin = false;
                uint8 unkWod61x = 0;
                uint32 Unknown703 = 0;
                uint32 LastLoginBuild = 0;

                struct PetInfo
                {
                    uint32 CreatureDisplayId = 0;
                    uint32 Level = 0;
                    uint32 CreatureFamily = 0;
                } Pet;

                bool BoostInProgress = false;
                int32 ProfessionIds[2] = { };

                struct VisualItemInfo
                {
                    uint32 DisplayId = 0;
                    uint32 DisplayEnchantId = 0;
                    uint8 InventoryType = 0;
                };

                VisualItemInfo VisualItems[INVENTORY_SLOT_BAG_END];
            };

            struct RaceUnlock
            {
                int32 RaceID;
                bool HasExpansion;
                bool HasAchievement;
                bool HasHeritageArmor;
            };

            EnumCharactersResult() : ServerPacket(SMSG_ENUM_CHARACTERS_RESULT, 15) { }

            WorldPacket const* Write() override;

            std::list<CharacterInfo> Characters;
            std::vector<RaceUnlock> RaceUnlockData;
            Optional<uint32> DisabledClassesMask;
            int32 MaxCharacterLevel = 1;
            bool Success = false;
            bool IsDeletedCharacters = false; 
            bool IsDemonHunterCreationAllowed = false;
            bool HasDemonHunterOnRealm = false;
            bool IsAlliedRacesCreationAllowed = false;
            bool Unknown7x = false;
        };

        class CreateChar final : public ClientPacket
        {
        public:
            CreateChar(WorldPacket&& packet) : ClientPacket(CMSG_CREATE_CHARACTER, std::move(packet)) { }

            void Read() override;

            std::shared_ptr<CharacterCreateInfo> CreateInfo;
        };

        class CharacterCreateResponse final : public ServerPacket
        {
        public:
            CharacterCreateResponse() : ServerPacket(SMSG_CREATE_CHAR, 1) { }

            WorldPacket const* Write() override;

            uint8 Code = 0;
        };

        class DeleteChar final : public ClientPacket
        {
        public:
            DeleteChar(WorldPacket&& packet) : ClientPacket(CMSG_CHAR_DELETE, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
        };

        class CharacterDeleteResponse final : public ServerPacket
        {
        public:
            CharacterDeleteResponse() : ServerPacket(SMSG_DELETE_CHAR, 1) { }

            WorldPacket const* Write() override;

            uint8 Code = 0;
        };

        class CharacterRenameRequest final : public ClientPacket
        {
        public:
            CharacterRenameRequest(WorldPacket&& packet) : ClientPacket(CMSG_CHARACTER_RENAME_REQUEST, std::move(packet)) { }

            void Read() override;

            std::shared_ptr<CharacterRenameInfo> RenameInfo;
        };

        class CharacterRenameResult final : public ServerPacket
        {
        public:
            CharacterRenameResult() : ServerPacket(SMSG_CHARACTER_RENAME_RESULT, 20) { }

            WorldPacket const* Write() override;

            std::string Name;
            uint8 Result = 0;
            Optional<ObjectGuid> Guid;
        };

        class CharRaceOrFactionChange final : public ClientPacket
        {
        public:
            CharRaceOrFactionChange(WorldPacket&& packet) : ClientPacket(CMSG_CHAR_RACE_OR_FACTION_CHANGE, std::move(packet)) { }

            void Read() override;

            std::shared_ptr<CharRaceOrFactionChangeInfo> RaceOrFactionChangeInfo;
        };

        class CharFactionChangeResult final : public ServerPacket
        {
        public:
            struct CharFactionChangeDisplayInfo
            {
                std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> CustomDisplay;
                std::string Name;
                uint8 SexID = 0;
                uint8 SkinID = 0;
                uint8 HairColorID = 0;
                uint8 HairStyleID = 0;
                uint8 FacialHairStyleID = 0;
                uint8 FaceID = 0;
                uint8 RaceID = RACE_NONE;
            };

            CharFactionChangeResult() : ServerPacket(SMSG_CHAR_FACTION_CHANGE_RESULT, 20 + sizeof(CharFactionChangeDisplayInfo)) { }

            WorldPacket const* Write() override;

            Optional<CharFactionChangeDisplayInfo> Display;
            ObjectGuid Guid;
            uint8 Result = 0;
        };

        class GenerateRandomCharacterName final : public ClientPacket
        {
        public:
            GenerateRandomCharacterName(WorldPacket&& packet) : ClientPacket(CMSG_GENERATE_RANDOM_CHARACTER_NAME, std::move(packet)) { }

            void Read() override;

            uint8 Sex = 0;
            uint8 Race = 0;
        };

        class GenerateRandomCharacterNameResult final : public ServerPacket
        {
        public:
            GenerateRandomCharacterNameResult() : ServerPacket(SMSG_GENERATE_RANDOM_CHARACTER_NAME_RESULT, 20) { }

            WorldPacket const* Write() override;

            std::string Name;
            bool Success = false;
        };

        class ReorderCharacters final : public ClientPacket
        {
        public:
            struct ReorderInfo
            {
                ObjectGuid PlayerGUID;
                uint8 NewPosition = 0;
            };

            ReorderCharacters(WorldPacket&& packet);

            void Read() override;

            Array<ReorderInfo, MAX_CHARACTERS_PER_REALM> Entries;
        };

        class UndeleteCharacter final : public ClientPacket
        {
        public:
            UndeleteCharacter(WorldPacket&& packet) : ClientPacket(CMSG_UNDELETE_CHARACTER, std::move(packet)) { }

            void Read() override;

            std::shared_ptr<CharacterUndeleteInfo> UndeleteInfo;
        };

        class UndeleteCharacterResponse final : public ServerPacket
        {
        public:
            UndeleteCharacterResponse() : ServerPacket(SMSG_UNDELETE_CHARACTER_RESPONSE, 26) { }

            WorldPacket const* Write() override;

            CharacterUndeleteInfo const* UndeleteInfo = nullptr;
            uint32 Result = 0; ///< @see enum CharacterUndeleteResult
        };

        class GetUndeleteCharacterCooldownStatus final : public ClientPacket
        {
        public:
            GetUndeleteCharacterCooldownStatus(WorldPacket&& packet) : ClientPacket(CMSG_GET_UNDELETE_CHARACTER_COOLDOWN_STATUS, std::move(packet)) { }

            void Read() override { }
        };

        class UndeleteCooldownStatusResponse final : public ServerPacket
        {
        public:
            UndeleteCooldownStatusResponse() : ServerPacket(SMSG_UNDELETE_COOLDOWN_STATUS_RESPONSE, 9) { }

            WorldPacket const* Write() override;

            bool OnCooldown = false;
            uint32 MaxCooldown = 0;
            uint32 CurrentCooldown = 0;
        };

        class PlayerLogin final : public ClientPacket
        {
        public:
            PlayerLogin(WorldPacket&& packet) : ClientPacket(CMSG_PLAYER_LOGIN, std::move(packet)) { }

            void Read() override;

            ObjectGuid Guid;
            float FarClip = 0.0f;
        };

        class LoginVerifyWorld final : public ServerPacket
        {
        public:
            LoginVerifyWorld() : ServerPacket(SMSG_LOGIN_VERIFY_WORLD, 4 + 4 * 4 + 4) { }

            WorldPacket const* Write() override;

            int32 MapID = -1;
            TaggedPosition<Position::XYZO> Pos;
            uint32 Reason = 0;
        };

        enum class LoginFailureReason : uint8
        {
            Failed                          = 0,
            NoWorld                         = 1,
            DuplicateCharacter              = 2,
            NoInstances                     = 3,
            Disabled                        = 4,
            NoCharacter                     = 5,
            LockedForTransfer               = 6,
            LockedByBilling                 = 7,
            LockedByMobileAH                = 8,
            TemporaryGMLock                 = 9,
            LockedByCharacterUpgrade        = 10,
            LockedByRevokedCharacterUpgrade = 11
        };

        class CharacterLoginFailed  final : public ServerPacket
        {
        public:
            CharacterLoginFailed(LoginFailureReason code) : ServerPacket(SMSG_CHARACTER_LOGIN_FAILED, 1), Code(code) { }

            WorldPacket const* Write() override;

            LoginFailureReason Code = LoginFailureReason::Failed;
        };

        class LogoutRequest final : public ClientPacket
        {
        public:
            LogoutRequest(WorldPacket&& packet) : ClientPacket(CMSG_LOGOUT_REQUEST, std::move(packet)) { }
            
            void Read() override;

            bool IdleLogout = false;
        };

        class LogoutResponse final : public ServerPacket
        {
        public:
            LogoutResponse() : ServerPacket(SMSG_LOGOUT_RESPONSE, 4 + 1) { }

            WorldPacket const* Write() override;

            int32 LogoutResult = 0;
            bool Instant = false;
        };

        class LogoutComplete final : public ServerPacket
        {
        public:
            LogoutComplete() : ServerPacket(SMSG_LOGOUT_COMPLETE, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class LogoutCancel final : public ClientPacket
        {
        public:
            LogoutCancel(WorldPacket&& packet) : ClientPacket(CMSG_LOGOUT_CANCEL, std::move(packet)) { }

            void Read() override { }
        };

        class LogoutInstant final : public ClientPacket
        {
        public:
            LogoutInstant(WorldPacket&& packet) : ClientPacket(CMSG_LOGOUT_INSTANT, std::move(packet)) { }

            void Read() override;

            Optional<uint32> Reason;
        };

        class LogoutCancelAck final : public ServerPacket
        {
        public:
            LogoutCancelAck() : ServerPacket(SMSG_LOGOUT_CANCEL_ACK, 0) { }

            WorldPacket const* Write() override { return &_worldPacket; }
        };

        class LoadingScreenNotify final : public ClientPacket
        {
        public:
            LoadingScreenNotify(WorldPacket&& packet) : ClientPacket(CMSG_LOADING_SCREEN_NOTIFY, std::move(packet)) { }

            void Read() override;

            int32 MapID = -1;
            bool Showing = false;
        };

        class InitialSetup final : public ServerPacket
        {
        public:
            InitialSetup() : ServerPacket(SMSG_INITIAL_SETUP, 1 + 1) { }

            WorldPacket const* Write() override;

            uint8 ServerExpansionTier = 0;
            uint8 ServerExpansionLevel = 0;
        };

        class SetActionBarToggles final : public ClientPacket
        {
        public:
            SetActionBarToggles(WorldPacket&& packet) : ClientPacket(CMSG_SET_ACTION_BAR_TOGGLES, std::move(packet)) { }

            void Read() override;

            uint8 Mask = 0;
        };

        class RequestPlayedTime final : public ClientPacket
        {
        public:
            RequestPlayedTime(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_PLAYED_TIME, std::move(packet)) { }

            void Read() override;

            bool TriggerScriptEvent = false;
        };

        class PlayedTime final : public ServerPacket
        {
        public:
            PlayedTime() : ServerPacket(SMSG_PLAYED_TIME, 9) { }

            WorldPacket const* Write() override;

            int32 TotalTime = 0;
            int32 LevelTime = 0;
            bool TriggerEvent = false;
        };

        class SetTitle final : public ClientPacket
        {
        public:
            SetTitle(WorldPacket&& packet) : ClientPacket(CMSG_SET_TITLE, std::move(packet)) { }

            void Read() override;

            int32 TitleID = 0;
        };

        class AlterApperance final : public ClientPacket
        {
        public:
            AlterApperance(WorldPacket&& packet) : ClientPacket(CMSG_ALTER_APPEARANCE, std::move(packet)), NewCustomDisplay{} { }

            void Read() override;

            uint32 NewHairStyle = 0;
            uint32 NewHairColor = 0;
            uint32 NewFacialHair = 0;
            uint32 NewSkinColor = 0;
            uint32 NewFace = 0;
            std::array<uint32, PLAYER_CUSTOM_DISPLAY_SIZE> NewCustomDisplay;
        };

        class BarberShopResultServer final : public ServerPacket
        {
        public:
            BarberShopResultServer(BarberShopResult result) : ServerPacket(SMSG_BARBER_SHOP_RESULT, 4), Result(result) { }

            WorldPacket const* Write() override;

            BarberShopResult Result = BARBER_SHOP_RESULT_SUCCESS;
        };

        class LogXPGain final : public ServerPacket
        {
        public:
            LogXPGain() : ServerPacket(SMSG_LOG_XP_GAIN, 30) { }

            WorldPacket const* Write() override;

            ObjectGuid Victim;
            int32 Original = 0;
            uint8 Reason = 0;
            int32 Amount = 0;
            float GroupBonus = 0.0f;
            uint8 ReferAFriendBonusType = 0;    // 1 - 300% of normal XP; 2 - 150% of normal XP
        };

        class TitleEarned final : public ServerPacket
        {
        public:
            TitleEarned(OpcodeServer opcode) : ServerPacket(opcode, 4) { }

            WorldPacket const* Write() override;

            uint32 Index = 0;
        };

        class SetFactionAtWar final : public ClientPacket
        {
        public:
            SetFactionAtWar(WorldPacket&& packet) : ClientPacket(CMSG_SET_FACTION_AT_WAR, std::move(packet)) { }

            void Read() override;

            uint8 FactionIndex = 0;
        };

        class SetFactionNotAtWar final : public ClientPacket
        {
        public:
            SetFactionNotAtWar(WorldPacket&& packet) : ClientPacket(CMSG_SET_FACTION_NOT_AT_WAR, std::move(packet)) { }

            void Read() override;

            uint8 FactionIndex = 0;
        };

        class SetFactionInactive final : public ClientPacket
        {
        public:
            SetFactionInactive(WorldPacket&& packet) : ClientPacket(CMSG_SET_FACTION_INACTIVE, std::move(packet)) { }

            void Read() override;

            uint32 Index = 0;
            bool State = false;
        };

        class SetWatchedFaction final : public ClientPacket
        {
        public:
            SetWatchedFaction(WorldPacket&& packet) : ClientPacket(CMSG_SET_WATCHED_FACTION, std::move(packet)) { }

            void Read() override;

            int32 FactionIndex = 0;
        };

        class SetFactionVisible final : public ServerPacket
        {
        public:
            SetFactionVisible(bool visible) : ServerPacket(visible ? SMSG_SET_FACTION_VISIBLE : SMSG_SET_FACTION_NOT_VISIBLE, 4) { }

            WorldPacket const* Write() override;

            uint32 FactionIndex = 0;
        };

        class XPGainAborted final : public ServerPacket
        {
        public:
            XPGainAborted() : ServerPacket(SMSG_XP_GAIN_ABORTED, 16 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid Victim;
            int32 XpToAdd = 0;
            int32 XpGainReason = 0;
            int32 XpAbortReason = 0;
        };

        class XpGainEnabled final : public ServerPacket
        {
        public:
            XpGainEnabled(bool enabled) : ServerPacket(SMSG_XP_GAIN_ENABLED, 1) { Enabled = enabled; }

            WorldPacket const* Write() override;

            bool Enabled = false;
        };

        class NeutralPlayerFactionSelectResult final : public ServerPacket
        {
        public:
            NeutralPlayerFactionSelectResult() : ServerPacket(SMSG_NEUTRAL_PLAYER_FACTION_SELECT_RESULT, 4 + 1) { }

            WorldPacket const* Write() override;

            uint32 NewRaceID = 0;
            bool Success = false;
        };

        class EmoteClient final : public ClientPacket
        {
        public:
            EmoteClient(WorldPacket&& packet) : ClientPacket(CMSG_EMOTE, std::move(packet)) { }

            void Read() override { }
        };

        class SetPlayerDeclinedNames final : public ClientPacket
        {
        public:
            SetPlayerDeclinedNames(WorldPacket&& packet) : ClientPacket(CMSG_SET_PLAYER_DECLINED_NAMES, std::move(packet)) { }
        
            void Read() override;

            ObjectGuid Player;
            DeclinedName DeclinedNames;
        };

        class SetPlayerDeclinedNamesResult final : public ServerPacket
        {
        public:
            SetPlayerDeclinedNamesResult() : ServerPacket(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 16 + 4) { }

            WorldPacket const* Write() override;

            ObjectGuid Player;
            int32 ResultCode = 0;
        };

        class CharCustomize final : public ClientPacket
        {
        public:
            CharCustomize(WorldPacket&& packet) : ClientPacket(CMSG_CHAR_CUSTOMIZE, std::move(packet)) { }

            void Read() override;

            std::shared_ptr<CharCustomizeInfo> CustomizeInfo;
        };

        class CharCustomizeResponse final : public ServerPacket
        {
        public:
            CharCustomizeResponse() : ServerPacket(SMSG_CHAR_CUSTOMIZE, 16 + 9 + 2), CustomDisplay{} { }
            explicit CharCustomizeResponse(CharCustomizeInfo const* customizeInfo);

            WorldPacket const* Write() override;

            ObjectGuid CharGUID;
            std::string CharName;
            uint8 SexID = 0;
            uint8 SkinID = 0;
            uint8 HairColorID = 0;
            uint8 HairStyleID = 0;
            uint8 FacialHairStyleID = 0;
            uint8 FaceID = 0;
            std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> CustomDisplay;
        };

        class CharCustomizeFailed final : public ServerPacket
        {
        public:
            CharCustomizeFailed() : ServerPacket(SMSG_CHAR_CUSTOMIZE_FAILED, 1 + 16) { }

            WorldPacket const* Write() override;

            ObjectGuid CharGUID;
            ResponseCodes Result = RESPONSE_SUCCESS;
        };

        class NeutralPlayerSelectFaction final : public ClientPacket
        {
        public:
            NeutralPlayerSelectFaction(WorldPacket&& packet) : ClientPacket(CMSG_NEUTRAL_PLAYER_SELECT_FACTION, std::move(packet)) { }

            void Read() override;

            uint32 Faction = 0;
        };

        class SetCurrencyFlags final : public ClientPacket
        {
        public:
            SetCurrencyFlags(WorldPacket&& packet) : ClientPacket(CMSG_SET_CURRENCY_FLAGS, std::move(packet)) { }

            void Read() override;

            uint32 CurrencyID = 0;
            uint32 Flags = 0;
        };
        
        class FailedPlayerCondition final : public ServerPacket
        {
        public:
            FailedPlayerCondition(int32 conditionID) : ServerPacket(SMSG_FAILED_PLAYER_CONDITION, 4), ConditionID(conditionID) { }

            WorldPacket const* Write() override;

            int32 ConditionID = 0;
        };
        
        class CooldownCheat final : public ServerPacket
        {
        public:
            CooldownCheat() : ServerPacket(SMSG_COOLDOWN_CHEAT, 8) { }

            WorldPacket const* Write() override;

            int32 CheatCode = 0;
            float Value = 0.0f;
        };

        class UpdateCharacterFlags final : public ServerPacket
        {
        public:
            UpdateCharacterFlags() : ServerPacket(SMSG_UPDATE_CHARACTER_FLAGS, 16 + 12) { }

            WorldPacket const* Write() override;

            ObjectGuid Character;
            Optional<uint32> Flags;
            Optional<uint32> Flags2;
            Optional<uint32> Flags3;
        };

        class KickReason final : public ServerPacket
        {
        public:
            KickReason() : ServerPacket(SMSG_KICK_REASON, 5) { }

            WorldPacket const* Write() override;

            int32 UnkInt = 0;
            uint8 Reason = 0;
        };

        class EngineSurvey final : public ClientPacket
        {
        public:
            EngineSurvey(WorldPacket&& packet) : ClientPacket(CMSG_ENGINE_SURVEY, std::move(packet)) { }

            void Read() override;

            uint64 TotalPhysMemory = 0;
            uint64 GPUVideoMemory = 0;
            uint64 GPUSystemMemory = 0;
            uint64 GPUSharedMemory = 0;
            uint32 GPUVendorID = 0;
            uint32 GPUModelID = 0;
            uint32 ProcessorUnkUnk = 0;
            uint32 ProcessorFeatures = 0;
            uint32 ProcessorVendor = 0;
            uint32 ProcessorNumberOfProcessors = 0;
            uint32 ProcessorNumberOfThreads = 0;
            uint32 GXDisplayResWidth = 0;
            uint32 GXDisplayResHeight = 0;
            uint32 GXUnk = 0;
            uint32 SystemOSIndex = 0;
            uint32 UnkDword4C = 0;
            uint32 UnkDword50 = 0;
            uint32 Farclip = 0;
            uint16 UnkWord58 = 0;
            uint16 UnkWord5A = 0;
            uint8 HasHDPlayerModels = 0;
            uint8 Is64BitSystem = 0;
            uint8 UnkByte5E = 0;
            uint8 UnkByte5F = 0;
            uint8 UnkByte60 = 0;
            uint8 UnkByte61 = 0;
            uint8 UnkByte62 = 0;
            uint8 UnkByte63 = 0;
            uint8 UnkByte64 = 0;
            uint8 UnkByte65 = 0;
            uint8 UnkByte66 = 0;
            uint8 UnkByte67 = 0;
        };
    }
}

#endif // CharacterPackets_h__
