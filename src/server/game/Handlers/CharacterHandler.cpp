/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "DatabaseEnv.h"
#include "BattlePayMgr.h"
#include "Mail.h"
#include "ObjectMgr.h"
#include "QuestData.h"
#include "CharacterPackets.h"
#include "PlayerDefines.h"
#include "CharacterData.h"
#include "GlobalFunctional.h"
#include "ScriptMgr.h"
#include "GuildMgr.h"
#include "GuildFinderMgr.h"
#include "PlayerDump.h"
#include "Chat.h"
#include "MiscPackets.h"
#include "CalendarPackets.h"
#include "InstanceSaveMgr.h"
#include "InstanceScript.h"
#include "AuthenticationPackets.h"
#include "ClientConfigPackets.h"
#include "SystemPackets.h"
#include "WorldStateMgr.h"
#include "AreaTriggerData.h"
#include "SocialMgr.h"
#include "AccountMgr.h"
#include <boost/regex.hpp>
#include "BattlegroundPackets.h"
#include "GitRevision.h"
#include "ArtifactPackets.h"
#include "LoginQueryHolder.h"
#include "GameEventMgr.h"

void WorldSession::HandleCharEnum(PreparedQueryResult result, bool isDeleted)
{
    m_DHCount = 0;
    m_DKCount = 0;
    m_raceMask = 0;
    m_canDK = false;
    m_canDH = false;

    uint32 heroicReqLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
    uint32 demonHunterReqLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_DEMON_HUNTER);

    bool canAlwaysCreateDemonHunter = false;
    if (demonHunterReqLevel == 0) // char level = 0 means this check is disabled, so always true
        canAlwaysCreateDemonHunter = true;

    WorldPackets::Character::EnumCharactersResult charEnum;
    charEnum.Success = true;
    charEnum.IsDeletedCharacters = isDeleted;
    charEnum.Unknown7x = true;
    charEnum.DisabledClassesMask = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK);

    _allowedCharsToLogin.clear();

    if (result)
    {
        do
        {
            charEnum.Characters.emplace_back(result->Fetch(), charEnumInfo, GetAccountId());
            auto& charInfo = charEnum.Characters.back();

            m_raceMask |= 1 << (charInfo.Race - 1);
            m_classMask |= 1 << (charInfo.Class - 1);

            if (!(charInfo.Flags & (CHARACTER_FLAG_LOCKED_FOR_TRANSFER | CHARACTER_FLAG_LOCKED_BY_BILLING)))
                _allowedCharsToLogin.insert(charInfo.Guid.GetCounter());

            if (charInfo.Class == CLASS_DEMON_HUNTER)
                m_DHCount++;

            if (charInfo.Class == CLASS_DEATH_KNIGHT)
                m_DKCount++;

            if (charInfo.Level >= heroicReqLevel)
                m_canDK = true;

            if (charInfo.Level >= demonHunterReqLevel)
                m_canDH = true;

            if (m_DHCount >= sWorld->getIntConfig(CONFIG_DEMON_HUNTERS_PER_REALM) && !canAlwaysCreateDemonHunter)
                charEnum.HasDemonHunterOnRealm = true;
            else
                charEnum.HasDemonHunterOnRealm = false;

            charEnum.MaxCharacterLevel = std::max<int32>(charEnum.MaxCharacterLevel, charInfo.Level);

        } while (result->NextRow());
    }

    charEnum.IsDemonHunterCreationAllowed = GetAccountExpansion() >= EXPANSION_LEGION || canAlwaysCreateDemonHunter;
    charEnum.IsAlliedRacesCreationAllowed = GetAccountExpansion() >= EXPANSION_LEGION;

    if (!charTemplateData.empty())
    {
        m_canDK = true;
        m_canDH = true;
        charEnum.IsDemonHunterCreationAllowed = true;
    }

    for (auto const& requirement : sObjectMgr->GetRaceUnlockRequirements())
    {
        WorldPackets::Character::EnumCharactersResult::RaceUnlock raceUnlock;
        raceUnlock.RaceID = requirement.first;
        raceUnlock.HasExpansion = GetAccountExpansion() >= requirement.second.Expansion;
        raceUnlock.HasAchievement = requirement.second.AchievementId == 0 || HasAchievement(requirement.second.AchievementId);
        raceUnlock.HasHeritageArmor = true;
        charEnum.RaceUnlockData.push_back(raceUnlock);
    }

    SendPacket(charEnum.Write());
    timeCharEnumOpcode = 0;

    sScriptMgr->OnSessionLogin(this);
}

void WorldSession::HandleCharEnumOpcode(WorldPackets::Character::EnumCharacters& enumCharacters)
{
    if (timeCharEnumOpcode)
        return;

    timeCharEnumOpcode = 1;

    SendCharacterEnum(enumCharacters.GetOpcode() == CMSG_ENUM_CHARACTERS_DELETED_BY_CLIENT);
}

void WorldSession::SendCharacterEnum(bool deleted /*= false*/)
{
    // remove expired bans
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_EXPIRED_BANS);
    CharacterDatabase.Execute(stmt);

    /// get all the data necessary for loading all characters (along with their pets) on the account

    //if (sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    //    stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM_DECLINED_NAME);
    //else
    if (deleted)
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM_DELETED);
    else
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ENUM);

    stmt->setUInt32(0, GetAccountId());

    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSession::HandleCharEnum, this, std::placeholders::_1, deleted)));
}

