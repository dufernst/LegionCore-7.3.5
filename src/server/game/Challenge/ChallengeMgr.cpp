/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "ChallengeMgr.h"
#include "DatabaseEnvFwd.h"
#include "Challenge.h"
#include "DatabaseEnv.h"
#include "GameEventMgr.h"

static uint32 stepLeveling[3][16]
{
    // Step 7.0.3 - 7.1.5 start 845
    // 0    1    2    3    4    5    6     7     8     9    10     11    12    13    14    15
    { 0, 0, 0, 0, 5, 5, 10, 10, 15, 15, 20, 25, 25, 30, 35, 40 },
    // Step 7.2.0 - 7.2.5 start 870
    // 0    1    2    3    4    5    6     7     8     9    10     11    12    13    14    15
    { 0, 0, 0, 0, 5, 5, 10, 10, 15, 15, 20, 20, 25, 30, 35, 40 },
    // Step 7.3.0 - 7.3.5 start 890
    // 0    1    2    3    4    5    6     7     8     9    10     11    12    13    14    15
    { 0, 0, 0, 0, 5, 5, 10, 15, 20, 20, 25, 30, 35, 40, 45, 50 }
};

static uint32 stepOplotLeveling[3][16]
{
    // Step 7.0.3 - 7.1.5 start 845
    // 0    1    2     3    4     5     6     7     8     9     10    11    12    13    14    15
    { 0, 0, 5, 10, 15, 20, 20, 25, 25, 30, 35, 35, 40, 45, 50, 55 },
    // Step 7.2.0 - 7.2.5 start 870
    // 0    1    2     3    4     5     6     7     8     9     10    11    12    13    14    15
    { 0, 0, 5, 10, 15, 20, 20, 25, 25, 30, 35, 40, 45, 50, 55, 60 },
    // Step 7.3.0 - 7.3.5 start 890
    // 0    1    2     3    4     5     6     7     8     9     10    11    12    13    14    15
    { 0, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70 }
};

bool ChallengeMember::operator<(const ChallengeMember& i) const
{
    return guid.GetCounter() > i.guid.GetCounter();
}

bool ChallengeMember::operator==(const ChallengeMember& i) const
{
    return guid.GetCounter() == i.guid.GetCounter();
}

ChallengeMgr::ChallengeMgr() = default;

ChallengeMgr::~ChallengeMgr()
{
    for (auto v : _challengeMap)
        delete v.second;

    _challengeMap.clear();
    _challengesOfMember.clear();
    _bestForMap.clear();
}

ChallengeMgr* ChallengeMgr::instance()
{
    static ChallengeMgr instance;
    return &instance;
}

void ChallengeMgr::CheckBestMapId(ChallengeData* challengeData)
{
    if (!challengeData)
        return;

    if (!_bestForMap[challengeData->ChallengeID] || _bestForMap[challengeData->ChallengeID]->RecordTime > challengeData->RecordTime)
        _bestForMap[challengeData->ChallengeID] = challengeData;
}

void ChallengeMgr::CheckBestGuildMapId(ChallengeData* challengeData)
{
    if (!challengeData || !challengeData->GuildID)
        return;

    if (!m_GuildBest[challengeData->GuildID][challengeData->ChallengeID] || m_GuildBest[challengeData->GuildID][challengeData->ChallengeID]->RecordTime > challengeData->RecordTime)
        m_GuildBest[challengeData->GuildID][challengeData->ChallengeID] = challengeData;
}

bool ChallengeMgr::CheckBestMemberMapId(ObjectGuid const& guid, ChallengeData* challengeData)
{
    bool isBest = false;
    if (!_challengesOfMember[guid][challengeData->ChallengeID] || _challengesOfMember[guid][challengeData->ChallengeID]->RecordTime > challengeData->RecordTime)
    {
        _challengesOfMember[guid][challengeData->ChallengeID] = challengeData;
        isBest = true;
    }

    if (!_lastForMember[guid][challengeData->ChallengeID] || _lastForMember[guid][challengeData->ChallengeID]->Date < challengeData->Date)
        _lastForMember[guid][challengeData->ChallengeID] = challengeData;

    _challengeWeekList[guid].insert(challengeData);

    return isBest;
}

