/*
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

#include "CreatureGroups.h"
#include "CreatureTextMgr.h"
#include "ScriptedEscortAI.h"
#include "siege_of_orgrimmar.h"

enum ScriptedTexts
{
    //Boss
    SAY_GALAKRAS_INTRO       = 0,
    SAY_GALAKRAS_1           = 1,
    SAY_GALAKRAS_2           = 2,
    SAY_GALAKRAS_3           = 3, //summon Korkron Demolisher
    SAY_GALAKRAS_4           = 4,
    SAY_GALAKRAS_5           = 5,
    SAY_GALAKRAS_6           = 6, //Start Second phase
    SAY_GALAKRAS_7           = 7,
    SAY_GALAKRAS_8           = 8,
    
    //Jaina or Sylvanas
    SAY_BATTLE_INTO          = 0,
    SAY_BATTLE_1             = 1,
    SAY_BATTLE_2             = 2,
    SAY_BATTLE_3             = 3,
    
    //Trash
    SAY_DEMOLITION_ASSISTANT = 0,

    SAY_KRUGRUK_0            = 0,
    SAY_KRUGRUK_1            = 1,
    SAY_KRUGRUK_2            = 2,
    
    SAY_DAGRYN_0             = 0,
    SAY_DAGRYN_1             = 1,
    SAY_DAGRYN_2             = 2,
};

enum eSpells
{
    //Galakras
    SPELL_PULSING_FLAMES                 = 147042,
    SPELL_PULSING_FLAMES_AURA            = 147043,
    SPELL_FLAMES_OF_GALAKROND            = 147068,

    //FriendlyForces
    SPELL_ENABLE_UNIT_FRAME              = 147233,
    SPELL_JAINA_FROSTBOLT                = 146781,
    SPELL_JAINA_BLIZZARD                 = 146782,
    SPELL_JAINA_SUMMON_WATER_ELEMENTAL   = 146783,
    SPELL_WINDRUNNER_SHOOT               = 146768,
    SPELL_WINDRUNNER_MULTISHOT           = 146772,
    SPELL_SYLVANA_SUMMON_SKELETON        = 146770,
    SPELL_AETHAS_FIREBALL                = 146786,
    SPELL_AETHAS_FLAMESTRIKE             = 148849,
    SPELL_VISUAL_TELEPORT                = 149498,

    //Tower events
    SPELL_GOT_THE_BOMB_VISUAL            = 147966,
    SPELL_MOST_COMPLICATED_BOMB_SOUTH    = 147897,
    SPELL_MOST_COMPLICATED_BOMB_NORTH    = 147916,
    SPELL_IN_A_TOWER                     = 147317,
    SPELL_ANTIAIR_CANNON                 = 147514,
    SPELL_EFFECTIVE_TEAM_E               = 148248,
    SPELL_EFFECTIVE_TEAM_A               = 148249,
    SPELL_BANTER_E                       = 148041,
    SPELL_BANTER_A                       = 148042,
    SPELL_BOMBARD                        = 148301,
    SPELL_TOWER_JUMP_1                   = 148845,
    SPELL_TOWER_JUMP_2                   = 148878,
    SPELL_TOWER_JUMP_3                   = 148879,
    SPELL_TOWER_JUMP_4                   = 148880,

    //Dragonmaw Flameslinger
    SPELL_FLAME_ARROWS_EVENT             = 146763,
    SPELL_FLAME_ARROWS_COMBAT            = 147552,

    //Dragonmaw Proto-Drake
    SPELL_FLAME_BREATH                   = 146776,
    SPELL_DRAKEFIRE                      = 148352,

    //Dragonmaw Tidal Shaman
    SPELL_CHAIN_HEAL                     = 146757,
    SPELL_TIDAL_WAVE                     = 148522,
    SPELL_HEALING_TIDE_TOTEM             = 146753,

    //Dragonmaw War Banner
    SPELL_WAR_BANNER                     = 147176,
    SPELL_THROW_AXE                      = 147669,

    //Dragonmaw Bonecrusher
    SPELL_SHATTERING_ROAR                = 147204,
    SPELL_FRACTURE                       = 146744,
    SPELL_FRACTURE_PEREODIC              = 146899,
    SPELL_PING_BOSS                      = 144037,

    //High Enforcer Thranok
    SPELL_CRUSHER_CALL                   = 146769,
    SPELL_SHATTERING_CLEAVE              = 146849,
    SPELL_SKULL_CRACKER                  = 146848,

    //Korgra the Snake
    SPELL_POISONTIPPED_BLADES            = 146902,
    SPELL_POISON_CLOUD                   = 147706,
    SPELL_CURSE_OF_VENOM                 = 147711,
    SPELL_VENOM_BOLT_VOLLEY              = 147713,

    //Master Cannoneer Dagryn
    SPELL_DAGRYN_SHOOT                   = 146773,
    SPELL_MUZZLE_SPRAY                   = 147825,

    //Lieutenant Krugruk
    SPELL_ARCING_SMASH                   = 147688,
    SPELL_THUNDER_CLAP                   = 147683,

    //Dragonmaw Ebon Stalker
    SPELL_SHADOW_STALK                   = 146864,
    SPELL_SHADOW_ASSAULT                 = 146868,

    //Dragonmaw Grunt
    SPELL_DRAGON_CLEAVE                  = 148025,
    SPELL_FIXATE                         = 148243,
    SPELL_KNOCKED_OVER                   = 148030,
};

enum sEvents
{
    //battle
    EVENT_GALAKRAS_INTRO               = 1,
    EVENT_SUMMON_DEMOLITIONS_SOUTH     = 2,
    EVENT_SUMMON_DEMOLITIONS_NORTH     = 3,
    EVENT_SUMMON_ADDS_1                = 4,
    EVENT_SUMMON_ADDS_2                = 5,
    EVENT_SUMMON_ADDS_3                = 6,
    EVENT_SUMMON_ADDS_4                = 7,
    EVENT_SUMMON_ADDS_5                = 8,
    EVENT_SUMMON_ADDS_6                = 9,
    EVENT_SUMMON_ADDS_7                = 10,
    EVENT_SUMMON_ADDS_8                = 11,
    EVENT_SUMMON_ADDS_9                = 12,
    EVENT_SUMMON_ADDS_10               = 13,
    EVENT_CHECK_PLAYERS                = 14,
    //Heroic summon
    EVENT_SUMMON_GRUNT_SOUTH           = 15,
    EVENT_SUMMON_GRUNT_NORTH           = 16,
    //second phase
    EVENT_GALAKRAS_EXECUTE_1           = 17,
    EVENT_GALAKRAS_EXECUTE_2           = 18,
    EVENT_GALAKRAS_EXECUTE_3           = 19,
    EVENT_GALAKRAS_EXECUTE_4           = 20,
    //Finish
    EVENT_FINISH                       = 21,

    //Trash
    EVENT_FLAMESLINGER_ATTACK          = 1,
    EVENT_SUMMON_DEMOLISHER            = 2,
    EVENT_PREPARE_ATTACK_TOWER         = 3,
    EVENT_ATTACK_TOWER                 = 4,
    EVENT_PROTO_DRAKE_GROUND           = 5,
    EVENT_PROTO_DRAKE_FLY              = 6,
    EVENT_DRAKE_FLAME_BREATH           = 7,
    EVENT_DRAKE_DRAKEFIRE              = 8,
    EVENT_TIDAL_WAVE                   = 9,
    EVENT_CHAIN_HEAL                   = 10,
    EVENT_TIDE_TOTEM                   = 11,
    EVENT_WAR_BANNER                   = 12,
    EVENT_THROW_AXE                    = 13,
    EVENT_SHATTERING_ROAR              = 14,
    EVENT_FRACTURE                     = 15,
    EVENT_CRUSHER_CALL                 = 16,
    EVENT_SHATTERING_CLEAVE            = 17,
    EVENT_SKULL_CRACKER                = 18,
    EVENT_POISONTIPPED_BLADES          = 19,
    EVENT_CURSE_OF_VENOM               = 20,
    EVENT_VENOM_BOLT_VOLLEY            = 21,
    EVENT_DAGRYN_SHOOT                 = 22,
    EVENT_MUZZLE_SPRAY_START           = 23,
    EVENT_ARCING_SMASH_START           = 24,
    EVENT_THUNDER_CLAP                 = 25,
    EVENT_SHADOW_STALK                 = 26,
    EVENT_SHADOW_ASSAULT               = 27,
    EVENT_JAINAORSYLVANA_SAY           = 28,
    EVENT_JAINAORSYLVANA_COMBAT_1      = 29,
    EVENT_JAINAORSYLVANA_COMBAT_2      = 30,
    EVENT_JAINAORSYLVANA_COMBAT_3      = 31,
    EVENT_VEREESORAETHAS_COMBAT_1      = 32,
    EVENT_VEREESORAETHAS_COMBAT_2      = 33,
    EVENT_REPEAT_DEMO_CAST             = 34,
    EVENT_SUMMON_FLY_PROTODRAKE_1      = 35,
    EVENT_SUMMON_FLY_PROTODRAKE_2      = 36,
    EVENT_SUMMON_FLY_PROTODRAKE_3      = 37,
    EVENT_SUMMON_FLY_PROTODRAKE_4      = 38,
    EVENT_BONECRUSHER_FRACTURE         = 39,
    EVENT_DRAGON_CLEAVE                = 40,
    EVENT_FIXATE                       = 41,
    EVENT_BANTER_GRUNT                 = 42,
    EVENT_KNOCKED_OVER                 = 43,
    EVENT_TOWER_KNOCK_BACK             = 44,
    EVENT_ARCING_SMASH_STOP            = 45,
    EVENT_MUZZLE_SPRAY_STOP            = 46,
    EVENT_POISON_CLOUD                 = 47,
};

Position const CannonPos[7]
{
    {1281.93f, -5011.49f, 1.13f, 5.68f},
    {1331.97f, -5000.61f, 1.10f, 5.01f},
    {1380.06f, -4961.46f, 0.84f, 4.92f},
    {1265.59f, -5023.52f, 0.99f, 5.29f},
    {1399.55f, -4931.88f, 11.28f, 4.58f},
    {1249.06f, -5039.08f, 1.91f, 5.04f},
    {1351.58f, -4989.40f, 0.59f, 5.33f},
};

Position const MinePos[21]
{
    {1405.49f, -4928.59f, 11.34f},
    {1405.82f, -4927.37f, 11.34f},
    {1405.01f, -4927.66f, 12.52f},
    {1404.16f, -4927.52f, 11.34f},
    {1375.21f, -4956.54f, 1.73f},
    {1374.42f, -4955.58f, 1.85f},
    {1374.51f, -4956.06f, 1.82f},
    {1374.03f, -4957.01f, 1.84f},
    {1355.39f, -4984.73f, 1.04f},
    {1355.76f, -4986.14f, 0.81f},
    {1353.94f, -4985.19f, 1.05f},
    {1327.44f, -4995.72f, 1.60f},
    {1326.37f, -4996.81f, 1.45f},
    {1280.89f, -5005.36f, 1.65f},
    {1279.71f, -5005.82f, 1.68f},
    {1280.18f, -5004.88f, 3.27f},
    {1280.1f,  -5004.41f, 1.75f},
    {1264.96f, -5017.68f, 1.68f},
    {1263.11f, -5017.77f, 1.77f},
    {1243.05f, -5036.72f, 2.67f},
    {1243.06f, -5034.79f, 2.80f},
};

Position const FriendlyForcesSpawn[15] =
{
    {1422.56f, -4900.93f, 11.3574f, 1.75f}, // Lady Sylvanas
    {1428.01f, -4900.04f, 11.3556f, 1.73f}, // Lor'themar Theron
    {1433.27f, -4898.97f, 11.2763f, 1.73f}, // Archmage Aethas Sunreaver
    {1415.63f, -4896.83f, 11.3191f, 1.77f}, // Sunreaver Magus
    {1424.02f, -4896.06f, 11.3072f, 1.81f},
    {1431.77f, -4894.84f, 11.1726f, 1.83f},
    {1439.64f, -4894.59f, 10.9108f, 2.01f},
    {1419.31f, -4883.37f, 11.188f,  1.67f}, // Sunreaver Sentinel
    {1426.4f,  -4882.78f, 11.1642f, 1.67f},
    {1432.51f, -4881.61f, 10.8313f, 1.59f},
    {1419.14f, -4889.91f, 11.2474f, 1.53f},
    {1426.53f, -4889.18f, 11.1932f, 1.64f},
    {1434.45f, -4887.59f, 10.7653f, 1.74f},
    {1412.26f, -4884.92f, 11.1638f, 1.27f}, // Sunreaver Construct
    {1441.75f, -4878.89f, 11.4579f, 2.60f},
};

const Position TowerSpawn[16] =
{
    {1454.31f, -4803.17f, 68.46f, 0.47f},
    {1363.16f, -4840.76f, 71.83f, 1.95f},
    {1458.17f, -4795.59f, 40.37f, 5.20f},
    {1448.68f, -4814.89f, 68.30f, 4.01f},
    {1451.19f, -4816.38f, 68.46f, 4.31f},
    {1470.42f, -4801.08f, 34.15f, 3.62f},
    {1464.85f, -4821.24f, 68.38f, 4.44f},
    {1363.43f, -4830.01f, 37.63f, 4.50f},
    {1366.41f, -4854.79f, 71.66f, 4.91f},
    {1363.99f, -4856.54f, 71.49f, 5.28f},
    {1378.74f, -4846.44f, 71.70f, 5.20f},
    {1352.89f, -4838.47f, 43.78f, 6.21f},
    {1362.47f, -4836.86f, 33.08f, 5.37f},
    {1463.31f, -4804.27f, 29.67f, 4.32f},
    {1371.95f, -4850.06f, 72.77f, 5.41f},
    {1456.77f, -4817.71f, 68.74f, 4.24f},
};

const Position AddsSpawn[1] =
{
    {1407.56f, -4813.16f, 21.48f, 4.93f}, // Galakras Adds Spawn
};

const Position AttackTowers[2] =
{
    {1369.48f, -4846.21f, 32.70f}, // demolisher Attack Target South
    {1458.72f, -4814.36f, 29.19f}, // demolisher Attack Target North
};

//Heroic spawn grunt
const Position HeroicGruntPos[2]
{
    {1373.57f, -4861.87f, 31.3380f, 1.9148f},//South
    {1454.72f, -4825.64f, 29.0519f, 1.2080f},//Norh
};

const Position ProtoDrake[10] =
{
    {1409.60f, -4862.25f, 80.4107f, 3.03f},
    {1428.28f, -4858.31f, 80.4107f, 1.93f},
    {1459.56f, -4846.65f, 80.4107f, 2.24f},
    {1482.62f, -4844.12f, 80.4107f, 1.89f},
    {1411.65f, -4875.41f, 80.4107f, 1.71f},
    {1447.68f, -4881.48f, 80.4107f, 1.82f},
    {1408.65f, -4895.28f, 80.4107f, 1.85f},
    {1376.97f, -4892.51f, 80.4107f, 1.86f},
    {1472.60f, -4860.60f, 80.4107f, 1.59f},
    {1430.69f, -4876.23f, 80.4107f, 2.37f},
};

const Position RinOrlortPos[2] =
{
    {1424.51f, -4879.86f, 11.23f, 1.8f},
    {1427.13f, -4894.16f, 11.25f, 4.9f},
};

const Position ExecutePos[1] =
{
    {1413.94f, -4824.83f, 18.93f}
};

const Position DemolitionSPos[2] =
{
    {1414.17f, -4900.76f, 11.33f, 3.21f}, // Expert South
    {1416.56f, -4900.58f, 11.33f, 3.21f}, // Assistant South
};

const Position DemolitionNPos[2] =
{
    {1433.97f, -4904.58f, 11.39f, 1.05f}, // Expert North
    {1431.86f, -4907.19f, 11.47f, 1.04f}, // Assistant North
};

uint32 demolicionsexpertlist[2] =
{
    NPC_DEMOLITIONS_EXPERT_S_A,
    NPC_DEMOLITIONS_EXPERT_S_H,
};

class boss_galakras : public CreatureScript
{
    public:
        boss_galakras() : CreatureScript("boss_galakras") {}

        struct boss_galakrasAI : public BossAI
        {
            boss_galakrasAI(Creature* creature) : BossAI(creature, DATA_GALAKRAS), summons(me)
            {
                instance = creature->GetInstanceScript();
                SouthGruntEvent = false;
                NorthGruntEvent = false;
                PreEvent        = true;
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            SummonList summons;
            bool SouthGruntEvent;
            bool NorthGruntEvent;
            bool PreEvent;
            bool phaseTwo;

            void Reset()
            {
                events.Reset();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                me->RemoveAurasDueToSpell(SPELL_PULSING_FLAMES_AURA);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ENABLE_UNIT_FRAME);
                EntryCheckPredicate pred1(NPC_TOWER_SOUTH);
                EntryCheckPredicate pred2(NPC_TOWER_NORTH);
                EntryCheckPredicate pred3(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_DEMOLITIONS_EXPERT_S_H : NPC_DEMOLITIONS_EXPERT_S_A);
                EntryCheckPredicate pred4(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_DEMOLITIONS_EXPERT_N_H : NPC_DEMOLITIONS_EXPERT_N_A);
                summons.DoAction(ACTION_TOWER_DESPAWN, pred1);
                summons.DoAction(ACTION_TOWER_DESPAWN, pred2);
                summons.DoAction(ACTION_TOWER_DESPAWN, pred3);
                summons.DoAction(ACTION_TOWER_DESPAWN, pred4);
                summons.DespawnAll();
                phaseTwo = false;

                if (instance->GetBossState(DATA_SHA_OF_PRIDE) == DONE)
                {
                    if (!PreEvent)
                        Summons();
                    else
                        SummonCannon();
                }
            }

            void Summons()
            {
                me->SummonCreature(NPC_TOWER_SOUTH, 1361.86f, -4838.46f, 33.07f, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 0);
                me->SummonCreature(NPC_TOWER_NORTH, 1462.55f, -4802.89f, 29.67f, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 0);
                me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_LADY_SYLVANAS_WINDRUNNER_H : NPC_LADY_JAINA_PROUDMOORE_A, FriendlyForcesSpawn[0]);
                me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_LORTHEMAR_THERON_H : NPC_KING_VARIAN_WRYNN_A, FriendlyForcesSpawn[1]);
                me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_ARCHMAGE_AETHAS_SUNREAVER_H : NPC_VEREESA_WINDRUNNER_A, FriendlyForcesSpawn[2]);
                for (uint8 n = 3; n < 7; n++)
                    me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_SUNREAVER_MAGUS_H : NPC_ALLIANCE_VANGUARD_DWARF_A, FriendlyForcesSpawn[n]);
                for (uint8 n = 7; n < 10; n++)
                    me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_SUNREAVER_SENTINEL_H : NPC_ALLIANCE_VANGUARD_GNOM_A, FriendlyForcesSpawn[n]);
                for (uint8 n = 10; n < 13; n++)
                    me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_SUNREAVER_SENTINEL_H : NPC_ALLIANCE_VANGUARD_HUMAN_A, FriendlyForcesSpawn[n]);
                me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_SUNREAVER_CONSTRUCT_H : NPC_HIGHGUARD_KAILUS_A, FriendlyForcesSpawn[13]);
                me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_SUNREAVER_CONSTRUCT_H : NPC_HIGHGUARD_TYRIUS_A, FriendlyForcesSpawn[14]);
            }
            
            void SummonCannon()
            {
                if (instance->GetData(DATA_GALAKRAS_PRE_EVENT) != IN_PROGRESS)
                    instance->SetData(DATA_GALAKRAS_PRE_EVENT, IN_PROGRESS);

                for (uint8 n = 0; n < 7; n++)
                    me->SummonCreature(NPC_KORKRON_CANNON, CannonPos[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                
                for (uint8 n = 0; n < 21; n++)
                    me->SummonCreature(NPC_SPIKE_MINE, MinePos[n]);
            }
            
            void EnterCombat(Unit* who)
            {
                _EnterCombat();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_PRE_EVENT:
                        SummonCannon();
                        break;
                    case ACTION_PRE_EVENT_FINISH:
                        PreEvent = false;
                        Summons();
                        if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                            Zaela->AI()->ZoneTalk(TEXT_GENERIC_9);
                        break;
                    case ACTION_GALAKRAS_START_EVENT:
                        events.RescheduleEvent(EVENT_GALAKRAS_INTRO, 1000);
                        events.RescheduleEvent(EVENT_CHECK_PLAYERS, 2000);
                        break;
                    case ACTION_DEMOLITIONS_SOUTH:
                        events.RescheduleEvent(EVENT_SUMMON_DEMOLITIONS_SOUTH, 40000);
                        break;
                    case ACTION_DEMOLITIONS_NORTH:
                        events.RescheduleEvent(EVENT_SUMMON_DEMOLITIONS_NORTH, 40000);
                        if (me->GetMap()->IsHeroic())
                        {
                            events.CancelEvent(EVENT_SUMMON_GRUNT_SOUTH);
                            events.CancelEvent(EVENT_SUMMON_GRUNT_NORTH);
                        }
                        break;
                    //Heroic mode
                    case ACTION_GRUNT_SOUTH:
                        if (me->GetMap()->IsHeroic())
                        {
                            SouthGruntEvent = true;
                            events.RescheduleEvent(EVENT_SUMMON_GRUNT_SOUTH, 6000);
                        }
                        break;
                    case ACTION_GRUNT_NORTH:
                        NorthGruntEvent = true;
                        break;
                    case ACTION_GRUNT_SOUTH_FINISH:
                        SouthGruntEvent = false;
                        break;
                    case ACTION_GRUNT_NORTH_FINISH:
                        NorthGruntEvent = false;
                        break;
                }
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ENABLE_UNIT_FRAME);
            }

            void SpellHit(Unit* pCaster, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_ANTIAIR_CANNON && !phaseTwo)
                {
                    phaseTwo = true;
                    events.Reset();
                    events.RescheduleEvent(EVENT_GALAKRAS_EXECUTE_1, 1000);
                    events.RescheduleEvent(EVENT_CHECK_PLAYERS, 2000);
                }
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHECK_PLAYERS:
                            if (!me->FindNearestPlayer(170.0f) || !me->isInCombat())
                            {
                                instance->SetBossState(DATA_GALAKRAS, NOT_STARTED);
                                EnterEvadeMode();
                            }
                            else
                                events.RescheduleEvent(EVENT_CHECK_PLAYERS, 2000);
                            break;
                        case EVENT_GALAKRAS_INTRO:
                        {
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_INTRO);
                            DoZoneInCombat();
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_1, 5000);
                            events.RescheduleEvent(EVENT_SUMMON_DEMOLITIONS_SOUTH, 0);
                            break;
                        }
                        case EVENT_SUMMON_GRUNT_SOUTH:
                            if (me->GetMap()->IsHeroic())
                            {
                                if (SouthGruntEvent)
                                    me->SummonCreature(NPC_DRAGONMAW_GRUNT_H, HeroicGruntPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                                events.RescheduleEvent(EVENT_SUMMON_GRUNT_SOUTH, 60000);
                            }
                            break;
                        case EVENT_SUMMON_GRUNT_NORTH:
                            if (me->GetMap()->IsHeroic())
                            {
                                if (NorthGruntEvent)
                                    me->SummonCreature(NPC_DRAGONMAW_GRUNT_H, HeroicGruntPos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                                events.RescheduleEvent(EVENT_SUMMON_GRUNT_NORTH, 60000);
                            }
                            break;
                        case EVENT_SUMMON_DEMOLITIONS_SOUTH:
                            me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_DEMOLITIONS_EXPERT_S_H: NPC_DEMOLITIONS_EXPERT_S_A, DemolitionSPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            break;
                        case EVENT_SUMMON_DEMOLITIONS_NORTH:
                            me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_DEMOLITIONS_EXPERT_N_H: NPC_DEMOLITIONS_EXPERT_N_A, DemolitionNPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            events.RescheduleEvent(EVENT_SUMMON_GRUNT_NORTH, 6000);
                            break;
                        case EVENT_SUMMON_ADDS_1:
                            me->SummonCreature(NPC_DRAGONMAW_BONECRUSHER, AddsSpawn[0].GetPositionX() + 4.0f, AddsSpawn[0].GetPositionY() + 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_BONECRUSHER, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAGBEARER, AddsSpawn[0].GetPositionX() - 4.0f, AddsSpawn[0].GetPositionY() - 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_2, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_2:
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 4.0f, AddsSpawn[0].GetPositionY() + 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAMESLINGER, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAMESLINGER, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_TIDAL_SHAMAN, AddsSpawn[0].GetPositionX() - 4.0f, AddsSpawn[0].GetPositionY() - 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_1);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_3, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_3:
                            me->SummonCreature(NPC_DRAGONMAW_EBON_STALKER, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_KORGRA_THE_SNAKE, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_EBON_STALKER, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_2);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_4, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_4:
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, AddsSpawn[0].GetPositionX() - 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_5, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_5:
                            me->SummonCreature(NPC_DRAGONMAW_FLAGBEARER, AddsSpawn[0].GetPositionX() + 4.0f, AddsSpawn[0].GetPositionY() + 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_TIDAL_SHAMAN, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_HIGH_ENFORCER_THRANOK, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_1);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_6, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_6:
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_BONECRUSHER, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAGBEARER, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_4);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_7, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_7:
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 4.0f, AddsSpawn[0].GetPositionY() + 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAGBEARER, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_TIDAL_SHAMAN, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAMESLINGER, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAMESLINGER, AddsSpawn[0].GetPositionX() - 4.0f, AddsSpawn[0].GetPositionY() - 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_1);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_8, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_8:
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 4.0f, AddsSpawn[0].GetPositionY() + 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_FLAGBEARER, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_TIDAL_SHAMAN, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_BONECRUSHER, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_9, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_9:
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, AddsSpawn[0].GetPositionX() - 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_10, 55000);
                            break;
                        case EVENT_SUMMON_ADDS_10:
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 4.0f, AddsSpawn[0].GetPositionY() + 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() + 2.0f, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_TIDAL_SHAMAN, AddsSpawn[0].GetPositionX(), AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_TIDAL_SHAMAN, AddsSpawn[0].GetPositionX() - 2.0, AddsSpawn[0].GetPositionY(), AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            me->SummonCreature(NPC_DRAGONMAW_GRUNT, AddsSpawn[0].GetPositionX() - 4.0f, AddsSpawn[0].GetPositionY() - 1.0f, AddsSpawn[0].GetPositionZ() + 1.0f, AddsSpawn[0].GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            events.RescheduleEvent(EVENT_SUMMON_ADDS_6, 55000);
                            break;
                        case EVENT_GALAKRAS_EXECUTE_1:
                            me->GetMotionMaster()->MovePoint(1, ExecutePos[0]);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                            {
                                Zaela->AI()->Talk(SAY_GALAKRAS_6);
                                Zaela->AI()->Talk(SAY_GALAKRAS_7);
                                Zaela->AI()->Talk(SAY_GALAKRAS_8);
                                me->Kill(Zaela);
                            }
                            events.RescheduleEvent(EVENT_GALAKRAS_EXECUTE_2, 4000);
                            break;
                        case EVENT_GALAKRAS_EXECUTE_2:
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                            me->RemoveAurasDueToSpell(SPELL_ANTIAIR_CANNON);
                            events.RescheduleEvent(EVENT_GALAKRAS_EXECUTE_3, 0);
                            events.RescheduleEvent(EVENT_GALAKRAS_EXECUTE_4, 20000);
                            break;
                        case EVENT_GALAKRAS_EXECUTE_3:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(pTarget, SPELL_FLAMES_OF_GALAKROND);
                            events.RescheduleEvent(EVENT_GALAKRAS_EXECUTE_3, 6000);
                            break;
                        case EVENT_GALAKRAS_EXECUTE_4:
                            DoCast(SPELL_PULSING_FLAMES);
                            events.RescheduleEvent(EVENT_GALAKRAS_EXECUTE_4, 25000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_galakrasAI(creature);
        }
};

class npc_varian_or_lorthemar : public CreatureScript
{
    public:
        npc_varian_or_lorthemar() : CreatureScript("npc_varian_or_lorthemar") {}

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        { 
            player->CLOSE_GOSSIP_MENU();
            if (action)
                creature->AI()->DoAction(ACTION_VARIAN_OR_LORTHEMAR_EVENT);

            return true;
        }

        struct npc_varian_or_lorthemarAI : public ScriptedAI
        {
            npc_varian_or_lorthemarAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                group_member = sFormationMgr->CreateCustomFormation(me);
                startEvent = false;
            }

            InstanceScript* instance;
            EventMap events;
            FormationInfo* group_member;
            bool startEvent;

            void Reset()
            {
            }

            void EnterCombat(Unit* who)
            {
            }
            
            void DoAction(int32 const action)
            {
                if (!instance)
                    return;

                if (me->GetDistance(me->GetHomePosition()) > 20.0f)
                {
                    EnterEvadeMode();
                    return;
                }
                
                switch (action)
                {
                    case ACTION_VARIAN_OR_LORTHEMAR_EVENT:
                    {
                        if (!startEvent)
                        {
                            startEvent = true;
                            me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, 0);
                            me->GetMotionMaster()->MovePoint(1, RinOrlortPos[0]);
                            me->SetHomePosition(RinOrlortPos[0]);
    
                            if (Creature* jainasylvana = instance->instance->GetCreature(instance->GetGuidData(DATA_JAINA_OR_SYLVANA)))
                                if (CreatureGroup* f = me->GetFormation())
                                    f->AddMember(jainasylvana, group_member);
                            if (Creature* VereesaAethas = instance->instance->GetCreature(instance->GetGuidData(DATA_VEREESA_OR_AETHAS)))
                                if (CreatureGroup* f = me->GetFormation())
                                    f->AddMember(VereesaAethas, group_member);
                            if (Creature* pGalakras = instance->instance->GetCreature(instance->GetGuidData(NPC_GALAKRAS)))
                                pGalakras->AI()->DoAction(ACTION_GALAKRAS_START_EVENT);
    
                            instance->SetBossState(DATA_GALAKRAS, IN_PROGRESS);
                            instance->SetData(DATA_SOUTH_TOWER, IN_PROGRESS);
                            DoCast(SPELL_ENABLE_UNIT_FRAME);
                            break;
                        }
                    }
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                instance->SetBossState(DATA_GALAKRAS, NOT_STARTED);
            }

            void SpellHit(Unit* pCaster, SpellInfo const* spell)
            {
                if(spell->Id == SPELL_FRACTURE)
                    DoCast(pCaster, SPELL_PING_BOSS, true);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_varian_or_lorthemarAI(creature);
        }
};

class npc_jaina_or_sylvana : public CreatureScript
{
    public:
        npc_jaina_or_sylvana() : CreatureScript("npc_jaina_or_sylvana") {}

        struct npc_jaina_or_sylvanaAI : public ScriptedAI
        {
            npc_jaina_or_sylvanaAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                instance->SetBossState(DATA_GALAKRAS, NOT_STARTED);
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_JAINAORSYLVANA_COMBAT_1, 1000);
                events.RescheduleEvent(EVENT_JAINAORSYLVANA_COMBAT_2, 5000);
                events.RescheduleEvent(EVENT_JAINAORSYLVANA_COMBAT_3, 15000);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_FRIENDLY_BOSS)
                {
                    DoCast(SPELL_ENABLE_UNIT_FRAME);
                    events.RescheduleEvent(EVENT_JAINAORSYLVANA_SAY, 10000);
                }
            }

            void SpellHit(Unit* pCaster, SpellInfo const* spell)
            {
                if(spell->Id == SPELL_FRACTURE)
                    DoCast(pCaster, SPELL_PING_BOSS, true);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();
                
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_JAINAORSYLVANA_SAY:
                            Talk(SAY_BATTLE_INTO);
                            break;
                        case EVENT_JAINAORSYLVANA_COMBAT_1:
                        {
                            if (Unit* Target = me->getVictim())
                            {
                                if (me->GetEntry() == NPC_LADY_JAINA_PROUDMOORE_A)
                                    DoCast(Target, SPELL_JAINA_FROSTBOLT);
                                else
                                    DoCast(Target, SPELL_WINDRUNNER_SHOOT);
                            }
                        }
                        events.RescheduleEvent(EVENT_JAINAORSYLVANA_COMBAT_1, 2000);
                            break;
                        case EVENT_JAINAORSYLVANA_COMBAT_2:
                        {
                            if (me->GetEntry() == NPC_LADY_JAINA_PROUDMOORE_A)
                            {
                                me->InterruptSpell(CURRENT_GENERIC_SPELL);
                                if (Unit* Target = me->getVictim())
                                    DoCast(Target, SPELL_JAINA_BLIZZARD);
                                uint8 chance = urand(0, 50);
                                if (chance > 25)
                                    Talk(SAY_BATTLE_1);
                            }
                            events.RescheduleEvent(EVENT_JAINAORSYLVANA_COMBAT_2, 20000);
                            break;
                        }
                        case EVENT_JAINAORSYLVANA_COMBAT_3:
                        {
                            if (Unit* Target = me->getVictim())
                            {
                                if (me->GetEntry() == NPC_LADY_JAINA_PROUDMOORE_A)
                                    DoCast(Target, SPELL_JAINA_SUMMON_WATER_ELEMENTAL, true);
                                else
                                    DoCast(Target, SPELL_SYLVANA_SUMMON_SKELETON, true);
                            }
                            events.RescheduleEvent(EVENT_JAINAORSYLVANA_COMBAT_3, 30000);
                            break;
                        }
                    }
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_jaina_or_sylvanaAI(creature);
        }
};

class npc_verees_or_aethas : public CreatureScript
{
    public:
        npc_verees_or_aethas() : CreatureScript("npc_verees_or_aethas") {}

        struct npc_verees_or_aethasAI : public ScriptedAI
        {
            npc_verees_or_aethasAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                instance->SetBossState(DATA_GALAKRAS, NOT_STARTED);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_FRIENDLY_BOSS)
                    DoCast(SPELL_ENABLE_UNIT_FRAME);
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_VEREESORAETHAS_COMBAT_1, 1000);
                events.RescheduleEvent(EVENT_VEREESORAETHAS_COMBAT_2, 5000);
            }

            void SpellHit(Unit* pCaster, SpellInfo const* spell)
            {
                if(spell->Id == SPELL_FRACTURE)
                    DoCast(pCaster, SPELL_PING_BOSS);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VEREESORAETHAS_COMBAT_1:
                        {
                            if (Unit* Target = me->getVictim())
                            {
                                if (me->GetEntry() == NPC_VEREESA_WINDRUNNER_A)
                                    DoCast(Target, SPELL_WINDRUNNER_SHOOT);
                                else
                                    DoCast(Target, SPELL_AETHAS_FIREBALL);
                            }
                            events.RescheduleEvent(EVENT_VEREESORAETHAS_COMBAT_1, 2000);
                            break;
                        }
                        case EVENT_VEREESORAETHAS_COMBAT_2:
                        {
                            if (Unit* Target = me->getVictim())
                            {
                                if (me->GetEntry() == NPC_VEREESA_WINDRUNNER_A)
                                    DoCast(Target, SPELL_WINDRUNNER_MULTISHOT);
                                else
                                {
                                    me->InterruptSpell(CURRENT_GENERIC_SPELL);
                                    DoCast(Target, SPELL_AETHAS_FLAMESTRIKE);
                                    uint8 chance = urand(0, 50);
                                    if (chance > 25)
                                        Talk(SAY_BATTLE_1);
                                }
                            }
                            events.RescheduleEvent(EVENT_VEREESORAETHAS_COMBAT_2, 20000);
                            break;
                        }
                    }
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_verees_or_aethasAI(creature);
        }
};

class npc_demolitions : public CreatureScript
{
    public:
        npc_demolitions() : CreatureScript("npc_demolitions") {}

        struct npc_demolitionsAI : public npc_escortAI
        {
            npc_demolitionsAI(Creature* creature) : npc_escortAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                ResetCast = false;
                RespawnEvent = true;
            }
            InstanceScript* instance;
            EventMap events;
            SummonList summons;
            uint32 SouthExpert = (me->GetEntry() == NPC_DEMOLITIONS_EXPERT_S_H || me->GetEntry() == NPC_DEMOLITIONS_EXPERT_S_A);
            uint32 NorthExpert = (me->GetEntry() == NPC_DEMOLITIONS_EXPERT_N_H || me->GetEntry() == NPC_DEMOLITIONS_EXPERT_N_A);
            bool ResetCast;
            bool RespawnEvent;

            void Reset()
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (!instance)
                    return;

                events.RescheduleEvent(EVENT_BANTER_GRUNT, 1000);
                events.RescheduleEvent(EVENT_KNOCKED_OVER, 1000);
                
                if (SouthExpert)
                {
                    DoCast(SPELL_GOT_THE_BOMB_VISUAL);
                    me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_DEMOLITIONS_ASSISTANT_S_H: NPC_DEMOLITIONS_ASSISTANT_S_A, DemolitionSPos[1]);

                    AddWaypoint(0, 1378.72f, -4911.45f, 11.59f);
                    AddWaypoint(1, 1371.93f, -4891.18f, 19.41f);
                    AddWaypoint(2, 1373.72f, -4877.68f, 24.46f);
                    AddWaypoint(3, 1372.29f, -4869.11f, 28.89f);
                    AddWaypoint(4, 1370.43f, -4859.61f, 32.15f);
                    AddWaypoint(5, 1368.04f, -4849.56f, 32.70f);
                    Start(false, true);
                    SetDespawnAtEnd(false);
                }
                if (NorthExpert)
                {
                    DoCast(SPELL_GOT_THE_BOMB_VISUAL);
                    me->SummonCreature(instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE ? NPC_DEMOLITIONS_ASSISTANT_N_H: NPC_DEMOLITIONS_ASSISTANT_N_A, DemolitionNPos[1]);
                    AddWaypoint(0, 1441.53f, -4890.43f, 10.66f);
                    AddWaypoint(1, 1454.28f, -4879.03f, 17.54f);
                    AddWaypoint(2, 1468.29f, -4865.44f, 24.77f);
                    AddWaypoint(3, 1463.99f, -4848.21f, 27.20f);
                    AddWaypoint(4, 1458.62f, -4822.29f, 29.19f);
                    AddWaypoint(5, 1457.39f, -4814.51f, 29.19f);
                    Start(false, true);
                    SetDespawnAtEnd(false);
                }
            }

            void WaypointReached(uint32 id)
            {
                if (SouthExpert)
                    if (id == 5)
                    {
                        ResetCast = true;
                        events.RescheduleEvent(EVENT_REPEAT_DEMO_CAST, 1000);
                        DoCast(SPELL_MOST_COMPLICATED_BOMB_SOUTH);
                    }

                if (NorthExpert)
                    if (id == 5)
                    {
                        ResetCast = true;
                        events.RescheduleEvent(EVENT_REPEAT_DEMO_CAST, 1000);
                        DoCast(SPELL_MOST_COMPLICATED_BOMB_NORTH);
                    }
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);
                summoned->GetMotionMaster()->MoveFollow(me, 0.0f, 0.0f);
                if (SouthExpert)
                    summoned->AI()->Talk(SAY_DEMOLITION_ASSISTANT);
                
                summons.SetReactState(REACT_PASSIVE);
                me->CastSpell(summoned, SPELL_EFFECTIVE_TEAM_E);
                summoned->CastSpell(me, SPELL_EFFECTIVE_TEAM_A);
            }

            void EnterCombat(Unit* who)
            {
            }

            void JustDied(Unit* /*killer*/)
            {
                summons.DespawnAll();

                if (SouthExpert && RespawnEvent)
                    if (Creature* Galakras = instance->instance->GetCreature(instance->GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_DEMOLITIONS_SOUTH);
                    
                if (NorthExpert && RespawnEvent)
                    if (Creature* Galakras = instance->instance->GetCreature(instance->GetGuidData(NPC_GALAKRAS)))
                        Galakras->AI()->DoAction(ACTION_DEMOLITIONS_NORTH);
            }
            
            void DoAction(const int32 action)
            {
                if (action == ACTION_TOWER_DESPAWN)
                    summons.DespawnAll();
                
                if (action == ACTION_DEMOLITIONS_COMPLETE)
                {
                    ResetCast = false;
                    RespawnEvent = false;
                    events.Reset();
                    me->CastStop();
                }
            }
            
            void UpdateAI(uint32 diff)
            {
                UpdateVictim();
                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_REPEAT_DEMO_CAST:
                        {
                            if (SouthExpert)
                                if (!me->HasAura(SPELL_MOST_COMPLICATED_BOMB_SOUTH) && ResetCast)
                                    DoCast(SPELL_MOST_COMPLICATED_BOMB_SOUTH);

                            if (NorthExpert)
                                if (!me->HasAura(SPELL_MOST_COMPLICATED_BOMB_NORTH) && ResetCast)
                                    DoCast(SPELL_MOST_COMPLICATED_BOMB_NORTH);
                            events.RescheduleEvent(EVENT_REPEAT_DEMO_CAST, 1000);
                            break;
                        }
                        case EVENT_BANTER_GRUNT:
                            if (!me->HasAura(SPELL_KNOCKED_OVER))
                                DoCast(me, SPELL_BANTER_A, true);
                            events.RescheduleEvent(EVENT_BANTER_GRUNT, 1000);
                            break;
                        case EVENT_KNOCKED_OVER:
                            if (me->HasAura(SPELL_KNOCKED_OVER))
                                SetEscortPaused(true);
                            else
                                SetEscortPaused(false);
                            events.RescheduleEvent(EVENT_KNOCKED_OVER, 500);
                            break;
                    }
                }
                npc_escortAI::UpdateAI(diff);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_demolitionsAI(creature);
        }
};