void WorldSession::HandleCharCreateOpcode(WorldPackets::Character::CreateChar& charCreate)
{
    auto SendCharCreate = [this](ResponseCodes result) -> void
    {
        WorldPackets::Character::CharacterCreateResponse response;
        response.Code = result;
        SendPacket(response.Write());
    };

    if (AccountMgr::IsPlayerAccount(GetSecurity()))
    {
        if (uint32 mask = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED))
        {
            bool disabled = false;

            uint32 team = Player::TeamForRace(charCreate.CreateInfo->Race);
            switch (team)
            {
                case ALLIANCE: 
                    disabled = (mask & (1 << 0)) != 0; 
                    break;
                case HORDE:    
                    disabled = (mask & (1 << 1)) != 0; 
                    break;
                case PANDAREN_NEUTRAL:
                    disabled = (mask & (1 << 2)) != 0;
                    break;
                default:
                    disabled = true;
                    break;
            }

            if (disabled)
            {
                SendCharCreate(CHAR_CREATE_DISABLED);
                return;
            }
        }
    }

    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(charCreate.CreateInfo->Class);
    if (!classEntry)
    {
        SendCharCreate(CHAR_CREATE_FAILED);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Class (%u) not found in DBC while creating new char for account (ID: %u): wrong DBC files or cheater?", charCreate.CreateInfo->Class, GetAccountId());
        return;
    }

    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(charCreate.CreateInfo->Race);
    if (!raceEntry)
    {
        SendCharCreate(CHAR_CREATE_FAILED);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Race (%u) not found in DBC while creating new char for account (ID: %u): wrong DBC files or cheater?", charCreate.CreateInfo->Race, GetAccountId());
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()))
    {
        if ((1 << (charCreate.CreateInfo->Race - 1)) & uint32(sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK)))
        {
            SendCharCreate(CHAR_CREATE_DISABLED);
            return;
        }

        if ((1 << (charCreate.CreateInfo->Class - 1)) & uint32(sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK)))
        {
            SendCharCreate(CHAR_CREATE_DISABLED);
            return;
        }
    }

    // prevent character creating with invalid name
    if (!normalizePlayerName(charCreate.CreateInfo->Name))
    {
        SendCharCreate(CHAR_NAME_NO_NAME);
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Account:[%d] but tried to Create character with empty [name] ", GetAccountId());
        return;
    }

    // prevent character creating Expansion race without Expansion account
    RaceUnlockRequirement const* raceExpansionRequirement = sObjectMgr->GetRaceUnlockRequirement(charCreate.CreateInfo->Race);
    if (!raceExpansionRequirement)
    {
        SendCharCreate(CHAR_CREATE_FAILED);
        return;
    }

    if (raceExpansionRequirement->Expansion > GetAccountExpansion())
    {
        SendCharCreate(CHAR_CREATE_EXPANSION);
        return;
    }

    //if (raceExpansionRequirement->AchievementId && !)
    //{
    //    TC_LOG_ERROR("entities.player.cheat", "Expansion %u account:[%d] tried to Create character without achievement %u race (%u)",
    //        GetAccountExpansion(), GetAccountId(), raceExpansionRequirement->AchievementId, charCreate.CreateInfo->Race);
    //    SendCharCreate(CHAR_CREATE_ALLIED_RACE_ACHIEVEMENT);
    //    return;
    //}

    // check name limitations
    ResponseCodes res = sCharacterDataStore->CheckPlayerName(charCreate.CreateInfo->Name, GetSessionDbcLocale(), true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharCreate(res);
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sCharacterDataStore->IsReservedName(charCreate.CreateInfo->Name))
    {
        SendCharCreate(CHAR_NAME_RESERVED);
        return;
    }

    // speedup check for heroic class disabled case
    uint32 heroic_free_slots = sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM);
    if (heroic_free_slots == 0 && AccountMgr::IsPlayerAccount(GetSecurity()) && charCreate.CreateInfo->Class == CLASS_DEATH_KNIGHT)
    {
        SendCharCreate(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
        return;
    }

    // speedup check for heroic class disabled case
    uint32 req_level_for_heroic = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && charCreate.CreateInfo->Class == CLASS_DEATH_KNIGHT && req_level_for_heroic > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        SendCharCreate(CHAR_CREATE_LEVEL_REQUIREMENT);
        return;
    }

    if (sWorld->GetCharacterInfo(charCreate.CreateInfo->Name))
    {
        SendCharCreate(CHAR_CREATE_NAME_IN_USE);
        return;
    }

    std::shared_ptr<WorldPackets::Character::CharacterCreateInfo> createInfo = charCreate.CreateInfo;
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
    stmt->setString(0, charCreate.CreateInfo->Name);

    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt)
        .WithChainingPreparedCallback([this, SendCharCreate](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (result)
        {
            SendCharCreate(CHAR_CREATE_NAME_IN_USE);
            return;
        }

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_SUM_REALM_CHARACTERS);
        stmt->setUInt32(0, GetAccountId());
        queryCallback.SetNextQuery(LoginDatabase.AsyncQuery(stmt));
    })
        .WithChainingPreparedCallback([this, SendCharCreate](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        uint64 acctCharCount = 0;
        if (result)
            acctCharCount = uint64(result->Fetch()[0].GetDouble());

        if (acctCharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
        {
            SendCharCreate(CHAR_CREATE_ACCOUNT_LIMIT);
            return;
        }

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
        stmt->setUInt32(0, GetAccountId());
        queryCallback.SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    })
        .WithChainingPreparedCallback([this, createInfo, SendCharCreate](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (result)
        {
            createInfo->CharCount = uint8(result->Fetch()[0].GetUInt64()); // SQL's COUNT() returns uint64 but it will always be less than uint8.Max
            if (createInfo->CharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
            {
                SendCharCreate(CHAR_CREATE_SERVER_LIMIT);
                return;
            }
        }

        bool allowTwoSideAccounts = !sWorld->IsPvPRealm();
        uint32 skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);

        std::function<void(PreparedQueryResult)> finalizeCharacterCreation = [this, createInfo, SendCharCreate](PreparedQueryResult result)
        {
            bool haveSameRace = false;
            uint32 heroicReqLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER);
            uint32 demonHunterReqLevel = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_DEMON_HUNTER);
            bool hasHeroicReqLevel = (heroicReqLevel != 0) && !createInfo->TemplateSet.is_initialized();
            bool hasDemonHunterReqLevel = (demonHunterReqLevel != 0) && !createInfo->TemplateSet.is_initialized();
            uint32 skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);
            bool checkHeroicReqs = createInfo->Class == CLASS_DEATH_KNIGHT;
            bool checkDemonHunterReqs = createInfo->Class == CLASS_DEMON_HUNTER;

            if (m_raceMask & (1 << (createInfo->Race - 1)))
                haveSameRace = true;

            uint32 freeHeroicSlots = m_DKCount > sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM) ? 0 : sWorld->getIntConfig(CONFIG_HEROIC_CHARACTERS_PER_REALM) - m_DKCount;
            uint32 freeDemonHunterSlots = m_DHCount > sWorld->getIntConfig(CONFIG_DEMON_HUNTERS_PER_REALM) ? 0 : sWorld->getIntConfig(CONFIG_DEMON_HUNTERS_PER_REALM) - m_DHCount;

            if (checkHeroicReqs && !m_canDK && hasHeroicReqLevel && freeHeroicSlots)
            {
                SendCharCreate(CHAR_CREATE_LEVEL_REQUIREMENT);
                return;
            }

            if (checkDemonHunterReqs && !m_canDH && hasDemonHunterReqLevel && freeDemonHunterSlots)
            {
                SendCharCreate(CHAR_CREATE_LEVEL_REQUIREMENT_DEMON_HUNTER);
                return;
            }

            // Avoid exploit of create multiple characters with same name
            if (!sWorld->CheckCharacterName(createInfo->Name))
            {
                SendCharCreate(CHAR_CREATE_NAME_IN_USE);
                return;
            }

            Player newChar(this);
            newChar.GetMotionMaster()->Initialize();
            if (!newChar.Create(sObjectMgr->GetGenerator<HighGuid::Player>()->Generate(), createInfo.get()))
            {
                // Player not create (race/class/etc problem?)
                newChar.CleanupsBeforeDelete();

                SendCharCreate(CHAR_CREATE_ERROR);
                return;
            }

            if ((haveSameRace && skipCinematics == 1) || skipCinematics == 2)
                newChar.setCinematic(1);                          // not show intro

            newChar.SetAtLoginFlag(AT_LOGIN_FIRST);               // First login

            // Player created, save it now
            newChar.SaveToDB(true);
            createInfo->CharCount += 1;

            SQLTransaction trans = LoginDatabase.BeginTransaction();

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM);
            stmt->setUInt32(0, GetAccountId());
            stmt->setUInt32(1, realm.Id.Realm);
            trans->Append(stmt);

            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
            stmt->setUInt32(0, createInfo->CharCount);
            stmt->setUInt32(1, GetAccountId());
            stmt->setUInt32(2, realm.Id.Realm);
            trans->Append(stmt);

            if (createInfo->TemplateSet)
            {
                if (CharacterTemplateData* charTemplateData = GetCharacterTemplateData(*createInfo->TemplateSet))
                {
                    stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_CHARACTER_TEMPLATE);
                    stmt->setUInt32(0, newChar.GetGUIDLow());
                    stmt->setUInt32(1, charTemplateData->id);
                    trans->Append(stmt);

                    if (charTemplateData->transferId)
                    {
                        uint8 raceID = 2;
                        if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(createInfo->Race))
                            raceID = rEntry->Alliance;

                        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_TRANSFER_REQUESTS);
                        stmt->setUInt32(0, newChar.GetGUIDLow());
                        stmt->setUInt8(1, createInfo->Class);
                        stmt->setUInt8(2, raceID);
                        stmt->setUInt32(3, charTemplateData->transferId);
                        trans->Append(stmt);
                        CharacterDatabase.PExecute("UPDATE `characters` SET transfer_request = '%u' WHERE guid = '%u'", charTemplateData->transferId, newChar.GetGUIDLow());
                    }
                }
            }

            LoginDatabase.CommitTransaction(trans);

            SendCharCreate(CHAR_CREATE_SUCCESS);

            TC_LOG_INFO(LOG_FILTER_CHARACTER, "Account: %d (IP: %s) Create Character:[%s] (GUID: %u)", GetAccountId(), GetRemoteAddress().c_str(), createInfo->Name.c_str(), newChar.GetGUIDLow());
            sScriptMgr->OnPlayerCreate(&newChar);
            sWorld->AddCharacterInfo(newChar.GetGUIDLow(), std::string(newChar.GetName()), newChar.getGender(), newChar.getRace(), newChar.getClass(), newChar.getLevel(), GetAccountId());
            sWorld->UpdateCharacterAccount(newChar.GetGUIDLow(), GetAccountId());

            newChar.GetAchievementMgr()->ClearMap();
            newChar.CleanupsBeforeDelete();
        };

        if (allowTwoSideAccounts && !skipCinematics && createInfo->Class != CLASS_DEATH_KNIGHT && createInfo->Class != CLASS_DEMON_HUNTER)
        {
            finalizeCharacterCreation(PreparedQueryResult(nullptr));
            return;
        }

        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CREATE_INFO);
        stmt->setUInt32(0, GetAccountId());
        stmt->setUInt32(1, (skipCinematics == 1 || createInfo->Class == CLASS_DEATH_KNIGHT || createInfo->Class == CLASS_DEMON_HUNTER) ? 12 : 1);
        queryCallback.WithPreparedCallback(std::move(finalizeCharacterCreation)).SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    }));
}

void WorldSession::HandleCharDeleteOpcode(WorldPackets::Character::DeleteChar& charDelete)
{
    if (ObjectAccessor::FindPlayer(charDelete.Guid))
        return;

    uint32 accountId = 0;
    uint32 guildId = 0;
    std::string name;

    auto SendCharDelete = [this](ResponseCodes result)-> void
    {
        WorldPackets::Character::CharacterDeleteResponse response;
        response.Code = result;
        SendPacket(response.Write());
    };

    if (sGuildMgr->GetGuildByLeader(charDelete.Guid))
    {
        SendCharDelete(CHAR_DELETE_FAILED_GUILD_LEADER);
        return;
    }

    if (auto nameData = sWorld->GetCharacterInfo(charDelete.Guid))
    {
        guildId = nameData->GuildId;
        accountId = nameData->AccountId;
        name = nameData->Name;
    }
    else
        return;

    if (accountId != GetAccountId())
        return;

    TC_LOG_INFO(LOG_FILTER_CHARACTER, "Account: %d (IP: %s) Delete Character:[%s] (GUID: %u)", GetAccountId(), GetRemoteAddress().c_str(), name.c_str(), charDelete.Guid.GetGUIDLow());
    sScriptMgr->OnPlayerDelete(charDelete.Guid);
    sWorld->DeleteCharacterNameData(charDelete.Guid.GetCounter());

    if (sLog->ShouldLog(LOG_FILTER_PLAYER_DUMP, LOG_LEVEL_INFO)) // optimize GetPlayerDump call
    {
        std::string dump;
        if (PlayerDumpWriter().GetDump(charDelete.Guid.GetCounter(), dump))
            sLog->outCharDump(dump.c_str(), GetAccountId(), charDelete.Guid.GetCounter(), name.c_str());
    }

    sGuildFinderMgr->RemoveMembershipRequest(charDelete.Guid, ObjectGuid::Create<HighGuid::Guild>(guildId));
    Player::DeleteFromDB(charDelete.Guid, GetAccountId());
    sWorld->DeleteCharName(name);

    SendCharDelete(CHAR_DELETE_SUCCESS);

    // reset timer.
    timeCharEnumOpcode = 0;
}

void WorldSession::HandlePlayerLoginOpcode(WorldPackets::Character::PlayerLogin& playerLogin)
{
    // Prevent flood of CMSG_PLAYER_LOGIN
    if (++playerLoginCounter > 10)
    {
        TC_LOG_ERROR(LOG_FILTER_OPCODES, "Player kicked due to flood of CMSG_PLAYER_LOGIN");
        KickPlayer();
        return;
    }

    if (PlayerLoading() || GetPlayer() != nullptr)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Player tries to login again, AccountId = %d", GetAccountId());
        return;
    }

    m_playerLoading = playerLogin.Guid;

    TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "Character (Guid: %s) logging in", playerLogin.Guid.ToString().c_str());

    if (!CharCanLogin(playerLogin.Guid.GetCounter()))
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Account (%u) can't login with that character (%u).", GetAccountId(), playerLogin.Guid.GetGUIDLow());
        KickPlayer();
        return;
    }

    SendConnectToInstance(WorldPackets::Auth::ConnectToSerial::WorldAttempt1);
}

void WorldSession::HandleLoadScreenOpcode(WorldPackets::Character::LoadingScreenNotify& loadingScreenNotify)
{
    if (!loadingScreenNotify.Showing)
    {
        if (auto player = GetPlayer())
            player->SendInitialPacketsAfterAddToMap(true);
        m_playerLoading.Clear();
    }
}

