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

#include "Creature.h"
#include "Formulas.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "InstanceScript.h"
#include "KillRewarder.h"
#include "Player.h"
#include "ScenarioMgr.h"

// == KillRewarder ====================================================
// KillRewarder incapsulates logic of rewarding player upon kill with:
// * XP;
// * honor;
// * reputation;
// * kill credit (for quest objectives).
// Rewarding is initiated in two cases: when player kills unit in Unit::Kill()
// and on battlegrounds in Battleground::RewardXPAtKill().
//
// Rewarding algorithm is:
// 1. Initialize internal variables to default values.
// 2. In case when player is in group, initialize variables necessary for group calculations:
// 2.1. _count - number of alive group members within reward distance;
// 2.2. _sumLevel - sum of levels of alive group members within reward distance;
// 2.3. _maxLevel - maximum level of alive group member within reward distance;
// 2.4. _maxNotGrayMember - maximum level of alive group member within reward distance,
//      for whom victim is not gray;
// 2.5. _isFullXP - flag identifying that for all group members victim is not gray,
//      so 100% XP will be rewarded (50% otherwise).
// 3. Reward killer (and group, if necessary).
// 3.1. If killer is in group, reward group.
// 3.1.1. Initialize initial XP amount based on maximum level of group member,
//        for whom victim is not gray.
// 3.1.2. Alter group rate if group is in raid (not for battlegrounds).
// 3.1.3. Reward each group member (even dead) within reward distance (see 4. for more details).
// 3.2. Reward single killer (not group case).
// 3.2.1. Initialize initial XP amount based on killer's level.
// 3.2.2. Reward killer (see 4. for more details).
// 4. Reward player.
// 4.1. Give honor (player must be alive and not on BG).
// 4.2. Give XP.
// 4.2.1. If player is in group, adjust XP:
//        * set to 0 if player's level is more than maximum level of not gray member;
//        * cut XP in half if _isFullXP is false.
// 4.2.2. Apply auras modifying rewarded XP.
// 4.2.3. Give XP to player.
// 4.3. Give reputation (player must not be on BG).
// 4.4. Give kill credit (player must not be in group, or he must be alive or without corpse).
// 5. Credit instance encounter.
// 6. Update guild achievements.

// 1. Initialize internal variables to default values.
KillRewarder::KillRewarder(Player* killer, Unit* victim, bool isBattleGround)
{
    _killer = killer;
    _victim = victim;
    _group = killer->GetGroup();
    _groupRate = 1.0f;
    _maxNotGrayMember = nullptr;
    _count = 0;
    _countForRep = 0;
    _sumLevel = 0;
    _maxLevel = 0;
    _xp = 0;
    _isFullXP = false;
    _isBattleGround = isBattleGround;
    _isPvP = false;

    if (victim->IsPlayer())
        _isPvP = true;
    else if (victim->GetCharmerOrOwnerGUID().IsPlayer())
        _isPvP = !victim->IsVehicle();

    _InitGroupData();
}

inline void KillRewarder::_InitGroupData()
{
    if (!_group)
    {
        _count = 1;
        return;
    }

    // 2. In case when player is in group, initialize variables necessary for group calculations:
    for (GroupReference* itr = _group->GetFirstMember(); itr != nullptr; itr = itr->next())
        if (Player* member = itr->getSource())
            if (member->IsAtGroupRewardDistance(_victim))
            {
                ++_countForRep;
                if (!member->isAlive())
                    continue;

                const uint8 lvl = member->getLevel();
                // 2.1. _count - number of alive group members within reward distance;
                ++_count;
                // 2.2. _sumLevel - sum of levels of alive group members within reward distance;
                _sumLevel += lvl;
                // 2.3. _maxLevel - maximum level of alive group member within reward distance;
                if (_maxLevel < lvl)
                    _maxLevel = lvl;
                // 2.4. _maxNotGrayMember - maximum level of alive group member within reward distance, for whom victim is not gray;
                uint32 grayLevel = Trinity::XP::GetGrayLevel(lvl);
                if (_victim->getLevel() > grayLevel && (!_maxNotGrayMember || _maxNotGrayMember->getLevel() < lvl))
                    _maxNotGrayMember = member;
            }
    // 2.5. _isFullXP - flag identifying that for all group members victim is not gray, so 100% XP will be rewarded (50% otherwise).
    _isFullXP = _maxNotGrayMember && (_maxLevel == _maxNotGrayMember->getLevel());
}

inline void KillRewarder::_InitXP(Player* player)
{
    // Get initial value of XP for kill.
    // XP is given:
    // * on battlegrounds;
    // * otherwise, not in PvP;
    // * not if killer is on vehicle.
    if (_isBattleGround || (!_isPvP && !_killer->GetVehicle()))
        _xp = Trinity::XP::Gain(player, _victim);
}

inline void KillRewarder::_RewardHonor(Player* player)
{
    if (player->isAlive())
        player->RewardHonor(_victim, _count, -1, true);
}

