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

#include "ScriptMgr.h"

#include "BattlegroundArathiBasin.h"
#include "BattlegroundWarsongGulch.h"
#include "BattlegroundIsleOfConquest.h"
#include "BattlegroundStrandOfTheAncients.h"
#include "BattlegroundAlteracValley.h"
#include "Vehicle.h"

class achievement_resilient_victory : public AchievementCriteriaScript
{
    public:
        achievement_resilient_victory() : AchievementCriteriaScript("achievement_resilient_victory") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            Battleground* bg = source->GetBattleground();
            if (!bg)
                return false;

            if (bg->GetTypeID(true) != MS::Battlegrounds::BattlegroundTypeId::BattlegroundArathiBasin)
                return false;

            if (!static_cast<BattlegroundArathiBasin*>(bg)->IsTeamScores500Disadvantage(source->GetTeam()))
                return false;

            return true;
        }
};

class achievement_bg_control_all_nodes : public AchievementCriteriaScript
{
    public:
        achievement_bg_control_all_nodes() : AchievementCriteriaScript("achievement_bg_control_all_nodes") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            Battleground* bg = source->GetBattleground();
            if (!bg)
                return false;

            if (!bg->IsAllNodesConrolledByTeam(source->GetTeam()))
                return false;

            return true;
        }
};

class achievement_save_the_day : public AchievementCriteriaScript
{
    public:
        achievement_save_the_day() : AchievementCriteriaScript("achievement_save_the_day") { }

        bool OnCheck(Player* source, Unit* target) override
        {
            if (!target)
                return false;

            if (Player const* player = target->ToPlayer())
            {
                Battleground* bg = source->GetBattleground();
                if (!bg)
                    return false;

                if (bg->GetTypeID(true) != MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch)
                    return false;

                if (static_cast<BattlegroundWarsongGulch*>(bg)->GetFlagState(player->GetTeam()) == BG_WS_FLAG_STATE_ON_BASE)
                    return true;
            }
            return false;
        }
};

class achievement_bg_ic_resource_glut : public AchievementCriteriaScript
{
    public:
        achievement_bg_ic_resource_glut() : AchievementCriteriaScript("achievement_bg_ic_resource_glut") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            if (source->HasAura(SPELL_OIL_REFINERY) && source->HasAura(SPELL_QUARRY))
                return true;

            return false;
        }
};

class achievement_bg_ic_glaive_grave : public AchievementCriteriaScript
{
    public:
        achievement_bg_ic_glaive_grave() : AchievementCriteriaScript("achievement_bg_ic_glaive_grave") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            if (Creature* vehicle = source->GetVehicleCreatureBase())
            {
                if (vehicle->GetEntry() == NPC_GLAIVE_THROWER_H ||  vehicle->GetEntry() == NPC_GLAIVE_THROWER_A)
                    return true;
            }

            return false;
        }
};

class achievement_bg_ic_mowed_down : public AchievementCriteriaScript
{
    public:
        achievement_bg_ic_mowed_down() : AchievementCriteriaScript("achievement_bg_ic_mowed_down") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            if (Creature* vehicle = source->GetVehicleCreatureBase())
            {
                if (vehicle->GetEntry() == NPC_KEEP_CANNON)
                    return true;
            }

            return false;
        }
};

class achievement_bg_sa_artillery : public AchievementCriteriaScript
{
    public:
        achievement_bg_sa_artillery() : AchievementCriteriaScript("achievement_bg_sa_artillery") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            if (Creature* vehicle = source->GetVehicleCreatureBase())
            {
                if (vehicle->GetEntry() == NPC_ANTI_PERSONNAL_CANNON)
                    return true;
            }

            return false;
        }
};

class achievement_arena_kills : public AchievementCriteriaScript
{
    public:
        achievement_arena_kills(char const* name, uint8 arenaType) : AchievementCriteriaScript(name),
            _arenaType(arenaType)
        {
        }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            // this checks GetBattleground() for NULL already
            if (!source->InArena())
                return false;

            return source->GetBattleground()->GetJoinType() == _arenaType;
        }

    private:
        uint8 const _arenaType;
};

class achievement_sickly_gazelle : public AchievementCriteriaScript
{
public:
    achievement_sickly_gazelle() : AchievementCriteriaScript("achievement_sickly_gazelle") { }

    bool OnCheck(Player* /*source*/, Unit* target) override
    {
        if (!target)
            return false;

        if (Player* victim = target->ToPlayer())
            if (victim->IsMounted())
                return true;

        return false;
    }
};

