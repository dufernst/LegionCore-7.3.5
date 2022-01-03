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

#include "CharacterPackets.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "PlayerDefines.h"
#include "World.h"
#include "WorldSession.h"

WorldPackets::Character::EnumCharactersResult::CharacterInfo::CharacterInfo(Field* fields, CharEnumMap& charInfo, uint32 accountId)
{
    enum fieldenum
    {
        // 0       1       2        3       4       5          6            7            8            9
        f_guid, f_name, f_race, f_class, f_skin, f_face, f_hairstyle, f_haircolor, f_facialhair, f_blindfold,
        //10       11      12
        f_sex, f_tattoo, f_horn,
        // 13       14     15       16            17            18            19           20
        f_level, f_zone, f_map, f_position_x, f_position_y, f_position_z, f_guildid, f_playerFlags,
        //  21         22        23         24              25              26        27         28          29
        f_at_login, f_entry, f_modelid, f_pet_level, f_equipmentCache, f_guid_lock, f_slot, f_lastPlayed, f_specID
    };

    Guid = ObjectGuid::Create<HighGuid::Player>(fields[f_guid].GetUInt64());
    CharEnumInfoData& charEnum = charInfo[Guid];

    charEnum.Name = Name = fields[f_name].GetString();
    charEnum.Race = Race = fields[f_race].GetUInt8();
    charEnum.Class = Class = fields[f_class].GetUInt8();
    charEnum.Skin = Skin = fields[f_skin].GetUInt8();
    charEnum.Face = Face = fields[f_face].GetUInt8();
    charEnum.HairStyle = HairStyle = fields[f_hairstyle].GetUInt8();
    charEnum.HairColor = HairColor = fields[f_haircolor].GetUInt8();
    charEnum.FacialHair = FacialHair = fields[f_facialhair].GetUInt8();
    CustomDisplay[0]  = fields[f_tattoo].GetUInt8();
    CustomDisplay[1]  = fields[f_horn].GetUInt8();
    CustomDisplay[2]  = fields[f_blindfold].GetUInt8();
    charEnum.Sex = Sex = fields[f_sex].GetUInt8();
    charEnum.Level = Level = fields[f_level].GetUInt8();
    charEnum.ZoneId = ZoneId = int32(fields[f_zone].GetUInt16());
    charEnum.MapId = MapId = int32(fields[f_map].GetUInt16());
    PreLoadPosition = Position(fields[f_position_x].GetFloat(), fields[f_position_y].GetFloat(), fields[f_position_z].GetFloat());

    uint32 guildId = fields[f_guildid].GetUInt32();
    if (guildId)
        charEnum.GuildGuid = GuildGuid = ObjectGuid::Create<HighGuid::Guild>(guildId);

    uint32 playerFlags = fields[f_playerFlags].GetUInt32();
    uint32 atLoginFlags = fields[f_at_login].GetUInt16();

    if (playerFlags & PLAYER_FLAGS_HIDE_HELM)
        Flags |= CHARACTER_FLAG_HIDE_HELM;

    if (playerFlags & PLAYER_FLAGS_HIDE_CLOAK)
        Flags |= CHARACTER_FLAG_HIDE_CLOAK;

    if (playerFlags & PLAYER_FLAGS_GHOST)
        Flags |= CHARACTER_FLAG_GHOST;

    if (atLoginFlags & AT_LOGIN_RENAME)
        Flags |= CHARACTER_FLAG_RENAME;

    if (fields[f_guid_lock].GetUInt32())
        Flags |= CHARACTER_FLAG_LOCKED_BY_BILLING;

    //if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    //{
    //    if (!fields[idx++].GetString().empty())
    //        Flags |= CHARACTER_FLAG_DECLINED;
    //}else
    Flags |= CHARACTER_FLAG_DECLINED;

    if (atLoginFlags & AT_LOGIN_CUSTOMIZE)
        Flags2 = CHARACTER_FLAG_2_CUSTOMIZE;
    else if (atLoginFlags & AT_LOGIN_CHANGE_FACTION)
        Flags2 = CHARACTER_FLAG_2_FACTION;
    else if (atLoginFlags & AT_LOGIN_CHANGE_RACE)
        Flags2 = CHARACTER_FLAG_2_RACE;

    Flags3 = 0;
    Flags4 = 0;
    FirstLogin = (atLoginFlags & AT_LOGIN_FIRST) != 0;

    // show pet at selection character in character list only for non-ghost character
    if (!(playerFlags & PLAYER_FLAGS_GHOST) && (Class == CLASS_WARLOCK || Class == CLASS_HUNTER || Class == CLASS_DEATH_KNIGHT))
    {
        if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(fields[f_entry].GetUInt32()))
        {
            Pet.CreatureDisplayId = fields[f_modelid].GetUInt32();
            Pet.Level = fields[f_pet_level].GetUInt16();
            Pet.CreatureFamily = creatureInfo->Family;
        }
    }

    BoostInProgress = false;
    ProfessionIds[0] = 0;
    ProfessionIds[1] = 0;

    Tokenizer equipment(fields[f_equipmentCache].GetString(), ' ');

    ListPosition = fields[f_slot].GetUInt8();
    LastPlayedTime = fields[f_lastPlayed].GetUInt32();
    SpecializationID = fields[f_specID].GetUInt16();
    LastLoginBuild = realm.Build;

    if (!sWorld->GetCharacterInfo(Guid))
        sWorld->AddCharacterInfo(fields[f_guid].GetUInt64(), Name, Sex, Race, Class, Level, accountId, ZoneId, guildId, 0 /*rankId*/, SpecializationID);

    for (uint8 slot = 0; slot < INVENTORY_SLOT_BAG_END; ++slot)
    {
        uint32 visualBase = slot * 3;
        VisualItems[slot].InventoryType = Player::GetUInt32ValueFromArray(equipment, visualBase);
        VisualItems[slot].DisplayId = Player::GetUInt32ValueFromArray(equipment, visualBase + 1);
        VisualItems[slot].DisplayEnchantId = Player::GetUInt32ValueFromArray(equipment, visualBase + 2);
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Character::EnumCharactersResult::CharacterInfo::VisualItemInfo const& visualItem)
{
    data << uint32(visualItem.DisplayId);
    data << uint32(visualItem.DisplayEnchantId);
    data << uint8(visualItem.InventoryType);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Character::EnumCharactersResult::CharacterInfo const& charInfo)
{
    data << charInfo.Guid;
    data << uint8(charInfo.ListPosition);
    data << uint8(charInfo.Race);
    data << uint8(charInfo.Class);
    data << uint8(charInfo.Sex);
    data << uint8(charInfo.Skin);
    data << uint8(charInfo.Face);
    data << uint8(charInfo.HairStyle);
    data << uint8(charInfo.HairColor);
    data << uint8(charInfo.FacialHair);
    data.append(charInfo.CustomDisplay.data(), charInfo.CustomDisplay.size());
    data << uint8(charInfo.Level);
    data << int32(charInfo.ZoneId);
    data << int32(charInfo.MapId);
    data << charInfo.PreLoadPosition;
    data << charInfo.GuildGuid;
    data << uint32(charInfo.Flags);
    data << uint32(charInfo.Flags2);
    data << uint32(charInfo.Flags3);
    data << uint32(charInfo.Pet.CreatureDisplayId);
    data << uint32(charInfo.Pet.Level);
    data << uint32(charInfo.Pet.CreatureFamily);

    data << uint32(charInfo.ProfessionIds[0]);
    data << uint32(charInfo.ProfessionIds[1]);

    for (WorldPackets::Character::EnumCharactersResult::CharacterInfo::VisualItemInfo const& visualItem : charInfo.VisualItems)
        data << visualItem;

    data << uint32(charInfo.LastPlayedTime);
    data << uint16(charInfo.SpecializationID);
    data << uint32(charInfo.Unknown703);
    data << uint32(charInfo.LastLoginBuild);
    data << uint32(charInfo.Flags4);
    data.WriteBits(charInfo.Name.length(), 6);
    data.WriteBit(charInfo.FirstLogin);
    data.WriteBit(charInfo.BoostInProgress);
    data.WriteBits(charInfo.unkWod61x, 5);
    data.FlushBits();

    data.WriteString(charInfo.Name);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Character::EnumCharactersResult::RaceUnlock const& raceUnlock)
{
    data << int32(raceUnlock.RaceID);
    data.WriteBit(raceUnlock.HasExpansion);
    data.WriteBit(raceUnlock.HasAchievement);
    data.WriteBit(raceUnlock.HasHeritageArmor);
    data.FlushBits();

    return data;
}