inline void KillRewarder::_RewardXP(Player* player, float rate)
{
    uint32 xp(_xp);
    if (_group)
    {
        // 4.2.1. If player is in group, adjust XP:
        //        * set to 0 if player's level is more than maximum level of not gray member;
        //        * cut XP in half if _isFullXP is false.
        if (_maxNotGrayMember && player->isAlive() &&
            _maxNotGrayMember->getLevel() >= player->getLevel())
            xp = _isFullXP ?
            uint32(xp * rate) :             // Reward FULL XP if all group members are not gray.
            uint32(xp * rate / 2) + 1;      // Reward only HALF of XP if some of group members are gray.
        else
            xp = 0;
    }
    if (xp)
    {
        xp *= player->GetTotalAuraMultiplier(SPELL_AURA_MOD_XP_PCT);
        xp *= player->GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_XP_FROM_CREATURE_TYPE, int32(_victim->GetCreatureType()));

        // 4.2.3. Give XP to player.
        // player->GiveXP(xp, _victim, _groupRate);
        float groupRate = _groupRate;
        ObjectGuid victimGuid = _victim ? _victim->GetGUID() : ObjectGuid::Empty;
        player->AddDelayedEvent(50, [=]() -> void
        {
            if (Unit* target = ObjectAccessor::GetUnit(*player, victimGuid))
                player->GiveXP(xp, target, groupRate);
            else
                player->GiveXP(xp, nullptr, groupRate);
        });
    }
}

inline void KillRewarder::_RewardReputation(Player* player, float rate)
{
    // 4.3. Give reputation (player must not be on BG).
    // Even dead players and corpses are rewarded.
    if (Creature* creature = _victim->ToCreature())
        if (uint32 questId = creature->GetTrackingQuestID())
            if(player->IsQuestRewarded(questId))
                return;

    player->RewardReputation(_victim, rate, _killer == player);
}

inline void KillRewarder::_RewardKillCredit(Player* player)
{
    // 4.4. Give kill credit (player must not be in group, or he must be alive or without corpse).
    if (!_group || player->isAlive() || !player->GetCorpse())
        if (Creature* creature = _victim->ToCreature())
        {
            ObjectGuid victimGuid = creature->GetGUID();
            player->AddDelayedEvent(50, [=]() -> void
            {
                if (Creature* target = ObjectAccessor::GetCreature(*player, victimGuid))
                {
                    player->KilledMonster(target->GetCreatureTemplate(), victimGuid);
                    target->AddRewardPlayer(player->GetGUID());
                }
            });
        }
}

void KillRewarder::_RewardPlayer(Player* player, bool isDungeon)
{
    // 4. Reward player.
    if (!_isBattleGround)
    {
        // 4.1. Give honor (player must be alive and not on BG).
        _RewardHonor(player);
        // 4.1.1 Send player killcredit for quests with PlayerSlain
        if (_victim->IsPlayer())
            player->KilledPlayerCredit();
    }

    // Give XP only in PvE or in battlegrounds. Give reputation and kill credit only in PvE.
    if (!_isPvP || _isBattleGround)
    {
        const float rate = _group ? _groupRate * float(player->getLevel()) / _sumLevel : 1.0f;// Group rate depends on summary level. Personal rate is 100%.
        if (_xp)
            _RewardXP(player, rate);
        if (!_isBattleGround)
        {
            if (Creature* creature = _victim->ToCreature())
                if (creature->IsPlayerRewarded(player->GetGUID()))
                    return;

            _RewardReputation(player, isDungeon ? 1.0f : rate);
            _RewardKillCredit(player);
        }
    }
}

void KillRewarder::_RewardGroup()
{
    if (_maxLevel)
    {
        if (_maxNotGrayMember)
            // 3.1.1. Initialize initial XP amount based on maximum level of group member, for whom victim is not gray.
            _InitXP(_maxNotGrayMember);

        // To avoid unnecessary calculations and calls, proceed only if XP is not ZERO or player is not on battleground (battleground rewards only XP, that's why).
        if (!_isBattleGround || _xp)
        {
            const bool isDungeon = !_isPvP && sMapStore.LookupEntry(_killer->GetMapId())->IsDungeon();
            if (!_isBattleGround)
                // 3.1.2. Alter group rate if group is in raid (not for battlegrounds).
                _groupRate = Trinity::XP::xp_in_group_rate(_countForRep, !_isPvP && sMapStore.LookupEntry(_killer->GetMapId())->IsRaid() && _group->isRaidGroup());

            // 3.1.3. Reward each group member (even dead or corpse) within reward distance.
            for (GroupReference* itr = _group->GetFirstMember(); itr != nullptr; itr = itr->next())
                if (Player* member = itr->getSource())
                    if (member->IsAtGroupRewardDistance(_victim) && member->CanContact())
                        _RewardPlayer(member, isDungeon);
        }
    }
}

void KillRewarder::Reward()
{
    // 3. Reward killer (and group, if necessary).
    if (_group)
        _RewardGroup(); // 3.1. If killer is in group, reward group.
    else
    {
        // 3.2. Reward single killer (not group case).
        // 3.2.1. Initialize initial XP amount based on killer's level.
        _InitXP(_killer);
        // To avoid unnecessary calculations and calls,
        // proceed only if XP is not ZERO or player is not on battleground
        // (battleground rewards only XP, that's why).
        if (!_isBattleGround || _xp)
            _RewardPlayer(_killer, false); // 3.2.2. Reward killer.
    }

    // 5. Credit instance encounter.
    // 6. Update guild achievements.
    if (Creature* victim = _victim->ToCreature())
    {
        if (victim->IsDungeonBoss())
            if (Map* instance = _victim->GetMap())
                instance->UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, _victim->GetEntry(), _victim, _killer);

        if (Guild* guild = sGuildMgr->GetGuildById(_killer->GetGuildId()))
            guild->UpdateAchievementCriteria(CRITERIA_TYPE_KILL_CREATURE, victim->GetEntry(), 1, 0, victim, _killer);

        if (Map* instance = _victim->GetMap())
            if (uint32 instanceId =  instance->GetInstanceId())
                if (Scenario* progress = sScenarioMgr->GetScenario(instanceId))
                    progress->UpdateAchievementCriteria(CRITERIA_TYPE_KILL_CREATURE, victim->GetEntry(), 1, 0, victim, _killer);
    }
}
