/*
* Copyright (C) 2012-2013 JadeCore <http://www.pandashan.com/>
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "GameObjectAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "terrace_of_endless_spring.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "GridNotifiers.h"

enum Platform
{
    PLATFORM_MIDDLE = 1,
    PLATFORM_RIGHT = 2,
    PLATFORM_LEFT = 3,
};
enum eShaOfFearSpells
{
    // Sha of Fear
    SPELL_REACHING_ATTACK = 119775,
    SPELL_EERIE_SKULL = 119519,
    SPELL_ENERGY_TO_ZERO = 119417,
    SPELL_BREATH_OF_FEAR = 119414,
    SPELL_THRASH_AURA = 131996,
    SPELL_THRASH_EXTRA_ATTACKS = 131994,
    SPELL_CONJURE_TERROR_SPAWN_TICK = 119108,
    SPELL_BREATH_OF_FEAR_COSMETIC = 129098,
    // 4 spells for spawn, cauz' of different spawn coordinates
    SPELL_CONJURE_TERROR_SPAWN_01 = 119312,
    SPELL_CONJURE_TERROR_SPAWN_02 = 119370,
    SPELL_CONJURE_TERROR_SPAWN_03 = 119371,
    SPELL_CONJURE_TERROR_SPAWN_04 = 119372,
    //   SPELL_BREATH_OF_FEAR          = 125786,

    // Other mobs
    WALL_OF_LIGHT_IMMUNITY_2 = 117999,
    WALL_OF_LIGHT_IMMUNITY = 117964,
    SPELL_LIGHT_WALL = 117865,
    SPELL_CHAMPION_OF_LIGHT = 117866,
    SPELL_LIGHT_WALL_READY = 117770,
    SPELL_LIGHT_WALL_VISUAL = 107145,
    SPELL_FEARLESS = 118977,
    SPELL_WALL_OF_LIGHT_BUFF = 117999,
    SPELL_PENETRATING_BOLT = 129075,
    SPELL_PENETRATING_BOLT_MISSILE = 129077,
    SPELL_DARK_BULWARK = 119083,

    // CACKLE
    SPELL_CACKLE_CAST = 119593,
    SPELL_CACKLE_TRANSFORM = 129147,

    // GUARDIAN
    DREAD_SPRAY_EFFECT = 119983,
    DREAD_SPRAY = 119956,
    DREAD_SPRAY_BUFF = 120047,
    DREAD_SPRAY_FEAR = 119985,

    GLOBUE_SPAWN = 129178,
    GLOBUE_VISUAL = 129187,
    GLOBUE_EFFECTY = 129189,
    GLOBUE_BOSS_HEAL = 129190,

    DEATH_BLOSSOM_CAST = 119888,
    DEATH_BLOSSOM_DAMAGE = 119887,
    DEATH_BLOSSOM_ARROW_VISUAL_1 = 119890,
    DEATH_BLOSSOM_ARROW_VISUAL_2 = 119944,
    DEATH_BLOSSOM_ARROW_VISUAL_3 = 119945,

    FEARLESS = 118977,

    SPELL_SHOT = 119862,
};

enum eShaOfFearEvents
{
    EVENT_CHECK_MELEE = 1,
    EVENT_EERIE_SKULL = 2,
    EVENT_CHECK_ENERGY = 3,
    EVENT_FIRST_TERRORS = 4,
    EVENT_PENETRATING_BOLT = 5,
    EVENT_SCHEDULE_LIGHT_WALL_VISUAL = 6,
    EVENT_BREATH_OF_FEAR = 7,
    EVENT_SHA_GLOBULES = 8,
    EVENT_DREAD_SPRAY = 9,
    EVENT_DEATH_BLOSSOM = 10,
    EVENT_DREAD_DAMAGE = 11,
    EVENT_SHOT = 12,
    EVENT_CACKLE = 13,
};

enum eShaOfFearActions
{
    ACTION_ACTIVATE_WALL_OF_LIGHT,
    ACTION_DESACTIVATE_WALL_OF_LIGHT,
    ACTION_SPAWN_TERROR,
    ACTION_SPAWN_CHENG,
    ACTION_SPAWN_YANG,
    ACTION_SPAWN_JULU,
    ACTION_REMOVE_DREAD_SPRAY,
};

enum eShaOfFearSays
{
    TALK_INTRO = 2,
    TALK_AGGRO = 0,
    TALK_DEATH = 1,
    TALK_BREATH_OF_FEAR = 4,
    TALK_SLAY = 3,
    TALK_SLAY_HEROIC,
    TALK_SUBMERGE = 6,
    TALK_HUDDLE = 5,
    TALK_LAST_PHASE = 7,
};
enum EntriesOfTriggers
{
    spawnWallOfLightEntry = 654840,
    spawnWallOfLightVisualEntry = 61797,
    spawnWallOfPureLightEntry = 60788,

    // DREAD SPRAY
    dread_spray_1_8 = 654842,
    dread_spray_2_8 = 654843,
    dread_spray_3_8 = 654844,
    dread_spray_4_8 = 654845,
    dread_spray_5_8 = 654846,
    dread_spray_6_8 = 654847,
    dread_spray_7_8 = 654848,
    dread_spray_8_8 = 654849,

    BACK_TO_TERRACE_PORTAL_1 = 65736,
    BACK_TO_TERRACE_PORTAL_2 = 657360,
    BACK_TO_TERRACE_PORTAL_3 = 657361,
};
enum cacklecreatures
{
    CHENG = 61042,
    YANG = 61038,
    JULU = 61046,
};

Player* GetChampionOfLight(Creature* me)
{
    if (!me)
        return NULL;

    Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
        if (Player* player = itr->getSource())
            if (player->HasAura(SPELL_CHAMPION_OF_LIGHT))
                return player;

    return NULL;
}
Position spawnTerrorPos[4] =
{
    { -1052.588f, -2788.164f, 38.268f, 0.0f },
    { -983.4550f, -2787.942f, 38.269f, 0.0f },
    { -989.6860f, -2772.245f, 38.303f, 0.0f },
    { -1046.274f, -2772.215f, 38.303f, 0.0f }
};

Position platform1 = { -1079.24f, -2552.98f, 15.87f, 4.932f }; // changs
Position platform2 = { -810.77f, -2741.60f, 31.704f, 3.277f }; // julu
Position platform3 = { -1235.58f, -2832.87f, 41.269f, 0.328124f }; // yang

Position yang_guoshi = { -1214.80f, -2824.82f, 41.243f, 3.506720f }; // left
Position cheng_kang = { -1075.19f, -2577.82f, 15.851f, 1.742210f }; // middle
Position julu_guoshi = { -832.07f, -2745.39f, 31.677f, 0.153694f }; // right

Position returntomainplat = { -954.64f, -2754.13f, 37.742f, 3.253044f }; // left
Position returntomainplat2 = { -1043.43f, -2729.68f, 38.254f, 5.220324f }; // middle
Position returntomainplat3 = { -1077.80f, -2753.44f, 37.747f, 5.918619f }; // right

Position spawnWallOfLightVisual[2] =
{
    { -990.25f, -2815.58f, 38.254f, 2.125254f },
    { -1045.28f, -2816.16f, 38.254f, 1.028898f }
};

Position spawnWallOfLight[6] =
{
    { -1024.51f, -2818.54f, 38.254f, 0.0f },
    { -1023.32f, -2810.017f, 38.266f, 0.0f },
    { -1021.94f, -2799.86f, 38.300f, 0.0f },
    { -1013.84f, -2799.90f, 38.303f, 0.0f },
    { -1012.22f, -2810.20f, 38.266f, 0.0f },
    { -1011.08f, -2818.51f, 38.254f, 0.0f }
};

SpecIndex std_dps_specs_spectrum[23] = { SPEC_HUNTER_BEASTMASTER, SPEC_HUNTER_MARKSMAN, SPEC_HUNTER_SURVIVAL, SPEC_PRIEST_SHADOW, SPEC_ROGUE_ASSASSINATION
, SPEC_ROGUE_COMBAT, SPEC_ROGUE_SUBTLETY, SPEC_SHAMAN_ELEMENTAL, SPEC_SHAMAN_ENHANCEMENT, SPEC_WARLOCK_AFFLICTION, SPEC_WARLOCK_DEMONOLOGY, SPEC_WARLOCK_DESTRUCTION,
SPEC_DK_UNHOLY, SPEC_DK_FROST, SPEC_DRUID_CAT, SPEC_DRUID_BALANCE, SPEC_WARRIOR_FURY, SPEC_WARRIOR_ARMS, SPEC_PALADIN_RETRIBUTION, SPEC_MAGE_FROST, SPEC_MAGE_FIRE
, SPEC_MAGE_ARCANE, SPEC_MONK_WINDWALKER };
SpecIndex std_heal_specs_spectrump[6] = { SPEC_SHAMAN_RESTORATION, SPEC_MONK_MISTWEAVER, SPEC_PRIEST_HOLY, SPEC_PRIEST_DISCIPLINE, SPEC_DRUID_RESTORATION, SPEC_PALADIN_HOLY };
SpecIndex std_tank_specs_spectrum[5] = { SPEC_MONK_BREWMASTER, SPEC_DK_BLOOD, SPEC_DRUID_BEAR, SPEC_PALADIN_PROTECTION, SPEC_PALADIN_PROTECTION };


/*

if(target->ToPlayer()->getClass() == CLASS_WARLOCK  | target->ToPlayer()->getClass() == CLASS_HUNTER || target->ToPlayer()->getClass() == CLASS_MAGE || target->ToPlayer()->getClass() == CLASS_ROGUE ||
target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DK_FROST) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DK_UNHOLY) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_SHAMAN_ELEMENTAL) ||
target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_SHAMAN_ENHANCEMENT) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_PALADIN_RETRIBUTION) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DROOD_CAT)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DROOD_BALANCE) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_WARRIOR_FURY) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_WARRIOR_ARMS)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_PRIEST_SHADOW) || target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_MONK_WINDWALKER))
{
if (dps <= 3)
{

dps += 1;
}
}
else if (target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_PALADIN_HOLY)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_PRIEST_DISCIPLINE)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_PRIEST_HOLY)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DROOD_RESTORATION)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_SHAMAN_RESTORATION)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_MONK_MISTWEAVER))
{
if (healer <= 1)
{
STD_NEW_GROUP.push_back(target->ToPlayer());
//STD_LOADED_PLAYERS.remove(std_loaded);
healer += 1;
}
}
else if (target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_PALADIN_PROTECTION)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_WARRIOR_PROTECTION)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DK_BLOOD)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_DROOD_BEAR)
|| target->ToPlayer()->GetSpecializationId(target->ToPlayer()->GetActiveSpec() == SPEC_MONK_BREWMASTER))
{
if (tank <= 1)
{
STD_NEW_GROUP.push_back(target->ToPlayer());
//STD_LOADED_PLAYERS.remove(std_loaded);
tank += 1;
}
}
}

if (dps > 2 && healer > 0 && tank > 0)
RampRand();


void RampRand()
{
while (!reramp)
{
// kick in platforms
switch (urand(0, 2))
{
case 0:
if (!cheng)
{
cheng = true;
OminousCacklePort();
reramp = true;
}
break;
case 1:
if (!yang)
{
yang = true;
OminousCacklePort();
reramp = true;
}
break;
case 2:
if (!julu)
{
julu = true;
OminousCacklePort();
reramp = true;
}
break;
}
}
}
void OminousCacklePort()
{
if (STD_NEW_GROUP.empty())
return;

if (!me && !me->IsInWorld() || me->isDead())
return;

if (cheng)
{
me->GetAI()->DoAction(ACTION_SPAWN_CHENG);
for (auto pls : STD_NEW_GROUP)
{
pls->NearTeleportTo(platform1.GetPositionX(), platform1.GetPositionY(), platform1.GetPositionZ(), platform1.GetOrientation());
reramp = false;
}
}
else if (yang)
{
me->GetAI()->DoAction(ACTION_SPAWN_YANG);
for (auto pls : STD_NEW_GROUP)
{
pls->NearTeleportTo(platform2.GetPositionX(), platform2.GetPositionY(), platform2.GetPositionZ(), platform2.GetOrientation());
reramp = false;
}
}
else if (julu)
{
me->GetAI()->DoAction(ACTION_SPAWN_JULU);
for (auto pls : STD_NEW_GROUP)
{
pls->NearTeleportTo(platform3.GetPositionX(), platform3.GetPositionY(), platform3.GetPositionZ(), platform3.GetOrientation());
reramp = false;
}
}
}

*/