void WorldSession::HandlePlayerLogin(LoginQueryHolder* holder)
{
    auto playerGuid = holder->GetGuid();

    auto pCurrChar = new Player(this);
    // for send server info and strings (config)
    auto chH = ChatHandler(pCurrChar);

    // "GetAccountId() == db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if (!pCurrChar->LoadFromDB(playerGuid, holder))
    {
        SetPlayer(nullptr);
        KickPlayer();                                       // disconnect client, player no set to session and it will not deleted or saved at kick
        delete pCurrChar;                                   // delete it manually
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading.Clear();
        return;
    }

    SendTutorialsData();

    sWorld->UpdateCharacterAccount(playerGuid.GetGUIDLow(), GetAccountId());

    pCurrChar->GetMotionMaster()->Initialize();
    pCurrChar->SendDungeonDifficulty();
    pCurrChar->SetChangeMap(true);

    WorldPackets::Character::LoginVerifyWorld loginVerifyWorld;
    loginVerifyWorld.MapID = pCurrChar->GetMapId();
    loginVerifyWorld.Pos = pCurrChar->GetPosition();
    SendPacket(loginVerifyWorld.Write());

    // load player specific part before send times
    LoadAccountData(holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOADACCOUNTDATA), PER_CHARACTER_CACHE_MASK);

    // rewrite client channel mask if not valid (autojoin LFG channel)
    if (auto aData = GetAccountData(PER_CHARACTER_CHAT_CACHE))
    {
        auto data = aData->Data;

        if (!data.empty())
        {
            boost::regex regEx("ZONECHANNELS[ \f\n\r\t\v][1-9][0-9]+");
            boost::smatch res;
            auto replace = false;
            if (boost::regex_search(data, res, regEx))
            {
                std::string m = res[0];
                auto channel = m.substr(12, m.size());
                auto channelMask = std::stoi(channel);

                if (channelMask != 0x2200003)
                {
                    // set now time for invalidate cache on client and forced request
                    aData->Time = sWorld->GetGameTime();
                    replace = true;
                }
            }

            if (replace)
                aData->Data = boost::regex_replace(data, regEx, "ZONECHANNELS 35651587");
        }
    }

    // add special survey data
    if (auto aData1 = GetAccountData(GLOBAL_CONFIG_CACHE))
    {
        std::string data = aData1->Data;

        if (data.empty())
            data.append("SET engineSurvey \"0\"");
        else if (size_t pos = data.find("engineSurvey"))
        {
            if (pos != std::string::npos)
            {
                std::string surveyIdStr = data.substr(pos + 14, 1);
                auto surveyID = std::stoi(surveyIdStr);
                if (surveyID)
                    data.replace(pos + 14, 1, "0");
            }
            else
            {
                if (data[data.size() - 1] == '\n')
                    data.append("SET engineSurvey \"0\"");
                else
                    data.append("\nSET engineSurvey \"0\"");
            }
        }

        // set now time for invalidate cache on client and forced request
        aData1->Time = sWorld->GetGameTime();
        aData1->Data = data;
    }

    WorldPackets::ClientConfig::AccountDataTimes accountDataTimes;
    accountDataTimes.PlayerGuid = playerGuid;
    accountDataTimes.ServerTime = uint32(sWorld->GetGameTime());
    for (uint32 i = 0; i < NUM_ACCOUNT_DATA_TYPES; ++i)
        accountDataTimes.AccountTimes[i] = uint32(GetAccountData(AccountDataType(i))->Time);

    SendPacket(accountDataTimes.Write());

    /// Send FeatureSystemStatus
    {
        WorldPackets::System::FeatureSystemStatus features;
        features.ComplaintStatus = 2;
        features.ScrollOfResurrectionRequestsRemaining = 1;
        features.ScrollOfResurrectionMaxRequestsPerDay = 1;
        features.TwitterPostThrottleLimit = 60;
        features.TwitterPostThrottleCooldown = 20;
        features.CfgRealmID = 2;
        features.CfgRealmRecID = 0;
        features.TokenPollTimeSeconds = 300;
        features.TokenRedeemIndex = 0;
        features.TokenBalanceAmount = 5500000;
        features.VoiceEnabled = false;
        features.ScrollOfResurrectionEnabled = false;
        features.CharUndeleteEnabled = HasAuthFlag(AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER);
        features.BpayStoreEnabled = GetBattlePayMgr()->IsAvailable();
        features.BpayStoreAvailable = GetBattlePayMgr()->IsAvailable();
        features.BpayStoreDisabledByParentalControls = false;
        features.ItemRestorationButtonEnabled = true;
        features.RecruitAFriendSendingEnabled = false;
        features.CommerceSystemEnabled = false;
        features.BrowserEnabled = false;//  GetBattlePayMgr()->IsAvailable(); // Has to be false, otherwise client will crash if "Customer Support" is opened
        features.TutorialsEnabled = true;
        features.NPETutorialsEnabled = true;
        features.RestrictedAccount = false;
        features.TwitterEnabled = false;
        features.Unk67 = true;
        features.WillKickFromWorld = false;
        features.KioskModeEnabled = false;
        features.CompetitiveModeEnabled = false;
        features.TokenBalanceEnabled = true;

        features.EuropaTicketSystemStatus = boost::in_place();
        features.EuropaTicketSystemStatus->ThrottleState.MaxTries = 10;
        features.EuropaTicketSystemStatus->ThrottleState.PerMilliseconds = 60000;
        features.EuropaTicketSystemStatus->ThrottleState.TryCount = 1;
        features.EuropaTicketSystemStatus->ThrottleState.LastResetTimeBeforeNow = 4045221;
        features.EuropaTicketSystemStatus->TicketsEnabled = true;
        features.EuropaTicketSystemStatus->BugsEnabled = true;
        features.EuropaTicketSystemStatus->ComplaintsEnabled = true;
        features.EuropaTicketSystemStatus->SuggestionsEnabled = true;

        features.QuickJoinConfig.ToastsDisabled = false;
        features.QuickJoinConfig.ToastDuration = 7;
        features.QuickJoinConfig.DelayDuration = 10;
        features.QuickJoinConfig.QueueMultiplier = 1;
        features.QuickJoinConfig.PlayerMultiplier = 1;
        features.QuickJoinConfig.PlayerFriendValue = 5;
        features.QuickJoinConfig.PlayerGuildValue = 1;
        features.QuickJoinConfig.ThrottleInitialThreshold = 0;
        features.QuickJoinConfig.ThrottleDecayTime = 60;
        features.QuickJoinConfig.ThrottlePrioritySpike = 20;
        features.QuickJoinConfig.ThrottleMinThreshold = 0;
        features.QuickJoinConfig.ThrottlePvPPriorityNormal = 50;
        features.QuickJoinConfig.ThrottlePvPPriorityLow = 1;
        features.QuickJoinConfig.ThrottlePvPHonorThreshold = 10;
        features.QuickJoinConfig.ThrottleLfgListPriorityDefault = 50;
        features.QuickJoinConfig.ThrottleLfgListPriorityAbove = 100;
        features.QuickJoinConfig.ThrottleLfgListPriorityBelow = 50;
        features.QuickJoinConfig.ThrottleLfgListIlvlScalingAbove = 1;
        features.QuickJoinConfig.ThrottleLfgListIlvlScalingBelow = 1;
        features.QuickJoinConfig.ThrottleRfPriorityAbove = 100;
        features.QuickJoinConfig.ThrottleRfIlvlScalingAbove = 1;
        features.QuickJoinConfig.ThrottleDfMaxItemLevel = 850;
        features.QuickJoinConfig.ThrottleDfBestPriority = 80;

        SendPacket(features.Write());
    }

    // Send MOTD
    {
        WorldPackets::System::MOTD motd;
        motd.Text = &sWorld->GetMotd();
        SendPacket(motd.Write());

        // send server info
        if (sWorld->getIntConfig(CONFIG_ENABLE_SINFO_LOGIN) == 1)
            chH.PSendSysMessage(GitRevision::GetFullVersion());

        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: Sent server info");
    }

    SendSetTimeZoneInformation();

    //QueryResult* result = CharacterDatabase.PQuery("SELECT guildid, rank FROM guild_member WHERE guid = '%u'", pCurrChar->GetGUIDLow());
    if (PreparedQueryResult resultGuild = holder->GetPreparedResult(PLAYER_LOGIN_QUERY_LOADGUILD))
    {
        Field* fields = resultGuild->Fetch();
        pCurrChar->SetInGuild(fields[0].GetUInt64());
        pCurrChar->SetRank(fields[1].GetUInt8());
        if (Guild* guild = sGuildMgr->GetGuildById(pCurrChar->GetGuildId()))
            pCurrChar->SetGuildLevel(guild->GetLevel());
    }
    else if (pCurrChar->GetGuildId())                        // clear guild related fields in case wrong data about non existed membership
    {
        pCurrChar->SetInGuild(UI64LIT(0));
        pCurrChar->SetRank(0);
        pCurrChar->SetGuildLevel(0);
    }

    WorldPackets::Battleground::PVPSeason season;
    season.PreviousSeason = sWorld->getIntConfig(CONFIG_ARENA_SEASON_ID) - 1;
    if (sWorld->getBoolConfig(CONFIG_ARENA_SEASON_IN_PROGRESS))
        season.CurrentSeason = sWorld->getIntConfig(CONFIG_ARENA_SEASON_ID);
    SendPacket(season.Write());

    auto sess = sWorld->FindSession(GetAccountId());
    SetMap(pCurrChar->GetMap());
    GetMap()->AddSession(sess);

    AddDelayedEvent(100, [=]() -> void
    {
        if (!this)
            return;

        auto player = GetPlayer();
        if (!player)
            return;

        auto chatHandler = ChatHandler(player);
        player->SendInitialPacketsBeforeAddToMap();

        WorldPackets::Artifact::ArtifactKnowledge artifactKnowledge;
        artifactKnowledge.ArtifactCategoryID = ARTIFACT_CATEGORY_CLASS;
        artifactKnowledge.KnowledgeLevel = player->GetCurrency(CURRENCY_TYPE_ARTIFACT_KNOWLEDGE);
        SendPacket(artifactKnowledge.Write());

        WorldPackets::Artifact::ArtifactKnowledge artifactKnowledgeFishingPole;
        artifactKnowledgeFishingPole.ArtifactCategoryID = ARTIFACT_CATEGORY_FISH;
        artifactKnowledgeFishingPole.KnowledgeLevel = 0;
        SendPacket(artifactKnowledgeFishingPole.Write());

        //Show cinematic at the first time that player login
        bool firstLogin = !player->getCinematic(); // it's needed below
        if (!player->getCinematic())
        {
            player->setCinematic(1);

                if (ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->getClass()))
                {
                    Position pos;
                    player->GetPosition(&pos);
                    if (player->getRace() == RACE_NIGHTBORNE)
                        player->SendSpellScene(1900, nullptr, true, &pos);
                    else if (player->getRace() == RACE_HIGHMOUNTAIN_TAUREN)
                        player->SendSpellScene(1901, nullptr, true, &pos);
                    else if (player->getRace() == RACE_VOID_ELF)
                        player->SendSpellScene(1903, nullptr, true, &pos);
                    else if (player->getRace() == RACE_LIGHTFORGED_DRAENEI)
                        player->SendSpellScene(1902, nullptr, true, &pos);
                    else if (player->getClass() == CLASS_DEMON_HUNTER) /// @todo: find a more generic solution
                        player->SendMovieStart(469);
                    else if (cEntry->CinematicSequenceID)
                        player->SendCinematicStart(cEntry->CinematicSequenceID);
                    else if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(player->getRace()))
                        player->SendCinematicStart(rEntry->CinematicSequenceID);

                    // send new char string if not empty
                    if (!sWorld->GetNewCharString().empty())
                        chatHandler.PSendSysMessage("%s", sWorld->GetNewCharString().c_str());
                }
            
        }

        if (!player->GetMap()->AddPlayerToMap(player) || !player->GetMap()->IsGarrison() && !player->CheckInstanceLoginValid())
        {
            if (AreaTriggerStruct const* at = sAreaTriggerDataStore->GetGoBackTrigger(player->GetMapId()))
                player->TeleportTo(at->target_mapId, at->target_X, at->target_Y, at->target_Z, player->GetOrientation());
            else
                player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, player->GetOrientation());
        }

        sObjectAccessor->AddObject(player);
        //TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Player %s added to Map.", player->GetName());

        if (!player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
        {
            if (player->GetGuildId() != 0)
            {
                if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
                    guild->SendLoginInfo(this);
                else
                {
                    // remove wrong guild data
                    TC_LOG_ERROR(LOG_FILTER_GENERAL, "Player %s (GUID: %u) marked as member of not existing guild (id: %u), removing guild membership for player.", player->GetName(), player->GetGUIDLow(), player->GetGuildId());
                    player->SetInGuild(0);
                }
            }
        }

        player->UpdateVisibilityForPlayer();

        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_ONLINE);
        stmt->setUInt32(0, player->GetGUIDLow());
        CharacterDatabase.Execute(stmt);

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_ONLINE);
        stmt->setUInt32(0, GetAccountId());
        LoginDatabase.Execute(stmt);

        player->SetInGameTime(getMSTime());

        if (auto group = player->GetGroup())
        {
            group->SendUpdate();
            group->ResetMaxEnchantingLevel();
        }

        // friend status
        if (!player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS))
            sSocialMgr->SendFriendStatus(player, FRIEND_ONLINE, player->GetGUID(), true);

        // Place character in world (and load zone) before some object loading
        player->LoadCorpse();

        // setting Ghost+speed if dead
        if (player->m_deathState != ALIVE)
        {
            // not blizz like, we must correctly save and load player instead...
            if (player->getRace() == RACE_NIGHTELF)
                player->CastSpell(player, 20584, true, nullptr);// auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
            player->CastSpell(player, 8326, true, nullptr);     // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

            player->SetWaterWalking(true);
        }

        player->SendOperationsAfterDelay(OAD_LOAD_PET);

        // Set FFA PvP for non GM in non-rest mode
        if (sWorld->IsFFAPvPRealm() && !player->isGameMaster() && !player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
        {
            player->SetByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_FFA_PVP);
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);
        }

        player->UpdatePvPState(true);

        if (player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP) && !player->IsFFAPvP())
        {
            player->ApplyModFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, false);
            player->ApplyModFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_PVP_TIMER, true);
            if (!player->pvpInfo.inHostileArea && player->IsPvP())
                player->pvpInfo.endTimer = time(nullptr);
        }

        if (player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
            player->SetContestedPvP();

        // Apply at_login requests
        if (player->HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
        {
            player->resetSpells();
            SendNotification(LANG_RESET_SPELLS);
        }

        if (player->HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
        {
            player->ResetTalentSpecialization();
            player->ResetTalents(true);
            player->SendTalentsInfoData(false);              // original talents send already in to SendInitialPacketsBeforeAddToMap, resend reset state
            SendNotification(LANG_RESET_TALENTS);
        }

        if (player->HasAtLoginFlag(AT_LOGIN_FIRST))
        {
            player->RemoveAtLoginFlag(AT_LOGIN_FIRST);
            player->CreateDefaultPet();
        }

        // show time before shutdown if shutdown planned.
        if (sWorld->IsShuttingDown())
            sWorld->ShutdownMsg(true, player);

        if (sWorld->getBoolConfig(CONFIG_ALL_TAXI_PATHS))
            player->SetTaxiCheater(true);

        if (player->isGameMaster())
            SendNotification(LANG_GM_ON);

        // Hackfix Remove Talent spell - Remove Glyph spell
        player->learnSpell(111621, false); // Reset Glyph
        player->learnSpell(113873, false); // Reset Talent

        TC_LOG_INFO(LOG_FILTER_CHARACTER, "Account: %d (IP: %s) Login Character:[%s] (GUID: %u) Level: %d", this->GetAccountId(), GetRemoteAddress().c_str(), player->GetName(), player->GetGUIDLow(), player->getLevel());

        if (!player->IsStandState() && !player->HasUnitState(UNIT_STATE_STUNNED))
            player->SetStandState(UNIT_STAND_STATE_STAND);

        sScriptMgr->OnPlayerLogin(player, firstLogin);
        player->SetChangeMap(false);
    });

    delete holder;
}