void ChallengeMgr::SaveChallengeToDB(ChallengeData const* challengeData)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHALLENGE);
    stmt->setUInt32(0, challengeData->ID);
    stmt->setUInt64(1, challengeData->GuildID);
    stmt->setUInt16(2, challengeData->MapID);
    stmt->setUInt32(3, challengeData->RecordTime);
    stmt->setUInt32(4, challengeData->Date);
    stmt->setUInt8(5, challengeData->ChallengeLevel);
    stmt->setUInt8(6, challengeData->TimerLevel);
    std::ostringstream affixesListIDs;
    for (uint16 affixe : challengeData->Affixes)
        if (affixe)
            affixesListIDs << affixe << ' ';
    stmt->setString(7, affixesListIDs.str());
    stmt->setUInt32(8, challengeData->ChestID);
    trans->Append(stmt);

    for (auto const& v : challengeData->member)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHALLENGE_MEMBER);
        stmt->setUInt32(0, challengeData->ID);
        stmt->setUInt64(1, v.guid.GetCounter());
        stmt->setUInt16(2, v.specId);
        stmt->setUInt32(3, v.ChallengeLevel);
        stmt->setUInt32(4, v.Date);
        stmt->setUInt32(5, v.ChestID);
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);
}

void ChallengeMgr::LoadFromDB()
{
    if (QueryResult result = CharacterDatabase.Query("SELECT `ID`, `GuildID`, `MapID`, `RecordTime`, `Date`, `ChallengeLevel`, `TimerLevel`, `Affixes`, `ChestID`, `ChallengeID` FROM `challenge`"))
    {
        do
        {
            Field* fields = result->Fetch();

            auto challengeData = new ChallengeData;
            challengeData->ID = fields[0].GetUInt64();
            challengeData->GuildID = fields[1].GetUInt64();
            challengeData->MapID = fields[2].GetUInt16();
            challengeData->RecordTime = fields[3].GetUInt32();
            if (challengeData->RecordTime < 10000)
                challengeData->RecordTime *= IN_MILLISECONDS;
            challengeData->Date = fields[4].GetUInt32();
            challengeData->ChallengeLevel = fields[5].GetUInt8();
            challengeData->TimerLevel = fields[6].GetUInt8();
            challengeData->ChestID = fields[8].GetUInt32();
            challengeData->ChallengeID = fields[9].GetUInt16();
            if (!challengeData->ChallengeID)
                if (MapChallengeModeEntry const* challengeEntry = sDB2Manager.GetChallengeModeByMapID(challengeData->MapID))
                    challengeData->ChallengeID = challengeEntry->ID;

            challengeData->Affixes.fill(0);

            uint8 i = 0;
            Tokenizer affixes(fields[7].GetString(), ' ');
            for (auto& affix : affixes)
                challengeData->Affixes[i] = atoul(affix);

            _challengeMap[challengeData->ID] = challengeData;
            CheckBestMapId(challengeData);
            CheckBestGuildMapId(challengeData);

        } while (result->NextRow());
    }

    if (QueryResult result = CharacterDatabase.Query("SELECT `id`, `member`, `specID`, `ChallengeLevel`, `Date`, `ChestID` FROM `challenge_member`"))
    {
        do
        {
            Field* fields = result->Fetch();
            ChallengeMember member;
            member.guid = ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64());
            member.specId = fields[2].GetUInt16();
            member.ChallengeLevel = fields[3].GetUInt32();
            member.Date = fields[4].GetUInt32();
            member.ChestID = fields[5].GetUInt32();

            auto itr = _challengeMap.find(fields[0].GetUInt64());
            if (itr == _challengeMap.end())
                continue;

            itr->second->member.insert(member);
            CheckBestMemberMapId(member.guid, itr->second);
        } while (result->NextRow());
    }

    for (auto v : _challengeMap)
        if (v.second->member.empty())
            CharacterDatabase.PQuery("DELETE FROM `challenge` WHERE `ID` = '%u';", v.first);

    if (QueryResult result = CharacterDatabase.Query("SELECT `guid`, `chestListID`, `date`, `ChallengeLevel` FROM `challenge_oplote_loot`"))
    {
        do
        {
            Field* fields = result->Fetch();
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64());
            OploteLoot& lootOplote = _oploteWeekLoot[guid];
            lootOplote.Date = fields[2].GetUInt32();
            lootOplote.ChallengeLevel = fields[3].GetUInt32();
            lootOplote.needSave = false;
            lootOplote.guid = guid;

            Tokenizer chestLists(fields[1].GetString(), ' ');
            for (auto& chestList : chestLists)
                lootOplote.chestListID.insert(atoul(chestList));

        } while (result->NextRow());
    }

    if (sWorld->getWorldState(WS_CHALLENGE_AFFIXE1_RESET_TIME) == 0)
        GenerateCurrentWeekAffixes();

	if ((sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX1) > 0 && sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX1) < 15) &&
		(sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX2) > 0 && sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX2) < 15) &&
		(sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX3) > 0 && sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX3) < 15))
		GenerateManualAffixes();
}