class achievement_everything_counts : public AchievementCriteriaScript
{
    public:
        achievement_everything_counts() : AchievementCriteriaScript("achievement_everything_counts") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            Battleground* bg = source->GetBattleground();
            if (!bg)
                return false;

            if (bg->GetTypeID(true) != MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley)
                return false;

            if (static_cast<BattlegroundAlteracValley*>(bg)->IsBothMinesControlledByTeam(source->GetTeam()))
                return true;

            return false;
        }
};

class achievement_bg_av_perfection : public AchievementCriteriaScript
{
    public:
        achievement_bg_av_perfection() : AchievementCriteriaScript("achievement_bg_av_perfection") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            Battleground* bg = source->GetBattleground();
            if (!bg)
                return false;

            if (bg->GetTypeID(true) != MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley)
                return false;

            if (static_cast<BattlegroundAlteracValley*>(bg)->IsAllTowersControlledAndCaptainAlive(source->GetTeam()))
                return true;

            return false;
        }
};

class achievement_bg_sa_defense_of_ancients : public AchievementCriteriaScript
{
    public:
        achievement_bg_sa_defense_of_ancients() : AchievementCriteriaScript("achievement_bg_sa_defense_of_ancients")
        {
        }

        bool OnCheck(Player* player, Unit* /*target*/) override
        {
            if (!player)
                return false;

            Battleground* battleground = player->GetBattleground();
            if (!battleground)
                return false;

            if (player->GetTeamId() == static_cast<BattlegroundStrandOfTheAncients*>(battleground)->Attackers)
                return false;

            if (!static_cast<BattlegroundStrandOfTheAncients*>(battleground)->gateDestroyed)
                return true;

            return false;
        }
};

enum ArgentTournamentAreas
{
    AREA_ARGENT_TOURNAMENT_FIELDS  = 4658,
    AREA_RING_OF_ASPIRANTS         = 4670,
    AREA_RING_OF_ARGENT_VALIANTS   = 4671,
    AREA_RING_OF_ALLIANCE_VALIANTS = 4672,
    AREA_RING_OF_HORDE_VALIANTS    = 4673,
    AREA_RING_OF_CHAMPIONS         = 4669,
};

class achievement_tilted : public AchievementCriteriaScript
{
    public:
        achievement_tilted() : AchievementCriteriaScript("achievement_tilted") {}

        bool OnCheck(Player* player, Unit* /*target*/) override
        {
            if (!player)
                return false;

            bool checkArea = player->GetCurrentAreaID() == AREA_ARGENT_TOURNAMENT_FIELDS ||
                                player->GetCurrentAreaID() == AREA_RING_OF_ASPIRANTS ||
                                player->GetCurrentAreaID() == AREA_RING_OF_ARGENT_VALIANTS ||
                                player->GetCurrentAreaID() == AREA_RING_OF_ALLIANCE_VALIANTS ||
                                player->GetCurrentAreaID() == AREA_RING_OF_HORDE_VALIANTS ||
                                player->GetCurrentAreaID() == AREA_RING_OF_CHAMPIONS;

            return checkArea && player->duel && player->duel->isMounted;
        }
};

class achievement_not_even_a_scratch : public AchievementCriteriaScript
{
    public:
        achievement_not_even_a_scratch() : AchievementCriteriaScript("achievement_not_even_a_scratch") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            if (!source)
                return false;

            Battleground* battleground = source->GetBattleground();
            if (!battleground)
                return false;

            if (static_cast<BattlegroundStrandOfTheAncients*>(battleground)->notEvenAScratch(source->GetTeam()))
                return true;

            return false;
        }
};

class achievement_a_test_of_valor : public AchievementCriteriaScript
{
    public:
        achievement_a_test_of_valor() : AchievementCriteriaScript("achievement_a_test_of_valor") { }

        bool OnCheck(Player* source, Unit* /*target*/) override
        {
            if (source->GetQuestStatus(32474) == QUEST_STATUS_INCOMPLETE || source->GetQuestStatus(32476) == QUEST_STATUS_INCOMPLETE)
                return true;

            return false;
        }
};

//BG: Deepwind Gorge. Achiev: Weighed Down
class achievement_weighed_down : public AchievementCriteriaScript
{
    public:
        achievement_weighed_down() : AchievementCriteriaScript("achievement_weighed_down") {}

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!player)
                return false;

            if (target->HasAura(140876))
                return true;

            if (AchievementEntry const* achiev = sAchievementStore.LookupEntry(8355))
                if (!player->HasAchieved(8355))
                    player->CompletedAchievement(achiev);

            return false;
        }
};