class boss_sha_of_fear : public CreatureScript
{
public:
    boss_sha_of_fear() : CreatureScript("boss_sha_of_fear") { }

    struct boss_sha_of_fearAI : public BossAI
    {
        boss_sha_of_fearAI(Creature* creature) : BossAI(creature, DATA_SHA_OF_FEAR)
        {
            pInstance = creature->GetInstanceScript();
            me->setFaction(16);
            me->Respawn();
        }

        InstanceScript* pInstance;
        EventMap events;

        std::list<Player*> STD_LOADED_PLAYERS;
        std::list<Player*> STD_NEW_GROUP;
        uint8 attacksCounter;
        uint8 terrorCounter;
        int dps;
        int tank;
        int healer;

        bool cheng;
        bool yang;
        bool julu;
        bool reramp;

        bool ominous;

        void Reset()
        {
            me->setFaction(16);

            _Reset();
            events.Reset();
            summons.DespawnAll();

            attacksCounter = 0;
            terrorCounter = 0;

            dps = 0;
            healer = 0;
            tank = 0;

            ominous = false;

            me->SetPower(POWER_ENERGY, 0);
            me->SetInt32Value(UNIT_FIELD_POWER, 0);
            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

            if (pInstance)
            {
                if (pInstance->GetBossState(DATA_PROTECTORS) == DONE)
                {
                    me->setFaction(14);
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                    if (pInstance->GetData(SPELL_RITUAL_OF_PURIFICATION))
                        me->AddAura(SPELL_RITUAL_OF_PURIFICATION, me);
                }
                else
                {
                    me->setFaction(16);
                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 35);
                }
            }
        }
        void MoveInLineOfSight(Unit* who)
        {
            if (who)
            {
                if (who->IsWithinDistInMap(me, 30.0f, true))
                {
                    if (instance && instance->GetBossState(DATA_LEI_SHI) == DONE && me->getFaction() == 35)
                    {
                        Talk(TALK_INTRO);
                        me->setFaction(16);
                    }
                }
            }
        }
        void JustReachedHome()
        {
            _JustReachedHome();
            summons.DespawnEntry(NPC_TERROR_SPAWN);
            summons.DespawnEntry(spawnWallOfLightVisualEntry);
            summons.DespawnEntry(spawnWallOfPureLightEntry);

            if (pInstance)
                pInstance->SetBossState(DATA_SHA_OF_FEAR, FAIL);
        }

