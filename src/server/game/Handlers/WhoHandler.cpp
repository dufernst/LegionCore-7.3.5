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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "AccountMgr.h"
#include "Opcodes.h"
#include "WhoPackets.h"
#include "Guild.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GlobalFunctional.h"
#include "ScriptMgr.h"

void WorldSession::HandleWhoOpcode(WorldPackets::Who::WhoRequestPkt& whoRequest)
{
    WorldPackets::Who::WhoRequest& request = whoRequest.Request;

    if (whoRequest.Areas.size() > 10)
        return;

    if (request.Words.size() > 4)
        return;

    std::vector<std::wstring> wWords;
    wWords.resize(request.Words.size());
    for (size_t i = 0; i < request.Words.size(); ++i)
    {
        if (!Utf8toWStr(request.Words[i].Word, wWords[i]))
            continue;

        wstrToLower(wWords[i]);
    }

    std::wstring wPlayerName;
    std::wstring wGuildName;

    if (!(Utf8toWStr(request.Name, wPlayerName) && Utf8toWStr(request.Guild, wGuildName)))
        return;

    wstrToLower(wPlayerName);
    wstrToLower(wGuildName);

    if (whoRequest.Request.MaxLevel >= MAX_LEVEL)
        whoRequest.Request.MaxLevel = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    uint32 security = GetSecurity();
    bool allowTwoSideWhoList = sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    uint32 gmLevelInWhoList  = sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);
    uint8 displaycount = 0;
    Player* target = nullptr;

    WorldPackets::Who::WhoResponsePkt response;

    std::set<ObjectGuid> playerGuids{};
    HashMapHolder<Player>::GetLock().lock_shared();

    HashMapHolder<Player>::MapType const& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        target = itr->second;
        if (AccountMgr::IsPlayerAccount(security))
        {
            if (target->GetTeam() != team && !allowTwoSideWhoList)
                continue;

            if (target->GetSession()->GetSecurity() > AccountTypes(gmLevelInWhoList))
                continue;
        }

        if (!target || !target->IsInWorld())
            continue;

        if (!target->IsVisibleGloballyFor(_player))
            continue;

        uint8 lvl = target->getLevel();
        if (lvl < request.MinLevel || lvl > request.MaxLevel)
            continue;

        if (request.ClassFilter >= 0 && !(request.ClassFilter & (1 << target->getClass())))
            continue;

        if (request.RaceFilter >= 0 && !(request.RaceFilter & (SI64LIT(1) << target->getRace())))
            continue;

        if (!whoRequest.Areas.empty())
        {
            if (std::find(whoRequest.Areas.begin(), whoRequest.Areas.end(), target->GetCurrentZoneID()) == whoRequest.Areas.end())
                continue;
        }

        std::wstring wTargetName;

        if (!Utf8toWStr(target->GetName(), wTargetName))
            continue;

        wstrToLower(wTargetName);

        if (!wPlayerName.empty() && wTargetName.find(wPlayerName) == std::wstring::npos)
            continue;

        std::string gname = target->GetGuildName();
        std::wstring wTargetGuildName;
        if (!Utf8toWStr(gname, wTargetGuildName))
            continue;

        wstrToLower(wTargetGuildName);
        if (!wGuildName.empty() && wTargetGuildName.find(wGuildName) == std::wstring::npos)
            continue;

        if (!wWords.empty())
        {
            std::string aName;
            if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(target->GetCurrentZoneID()))
                aName = areaEntry->AreaName->Str[sObjectMgr->GetDBCLocaleIndex()];

            bool show = false;
            for (size_t i = 0; i < wWords.size(); ++i)
            {
                if (!wWords[i].empty())
                {
                    if (wTargetName.find(wWords[i]) != std::wstring::npos ||
                        wTargetGuildName.find(wWords[i]) != std::wstring::npos ||
                        Utf8FitTo(aName, wWords[i]))
                    {
                        show = true;
                        break;
                    }
                }
            }

            if (!show)
                continue;
        }

        WorldPackets::Who::WhoEntry whoEntry;
        if (!whoEntry.PlayerData.Initialize(target->GetGUID(), target))
            continue;

        if (Guild* targetGuild = target->GetGuild())
        {
            whoEntry.GuildGUID = targetGuild->GetGUID();
            whoEntry.GuildVirtualRealmAddress = GetVirtualRealmAddress();
            whoEntry.GuildName = targetGuild->GetName();
        }

        whoEntry.AreaID = target->GetCurrentZoneID();
        whoEntry.IsGM = target->isGameMaster();

        response.Response.Entries.push_back(whoEntry);
        playerGuids.insert(target->GetGUID());

        if ((displaycount++) >= sWorld->getIntConfig(CONFIG_MAX_WHO))
        {
            if (sWorld->getBoolConfig(CONFIG_LIMIT_WHO_ONLINE))
                break;
            continue;
        }
    }
    HashMapHolder<Player>::GetLock().unlock_shared();

    sScriptMgr->OnPlayerWhoListCall(_player, playerGuids);
    SendPacket(response.Write());
}

void WorldSession::HandleWhoisOpcode(WorldPackets::Who::WhoIsRequest& packet)
{
    if (!AccountMgr::IsAdminAccount(GetSecurity()))
    {
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if (packet.CharName.empty() || !normalizePlayerName (packet.CharName))
    {
        SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player* player = sObjectAccessor->FindPlayerByName(packet.CharName);
    if (!player)
    {
        SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, packet.CharName.c_str());
        return;
    }

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_WHOIS);
    stmt->setUInt32(0, player->GetSession()->GetAccountId());
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
    {
        SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, packet.CharName.c_str());
        return;
    }

    Field* fields = result->Fetch();
    std::string acc = fields[0].GetString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetString();
    if (email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetString();
    if (lastip.empty())
        lastip = "Unknown";

    WorldPackets::Who::WhoIsResponse response;
    response.AccountName = packet.CharName + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;
    SendPacket(response.Write());
}