class npc_galakras_south_tower : public CreatureScript
{
    public:
        npc_galakras_south_tower() : CreatureScript("npc_galakras_south_tower") {}

        struct npc_galakras_south_towerAI : public ScriptedAI
        {
            npc_galakras_south_towerAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
            EventMap events;
            SummonList summons;

            void Reset()
            {
                events.Reset();

                for (uint8 n = 7; n < 12; n++)
                {
                    me->SummonCreature(NPC_DRAGONMAW_FLAMESLINGER, TowerSpawn[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                }
                me->SummonCreature(NPC_LIEUTENANT_KRUGRUK, TowerSpawn[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                me->SummonCreature(NPC_DRAGONMAW_GRUNT, TowerSpawn[12], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                me->SummonCreature(NPC_ANTIAIR_TURRET, TowerSpawn[14], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);
                summoned->SetReactState(REACT_PASSIVE);
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_TOWER_GUARDS:
                    {
                        summons.SetReactState(REACT_AGGRESSIVE);
                        EntryCheckPredicate pred(NPC_DRAGONMAW_FLAMESLINGER);
                        summons.DoAction(ACTION_TOWER_GUARDS, pred);
                        ZoneTalk(TEXT_GENERIC_0);
                        events.RescheduleEvent(EVENT_SUMMON_DEMOLISHER, 20000);
                        events.RescheduleEvent(EVENT_SUMMON_FLY_PROTODRAKE_1, 1000);
                        break;
                    }
                    case ACTION_TOWER_TURRET:
                    {
                        EntryCheckPredicate pred(NPC_ANTIAIR_TURRET);
                        summons.DoAction(ACTION_TOWER_GUARDS, pred);
                        break;
                    }
                    case ACTION_TOWER_DESPAWN:
                        events.Reset();
                        summons.DespawnAll();
                        break;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                damage = 0;
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUMMON_DEMOLISHER:
                            me->SummonCreature(NPC_KORKRON_DEMOLISHER, AddsSpawn[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_3);
                            break;
                        case EVENT_SUMMON_FLY_PROTODRAKE_1:
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            events.RescheduleEvent(EVENT_SUMMON_FLY_PROTODRAKE_2, 5000);
                            break;
                        case EVENT_SUMMON_FLY_PROTODRAKE_2:
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            events.RescheduleEvent(EVENT_SUMMON_FLY_PROTODRAKE_3, 5000);
                            break;
                        case EVENT_SUMMON_FLY_PROTODRAKE_3:
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            events.RescheduleEvent(EVENT_SUMMON_FLY_PROTODRAKE_4, 5000);
                            break;
                        case EVENT_SUMMON_FLY_PROTODRAKE_4:
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            me->SummonCreature(NPC_DRAGONMAW_PROTO_DRAKE, ProtoDrake[urand(0,9)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_galakras_south_towerAI(creature);
        }
};

class npc_galakras_north_tower : public CreatureScript
{
    public:
        npc_galakras_north_tower() : CreatureScript("npc_galakras_north_tower") {}

        struct npc_galakras_north_towerAI : public ScriptedAI
        {
            npc_galakras_north_towerAI(Creature* creature) : ScriptedAI(creature), summons(me)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
            EventMap events;
            SummonList summons;

            void Reset()
            {
                events.Reset();

                for (uint8 n = 2; n < 7; n++)
                {
                    me->SummonCreature(NPC_DRAGONMAW_FLAMESLINGER, TowerSpawn[n], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                }
                me->SummonCreature(NPC_MASTER_CANNONEER_DAGRYN, TowerSpawn[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                me->SummonCreature(NPC_DRAGONMAW_GRUNT, TowerSpawn[13], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                me->SummonCreature(NPC_ANTIAIR_TURRET, TowerSpawn[15], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);
                summoned->SetReactState(REACT_PASSIVE);
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_TOWER_GUARDS:
                    {
                        summons.SetReactState(REACT_AGGRESSIVE);
                        EntryCheckPredicate pred(NPC_DRAGONMAW_FLAMESLINGER);
                        summons.DoAction(ACTION_TOWER_GUARDS, pred);
                        ZoneTalk(TEXT_GENERIC_0);
                        events.RescheduleEvent(EVENT_SUMMON_DEMOLISHER, 20000);
                        break;
                    }
                    case ACTION_TOWER_TURRET:
                    {
                        EntryCheckPredicate pred(NPC_ANTIAIR_TURRET);
                        summons.DoAction(ACTION_TOWER_GUARDS, pred);
                        break;
                    }
                    case ACTION_TOWER_DESPAWN:
                        summons.DespawnAll();
                        break;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                damage = 0;
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUMMON_DEMOLISHER:
                            me->SummonCreature(NPC_KORKRON_DEMOLISHER, AddsSpawn[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_galakras_north_towerAI(creature);
        }
};

//72408
class npc_antiair_turret : public CreatureScript
{
    public:
        npc_antiair_turret() : CreatureScript("npc_antiair_turret") {}

        struct npc_antiair_turretAI : public Scripted_NoMovementAI
        {
            npc_antiair_turretAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_SELECTABLE);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            }

            InstanceScript* instance;

            void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
            {
                damage = 0;
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_TOWER_GUARDS && instance)
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                    {
                        if (Creature* Summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                        {
                            if (Summoner->GetEntry() == NPC_TOWER_SOUTH)
                            {
                                ZoneTalk(TEXT_GENERIC_0);
                                instance->SetData(DATA_ACTIVE_SOUTH_ROPE, 0);
                            }
                            else if (Summoner->GetEntry() == NPC_TOWER_NORTH)
                            {
                                ZoneTalk(TEXT_GENERIC_1);
                                instance->SetData(DATA_ACTIVE_NORTH_ROPE, 0);
                            }
                        }
                    }
                }
            }
        };
        
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_antiair_turretAI(creature);
        }
};

// Commander South Tower
class npc_lieutenant_krugruk : public CreatureScript
{
    public:
        npc_lieutenant_krugruk() : CreatureScript("npc_lieutenant_krugruk") {}

        struct npc_lieutenant_krugrukAI : public ScriptedAI
        {
            npc_lieutenant_krugrukAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                Talk(SAY_KRUGRUK_0);
                events.RescheduleEvent(EVENT_ARCING_SMASH_START, 12000);
                events.RescheduleEvent(EVENT_THUNDER_CLAP, 22000);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                if (me->GetDistance(me->GetHomePosition()) > 25.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARCING_SMASH_START:
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->GetMotionMaster()->MoveTargetedHome();
                            events.RescheduleEvent(EVENT_ARCING_SMASH_START, 21000);
                            break;
                        case EVENT_TOWER_KNOCK_BACK:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            {
                                me->SetFacingToObject(pTarget);
                                DoCast(pTarget, SPELL_ARCING_SMASH);
                            }
                            events.RescheduleEvent(EVENT_TOWER_KNOCK_BACK, 2000);
                            break;
                        case EVENT_ARCING_SMASH_STOP:
                            me->SetReactState(REACT_AGGRESSIVE);
                            events.CancelEvent(EVENT_TOWER_KNOCK_BACK);
                            break;
                        case EVENT_THUNDER_CLAP:
                            DoCast(SPELL_THUNDER_CLAP);
                            events.RescheduleEvent(EVENT_THUNDER_CLAP, 15000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            void JustReachedHome()
            {
                if (me->isInCombat())
                {
                    events.RescheduleEvent(EVENT_TOWER_KNOCK_BACK, 500);
                    events.RescheduleEvent(EVENT_ARCING_SMASH_STOP, 6000);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                uint32 SouthTowerCount = instance->GetData(DATA_SOUTH_COUNT) - 40;
                instance->SetData(DATA_SOUTH_COUNT, SouthTowerCount);
                Talk(SAY_KRUGRUK_2);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_lieutenant_krugrukAI(creature);
        }
};
// Commander North Tower
class npc_master_cannoneer_dagryn : public CreatureScript
{
    public:
        npc_master_cannoneer_dagryn() : CreatureScript("npc_master_cannoneer_dagryn") {}

        struct npc_master_cannoneer_dagrynAI : public ScriptedAI
        {
            npc_master_cannoneer_dagrynAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                Talk(SAY_DAGRYN_0);
                events.RescheduleEvent(EVENT_DAGRYN_SHOOT, 1000);
                events.RescheduleEvent(EVENT_MUZZLE_SPRAY_START, 15000);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                if (me->GetDistance(me->GetHomePosition()) > 25.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DAGRYN_SHOOT:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                DoCast(pTarget, SPELL_DAGRYN_SHOOT);
                            events.RescheduleEvent(EVENT_DAGRYN_SHOOT, 1500);
                            break;
                        case EVENT_MUZZLE_SPRAY_START:
                            me->SetReactState(REACT_PASSIVE);
                            me->AttackStop();
                            me->GetMotionMaster()->MoveTargetedHome();
                            events.CancelEvent(EVENT_DAGRYN_SHOOT);
                            events.RescheduleEvent(EVENT_MUZZLE_SPRAY_START, 30000);
                            break;
                        case EVENT_TOWER_KNOCK_BACK:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            {
                                me->SetFacingToObject(pTarget);
                                DoCast(pTarget, SPELL_MUZZLE_SPRAY);
                            }
                            events.RescheduleEvent(EVENT_TOWER_KNOCK_BACK, 4000);
                            break;
                        case EVENT_MUZZLE_SPRAY_STOP:
                            me->SetReactState(REACT_AGGRESSIVE);
                            events.CancelEvent(EVENT_TOWER_KNOCK_BACK);
                            events.RescheduleEvent(EVENT_DAGRYN_SHOOT, 2000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            void JustReachedHome()
            {
                if (me->isInCombat())
                {
                    events.RescheduleEvent(EVENT_TOWER_KNOCK_BACK, 500);
                    events.RescheduleEvent(EVENT_MUZZLE_SPRAY_STOP, 9000);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                uint32 NorthTowerCount = instance->GetData(DATA_NORTH_COUNT) - 40;
                instance->SetData(DATA_NORTH_COUNT, NorthTowerCount);
                Talk(SAY_DAGRYN_2);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_master_cannoneer_dagrynAI(creature);
        }
};

class npc_korkron_demolisher : public CreatureScript
{
    public:
        npc_korkron_demolisher() : CreatureScript("npc_korkron_demolisher") {}

        struct npc_korkron_demolisherAI : public ScriptedAI
        {
            npc_korkron_demolisherAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
            }

            void IsSummonedBy(Unit* summoner)
            {
                ZoneTalk(TEXT_GENERIC_0);
                me->GetMotionMaster()->MovePoint(1, 1416.77f, -4827.58f, 18.22f);
                events.RescheduleEvent(EVENT_PREPARE_ATTACK_TOWER, 4000);
            }
            
            void UpdateAI(uint32 diff)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PREPARE_ATTACK_TOWER:
                            if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                                if (Creature* Summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                                {
                                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    me->SetFacingToObject(Summoner);
                                }
                            if (Creature* Zaela = instance->instance->GetCreature(instance->GetGuidData(NPC_WARLORD_ZAELA)))
                                Zaela->AI()->Talk(SAY_GALAKRAS_4);
                            events.RescheduleEvent(EVENT_ATTACK_TOWER, 2000);
                            break;
                        case EVENT_ATTACK_TOWER:
                            if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                                if (Creature* Summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                                {
                                    if (Summoner->GetEntry() == NPC_TOWER_SOUTH)
                                        me->CastSpell(AttackTowers[0].GetPositionX(), AttackTowers[0].GetPositionY(), AttackTowers[0].GetPositionZ(), SPELL_BOMBARD);
                                    if (Summoner->GetEntry() == NPC_TOWER_NORTH)
                                        me->CastSpell(AttackTowers[1].GetPositionX(), AttackTowers[1].GetPositionY(), AttackTowers[1].GetPositionZ(), SPELL_BOMBARD);
                                }
                            events.RescheduleEvent(EVENT_ATTACK_TOWER, 2000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_korkron_demolisherAI(creature);
        }
};

//72943
class npc_dragonmaw_proto_drake : public CreatureScript
{
    public:
        npc_dragonmaw_proto_drake() : CreatureScript("npc_dragonmaw_proto_drake") {}

        struct npc_dragonmaw_proto_drakeAI : public ScriptedAI
        {
            npc_dragonmaw_proto_drakeAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset(){}

            void EnterCombat(Unit* who){}

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_ANTIAIR_CANNON)
                    me->Kill(me);
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (summoner->GetEntry() == NPC_GALAKRAS)
                    events.RescheduleEvent(EVENT_PROTO_DRAKE_GROUND, 1000);
                
                if (summoner->GetEntry() == NPC_TOWER_SOUTH)
                    events.RescheduleEvent(EVENT_PROTO_DRAKE_FLY, 1000);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PROTO_DRAKE_GROUND:
                            me->SetCanFly(false);
                            me->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
                            me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                            me->SetHomePosition(RinOrlortPos[1]);
                            DoZoneInCombat(me, 40.0f);
                            events.RescheduleEvent(EVENT_DRAKE_FLAME_BREATH, 15000);
                            break;
                        case EVENT_PROTO_DRAKE_FLY:
                            me->SetCanFly(true);
                            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                            me->GetMotionMaster()->MoveRandom(5.0f);
                            DoZoneInCombat(me, 180.0f);
                            events.RescheduleEvent(EVENT_DRAKE_DRAKEFIRE, 7000);
                            break;
                        case EVENT_DRAKE_FLAME_BREATH:
                            if (Unit* Target = me->getVictim())
                                DoCast(Target, SPELL_FLAME_BREATH);
                            events.RescheduleEvent(EVENT_DRAKE_FLAME_BREATH, 25000);
                            break;
                        case EVENT_DRAKE_DRAKEFIRE:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_DRAKEFIRE);
                            events.RescheduleEvent(EVENT_DRAKE_DRAKEFIRE, 45000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_proto_drakeAI(creature);
        }
};

class npc_dragonmaw_grunt : public CreatureScript
{
    public:
        npc_dragonmaw_grunt() : CreatureScript("npc_dragonmaw_grunt") {}

        struct npc_dragonmaw_gruntAI : public ScriptedAI
        {
            npc_dragonmaw_gruntAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_THROW_AXE, 8000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (summoner->GetEntry() == NPC_GALAKRAS)
                {
                    me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                    me->SetHomePosition(RinOrlortPos[1]);
                }
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_THROW_AXE:
                            DoCast(SPELL_THROW_AXE);
                            events.RescheduleEvent(EVENT_THROW_AXE, 8000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                {
                    if (Creature* summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                    {
                        if (summoner->GetEntry() == NPC_TOWER_SOUTH)
                        {
                            uint32 SouthTowerCount = instance->GetData(DATA_SOUTH_COUNT) - 10;
                            instance->SetData(DATA_SOUTH_COUNT, SouthTowerCount);
                        }
                        if (summoner->GetEntry() == NPC_TOWER_NORTH)
                        {
                            uint32 NorthTowerCount = instance->GetData(DATA_NORTH_COUNT) - 10;
                            instance->SetData(DATA_NORTH_COUNT, NorthTowerCount);
                        }
                    }
                }
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_gruntAI(creature);
        }
};

class npc_dragonmaw_grunt_h : public CreatureScript
{
    public:
        npc_dragonmaw_grunt_h() : CreatureScript("npc_dragonmaw_grunt_h") {}

        struct npc_dragonmaw_grunt_hAI : public ScriptedAI
        {
            npc_dragonmaw_grunt_hAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_DRAGON_CLEAVE, 5000);
            }

            void IsSummonedBy(Unit* summoner)
            {
                events.RescheduleEvent(EVENT_FIXATE, 1000);
            }

            void SpellHit(Unit* dTarget, SpellInfo const* spell)
            {
                if (dTarget)
                    if (spell->Id == SPELL_BANTER_A)
                        dTarget->CastSpell(dTarget, SPELL_KNOCKED_OVER);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_DRAGON_CLEAVE:
                        if (Unit* dTarget = me->getVictim())
                            if (me->GetDistance(dTarget) <= 5.0f)
                                DoCast(dTarget, SPELL_DRAGON_CLEAVE);
                        events.RescheduleEvent(EVENT_DRAGON_CLEAVE, 10000);
                        break;
                    case EVENT_FIXATE:
                        for (uint8 n = 0; n < 2; n++)
                        {
                            if (Creature* dex = me->FindNearestCreature(demolicionsexpertlist[n], 50.0f, true))
                            {
                                AttackStart(dex);
                                me->AddThreat(dex, 1000000.0f);
                                DoCast(dex, SPELL_FIXATE, true);
                                return;
                            }
                        }
                        events.RescheduleEvent(EVENT_FIXATE, 5000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_grunt_hAI(creature);
        }
};

class npc_dragonmaw_flameslinger : public CreatureScript
{
    public:
        npc_dragonmaw_flameslinger() : CreatureScript("npc_dragonmaw_flameslinger") {}

        struct npc_dragonmaw_flameslingerAI : public ScriptedAI
        {
            npc_dragonmaw_flameslingerAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void IsSummonedBy(Unit* summoner)
            {
                if (summoner->GetEntry() == NPC_GALAKRAS)
                    DoZoneInCombat(me, 150);
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_FLAMESLINGER_ATTACK, 0);
                DoZoneInCombat(me, 150);
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_TOWER_GUARDS)
                {
                    events.RescheduleEvent(EVENT_FLAMESLINGER_ATTACK, 1000);
                    DoZoneInCombat(me, 150.0f);
                }
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();
                
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                { 
                    switch (eventId)
                    {
                        case EVENT_FLAMESLINGER_ATTACK:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                if (me->GetDistance(pTarget) >= 20.0f)
                                    DoCast(pTarget, SPELL_FLAME_ARROWS_EVENT);
                                else
                                    if (Unit* Target = me->getVictim())
                                        DoCast(Target, SPELL_FLAME_ARROWS_COMBAT);
                            }
                            events.RescheduleEvent(EVENT_FLAMESLINGER_ATTACK, 2000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*killer*/)
            {
                if (!instance)
                    return;

                if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
                {
                    if (Creature* summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                    {
                        if (summoner->GetEntry() == NPC_TOWER_SOUTH)
                        {
                            uint32 SouthTowerCount = instance->GetData(DATA_SOUTH_COUNT) - 10;
                            instance->SetData(DATA_SOUTH_COUNT, SouthTowerCount);
                        }
                        if (summoner->GetEntry() == NPC_TOWER_NORTH)
                        {
                            uint32 NorthTowerCount = instance->GetData(DATA_NORTH_COUNT) - 10;
                            instance->SetData(DATA_NORTH_COUNT, NorthTowerCount);
                        }
                    }
                }
            }

        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_flameslingerAI(creature);
        }
};

class npc_dragonmaw_flagbearer : public CreatureScript
{
    public:
        npc_dragonmaw_flagbearer() : CreatureScript("npc_dragonmaw_flagbearer") {}

        struct npc_dragonmaw_flagbearerAI : public ScriptedAI
        {
            npc_dragonmaw_flagbearerAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_WAR_BANNER, 15000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                me->SetHomePosition(RinOrlortPos[1]);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WAR_BANNER:
                            DoCast(SPELL_WAR_BANNER);
                            events.RescheduleEvent(EVENT_WAR_BANNER, 30000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_flagbearerAI(creature);
        }
};

class npc_dragonmaw_bonecrusher : public CreatureScript
{
    public:
        npc_dragonmaw_bonecrusher() : CreatureScript("npc_dragonmaw_bonecrusher") {}

        struct npc_dragonmaw_bonecrusherAI : public ScriptedAI
        {
            npc_dragonmaw_bonecrusherAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                FractureEvent = false;
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;
            bool FractureEvent;
            int8 count;

            void Reset()
            {
                events.Reset();
                count = 1;
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_SHATTERING_ROAR, 9000);
                events.RescheduleEvent(EVENT_FRACTURE, 51000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                me->SetHomePosition(RinOrlortPos[1]);
            }

            void SpellHit(Unit* bTarget, SpellInfo const* spell)
            {
                if(spell->Id == SPELL_PING_BOSS && !FractureEvent)
                {
                    if(roll_chance_i(35 * count))
                    {
                        FractureEvent = true;
                        me->SetReactState(REACT_PASSIVE);
                        AttackStart(bTarget);
                        me->GetMotionMaster()->MoveCharge(bTarget->GetPositionX(), bTarget->GetPositionY(), bTarget->GetPositionZ(), 30.0f);
                        events.RescheduleEvent(EVENT_BONECRUSHER_FRACTURE, 0);
                        count = 1;
                    }
                    else
                        count++;
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHATTERING_ROAR:
                            DoCast(SPELL_SHATTERING_ROAR);
                            events.RescheduleEvent(EVENT_SHATTERING_ROAR, 9000);
                            break;
                        case EVENT_FRACTURE:
                            DoCast(SPELL_FRACTURE);
                            events.RescheduleEvent(EVENT_FRACTURE, 16000);
                            break;
                        case EVENT_BONECRUSHER_FRACTURE:
                            if(Unit* bTarget = me->getVictim())
                            {
                                if (me->GetDistance(bTarget) <= 5.0f)
                                {
                                    DoCast(bTarget, SPELL_FRACTURE_PEREODIC);
                                    me->SetReactState(REACT_AGGRESSIVE);
                                    FractureEvent = false;
                                }
                                else
                                {
                                    me->GetMotionMaster()->MoveCharge(bTarget->GetPositionX(), bTarget->GetPositionY(), bTarget->GetPositionZ(), 30.0f);
                                    events.RescheduleEvent(EVENT_BONECRUSHER_FRACTURE, 500);
                                }
                            }
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_bonecrusherAI(creature);
        }
};

class npc_dragonmaw_tidal_shaman : public CreatureScript
{
    public:
        npc_dragonmaw_tidal_shaman() : CreatureScript("npc_dragonmaw_tidal_shaman") {}

        struct npc_dragonmaw_tidal_shamanAI : public ScriptedAI
        {
            npc_dragonmaw_tidal_shamanAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_CHAIN_HEAL, 20000);
                //events.RescheduleEvent(EVENT_TIDAL_WAVE, 5000);
                events.RescheduleEvent(EVENT_TIDE_TOTEM, 18000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                me->SetHomePosition(RinOrlortPos[1]);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TIDAL_WAVE:
                            DoCast(SPELL_TIDAL_WAVE);
                            events.RescheduleEvent(EVENT_TIDAL_WAVE, 2000);
                            break;
                        case EVENT_CHAIN_HEAL:
                            DoCast(SPELL_CHAIN_HEAL);
                            events.RescheduleEvent(EVENT_CHAIN_HEAL, 15000);
                            break;
                        case EVENT_TIDE_TOTEM:
                            DoCast(SPELL_HEALING_TIDE_TOTEM);
                            events.RescheduleEvent(EVENT_TIDE_TOTEM, 18000, true);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_tidal_shamanAI(creature);
        }
};

class npc_high_enforcer_thranok : public CreatureScript
{
    public:
        npc_high_enforcer_thranok() : CreatureScript("npc_high_enforcer_thranok") {}

        struct npc_high_enforcer_thranokAI : public ScriptedAI
        {
            npc_high_enforcer_thranokAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_CRUSHER_CALL, 15000);
                events.RescheduleEvent(EVENT_SHATTERING_CLEAVE, 5000);
                events.RescheduleEvent(EVENT_SKULL_CRACKER, 17000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                me->SetHomePosition(RinOrlortPos[1]);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CRUSHER_CALL:
                            DoCast(SPELL_CRUSHER_CALL);
                            events.RescheduleEvent(EVENT_CRUSHER_CALL, 30000);
                            break;
                        case EVENT_SHATTERING_CLEAVE:
                        {
                            if (Unit* Target = me->getVictim())
                                DoCast(Target, SPELL_SHATTERING_CLEAVE);
                            events.RescheduleEvent(EVENT_SHATTERING_CLEAVE, 7000);
                            break;
                        }
                        case EVENT_SKULL_CRACKER:
                            DoCast(SPELL_SKULL_CRACKER);
                            events.RescheduleEvent(EVENT_SKULL_CRACKER, 30000);
                            break;
                    }
                }
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_high_enforcer_thranokAI(creature);
        }
};

class npc_korgra_the_snake : public CreatureScript
{
    public:
        npc_korgra_the_snake() : CreatureScript("npc_korgra_the_snake") {}

        struct npc_korgra_the_snakeAI : public ScriptedAI
        {
            npc_korgra_the_snakeAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_POISONTIPPED_BLADES, 12000);
                events.RescheduleEvent(EVENT_POISON_CLOUD, 18000);
                events.RescheduleEvent(EVENT_CURSE_OF_VENOM, 20000);
                events.RescheduleEvent(EVENT_VENOM_BOLT_VOLLEY, 25000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                me->SetHomePosition(RinOrlortPos[1]);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_POISONTIPPED_BLADES:
                        {
                            if (Unit* Target = me->getVictim())
                                DoCast(Target, SPELL_POISONTIPPED_BLADES);
                            events.RescheduleEvent(EVENT_POISONTIPPED_BLADES, 6000);
                            break;
                        }
                        case EVENT_POISON_CLOUD:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                                DoCast(pTarget, SPELL_POISON_CLOUD);
                            events.RescheduleEvent(EVENT_POISON_CLOUD, 60000);
                            break;
                        case EVENT_CURSE_OF_VENOM:
                            events.CancelEvent(EVENT_POISONTIPPED_BLADES);
                            DoCast(SPELL_CURSE_OF_VENOM);
                            break;
                        case EVENT_VENOM_BOLT_VOLLEY:
                            DoCast(SPELL_VENOM_BOLT_VOLLEY);
                            events.RescheduleEvent(EVENT_VENOM_BOLT_VOLLEY, urand(3000, 4000));
                            break;
                    }
                }
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_korgra_the_snakeAI(creature);
        }
};

class npc_dragonmaw_ebon_stalker : public CreatureScript
{
    public:
        npc_dragonmaw_ebon_stalker() : CreatureScript("npc_dragonmaw_ebon_stalker") {}

        struct npc_dragonmaw_ebon_stalkerAI : public ScriptedAI
        {
            npc_dragonmaw_ebon_stalkerAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* who)
            {
                events.RescheduleEvent(EVENT_SHADOW_STALK, 15000);
                events.RescheduleEvent(EVENT_SHADOW_ASSAULT, 16000);
                DoZoneInCombat(me, 30.0f);
            }

            void IsSummonedBy(Unit* summoner)
            {
                me->GetMotionMaster()->MovePoint(1, RinOrlortPos[1]);
                me->SetHomePosition(RinOrlortPos[1]);
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SHADOW_STALK:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 4.0f, true))
                                DoCast(pTarget, SPELL_SHADOW_STALK, true);
                            events.RescheduleEvent(EVENT_SHADOW_STALK, 10000);
                            break;
                        case EVENT_SHADOW_ASSAULT:
                            DoCast(SPELL_SHADOW_ASSAULT);
                            events.RescheduleEvent(EVENT_SHADOW_ASSAULT, 10000);
                            break;
                    }
                }
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_dragonmaw_ebon_stalkerAI(creature);
        }
};

class at_galakras_towers : public AreaTriggerScript
{
public:
    at_galakras_towers() : AreaTriggerScript("at_galakras_towers") { }

    bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (!player->HasAura(SPELL_IN_A_TOWER))
                player->CastSpell(player, SPELL_IN_A_TOWER, true);
            else
                player->RemoveAura(SPELL_IN_A_TOWER);

            return false;
        }
        return false;
    }
};

class spell_most_complicated_bomb : public SpellScriptLoader
{
public:
    spell_most_complicated_bomb() : SpellScriptLoader("spell_most_complicated_bomb") { }

    class spell_most_complicated_bomb_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_most_complicated_bomb_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (InstanceScript* pInstance = GetCaster()->GetInstanceScript())
            {
                if (!pInstance)
                    return;

                if (GetSpellInfo()->Id == SPELL_MOST_COMPLICATED_BOMB_SOUTH)
                {                    
                    uint32 southTower = pInstance->GetData(DATA_SOUTH_COUNT) + 1;
                    pInstance->SetData(DATA_SOUTH_COUNT, southTower);
                }

                if (GetSpellInfo()->Id == SPELL_MOST_COMPLICATED_BOMB_NORTH)
                {                    
                    uint32 northTower = pInstance->GetData(DATA_NORTH_COUNT) + 1;
                    pInstance->SetData(DATA_NORTH_COUNT, northTower);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_most_complicated_bomb_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_most_complicated_bomb_AuraScript();
    }
};

// Flames of Galakrond - 146991
class spell_galakras_flames_of_galakrond : public SpellScriptLoader
{
public:
    spell_galakras_flames_of_galakrond() : SpellScriptLoader("spell_galakras_flames_of_galakrond") { }

    class spell_galakras_flames_of_galakrond_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_galakras_flames_of_galakrond_SpellScript);

        void HandleBeforeCast()
        {
            GetSpell()->destAtTarget = *GetExplTargetDest();
        }

        void HandleOnHit(SpellEffIndex /*effIndex*/)
        {
            if(Unit* caster = GetCaster())
                GetSpell()->destTarget = &*caster;
        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_galakras_flames_of_galakrond_SpellScript::HandleBeforeCast);
            OnEffectHit += SpellEffectFn(spell_galakras_flames_of_galakrond_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_CREATE_AREATRIGGER);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_galakras_flames_of_galakrond_SpellScript();
    }
};

class spell_galakras_tower_rope_jump : public SpellScriptLoader
{
public:
    spell_galakras_tower_rope_jump() : SpellScriptLoader("spell_galakras_tower_rope_jump") { }

    class spell_galakras_tower_rope_jump_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_galakras_tower_rope_jump_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetCaster()->GetMotionMaster()->MoveJump(GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ() + 15, 10.0f, 20.0f);
        }

        void HandlePeriodicTick(AuraEffect const* aurEff)
        {
            uint32 ticks = aurEff->GetTickNumber();
            Unit* caster = GetCaster();
            if (!caster)
                return;

            switch (ticks)
            {
                case 1:
                    caster->GetMotionMaster()->MoveJump(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ() + 15, 10.0f, 20.0f);
                    break;
                case 2:
                    caster->GetMotionMaster()->MoveJump(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ() + 18, 10.0f, 20.0f);
                    break;
                case 3:
                    if (Unit* target = caster->FindNearestCreature(12999, 20.0f))
                        caster->CastSpell(target, SPELL_TOWER_JUMP_4);
                    break;
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_galakras_tower_rope_jump_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_galakras_tower_rope_jump_AuraScript();
    }
};

class CreatureTargetFilter
{
    public:
        bool operator()(WorldObject* target) const
        {
            if (Unit* unit = target->ToCreature())
                if (!unit->isTotem())
                    if (!unit->isPet())
                        return false;

            return true;
        }
};

class spell_galakras_shattering_roar : public SpellScriptLoader
{
    public:
        spell_galakras_shattering_roar() : SpellScriptLoader("spell_galakras_shattering_roar") { }

        class spell_galakras_shattering_roar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_galakras_shattering_roar_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(CreatureTargetFilter());
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_galakras_shattering_roar_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_galakras_shattering_roar_SpellScript();
        }
};

//223281, 223287, 223282
class go_rope_skein : public GameObjectScript
{
public:
    go_rope_skein() : GameObjectScript("go_rope_skein") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        InstanceScript* pInstance = (InstanceScript*)go->GetInstanceScript();
        if (!pInstance)
            return false;

        switch (go->GetEntry())
        {
        case GO_SOUTH_ROPE_SKEIN:
            player->NearTeleportTo(1385.46f, -4839.81f, 33.4012f, 4.9578f);
            break;
        case GO_NORTH_ROPE_SKEIN:
            player->NearTeleportTo(1441.98f, -4815.71f, 28.4978f, 5.4447f);
            break;
        case GO_ROPE:
            if (GameObject* _go = go->FindNearestGameObject(GO_SOUTH_ROPE_SKEIN, 50.0f))
                player->NearTeleportTo(1379.31f, -4839.33f, 71.7601f, 3.3273f);
            if (GameObject* _go2 = go->FindNearestGameObject(GO_NORTH_ROPE_SKEIN, 50.0f))
                player->NearTeleportTo(1445.98f, -4811.17f, 68.3289f, 0.3475f);
            break;
        default:
            break;
        }
        return true;
    }
};

void AddSC_boss_galakras()
{
    new boss_galakras();
    new npc_varian_or_lorthemar();
    new npc_jaina_or_sylvana();
    new npc_verees_or_aethas();
    new npc_demolitions();
    new npc_galakras_south_tower();
    new npc_galakras_north_tower();
    new npc_master_cannoneer_dagryn();
    new npc_lieutenant_krugruk();
    new npc_dragonmaw_flameslinger();
    new npc_dragonmaw_grunt();
    new npc_antiair_turret();
    new npc_korkron_demolisher();
    new npc_high_enforcer_thranok();
    new npc_korgra_the_snake();
    new npc_dragonmaw_flagbearer();
    new npc_dragonmaw_proto_drake();
    new npc_dragonmaw_bonecrusher();
    new npc_dragonmaw_ebon_stalker();
    new npc_dragonmaw_tidal_shaman();
    new npc_dragonmaw_grunt_h();
    new at_galakras_towers();
    new spell_most_complicated_bomb();
    new spell_galakras_flames_of_galakrond();
    //new spell_galakras_tower_rope_jump();
    new spell_galakras_shattering_roar();
    new go_rope_skein();
}
