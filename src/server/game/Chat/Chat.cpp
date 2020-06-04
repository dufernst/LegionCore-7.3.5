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
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"

#include "AccountMgr.h"
#include "CellImpl.h"
#include "Chat.h"
#include "GridNotifiersImpl.h"
#include "Language.h"
#include "Log.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ChatLink.h"
#include "Group.h"
#include "ChatPackets.h"
#include "ObjectVisitors.hpp"
#include "GlobalFunctional.h"
#include "SpellAuraEffects.h"
#include "GuildMgr.h"

bool ChatHandler::load_command_table = true;

std::vector<ChatCommand> const& ChatHandler::getCommandTable()
{
    static std::vector<ChatCommand> commandTableCache;

    if (LoadCommandTable())
    {
        SetLoadCommandTable(false);

        std::vector<ChatCommand> cmds = sScriptMgr->GetChatCommands();
        commandTableCache.swap(cmds);

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_COMMANDS);
        PreparedQueryResult result = WorldDatabase.Query(stmt);
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                std::string name = fields[0].GetString();

                SetDataForCommandInTable(commandTableCache, name.c_str(), fields[1].GetUInt8(), fields[2].GetString(), name);

            } while (result->NextRow());
        }
    }

    return commandTableCache;
}

std::string ChatHandler::PGetParseString(int32 entry, ...) const
{
    const char *format = GetTrinityString(entry);
    char str[1024];
    va_list ap;
    va_start(ap, entry);
    vsnprintf(str, 1024, format, ap);
    va_end(ap);
    return std::string(str);
}

const char *ChatHandler::GetTrinityString(int32 entry) const
{
    return m_session->GetTrinityString(entry);
}

bool ChatHandler::isAvailable(ChatCommand const& cmd) const
{
    // check security level only for simple  command (without child commands)
    return m_session->GetSecurity() >= AccountTypes(cmd.SecurityLevel);
}

bool ChatHandler::HasLowerSecurity(Player* target, ObjectGuid guid, bool strong)
{
    WorldSession* target_session = nullptr;
    uint32 target_account = 0;

    if (target)
        target_session = target->GetSession();
    else if (!guid.IsEmpty())
        target_account = ObjectMgr::GetPlayerAccountIdByGUID(guid);

    if (!target_session && !target_account)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return true;
    }

    return HasLowerSecurityAccount(target_session, target_account, strong);
}

bool ChatHandler::HasLowerSecurityAccount(WorldSession* target, uint32 target_account, bool strong)
{
    uint32 target_sec;

    // allow everything from console and RA console
    if (!m_session)
        return false;

    // ignore only for non-players for non strong checks (when allow apply command at least to same sec level)
    if (!AccountMgr::IsPlayerAccount(m_session->GetSecurity()) && !strong && !sWorld->getBoolConfig(CONFIG_GM_LOWER_SECURITY))
        return false;

    if (target)
        target_sec = target->GetSecurity();
    else if (target_account)
        target_sec = AccountMgr::GetSecurity(target_account, realm.Id.Realm);
    else
        return true;                                        // caller must report error for (target == NULL && target_account == 0)

    auto target_ac_sec = AccountTypes(target_sec);
    if (m_session->GetSecurity() < target_ac_sec || (strong && m_session->GetSecurity() <= target_ac_sec))
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return true;
    }

    return false;
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part)
{
    // non "" command
    if (*name)
    {
        // "" part from non-"" command
        if (!*part)
            return false;

        for (;;)
        {
            if (!*part)
                return true;
            if (!*name)
                return false;
            if (tolower(*name) != tolower(*part))
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(char const* str)
{
    WorldPackets::Chat::Chat packet;
    char* buf = strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        packet.Initialize(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        if (Player* player = m_session->GetPlayer())
            player->SendDirectMessage(packet.Write());
    }

    free(buf);
}

void ChatHandler::SendGlobalSysMessage(char const* str)
{
    WorldPackets::Chat::Chat packet;
    char* buf = strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        packet.Initialize(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        sWorld->SendGlobalMessage(packet.Write());
    }

    free(buf);
}

void ChatHandler::SendGlobalGMSysMessage(const char *str)
{
    WorldPackets::Chat::Chat packet;
    char* buf = strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        packet.Initialize(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, line);
        sWorld->SendGlobalGMMessage(packet.Write());
    }

    free(buf);
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetTrinityString(entry));
}

bool ChatHandler::ExecuteCommandInTable(std::vector<ChatCommand> const& table, const char* text, std::string const& fullcmd)
{
    char const* oldtext = text;
    std::string cmd;

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); ++i)
    {
        if (!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        bool match = false;
        if (strlen(table[i].Name) > cmd.length())
        {
            for (uint32 j = 0; j < table.size(); ++j)
            {
                if (!hasStringAbbr(table[j].Name, cmd.c_str()))
                    continue;

                if (strcmp(table[j].Name, cmd.c_str()) != 0)
                    continue;
                match = true;
                break;
            }
        }
        if (match)
            continue;

        // select subcommand from child commands list
        if (!table[i].ChildCommands.empty())
        {
            if (!ExecuteCommandInTable(table[i].ChildCommands, text, fullcmd))
            {
                if (text && text[0] != '\0')
                    SendSysMessage(LANG_NO_SUBCMD);
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                ShowHelpForCommand(table[i].ChildCommands, text);
            }

            return true;
        }

        // must be available and have handler
        if (!table[i].Handler || !isAvailable(table[i]))
            continue;

        SetSentErrorMessage(false);
        // table[i].Name == "" is special case: send original command to handler
        if ((table[i].Handler)(this, table[i].Name[0] != '\0' ? text : oldtext))
        {
            if (!AccountMgr::IsPlayerAccount(table[i].SecurityLevel))
            {
                // chat case
                if (m_session)
                {
                    Player* p = m_session->GetPlayer();
                    ObjectGuid sel_guid = p->GetSelection();

                    /*sLog->outCommand(m_session->GetAccountId(), "Command: %s [Player: %s (Account: %u) X: %f Y: %f Z: %f Map: %u Selected: %s (GUID: %u)]",
                        fullcmd.c_str(), p->GetName(), m_session->GetAccountId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetMapId(),
                        sel_guid.GetTypeName(), (p->GetSelectedUnit()) ? p->GetSelectedUnit()->GetName() : "", sel_guid.GetCounter());*/
                }
            }
        }
        // some commands have custom error messages. Don't send the default one in these cases.
        else if (!HasSentErrorMessage())
        {
            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());
            else
                SendSysMessage(LANG_CMD_SYNTAX);
        }

        return true;
    }

    return false;
}