void WorldSession::HandleSetFactionAtWar(WorldPackets::Character::SetFactionAtWar& packet)
{
    GetPlayer()->GetReputationMgr().SetAtWar(packet.FactionIndex, true);
}

void WorldSession::HandleUnsetFactionAtWar(WorldPackets::Character::SetFactionNotAtWar& packet)
{
    GetPlayer()->GetReputationMgr().SetAtWar(packet.FactionIndex, false);
}

void WorldSession::HandleTutorialFlag(WorldPackets::Misc::TutorialSetFlag& packet)
{
    switch (packet.Action)
    {
        case TUTORIAL_ACTION_UPDATE:
        {
            auto index = uint8(packet.TutorialBit >> 5);
            if (index >= MAX_ACCOUNT_TUTORIAL_VALUES)
                return;

            auto flag = GetTutorialInt(index);
            flag |= (1 << (packet.TutorialBit & 0x1F));
            SetTutorialInt(index, flag);
            break;
        }
        case TUTORIAL_ACTION_CLEAR:
            for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
                SetTutorialInt(i, 0xFFFFFFFF);
            break;
        case TUTORIAL_ACTION_RESET:
            for (uint8 i = 0; i < MAX_ACCOUNT_TUTORIAL_VALUES; ++i)
                SetTutorialInt(i, 0x00000000);
            break;
        default:
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "CMSG_TUTORIAL_FLAG received unknown TutorialAction %u.", packet.Action);
            break;
    }
}

void WorldSession::HandleSetWatchedFaction(WorldPackets::Character::SetWatchedFaction& packet)
{
    if (packet.FactionIndex < 0 && packet.FactionIndex != -1)
        return;

    GetPlayer()->SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, packet.FactionIndex);
}

void WorldSession::HandleSetFactionInactive(WorldPackets::Character::SetFactionInactive& packet)
{
    _player->GetReputationMgr().SetInactive(packet.Index, packet.State);
}

void WorldSession::HandleCharacterRenameRequest(WorldPackets::Character::CharacterRenameRequest& packet)
{
    if (!normalizePlayerName(packet.RenameInfo->NewName))
    {
        WorldPackets::Character::CharacterRenameResult result;
        result.Result = CHAR_NAME_NO_NAME;
        result.Name = packet.RenameInfo->NewName;
        SendPacket(result.Write());
        return;
    }

    uint8 res = sCharacterDataStore->CheckPlayerName(packet.RenameInfo->NewName, GetSessionDbcLocale(), true);
    if (res != CHAR_NAME_SUCCESS)
    {
        WorldPackets::Character::CharacterRenameResult result;
        result.Result = res;
        result.Name = packet.RenameInfo->NewName;
        SendPacket(result.Write());
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sCharacterDataStore->IsReservedName(packet.RenameInfo->NewName))
    {
        WorldPackets::Character::CharacterRenameResult result;
        result.Result = CHAR_NAME_RESERVED;
        result.Name = packet.RenameInfo->NewName;
        SendPacket(result.Write());
        return;
    }

    const CharacterInfo* nameData = sWorld->GetCharacterInfo(packet.RenameInfo->Guid);
    const CharacterInfo* newNameData = sWorld->GetCharacterInfo(packet.RenameInfo->NewName);

    if (newNameData || !nameData)
    {
        WorldPackets::Character::CharacterRenameResult result;
        result.Result = CHAR_NAME_NO_NAME;
        result.Name = packet.RenameInfo->NewName;
        SendPacket(result.Write());
        return;
    }

    ObjectGuid::LowType guidLow = packet.RenameInfo->Guid.GetCounter();
    std::string oldName = nameData->Name;

    ObjectGuid guid = packet.RenameInfo->Guid;

    // Update name and at_login flag in the db
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_NAME);

    stmt->setString(0, packet.RenameInfo->NewName);
    stmt->setUInt16(1, AT_LOGIN_RENAME);
    stmt->setUInt64(2, guidLow);

    CharacterDatabase.Execute(stmt);

    // Removed declined name from db
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt64(0, guidLow);
    CharacterDatabase.Execute(stmt);

    // Logging
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_NAME_LOG);

    stmt->setUInt64(0, guidLow);
    stmt->setString(1, oldName);
    stmt->setString(2, packet.RenameInfo->NewName);

    CharacterDatabase.Execute(stmt);

    WorldPackets::Character::CharacterRenameResult packetResult;
    packetResult.Result = RESPONSE_SUCCESS;
    packetResult.Name = packet.RenameInfo->NewName;
    packetResult.Guid = guid;
    SendPacket(packetResult.Write());
    sWorld->UpdateCharacterInfo(guid, packet.RenameInfo->NewName);
}

void WorldSession::HandleSetPlayerDeclinedNames(WorldPackets::Character::SetPlayerDeclinedNames& packet)
{
    if (!sWorld->getBoolConfig(CONFIG_DECLINED_NAMES_USED))
    {
        SendSetPlayerDeclinedNamesResult(DECLINED_NAMES_RESULT_SUCCESS, packet.Player);
        return;
    }

    std::string name;
    if (!ObjectMgr::GetPlayerNameByGUID(packet.Player, name))
    {
        SendSetPlayerDeclinedNamesResult(DECLINED_NAMES_RESULT_ERROR, packet.Player);
        return;
    }

    std::wstring wname;
    if (!Utf8toWStr(name, wname) || !isCyrillicCharacter(wname[0]))
    {
        SendSetPlayerDeclinedNamesResult(DECLINED_NAMES_RESULT_ERROR, packet.Player);
        return;
    }

    if (!sCharacterDataStore->CheckDeclinedNames(wname, packet.DeclinedNames))
    {
        SendSetPlayerDeclinedNamesResult(DECLINED_NAMES_RESULT_ERROR, packet.Player);
        return;
    }

    for (auto& i : packet.DeclinedNames.name)
        if (!normalizePlayerName(i))
        {
            SendSetPlayerDeclinedNamesResult(DECLINED_NAMES_RESULT_ERROR, packet.Player);
            return;
        }

    for (auto& i : packet.DeclinedNames.name)
        CharacterDatabase.EscapeString(i);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt64(0, packet.Player.GetCounter());
    trans->Append(stmt);
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_DECLINED_NAME);
    stmt->setUInt64(0, packet.Player.GetCounter());
    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; i++)
        stmt->setString(i + 1, packet.DeclinedNames.name[i]);
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);

    SendSetPlayerDeclinedNamesResult(DECLINED_NAMES_RESULT_SUCCESS, packet.Player);
}

