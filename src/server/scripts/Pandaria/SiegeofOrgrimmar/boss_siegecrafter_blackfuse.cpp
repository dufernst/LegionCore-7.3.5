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

#include "siege_of_orgrimmar.h"
#include "PlayerDefines.h"

enum eSpells
{
    //Blackfuse
    SPELL_ELECTROSTATIC_CHARGE      = 143385,
    SPELL_AUTOMATIC_REPAIR_BEAM_AT  = 144212,
    SPELL_AUTOMATIC_REPAIR_BEAM     = 144213,
    SPELL_LAUNCH_SAWBLADE           = 143265,
    SPELL_LAUNCH_SAWBLADE_AT        = 143329, 
    SPELL_SERRATED_SLASH_DMG        = 143327,
    //overcharge 145774
    //Automated shredder
    SPELL_REACTIVE_ARMOR            = 143387,
    SPELL_DEATH_FROM_ABOVE_K_B      = 144208,
    SPELL_DEATH_FROM_ABOVE          = 147010,
    SPELL_OVERLOAD                  = 145444,
    //Crawler Mine
    SPELL_DETONATE                  = 149146,
    SPELL_READY_TO_GO               = 145580,
    SPELL_BREAK_IN_PERIOD           = 145269,
    SPELL_CRAWLER_MINE_FIXATE       = 144010,
    SPELL_CRAWLER_MINE_FIXATE_D     = 144009,
    SPELL_CRAWLER_MINE_FIXATE_PL    = 149147,
    //Electromagnet
    SPELL_MAGNETIC_CRASH_AT         = 143487,
    SPELL_MAGNETIC_CRASH_DMG        = 144466,
    //Shock Wave
    SPELL_SHOCKWAVE_VISUAL_SPAWN    = 144647,
    SPELL_SHOCKWAVE_VISUAL_SPAWN_HM = 146155,
    SPELL_SHOCKWAVE_VISUAL_TURRET   = 143640, 
    SPELL_SHOCKWAVE_MISSILE_T_M     = 143641,
    SPELL_SHOCKWAVE_MISSILE         = 144658,
    SPELL_SHOCKWAVE_MISSILE2        = 144660,
    SPELL_SHOCKWAVE_MISSILE3        = 144661,
    SPELL_SHOCKWAVE_MISSILE4        = 144662,
    SPELL_SHOCKWAVE_MISSILE5        = 144663,
    SPELL_SHOCKWAVE_MISSILE6        = 144664,
    //Special
    SPELL_MAGNETIC_LASSO_VISUAL     = 145351,
    SPELL_MAGNETIC_LASSO            = 145358,
    SPELL_PATTERN_RECOGNITION       = 144236,
    //Laser array trigger = 71910,
    SPELL_CONVEYOR_DEATH_BEAM_V     = 144284,
    SPELL_CONVEYOR_DEATH_BEAM_V2    = 149016,
    SPELL_CONVEYOR_DEATH_BEAM_AT    = 144282,
    SPELL_PURIFICATION_BEAM         = 144335,
    SPELL_CREATE_CONVEYOR_TRIGGER   = 145272,
    //Laser turret
    SPELL_DISINTEGRATION_LASER_V    = 143867,
    SPELL_PURSUIT_LASER             = 143828,
    SPELL_LASER_GROUND_PERIODIC_AT  = 143829,
    SPELL_SUPERHEATER               = 144040,
};

uint32 shockwavemissilelist[6] =
{
    SPELL_SHOCKWAVE_MISSILE,
    SPELL_SHOCKWAVE_MISSILE2,
    SPELL_SHOCKWAVE_MISSILE3,
    SPELL_SHOCKWAVE_MISSILE4,
    SPELL_SHOCKWAVE_MISSILE5,
    SPELL_SHOCKWAVE_MISSILE6,
};

enum eEvents
{
    EVENT_SAWBLADE                  = 1,
    EVENT_ELECTROSTATIC_CHARGE      = 2,
    EVENT_ACTIVE_CONVEYER           = 3,
    EVENT_START_CONVEYER            = 4,
    EVENT_SUMMON_SHREDDER           = 5,
    //Weapon
    EVENT_ACTIVE                    = 6,
    //Crawler Mine
    EVENT_PURSUE                    = 7,
    EVENT_CHECK_DISTANCE            = 8,
    //Rocket Turret
    EVENT_SHOCKWAVE_MISSILE         = 9,
    //Automated Shredder
    EVENT_DEATH_FROM_ABOVE          = 10,
    EVENT_OVERLOAD                  = 11,
    //Special
    EVENT_DESPAWN                   = 12,
    EVENT_START_ROTATE              = 13,
    EVENT_LASER_ROTATE              = 14,
    EVENT_LASER_ROTATE2             = 15,
    EVENT_LAUNCH_FORWARD            = 16,
    EVENT_LAUNCH_BACK               = 17,
    EVENT_CHECK_PROGRESS            = 18,
};

enum _ATentry
{
    ENTER_1                         = 9250,
    ENTER_2                         = 9251,
    ENTER_3                         = 9252,
    LEAVE_1                         = 9253,
    LEAVE_2                         = 9371,
    LEAVE_3                         = 9240,
    ENTER_CONVEYOR                  = 9189,
    ENTER_CONVEYOR_2                = 9190,
    LEAVE_CONVEYOR                  = 9493,
    LEAVE_CONVEYOR_2                = 9194,
    LEAVE_CONVEYOR_3                = 9238,
    LEAVE_CONVEYOR_4                = 9239,
};

Position atdestpos[4] =
{
    { 1927.78f, -5566.85f, -309.3259f, 5.2955f },
    { 1889.66f, -5511.84f, -294.8935f, 2.1186f },
    { 1994.60f, -5503.39f, -302.8841f, 2.1813f },
    { 2008.93f, -5600.63f, -309.3268f, 3.6947f },
};

Position lapos[5] =
{
    { 1999.54f, -5537.81f, -302.9150f, 0.0f },
    { 2005.43f, -5534.02f, -302.9150f, 0.0f }, 
    { 2011.31f, -5530.24f, -302.9150f, 0.0f },
    { 2017.20f, -5526.45f, -302.9150f, 0.0f },
    { 2023.09f, -5522.66f, -302.9150f, 0.0f },
};

Position lapos2[5] =
{
    { 2017.53f, -5563.97f, -303.5679f, 0.0f },
    { 2023.42f, -5560.20f, -303.5679f, 0.0f },
    { 2029.31f, -5556.41f, -303.5679f, 0.0f },
    { 2035.19f, -5552.62f, -303.5679f, 0.0f },
    { 2041.08f, -5548.85f, -303.5679f, 0.0f },
};

Position lapos3[5] =
{
    { 2033.35f, -5587.66f, -303.2579f, 0.0f },
    { 2039.25f, -5583.87f, -303.2579f, 0.0f },
    { 2045.14f, -5580.10f, -303.2579f, 0.0f },
    { 2051.03f, -5576.31f, -303.2579f, 0.0f },
    { 2056.91f, -5572.51f, -303.2579f, 0.0f },
};

uint32 wavearray[6][4] =
{
    {0, NPC_DEACTIVATED_MISSILE_TURRET, NPC_DISASSEMBLED_CRAWLER_MINE, NPC_DEACTIVATED_LASER_TURRET},
    {1, NPC_DISASSEMBLED_CRAWLER_MINE, NPC_DEACTIVATED_LASER_TURRET, NPC_DEACTIVATED_MISSILE_TURRET},
    {2, NPC_DEACTIVATED_LASER_TURRET, NPC_DEACTIVATED_ELECTROMAGNET, NPC_DEACTIVATED_MISSILE_TURRET},
    {3, NPC_DEACTIVATED_LASER_TURRET, NPC_DEACTIVATED_MISSILE_TURRET, NPC_DISASSEMBLED_CRAWLER_MINE},
    {4, NPC_DEACTIVATED_MISSILE_TURRET, NPC_DEACTIVATED_ELECTROMAGNET, NPC_DISASSEMBLED_CRAWLER_MINE},
    {5, NPC_DISASSEMBLED_CRAWLER_MINE, NPC_DEACTIVATED_LASER_TURRET, NPC_DISASSEMBLED_CRAWLER_MINE},
};

Position spawnweaponpos[3] =
{
    { 1973.65f, -5472.38f, -302.8868f, 5.294743f }, //old z -299.0f
    { 1958.50f, -5450.76f, -302.8868f, 5.294743f },
    { 1941.65f, -5425.50f, -302.8868f, 5.294743f },
};

uint32 aweaponentry[3] =
{
    NPC_ACTIVATED_LASER_TURRET,
    NPC_ACTIVATED_ELECTROMAGNET,
    NPC_ACTIVATED_MISSILE_TURRET,
};