bool ChatHandler::SetDataForCommandInTable(std::vector<ChatCommand>& table, char const* text, uint32 permission, std::string const& help, std::string const& fullcommand)
{
    std::string cmd;

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; i < table.size(); i++)
    {
        // for data fill use full explicit command names
        if (table[i].Name != cmd)
            continue;

        // select subcommand from child commands list (including "")
        if (!table[i].ChildCommands.empty())
        {
            if (SetDataForCommandInTable(table[i].ChildCommands, text, permission, help, fullcommand))
                return true;
            if (*text)
                return false;

            // fail with "" subcommands, then use normal level up command instead
        }
        // expected subcommand by full name DB content
        else if (*text)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `command` have unexpected subcommand '%s' in command '%s', skip.", text, fullcommand.c_str());
            return false;
        }

        // if (table[i].SecurityLevel != permission)
            // TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Table `command` overwrite for command '%s' default security (%u) by %u", fullcommand.c_str(), table[i].SecurityLevel, security);

        table[i].SecurityLevel = permission;
        table[i].Help = help;
        return true;
    }

    // in case "" command let process by caller
    if (!cmd.empty())
    {
        if (&table == &getCommandTable())
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `command` have not existed command '%s', skip.", cmd.c_str());
        else
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `command` have not existed subcommand '%s' in command '%s', skip.", cmd.c_str(), fullcommand.c_str());
    }

    return false;
}

int ChatHandler::ParseCommands(const char* text)
{
    ASSERT(text);
    ASSERT(*text);

    std::string fullcmd = text;

    if (m_session && AccountMgr::IsPlayerAccount(m_session->GetSecurity()) && !sWorld->getBoolConfig(CONFIG_ALLOW_PLAYER_COMMANDS) && !(text[1] == 'd' && text[2] == 'o' && text[3] == 'n')) // If the config, players can still use .donate
        return 0;

    /// chat case (.command or !command format)
    if (m_session)
    {
        if (text[0] != '!' && text[0] != '.')
            return 0;
    }

    /// ignore single . and ! in line
    if (strlen(text) < 2)
        return 0;
    // original `text` can't be used. It content destroyed in command code processing.

    /// ignore messages staring from many dots.
    if ((text[0] == '.' && text[1] == '.') || (text[0] == '!' && text[1] == '!'))
        return 0;

    /// skip first . or ! (in console allowed use command with . and ! and without its)
    if (text[0] == '!' || text[0] == '.')
        ++text;

    if (!ExecuteCommandInTable(getCommandTable(), text, fullcmd))
    {
        if (m_session && AccountMgr::IsPlayerAccount(m_session->GetSecurity()))
            return 0;

        SendSysMessage(LANG_NO_CMD);
    }
    return 1;
}