void WorldSession::SendSetPlayerDeclinedNamesResult(DeclinedNameResult result, ObjectGuid guid)
{
    WorldPackets::Character::SetPlayerDeclinedNamesResult packet;
    packet.ResultCode = result;
    packet.Player = guid;
    SendPacket(packet.Write());
}

void WorldSession::HandleAlterAppearance(WorldPackets::Character::AlterApperance& packet)
{
    Player* player = GetPlayer();

    BarberShopStyleEntry const* bs_hair = sBarberShopStyleStore.LookupEntry(packet.NewHairStyle);
    if (!bs_hair || bs_hair->Type != 0 || bs_hair->Race != player->getRace() || bs_hair->Sex != player->getGender())
        return;

    BarberShopStyleEntry const* bs_facialHair = sBarberShopStyleStore.LookupEntry(packet.NewFacialHair);
    if (!bs_facialHair || bs_facialHair->Type != 2 || bs_facialHair->Race != player->getRace() || bs_facialHair->Sex != player->getGender())
        return;

    BarberShopStyleEntry const* bs_skinColor = sBarberShopStyleStore.LookupEntry(packet.NewSkinColor);
    if (bs_skinColor && (bs_skinColor->Type != 3 || bs_skinColor->Race != player->getRace() || bs_skinColor->Sex != player->getGender()))
        return;

    BarberShopStyleEntry const* bs_face = sBarberShopStyleStore.LookupEntry(packet.NewFace);
    if (bs_face && (bs_face->Type != 4 || bs_face->Race != player->getRace() || bs_face->Sex != player->getGender()))
        return;

    std::array<BarberShopStyleEntry const*, PLAYER_CUSTOM_DISPLAY_SIZE> customDisplayEntries;
    std::array<uint8, PLAYER_CUSTOM_DISPLAY_SIZE> customDisplay;
    for (auto i = 0; i < PLAYER_CUSTOM_DISPLAY_SIZE; ++i)
    {
        BarberShopStyleEntry const* bs_customDisplay = sBarberShopStyleStore.LookupEntry(packet.NewCustomDisplay[i]);
        if (bs_customDisplay && (bs_customDisplay->Type != 5 + i || bs_customDisplay->Race != player->getRace() || bs_customDisplay->Sex != player->GetGenderValue()))
            return;

        customDisplayEntries[i] = bs_customDisplay;
        customDisplay[i] = bs_customDisplay ? bs_customDisplay->Data : 0;
    }

    if (!Player::ValidateAppearance(player->getRace(), player->getClass(), player->GetGenderValue(), bs_hair->Data, packet.NewHairColor, bs_face ? bs_face->Data : player->GetFaceValue(), bs_facialHair->Data, bs_skinColor ? bs_skinColor->Data : player->GetSkinValue(), customDisplay))
        return;

    GameObject* go = player->FindNearestGameObjectOfType(GAMEOBJECT_TYPE_BARBER_CHAIR, 5.0f);
    if (!go)
    {
        SendPacket(WorldPackets::Character::BarberShopResultServer(BARBER_SHOP_RESULT_NOT_ON_CHAIR).Write());
        return;
    }

    if (player->getStandState() != UNIT_STAND_STATE_SIT_LOW_CHAIR + go->GetGOInfo()->barberChair.chairheight)
    {
        SendPacket(WorldPackets::Character::BarberShopResultServer(BARBER_SHOP_RESULT_NOT_ON_CHAIR).Write());
        return;
    }

    uint32 cost = 0;

    if (!IsHolidayActive(HOLIDAY_TRIAL_OF_STYLE))
    {
        cost = player->GetBarberShopCost(bs_hair, packet.NewHairColor, bs_facialHair, bs_skinColor, bs_face, customDisplayEntries);

        if (!player->HasEnoughMoney(static_cast<uint64>(cost)))
        {
            SendPacket(WorldPackets::Character::BarberShopResultServer(BARBER_SHOP_RESULT_NO_MONEY).Write());
            return;
        }
    }

    SendPacket(WorldPackets::Character::BarberShopResultServer(BARBER_SHOP_RESULT_SUCCESS).Write());

    player->ModifyMoney(-int64(cost));
    player->UpdateAchievementCriteria(CRITERIA_TYPE_GOLD_SPENT_AT_BARBER, cost);

    player->SetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_HAIR_STYLE_ID, uint8(bs_hair->Data));
    player->SetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_HAIR_COLOR_ID, uint8(packet.NewHairColor));
    player->SetByteValue(PLAYER_FIELD_BYTES_2, PLAYER_BYTES_2_OFFSET_FACIAL_STYLE, uint8(bs_facialHair->Data));
    if (bs_skinColor)
        player->SetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_SKIN_ID, uint8(bs_skinColor->Data));
    if (bs_face)
        player->SetByteValue(PLAYER_FIELD_BYTES_1, PLAYER_BYTES_1_FACE_ID, uint8(bs_face->Data));
    for (uint8 i = 0; i < PLAYER_CUSTOM_DISPLAY_SIZE; ++i)
        _player->SetByteValue(PLAYER_FIELD_BYTES_2, PLAYER_BYTES_2_OFFSET_CUSTOM_DISPLAY_OPTION + i, customDisplay[i]);

    player->UpdateAchievementCriteria(CRITERIA_TYPE_VISIT_BARBER_SHOP, 1);

    player->SetStandState(UNIT_STAND_STATE_STAND);
}

void WorldSession::HandleCharCustomize(WorldPackets::Character::CharCustomize& packet)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CUSTOMIZE_INFO);
    stmt->setUInt64(0, packet.CustomizeInfo->CharGUID.GetCounter());
    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSession::HandleCharCustomizeCallback, this, packet.CustomizeInfo, std::placeholders::_1)));
}

void WorldSession::HandleCharCustomizeCallback(std::shared_ptr<WorldPackets::Character::CharCustomizeInfo> customizeInfo, PreparedQueryResult result)
{
    auto SendCharCustomize = [this](ResponseCodes result, WorldPackets::Character::CharCustomizeInfo const* customizeInfo) -> void
    {
        if (result == RESPONSE_SUCCESS)
            SendPacket(WorldPackets::Character::CharCustomizeResponse(customizeInfo).Write());
        else
        {
            WorldPackets::Character::CharCustomizeFailed failed;
            failed.Result = result;
            failed.CharGUID = customizeInfo->CharGUID;
            SendPacket(failed.Write());
        }
    };

    if (!result)
    {
        SendCharCustomize(CHAR_CREATE_ERROR, customizeInfo.get());
        return;
    }

    Field* fields = result->Fetch();
    std::string oldName = fields[0].GetString();
    uint8 plrRace = fields[1].GetUInt8();
    uint8 plrClass = fields[2].GetUInt8();
    uint8 plrGender = fields[3].GetUInt8();
    uint16 atLoginFlags = fields[4].GetUInt16();

    if (!Player::ValidateAppearance(plrRace, plrClass, plrGender, customizeInfo->HairStyleID, customizeInfo->HairColorID, customizeInfo->FaceID, customizeInfo->FacialHairStyleID, customizeInfo->SkinID, customizeInfo->CustomDisplay))
    {
        SendCharCustomize(CHAR_CREATE_ERROR, customizeInfo.get());
        return;
    }

    if (!(atLoginFlags & AT_LOGIN_CUSTOMIZE))
    {
        SendCharCustomize(CHAR_CREATE_ERROR, customizeInfo.get());
        return;
    }

    atLoginFlags &= ~AT_LOGIN_CUSTOMIZE;

    if (!normalizePlayerName(customizeInfo->CharName))
    {
        SendCharCustomize(CHAR_NAME_NO_NAME, customizeInfo.get());
        return;
    }

    ResponseCodes res = sCharacterDataStore->CheckPlayerName(customizeInfo->CharName, GetSessionDbcLocale(), true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharCustomize(res, customizeInfo.get());
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sCharacterDataStore->IsReservedName(customizeInfo->CharName))
    {
        SendCharCustomize(CHAR_NAME_RESERVED, customizeInfo.get());
        return;
    }

    if (ObjectGuid newguid = ObjectMgr::GetPlayerGUIDByName(customizeInfo->CharName))
        if (newguid != customizeInfo->CharGUID)
        {
            SendCharCustomize(CHAR_CREATE_NAME_IN_USE, customizeInfo.get());
            return;
        }

    uint16 atLoginFlag = oldName != customizeInfo->CharName ? AT_LOGIN_CUSTOMIZE | AT_LOGIN_RENAME : AT_LOGIN_CUSTOMIZE;

    ObjectGuid::LowType lowGuid = customizeInfo->CharGUID.GetCounter();
    Player::Customize(lowGuid, customizeInfo.get());

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_NAME);
    stmt->setUInt64(0, lowGuid);
    if (result = CharacterDatabase.Query(stmt))
        TC_LOG_INFO(LOG_FILTER_CHARACTER, "Account: %d (IP: %s), Character[%s] (guid:%u) Customized to: %s", GetAccountId(), GetRemoteAddress().c_str(), result->Fetch()[0].GetString().c_str(), lowGuid, customizeInfo->CharName.c_str());

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_NAME_AT_LOGIN);
    stmt->setString(0, customizeInfo->CharName);
    stmt->setUInt16(1, atLoginFlag);
    stmt->setUInt64(2, lowGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt64(0, lowGuid);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    sWorld->UpdateCharacterInfo(customizeInfo->CharGUID, customizeInfo->CharName, customizeInfo->SexID);

    SendCharCustomize(RESPONSE_SUCCESS, customizeInfo.get());
}

