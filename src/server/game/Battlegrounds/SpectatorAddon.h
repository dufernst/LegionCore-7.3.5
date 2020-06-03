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

#pragma once

#include "WorldPacket.h"

enum SpectatorPrefix
{
    SPECTATOR_PREFIX_PLAYER,
    SPECTATOR_PREFIX_NAME,
    SPECTATOR_PREFIX_STATUS,
    SPECTATOR_PREFIX_MAXHP,
    SPECTATOR_PREFIX_CURHP,
    SPECTATOR_PREFIX_MAXPOWER,
    SPECTATOR_PREFIX_CURPOWER,
    SPECTATOR_PREFIX_POWERTYPE,
    SPECTATOR_PREFIX_TARGET,
    SPECTATOR_PREFIX_CLASS,
    SPECTATOR_PREFIX_TEAM,
    SPECTATOR_PREFIX_SPELL,
    SPECTATOR_PREFIX_AURA,
    SPECTATOR_PREFIX_ARENA_TIMER,
    SPECTATOR_PREFIX_PET,
    SPECTATOR_PREFIX_PET_HP,
    SPECTATOR_PREFIX_COOLDOWN,

    SPECTATOR_PREFIX_COUNT
};

class SpectatorAddonMsg
{
public:
    SpectatorAddonMsg();

    void SetPlayer(ObjectGuid _player);
    void SetName(std::string _name);
    void SetStatus(bool _isAlive);
    void SetClass(uint8 _class);
    void SetTarget(ObjectGuid _target);
    void SetTeam(uint32 _team);
    void SetMaxHP(uint32 hp);
    void SetCurrentHP(uint32 hp);
    void SetMaxPower(int32 power);
    void SetCurrentPower(int32 power);
    void SetPowerType(Powers power);
    void SetPet(uint8 _family);
    void SetPetHP(uint32 hp);
    void SetEndTime(uint32 _time);
    void CastSpell(uint32 _spellId, uint32 _castTime);
    void CreateAura(ObjectGuid _caster, uint32 _spellId, bool _isDebuff, uint8 _type, int32 _duration, int32 _expire, uint16 _stack, bool _isRemove);
    void AddCooldown(uint32 _spellId, uint32 _cooldown);
    static bool SendPacket(SpectatorAddonMsg& msg, ObjectGuid receiver);
    bool SendPacket(ObjectGuid receiver);
    bool isFilledIn(uint8 prefix);
    static bool CanSandAura(uint32 auraID);
private:

    WorldPacket const* CachedMessage();
    void EnableFlag(uint8 prefix);

    WorldPacket const* cachedMessage;
    ObjectGuid player;
    ObjectGuid target;
    ObjectGuid aCaster;
    uint32 time;
    uint32 maxHP;
    uint32 currHP;
    uint32 petHP;
    uint32 spellId;
    uint32 castTime;
    uint32 cooldown;
    uint32 team;
    uint32 aSpellId;
    int32 maxPower;
    int32 currPower;
    int32 aDuration;
    int32 aExpire;
    Powers powerType;
    uint16 aStack;
    std::string name;
    uint8 pClass;
    uint8 family;
    uint8 aType;
    bool isAlive;
    bool aIsDebuff;
    bool aRemove;
    bool prefixFlags[SPECTATOR_PREFIX_COUNT];
};