WorldPacket const* WorldPackets::Character::EnumCharactersResult::Write()
{
    _worldPacket.reserve(9 + Characters.size() * sizeof(CharacterInfo) + RaceUnlockData.size() * sizeof(RaceUnlock));

    _worldPacket.WriteBit(Success);
    _worldPacket.WriteBit(IsDeletedCharacters);
    _worldPacket.WriteBit(IsDemonHunterCreationAllowed);
    _worldPacket.WriteBit(HasDemonHunterOnRealm);
    _worldPacket.WriteBit(Unknown7x);
    _worldPacket.WriteBit(DisabledClassesMask.is_initialized());
    _worldPacket.WriteBit(IsAlliedRacesCreationAllowed);
    _worldPacket << uint32(Characters.size());
    _worldPacket << int32(MaxCharacterLevel);
    _worldPacket << uint32(RaceUnlockData.size());

    if (DisabledClassesMask)
        _worldPacket << uint32(*DisabledClassesMask);

    for (CharacterInfo const& charInfo : Characters)
        _worldPacket << charInfo;

    for (RaceUnlock const& raceUnlock : RaceUnlockData)
        _worldPacket << raceUnlock;

    return &_worldPacket;
}

void WorldPackets::Character::CreateChar::Read()
{
    CreateInfo.reset(new CharacterCreateInfo());
    uint32 nameLength = _worldPacket.ReadBits(6);
    bool const hasTemplateSet = _worldPacket.ReadBit();
    _worldPacket >> CreateInfo->Race;
    _worldPacket >> CreateInfo->Class;
    _worldPacket >> CreateInfo->Sex;
    _worldPacket >> CreateInfo->Skin;
    _worldPacket >> CreateInfo->Face;
    _worldPacket >> CreateInfo->HairStyle;
    _worldPacket >> CreateInfo->HairColor;
    _worldPacket >> CreateInfo->FacialHairStyle;
    _worldPacket >> CreateInfo->OutfitId;
    _worldPacket.read(CreateInfo->CustomDisplay.data(), CreateInfo->CustomDisplay.size());
    CreateInfo->Name = _worldPacket.ReadString(nameLength);
    if (hasTemplateSet)
        CreateInfo->TemplateSet = _worldPacket.read<int32>();
}