void WorldSession::HandleCharRaceOrFactionChange(WorldPackets::Character::CharRaceOrFactionChange& packet)
{
    auto SendCharFactionChange = [this](ResponseCodes result, WorldPackets::Character::CharRaceOrFactionChangeInfo const* factionChangeInfo) -> void
    {
        WorldPackets::Character::CharFactionChangeResult packet;
        packet.Result = result;
        packet.Guid = factionChangeInfo->Guid;

        if (result == RESPONSE_SUCCESS)
        {
            packet.Display = boost::in_place();
            packet.Display->Name = factionChangeInfo->Name;
            packet.Display->SexID = factionChangeInfo->SexID;
            packet.Display->SkinID = factionChangeInfo->SkinID;
            packet.Display->HairColorID = factionChangeInfo->HairColorID;
            packet.Display->HairStyleID = factionChangeInfo->HairStyleID;
            packet.Display->FacialHairStyleID = factionChangeInfo->FacialHairStyleID;
            packet.Display->FaceID = factionChangeInfo->FaceID;
            packet.Display->RaceID = factionChangeInfo->RaceID;
            packet.Display->CustomDisplay = factionChangeInfo->CustomDisplay;
        }

        SendPacket(packet.Write());
    };

    auto info = packet.RaceOrFactionChangeInfo.get();
    auto lowGuid = info->Guid.GetCounter();

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_CLASS_LVL_AT_LOGIN);
    stmt->setUInt64(0, lowGuid);
    auto result = CharacterDatabase.Query(stmt);
    if (!result)
    {
        SendCharFactionChange(CHAR_CREATE_ERROR, info);
        return;
    }

    TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d Started faction change", GetAccountId());

    Field* fields = result->Fetch();
    uint8  oldRace = fields[0].GetUInt8();
    auto playerClass = uint32(fields[1].GetUInt8());
    auto level = uint32(fields[2].GetUInt8());
    uint32 at_loginFlags = fields[3].GetUInt16();
    uint32 used_loginFlag = info->FactionChange ? AT_LOGIN_CHANGE_FACTION : AT_LOGIN_CHANGE_RACE;
    std::string knownTitlesStr = fields[4].GetString();
    std::string oldName = fields[5].GetString();

    TeamId oldTeam = TEAM_ALLIANCE;

    // Search each faction is targeted
    switch (oldRace)
    {
        case RACE_ORC:
        case RACE_GOBLIN:
        case RACE_TAUREN:
        case RACE_UNDEAD_PLAYER:
        case RACE_TROLL:
        case RACE_BLOODELF:
        case RACE_PANDAREN_HORDE:
        case RACE_NIGHTBORNE:
        case RACE_HIGHMOUNTAIN_TAUREN:
            oldTeam = TEAM_HORDE;
            break;
        default:
            break;
    }

    // We need to correct race when it's pandaren
    // Because client always send neutral ID
    if (info->RaceID == RACE_PANDAREN_NEUTRAL)
    {
        if (used_loginFlag == AT_LOGIN_CHANGE_RACE)
            info->RaceID = oldTeam == TEAM_ALLIANCE ? RACE_PANDAREN_ALLIANCE : RACE_PANDAREN_HORDE;
        else
            info->RaceID = oldTeam == TEAM_ALLIANCE ? RACE_PANDAREN_HORDE : RACE_PANDAREN_ALLIANCE;
    }

    if (!sObjectMgr->GetPlayerInfo(info->RaceID, playerClass))
    {
        SendCharFactionChange(CHAR_CREATE_ERROR, info);
        return;
    }

    if (!(at_loginFlags & used_loginFlag))
    {
        SendCharFactionChange(CHAR_CREATE_ERROR, info);
        return;
    }

    if (AccountMgr::IsPlayerAccount(GetSecurity()))
        if ((1 << (info->RaceID - 1)) & sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK))
        {
            SendCharFactionChange(CHAR_CREATE_ERROR, info);
            return;
        }

    // prevent character rename to invalid name
    if (!normalizePlayerName(info->Name))
    {
        SendCharFactionChange(CHAR_NAME_NO_NAME, info);
        return;
    }

    ResponseCodes res = sCharacterDataStore->CheckPlayerName(info->Name, GetSessionDbcLocale(), true);
    if (res != CHAR_NAME_SUCCESS)
    {
        SendCharFactionChange(res, info);
        return;
    }

    // check name limitations
    if (AccountMgr::IsPlayerAccount(GetSecurity()) && sCharacterDataStore->IsReservedName(info->Name))
    {
        SendCharFactionChange(CHAR_NAME_RESERVED, info);
        return;
    }

    // character with this name already exist
    ObjectGuid newguid = ObjectMgr::GetPlayerGUIDByName(info->Name);
    if (!newguid.IsEmpty())
        if (newguid != info->Guid)
        {
            SendCharFactionChange(CHAR_CREATE_NAME_IN_USE, info);
            return;
        }

    CharacterDatabase.EscapeString(info->Name);
    Player::Customize(lowGuid, info);
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if ((info->Name) != oldName)
        used_loginFlag |= AT_LOGIN_RENAME;
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_FACTION_OR_RACE);
    stmt->setString(0, info->Name);
    stmt->setUInt8(1, info->RaceID);
    stmt->setUInt16(2, used_loginFlag);
    stmt->setUInt64(3, lowGuid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_FACTION_OR_RACE_LOG);
    stmt->setUInt64(0, lowGuid);
    stmt->setUInt32(1, GetAccountId());
    stmt->setUInt8(2, oldRace);
    stmt->setUInt8(3, info->RaceID);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_DECLINED_NAME);
    stmt->setUInt64(0, lowGuid);
    trans->Append(stmt);

    sWorld->UpdateCharacterInfo(info->Guid, info->Name, info->SexID, info->RaceID);

    TeamId team = TEAM_ALLIANCE;

    // Search each faction is targeted
    switch (info->RaceID)
    {
        case RACE_ORC:
        case RACE_GOBLIN:
        case RACE_TAUREN:
        case RACE_UNDEAD_PLAYER:
        case RACE_TROLL:
        case RACE_BLOODELF:
        case RACE_PANDAREN_HORDE:
        case RACE_NIGHTBORNE:
        case RACE_HIGHMOUNTAIN_TAUREN:
            team = TEAM_HORDE;
            break;
        default:
            break;
    }

    // Switch Languages
    // delete all languages first
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SKILL_LANGUAGES);
    stmt->setUInt64(0, lowGuid);
    trans->Append(stmt);

    // Now add them back
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SKILL_LANGUAGE);
    stmt->setUInt64(0, lowGuid);

    // Faction specific languages
    if (team == TEAM_HORDE)
        stmt->setUInt16(1, 109);
    else
        stmt->setUInt16(1, 98);

    trans->Append(stmt);

    // Race specific languages

    if (info->RaceID != RACE_ORC && info->RaceID != RACE_HUMAN)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SKILL_LANGUAGE);
        stmt->setUInt64(0, lowGuid);

        uint16 raceLang = 0;
        switch (info->RaceID)
        {
            case RACE_DWARF:
                raceLang = 111;
                break;
            case RACE_DRAENEI:
            case RACE_LIGHTFORGED_DRAENEI: 
                raceLang = 759;
                break;
            case RACE_GNOME:
                raceLang = 313;
                break;
            case RACE_NIGHTELF:
                raceLang = 113;
                break;
            case RACE_WORGEN:
                raceLang = 791;
                break;
            case RACE_UNDEAD_PLAYER:
                raceLang = 673;
                break;
            case RACE_TAUREN:
            case RACE_HIGHMOUNTAIN_TAUREN:
                raceLang = 115;
                break;
            case RACE_TROLL:
                raceLang = 315;
                break;
            case RACE_BLOODELF:
                raceLang = 137;
                break;
            case RACE_GOBLIN:
                raceLang = 792;
                break;
            case RACE_NIGHTBORNE:
                raceLang = 2464;
                break;
            case RACE_VOID_ELF:
                raceLang = 2465;
                break;
            default:
                break;
        }

        stmt->setUInt16(1, raceLang);
        trans->Append(stmt);
    }

    if (used_loginFlag & AT_LOGIN_CHANGE_FACTION)
    {
        // Delete all Flypaths
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_TAXI_PATH);
        stmt->setUInt64(0, lowGuid);
        trans->Append(stmt);

        if (level > 7)
        {
            // Update Taxi path
            // this doesn't seem to be 100% blizzlike... but it can't really be helped.

            // Disable, because this code freeze server on line taximaskstream << uint32(factionMask[i] | deathKnightExtraNode) << ' ';
            // std::ostringstream taximaskstream;
            // TaxiMask const& factionMask = team == TEAM_HORDE ? sHordeTaxiNodesMask : sAllianceTaxiNodesMask;
            // for (uint8 i = 0; i < TaxiMaskSize; ++i)
            // {
                // // i = (315 - 1) / 8 = 39
                // // m = 1 << ((315 - 1) % 8) = 4
                // uint8 deathKnightExtraNode = playerClass != CLASS_DEATH_KNIGHT || i != 39 ? 0 : 4;
                // taximaskstream << uint32(factionMask[i] | deathKnightExtraNode) << ' ';
            // }

            // stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_TAXIMASK);
            // stmt->setString(0, taximaskstream.str());
            // stmt->setUInt64(1, lowGuid);
            // trans->Append(stmt);
        }

        // Delete all current quests
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_QUESTSTATUS);
        stmt->setUInt64(0, lowGuid);
        trans->Append(stmt);

        // Delete record of the faction old completed quests
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_QUESTSTATUSREW_NON_ACC);
        stmt->setUInt32(0, GetAccountId());
        stmt->setUInt64(1, lowGuid);
        result = CharacterDatabase.Query(stmt);
        if (result)
        {
            std::ostringstream quests;
            do
            {
                if (Quest const* qinfo = sQuestDataStore->GetQuestTemplate(result->Fetch()[0].GetUInt32()))
                {
                    uint64 requiredRaces = qinfo->AllowableRaces;
                    if (!requiredRaces)
                        continue;

                    uint64 newRaceMask = (team == TEAM_ALLIANCE) ? RACEMASK_ALLIANCE : RACEMASK_HORDE;
                    if (requiredRaces != uint64(-1) && !(requiredRaces & newRaceMask))
                    {
                        quests << uint32(qinfo->GetQuestId());
                        quests << ',';
                    }
                }

            } while (result->NextRow());

            std::string questsStr = quests.str();
            questsStr = questsStr.substr(0, questsStr.length() - 1);

            if (!questsStr.empty())
                trans->PAppend("DELETE FROM `character_queststatus_rewarded` WHERE guid='%u' AND quest IN (%s)", lowGuid, questsStr.c_str());
        }

        if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
        {
            // Reset guild
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_MEMBER);
            stmt->setUInt64(0, lowGuid);
            if (result = CharacterDatabase.Query(stmt))
                if (Guild* guild = sGuildMgr->GetGuildById((result->Fetch()[0]).GetUInt64()))
                    guild->DeleteMember(ObjectGuid::Create<HighGuid::Player>(lowGuid));
        }

        if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND))
        {
            // Delete Friend List
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SOCIAL_BY_GUID);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SOCIAL_BY_FRIEND);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);
        }

        // Reset homebind and position
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_HOMEBIND);
        stmt->setUInt64(0, lowGuid);
        trans->Append(stmt);

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change. Home bind start", GetAccountId());

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_PLAYER_HOMEBIND);
        stmt->setUInt64(0, lowGuid);
        if (team == TEAM_ALLIANCE)
        {
            stmt->setUInt16(1, 0);
            stmt->setUInt16(2, 1519);
            stmt->setFloat(3, -8867.68f);
            stmt->setFloat(4, 673.373f);
            stmt->setFloat(5, 97.9034f);
            Player::SavePositionInDB(0, -8867.68f, 673.373f, 97.9034f, 0.0f, 1519, info->Guid);
        }
        else
        {
            stmt->setUInt16(1, 1);
            stmt->setUInt16(2, 1637);
            stmt->setFloat(3, 1633.33f);
            stmt->setFloat(4, -4439.11f);
            stmt->setFloat(5, 17.7588f);
            Player::SavePositionInDB(1, 1633.33f, -4439.11f, 17.7588f, 0.0f, 1637, info->Guid);
        }
        trans->Append(stmt);

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change, HOME BIND success", GetAccountId());

        // Achievement conversion
        for (auto it : sCharacterDataStore->GetFactionChangeAchievements())
        {
            uint32 achiev_alliance = it.first;
            uint32 achiev_horde = it.second;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_ACHIEVEMENT_BY_ACHIEVEMENT);
            stmt->setUInt16(0, uint16(team == TEAM_ALLIANCE ? achiev_alliance : achiev_horde));
            stmt->setUInt64(1, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_ACHIEVEMENT);
            stmt->setUInt16(0, uint16(team == TEAM_ALLIANCE ? achiev_alliance : achiev_horde));
            stmt->setUInt16(1, uint16(team == TEAM_ALLIANCE ? achiev_horde : achiev_alliance));
            stmt->setUInt64(2, lowGuid);
            trans->Append(stmt);
        }

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change Achievements success", GetAccountId());
        // Item conversion
        for (auto it : sCharacterDataStore->GetFactionChangeItems())
        {
            uint32 item_alliance = it.first;
            uint32 item_horde = it.second;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_INVENTORY_FACTION_CHANGE);
            stmt->setUInt32(0, (team == TEAM_ALLIANCE ? item_alliance : item_horde));
            stmt->setUInt32(1, (team == TEAM_ALLIANCE ? item_horde : item_alliance));
            stmt->setUInt64(2, lowGuid);
            trans->Append(stmt);
        }

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change Items success", GetAccountId());

        // Spell conversion
        for (auto it : sCharacterDataStore->GetFactionChangeSpells())
        {
            uint32 spell_alliance = it.first;
            uint32 spell_horde = it.second;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_BY_SPELL);
            stmt->setUInt32(0, (team == TEAM_ALLIANCE ? spell_alliance : spell_horde));
            stmt->setUInt64(1, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_SPELL_FACTION_CHANGE);
            stmt->setUInt32(0, (team == TEAM_ALLIANCE ? spell_alliance : spell_horde));
            stmt->setUInt32(1, (team == TEAM_ALLIANCE ? spell_horde : spell_alliance));
            stmt->setUInt64(2, lowGuid);
            trans->Append(stmt);
        }

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change spells success", GetAccountId());

        // Reputation conversion
        for (auto it : sCharacterDataStore->GetFactionChangeReputation())
        {
            uint32 reputation_alliance = it.first;
            uint32 reputation_horde = it.second;

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_REP_BY_FACTION);
            stmt->setUInt32(0, uint16(team == TEAM_ALLIANCE ? reputation_alliance : reputation_horde));
            stmt->setUInt64(1, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_REP_FACTION_CHANGE);
            stmt->setUInt16(0, uint16(team == TEAM_ALLIANCE ? reputation_alliance : reputation_horde));
            stmt->setUInt16(1, uint16(team == TEAM_ALLIANCE ? reputation_horde : reputation_alliance));
            stmt->setUInt64(2, lowGuid);
            trans->Append(stmt);
        }

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change reputation success", GetAccountId());
        // Title conversion
        if (!knownTitlesStr.empty())
        {
            const uint32 ktcount = KNOWN_TITLES_SIZE * 2;
            uint32 knownTitles[ktcount];
            Tokenizer tokens(knownTitlesStr, ' ', ktcount);

            TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change titles process... tokens size %u ktcount %u input %s", GetAccountId(), tokens.size(), ktcount, knownTitlesStr.c_str());

            if (tokens.size() != ktcount)
                return;

            for (uint32 index = 0; index < ktcount; ++index)
                knownTitles[index] = atol(tokens[index]);

            for (auto it : sCharacterDataStore->GetFactionChangeTitles())
            {
                uint32 title_alliance = it.first;
                uint32 title_horde = it.second;

                CharTitlesEntry const* atitleInfo = sCharTitlesStore.LookupEntry(title_alliance);
                CharTitlesEntry const* htitleInfo = sCharTitlesStore.LookupEntry(title_horde);
                // new team
                if (team == TEAM_ALLIANCE)
                {
                    uint32 bitIndex = htitleInfo->MaskID;
                    uint32 index = bitIndex / 32;
                    uint32 old_flag = 1 << (bitIndex % 32);
                    uint32 new_flag = 1 << (atitleInfo->MaskID % 32);
                    if (knownTitles[index] & old_flag)
                    {
                        knownTitles[index] &= ~old_flag;
                        // use index of the new title
                        knownTitles[atitleInfo->MaskID / 32] |= new_flag;
                    }
                }
                else
                {
                    uint32 bitIndex = atitleInfo->MaskID;
                    uint32 index = bitIndex / 32;
                    uint32 old_flag = 1 << (bitIndex % 32);
                    uint32 new_flag = 1 << (htitleInfo->MaskID % 32);
                    if (knownTitles[index] & old_flag)
                    {
                        knownTitles[index] &= ~old_flag;
                        // use index of the new title
                        knownTitles[htitleInfo->MaskID / 32] |= new_flag;
                    }
                }

                std::ostringstream ss;
                for (auto knownTitle : knownTitles)
                    ss << knownTitle << ' ';

                stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_TITLES_FACTION_CHANGE);
                stmt->setString(0, ss.str());
                stmt->setUInt64(1, lowGuid);
                trans->Append(stmt);

                // unset any currently chosen title
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_RES_CHAR_TITLES_FACTION_CHANGE);
                stmt->setUInt64(0, lowGuid);
                trans->Append(stmt);
            }
        }

        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change titles success", GetAccountId());

        if (level >= 101)
        {
            // Unload map if exist
            sMapMgr->SetUnloadGarrison(lowGuid);

            // delete info about garrison
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_ONLY);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BLUEPRINTS);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_BUILDINGS);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            /*
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_FOLLOWERS);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_MISSIONS);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_SHIPMENTS);
            stmt->setUInt64(0, lowGuid);
            trans->Append(stmt);

            // stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_GARRISON_TALENTS);
            // stmt->setUInt64(0, lowGuid);
            // trans->Append(stmt);
            */

            trans->PAppend("insert into `character_reward` (`id`, `type`, `owner_guid`)value ('%u','10', '%u')", (team == TEAM_HORDE ? 71 : 2), lowGuid); // for recreate new garrison
        }
        TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change actions 110 success", GetAccountId());
    }

    CharacterDatabase.CommitTransaction(trans);
    TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change commit transaction success", GetAccountId());

    TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d (IP: %s), Character guid: %u Change Race/Faction to: %s", GetAccountId(), GetRemoteAddress().c_str(), lowGuid, info->Name.c_str());

    SendCharFactionChange(RESPONSE_SUCCESS, info);
    TC_LOG_DEBUG(LOG_FILTER_UNITS, "Account: %d faction change success end", GetAccountId());
}