ChallengeData* ChallengeMgr::BestServerChallenge(uint16 ChallengeID)
{
    return Trinity::Containers::MapGetValuePtr(_bestForMap, ChallengeID);
}

ChallengeData* ChallengeMgr::BestGuildChallenge(ObjectGuid::LowType const& GuildID, uint16 ChallengeID)
{
    if (!GuildID)
        return nullptr;

    auto itr = m_GuildBest.find(GuildID);
    if (itr == m_GuildBest.end())
        return nullptr;

    return Trinity::Containers::MapGetValuePtr(itr->second, ChallengeID);
}

void ChallengeMgr::SetChallengeMapData(ObjectGuid::LowType const& ID, ChallengeData* data)
{
    _challengeMap[ID] = data;
}

ChallengeByMap* ChallengeMgr::BestForMember(ObjectGuid const& guid)
{
   return Trinity::Containers::MapGetValuePtr(_challengesOfMember, guid);
}

ChallengeByMap* ChallengeMgr::LastForMember(ObjectGuid const& guid)
{
    return Trinity::Containers::MapGetValuePtr(_lastForMember, guid);
}

ChallengeData* ChallengeMgr::LastForMemberMap(ObjectGuid const& guid, uint32 ChallengeID)
{
    if (ChallengeByMap* _lastResalt = LastForMember(guid))
    {
        auto itr = _lastResalt->find(ChallengeID);
        if (itr != _lastResalt->end())
            return itr->second;
    }

    return nullptr;
}

ChallengeData* ChallengeMgr::BestForMemberMap(ObjectGuid const& guid, uint32 ChallengeID)
{
    if (ChallengeByMap* _lastResalt = BestForMember(guid))
    {
        auto itr = _lastResalt->find(ChallengeID);
        if (itr != _lastResalt->end())
            return itr->second;
    }

    return nullptr;
}

void ChallengeMgr::GenerateCurrentWeekAffixes()
{
    uint32 affixes[12][3] =
    {
        { Raging, Volcanic, Tyrannical},
        { Teeming, FelExplosives, Fortified},
        { Bolstering, Grievous, Tyrannical},
        { Sanguine, Volcanic, Fortified},
        { Bursting, Skittish, Tyrannical},
        { Teeming, Quaking, Fortified},
        { Raging, Necrotic, Tyrannical},
        { Bolstering, Skittish, Fortified},
        { Teeming, Volcanic, Tyrannical},
        { Sanguine, Grievous, Fortified},
        { Bolstering, FelExplosives, Tyrannical},
        { Bursting, Quaking, Fortified},
    };

    auto weekContainer = affixes[GetActiveAffixe()];

    sWorld->setWorldState(WS_CHALLENGE_AFFIXE1_RESET_TIME, weekContainer[0]);
    sWorld->setWorldState(WS_CHALLENGE_AFFIXE2_RESET_TIME, weekContainer[1]);
    sWorld->setWorldState(WS_CHALLENGE_AFFIXE3_RESET_TIME, weekContainer[2]);
}

void ChallengeMgr::GenerateManualAffixes()
{
	sWorld->setWorldState(WS_CHALLENGE_AFFIXE1_RESET_TIME, sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX1));
	sWorld->setWorldState(WS_CHALLENGE_AFFIXE2_RESET_TIME, sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX2));
	sWorld->setWorldState(WS_CHALLENGE_AFFIXE3_RESET_TIME, sWorld->getIntConfig(CONFIG_CHALLENGE_MANUAL_AFFIX3));
}