// Switch item reward faction
class achievement_reward : public AchievementRewardScript
{
public:
    achievement_reward() : AchievementRewardScript("achievement_reward") {}

    uint32 SelectItem(Player* source, AchievementReward const* reward)
    {
        switch (reward->itemId)
        {
            case 129280: // http://wowhead.com/achievement=11474
                if (source->GetTeam() == HORDE)
                    return 143864;
            default:
                return 0;
        }
    }
};

//12401
class achievement_glory_of_the_ulduar_raider : public AchievementRewardScript
{
public:
    achievement_glory_of_the_ulduar_raider() : AchievementRewardScript("achievement_glory_of_the_ulduar_raider") {}

    bool OnGet(Player* source, AchievementReward const* reward)
    {
        if (source)
        {
            source->learnSpell(63963, false); //Rusted Proto-Drake
            source->learnSpell(63956, false); //Ironbound Proto-Drake
            return true;
        }
        
        return false;
    }
};

//37930
class achievement_kill_antoran_demons : public AchievementCriteriaScript
{
public:
    achievement_kill_antoran_demons() : AchievementCriteriaScript("achievement_kill_antoran_demons") { }

    bool OnCheck(Player* source, Unit* target) override
    {
        if (!target)
            return false;

        if (auto cre = target->ToCreature())
            if (cre->GetMapId() == 1669 && cre->GetCreatureType() == CREATURE_TYPE_DEMON)
                return true;

        return false;
    }
};

//24823
class achievement_almost_blink_luck : public AchievementCriteriaScript
{
public:
    achievement_almost_blink_luck() : AchievementCriteriaScript("achievement_almost_blink_luck") { }

    bool OnCheck(Player* source, Unit* target) override
    {
        if (!source)
            return false;

        if (source->HasAura(65607))
            if (IsEventActive(24))
                return true;

        return false;
    }
};

//17836
class achievement_master_of_the_molten_flow : public AchievementCriteriaScript
{
public:
    achievement_master_of_the_molten_flow() : AchievementCriteriaScript("achievement_master_of_the_molten_flow") { }

    bool OnCheck(Player* source, Unit* target) override
    {
        if (!target)
            return false;

        if (auto npc = target->ToCreature())
            if (npc->IsAIEnabled && npc->AI()->GetData(1))
                return true;

        return false;
    }
};

//30103
class achievement_a_classy_outfit : public AchievementCriteriaScript
{
public:
    achievement_a_classy_outfit() : AchievementCriteriaScript("achievement_a_classy_outfit") { }

    bool OnCheck(Player* source, Unit* target) override
    {
        if (!source)
            return false;

        auto xp = source->GetArtifactWeapon()->GetUInt64Value(ITEM_FIELD_ARTIFACT_XP);
        if (xp && xp >= 100000)
            return true;

        return false;
    }
};

//19825
class achievement_run_with_the_wind : public AchievementCriteriaScript
{
public:
    achievement_run_with_the_wind() : AchievementCriteriaScript("achievement_run_with_the_wind") { }

    bool OnCheck(Player* source, Unit* target) override
    {
        if (!source)
            return false;

        if (source->HasAura(125146))
            return true;

        return false;
    }
};

void AddSC_achievement_scripts()
{
    new achievement_resilient_victory();
    new achievement_bg_control_all_nodes();
    new achievement_save_the_day();
    new achievement_bg_ic_resource_glut();
    new achievement_bg_ic_glaive_grave();
    new achievement_bg_ic_mowed_down();
    new achievement_bg_sa_artillery();
    new achievement_sickly_gazelle();
    new achievement_everything_counts();
    new achievement_bg_av_perfection();
    new achievement_arena_kills("achievement_arena_2v2_kills", MS::Battlegrounds::JoinType::Arena2v2);
    new achievement_arena_kills("achievement_arena_3v3_kills", MS::Battlegrounds::JoinType::Arena3v3);
    new achievement_bg_sa_defense_of_ancients();
    new achievement_tilted();
    new achievement_not_even_a_scratch();
    new achievement_a_test_of_valor();
    new achievement_weighed_down();
    new achievement_reward();
    new achievement_glory_of_the_ulduar_raider();
    new achievement_kill_antoran_demons();
    new achievement_almost_blink_luck();
    new achievement_master_of_the_molten_flow();
    //new achievement_a_classy_outfit();
    new achievement_run_with_the_wind();
}