Position droppos = {1966.44f, -5562.38f, -309.3269f};
Position destpos = {2073.01f, -5620.12f, -302.2553f};
Position cmdestpos = {1905.39f, -5631.86f, -309.3265f};
Position sumshrederpos = {1902.65f, -5625.15f, -309.3269f};
Position sehsumpos = {2009.70f, -5549.21f, -302.8851f};
Position _centerpos = {1956.0f, -5608.66f, -309.327f};
Position sbdestpos = {1900.41f, -5646.18f, -307.45f};
Position oelectromagnetpos = {2023.32f, -5561.57f, -303.0197f, 3.8353f};
float const sbspeed = 25.0f;//sawblade speed

enum CretureText
{
    SAY_PULL,
    SAY_SAWBLADE,
    SAY_SPAWN_SHREDDER,
    SAY_WEAPON_DEATH,
    SAY_KILL_UNIT,
    SAY_DEATH,
    SAY_DEATH2,
};

enum SActions
{
    ACTION_DESPAWN,
};

//71504
class boss_siegecrafter_blackfuse : public CreatureScript
{
public:
    boss_siegecrafter_blackfuse() : CreatureScript("boss_siegecrafter_blackfuse") {}
    
    struct boss_siegecrafter_blackfuseAI : public BossAI
    {
        boss_siegecrafter_blackfuseAI(Creature* creature) : BossAI(creature, DATA_BLACKFUSE), summon(me)
        {
            instance = creature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_ID, 348, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, 108686, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
            reset = 0;
        }

        InstanceScript* instance;
        SummonList summon;
        uint32 checkvictim;
        uint32 updatehmlaserwalls;
        uint32 reset;
        uint32 berserk;
        uint8 weaponwavecount;
        ObjectGuid laserline[3][5];
        uint8 laserwallmod[3];
        bool createconveyer;

