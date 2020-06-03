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

#include "Player.h"
#include "SpectatorAddon.h"
#include <utility>
#include "ChatPackets.h"

SpectatorAddonMsg::SpectatorAddonMsg() : cachedMessage{ nullptr }, maxHP{ 0 }, currHP{ 0 }, petHP{ 0 }, spellId{ 0 }, castTime{ 0 }, cooldown{ 0 }, aSpellId{ 0 }, maxPower{ 0 }, currPower{ 0 }, aDuration{ 0 }, aExpire{ 0 }, aStack{ 0 }, family{ 0 }, aType{ 0 }, isAlive{ false }, aIsDebuff{ false }, aRemove{ false }
{
    for (auto& prefixFlag : prefixFlags)
        prefixFlag = false;
    pClass = CLASS_WARRIOR;
    powerType = POWER_MANA;
    team = ALLIANCE;
    time = std::chrono::duration_cast<Seconds>(Minutes(25)).count();
}

void SpectatorAddonMsg::SetPlayer(ObjectGuid _player)
{
    player = _player;
    EnableFlag(SPECTATOR_PREFIX_PLAYER);
}

void SpectatorAddonMsg::SetName(std::string _name)
{
    name = std::move(_name);
    EnableFlag(SPECTATOR_PREFIX_NAME);
}

void SpectatorAddonMsg::SetStatus(bool _isAlive)
{
    isAlive = _isAlive;
    EnableFlag(SPECTATOR_PREFIX_STATUS);
}

void SpectatorAddonMsg::SetClass(uint8 _class)
{
    pClass = _class;
    EnableFlag(SPECTATOR_PREFIX_CLASS);
}

void SpectatorAddonMsg::SetTarget(ObjectGuid _target)
{
    target = _target;
    EnableFlag(SPECTATOR_PREFIX_TARGET);
}

void SpectatorAddonMsg::SetTeam(uint32 _team)
{
    team = _team;
    EnableFlag(SPECTATOR_PREFIX_TEAM);
}

void SpectatorAddonMsg::SetMaxHP(uint32 hp)
{
    maxHP = hp;
    EnableFlag(SPECTATOR_PREFIX_MAXHP);
}

void SpectatorAddonMsg::SetCurrentHP(uint32 hp)
{
    currHP = hp;
    EnableFlag(SPECTATOR_PREFIX_CURHP);
}

void SpectatorAddonMsg::SetMaxPower(int32 power)
{
    maxPower = power;
    EnableFlag(SPECTATOR_PREFIX_MAXPOWER);
}

void SpectatorAddonMsg::SetCurrentPower(int32 power)
{
    currPower = power;
    EnableFlag(SPECTATOR_PREFIX_CURPOWER);
}

void SpectatorAddonMsg::SetPowerType(Powers power)
{
    powerType = power;
    EnableFlag(SPECTATOR_PREFIX_POWERTYPE);
}

void SpectatorAddonMsg::SetPet(uint8 _family)
{
    family = _family;
    EnableFlag(SPECTATOR_PREFIX_PET);
}

void SpectatorAddonMsg::SetPetHP(uint32 hp)
{
    petHP = hp;
    EnableFlag(SPECTATOR_PREFIX_PET_HP);
}

void SpectatorAddonMsg::SetEndTime(uint32 _time)
{
    time = _time;
    EnableFlag(SPECTATOR_PREFIX_ARENA_TIMER);
}

void SpectatorAddonMsg::CastSpell(uint32 _spellId, uint32 _castTime)
{
    spellId = _spellId;
    castTime = _castTime;
    EnableFlag(SPECTATOR_PREFIX_SPELL);
}

bool SpectatorAddonMsg::CanSandAura(uint32 auraID)
{
    auto spell = sSpellMgr->GetSpellInfo(auraID);
    return !(!spell || spell->HasAttribute(SPELL_ATTR0_HIDDEN_CLIENTSIDE) || spell->HasAttribute(SPELL_ATTR0_HIDE_IN_COMBAT_LOG));
}

void SpectatorAddonMsg::CreateAura(ObjectGuid _caster, uint32 _spellId, bool _isDebuff, uint8 _type, int32 _duration, int32 _expire, uint16 _stack, bool _isRemove)
{
    if (!CanSandAura(_spellId))
        return;

    aCaster = _caster;
    aSpellId = _spellId;
    aIsDebuff = _isDebuff;
    aType = _type;
    aDuration = _duration;
    aExpire = _expire;
    aStack = _stack;
    aRemove = _isRemove;
    EnableFlag(SPECTATOR_PREFIX_AURA);
}