        void EnterCombat(Unit* attacker)
        {
            if (pInstance)
            {
                pInstance->SetBossState(DATA_SHA_OF_FEAR, IN_PROGRESS);
                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                events.ScheduleEvent(EVENT_CHECK_MELEE, 1000);
                events.ScheduleEvent(EVENT_EERIE_SKULL, 5000);
                events.ScheduleEvent(EVENT_CHECK_ENERGY, 1000);
                events.ScheduleEvent(EVENT_FIRST_TERRORS, 20000);
                events.ScheduleEvent(EVENT_CACKLE, 90 * IN_MILLISECONDS);

                cheng = false;
                yang = false;
                julu = false;
                reramp = false;

                DoZoneInCombat();
                Talk(TALK_AGGRO);

                me->SetPower(POWER_ENERGY, 0);
                me->SetInt32Value(UNIT_FIELD_POWER, 0);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetInt32Value(UNIT_FIELD_MAX_POWER, 100);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);
            }
        }

        void JustDied(Unit* killer)
        {
            if (pInstance)
            {
                summons.DespawnAll();
                pInstance->SetBossState(DATA_SHA_OF_FEAR, DONE);
                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();

                Map::PlayerList const &PlList = me->GetMap()->GetPlayers();
                if (PlList.isEmpty())
                    return;
                AchievementEntry const* achievementEntry = sAchievementStore.LookupEntry(6844);
                for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
                    if (Player* player = i->getSource())
                        if (player->GetAchievementMgr()->HasAchieved(6844))
                            player->CompletedAchievement(achievementEntry);
            }
        }
        void JustSummoned(Creature* summon)
        {
            if (summon->GetEntry() == NPC_TERROR_SPAWN)
                ++terrorCounter;

            summons.Summon(summon);
        }
        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_CACKLE_CAST)
            {
            }
        }
        void SummonedCreatureDespawn(Creature* summon)
        {
            if (summon->GetEntry() == NPC_TERROR_SPAWN)
                --terrorCounter;

            summons.Despawn(summon);
        }
        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (IsHeroic())
                    Talk(TALK_SLAY_HEROIC);
                else
                    Talk(TALK_SLAY);
            }
        }
        void DoAction(const int32 action)
        {
            switch (action)
            {
            case ACTION_SPAWN_TERROR:
            {
                if (!terrorCounter)
                {
                    me->CastSpell(spawnTerrorPos[0].GetPositionX(), spawnTerrorPos[0].GetPositionY(),
                        spawnTerrorPos[0].GetPositionZ(), SPELL_CONJURE_TERROR_SPAWN_01, true);
                    me->CastSpell(spawnTerrorPos[1].GetPositionX(), spawnTerrorPos[1].GetPositionY(),
                        spawnTerrorPos[1].GetPositionZ(), SPELL_CONJURE_TERROR_SPAWN_02, true);
                }
                else
                {
                    me->CastSpell(spawnTerrorPos[2].GetPositionX(), spawnTerrorPos[2].GetPositionY(),
                        spawnTerrorPos[2].GetPositionZ(), SPELL_CONJURE_TERROR_SPAWN_03, true);
                    me->CastSpell(spawnTerrorPos[3].GetPositionX(), spawnTerrorPos[3].GetPositionY(),
                        spawnTerrorPos[3].GetPositionZ(), SPELL_CONJURE_TERROR_SPAWN_04, true);
                }
                break;
            }
            case ACTION_SPAWN_CHENG:
            {
                if (ominous)
                {
                    ominous = false;
                    me->SummonCreature(CHENG, cheng_kang.GetPositionX(), cheng_kang.GetPositionY(), cheng_kang.GetPositionZ(), cheng_kang.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
                    break;
                }
            }
            case ACTION_SPAWN_YANG:
            {
                if (ominous)
                {
                    ominous = false;
                    me->SummonCreature(YANG, julu_guoshi.GetPositionX(), julu_guoshi.GetPositionY(), julu_guoshi.GetPositionZ(), julu_guoshi.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
                    break;
                }
            }
            case ACTION_SPAWN_JULU:
            {
                if (ominous)
                {
                    ominous = false;
                    me->SummonCreature(JULU, yang_guoshi.GetPositionX(), yang_guoshi.GetPositionY(), yang_guoshi.GetPositionZ(), yang_guoshi.GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
                    break;
                }
            }
            default:
                break;
            }
        }
        void DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType damageType)
        {
            if (damageType == DIRECT_DAMAGE)
            {
                if (attacksCounter >= 3 && !me->m_extraAttacks)
                {
                    me->CastSpell(me, SPELL_THRASH_EXTRA_ATTACKS, true);
                    attacksCounter = 0;
                }
                else if (attacksCounter >= 2 && !me->m_extraAttacks)
                {
                    me->CastSpell(me, SPELL_THRASH_AURA, true);
                    ++attacksCounter;
                }
                else if (!me->m_extraAttacks)
                {
                    me->RemoveAura(SPELL_THRASH_AURA);
                    ++attacksCounter;
                }
            }
        }
        void OnAddThreat(Unit* victim, float& fThreat, SpellSchoolMask /*schoolMask*/, SpellInfo const* /*threatSpell*/)
        {
            if (!victim->HasAura(SPELL_CHAMPION_OF_LIGHT))
                fThreat = 0;
            return;
        }
        void DespawnCreaturesInArea(uint32 entry, WorldObject* object)
        {
            std::list<Creature*> creatures;
            GetCreatureListWithEntryInGrid(creatures, object, entry, 300.0f);
            if (creatures.empty())
                return;

            for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }

        void RegeneratePower(Powers power, float& value) override
        {
            if (power != POWER_ENERGY)
                return;

            // Sha of Fear regenerates 6 energy every 2s (15 energy for 5s)
            value = 3;

            int32 val = me->GetPower(POWER_ENERGY);
            if (val + value > 100)
                val = 100;
            else
                val += value;

            if (value >= 100)
            {
                me->SetPower(POWER_ENERGY, 0);
                me->SetInt32Value(UNIT_FIELD_POWER, 0);
                events.ScheduleEvent(EVENT_CHECK_ENERGY, 1000);
                //events.ScheduleEvent(EVENT_BREATH_OF_FEAR, 1000);
            }

            me->SetInt32Value(UNIT_FIELD_POWER, val);
        }
        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
            {
                if (pInstance && pInstance->GetData(SPELL_RITUAL_OF_PURIFICATION) == false)
                    me->RemoveAura(SPELL_RITUAL_OF_PURIFICATION);

                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_CHECK_MELEE:
            {
                if (!me->IsWithinMeleeRange(me->getVictim(), 7.0f))
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f))
                        me->CastSpell(target, SPELL_REACHING_ATTACK, false);
                }
                else
                {
                    // Always attack champion of light
                    if (Player* target = GetChampionOfLight(me))
                        if (me->getVictim() && me->getVictim()->GetGUID() != target->GetGUID())
                            AttackStart(target);
                }

                events.ScheduleEvent(EVENT_CHECK_MELEE, 1000);
                break;
            }
            case EVENT_EERIE_SKULL:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                    me->CastSpell(target, SPELL_EERIE_SKULL, true);

                events.ScheduleEvent(EVENT_EERIE_SKULL, 5000);
                break;
            }
            case EVENT_CHECK_ENERGY:
            {
                if (me->GetUInt32Value(UNIT_FIELD_POWER) == 100)
                {
                    std::list<Creature*> bof_triggers;

                    GetCreatureListWithEntryInGrid(bof_triggers, me, 654841, 300.0f);

                    if (bof_triggers.empty())
                        return;

                    for (auto _bof : bof_triggers)
                    {
                        me->CastSpell(_bof, SPELL_BREATH_OF_FEAR, true);
                    }

                    me->SetPower(POWER_ENERGY, 0);
                    me->SetInt32Value(UNIT_FIELD_POWER, 0);
                    Talk(TALK_BREATH_OF_FEAR);
                }
                events.ScheduleEvent(EVENT_CHECK_ENERGY, 1000);
                break;
            }
            case EVENT_FIRST_TERRORS:
            {
                me->CastSpell(me, SPELL_CONJURE_TERROR_SPAWN_TICK, true);
                break;
            }
            case EVENT_CACKLE:
            {
                ominous = true;

                DespawnCreaturesInArea(65736, me);
                DespawnCreaturesInArea(657360, me);
                DespawnCreaturesInArea(657361, me);

                me->CastSpell(me, SPELL_CACKLE_CAST);
                events.ScheduleEvent(EVENT_CACKLE, 90 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
            }

            DoMeleeAttackIfReady();
        }
    private:
        bool _evadeCheck;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_sha_of_fearAI(creature);
    }
};