void WorldSession::HandleGenerateRandomCharacterName(WorldPackets::Character::GenerateRandomCharacterName& packet)
{
    WorldPackets::Character::GenerateRandomCharacterNameResult result;
    if (!Player::IsValidRace(packet.Race))
    {
        SendPacket(result.Write());
        return;
    }

    if (!Player::IsValidGender(packet.Sex))
    {
        SendPacket(result.Write());
        return;
    }

    result.Success = true;
    result.Name = sDB2Manager.GetNameGenEntry(packet.Race, packet.Sex);
    SendPacket(result.Write());
}

void WorldSession::HandleReorderCharacters(WorldPackets::Character::ReorderCharacters& packet)
{
    auto trans = CharacterDatabase.BeginTransaction();
    for (auto const& reorderInfo : packet.Entries)
    {
        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_LIST_SLOT);
        stmt->setUInt8(0, reorderInfo.NewPosition);
        stmt->setUInt64(1, reorderInfo.PlayerGUID.GetCounter());
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);
}

void WorldSession::HandleSaveCUFProfiles(WorldPackets::Misc::SaveCUFProfiles& packet)
{
    Player* player = GetPlayer();
    if (packet.CUFProfiles.size() > MAX_CUF_PROFILES || !player)
        return;

    for (uint8 i = 0; i < packet.CUFProfiles.size(); ++i)
        player->SaveCUFProfile(i, std::move(packet.CUFProfiles[i]));

    for (uint8 i = packet.CUFProfiles.size(); i < MAX_CUF_PROFILES; ++i)
        player->SaveCUFProfile(i, nullptr);
}

void WorldSession::SendLoadCUFProfiles()
{
    Player* player = GetPlayer();
    if (!player)
        return;

    WorldPackets::Misc::LoadCUFProfiles loadCUFProfiles;
    for (uint8 i = 0; i < MAX_CUF_PROFILES; i++)
        if (CUFProfile const* cufProfile = player->GetCUFProfile(i))
            loadCUFProfiles.CUFProfiles.push_back(cufProfile);
    SendPacket(loadCUFProfiles.Write());
}

void WorldSession::HandleSetAdvancedCombatLogging(WorldPackets::ClientConfig::SetAdvancedCombatLogging& packet)
{
    _player->SetAdvancedCombatLogging(packet.Enable);
}

void WorldSession::HandleContinuePlayerLogin()
{
    if (!PlayerLoading() || GetPlayer())
    {
        KickPlayer();
        return;
    }

    auto holder = new LoginQueryHolder(GetAccountId(), m_playerLoading);
    if (!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading.Clear();
        return;
    }

    TC_LOG_INFO(LOG_FILTER_OPCODES, "WorldSession::HandleContinuePlayerLogin");

    SendPacket(WorldPackets::Auth::ResumeComms(CONNECTION_TYPE_INSTANCE).Write());
    _charLoginCallback = CharacterDatabase.DelayQueryHolder(holder);
}

void WorldSession::AbortLogin(WorldPackets::Character::LoginFailureReason reason)
{
    if (!PlayerLoading() || GetPlayer())
    {
        KickPlayer();
        return;
    }

    m_playerLoading.Clear();
    SendPacket(WorldPackets::Character::CharacterLoginFailed(reason).Write());
}

void WorldSession::HandleSetActionBarToggles(WorldPackets::Character::SetActionBarToggles& packet)
{
    if (!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if (packet.Mask != 0)
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "WorldSession::HandleSetActionBarToggles in not logged state with value: %u, ignored", packet.Mask);
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES_4, PLAYER_BYTES_4_ACTION_BAR_TOGGLES, packet.Mask);
}

void WorldSession::HandleRequestPlayedTime(WorldPackets::Character::RequestPlayedTime& packet)
{
    WorldPackets::Character::PlayedTime playedTime;
    playedTime.TotalTime = _player->GetTotalPlayedTime();
    playedTime.LevelTime = _player->GetLevelPlayedTime();
    playedTime.TriggerEvent = packet.TriggerScriptEvent;
    SendPacket(playedTime.Write());
}