void SpectatorAddonMsg::AddCooldown(uint32 _spellId, uint32 _cooldown)
{
    spellId = _spellId;
    cooldown = _cooldown;
    EnableFlag(SPECTATOR_PREFIX_COOLDOWN);
}

WorldPacket const* SpectatorAddonMsg::CachedMessage()
{
    if (cachedMessage && !cachedMessage->empty())
        return cachedMessage;

    std::string msg;

    if (!isFilledIn(SPECTATOR_PREFIX_PLAYER))
    {
        //sLog->outString("SPECTATOR ADDON: player is not filled in.");
    }
    else
    {
        for (uint8 i = 0; i < std::extent<decltype(prefixFlags)>::value; ++i)
        {
            if (!isFilledIn(i))
                continue;

            switch (i)
            {
                case SPECTATOR_PREFIX_PLAYER:
                {
                    char buffer[64];
                    sprintf(buffer, "%i;", player.GetGUIDLow());
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_NAME:
                    msg += "NME=" + name + ";";
                    break;
                case SPECTATOR_PREFIX_TARGET:
                {
                    char buffer[20];
                    sprintf(buffer, "TRG=%i;", target.GetGUIDLow());
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_TEAM:
                {
                    char buffer[20];
                    sprintf(buffer, "TEM=%i;", static_cast<uint16>(team));
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_STATUS:
                {
                    char buffer[20];
                    sprintf(buffer, "STA=%d;", isAlive);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_CLASS:
                {
                    char buffer[20];
                    sprintf(buffer, "CLA=%i;", static_cast<int>(pClass));
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_MAXHP:
                {
                    char buffer[30];
                    sprintf(buffer, "MHP=%i;", maxHP);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_CURHP:
                {
                    char buffer[30];
                    sprintf(buffer, "CHP=%i;", currHP);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_MAXPOWER:
                {
                    char buffer[30];
                    sprintf(buffer, "MPW=%i;", maxPower);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_CURPOWER:
                {
                    char buffer[30];
                    sprintf(buffer, "CPW=%i;", currPower);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_POWERTYPE:
                {
                    char buffer[20];
                    sprintf(buffer, "PWT=%i;", static_cast<uint8>(powerType));
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_SPELL:
                {
                    char buffer[80];
                    sprintf(buffer, "SPE=%i,%i;", spellId, castTime);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_ARENA_TIMER:
                {
                    char buffer[20];
                    sprintf(buffer, "TIM=%i;", time);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_PET:
                {
                    char buffer[10];
                    sprintf(buffer, "PET=%i;", family);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_PET_HP:
                {
                    char buffer[30];
                    sprintf(buffer, "PHP=%i;", petHP);
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_AURA:
                {
                    char buffer[300];
                    sprintf(buffer, "AUR=%i,%i,%i,%i,%i,%i,%i,0x%X;", aRemove, aStack, aExpire, aDuration, aSpellId, aType, aIsDebuff, static_cast<uint32>(aCaster.GetCounter()));
                    msg += buffer;
                    break;
                }
                case SPECTATOR_PREFIX_COOLDOWN:
                {
                    char buffer[80];
                    sprintf(buffer, "CD=%i,%i;", spellId, cooldown);
                    msg += buffer;
                    break;
                }
                default:
                    break;
            }
        }

        //if (!msg.empty())
        //    msg.insert(0, "ARENASPEC\t");
    }

    WorldPackets::Chat::Chat packet;
    packet.Initialize(CHAT_MSG_WHISPER, LANG_ADDON, ObjectAccessor::FindPlayer(player), nullptr, msg, 0, "", DEFAULT_LOCALE, "ARENASPEC");
    cachedMessage = packet.Write();
    return cachedMessage;
}

void SpectatorAddonMsg::EnableFlag(uint8 prefix)
{
    prefixFlags[prefix] = true;
}

bool SpectatorAddonMsg::SendPacket(ObjectGuid receiver)
{
    auto rPlayer = ObjectAccessor::FindPlayer(receiver);
    if (!rPlayer)
        return false;

    return true;
}

bool SpectatorAddonMsg::isFilledIn(uint8 prefix)
{
    return prefixFlags[prefix];
}

bool SpectatorAddonMsg::SendPacket(SpectatorAddonMsg& msg, ObjectGuid receiver)
{
    auto rPlayer = ObjectAccessor::FindPlayer(receiver);
    if (!rPlayer)
        return false;

    return true;
}