// Pure Light Terrace - 60788
class mob_pure_light_terrace : public CreatureScript
{
public:
    mob_pure_light_terrace() : CreatureScript("mob_pure_light_terrace") { }

    struct mob_pure_light_terraceAI : public ScriptedAI
    {
        mob_pure_light_terraceAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            pInstance = creature->GetInstanceScript();
            wallActivated = false;
            me->Respawn();
        }

        InstanceScript* pInstance;
        SummonList summons;
        bool wallActivated;

        void Reset()
        {
            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->CastSpell(me, SPELL_LIGHT_WALL, true);
            me->CastSpell(me, SPELL_LIGHT_WALL_READY, true);

            me->SetFacingTo(1.606599f);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
                case ACTION_ACTIVATE_WALL_OF_LIGHT:
                {
                    if (wallActivated)
                        return;

                    me->CastSpell(me, SPELL_LIGHT_WALL_VISUAL, true);
                    wallActivated = true;

                    me->RemoveAura(SPELL_LIGHT_WALL_READY);
                    break;
                }
                case ACTION_DESACTIVATE_WALL_OF_LIGHT:
                {
                    if (!wallActivated)
                        break;

                    wallActivated = false;
                    me->CastSpell(me, SPELL_LIGHT_WALL_READY, true);
                    me->RemoveAura(SPELL_LIGHT_WALL_VISUAL);
                    summons.DespawnEntry(spawnWallOfLightEntry);
                    summons.DespawnEntry(spawnWallOfLightVisualEntry);
                    break;
                }
                default:
                    break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
            return;
        }