uint8 ChallengeMgr::GetActiveAffixe()
{
    if (sGameEventMgr->IsActiveEvent(126))
        return 0;
    if (sGameEventMgr->IsActiveEvent(127))
        return 1;
    if (sGameEventMgr->IsActiveEvent(128))
        return 2;
    if (sGameEventMgr->IsActiveEvent(129))
        return 3;
    if (sGameEventMgr->IsActiveEvent(130))
        return 4;
    if (sGameEventMgr->IsActiveEvent(131))
        return 5;
    if (sGameEventMgr->IsActiveEvent(132))
        return 6;
    if (sGameEventMgr->IsActiveEvent(133))
        return 7;
    if (sGameEventMgr->IsActiveEvent(134))
        return 8;
    if (sGameEventMgr->IsActiveEvent(135))
        return 9;
    if (sGameEventMgr->IsActiveEvent(136))
        return 10;
    if (sGameEventMgr->IsActiveEvent(137))
        return 11;

    return 0;
}

bool ChallengeMgr::HasOploteLoot(ObjectGuid const& guid)
{
    return Trinity::Containers::MapGetValuePtr(_oploteWeekLoot, guid);
}

OploteLoot* ChallengeMgr::GetOploteLoot(ObjectGuid const& guid)
{
    return Trinity::Containers::MapGetValuePtr(_oploteWeekLoot, guid);
}

void ChallengeMgr::SaveOploteLootToDB()
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    for (auto const& v : _oploteWeekLoot)
    {
        if (v.second.needSave)
        {
            auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHALLENGE_OPLOTE_LOOT);
            stmt->setUInt32(0, v.second.guid.GetCounter());
            std::ostringstream chestLists;
            for (uint32 chestList : v.second.chestListID)
                if (chestList)
                    chestLists << chestList << ' ';
            stmt->setString(1, chestLists.str());
            stmt->setUInt32(2, v.second.Date);
            stmt->setUInt32(3, v.second.ChallengeLevel);
            trans->Append(stmt);
        }
    }
    CharacterDatabase.CommitTransaction(trans);
}

void ChallengeMgr::DeleteOploteLoot(ObjectGuid const& guid)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHALLENGE_OPLOTE_LOOT_BY_GUID);
    stmt->setUInt32(0, guid.GetCounter());
    CharacterDatabase.Execute(stmt);

    _oploteWeekLoot.erase(guid);
}

void ChallengeMgr::GenerateOploteLoot(bool manual)
{
    TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "GenerateOploteLoot manual %u _challengeWeekList %u", manual, _challengeWeekList.size());

    CharacterDatabase.Query("DELETE FROM challenge_oplote_loot WHERE date <= UNIX_TIMESTAMP()");
    _oploteWeekLoot.clear();

    for (auto const& c : _challengeWeekList)
    {
        for (auto const& v : c.second)
        {
            if (manual && (v->Date > sWorld->getWorldState(WS_CHALLENGE_LAST_RESET_TIME) || v->Date < (sWorld->getWorldState(WS_CHALLENGE_LAST_RESET_TIME) - (7 * DAY))))
                continue;

            if (!manual && (v->Date > sWorld->getWorldState(WS_CHALLENGE_KEY_RESET_TIME) || v->Date < sWorld->getWorldState(WS_CHALLENGE_LAST_RESET_TIME)))
                continue;

            if (!v->ChestID)
                continue;

            auto itr = _oploteWeekLoot.find(c.first);
            if (itr != _oploteWeekLoot.end())
            {
                if (itr->second.ChallengeLevel < v->ChallengeLevel)
                    itr->second.ChallengeLevel = v->ChallengeLevel;

                itr->second.chestListID.insert(v->ChestID);
            }
            else
            {
                OploteLoot& lootOplote = _oploteWeekLoot[c.first];
                lootOplote.Date = sWorld->getNextChallengeKeyReset();
                lootOplote.ChallengeLevel = v->ChallengeLevel;
                lootOplote.needSave = true;
                lootOplote.guid = c.first;
                lootOplote.chestListID.insert(v->ChestID);
            }
        }
    }
    _challengeWeekList.clear();
    SaveOploteLootToDB();
}

