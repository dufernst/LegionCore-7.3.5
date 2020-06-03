/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_CHALLENGE_H
#define TRINITY_CHALLENGE_H

#include "Common.h"
#include "Scenario.h"
#include "InstanceScript.h"

struct ChallengeEntry;
class Scenario;

enum ChallengeSpells : uint32
{
    ChallengersMight                = 206150, /// generic creature aura
    ChallengersBurden               = 206151, /// generic player aura
    ChallengerBolstering            = 209859,
    ChallengerNecrotic              = 209858,
    ChallengerOverflowing           = 221772,
    ChallengerSanguine              = 226489,
    ChallengerRaging                = 228318,
    ChallengerSummonVolcanicPlume   = 209861,
    ChallengerVolcanicPlume         = 209862,
    ChallengerBursting              = 240443,
    ChallengerQuake                 = 240447,
    ChallengerGrievousWound         = 240559,

    //Explosive
    SPELL_FEL_EXPLOSIVES_SUMMON_1   = 240444, //Short dist
    SPELL_FEL_EXPLOSIVES_SUMMON_2   = 243110, //Long dist
    SPELL_FEL_EXPLOSIVES_VISUAL     = 240445,
    SPELL_FEL_EXPLOSIVES_DMG        = 240446,

    SPELL_CHALLENGE_ANTIKICK        = 305284,
};

enum ChallengeNpcs : uint32
{
    NpcVolcanicPlume        = 105877,
    NPC_FEL_EXPLOSIVES      = 120651,
};

enum MiscChallengeData : uint32
{
    ChallengeDelayTimer     = 10,

};

class Challenge : public InstanceScript
{
public:
    Challenge(Map* map, Player* player, uint32 instanceID, Scenario* scenario);
    ~Challenge();

    void OnPlayerEnterForScript(Player* player) override;
    void OnPlayerLeaveForScript(Player* player) override;
    void OnPlayerDiesForScript(Player* player) override;
    void OnCreatureCreateForScript(Creature* creature) override;
    void OnCreatureRemoveForScript(Creature* creature) override;
    void OnCreatureUpdateDifficulty(Creature* creature) override;
    void EnterCombatForScript(Creature* creature, Unit* enemy) override;
    void CreatureDiesForScript(Creature* creature, Unit* killer) override;
    void OnGameObjectCreateForScript(GameObject* /*go*/) override {}
    void OnGameObjectRemoveForScript(GameObject* /*go*/) override {}
    void OnUnitCharmed(Unit* unit, Unit* charmer) override;
    void OnUnitRemoveCharmed(Unit* unit, Unit* charmer) override;

    void Update(uint32 diff) override;

    bool CanStart();
    void Start();
    void Complete();

    void BroadcastPacket(WorldPacket const* data) const override;

    void HitTimer();

    uint32 GetChallengeLevel() const;
    std::array<uint32, 3> GetAffixes() const;
    bool HasAffix(Affixes affix);

    uint32 GetChallengeTimerToNow() const;
    void ModChallengeTimer(uint32 timer);
    uint32 GetChallengeTimer();

    void ResetGo();
    void SendStartTimer(Player* player = nullptr);
    void SendStartElapsedTimer(Player* player = nullptr);
    void SendChallengeModeStart(Player* player = nullptr);
    void SendChallengeModeNewPlayerRecord(Player* player);
    void SendChallengeModeMapStatsUpdate(Player* player);
    void SummonWall(Player* player);
    uint8 GetItemCount(ObjectGuid guid) const;
    uint8 GetLevelBonus() const;

    void SetInstanceScript(InstanceScript* instanceScript);
    InstanceScript* GetInstanceScript() const;

    GuidUnorderedSet _challengers;
    bool _checkStart;
    bool _canRun;
    bool _run;
    bool _complete;

    ObjectGuid m_gguid;
    ObjectGuid m_ownerGuid;
    ObjectGuid m_itemGuid;

    uint32 _challengeTimer;
    uint32 _affixQuakingTimer;

    FunctionProcessor m_Functions;
    uint32 _mapID;

private:
    std::map<ObjectGuid, uint8> _countItems;

    ObjectGuid _creator;
    std::array<uint32, 3> _affixes;
    std::bitset<size_t(Affixes::MaxAffixes)> _affixesTest;
    uint16 _chestTimers[3];
    Item* _item;
    Map* _map;
    InstanceScript* _instanceScript;
    MapChallengeModeEntry const* _challengeEntry;
    uint32 _challengeLevel;
    uint32 _instanceID;
    uint8 _rewardLevel;
    bool _isKeyDepleted;
    Scenario* _scenario;
    uint32 _deathCount = 0;
};

#endif