        void UpdateAI(const uint32 diff)
        {
            /*
            Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
            if (Player* player = itr->getSource())
            {
            if (wallActivated = true)
            {
            if (me->isInFront(player))
            if (!player->HasAura(SPELL_WALL_OF_LIGHT_BUFF))
            player->CastSpell(player, SPELL_WALL_OF_LIGHT_BUFF, true);
            else
            player->RemoveAura(SPELL_WALL_OF_LIGHT_BUFF);
            }
            }
            }
            */
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_pure_light_terraceAI(creature);
    }
};

class trigger_PureLight_Visual : public CreatureScript
{
public:
    trigger_PureLight_Visual() : CreatureScript("trigger_PureLight_Visual") { }

    struct trigger_PureLight_VisualAI : public Scripted_NoMovementAI
    {
        trigger_PureLight_VisualAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        EventMap events;

        InstanceScript* pInstance;

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->setFaction(35);
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (Creature* PureLight = me->FindNearestCreature(spawnWallOfPureLightEntry, 300.0F, true))
            {
                me->CastSpell(PureLight, SPELL_LIGHT_WALL_VISUAL, true);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new trigger_PureLight_VisualAI(creature);
    }
};

class trigger_PureLight : public CreatureScript
{
public:
    trigger_PureLight() : CreatureScript("trigger_PureLight") { }

    struct trigger_PureLightAI : public ScriptedAI
    {
        trigger_PureLightAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;
        std::list<Player*> _lookForPlayersFearImmunity;

        void Reset()
        {
            me->SetUnitMovementFlags(MOVEMENTFLAG_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->setFaction(35);
        }
        void UpdateAI(const uint32 diff)
        {
            std::list<Player*> PL_list;

            Trinity::AnyPlayerInObjectRangeCheck check(me, 10.0);
            Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, PL_list, check);
            me->VisitNearbyObject(10.0, searcher);

            for (std::list<Player*>::const_iterator it = PL_list.begin(); it != PL_list.end(); ++it)
            {
                if (!(*it))
                    return;

                if (!(*it)->HasAura(SPELL_WALL_OF_LIGHT_BUFF))
                    me->AddAura(SPELL_WALL_OF_LIGHT_BUFF, (*it));
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new trigger_PureLightAI(creature);
    }
};


// Return to the Terrace - 65736
class mob_return_to_the_terrace : public CreatureScript
{
public:
    mob_return_to_the_terrace() : CreatureScript("mob_return_to_the_terrace") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player && player->IsInWorld())
        {
            switch (creature->GetEntry())
            {
            case BACK_TO_TERRACE_PORTAL_1:
                player->NearTeleportTo(returntomainplat.GetPositionX(), returntomainplat.GetPositionY(), returntomainplat.GetPositionZ(), returntomainplat.GetOrientation());
                break;
            case BACK_TO_TERRACE_PORTAL_2:
                player->NearTeleportTo(returntomainplat2.GetPositionX(), returntomainplat2.GetPositionY(), returntomainplat2.GetPositionZ(), returntomainplat2.GetOrientation());
                break;
            case BACK_TO_TERRACE_PORTAL_3:
                player->NearTeleportTo(returntomainplat3.GetPositionX(), returntomainplat3.GetPositionY(), returntomainplat3.GetPositionZ(), returntomainplat3.GetOrientation());
                break;
            }
            creature->AddAura(FEARLESS, player);
            return true;
        }
        return false;
    }
    struct mob_return_to_the_terraceAI : public ScriptedAI
    {
        mob_return_to_the_terraceAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;

        void Reset()
        {
            // Sniffed values
            me->SetObjectScale(2.8f);
            me->CastSpell(me, 120216);
            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
            me->SetFlag(UNIT_FIELD_INTERACT_SPELL_ID, 118977);
        }

        void UpdateAI(const uint32 diff) { }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_return_to_the_terraceAI(creature);
    }
};

// Terror Spawn - 61034
class mob_terror_spawn : public CreatureScript
{
public:
    mob_terror_spawn() : CreatureScript("mob_terror_spawn") { }

    struct mob_terror_spawnAI : public ScriptedAI
    {
        mob_terror_spawnAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            if (pInstance)
                if (Creature* pureLight = Creature::GetCreature(*me, pInstance->GetGuidData(NPC_PURE_LIGHT_TERRACE)))
                    me->SetFacingToObject(pureLight);

            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

            me->CastSpell(me, SPELL_DARK_BULWARK, true);

            if (me->GetMap()->Is25ManRaid())
            {
                me->SetMaxHealth(3700000);
                me->SetHealth(3700000);
            }
            else
            {
                me->SetMaxHealth(3700000);
                me->SetHealth(3700000);
            }

            events.Reset();
            events.ScheduleEvent(EVENT_PENETRATING_BOLT, 5000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (pInstance)
                if (Creature* pureLight = Creature::GetCreature(*me, pInstance->GetGuidData(NPC_PURE_LIGHT_TERRACE)))
                    me->SetFacingToObject(pureLight);

            switch (events.ExecuteEvent())
            {
            case EVENT_PENETRATING_BOLT:
            {
                me->CastSpell(me, SPELL_PENETRATING_BOLT, false);
                events.ScheduleEvent(EVENT_PENETRATING_BOLT, 5000);
                break;
            }
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_terror_spawnAI(creature);
    }
};

// CHENG, JULU, YANG
class mob_guardian : public CreatureScript
{
public:
    mob_guardian() : CreatureScript("mob_guardian") { }

    struct mob_guardianAI : public ScriptedAI
    {
        mob_guardianAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;
        EventMap events;
        SummonList summons;
        std::list<Creature*> dreadspraytriggers;
        int32 stackstobreak;
        bool dreadshotting;