bool ChatHandler::PlayerExtraCommand(const char * text)
{
    ASSERT(text);
    ASSERT(*text);

    std::string fullcmd = text;

    if (fullcmd == "_scan")
    {
        if (!sWorld->getBoolConfig(CONFIG_PLAYER_AURA_SCAN_COMMAND))
            return false;

        if (m_session)
        {
            if (Player* plr = m_session->GetPlayer())
            {
                if (plr->GetCommandCooldown())
                    return true;

                Unit* unit = getSelectedUnit();
                if (!unit)
                {
                    SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
                    SetSentErrorMessage(true);
                    return false;
                }

                std::vector<Aura const*> myAura;
                std::vector<Aura const*> othersAura;
                uint32 targetGUID = unit->GetGUID().GetGUIDLow();
                char const* tool = "|cff808080id:|cffffff00%d%s%s";

                Unit::AuraApplicationMap const& auras = unit->GetAppliedAuras();
                uint16 auraListSize = auras.size();
                std::ostringstream h_info;
                Player* tPlr = unit->ToPlayer();
                if (tPlr)
                {
                    const char* pName = unit->GetName();
                    h_info << pName << " has " << auraListSize << " auras";
                }
                else
                {
                    h_info << "NPC has " << auraListSize << " auras";
                }

                PSendSysMessage(h_info.str().c_str());

                for (const auto& itr : auras)
                {
                    AuraApplicationPtr aurApp = itr.second;

                    Aura const* aura = aurApp->GetBase();
                    uint32 casterGUID = aura->GetCasterGUID().GetGUIDLow();
                    uint32 quality = 2;

                    if (tPlr && aura->GetCastItemGUID())
                    {
                        if (Item* item = tPlr->GetItemByGuid(aura->GetCastItemGUID()))
                        {
                            if (BonusData const* bdata = item->GetBonus())
                            {
                                quality = bdata->Quality;
                            }
                        }
                    }
                    else if (targetGUID == casterGUID)
                    {
                        myAura.push_back(aura);
                        continue;
                    }
                    else
                    {
                        othersAura.push_back(aura);
                        continue;
                    }

                    std::ostringstream ss_info;
                    bool _find = false;

                    for (size_t i = 0; i < MAX_SPELL_EFFECTS; i++)
                    {
                        if (AuraEffect* eff = aura->GetEffect(i))
                        {
                            if (float a = eff->GetAmount())
                            {
                                if (!_find)
                                {
                                    ss_info << "|cff808080bp:" << "|cffffffff" << i << ":|cffff0000" << a << "  ";
                                    _find = true;
                                    continue;
                                }
                                ss_info << "|cffffffff" << i << ":|cffff0000" << a << "  ";
                            }
                        }
                    }

                    int32 dur = aura->GetDuration();
                    if (dur > 0)
                        ss_info << "|cff808080dur:" << "|cffffff00" << dur;

                    char const* nameColor = "|cffffffff|Hspell:";

                    switch (quality)
                    {
                        case 6: // art
                        {
                            nameColor = "|cffe6d386|Hspell:";
                            break;
                        }
                        case 5: // legend
                        {
                            nameColor = "|cffff7f00|Hspell:";
                            break;
                        }
                        case 4: // epic
                        {
                            nameColor = "|cffa335ee|Hspell:";
                            break;
                        }
                        case 3: // rare
                        {
                            nameColor = "|cff0070e0|Hspell:";
                            break;
                        }
                    }

                    char const* name = aura->GetSpellInfo()->SpellName;
                    std::ostringstream ss_name;
                    ss_name << nameColor << aura->GetId() << "|h[" << name << "]|h|r";

                    PSendSysMessage(tool, aura->GetId(), (ss_name.str().c_str()),
                        (ss_info.str().c_str()));

                }

                if (!myAura.empty())
                {
                    PSendSysMessage("Their auras %d: ", myAura.size());

                    for (auto itr : myAura)
                    {
                        std::ostringstream ss_info;
                        bool _find = false;

                        for (size_t i = 0; i < MAX_SPELL_EFFECTS; i++)
                        {
                            if (AuraEffect* eff = itr->GetEffect(i))
                            {
                                if (float a = eff->GetAmount())
                                {
                                    if (!_find)
                                    {
                                        ss_info << "|cff808080bp:" << "|cffffffff" << i << ":|cffff0000" << a << "  ";
                                        _find = true;
                                        continue;
                                    }
                                    ss_info << "|cffffffff" << i << ":|cffff0000" << a << "  ";
                                }
                            }
                        }

                        int32 dur = itr->GetDuration();
                        if (dur > 0)
                            ss_info << "|cff808080dur:" << "|cffffff00" << dur;

                        char const* name = itr->GetSpellInfo()->SpellName;
                        std::ostringstream ss_name;
                        ss_name << "|cff7dd0ff|Hspell:" << itr->GetId() << "|h[" << name << "]|h|r";

                        PSendSysMessage(tool, itr->GetId(), (ss_name.str().c_str()),
                            (ss_info.str().c_str()));
                    }
                }

                if (!othersAura.empty())
                {
                    PSendSysMessage("Other auras %d: ", othersAura.size());

                    for (auto itr : othersAura)
                    {
                        std::ostringstream ss_info;
                        bool _find = false;

                        for (size_t i = 0; i < MAX_SPELL_EFFECTS; i++)
                        {
                            if (AuraEffect* eff = itr->GetEffect(i))
                            {
                                if (float a = eff->GetAmount())
                                {
                                    if (!_find)
                                    {
                                        ss_info << "|cff808080bp:" << "|cffffffff" << i << ":|cffff0000" << a << "  ";
                                        _find = true;
                                        continue;
                                    }
                                    ss_info << "|cffffffff" << i << ":|cffff0000" << a << "  ";
                                }
                            }
                        }

                        int32 dur = itr->GetDuration();
                        if (dur > 0)
                            ss_info << "|cff808080dur:" << "|cffffff00" << dur;

                        char const* name = itr->GetSpellInfo()->SpellName;
                        std::ostringstream ss_name;
                        ss_name << "|cffffffff|Hspell:" << itr->GetId() << "|h[" << name << "]|h|r";

                        PSendSysMessage(tool, itr->GetId(), (ss_name.str().c_str()),
                            (ss_info.str().c_str()));
                    }
                }
                plr->ResetCommandCooldown();
                return true;
            }
        }
    }
    else if (fullcmd == "_invisible on")
    {
        if (m_session)
        {
            if (Player* plr = m_session->GetPlayer())
            {
                if (plr->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS) || plr->GetCommandCooldown())
                    return true;

                if (plr->InvisibleStatusRatingRequirements() && plr->InvisibleStatusMapRequirements())
                {
                    sSocialMgr->SendFriendStatus(plr, FRIEND_OFFLINE, plr->GetGUID(), true);

                    if (plr->GetGuildId() != 0)
                    {
                        if (Guild* guild = sGuildMgr->GetGuildById(plr->GetGuildId()))
                            guild->SendGuildEventPresenceChanged(plr->GetGUID(), plr->GetName(), false);
                    }

                    plr->SetPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS);
                    plr->SendInvisibleStatusMsg(1);
                }
                
                plr->ResetCommandCooldown();
                return true;
            }
        }
    }
    else if (fullcmd == "_invisible off")
    {
        if (m_session)
        {
            if (Player* plr = m_session->GetPlayer())
            {
                if (!plr->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS) || plr->GetCommandCooldown())
                    return true;

                plr->SetPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS, false);
                sSocialMgr->SendFriendStatus(plr, FRIEND_ONLINE, plr->GetGUID(), true);

                if (plr->GetGuildId() != 0)
                {
                    if (Guild* guild = sGuildMgr->GetGuildById(plr->GetGuildId()))
                    {
                        guild->SendGuildEventPresenceChanged(plr->GetGUID(), plr->GetName(), true);
                        guild->SendGuildEventPresenceChanged(plr->GetGUID(), plr->GetName(), true, m_session);
                    }
                }

                plr->SendInvisibleStatusMsg(0);
                plr->ResetCommandCooldown();
                return true;
            }
        }
    }
    return false;
}