WorldPacket const* WorldPackets::Character::CharacterCreateResponse::Write()
{
    _worldPacket << uint8(Code);

    return &_worldPacket;
}

void WorldPackets::Character::DeleteChar::Read()
{
    _worldPacket >> Guid;
}

WorldPacket const* WorldPackets::Character::CharacterDeleteResponse::Write()
{
    _worldPacket << uint8(Code);

    return &_worldPacket;
}

void WorldPackets::Character::CharacterRenameRequest::Read()
{
    RenameInfo.reset(new CharacterRenameInfo());
    _worldPacket >> RenameInfo->Guid;
    RenameInfo->NewName = _worldPacket.ReadString(_worldPacket.ReadBits(6));
}

WorldPacket const* WorldPackets::Character::CharacterRenameResult::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket.WriteBit(Guid.is_initialized());
    _worldPacket.WriteBits(Name.length(), 6);

    if (Guid)
        _worldPacket << *Guid;

    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

void WorldPackets::Character::CharRaceOrFactionChange::Read()
{
    RaceOrFactionChangeInfo.reset(new CharRaceOrFactionChangeInfo());

    RaceOrFactionChangeInfo->FactionChange = _worldPacket.ReadBit();

    uint32 nameLength = _worldPacket.ReadBits(6);

    _worldPacket >> RaceOrFactionChangeInfo->Guid;
    _worldPacket >> RaceOrFactionChangeInfo->SexID;
    _worldPacket >> RaceOrFactionChangeInfo->RaceID;
    _worldPacket >> RaceOrFactionChangeInfo->SkinID;
    _worldPacket >> RaceOrFactionChangeInfo->HairColorID;
    _worldPacket >> RaceOrFactionChangeInfo->HairStyleID;
    _worldPacket >> RaceOrFactionChangeInfo->FacialHairStyleID;
    _worldPacket >> RaceOrFactionChangeInfo->FaceID;
    _worldPacket.read(RaceOrFactionChangeInfo->CustomDisplay.data(), RaceOrFactionChangeInfo->CustomDisplay.size());
    RaceOrFactionChangeInfo->Name = _worldPacket.ReadString(nameLength);
}