bool ChallengeMgr::GetStartPosition(uint32 mapID, float& x, float& y, float& z, float& o, ObjectGuid OwnerGuid)
{
    uint32 WorldSafeLocID = 0;
    switch (mapID)
    {
        case 1493:
            WorldSafeLocID = 5105;
            break;
        case 1477:
            WorldSafeLocID = 5098;
            break;
        case 1466:
            WorldSafeLocID = 5334;
            break;
        case 1501:
            WorldSafeLocID = 5352;
            break;
        case 1492:
            WorldSafeLocID = 5102;
            break;
        case 1544:
            WorldSafeLocID = 5293;
            break;
        case 1456:
            WorldSafeLocID = 5100;
            break;
        case 1571:
            WorldSafeLocID = 5432;
            break;
        case 1516:
            WorldSafeLocID = 5194;
            break;
        case 1458:
            WorldSafeLocID = 5355;
            break;
        case 1651:
            if (Player* keyOwner = ObjectAccessor::FindPlayer(OwnerGuid))
            {
                if (keyOwner->m_challengeKeyInfo.ID == 227)
                    WorldSafeLocID = 6022; // 7.2 Karazhan - Challenge Mode Start (Lower Karazhan)
                else
                    WorldSafeLocID = 6023; // 7.2 Karazhan - Challenge Mode Start (Upper Karazhan)
            }
            break;
        case 1677:  // Cathedral of Eternal Night
            WorldSafeLocID = 5891;
            break;
        case 1753:  // Seat of the Triumvirate
            WorldSafeLocID = 6113;
            break;
        default:
            break;
    }
    if (WorldSafeLocID == 0)
        return false;

    if (WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(WorldSafeLocID))
    {
        x = entry->Loc.X;
        y = entry->Loc.Y;
        z = entry->Loc.Z;
        o = entry->Loc.O * M_PI / 180.0f;
        return true;
    }
    return false;
}

uint32 ChallengeMgr::GetLootTreeMod(int32& levelBonus, uint32& challengeLevel, Challenge* challenge)
{
    auto isOplote = bool(challenge == nullptr);
    if (challenge)
        challengeLevel = std::min(challenge->GetChallengeLevel(), 15u);

    uint8 levelingStep = sWorld->getIntConfig(CONFIG_CHALLENGE_LEVEL_STEP);
    uint8 leveling = challengeLevel;

    if (sWorld->getIntConfig(CONFIG_CHALLENGE_LEVEL_MAX) < leveling)
        leveling = sWorld->getIntConfig(CONFIG_CHALLENGE_LEVEL_MAX);

    levelBonus = isOplote ? stepOplotLeveling[levelingStep][leveling] : stepLeveling[levelingStep][leveling];

    return 16;
}

uint32 ChallengeMgr::GetCAForLoot(Challenge* const challenge, uint32 goEntry)
{
    if (!challenge)
        return 0;

    switch (challenge->_mapID)
    {
        case 1492: // Maw of Souls
        {
            // Lesser Dungeons
            switch (challenge->GetChallengeLevel())
            {
                case 0:
                case 1:
                    return 0;
                case 2:
                case 3:
                    return 147398;
                case 4:
                case 5:
                case 6:
                    return 147399;
                case 7:
                case 8:
                case 9:
                    return 147400;
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    return 147401;
                case 15:
                default:
                    return 147719;
            }
        }
        case 1493: // Vault of the Wardens
        case 1466: // Darkheart Thicket
        case 1501: // Black Rook Hold
        case 1544: // Assault on Violet Hold
        case 1456: // Eye of Azshara
        case 1571: // Court of Stars
        case 1458: // Neltharion's Lair
        case 1651: // Karazhan
        case 1677: // Cathedral of Eternal Night
        case 1753: // Seat of the Triumvirate
        {
            // Regular Dungeons
            switch (challenge->GetChallengeLevel())
            {
                case 0:
                case 1:
                    return 0;
                case 2:
                case 3:
                    return 147402;
                case 4:
                case 5:
                case 6:
                    return 147403;
                case 7:
                case 8:
                case 9:
                    return 147404;
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    return 147405;
                case 15:
                default:
                    return 147718;
            }
        }
        case 1516: // The Arcway
        case 1477: // Halls of Valor
        {
            // Greater Dungeons
            switch (challenge->GetChallengeLevel())
            {
                case 0:
                case 1:
                    return 0;
                case 2:
                case 3:
                    return 147406;
                case 4:
                case 5:
                case 6:
                    return 147407;
                case 7:
                case 8:
                case 9:
                    return 147408;
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                    return 147409;
                case 15:
                default:
                    return 147720;
            }
        }
        default:
            break;
    }

    return 0;
}