        void Reset()
        {
            events.Reset();
            if (Is25ManRaid())
            {
                me->SetMaxHealth(14392521);
                me->SetHealth(14392521);
            }
            else
            {
                me->SetMaxHealth(377932);
                me->SetHealth(377932);
            }

            me->SetLevel(93);
            me->setFaction(16);
            me->SetReactState(REACT_AGGRESSIVE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

            dreadshotting = false;
        }
        void EnterCombat(Unit* attacker)
        {
            events.ScheduleEvent(EVENT_DREAD_SPRAY, 20 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_DEATH_BLOSSOM, 40 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_SHOT, 2000);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (roll_chance_i(25)) // NOT SNIFFED
            {
                me->CastSpell(me, GLOBUE_SPAWN);
            }
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
            case ACTION_REMOVE_DREAD_SPRAY:
                events.ScheduleEvent(EVENT_SHOT, 2000);
                break;
            }
        }
        void JustDied(Unit* killer)
        {
            if (Creature* SHA = me->GetMap()->GetCreature(pInstance->GetGuidData(NPC_SHA_OF_FEAR)))
            {
                if (boss_sha_of_fear::boss_sha_of_fearAI* linkAI = CAST_AI(boss_sha_of_fear::boss_sha_of_fearAI, SHA->GetAI()))
                {
                    switch (me->GetEntry())
                    {
                    case YANG:
                        me->SummonCreature(BACK_TO_TERRACE_PORTAL_1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        linkAI->yang = false;
                        linkAI->reramp = false;
                        break;
                    case CHENG:
                        me->SummonCreature(BACK_TO_TERRACE_PORTAL_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        linkAI->cheng = false;
                        linkAI->reramp = false;
                        break;
                    case JULU:
                        me->SummonCreature(BACK_TO_TERRACE_PORTAL_3, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        linkAI->julu = false;
                        linkAI->reramp = false;
                        break;
                    }
                }
            }
            me->DespawnOrUnsummon();
            summons.DespawnAll();
        }
        void UpdateAI(const uint32 diff)
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_SHOT:
            {
                me->CastSpell(me->getVictim(), SPELL_SHOT);
                events.ScheduleEvent(EVENT_SHOT, 2000);
                break;
            }
            case EVENT_DREAD_SPRAY:
            {
                dreadshotting = true;
                me->CastSpell(me, DREAD_SPRAY_BUFF);

                events.ScheduleEvent(EVENT_DREAD_SPRAY, 20 * IN_MILLISECONDS);
                break;
            }
            case EVENT_DEATH_BLOSSOM:
            {
                me->MonsterYell("Seek shelter, lest I strike you down like the sha commands", LANG_UNIVERSAL, me->GetGUID());

                me->CastSpell(me, DEATH_BLOSSOM_CAST);
                events.ScheduleEvent(EVENT_DEATH_BLOSSOM, 40 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_guardianAI(creature);
    }
};

// mob globue
class mob_globue : public CreatureScript
{
public:
    mob_globue() : CreatureScript("mob_globue") { }

    struct mob_globueAI : public ScriptedAI
    {
        mob_globueAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
            Init();
        }

        InstanceScript* pInstance;
        EventMap events;

        int32 stackstobreak;
        uint32 timertodespawn;

        void Init()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
            me->setFaction(35);

            me->CastSpell(me, GLOBUE_VISUAL);
            timertodespawn = 8000;
        }
        void MoveInLineOfSight(Unit* who)
        {
            if (who->IsWithinDistInMap(me, 1.0f, true))
            {
                // add aura
                me->AddAura(GLOBUE_EFFECTY, who);
                me->DespawnOrUnsummon();
            }
        }
        void UpdateAI(const uint32 diff)
        {
            if (timertodespawn <= diff)
            {
                if (Creature* cheng = me->FindNearestCreature(CHENG, 90.0f, true))
                    me->CastSpell(cheng, GLOBUE_BOSS_HEAL);

                if (Creature* Yang = me->FindNearestCreature(YANG, 90.0f, true))
                    me->CastSpell(Yang, GLOBUE_BOSS_HEAL);

                if (Creature* jol = me->FindNearestCreature(JULU, 90.0f, true))
                    me->CastSpell(jol, GLOBUE_BOSS_HEAL);

                me->DespawnOrUnsummon();
            }
            else
                timertodespawn -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_globueAI(creature);
    }
};

// Champion of Light - 117866
class spell_champion_of_light : public SpellScriptLoader
{
public:
    spell_champion_of_light() : SpellScriptLoader("spell_champion_of_light") { }

    class spell_champion_of_light_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_champion_of_light_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (GetTarget())
                {
                    if (caster->ToCreature() && caster->ToCreature()->AI() && GetTarget()->GetTypeId() == TYPEID_PLAYER)
                        caster->ToCreature()->AI()->DoAction(ACTION_ACTIVATE_WALL_OF_LIGHT);

                    caster->AddAura(SPELL_LIGHT_WALL_VISUAL, caster);

                    for (int i = 0; i <= 1; i++)
                    {
                        GetCaster()->SummonCreature(spawnWallOfLightVisualEntry, spawnWallOfLightVisual[i].GetPositionX(), spawnWallOfLightVisual[i].GetPositionY(), spawnWallOfLightVisual[i].GetPositionZ(), spawnWallOfLightVisual[i].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
                    }
                    for (int i = 0; i <= 6; i++)
                    {
                        GetCaster()->SummonCreature(spawnWallOfLightEntry, spawnWallOfLight[i].GetPositionX(), spawnWallOfLight[i].GetPositionY(), spawnWallOfLight[i].GetPositionZ(), spawnWallOfLight[i].GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0);
                    }
                }
            }
        }
        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (GetTarget())
                {
                    if (caster->ToCreature() && caster->ToCreature()->AI() && GetTarget()->GetTypeId() == TYPEID_PLAYER)
                        caster->ToCreature()->AI()->DoAction(ACTION_DESACTIVATE_WALL_OF_LIGHT);

                    std::list<Creature*> despawnLightWell;
                    std::list<Creature*> despawnLightWellnonvisual;

                    GetCaster()->GetCreatureListWithEntryInGrid(despawnLightWell, 61797, 300.0f);
                    GetCaster()->GetCreatureListWithEntryInGrid(despawnLightWellnonvisual, spawnWallOfLightEntry, 300.0f);

                    if (despawnLightWell.empty() || despawnLightWellnonvisual.empty())
                        return;

                    for (auto lightwelltriggers : despawnLightWell)
                    {
                        lightwelltriggers->DespawnOrUnsummon();
                    }
                    for (auto lightwelltriggersnonvisual : despawnLightWellnonvisual)
                    {
                        lightwelltriggersnonvisual->DespawnOrUnsummon();
                    }

                    InstanceScript* instance = GetCaster()->GetInstanceScript();

                    if (instance)
                    {
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WALL_OF_LIGHT_BUFF);
                    }
                }
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_champion_of_light_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_champion_of_light_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_champion_of_light_AuraScript();
    }
};

// Conjure Terror Spawn - 119108
class spell_conjure_terror_spawn : public SpellScriptLoader
{
public:
    spell_conjure_terror_spawn() : SpellScriptLoader("spell_conjure_terror_spawn") { }