        void Reset()
        {
            events.Reset();
            instance->SetBossState(DATA_BLACKFUSE, NOT_STARTED);
            SpecialDespawnSummons();
            me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), me->GetHomePosition().GetOrientation());
            checkvictim = 0;
            weaponwavecount = 0;
            updatehmlaserwalls = 0;
            berserk = 0;
            DespawnOverchargedElectromagnet();
            RemoveDebuffs();
            me->RemoveAurasDueToSpell(SPELL_PROTECTIVE_FRENZY);
            me->RemoveAurasDueToSpell(SPELL_AUTOMATIC_REPAIR_BEAM_AT);
            me->RemoveAurasDueToSpell(SPELL_ENERGIZED_DEFENSIVE_MATRIX);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ELECTROSTATIC_CHARGE);
            me->SetReactState(REACT_DEFENSIVE);
            ClearConveyerArray();
        }

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
        }

        void SpecialDespawnSummons()
        {
            if (!summon.empty())
            {
                for (std::list<ObjectGuid>::const_iterator itr = summon.begin(); itr != summon.end(); ++itr)
                {
                    if (Creature* _sum = me->GetCreature(*me, *itr))
                    {
                        switch (_sum->GetEntry())
                        {
                        case NPC_LASER_TARGET:
                        case NPC_ACTIVATED_LASER_TURRET:
                            _sum->AI()->DoAction(ACTION_DESPAWN);
                            break;
                        default:
                            _sum->DespawnOrUnsummon();
                            break;
                        }
                    }
                }
            }
        }

        void JustReachedHome()
        {
            reset = 5000;
        }

        void ClearConveyerArray()
        {
            for (uint8 n = 0; n < 3; n++)
                for (uint8 m = 0; m < 5; m++)
                    laserline[n][m].Clear();
            for (uint8 b = 0; b < 3; b++)
                laserwallmod[b] = 0;
            createconveyer = false;
        }

        void DespawnOverchargedElectromagnet()
        {
            std::list<Creature*>addlist;
            addlist.clear();
            GetCreatureListWithEntryInGrid(addlist, me, NPC_OVERCHARGED_ELECTROMAGNET, 250.0f);
            if (!addlist.empty())
                for (std::list<Creature*>::const_iterator itr = addlist.begin(); itr != addlist.end(); itr++)
                    (*itr)->DespawnOrUnsummon();

            std::list<AreaTrigger*> atlist;
            atlist.clear();
            me->GetAreaTriggersWithEntryInRange(atlist, 4903, ObjectGuid::Empty, 250.0f);
            me->GetAreaTriggersWithEntryInRange(atlist, 4894, ObjectGuid::Empty, 250.0f);
            if (!atlist.empty())
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                    (*itr)->RemoveFromWorld();
        }

        void RemoveDebuffs()
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CRAWLER_MINE_FIXATE_PL);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SUPERHEATER);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ON_CONVEYOR);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PATTERN_RECOGNITION);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGNETIC_CRASH_DMG);
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_GET_WEAPON_WAVE_INDEX)
                return weaponwavecount;
            return 0;
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            Talk(SAY_PULL);
            checkvictim = 1000;
            berserk = 600000;
            DoCast(me, SPELL_AUTOMATIC_REPAIR_BEAM_AT, true);
            events.RescheduleEvent(EVENT_ELECTROSTATIC_CHARGE, 1000);
            events.RescheduleEvent(EVENT_CHECK_PROGRESS, 5000);
            events.RescheduleEvent(EVENT_SAWBLADE, 10000);
            events.RescheduleEvent(EVENT_ACTIVE_CONVEYER, 2000);
            events.RescheduleEvent(EVENT_SUMMON_SHREDDER, 36000);
            if (Creature* seh = me->SummonCreature(NPC_SIEGE_ENGINEER_HELPER, sehsumpos))
                seh->CastSpell(seh, SPELL_CREATE_CONVEYOR_TRIGGER);
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->ToPlayer())
                Talk(SAY_KILL_UNIT);
        }

        void CreateWeaponWave(uint8 wavecount)
        {
            for (uint8 n = 1; n < 4; n++)
                if (Creature* weapon = me->SummonCreature(wavearray[wavecount][n], spawnweaponpos[n - 1]))
                    weapon->GetMotionMaster()->MoveJump(destpos.GetPositionX(), destpos.GetPositionY(), destpos.GetPositionZ(), 8.0f, 0.0f, 0);
            me->MonsterTextEmote("New Weapons Spawn on Conveyor", ObjectGuid::Empty, true);
            weaponwavecount = weaponwavecount >= 5 ? 0 : ++weaponwavecount;
        }

        void CreateLaserWalls()
        {
            createconveyer = true;
            for (uint8 n = 0; n < 5; n++)
                if (Creature* laser = me->SummonCreature(NPC_LASER_ARRAY, lapos[n]))
                    laserline[0][n] = laser->GetGUID();
            for (uint8 m = 0; m < 5; m++)
                if (Creature* laser2 = me->SummonCreature(NPC_LASER_ARRAY, lapos2[m]))
                    laserline[1][m] = laser2->GetGUID();
            for (uint8 b = 0; b < 5; b++)
                if (Creature* laser3 = me->SummonCreature(NPC_LASER_ARRAY, lapos3[b]))
                    laserline[2][b] = laser3->GetGUID();
            if (me->GetMap()->IsHeroic())
                InitializeHMLaserWalls();
            else
                UpdateLaserWalls();
        }

        void ResetHMLaserWalls()
        {
            updatehmlaserwalls = 0;
            laserwallmod[0] = 1;
            laserwallmod[1] = 2;
            laserwallmod[2] = 3;
            for (uint8 n = 0; n < 3; n++)
            {
                for (uint8 m = 0; m < 5; m++)
                {
                    if (Creature* laser = me->GetCreature(*me, laserline[n][m]))
                    {
                        if (m == laserwallmod[n])
                        {
                            if (laser->HasAura(SPELL_CONVEYOR_DEATH_BEAM_V))
                            {
                                if (AreaTrigger* at = laser->GetAreaObject(SPELL_CONVEYOR_DEATH_BEAM_AT))
                                    at->Despawn();
                                laser->RemoveAurasDueToSpell(SPELL_CONVEYOR_DEATH_BEAM_V);
                            }
                        }
                        else
                        {
                            if (!laser->HasAura(SPELL_CONVEYOR_DEATH_BEAM_V))
                            {
                                laser->AddAura(SPELL_CONVEYOR_DEATH_BEAM_V, laser);
                                laser->CastSpell(laser, SPELL_CONVEYOR_DEATH_BEAM_AT, true);
                            }
                        }
                    }
                }
            }
            updatehmlaserwalls = 4000;
        }

        void InitializeHMLaserWalls()
        {
            uint8 mod = 1;
            for (uint8 n = 0; n < 3; n++, mod++)
            {
                for (uint8 m = 0; m < 5; m++)
                {
                    if (Creature* laser = me->GetCreature(*me, laserline[n][m]))
                    {
                        if (m == mod)
                            laser->CastSpell(laser, SPELL_CONVEYOR_DEATH_BEAM_V2);
                        else
                        {
                            laser->AddAura(SPELL_CONVEYOR_DEATH_BEAM_V, laser);
                            laser->CastSpell(laser, SPELL_CONVEYOR_DEATH_BEAM_AT, true);
                        }
                    }
                }
            }
            laserwallmod[0] = 1;
            laserwallmod[1] = 2;
            laserwallmod[2] = 3;
            updatehmlaserwalls = 4000;
        }

        void UpdateLaserWalls()
        {
            uint8 mod, mod2;
            for (uint8 n = 0; n < 3; n++)
            {
                mod = urand(0, 1);
                mod2 = urand(2, 4);
                for (uint8 m = 0; m < 5; m++)
                {
                    if (Creature* laser = me->GetCreature(*me, laserline[n][m]))
                    {
                        if (m == mod || m == mod2)
                        {
                            if (laser->HasAura(SPELL_CONVEYOR_DEATH_BEAM_V))
                            {
                                if (AreaTrigger* at = laser->GetAreaObject(SPELL_CONVEYOR_DEATH_BEAM_AT))
                                    at->Despawn();
                                laser->RemoveAurasDueToSpell(SPELL_CONVEYOR_DEATH_BEAM_V);
                            }
                        }
                        else
                        {
                            if (!laser->HasAura(SPELL_CONVEYOR_DEATH_BEAM_V))
                            {
                                laser->AddAura(SPELL_CONVEYOR_DEATH_BEAM_V, laser);
                                laser->CastSpell(laser, SPELL_CONVEYOR_DEATH_BEAM_AT, true);
                            }
                        }
                    }
                }
            }
        }

        void UpdateHMLaserWalls()
        {
            for (uint8 b = 0; b < 3; b++)
            {
                if (laserwallmod[b] + 1 > 4)
                    laserwallmod[b] = 0;
                else
                    laserwallmod[b] += 1;
            }

            for (uint8 n = 0; n < 3; n++)
            {
                for (uint8 m = 0; m < 5; m++)
                {
                    if (Creature* laser = me->GetCreature(*me, laserline[n][m]))
                    {
                        if (m == laserwallmod[n])
                        {
                            if (laser->HasAura(SPELL_CONVEYOR_DEATH_BEAM_V))
                            {
                                if (AreaTrigger* at = laser->GetAreaObject(SPELL_CONVEYOR_DEATH_BEAM_AT))
                                    at->Despawn();
                                laser->RemoveAurasDueToSpell(SPELL_CONVEYOR_DEATH_BEAM_V);
                                laser->CastSpell(laser, SPELL_CONVEYOR_DEATH_BEAM_V2);
                            }
                        }
                        else
                        {
                            if (!laser->HasAura(SPELL_CONVEYOR_DEATH_BEAM_V))
                            {
                                laser->AddAura(SPELL_CONVEYOR_DEATH_BEAM_V, laser);
                                laser->CastSpell(laser, SPELL_CONVEYOR_DEATH_BEAM_AT, true);
                            }
                        }
                    }
                }
            }
            updatehmlaserwalls = 4000;
        }

        void JustDied(Unit* killer)
        {
            SpecialDespawnSummons();
            RemoveDebuffs();
            Talk(urand(SAY_DEATH, SAY_DEATH2));
            _JustDied();
            Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
            if (!PlayerList.isEmpty())
                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                    if (Player* player = Itr->getSource())
                        player->ModifyCurrency(CURRENCY_TYPE_VALOR_POINTS, 7000);
        }

        bool CheckEvade()
        {
            if (Creature* stalker = me->FindNearestCreature(NPC_SHOCKWAVE_MISSILE_STALKER, 60.0f, true))
                return true;
            return false;
        }

        void UpdateAI(uint32 diff)
        {
            if (reset)
            {
                if (reset <= diff)
                {
                    reset = 0;
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                }
                else
                    reset -= diff;
            }

            if (!UpdateVictim())
                return;

            if (checkvictim)
            {
                if (checkvictim <= diff)
                {
                    if (!CheckEvade())
                    {
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        EnterEvadeMode();
                        return;
                    }
                    checkvictim = 1000;
                }
                else
                    checkvictim -= diff;
            }

            if (updatehmlaserwalls)
            {
                if (updatehmlaserwalls <= diff)
                {
                    updatehmlaserwalls = 0;
                    UpdateHMLaserWalls();
                }
                else
                    updatehmlaserwalls -= diff;
            }

            if (berserk)
            {
                if (berserk <= diff)
                {
                    berserk = 0;
                    Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                    if (!PlayerList.isEmpty())
                        for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                            if (Player* player = Itr->getSource())
                                if (player->isAlive())
                                    me->Kill(player, true);
                }
                else
                    berserk -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_PROGRESS:
                    if (instance && instance->GetBossState(DATA_MALKOROK) != DONE)
                    {
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        EnterEvadeMode();
                    }
                    break;
                case EVENT_SAWBLADE:
                {
                    bool havetarget = false;
                    std::vector<ObjectGuid>_pllist;
                    _pllist.clear();
                    std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                    if (!ThreatList.empty())
                        for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); itr++)
                            if (Player* pl = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                                if (!pl->isInTankSpec() && !pl->HasAura(SPELL_ON_CONVEYOR) && !pl->HasAura(SPELL_PATTERN_RECOGNITION))
                                    _pllist.push_back(pl->GetGUID());

                    if (!_pllist.empty())
                    {
                        std::random_shuffle(_pllist.begin(), _pllist.end());
                        for (std::vector<ObjectGuid>::const_iterator itr = _pllist.begin(); itr != _pllist.end(); itr++)
                        {
                            if (Player* pl = me->GetPlayer(*me, *itr))
                            {
                                if (pl->isAlive() && me->GetExactDist(pl) >= 15.0f)
                                {
                                    havetarget = true;
                                    Talk(SAY_SAWBLADE);
                                    DoCast(pl, SPELL_LAUNCH_SAWBLADE);
                                    break;
                                }
                            }
                        }
                        if (!havetarget)
                        {
                            for (std::vector<ObjectGuid>::const_iterator itr = _pllist.begin(); itr != _pllist.end(); itr++)
                            {
                                if (Player* pl = me->GetPlayer(*me, *itr))
                                {
                                    if (pl->isAlive())
                                    {
                                        Talk(SAY_SAWBLADE);
                                        DoCast(pl, SPELL_LAUNCH_SAWBLADE);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_SAWBLADE, 13000);
                    break;
                }
                case EVENT_ELECTROSTATIC_CHARGE:
                    if (me->getVictim())
                        if (Player* pl = me->getVictim()->ToPlayer())
                            if (pl->isInTankSpec())
                                DoCast(pl, SPELL_ELECTROSTATIC_CHARGE);
                    events.RescheduleEvent(EVENT_ELECTROSTATIC_CHARGE, 15000);
                    break;
                case EVENT_ACTIVE_CONVEYER:
                    if (!createconveyer)
                        CreateLaserWalls();
                    else
                    {
                        if (me->GetMap()->IsHeroic())
                            ResetHMLaserWalls();
                        else
                            UpdateLaserWalls();
                    }
                    events.RescheduleEvent(EVENT_START_CONVEYER, 10000);
                    break;
                case EVENT_START_CONVEYER:
                    CreateWeaponWave(weaponwavecount);
                    events.RescheduleEvent(EVENT_ACTIVE_CONVEYER, 30000);
                    break;
                case EVENT_SUMMON_SHREDDER:
                    Talk(SAY_SPAWN_SHREDDER);
                    if (Creature* shredder = me->SummonCreature(NPC_AUTOMATED_SHREDDER, sumshrederpos.GetPositionX(), sumshrederpos.GetPositionY(), sumshrederpos.GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                        shredder->AI()->DoZoneInCombat(shredder, 100.0f);
                    events.RescheduleEvent(EVENT_SUMMON_SHREDDER, 60000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_siegecrafter_blackfuseAI(creature);
    }
};

//71532(0), 72694(1)
class npc_blackfuse_passenger : public CreatureScript
{
public:
    npc_blackfuse_passenger() : CreatureScript("npc_blackfuse_passenger") {}

    struct npc_blackfuse_passengerAI : public ScriptedAI
    {
        npc_blackfuse_passengerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }
        InstanceScript* instance;
        EventMap events;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}
        
        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
            {
                switch (pointId)
                {
                case 4:
                    if (Creature* electromagnet = me->GetCreature(*me, instance->GetGuidData(NPC_ACTIVATED_ELECTROMAGNET)))
                    {
                        if (electromagnet->AI()->GetData(DATA_ACTIVE_SUPERHEAT))
                            electromagnet->AI()->SetData(DATA_SAWBLADE_IN_POINT_ELECTROMAGNET, 0);
                        else
                            me->DespawnOrUnsummon();
                    }
                    break;
                case 5:
                    if (Creature* electromagnet = me->GetCreature(*me, instance->GetGuidData(NPC_ACTIVATED_ELECTROMAGNET)))
                        electromagnet->AI()->SetData(DATA_SAWBLADE_IN_POINT_CONVEYER, 0);
                    break;
                }
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            if (me->GetEntry() == NPC_BLACKFUSE_SAWBLADE)
            {
                switch (type)
                {
                case DATA_SAWBLADE_CHANGE_POLARITY_FORWARD:
                    events.RescheduleEvent(EVENT_LAUNCH_FORWARD, 1500);
                    break;
                case DATA_SAWBLADE_CHANGE_POLARITY_BACK:
                    events.RescheduleEvent(EVENT_LAUNCH_BACK, 1500);
                    break;
                }
            }
        }
        
        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
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
                case EVENT_LAUNCH_FORWARD:
                {
                    float x, y;
                    GetPositionWithDistInOrientation(me, 100.0f, me->GetOrientation(), x, y);
                    me->GetMotionMaster()->MoveJump(x, y, me->GetPositionZ(), sbspeed, 0, 5);
                }
                break;
                case EVENT_LAUNCH_BACK:
                    me->GetMotionMaster()->MoveCharge(sbdestpos.GetPositionX(), sbdestpos.GetPositionY(), me->GetPositionZ(), sbspeed, 4);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackfuse_passengerAI(creature);
    }
};

//71591
class npc_automated_shredder : public CreatureScript
{
public:
    npc_automated_shredder() : CreatureScript("npc_automated_shredder") {}

    struct npc_automated_shredderAI : public ScriptedAI
    {
        npc_automated_shredderAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->ModifyAuraState(AURA_STATE_UNKNOWN22, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, 132469, true); //Typhoon
            me->ApplySpellImmune(0, IMMUNITY_ID, 51490,  true); //Thunderstorm
            me->ApplySpellImmune(0, IMMUNITY_ID, 117962, true); //Crackling Jade Shock
            me->ApplySpellImmune(0, IMMUNITY_ID, 149575, true); //Explosive Trap
            me->ApplySpellImmune(0, IMMUNITY_ID, 143327, true);
        }
        InstanceScript* instance;
        EventMap events;
        uint32 landing;

        void Reset()
        {
            events.Reset();
            me->SetReactState(REACT_AGGRESSIVE);
            if (!me->HasAura(SPELL_REACTIVE_ARMOR))
                DoCast(me, SPELL_REACTIVE_ARMOR, true);
            landing = 0;
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_OVERLOAD, 6000);
            events.RescheduleEvent(EVENT_DEATH_FROM_ABOVE, 18000);
        }

        void JustDied(Unit* killer)
        {
            if (me->ToTempSummon())
                if (Unit* blackfuse = me->ToTempSummon()->GetSummoner())
                    blackfuse->CastSpell(blackfuse, SPELL_PROTECTIVE_FRENZY, true);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (pointId == 0)
                landing = 200;
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (landing)
            {
                if (landing <= diff)
                {
                    me->GetMotionMaster()->MoveCharge(me->GetPositionX(), me->GetPositionY(), -309.3269f, 10.0f, 1);
                    me->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
                    landing = 0;
                }
                else
                    landing -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DEATH_FROM_ABOVE:
                    me->StopAttack();
                    DoCast(me, SPELL_DEATH_FROM_ABOVE_K_B);
                    events.RescheduleEvent(EVENT_DEATH_FROM_ABOVE, 35000);
                    break;
                case EVENT_OVERLOAD:
                    DoCast(me, SPELL_OVERLOAD);
                    events.RescheduleEvent(EVENT_OVERLOAD, 10000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_automated_shredderAI(creature);
    }
};

//71790, 71751, 71694, 71606, 71788, 71752, 71696, 71638
class npc_blackfuse_weapon : public CreatureScript
{
public:
    npc_blackfuse_weapon() : CreatureScript("npc_blackfuse_weapon") {}

    struct npc_blackfuse_weaponAI : public ScriptedAI
    {
        npc_blackfuse_weaponAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            me->SetReactState(REACT_PASSIVE);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            targetGuid.Clear();
            done = false;
            modang = 0.7f;
            missilecount = 0;
            sawbladenum = 0;
            oelectromagnetGuid.Clear();
            _sawbladelist.clear();
            superheat = false;
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;
        ObjectGuid oelectromagnetGuid;
        float modang;
        uint8 missilecount;
        uint8 sawbladenum;
        bool done, superheat;
        std::vector<ObjectGuid>_sawbladelist;

        void Reset()
        {
            //fix problems with - SPELL_AURA_INTERFERE_TARGETTING
            switch (me->GetEntry())
            {
            case NPC_DISASSEMBLED_CRAWLER_MINE:
            case NPC_DEACTIVATED_LASER_TURRET:
            case NPC_DEACTIVATED_ELECTROMAGNET:
            case NPC_DEACTIVATED_MISSILE_TURRET:
                DoCast(me, SPELL_ON_CONVEYOR, true);
                break;
            default:
                break;
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            switch (me->GetEntry())
            {
            case NPC_BLACKFUSE_CRAWLER_MINE:
                me->getThreatManager().addThreat(attacker, 0.0f);
                break;
            case NPC_ACTIVATED_LASER_TURRET:
            case NPC_ACTIVATED_ELECTROMAGNET:
            case NPC_ACTIVATED_MISSILE_TURRET:
                damage = 0;
                break;
            case NPC_DISASSEMBLED_CRAWLER_MINE:
            case NPC_DEACTIVATED_LASER_TURRET:
            case NPC_DEACTIVATED_ELECTROMAGNET:
            case NPC_DEACTIVATED_MISSILE_TURRET:
                if (!attacker->HasAura(SPELL_ON_CONVEYOR))
                    damage = 0;
                else
                    if (damage >= me->GetHealth())
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                break;
            }
        }

        void JustDied(Unit* killer)
        {
            switch (me->GetEntry())
            {
            case NPC_DISASSEMBLED_CRAWLER_MINE:
            case NPC_DEACTIVATED_LASER_TURRET:
            case NPC_DEACTIVATED_ELECTROMAGNET:
            case NPC_DEACTIVATED_MISSILE_TURRET:
                if (me->ToTempSummon())
                    if (Unit* blackfuse = me->ToTempSummon()->GetSummoner())
                        blackfuse->ToCreature()->AI()->Talk(SAY_WEAPON_DEATH);
                instance->SetData(DATA_SAFE_WEAPONS, me->GetEntry());
                me->DespawnOrUnsummon(1000);
                break;
            case NPC_BLACKFUSE_CRAWLER_MINE:
                if (Player* pl = me->GetPlayer(*me, targetGuid))
                    if (pl->isAlive())
                        pl->RemoveAurasDueToSpell(SPELL_CRAWLER_MINE_FIXATE_PL);

                if (me->HasAura(SPELL_SUPERHEATED_CRAWLER_MINE))
                    if (me->ToTempSummon())
                        if (Unit* blackfuse = me->ToTempSummon()->GetSummoner())
                            for (uint8 n = 0; n < 2; n++)
                                if (Creature* mine = blackfuse->SummonCreature(NPC_BLACKFUSE_CRAWLER_MINE, me->GetPositionX() + n * 2, me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
                                    mine->AI()->SetData(DATA_CRAWLER_MINE_ENTERCOMBAT, 0);
                me->DespawnOrUnsummon();
                break;
            default:
                break;
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_CRAWLER_MINE_ENTERCOMBAT:
            {
                uint32 mod = 0;
                switch (data)
                {
                case 0:
                    mod = 0;
                    break;
                case 1:
                    mod = 2000;
                    break;
                case 2:
                    mod = 4000;
                    break;
                case 3:
                    mod = 6000;
                    break;
                case 4:
                    mod = 8000;
                    break;
                case 5:
                    mod = 10000;
                    break;
                case 6:
                    mod = 12000;
                    break;
                }
                events.RescheduleEvent(EVENT_ACTIVE, 500 + mod);
            }
            break;
            case DATA_SAWBLADE_IN_POINT_ELECTROMAGNET:
                sawbladenum++;
                if (sawbladenum >= _sawbladelist.size() - 1)
                {
                    sawbladenum = 0;
                    float baseang = 0;
                    float mod = 0;
                    int8 mod2 = 1;
                    for (std::vector<ObjectGuid>::const_iterator itr = _sawbladelist.begin(); itr != _sawbladelist.end(); ++itr)
                    {
                        if (itr == _sawbladelist.begin())
                        {
                            if (Creature* sawblade = me->GetCreature(*me, *itr))
                            {
                                if (Creature* stalker = me->GetCreature(*me, instance->GetGuidData(NPC_SHOCKWAVE_MISSILE_STALKER)))
                                {
                                    sawblade->SetFacingTo(0.5938f);
                                    baseang = 0.5938f;
                                    sawblade->AI()->SetData(DATA_SAWBLADE_CHANGE_POLARITY_FORWARD, 0);
                                    mod += 0.2f;
                                }
                            }
                        }
                        else
                        {
                            if (Creature* sawblade = me->GetCreature(*me, *itr))
                            {
                                float newang = 0;
                                mod2 *= -1;
                                if (mod2 < 0)
                                {
                                    if (mod == 0.6f)
                                        newang = 6.27f;
                                    else if (mod > 0.6f)
                                        newang = 6.27f - (mod - 0.6f);
                                    else
                                        newang = baseang - mod;
                                }
                                else
                                {
                                    newang = baseang + mod;
                                    mod += 0.2f;
                                }
                                sawblade->SetFacingTo(newang);
                                sawblade->AI()->SetData(DATA_SAWBLADE_CHANGE_POLARITY_FORWARD, 0);
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_LAUNCH_FORWARD, 1500);
                }
                break;
            case DATA_SAWBLADE_IN_POINT_CONVEYER:
                sawbladenum++;
                if (sawbladenum >= _sawbladelist.size() - 1)
                {
                    sawbladenum = 0;
                    for (std::vector<ObjectGuid>::const_iterator itr = _sawbladelist.begin(); itr != _sawbladelist.end(); ++itr)
                        if (Creature* sawblade = me->GetCreature(*me, *itr))
                            sawblade->AI()->SetData(DATA_SAWBLADE_CHANGE_POLARITY_BACK, 0);
                    events.RescheduleEvent(EVENT_LAUNCH_BACK, 1500);
                }
                break;
            case DATA_ACTIVE_SUPERHEAT:
                superheat = true;
                break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_ACTIVE_SUPERHEAT)
                return uint32(superheat);
            return 0;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE || type == POINT_MOTION_TYPE)
            {
                switch (pointId)
                {
                //offline weapon in dest point
                case 0:
                    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) || me->GetEntry() == NPC_BLACKFUSE_CRAWLER_MINE)
                        return;
                    instance->SetData(DATA_D_WEAPON_IN_DEST_POINT, 0);
                    me->DespawnOrUnsummon();
                    break;
                //online weapon in dest point
                case 1:
                    me->SetFacingTo(0.435088f);
                    switch (me->GetEntry())
                    {
                    case NPC_BLACKFUSE_CRAWLER_MINE:
                        if (!me->HasAura(SPELL_SUPERHEATED_CRAWLER_MINE))
                            instance->SetData(DATA_CRAWLER_MINE_READY, 0);
                        else
                            events.RescheduleEvent(EVENT_ACTIVE, 1000);
                        break;
                    case NPC_ACTIVATED_LASER_TURRET:
                        events.RescheduleEvent(EVENT_ACTIVE, 3000);
                        break;
                    case NPC_ACTIVATED_ELECTROMAGNET:
                        events.RescheduleEvent(EVENT_ACTIVE, 5000);
                        break;
                    case NPC_ACTIVATED_MISSILE_TURRET:
                        events.RescheduleEvent(EVENT_ACTIVE, 4000);
                        break;
                    }
                    break;
                //crawler mine in platform and ready start pursuit
                case 3:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    events.RescheduleEvent(EVENT_PURSUE, 500);
                    break;
                }
            }
        }

        bool IsPlayerRangeddOrHeal(Player* pl)
        {
            switch (pl->getClass())
            {
                case CLASS_PRIEST:
                case CLASS_WARLOCK:
                case CLASS_MAGE:
                case CLASS_HUNTER:
                    return true;
                case CLASS_PALADIN:
                    if (pl->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_PALADIN_HOLY)
                        return true;
                case CLASS_MONK:
                    if (pl->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_MONK_MISTWEAVER)
                        return true;
                case CLASS_SHAMAN:
                    if (pl->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_SHAMAN_ELEMENTAL || pl->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_SHAMAN_RESTORATION)
                        return true;
                case CLASS_DRUID:
                    if (pl->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_DRUID_RESTORATION || pl->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_DRUID_BALANCE)
                        return true;
                default:
                    return false;
            }
            return false;
        }

        void StartDisentegrationLaser()
        {
            if (me->ToTempSummon())
            {
                if (Unit* blackfuse = me->ToTempSummon()->GetSummoner())
                {
                    if (superheat)
                    {
                        float x, y;
                        float dist;
                        for (uint8 n = 2; n < 5; n++)
                        {
                            if (Creature* stalker = blackfuse->SummonCreature(NPC_SHOCKWAVE_MISSILE_STALKER, _centerpos, TEMPSUMMON_TIMED_DESPAWN, 16000 + n*5000))
                            {
                                switch (n)
                                {
                                case 2:
                                    dist = 14.0f;
                                    break;
                                case 3:
                                    dist = 35.0f;
                                    break;
                                case 4:
                                    dist = 55.0f;
                                    break;
                                }
                                GetPosInRadiusWithRandomOrientation(stalker, dist, x, y);
                                if (Creature* laser = blackfuse->SummonCreature(NPC_LASER_TARGET, x, y, stalker->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 120000))
                                {
                                    float ang = stalker->GetAngle(laser);
                                    stalker->SetFacingTo(ang);
                                    stalker->AI()->SetGUID(laser->GetGUID(), n);
                                    laser->AI()->SetGUID(stalker->GetGUID(), n + 3);
                                }
                            }
                        }
                    }
                    else
                    {
                        std::list<Player*>pllist;
                        GetPlayerListInGrid(pllist, me, 150.0f);
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->HasAura(SPELL_PATTERN_RECOGNITION) && !(*itr)->HasAura(SPELL_ON_CONVEYOR) && !(*itr)->HasAura(SPELL_CRAWLER_MINE_FIXATE_PL))
                                {
                                    if (IsPlayerRangeddOrHeal(*itr))
                                    {
                                        if (Creature* laser = blackfuse->SummonCreature(NPC_LASER_TARGET, (*itr)->GetPositionX() + 10.0f, (*itr)->GetPositionY(), blackfuse->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 120000))
                                        {
                                            laser->CastSpell(*itr, SPELL_PURSUIT_LASER, true);
                                            DoCast(laser, SPELL_DISINTEGRATION_LASER_V, true);
                                            laser->AI()->SetGUID(ObjectGuid::Empty, 8);
                                            laser->CastSpell(laser, SPELL_LASER_GROUND_PERIODIC_AT);
                                            laser->AddThreat(*itr, 50000000.0f);
                                            laser->SetReactState(REACT_AGGRESSIVE);
                                            laser->TauntApply(*itr);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_DESPAWN, 16000);//for safe
                    }
                }
            }
        }

        void ActivateElectromagnet()
        {
            DoCast(me, SPELL_MAGNETIC_CRASH_AT);
            std::list<Creature*> sawbladelist;
            GetCreatureListWithEntryInGrid(sawbladelist, me, NPC_BLACKFUSE_SAWBLADE, 200.0f);
            if (!sawbladelist.empty())
            {
                for (std::list<Creature*>::const_iterator itr = sawbladelist.begin(); itr != sawbladelist.end(); itr++)
                {
                    if (!(*itr)->IsOnVehicle()) //filter sawblade-passenger Blackfuse
                    {
                        (*itr)->GetMotionMaster()->Clear(false);
                        (*itr)->GetMotionMaster()->MoveCharge(sbdestpos.GetPositionX(), sbdestpos.GetPositionY(), me->GetPositionZ(), sbspeed, 4);
                        _sawbladelist.push_back((*itr)->GetGUID());
                    }
                }
            }
            events.RescheduleEvent(EVENT_DESPAWN, 30000);
        }

        void ActivateMissileTurret()
        {
            if (me->ToTempSummon())
            {
                if (Unit* blackfuse = me->ToTempSummon()->GetSummoner())
                {
                    if (Creature* stalker = me->GetCreature(*me, instance->GetGuidData(NPC_SHOCKWAVE_MISSILE_STALKER)))
                    {
                        if (superheat)
                        {
                            if (!missilecount)
                            {
                                float ang1 = stalker->GetAngle(blackfuse);
                                if (!ang1 || (ang1 - modang) < 0)
                                    modang = 6.0f - modang;
                                else
                                    modang = ang1 - modang;
                            }
                            stalker->SetFacingTo(modang);
                            float x, y;
                            GetPositionWithDistInOrientation(stalker, 56, modang, x, y);
                            if (Creature* mt = blackfuse->SummonCreature(NPC_SHOCKWAVE_MISSILE, x, y, stalker->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                mt->SetFacingToObject(stalker);
                                DoCast(mt, SPELL_SHOCKWAVE_VISUAL_TURRET);
                                modang -= 0.7f;
                                missilecount++;
                                if (missilecount == 3)
                                    mt->AI()->SetGUID(me->GetGUID(), 1);
                                else
                                    events.RescheduleEvent(EVENT_SHOCKWAVE_MISSILE, 10000);
                            }
                        }
                        else
                        {
                            float x, y;
                            GetPosInRadiusWithRandomOrientation(stalker, 56.0f, x, y);
                            if (Creature* mt = blackfuse->SummonCreature(NPC_SHOCKWAVE_MISSILE, x, y, stalker->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                mt->SetFacingToObject(stalker);
                                DoCast(mt, SPELL_SHOCKWAVE_VISUAL_TURRET);
                                mt->AI()->SetGUID(me->GetGUID(), 1);
                            }
                        }
                    }
                }
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_DESPAWN)
            {
                events.Reset();
                me->RemoveAurasDueToSpell(SPELL_DISINTEGRATION_LASER_V);
                events.RescheduleEvent(EVENT_DESPAWN, 2000);
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE:
                {
                    switch (me->GetEntry())
                    {
                    case NPC_BLACKFUSE_CRAWLER_MINE:
                        me->GetMotionMaster()->MoveJump(cmdestpos.GetPositionX(), cmdestpos.GetPositionY(), cmdestpos.GetPositionZ(), 20.0f, 20.0f, 3);
                        break;
                    case NPC_ACTIVATED_LASER_TURRET:
                        StartDisentegrationLaser();
                        break;
                    case NPC_ACTIVATED_ELECTROMAGNET:
                        ActivateElectromagnet();
                        break;
                    case NPC_ACTIVATED_MISSILE_TURRET:
                        ActivateMissileTurret();
                        break;
                    }
                }
                break;
                case EVENT_PURSUE:
                {
                    if (Player* pl = me->GetPlayer(*me, targetGuid))
                        if (pl->isAlive())
                            pl->RemoveAurasDueToSpell(SPELL_CRAWLER_MINE_FIXATE_PL);

                    std::list<Player*>pllist;
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->isInTankSpec())
                            {
                                if (!(*itr)->HasAura(SPELL_ON_CONVEYOR) && !(*itr)->HasAura(SPELL_PATTERN_RECOGNITION) && !(*itr)->HasAura(SPELL_PURSUIT_LASER) && !(*itr)->HasAura(SPELL_CRAWLER_MINE_FIXATE_PL))
                                {
                                    (*itr)->AddAura(SPELL_CRAWLER_MINE_FIXATE_PL, *itr);
                                    DoCast(*itr, SPELL_CRAWLER_MINE_FIXATE, true);
                                    targetGuid = (*itr)->GetGUID();
                                    DoCast(me, SPELL_BREAK_IN_PERIOD, true);
                                    me->AddThreat(*itr, 50000000.0f);
                                    me->SetReactState(REACT_AGGRESSIVE);
                                    me->TauntApply(*itr);
                                    break;
                                }
                            }
                        }
                    }
                    if (!targetGuid)
                        events.RescheduleEvent(EVENT_PURSUE, 1000);
                    else
                        events.RescheduleEvent(EVENT_CHECK_DISTANCE, 1000);
                }
                break;
                case EVENT_CHECK_DISTANCE:
                {
                    Player* pl = me->GetPlayer(*me, targetGuid);
                    if (pl && pl->isAlive() && !pl->HasAura(SPELL_ON_CONVEYOR))
                    {
                        if (IsInControl())
                        {
                            events.RescheduleEvent(EVENT_CHECK_DISTANCE, 1000);
                            return;
                        }

                        if (!pl->HasAura(SPELL_CRAWLER_MINE_FIXATE))
                            DoCast(pl, SPELL_CRAWLER_MINE_FIXATE, true);

                        if (me->GetDistance(pl) <= 6.0f && !done)
                        {
                            done = true;
                            pl->RemoveAurasDueToSpell(SPELL_CRAWLER_MINE_FIXATE_PL);
                            me->GetMotionMaster()->Clear(false);
                            DoCast(pl, SPELL_DETONATE, true);
                            me->DespawnOrUnsummon(1000);
                            return;
                        }
                    }
                    else
                    {
                        me->StopAttack();
                        events.RescheduleEvent(EVENT_PURSUE, 1000);
                        return;
                    }
                    events.RescheduleEvent(EVENT_CHECK_DISTANCE, 1000);
                }
                break;
                case EVENT_DESPAWN:
                {
                    if (me->GetEntry() == NPC_ACTIVATED_ELECTROMAGNET)
                    {
                        if (Creature* electromagnet = me->FindNearestCreature(NPC_OVERCHARGED_ELECTROMAGNET, 200.0f, true))
                            electromagnet->DespawnOrUnsummon();
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGNETIC_CRASH_DMG);
                        for (std::vector<ObjectGuid>::const_iterator itr = _sawbladelist.begin(); itr != _sawbladelist.end(); ++itr)
                            if (Creature* sawblade = me->GetCreature(*me, *itr))
                                sawblade->DespawnOrUnsummon();
                    }
                    me->DespawnOrUnsummon();
                }
                break;
                case EVENT_SHOCKWAVE_MISSILE:
                    ActivateMissileTurret();
                    break;
                case EVENT_LAUNCH_FORWARD:
                    me->RemoveAurasDueToSpell(SPELL_MAGNETIC_CRASH_AT);
                    if (AreaTrigger* at = me->GetAreaObject(SPELL_MAGNETIC_CRASH_AT))
                        at->Despawn();
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGNETIC_CRASH_DMG);
                    if (Creature* oelectromagnet = me->SummonCreature(NPC_OVERCHARGED_ELECTROMAGNET, oelectromagnetpos))
                    {
                        oelectromagnet->SetReactState(REACT_PASSIVE);
                        oelectromagnet->setFaction(16);
                        oelectromagnet->SetDisplayId(11686);
                        oelectromagnet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        oelectromagnet->CastSpell(oelectromagnet, SPELL_MAGNETIC_CRASH_AT);
                        oelectromagnetGuid = oelectromagnet->GetGUID();
                    }
                    break;
                case EVENT_LAUNCH_BACK:
                    if (Creature* oelectromagnet = me->GetCreature(*me, oelectromagnetGuid))
                        oelectromagnet->DespawnOrUnsummon();
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGNETIC_CRASH_DMG);
                    DoCast(me, SPELL_MAGNETIC_CRASH_AT);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackfuse_weaponAI(creature);
    }
};

//71910, 71740, 72710, 72052, 71520
class npc_blackfuse_trigger : public CreatureScript
{
public:
    npc_blackfuse_trigger() : CreatureScript("npc_blackfuse_trigger") {}

    struct npc_blackfuse_triggerAI : public ScriptedAI
    {
        npc_blackfuse_triggerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            if (me->GetEntry() == NPC_LASER_ARRAY && !me->ToTempSummon())
            {
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->AddAura(SPELL_CONVEYOR_DEATH_BEAM_V, me);
                DoCast(me, SPELL_CONVEYOR_DEATH_BEAM_AT, true);
            }
            superheat = false;
            
        }
        InstanceScript* instance;
        EventMap events;
        ObjectGuid cannonGuid, laserGuid, stalkerGuid, turretGuid;
        uint32 rotatespeed;
        float dist, speed;
        bool superheat;
        uint8 num;

        void Reset()
        {
            events.Reset();
            num = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!me->GetMap()->IsHeroic())
                damage = 0;
            else if (me->GetMap()->IsHeroic() && me->GetEntry() != NPC_SHOCKWAVE_MISSILE)
                damage = 0;
        }

        void SpellHit(Unit* caster, SpellInfo const *spell)
        {
            if (me->GetEntry() == NPC_SHOCKWAVE_MISSILE && spell->Id == SPELL_SHOCKWAVE_MISSILE_T_M)
            {
                turretGuid = caster->GetGUID();
                CreateShockWaveMissileEvent();
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_DESPAWN)
                if (!superheat)
                    me->DespawnOrUnsummon(1000);
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            switch (id)
            {
            //HM ShockWave Missile
            case 1:
                cannonGuid = guid;
                break;
            //HM Disentegration Laser
            //stalker proc
            case 2:
                dist = 14.0f;
                speed = 6.0f;
                rotatespeed = 14000;
                laserGuid = guid;
                events.RescheduleEvent(EVENT_START_ROTATE, 1000);
                break;
            case 3:
                dist = 35.0f;
                speed = 16.0f;
                rotatespeed = 24000;
                laserGuid = guid;
                events.RescheduleEvent(EVENT_START_ROTATE, 1000);
                break;
            case 4:
                dist = 55.0f;
                speed = 26.0f;
                rotatespeed = 34000;
                laserGuid = guid;
                events.RescheduleEvent(EVENT_START_ROTATE, 1000);
                break;
            //laser proc
            case 5:
                dist = 14.0f;
                speed = 6.0f;
                stalkerGuid = guid;
                events.RescheduleEvent(EVENT_DESPAWN, 16000);
                break;
            case 6:
                dist = 35.0f;
                speed = 10.0f;
                stalkerGuid = guid;
                events.RescheduleEvent(EVENT_DESPAWN, 26000);
                break;
            case 7:
                dist = 55.0f;
                speed = 15.0f;
                stalkerGuid = guid;
                events.RescheduleEvent(EVENT_DESPAWN, 36000);
                break;
            //Normal Disentegration Laser
            case 8:
                events.RescheduleEvent(EVENT_DESPAWN, 15000);
                break;
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE || type == POINT_MOTION_TYPE)
                if (pointId == 1)
                    events.RescheduleEvent(EVENT_LASER_ROTATE2, 10);
        }

        void CreateShockWaveMissileEvent()
        {
            if (me->ToTempSummon())
            {
                if (Unit* blackfuse = me->ToTempSummon()->GetSummoner())
                {
                    if (Creature* turret = me->GetCreature(*me, turretGuid))
                    {
                        if (turret->AI()->GetData(DATA_ACTIVE_SUPERHEAT))
                        {
                            superheat = true;
                            DoCast(me, SPELL_SHOCKWAVE_VISUAL_SPAWN_HM, true);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        }
                        else
                            DoCast(me, SPELL_SHOCKWAVE_VISUAL_SPAWN, true);
                        DoCast(me, shockwavemissilelist[num++]);
                        events.RescheduleEvent(EVENT_SHOCKWAVE_MISSILE, 3500);
                    }
                }
                if (Creature* cannon = me->GetCreature(*me, cannonGuid))
                    cannon->DespawnOrUnsummon();
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_DESPAWN)
            {
                events.Reset();
                me->RemoveAurasDueToSpell(SPELL_DISINTEGRATION_LASER_V);
                events.RescheduleEvent(EVENT_DESPAWN, 2000);
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SHOCKWAVE_MISSILE:
                    DoCast(me, shockwavemissilelist[num++]);
                    if (num < 6)
                        events.RescheduleEvent(EVENT_SHOCKWAVE_MISSILE, 3500);
                    else if (num >= 6 && superheat)
                    {
                        num = 0;
                        events.RescheduleEvent(EVENT_SHOCKWAVE_MISSILE, 3500);
                    }
                    break;
                case EVENT_START_ROTATE: //from stalker(start)
                    if (Creature* lturret = me->GetCreature(*me, instance->GetGuidData(NPC_ACTIVATED_LASER_TURRET)))
                    {
                        if (Creature* laser = me->GetCreature(*me, laserGuid))
                        {
                            if (dist != 14.0f)
                            {
                                Position pos;
                                lturret->GetPosition(&pos);
                                if (Creature* blackfuse = me->GetCreature(*me, instance->GetGuidData(NPC_BLACKFUSE_MAUNT)))
                                {
                                    if (Creature* _lturret = blackfuse->SummonCreature(NPC_ACTIVATED_LASER_TURRET, pos))
                                    {
                                        _lturret->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                        _lturret->CastSpell(laser, SPELL_DISINTEGRATION_LASER_V, true);
                                    }
                                }
                            }
                            else
                                lturret->CastSpell(laser, SPELL_DISINTEGRATION_LASER_V, true);
                            laser->CastSpell(laser, SPELL_LASER_GROUND_PERIODIC_AT);
                        }
                    }
                    me->GetMotionMaster()->MoveRotate(rotatespeed, ROTATE_DIRECTION_RIGHT);
                    events.RescheduleEvent(EVENT_LASER_ROTATE, 500);
                    break;
                case EVENT_LASER_ROTATE: //from stalker(first point)
                    if (Creature* laser = me->GetCreature(*me, laserGuid))
                    {
                        float x, y;
                        GetPositionWithDistInOrientation(me, dist, me->GetOrientation(), x, y);
                        laser->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), speed, 1);
                    }
                    break;
                case EVENT_LASER_ROTATE2: //from laser(after finilize point)
                    if (Creature* stalker = me->GetCreature(*me, stalkerGuid))
                    {
                        float x, y;
                        GetPositionWithDistInOrientation(stalker, dist, stalker->GetOrientation(), x, y);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), speed, 1);
                    }
                    break;
                case EVENT_DESPAWN: //from laser
                    me->RemoveAurasDueToSpell(SPELL_DISINTEGRATION_LASER_V);
                    me->RemoveAurasDueToSpell(SPELL_LASER_GROUND_PERIODIC_AT);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackfuse_triggerAI(creature);
    }
};

//143265
class spell_blackfuse_launch_sawblade : public SpellScriptLoader
{
public:
    spell_blackfuse_launch_sawblade() : SpellScriptLoader("spell_blackfuse_launch_sawblade") { }

    class spell_blackfuse_launch_sawblade_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_blackfuse_launch_sawblade_SpellScript);

        void HandleHit()
        {
            if (GetCaster() && GetHitUnit())
            { 
                if (GetHitUnit()->ToPlayer())
                {
                    Position pos;
                    GetCaster()->GetNearPosition(pos, 7.0f, 5.5f);
                    if (Creature* sawblade = GetCaster()->SummonCreature(NPC_BLACKFUSE_SAWBLADE, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 1.0f, 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        sawblade->AddAura(SPELL_LAUNCH_SAWBLADE_AT, sawblade);
                        sawblade->GetMotionMaster()->MoveCharge(GetHitUnit()->GetPositionX(), GetHitUnit()->GetPositionY(), GetCaster()->GetPositionZ() + 1.0f, 25.0f);
                    }
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_blackfuse_launch_sawblade_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_blackfuse_launch_sawblade_SpellScript();
    }
};

//145269
class spell_break_in_period : public SpellScriptLoader
{
public:
    spell_break_in_period() : SpellScriptLoader("spell_break_in_period") { }

    class spell_break_in_period_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_break_in_period_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                GetTarget()->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                GetTarget()->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                GetTarget()->CastSpell(GetTarget(), SPELL_READY_TO_GO);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_break_in_period_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_break_in_period_AuraScript();
    }
};

//143867
class spell_disintegration_laser : public SpellScriptLoader
{
public:
    spell_disintegration_laser() : SpellScriptLoader("spell_disintegration_laser") { }

    class spell_disintegration_laser_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_disintegration_laser_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                if (GetTarget()->ToCreature())
                {
                    GetTarget()->ToCreature()->StopAttack();
                    if (GetCaster() && GetCaster()->ToCreature())
                        GetCaster()->ToCreature()->DespawnOrUnsummon();
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_disintegration_laser_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_disintegration_laser_AuraScript();
    }
};

//144210
class spell_death_from_above : public SpellScriptLoader
{
public:
    spell_death_from_above() : SpellScriptLoader("spell_death_from_above") { }

    class spell_death_from_above_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_death_from_above_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
                if (GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->SetReactState(REACT_AGGRESSIVE);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_death_from_above_AuraScript::HandleEffectRemove, EFFECT_2, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_death_from_above_AuraScript();
    }
};

class ShockWaveMissiletFilterTarget
{
public:
    ShockWaveMissiletFilterTarget(WorldObject* caster, float mindist, float maxdist) : _caster(caster),  _mindist(mindist), _maxdist(maxdist){}

    bool operator()(WorldObject* unit)
    {
        if (_caster->GetExactDist2d(unit) > _mindist && _caster->GetExactDist2d(unit) < _maxdist)
            return false;
        return true;
    }
private:
    WorldObject* _caster;
    float _mindist, _maxdist;
};

class ShockWaveMissiletFilterTarget2
{
public:
    ShockWaveMissiletFilterTarget2(WorldObject* caster, float mindist, float maxdist) : _caster(caster), _mindist(mindist), _maxdist(maxdist){}

    bool operator()(WorldObject* unit)
    {
        if (unit->ToCreature() && unit->GetEntry() == NPC_AUTOMATED_SHREDDER)
            if (_caster->GetExactDist2d(unit) > _mindist && _caster->GetExactDist2d(unit) < _maxdist)
                return false;
        return true;
    }
private:
    WorldObject* _caster;
    float _mindist, _maxdist;
};

//144660, 144661, 144662, 144663, 144664
class spell_shockwave_missile : public SpellScriptLoader
{
public:
    spell_shockwave_missile() : SpellScriptLoader("spell_shockwave_missile") { }

    class spell_shockwave_missile_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shockwave_missile_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (GetCaster() && !targets.empty())
            {
                switch (GetSpellInfo()->Id)
                {
                case SPELL_SHOCKWAVE_MISSILE2:
                    targets.remove_if(ShockWaveMissiletFilterTarget(GetCaster(), 10.0f, 25.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE3:
                    targets.remove_if(ShockWaveMissiletFilterTarget(GetCaster(), 25.0f, 45.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE4:
                    targets.remove_if(ShockWaveMissiletFilterTarget(GetCaster(), 45.0f, 65.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE5:
                    targets.remove_if(ShockWaveMissiletFilterTarget(GetCaster(), 65.0f, 85.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE6:
                    targets.remove_if(ShockWaveMissiletFilterTarget(GetCaster(), 85.0f, 105.0f));
                    break;
                }
            }
        }

        void FilterTargets2(std::list<WorldObject*>& targets)
        {
            if (GetCaster() && !targets.empty())
            {
                switch (GetSpellInfo()->Id)
                {
                case SPELL_SHOCKWAVE_MISSILE2:
                    targets.remove_if(ShockWaveMissiletFilterTarget2(GetCaster(), 10.0f, 25.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE3:
                    targets.remove_if(ShockWaveMissiletFilterTarget2(GetCaster(), 25.0f, 45.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE4:
                    targets.remove_if(ShockWaveMissiletFilterTarget2(GetCaster(), 45.0f, 65.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE5:
                    targets.remove_if(ShockWaveMissiletFilterTarget2(GetCaster(), 65.0f, 85.0f));
                    break;
                case SPELL_SHOCKWAVE_MISSILE6:
                    targets.remove_if(ShockWaveMissiletFilterTarget2(GetCaster(), 85.0f, 105.0f));
                    break;
                }
            }
        }

        void HandleAfterCast()
        {
            if (GetSpellInfo()->Id == SPELL_SHOCKWAVE_MISSILE6)
                if (GetCaster() && GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->SetData(DATA_DESPAWN, 0);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shockwave_missile_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shockwave_missile_SpellScript::FilterTargets2, EFFECT_2, TARGET_UNIT_SRC_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shockwave_missile_SpellScript::FilterTargets, EFFECT_3, TARGET_UNIT_DEST_AREA_ENEMY);
            AfterCast += SpellCastFn(spell_shockwave_missile_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_shockwave_missile_SpellScript();
    }
};

class CMExploseFilterTarget
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (unit->ToPlayer())
            return false;
        else if (unit->ToCreature() && unit->GetEntry() == NPC_AUTOMATED_SHREDDER)
            return false;
        
        return true;
    }
};

//149146
class spell_blacksue_cm_explose : public SpellScriptLoader
{
public:
    spell_blacksue_cm_explose() : SpellScriptLoader("spell_blacksue_cm_explose") { }

    class spell_blacksue_cm_explose_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_blacksue_cm_explose_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(CMExploseFilterTarget());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_blacksue_cm_explose_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_blacksue_cm_explose_SpellScript();
    }
};

//143641, 144658
class spell_shockwave_missiles_tm : public SpellScriptLoader
{
public:
    spell_shockwave_missiles_tm() : SpellScriptLoader("spell_shockwave_missiles_tm") { }

    class spell_shockwave_missiles_tm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shockwave_missiles_tm_SpellScript);

        void RecalculateDamage()
        {
            if (GetHitUnit() && GetHitUnit()->ToCreature())
                SetHitDamage(0);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_shockwave_missiles_tm_SpellScript::RecalculateDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_shockwave_missiles_tm_SpellScript();
    }
};

//144287
class spell_on_conveyor : public SpellScriptLoader
{
public:
    spell_on_conveyor() : SpellScriptLoader("spell_on_conveyor") { }

    class spell_on_conveyor_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_on_conveyor_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->ToPlayer()) //only players
            {
                GetTarget()->RemoveAurasDueToSpell(SPELL_MAGNETIC_CRASH_DMG);

                //Protect mechanic
                if (Player* plr = GetTarget()->ToPlayer())
                {
                    if (plr->getClass() != CLASS_HUNTER && plr->getClass() != CLASS_MONK && !plr->HasAura(SPELL_PATTERN_RECOGNITION))
                    {
                        plr->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
                        plr->RemoveAurasDueToSpell(SPELL_ON_CONVEYOR);
                        plr->GetMotionMaster()->MoveJump(1983.22f, -5559.18f, -309.3264f, 20.0f, 20.0f, 145351);
                    }
                    else if (plr->getClass() != CLASS_HUNTER && plr->getClass() != CLASS_MONK && plr->HasAura(SPELL_PATTERN_RECOGNITION))
                    {
                        if (Aura* aura = plr->GetAura(SPELL_PATTERN_RECOGNITION))
                        {
                            if (aura->GetDuration() <= 55000)
                            {
                                plr->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
                                plr->RemoveAurasDueToSpell(SPELL_ON_CONVEYOR);
                                plr->GetMotionMaster()->MoveJump(1983.22f, -5559.18f, -309.3264f, 20.0f, 20.0f, 145351);
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_on_conveyor_AuraScript::OnApply, EFFECT_0, SPELL_AURA_INTERFERE_TARGETTING, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_on_conveyor_AuraScript();
    }
};

//9250, 9251, 9252, 9253, 9371, 9240, 9189, 9190, 9493, 9194, 9238, 9239, 
class at_blackfuse_pipe : public AreaTriggerScript
{
public:
    at_blackfuse_pipe() : AreaTriggerScript("at_blackfuse_pipe") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger, bool enter)
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (enter)
            {
                switch (areaTrigger->ID)
                {
                    case ENTER_1:
                    case ENTER_2:
                    case ENTER_3:
                        if (instance->GetBossState(DATA_BLACKFUSE) != IN_PROGRESS)
                            player->NearTeleportTo(atdestpos[0].GetPositionX(), atdestpos[0].GetPositionY(), atdestpos[0].GetPositionZ(), atdestpos[0].GetOrientation());
                        else
                            player->GetMotionMaster()->MoveJump(atdestpos[1].GetPositionX(), atdestpos[1].GetPositionY(), atdestpos[1].GetPositionZ(), 15.0f, 15.0f);
                        break;
                    case LEAVE_1:
                    case LEAVE_2:
                    case LEAVE_3:
                        if (instance->GetBossState(DATA_BLACKFUSE) != IN_PROGRESS)
                            player->NearTeleportTo(atdestpos[1].GetPositionX(), atdestpos[1].GetPositionY(), atdestpos[1].GetPositionZ(), atdestpos[1].GetOrientation());
                        else
                        {
                            if (Creature* blackfuse = instance->instance->GetCreature(instance->GetGuidData(NPC_BLACKFUSE_MAUNT)))
                                blackfuse->CastSpell(player, SPELL_MAGNETIC_LASSO_VISUAL, true);
                            player->GetMotionMaster()->MoveJump(atdestpos[0].GetPositionX(), atdestpos[0].GetPositionY(), atdestpos[0].GetPositionZ(), 15.0f, 15.0f);
                        }
                        break;
                    case ENTER_CONVEYOR:
                    case ENTER_CONVEYOR_2:
                        if (instance->GetBossState(DATA_BLACKFUSE) == IN_PROGRESS)
                        {
                            if (!player->HasAura(SPELL_PATTERN_RECOGNITION) && !player->HasAura(SPELL_PURSUIT_LASER) && !player->HasAura(SPELL_ON_CONVEYOR) && !player->HasAura(SPELL_CRAWLER_MINE_FIXATE_PL))
                            {
                                player->CastSpell(player, SPELL_ON_CONVEYOR, true);
                                player->CastSpell(player, SPELL_PATTERN_RECOGNITION, true);
                                player->NearTeleportTo(atdestpos[2].GetPositionX(), atdestpos[2].GetPositionY(), atdestpos[2].GetPositionZ(), atdestpos[2].GetOrientation());
                            }
                            else
                            {
                                if (Creature* blackfuse = instance->instance->GetCreature(instance->GetGuidData(NPC_BLACKFUSE_MAUNT)))
                                    blackfuse->CastSpell(player, SPELL_MAGNETIC_LASSO_VISUAL, true);
                                player->GetMotionMaster()->MoveJump(droppos.GetPositionX(), droppos.GetPositionY(), droppos.GetPositionZ(), 15.0f, 15.0f);
                            }
                        }
                        else
                            player->GetMotionMaster()->MoveJump(droppos.GetPositionX(), droppos.GetPositionY(), droppos.GetPositionZ(), 15.0f, 15.0f);
                        break;
                    case LEAVE_CONVEYOR:
                    case LEAVE_CONVEYOR_2:
                    case LEAVE_CONVEYOR_3:
                    case LEAVE_CONVEYOR_4:
                        player->RemoveAurasDueToSpell(SPELL_ON_CONVEYOR);
                        player->NearTeleportTo(atdestpos[3].GetPositionX(), atdestpos[3].GetPositionY(), atdestpos[3].GetPositionZ(), atdestpos[3].GetOrientation());
                        break;
                }
            }
        }
        return true;
    }
};

void AddSC_boss_siegecrafter_blackfuse()
{
    new boss_siegecrafter_blackfuse();
    new npc_blackfuse_passenger();
    new npc_automated_shredder();
    new npc_blackfuse_weapon();
    new npc_blackfuse_trigger();
    new spell_blackfuse_launch_sawblade();
    new spell_break_in_period();
    new spell_disintegration_laser();
    new spell_death_from_above();
    new spell_shockwave_missile();
    new spell_blacksue_cm_explose();
    new spell_shockwave_missiles_tm();
    new spell_on_conveyor();
    new at_blackfuse_pipe();
}