uint32 ChallengeMgr::GetBigCAForLoot(Challenge* const challenge, uint32 goEntry, uint32& count)
{
    if (!challenge || challenge->GetChallengeLevel() <= 10)
        return 0;

    if (challenge->GetChallengeLevel() >= 15)
        count = challenge->GetChallengeLevel() - 15;
    else
        count = challenge->GetChallengeLevel() - 10;

    switch (challenge->_mapID)
    {
        case 1492: // Maw of Souls
        {
            // Lesser Dungeons
            return 147808; // Lesser Adept's Spoils
        }
        case 1493: // Vault of the Wardens
        case 1466: // Darkheart Thicket
        case 1501: // Black Rook Hold
        case 1544: // Assault on Violet Hold
        case 1456: // Eye of Azshara
        case 1571: // Court of Stars
        case 1458: // Neltharion's Lair
        case 1651: // Karazhan
        case 1677: // Cathedral of Eternal Night
        case 1753: // Seat of the Triumvirate
        {
            // Regular Dungeons
            return 147809; // Adept's Spoils
        }
        case 1516: // The Arcway
        case 1477: // Halls of Valor
        {
            // Greater Dungeons
            return 147810; // Greater Adept's Spoils
        }
        default:
            break;
    }

    return 0;
}

uint32 ChallengeMgr::GetCAForOplote(uint32 challengeLevel)
{
    switch (challengeLevel)
    {
        // Is bug???
        case 0:
        case 1:
            return 0;
        case 2:
        case 3:
            return 147551;
        case 4:
        case 5:
        case 6:
            return 147550;
        case 7:
        case 8:
        case 9:
            return 147549;
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            return 147548;
        default: // 15+
            return 147721;
    }
}


uint32 ChallengeMgr::GetBigCAForOplote(uint32 challengeLevel, uint32& count)
{
    if (challengeLevel <= 10)
        return 0;

    if (challengeLevel >= 15)
        count = challengeLevel - 15;
    else
        count = challengeLevel - 10;

    return 147819;
}

float ChallengeMgr::GetChanceItem(uint8 mode, uint32 challengeLevel)
{
    float base_chance = 200.0f;
    float add_chance = 0.0f;

    if (challengeLevel > 10)
        add_chance += (challengeLevel - 10) * 40.0f;

    switch (mode)
    {
        case CHALLENGE_TIMER_LEVEL_3: /// 3 chests + 3 levels
        case CHALLENGE_TIMER_LEVEL_2: /// 2 chests + 2 levels
        case CHALLENGE_TIMER_LEVEL_1: /// 1 chest + 1 level
            base_chance += 100.0f; // 300% is 3 items
            break;
        case CHALLENGE_NOT_IN_TIMER:  /// 0 chest
            base_chance += 0.0f; // 200% is 2 items
            break;
        default:
            break;
    }

    base_chance += add_chance;

    return base_chance;
}

bool ChallengeMgr::IsChest(uint32 goEntry)
{
    switch (goEntry)
    {
        case 252674: // Vault of the Wardens 100-110
        case 252677: // Black Rook Hold Dungeon 100-110
        case 252686: // Court of Stars 110
        case 252668: // Dark Heart Thicket 100-110
        case 252665: // Eye of Azshara 100-110
        case 252056: // Halls of Valor 100-110
        case 252680: // Maw of Souls 100-110
        case 252671: // Neltharions Lair 100-110
        case 252683: // The Arcway 100-110
        case 269852: // Lower Return to Karazhan 110
        case 269871: // Upper Return to Karazhan 110
        case 269843: // Cathedral of Eternal Night
        case 272689: // Seat of the Triumvirate
            return true;
        default:
            break;
    }

    return false;
}

bool ChallengeMgr::IsDoor(uint32 goEntry)
{
    switch (goEntry)
    {
        case 211989:
        case 211991:
        case 212972:
        case 211992:
        case 211988:
        case 212282:
        case 212387:
        case 239323:
        case 239408:
            return true;
        default:
            break;
    }

    return false;
}