    class spell_conjure_terror_spawn_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_conjure_terror_spawn_AuraScript);

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            if (Creature* caster = GetCaster()->ToCreature())
                caster->AI()->DoAction(ACTION_SPAWN_TERROR);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_conjure_terror_spawn_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_conjure_terror_spawn_AuraScript();
    }
};

// called by 120047
class dread_spray_buff : public SpellScriptLoader
{
public:
    dread_spray_buff() : SpellScriptLoader("dread_spray_buff") { }

    class dread_spray_buff_aura_script : public AuraScript
    {
        PrepareAuraScript(dread_spray_buff_aura_script);

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            if (Creature* caster = GetCaster()->ToCreature())
            {
                if (dreadspraytriggers.empty())
                    return;

                Creature* dreadtrigger = dreadspraytriggers.back();

                if (dreadtrigger)
                {
                    GetCaster()->SetFacingToObject(dreadtrigger);
                    GetCaster()->CastSpell(dreadtrigger, 119958, true); // visual is bugged?
                    dreadspraytriggers.remove(dreadtrigger);
                }
            }
        }
        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->ToCreature() && caster->ToCreature()->AI())
                    caster->GetAI()->DoAction(ACTION_REMOVE_DREAD_SPRAY);
            }
        }
        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                GetCaster()->GetCreatureListWithEntryInGrid(dreadspraytriggers, dread_spray_1_8, 65.0f);
            }
        }

    private:
        std::list<Creature*> dreadspraytriggers;

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(dread_spray_buff_aura_script::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(dread_spray_buff_aura_script::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(dread_spray_buff_aura_script::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new dread_spray_buff_aura_script();
    }
};
// called by 119953
class spell_dread_spray_hit : public SpellScriptLoader
{
public:
    spell_dread_spray_hit() : SpellScriptLoader("spell_dread_spray_hit") { }

    class spell_dread_spray_hit_spell_Script : public SpellScript
    {
        PrepareSpellScript(spell_dread_spray_hit_spell_Script);

        void HandleFearHit()
        {
            if (GetCaster())
            {
                if (GetHitUnit())
                {
                    if (GetHitUnit()->HasAura(119958))
                        GetCaster()->AddAura(DREAD_SPRAY_FEAR, GetHitUnit());
                }
            }
        }
        void Register()
        {
            OnHit += SpellHitFn(spell_dread_spray_hit_spell_Script::HandleFearHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dread_spray_hit_spell_Script();
    }
};
// Penetrating Bolt - 129075
class spell_penetrating_bolt : public SpellScriptLoader
{
public:
    spell_penetrating_bolt() : SpellScriptLoader("spell_penetrating_bolt") { }

    class spell_penetrating_bolt_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_penetrating_bolt_SpellScript);

        ObjectGuid targetGuid;

        void CorrectRange(std::list<WorldObject*>& targets)
        {
            targetGuid = ObjectGuid::Empty;

            if (!targets.empty())
                Trinity::Containers::RandomResizeList(targets, 1);

            for (auto itr : targets)
                if (itr->GetGUID())
                    targetGuid = itr->GetGUID();
        }

        void HandleDummy(SpellEffIndex index)
        {
            if (Unit* caster = GetCaster())
            {
                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (Player* target = Player::GetPlayer(*caster, targetGuid))
                        caster->CastSpell(target, SPELL_PENETRATING_BOLT_MISSILE, true, nullptr, nullptr, instance->GetGuidData(NPC_SHA_OF_FEAR));
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_penetrating_bolt_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectLaunch += SpellEffectFn(spell_penetrating_bolt_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_penetrating_bolt_SpellScript();
    }
};
// called by 119888
class spell_death_blossom : public SpellScriptLoader
{
public:
    spell_death_blossom() : SpellScriptLoader("spell_death_blossom") { }

    class spell_death_blossom_aura_Script : public AuraScript
    {
        PrepareAuraScript(spell_death_blossom_aura_Script);

        void DamagePeriodTimer(AuraEffect* aurEff)
        {
            if (GetCaster())
            {
                int32 timer = aurEff->GetPeriodicTimer();
                if (timer <= 1)
                    return;

                for (int i = 0; i <= 6; i++)
                    GetCaster()->CastSpell(GetCaster(), 119945, true); // VISUAL       
            }
        }

        void Register()
        {
            OnEffectUpdatePeriodic += AuraEffectUpdatePeriodicFn(spell_death_blossom_aura_Script::DamagePeriodTimer, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_death_blossom_aura_Script();
    }
};
// called by 119887
class spell_death_blossom_damage : public SpellScriptLoader
{
public:
    spell_death_blossom_damage() : SpellScriptLoader("spell_death_blossom_damage") { }

    class spell_massive_attacks_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_massive_attacks_SpellScript);

        void RecalculateDamage(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit() && GetCaster() && !GetHitUnit()->IsWithinLOSInMap(GetCaster()))
                SetHitDamage(0);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_massive_attacks_SpellScript::RecalculateDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_massive_attacks_SpellScript();
    }
};

class ObjectNonAuraImmune
{
    public:
        ObjectNonAuraImmune() {}
        bool operator()(WorldObject* object)
        {
            if (object->ToPlayer()->HasAura(SPELL_WALL_OF_LIGHT_BUFF) || object->ToPlayer()->HasAura(SPELL_CHAMPION_OF_LIGHT))
                return true;
            else
                return false;
        }
};

// Cleansing Waters - 117283
class spell_breath_of_fear_spell : public SpellScriptLoader
{
public:
    spell_breath_of_fear_spell() : SpellScriptLoader("spell_breath_of_fear_spell") { }

    class spell_cleansing_waters_regen_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_cleansing_waters_regen_SpellScript);

        void CorrectTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            targets.remove_if(ObjectNonAuraImmune());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_cleansing_waters_regen_SpellScript::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_cleansing_waters_regen_SpellScript::CorrectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_cleansing_waters_regen_SpellScript::CorrectTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_cleansing_waters_regen_SpellScript::CorrectTargets, EFFECT_3, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_cleansing_waters_regen_SpellScript();
    }
};
class OrientationCheck
{
public:
    explicit OrientationCheck(WorldObject* _caster) : caster(_caster) { }
    bool operator() (WorldObject* unit)
    {
        if (caster->isInFront(unit))
            return false;

        return true;
    }

private:
    WorldObject* caster;
};
// Dread Spray - 119958
class spell_dread_spray_damage_ori : public SpellScriptLoader
{
public:
    spell_dread_spray_damage_ori() : SpellScriptLoader("spell_dread_spray_damage_ori") { }

    class spell_dread_shadow_spell_script : public SpellScript
    {
        PrepareSpellScript(spell_dread_shadow_spell_script);

        void CorrectTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            targets.remove_if(ObjectNonAuraImmune());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dread_shadow_spell_script::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dread_shadow_spell_script::CorrectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_dread_shadow_spell_script();
    }
};

class ChengSpawn : public BasicEvent
{
public:
    explicit ChengSpawn(Unit* caster) : BasicEvent(), unitTarget(caster)
    {
    }

    bool Execute(uint64 /*currTime*/, uint32 /*diff*/)
    {
        if (unitTarget->GetAI())
        {
            unitTarget->GetAI()->DoAction(ACTION_SPAWN_CHENG);
        }
        return true;
    }

private:
    Unit* unitTarget;

};

class YangSpawn : public BasicEvent
{
public:
    explicit YangSpawn(Unit* caster) : BasicEvent(), unitTarget(caster)
    {
    }

    bool Execute(uint64 /*currTime*/, uint32 /*diff*/)
    {
        if (unitTarget->GetAI())
        {
            unitTarget->GetAI()->DoAction(ACTION_SPAWN_YANG);
        }
        return true;
    }

private:
    Unit* unitTarget;

};

class JuluSpawn : public BasicEvent
{
public:
    explicit JuluSpawn(Unit* caster) : BasicEvent(), unitTarget(caster)
    {
    }

    bool Execute(uint64 /*currTime*/, uint32 /*diff*/)
    {
        if (unitTarget->GetAI())
        {
            unitTarget->GetAI()->DoAction(ACTION_SPAWN_JULU);
        }
        return true;
    }

private:
    Unit* unitTarget;

};
class spell_ominous_cackle : public SpellScriptLoader
{
public:
    spell_ominous_cackle() : SpellScriptLoader("spell_ominous_cackle") { }

    class spell_ominous_cackle_spell_script : public SpellScript
    {
        PrepareSpellScript(spell_ominous_cackle_spell_script);

        void CorrectTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            if (count >= 6)
                return;

            if (!GetCaster())
                return;

            // handle targets reset 
            targets.clear();
            targets.empty();

            Map::PlayerList const &PlayerList = GetCaster()->GetMap()->GetPlayers();
            if (PlayerList.isEmpty())
                return;

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                Player* player = i->getSource();

                if (player && player->isAlive())
                {
                    // dps
                    switch (player->getClass())
                    {
                    case CLASS_WARLOCK:
                    case CLASS_ROGUE:
                    case CLASS_HUNTER:
                    case CLASS_DEATH_KNIGHT:
                    case CLASS_WARRIOR:
                    case CLASS_MAGE:
                    case CLASS_PALADIN:
                    case CLASS_SHAMAN:
                    case CLASS_DRUID:
                    case CLASS_MONK:
                    case CLASS_PRIEST:
                        if (countdps <= 4)
                        {
                            count++;
                            countdps++;
                            targets.push_back(player);
                        }
                        break;
                    }
                    // healers
                    if (counthealer < 1)
                        if (player->GetSpecializationId() == SPEC_PALADIN_HOLY || player->GetSpecializationId() == SPEC_PRIEST_HOLY || player->GetSpecializationId() == SPEC_PRIEST_HOLY || player->GetSpecializationId() == SPEC_DRUID_RESTORATION || player->GetSpecializationId() == SPEC_SHAMAN_RESTORATION || player->GetSpecializationId() == SPEC_MONK_MISTWEAVER)
                            targets.push_back(player);
                    // tanks
                    if (player->GetSpecializationId() == SPEC_MONK_BREWMASTER || player->GetSpecializationId() == SPEC_DK_BLOOD || player->GetSpecializationId() == SPEC_WARRIOR_PROTECTION || player->GetSpecializationId() == SPEC_DRUID_BEAR || player->GetSpecializationId() == SPEC_PALADIN_PROTECTION)
                        if (counttank < 1)
                            targets.push_back(player);
                }
            }
        }
        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            if (GetHitUnit())
            {
                if (GetCaster())
                {
                    if (GetCaster()->GetAI())
                    {
                        if (!GetHitUnit()->FindNearestCreature(61042, 1000.0f, true))
                        {
                            GetHitUnit()->NearTeleportTo(platform1.GetPositionX(), platform1.GetPositionY(), platform1.GetPositionZ(), platform1.GetOrientation());
                            GetCaster()->m_Events.AddEvent(new ChengSpawn(GetCaster()), GetCaster()->m_Events.CalculateTime(5000));
                        }
                        else if (!GetHitUnit()->FindNearestCreature(61038, 1000.0f, true))
                        {
                            GetHitUnit()->NearTeleportTo(platform2.GetPositionX(), platform2.GetPositionY(), platform2.GetPositionZ(), platform2.GetOrientation());
                            GetCaster()->m_Events.AddEvent(new YangSpawn(GetCaster()), GetCaster()->m_Events.CalculateTime(5000));
                        }
                        else if (!GetHitUnit()->FindNearestCreature(61046, 1000.0f, true))
                        {
                            GetHitUnit()->NearTeleportTo(platform3.GetPositionX(), platform3.GetPositionY(), platform3.GetPositionZ(), platform3.GetOrientation());
                            GetCaster()->m_Events.AddEvent(new JuluSpawn(GetCaster()), GetCaster()->m_Events.CalculateTime(5000));
                        }
                    }
                }
            }
        }
        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_ominous_cackle_spell_script::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ominous_cackle_spell_script::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ominous_cackle_spell_script::CorrectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }

    private:
        int count;
        int countdps;
        int counthealer;
        int counttank;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ominous_cackle_spell_script();
    }
};

void AddSC_boss_sha_of_fear()
{
    // boss
    new boss_sha_of_fear();
    // mobs
    new mob_guardian();
    new mob_terror_spawn();
    //triggers
    new mob_pure_light_terrace();
    new mob_return_to_the_terrace();
    new mob_globue();
    new trigger_PureLight_Visual();

    // spells 
    new spell_dread_spray_hit();
    new dread_spray_buff();
    new spell_champion_of_light();
    new spell_conjure_terror_spawn();
    new spell_penetrating_bolt();
    new spell_death_blossom();
    new spell_death_blossom_damage();
    new trigger_PureLight();
    new spell_breath_of_fear_spell();
    new spell_dread_spray_damage_ori();
    new spell_ominous_cackle();
}