bool ChatHandler::isValidChatMessage(const char* message)
{
    /*
    Valid examples:
    |cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
    |cff808080|Hquest:2278:47|h[The Platinum Discs]|h|r
    |cffffd000|Htrade:4037:1:150:1:6AAAAAAAAAAAAAAAAAAAAAAOAADAAAAAAAAAAAAAAAAIAAAAAAAAA|h[Engineering]|h|r
    |cff4e96f7|Htalent:2232:-1|h[Taste for Blood]|h|r
    |cff71d5ff|Hspell:21563|h[Command]|h|r
    |cffffd000|Henchant:3919|h[Engineering: Rough Dynamite]|h|r
    |cffffff00|Hachievement:546:0000000000000001:0:0:0:-1:0:0:0:0|h[Safe Deposit]|h|r
    |cff66bbff|Hglyph:21:762|h[Glyph of Bladestorm]|h|r

    | will be escaped to ||
    */

    if (strlen(message) > 255)
        return false;

    // more simple checks
    if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) < 3)
    {
        const char validSequence[6] = "cHhhr";
        const char* validSequenceIterator = validSequence;
        const std::string validCommands = "cHhr|";

        while (*message)
        {
            // find next pipe command
            message = strchr(message, '|');

            if (!message)
                return true;

            ++message;
            char commandChar = *message;
            if (validCommands.find(commandChar) == std::string::npos)
                return false;

            ++message;
            // validate sequence
            if (sWorld->getIntConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) == 2)
            {
                if (commandChar == *validSequenceIterator)
                {
                    if (validSequenceIterator == validSequence + 4)
                        validSequenceIterator = validSequence;
                    else
                        ++validSequenceIterator;
                }
                else
                    return false;
            }
        }
        return true;
    }

    return LinkExtractor(message).IsValidMessage();
}