WorldPacket const* WorldPackets::Character::CharFactionChangeResult::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket << Guid;
    _worldPacket.WriteBit(Display.is_initialized());
    _worldPacket.FlushBits();

    if (Display)
    {
        _worldPacket.WriteBits(Display->Name.length(), 6);
        _worldPacket << uint8(Display->SexID);
        _worldPacket << uint8(Display->SkinID);
        _worldPacket << uint8(Display->HairColorID);
        _worldPacket << uint8(Display->HairStyleID);
        _worldPacket << uint8(Display->FacialHairStyleID);
        _worldPacket << uint8(Display->FaceID);
        _worldPacket << uint8(Display->RaceID);
        _worldPacket.append(Display->CustomDisplay.data(), Display->CustomDisplay.size());
        _worldPacket.WriteString(Display->Name);
    }

    return &_worldPacket;
}

void WorldPackets::Character::GenerateRandomCharacterName::Read()
{
    _worldPacket >> Race;
    _worldPacket >> Sex;
}

WorldPacket const* WorldPackets::Character::GenerateRandomCharacterNameResult::Write()
{
    _worldPacket.WriteBit(Success);
    _worldPacket.WriteBits(Name.length(), 6);
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPackets::Character::ReorderCharacters::ReorderCharacters(WorldPacket&& packet) : ClientPacket(CMSG_REORDER_CHARACTERS, std::move(packet)) { }

void WorldPackets::Character::ReorderCharacters::Read()
{
    Entries.resize(_worldPacket.ReadBits(9));
    for (ReorderInfo& reorderInfo : Entries)
    {
        _worldPacket >> reorderInfo.PlayerGUID;
        _worldPacket >> reorderInfo.NewPosition;
    }
}

void WorldPackets::Character::UndeleteCharacter::Read()
{
    UndeleteInfo.reset(new CharacterUndeleteInfo());
    _worldPacket >> UndeleteInfo->ClientToken;
    _worldPacket >> UndeleteInfo->CharacterGuid;
}

WorldPacket const* WorldPackets::Character::UndeleteCharacterResponse::Write()
{
    ASSERT(UndeleteInfo);
    _worldPacket << int32(UndeleteInfo->ClientToken);
    _worldPacket << uint32(Result);
    _worldPacket << UndeleteInfo->CharacterGuid;
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::UndeleteCooldownStatusResponse::Write()
{
    _worldPacket.WriteBit(OnCooldown);
    _worldPacket.FlushBits();

    _worldPacket << uint32(MaxCooldown);
    _worldPacket << uint32(CurrentCooldown);

    return &_worldPacket;
}

void WorldPackets::Character::PlayerLogin::Read()
{
    _worldPacket >> Guid;
    _worldPacket >> FarClip;
}

WorldPacket const* WorldPackets::Character::LoginVerifyWorld::Write()
{
    _worldPacket << int32(MapID);
    _worldPacket << Pos;
    _worldPacket << uint32(Reason);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::CharacterLoginFailed::Write()
{
    _worldPacket << uint8(Code);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::LogoutResponse::Write()
{
    _worldPacket << int32(LogoutResult);
    _worldPacket.WriteBit(Instant);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Character::LogoutRequest::Read()
{
    IdleLogout = _worldPacket.ReadBit();
}

void WorldPackets::Character::LogoutInstant::Read()
{
    if (_worldPacket.ReadBit())
        _worldPacket >> *Reason;
}

void WorldPackets::Character::LoadingScreenNotify::Read()
{
    _worldPacket >> MapID;
    Showing = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Character::InitialSetup::Write()
{
    _worldPacket << uint8(ServerExpansionLevel);
    _worldPacket << uint8(ServerExpansionTier);

    return &_worldPacket;
}

void WorldPackets::Character::SetActionBarToggles::Read()
{
    _worldPacket >> Mask;
}

void WorldPackets::Character::RequestPlayedTime::Read()
{
    TriggerScriptEvent = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Character::PlayedTime::Write()
{
    _worldPacket << int32(TotalTime);
    _worldPacket << int32(LevelTime);
    _worldPacket.WriteBit(TriggerEvent);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Character::SetTitle::Read()
{
    _worldPacket >> TitleID;
}

void WorldPackets::Character::AlterApperance::Read()
{
    _worldPacket >> NewHairStyle;
    _worldPacket >> NewHairColor;
    _worldPacket >> NewFacialHair;
    _worldPacket >> NewSkinColor;
    _worldPacket >> NewFace;
    for (auto& itr : NewCustomDisplay)
        _worldPacket >> itr;
}

WorldPacket const* WorldPackets::Character::BarberShopResultServer::Write()
{
    _worldPacket << int32(Result);
    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::LogXPGain::Write()
{
    _worldPacket << Victim;
    _worldPacket << int32(Original);
    _worldPacket << uint8(Reason);
    _worldPacket << int32(Amount);
    _worldPacket << float(GroupBonus);
    _worldPacket << ReferAFriendBonusType;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::TitleEarned::Write()
{
    _worldPacket << uint32(Index);

    return &_worldPacket;
}

void WorldPackets::Character::SetFactionAtWar::Read()
{
    _worldPacket >> FactionIndex;
}

void WorldPackets::Character::SetFactionNotAtWar::Read()
{
    _worldPacket >> FactionIndex;
}

void WorldPackets::Character::SetFactionInactive::Read()
{
    _worldPacket >> Index;
    State = _worldPacket.ReadBit();
}

void WorldPackets::Character::SetWatchedFaction::Read()
{
    _worldPacket >> FactionIndex;
}

WorldPacket const* WorldPackets::Character::SetFactionVisible::Write()
{
    _worldPacket << FactionIndex;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::XPGainAborted::Write()
{
    _worldPacket << Victim;
    _worldPacket << XpToAdd;
    _worldPacket << XpGainReason;
    _worldPacket << XpAbortReason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::XpGainEnabled::Write()
{
    _worldPacket.WriteBit(Enabled);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::NeutralPlayerFactionSelectResult::Write()
{
    _worldPacket << NewRaceID;
    _worldPacket.WriteBit(Success);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Character::SetPlayerDeclinedNames::Read()
{
    _worldPacket >> Player;

    uint8 stringLengths[MAX_DECLINED_NAME_CASES] = { };
    for (auto& itr : stringLengths)
        itr = _worldPacket.ReadBits(7);

    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        DeclinedNames.name[i] = _worldPacket.ReadString(stringLengths[i]);
}

WorldPacket const * WorldPackets::Character::SetPlayerDeclinedNamesResult::Write()
{
    _worldPacket << ResultCode;
    _worldPacket << Player;

    return &_worldPacket;
}

void WorldPackets::Character::CharCustomize::Read()
{
    CustomizeInfo.reset(new CharCustomizeInfo());
    _worldPacket >> CustomizeInfo->CharGUID;
    _worldPacket >> CustomizeInfo->SexID;
    _worldPacket >> CustomizeInfo->SkinID;
    _worldPacket >> CustomizeInfo->HairColorID;
    _worldPacket >> CustomizeInfo->HairStyleID;
    _worldPacket >> CustomizeInfo->FacialHairStyleID;
    _worldPacket >> CustomizeInfo->FaceID;
    _worldPacket.read(CustomizeInfo->CustomDisplay.data(), CustomizeInfo->CustomDisplay.size());
    CustomizeInfo->CharName = _worldPacket.ReadString(_worldPacket.ReadBits(6));
}

WorldPackets::Character::CharCustomizeResponse::CharCustomizeResponse(CharCustomizeInfo const* info) : ServerPacket(SMSG_CHAR_CUSTOMIZE, 16 + 9 + 2)
{
    CharGUID = info->CharGUID;
    SexID = info->SexID;
    SkinID = info->SkinID;
    HairColorID = info->HairColorID;
    HairStyleID = info->HairStyleID;
    FacialHairStyleID = info->FacialHairStyleID;
    FaceID = info->FaceID;
    CharName = info->CharName;
    CustomDisplay = info->CustomDisplay;
}

WorldPacket const* WorldPackets::Character::CharCustomizeResponse::Write()
{
    _worldPacket << CharGUID;
    _worldPacket << SexID;
    _worldPacket << SkinID;
    _worldPacket << HairColorID;
    _worldPacket << HairStyleID;
    _worldPacket << FacialHairStyleID;
    _worldPacket << FaceID;
    _worldPacket.append(CustomDisplay.data(), CustomDisplay.size());
    _worldPacket.WriteBits(CharName.length(), 6);
    _worldPacket.FlushBits();
    _worldPacket.WriteString(CharName);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::CharCustomizeFailed::Write()
{
    _worldPacket << uint8(Result);
    _worldPacket << CharGUID;

    return &_worldPacket;
}

void WorldPackets::Character::NeutralPlayerSelectFaction::Read()
{
    _worldPacket >> Faction;
}

void WorldPackets::Character::SetCurrencyFlags::Read()
{
    _worldPacket >> CurrencyID;
    _worldPacket >> Flags;
}

WorldPacket const* WorldPackets::Character::FailedPlayerCondition::Write()
{
    _worldPacket << ConditionID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::CooldownCheat::Write()
{
    _worldPacket << CheatCode;
    _worldPacket << Value;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::UpdateCharacterFlags::Write()
{
    _worldPacket << Character;
    _worldPacket.WriteBit(Flags.is_initialized());
    _worldPacket.WriteBit(Flags2.is_initialized());
    _worldPacket.WriteBit(Flags3.is_initialized());
    _worldPacket.FlushBits();

    if (Flags)
        _worldPacket << *Flags;

    if (Flags2)
        _worldPacket << *Flags2;

    if (Flags3)
        _worldPacket << *Flags3;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Character::KickReason::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Reason;

    return &_worldPacket;
}

void WorldPackets::Character::EngineSurvey::Read()
{
    _worldPacket >> TotalPhysMemory;
    _worldPacket >> GPUVideoMemory;
    _worldPacket >> GPUSystemMemory;
    _worldPacket >> GPUSharedMemory;
    _worldPacket >> GPUVendorID;
    _worldPacket >> GPUModelID;
    _worldPacket >> ProcessorUnkUnk;
    _worldPacket >> ProcessorFeatures;
    _worldPacket >> ProcessorVendor;
    _worldPacket >> GXDisplayResWidth;
    _worldPacket >> GXDisplayResHeight;
    _worldPacket >> SystemOSIndex;
    _worldPacket >> GXUnk;
    _worldPacket >> ProcessorNumberOfProcessors;
    _worldPacket >> ProcessorNumberOfThreads;
    _worldPacket >> UnkDword4C;
    _worldPacket >> UnkDword50;
    _worldPacket >> Farclip;
    _worldPacket >> UnkWord58;
    _worldPacket >> UnkWord5A;
    _worldPacket >> HasHDPlayerModels;
    _worldPacket >> Is64BitSystem;
    _worldPacket >> UnkByte5E;
    _worldPacket >> UnkByte5F;
    _worldPacket >> UnkByte60;
    _worldPacket >> UnkByte61;
    _worldPacket >> UnkByte62;
    _worldPacket >> UnkByte63;
    _worldPacket >> UnkByte64;
    _worldPacket >> UnkByte65;
    _worldPacket >> UnkByte66;
    _worldPacket >> UnkByte67;
}