void WorldSession::HandleSetTitle(WorldPackets::Character::SetTitle& packet)
{
    if (packet.TitleID > 0 && packet.TitleID < MAX_TITLE_INDEX)
    {
        if (!GetPlayer()->HasTitle(packet.TitleID))
            return;
    }
    else
        packet.TitleID = 0;

    GetPlayer()->SetUInt32Value(PLAYER_FIELD_PLAYER_TITLE, packet.TitleID);
}

void WorldSession::HandleLogoutRequest(WorldPackets::Character::LogoutRequest& /*packet*/)
{
    Player* player = GetPlayer();

    ObjectGuid lguid = player->GetLootGUID();
    if (!lguid.IsEmpty())
        DoLootRelease(lguid);

    uint32 reason = 0;
    if (player->isInCombat())
        reason = 1;
    else if (player->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR))
        reason = 3;                                         // is jumping or falling
    else if (player->duel || player->HasAura(9454)) // is dueling or frozen by GM via freeze command
        reason = 2;                                         // FIXME - Need the correct value
    else if (auto instance = player->GetInstanceScript())
        if (!player->isGameMaster() && instance->IsEncounterInProgress()) // Don`t res if instance in progress
            reason = 1;

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in worldserver.conf
    bool instantLogout = player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || player->isInFlight() ||
        GetSecurity() >= AccountTypes(sWorld->getIntConfig(CONFIG_INSTANT_LOGOUT)) || player->GetMapId() == 1179;//duel zone

    WorldPackets::Character::LogoutResponse logoutResponse;
    logoutResponse.LogoutResult = reason;
    logoutResponse.Instant = instantLogout;
    SendPacket(logoutResponse.Write());

    if (reason)
    {
        LogoutRequest(0);
        return;
    }

    // instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in worldserver.conf
    if (instantLogout)
    {
        // LogoutPlayer(true);
        player->SetRemoveFromMap(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (player->CanFreeMove())
    {
        player->SetStandState(UNIT_STAND_STATE_SIT);
        player->SetRooted(true);
        player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    LogoutRequest(time(nullptr));
}

void WorldSession::HandleLogoutInstant(WorldPackets::Character::LogoutInstant& /*packet*/)
{
}

void WorldSession::HandleSetCurrencyFlags(WorldPackets::Character::SetCurrencyFlags& packet)
{
    GetPlayer()->ModifyCurrencyFlag(packet.CurrencyID, packet.Flags);
}

void WorldSession::HandleRequestRaidInfo(WorldPackets::Party::RequestRaidInfo& /*packet*/)
{
    _player->SendRaidInfo();
}

void WorldSession::HandleSetSavedInstanceExtend(WorldPackets::Calendar::SetSavedInstanceExtend& packet)
{
    auto instanceBind = _player->GetBoundInstance(packet.MapID, Difficulty(packet.DifficultyID));
    if (!instanceBind || !instanceBind->perm || _player->GetMap()->IsGarrison())
        return;

    auto instanceSave = instanceBind->save;
    if (!instanceSave)
        return;

    if (packet.Extend == instanceSave->GetExtended())
        return;

    instanceSave->SetExtended(packet.Extend);
    _player->SendRaidInfo();
    _player->UpdateInstance(instanceSave);
}

void WorldSession::HandleUndeleteCharacter(WorldPackets::Character::UndeleteCharacter& packet)
{
    auto SendUndeleteCharacterResponse = [this](CharacterUndeleteResult result, WorldPackets::Character::CharacterUndeleteInfo const* undeleteInfo) -> void
    {
        WorldPackets::Character::UndeleteCharacterResponse response;
        response.UndeleteInfo = undeleteInfo;
        response.Result = result;
        SendPacket(response.Write());
    };

    if (!HasAuthFlag(AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER))
    {
        SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_ERROR_DISABLED, packet.UndeleteInfo.get());
        return;
    }

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_LAST_CHAR_UNDELETE);
    stmt->setUInt32(0, GetAccountId());

    auto undeleteInfo = packet.UndeleteInfo;
    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithChainingPreparedCallback([undeleteInfo, SendUndeleteCharacterResponse](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (result)
        {
            auto lastUndelete = result->Fetch()[0].GetUInt32();
            if (lastUndelete && (lastUndelete + uint32(MONTH) > time(nullptr)))
            {
                SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_ERROR_COOLDOWN, undeleteInfo.get());
                return;
            }
        }

        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHAR_DEL_INFO_BY_GUID);
        stmt->setUInt64(0, undeleteInfo->CharacterGuid.GetCounter());
        queryCallback.SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    }).WithChainingPreparedCallback([this, undeleteInfo, SendUndeleteCharacterResponse](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (!result)
        {
            SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_ERROR_CHAR_CREATE, undeleteInfo.get());
            return;
        }

        auto fields = result->Fetch();
        undeleteInfo->Name = fields[1].GetString();
        if (fields[2].GetUInt32() != GetAccountId())
        {
            SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_ERROR_UNKNOWN, undeleteInfo.get());
            return;
        }

        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, undeleteInfo->Name);
        queryCallback.SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    }).WithChainingPreparedCallback([this, undeleteInfo, SendUndeleteCharacterResponse](QueryCallback& queryCallback, PreparedQueryResult result)
    {
        if (result)
        {
            SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_ERROR_NAME_TAKEN_BY_THIS_ACCOUNT, undeleteInfo.get());
            return;
        }

        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
        stmt->setUInt32(0, GetAccountId());
        queryCallback.SetNextQuery(CharacterDatabase.AsyncQuery(stmt));
    }).WithPreparedCallback([this, undeleteInfo, SendUndeleteCharacterResponse](PreparedQueryResult result)
    {
        if (result)
        {
            if (result->Fetch()[0].GetUInt64() >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM)) // SQL's COUNT() returns uint64 but it will always be less than uint8.Max
            {
                SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_ERROR_CHAR_CREATE, undeleteInfo.get());
                return;
            }
        }

        auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_RESTORE_DELETE_INFO);
        stmt->setString(0, undeleteInfo->Name);
        stmt->setUInt32(1, GetAccountId());
        stmt->setUInt64(2, undeleteInfo->CharacterGuid.GetCounter());
        CharacterDatabase.Execute(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_LAST_CHAR_UNDELETE);
        stmt->setUInt32(0, GetAccountId());
        CharacterDatabase.Execute(stmt);

        sWorld->UpdateCharacterInfoDeleted(undeleteInfo->CharacterGuid.GetCounter(), false, &undeleteInfo->Name);

        RemoveAuthFlag(AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER);
        SendUndeleteCharacterResponse(CHARACTER_UNDELETE_RESULT_OK, undeleteInfo.get());
    }));
}

void WorldSession::HandleGetUndeleteCharacterCooldownStatus(WorldPackets::Character::GetUndeleteCharacterCooldownStatus& /*packet*/)
{
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_LAST_CHAR_UNDELETE);
    stmt->setUInt32(0, GetAccountId());
    _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt).WithPreparedCallback(std::bind(&WorldSession::HandleUndeleteCooldownStatusCallback, this, std::placeholders::_1)));
}

void WorldSession::HandleUndeleteCooldownStatusCallback(PreparedQueryResult const& result)
{
    uint32 cooldown = 0;
    if (result)
    {
        auto now = uint32(time(nullptr));
        auto undeleteTime = result->Fetch()[0].GetUInt32() + uint32(MONTH);
        if (undeleteTime > now)
            cooldown = std::max<uint32>(0, undeleteTime - now);
    }

    WorldPackets::Character::UndeleteCooldownStatusResponse response;
    response.OnCooldown = cooldown > 0;
    response.MaxCooldown = uint32(MONTH);
    response.CurrentCooldown = cooldown;
    SendPacket(response.Write());
}

void WorldSession::HandleEngineSurvey(WorldPackets::Character::EngineSurvey& packet)
{
    std::hash<std::string> hash_gen;
    std::string baseData(
    "TotalPhysMemory:" + std::to_string(packet.TotalPhysMemory) + 
    "GPUVideoMemory:" + std::to_string(packet.GPUVideoMemory) + 
    "GPUSystemMemory:" + std::to_string(packet.GPUSystemMemory) + 
    "GPUSharedMemory:" + std::to_string(packet.GPUSharedMemory) + 
    "GPUVendorID:" + std::to_string(packet.GPUVendorID) + 
    "GPUModelID:" + std::to_string(packet.GPUModelID) + 
    "ProcessorUnkUnk:" + std::to_string(packet.ProcessorUnkUnk) + 
    "ProcessorFeatures:" + std::to_string(packet.ProcessorFeatures) + 
    "ProcessorVendor:" + std::to_string(packet.ProcessorVendor) + 
    "ProcessorNumberOfProcessors:" + std::to_string(packet.ProcessorNumberOfProcessors) + 
    "ProcessorNumberOfThreads:" + std::to_string(packet.ProcessorNumberOfThreads) + 
    "SystemOSIndex:" + std::to_string(packet.SystemOSIndex) + 
    "Is64BitSystem:" + std::to_string(packet.Is64BitSystem)
    );

    auto str_hash = hash_gen(baseData);
    if (_hwid == str_hash) // Not need update
        return;

    _hwid = str_hash;

    LoginDatabase.PExecute("UPDATE account SET hwid = " UI64FMTD " WHERE id = %u;", _hwid, GetAccountId());

    if (!_hwid)
        return;

    if (auto result = LoginDatabase.PQuery("SELECT penalties, last_reason from hwid_penalties where hwid = " UI64FMTD, _hwid))
    {
        auto fields = result->Fetch();
        _countPenaltiesHwid = fields[0].GetInt32();

        if ((sWorld->getIntConfig(CONFIG_ANTI_FLOOD_HWID_BANS_COUNT) && _countPenaltiesHwid >= sWorld->getIntConfig(CONFIG_ANTI_FLOOD_HWID_BANS_COUNT)) || _countPenaltiesHwid < 0)
        {
            std::stringstream ss;
            ss << (fields[1].GetString().empty() ? "Antiflood unknwn" : fields[1].GetCString()) << "*";

            if (sWorld->getBoolConfig(CONFIG_ANTI_FLOOD_HWID_BANS_ALLOW))
                sWorld->BanAccount(BAN_ACCOUNT, GetAccountName(), "-1", ss.str(), "Server");
            else if (sWorld->getBoolConfig(CONFIG_ANTI_FLOOD_HWID_MUTE_ALLOW))
                sWorld->MuteAccount(GetAccountId(), -1, ss.str(), "Server", this);
            else if (sWorld->getBoolConfig(CONFIG_ANTI_FLOOD_HWID_KICK_ALLOW))
                KickPlayer();
        }
    }
}