bool ChatHandler::ShowHelpForSubCommands(std::vector<ChatCommand> const& table, char const* cmd, char const* subcmd)
{
    std::string list;
    for (uint32 i = 0; i < table.size(); ++i)
    {
        // must be available (ignore handler existence for show command with possible available subcommands)
        if (!isAvailable(table[i]))
            continue;

        // for empty subcmd show all available
        if (*subcmd && !hasStringAbbr(table[i].Name, subcmd))
            continue;

        if (m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += table[i].Name;

        if (!table[i].ChildCommands.empty())
            list += " ...";
    }

    if (list.empty())
        return false;

    if (&table == &getCommandTable())
    {
        SendSysMessage(LANG_AVIABLE_CMD);
        PSendSysMessage("%s", list.c_str());
    }
    else
        PSendSysMessage(LANG_SUBCMDS_LIST, cmd, list.c_str());

    return true;
}

bool ChatHandler::ShowHelpForCommand(std::vector<ChatCommand> const& table, const char* cmd)
{
    if (*cmd)
    {
        for (uint32 i = 0; i < table.size(); ++i)
        {
            // must be available (ignore handler existence for show command with possible available subcommands)
            if (!isAvailable(table[i]))
                continue;

            if (!hasStringAbbr(table[i].Name, cmd))
                continue;

            // have subcommand
            char const* subcmd = (*cmd) ? strtok(nullptr, " ") : "";

            if (!table[i].ChildCommands.empty() && subcmd && *subcmd)
            {
                if (ShowHelpForCommand(table[i].ChildCommands, subcmd))
                    return true;
            }

            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if (!table[i].ChildCommands.empty())
                if (ShowHelpForSubCommands(table[i].ChildCommands, table[i].Name, subcmd ? subcmd : ""))
                    return true;

            return !table[i].Help.empty();
        }
    }
    else
    {
        for (uint32 i = 0; i < table.size(); ++i)
        {
            // must be available (ignore handler existence for show command with possible available subcommands)
            if (!isAvailable(table[i]))
                continue;

            if (strlen(table[i].Name))
                continue;

            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if (!table[i].ChildCommands.empty())
                if (ShowHelpForSubCommands(table[i].ChildCommands, "", ""))
                    return true;

            return !table[i].Help.empty();
        }
    }

    return ShowHelpForSubCommands(table, "", cmd);
}

Player* ChatHandler::getSelectedPlayer()
{
    if (!m_session)
        return nullptr;

    ObjectGuid guid = m_session->GetPlayer()->GetSelection();

    if (guid.IsEmpty())
        return m_session->GetPlayer();

    return ObjectAccessor::FindPlayer(guid);
}

Unit* ChatHandler::getSelectedUnit()
{
    if (!m_session)
        return nullptr;

    ObjectGuid guid = m_session->GetPlayer()->GetSelection();

    if (guid.IsEmpty())
        return m_session->GetPlayer();

    return ObjectAccessor::GetUnit(*m_session->GetPlayer(), guid);
}

WorldObject* ChatHandler::getSelectedObject()
{
    if (!m_session)
        return nullptr;

    ObjectGuid guid = m_session->GetPlayer()->GetSelection();

    if (guid.IsEmpty())
        return GetNearbyGameObject();

    return ObjectAccessor::GetUnit(*m_session->GetPlayer(), guid);
}

Creature* ChatHandler::getSelectedCreature()
{
    if (!m_session)
        return nullptr;

    return ObjectAccessor::GetCreatureOrPetOrVehicle(*m_session->GetPlayer(), m_session->GetPlayer()->GetSelection());
}

char* ChatHandler::extractKeyFromLink(char* text, char const* linkType, char** something1)
{
    // skip empty
    if (!text)
        return nullptr;

    // skip spaces
    while (*text == ' ' || *text == '\t' || *text == '\b')
        ++text;

    if (!*text)
        return nullptr;

    // return non link case
    if (text[0] != '|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if (!check)
        return nullptr;                                        // end of data

    char* cLinkType = strtok(nullptr, ":");                    // linktype
    if (!cLinkType)
        return nullptr;                                        // end of data

    if (strcmp(cLinkType, linkType) != 0)
    {
        strtok(nullptr, " ");                                  // skip link tail (to allow continue strtok(NULL, s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return nullptr;
    }

    char* cKeys = strtok(nullptr, "|");                        // extract keys and values
    char* cKeysTail = strtok(nullptr, "");

    char* cKey = strtok(cKeys, ":|");                       // extract key
    if (something1)
        *something1 = strtok(nullptr, ":|");                   // extract something

    strtok(cKeysTail, "]");                                 // restart scan tail and skip name with possible spaces
    strtok(nullptr, " ");                                      // skip link tail (to allow continue strtok(NULL, s) use after return from function
    return cKey;
}

char* ChatHandler::extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1)
{
    // skip empty
    if (!text)
        return nullptr;

    // skip spaces
    while (*text == ' ' || *text == '\t' || *text == '\b')
        ++text;

    if (!*text)
        return nullptr;

    // return non link case
    if (text[0] != '|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r
    // or
    // [name] Shift-click form |linkType:key|h[name]|h|r

    char* tail;

    if (text[1] == 'c')
    {
        char* check = strtok(text, "|");                    // skip color
        if (!check)
            return nullptr;                                    // end of data

        tail = strtok(nullptr, "");                            // tail
    }
    else
        tail = text + 1;                                      // skip first |

    char* cLinkType = strtok(tail, ":");                    // linktype
    if (!cLinkType)
        return nullptr;                                        // end of data

    for (int i = 0; linkTypes[i]; ++i)
    {
        if (strcmp(cLinkType, linkTypes[i]) == 0)
        {
            char* cKeys = strtok(nullptr, "|");                // extract keys and values
            char* cKeysTail = strtok(nullptr, "");

            char* cKey = strtok(cKeys, ":|");               // extract key
            if (something1)
                *something1 = strtok(nullptr, ":|");           // extract something

            strtok(cKeysTail, "]");                         // restart scan tail and skip name with possible spaces
            strtok(nullptr, " ");                              // skip link tail (to allow continue strtok(NULL, s) use after return from function
            if (found_idx)
                *found_idx = i;
            return cKey;
        }
    }

    strtok(nullptr, " ");                                      // skip link tail (to allow continue strtok(NULL, s) use after return from function
    SendSysMessage(LANG_WRONG_LINK_TYPE);
    return nullptr;
}

GameObject* ChatHandler::GetNearbyGameObject()
{
    if (!m_session)
        return nullptr;

    Player* pl = m_session->GetPlayer();
    GameObject* obj = nullptr;
    Trinity::NearestGameObjectCheck check(*pl);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectCheck> searcher(pl, obj, check);
    Trinity::VisitNearbyGridObject(pl, SIZE_OF_GRIDS, searcher);
    return obj;
}

GameObject* ChatHandler::GetObjectGlobalyWithGuidOrNearWithDbGuid(ObjectGuid::LowType lowguid, uint32 entry)
{
    if (!m_session)
        return nullptr;

    Player* pl = m_session->GetPlayer();

    ObjectGuid guid = ObjectGuid::Create<HighGuid::GameObject>(pl->GetMapId(), entry, lowguid);
    GameObject* obj = pl->GetMap()->GetGameObject(guid);


    if (!obj && sObjectMgr->GetGOData(lowguid))                   // guid is DB guid of object
    {
        // search near player then
        CellCoord p(Trinity::ComputeCellCoord(pl->GetPositionX(), pl->GetPositionY()));
        Cell cell(p);

        Trinity::GameObjectWithDbGUIDCheck go_check(lowguid);
        Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(pl, obj, go_check);

        cell.Visit(p, Trinity::makeGridVisitor(checker), *pl->GetMap(), *pl, pl->GetGridActivationRange());
    }

    return obj;
}

enum SpellLinkType
{
    SPELL_LINK_SPELL   = 0,
    SPELL_LINK_TALENT  = 1,
    SPELL_LINK_ENCHANT = 2,
    SPELL_LINK_TRADE   = 3,
    SPELL_LINK_GLYPH   = 4
};

static char const* const spellKeys[] =
{
    "Hspell",                                               // normal spell
    "Htalent",                                              // talent spell
    "Henchant",                                             // enchanting recipe spell
    "Htrade",                                               // profession/skill spell
    "Hglyph",                                               // glyph
    nullptr
};

uint32 ChatHandler::extractSpellIdFromLink(char* text)
{
    // number or [name] Shift-click form |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r
    // number or [name] Shift-click form |color|Hglyph:glyph_slot_id:glyph_prop_id|h[%s]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id, rank|h[name]|h|r
    // number or [name] Shift-click form |color|Htrade:spell_id, skill_id, max_value, cur_value|h[name]|h|r
    int type = 0;
    char* param1_str = nullptr;
    char* idS = extractKeyFromLink(text, spellKeys, &type, &param1_str);
    if (!idS)
        return 0;

    auto id = static_cast<uint32>(atol(idS));
    switch (type)
    {
        case SPELL_LINK_SPELL:
            return id;
        case SPELL_LINK_TALENT:
            if (TalentEntry const* talentEntry = sTalentStore.LookupEntry(id))
                return talentEntry->SpellID;
        case SPELL_LINK_ENCHANT:
        case SPELL_LINK_TRADE:
            return id;
        case SPELL_LINK_GLYPH:
            if (GlyphPropertiesEntry const* glyphPropEntry = sGlyphPropertiesStore.LookupEntry(param1_str ? atoul(param1_str) : 0))
                return glyphPropEntry->SpellID;
        default:
            return 0;
    }
}

GameTele const* ChatHandler::extractGameTeleFromLink(char* text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = extractKeyFromLink(text, "Htele");
    if (!cId)
        return nullptr;

    // id case (explicit or from shift link)
    if (cId[0] >= '0' || cId[0] >= '9')
        if (uint32 id = atoi(cId))
            return sObjectMgr->GetGameTele(id);

    return sObjectMgr->GetGameTele(cId);
}

enum GuidLinkType
{
    SPELL_LINK_PLAYER     = 0,                              // must be first for selection in not link case
    SPELL_LINK_CREATURE   = 1,
    SPELL_LINK_GAMEOBJECT = 2
};

static char const* const guidKeys[] =
{
    "Hplayer",
    "Hcreature",
    "Hgameobject",
    nullptr
};

ObjectGuid ChatHandler::extractGuidFromLink(char* text)
{
    int type = 0;

    // |color|Hcreature:creature_guid|h[name]|h|r
    // |color|Hgameobject:go_guid|h[name]|h|r
    // |color|Hplayer:name|h[name]|h|r
    char* idS = extractKeyFromLink(text, guidKeys, &type);
    if (!idS)
        return ObjectGuid::Empty;

    switch (type)
    {
        case SPELL_LINK_PLAYER:
        {
            std::string name = idS;
            if (!normalizePlayerName(name))
                return ObjectGuid::Empty;

            if (Player* player = sObjectAccessor->FindPlayerByName(name))
                return player->GetGUID();

            return ObjectMgr::GetPlayerGUIDByName(name);
        }
        case SPELL_LINK_CREATURE:
        {
            ObjectGuid::LowType lowguid = strtoull(idS, nullptr, 10);

            if (CreatureData const* data = sObjectMgr->GetCreatureData(lowguid))
                return ObjectGuid::Create<HighGuid::Creature>(data->mapid, data->id, lowguid);

            return ObjectGuid::Empty;
        }
        case SPELL_LINK_GAMEOBJECT:
        {
            ObjectGuid::LowType lowguid = strtoull(idS, nullptr, 10);

            if (GameObjectData const* data = sObjectMgr->GetGOData(lowguid))
                return ObjectGuid::Create<HighGuid::GameObject>(data->mapid, data->id, lowguid);

            return ObjectGuid::Empty;
        }
        default:
            break;
    }

    // unknown type?
    return ObjectGuid::Empty;
}

std::string ChatHandler::extractPlayerNameFromLink(char* text)
{
    // |color|Hplayer:name|h[name]|h|r
    char* name_str = extractKeyFromLink(text, "Hplayer");
    if (!name_str)
        return "";

    std::string name = name_str;
    if (!normalizePlayerName(name))
        return "";

    return name;
}

bool ChatHandler::extractPlayerTarget(char* args, Player** player, ObjectGuid* player_guid /*=NULL*/, std::string* player_name /*= NULL*/)
{
    if (args && *args)
    {
        std::string name = extractPlayerNameFromLink(args);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Player* pl = sObjectAccessor->FindPlayerByName(name);

        // if allowed player pointer
        if (player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        ObjectGuid guid = !pl && (player_guid || player_name) ? ObjectMgr::GetPlayerGUIDByName(name) : ObjectGuid::Empty;

        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : guid;

        if (player_name)
            *player_name = pl || !guid.IsEmpty() ? name : "";
    }
    else
    {
        Player* pl = getSelectedPlayer();
        // if allowed player pointer
        if (player)
            *player = pl;
        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetGUID() : ObjectGuid::Empty;

        if (player_name)
            *player_name = pl ? pl->GetName() : "";
    }

    // some from req. data must be provided (note: name is empty if player not exist)
    if ((!player || !*player) && (!player_guid || !*player_guid) && (!player_name || player_name->empty()))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

void ChatHandler::extractOptFirstArg(char* args, char** arg1, char** arg2)
{
    char* p1 = strtok(args, " ");
    char* p2 = strtok(nullptr, " ");

    if (!p2)
    {
        p2 = p1;
        p1 = nullptr;
    }

    if (arg1)
        *arg1 = p1;

    if (arg2)
        *arg2 = p2;
}

char* ChatHandler::extractQuotedArg(char* args)
{
    if (!*args)
        return nullptr;

    if (*args == '"')
        return strtok(args + 1, "\"");
    char* space = strtok(args, "\"");
    if (!space)
        return nullptr;
    return strtok(nullptr, "\"");
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyFor(chr);
}

LocaleConstant ChatHandler::GetSessionDbcLocale() const
{
    return m_session->GetSessionDbcLocale();
}

int ChatHandler::GetSessionDbLocaleIndex() const
{
    return m_session->GetSessionDbLocaleIndex();
}

CliHandler::CliHandler(void* callbackArg, Print* zprint) : m_callbackArg(callbackArg), m_print(zprint)
{
}

const char *CliHandler::GetTrinityString(int32 entry) const
{
    return sObjectMgr->GetTrinityStringForDBCLocale(entry);
}

bool CliHandler::isAvailable(ChatCommand const& cmd) const
{
    // skip non-console commands in console case
    return cmd.AllowConsole;
}

void CliHandler::SendSysMessage(const char *str)
{
    m_print(m_callbackArg, str);
    m_print(m_callbackArg, "\r\n");
}

std::string CliHandler::GetNameLink() const
{
    return GetTrinityString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

bool ChatHandler::GetPlayerGroupAndGUIDByName(const char* cname, Player* &player, Group* &group, ObjectGuid &guid, bool offline)
{
    player = nullptr;
    guid = ObjectGuid::Empty;

    if (cname)
    {
        std::string name = cname;
        if (!name.empty())
        {
            if (!normalizePlayerName(name))
            {
                PSendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }

            player = sObjectAccessor->FindPlayerByName(name);
            if (offline)
                guid = ObjectMgr::GetPlayerGUIDByName(name);
        }
    }

    if (player)
    {
        group = player->GetGroup();
        if (guid.IsEmpty() || !offline)
            guid = player->GetGUID();
    }
    else
    {
        if (getSelectedPlayer())
            player = getSelectedPlayer();
        else
            player = m_session->GetPlayer();

        if (guid.IsEmpty() || !offline)
            guid = player->GetGUID();
        group = player->GetGroup();
    }

    return true;
}

LocaleConstant CliHandler::GetSessionDbcLocale() const
{
    return sWorld->GetDefaultDbcLocale();
}

int CliHandler::GetSessionDbLocaleIndex() const
{
    return sObjectMgr->GetDBCLocaleIndex();
}

CommandArgs::CommandArgs(ChatHandler* handler, char const* args): _validArgs(false), _handler(handler), _charArgs(args) { }

CommandArgs::CommandArgs(ChatHandler* handler, char const* args, std::initializer_list<CommandArgsType> argsType): _validArgs(false), _handler(handler), _charArgs(args)
{
    Initialize(argsType);
}

bool CommandArgs::ValidArgs() const
{
    return _validArgs;
}

uint32 CommandArgs::GetArgInt(uint32 index)
{
    return GetArg<int32>(index);
}

uint32 CommandArgs::GetArgUInt(uint32 index)
{
    return GetArg<uint32>(index);
}

float CommandArgs::GetArgFloat(uint32 index)
{
    return GetArg<float>(index);
}

std::string CommandArgs::GetArgString(uint32 index)
{
    return GetArg<std::string>(index);
}

void CommandArgs::Initialize(std::initializer_list<CommandArgsType> argsType)
{
    try
    {
        auto argsTypeVector = std::vector<CommandArgsType>(argsType);
        auto arg = strtok(const_cast<char*>(_charArgs), " ");
        uint8 argsCount = 0;

        while (arg != nullptr)
        {
            if (argsCount > argsTypeVector.size())
                return;

            switch (argsTypeVector[argsCount++])
            {
            case ARG_INT:
                _args.emplace_back(int32(atoi(arg)));
                break;
            case ARG_UINT:
            {
                auto value = atoi(arg);
                if (value < 0)
                    return;

                _args.emplace_back(uint32(value));
                break;
            }
            case ARG_FLOAT:
                _args.emplace_back(float(atof(arg)));
                break;
            case ARG_STRING:
                _args.emplace_back(std::string(arg));
                break;
            case ARG_QUOTE_ENCLOSED_STRING:
                _args.emplace_back(std::string(_handler->extractQuotedArg(arg)));
                break;
            default:
                break;
            }
            arg = strtok(nullptr, " ");
        }

        if (argsCount == argsTypeVector.size())
            _validArgs = true;
    }
    catch (std::exception e)
    {
        _validArgs = false;
    }
}
