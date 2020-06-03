/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "pit_of_saron.h"

enum eSpells
{
    SPELL_FIREBALL              = 69583, //Ymirjar Flamebearer
    SPELL_HELLFIRE              = 69586,
    SPELL_TACTICAL_BLINK        = 69584,
    SPELL_FROST_BREATH          = 69527, //Iceborn Proto-Drake
    SPELL_BLINDING_DIRT         = 70302, //Wrathbone Laborer
    SPELL_PUNCTURE_WOUND        = 70278,
    SPELL_SHOVELLED             = 69572,
    SPELL_LEAPING_FACE_MAUL     = 69504, // Geist Ambusher
    SPELL_NECROMANTIC_POWER     = 32889,
};

enum eEvents
{
    // Ymirjar Flamebearer
    EVENT_FIREBALL              = 1,
    EVENT_TACTICAL_BLINK        = 2,

    //Wrathbone Laborer
    EVENT_BLINDING_DIRT         = 3,
    EVENT_PUNCTURE_WOUND        = 4,
    EVENT_SHOVELLED             = 5,
    TYRANNYS_GUIDS                = 0,
};

class mob_ymirjar_flamebearer : public CreatureScript
{
    public:
        mob_ymirjar_flamebearer() : CreatureScript("mob_ymirjar_flamebearer") { }

        struct mob_ymirjar_flamebearerAI: public ScriptedAI
        {
            mob_ymirjar_flamebearerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset() override
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _events.ScheduleEvent(EVENT_FIREBALL, 4000);
                _events.ScheduleEvent(EVENT_TACTICAL_BLINK, 15000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIREBALL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_FIREBALL);
                            _events.RescheduleEvent(EVENT_FIREBALL, 5000);
                            break;
                        case EVENT_TACTICAL_BLINK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_TACTICAL_BLINK);
                            DoCast(me, SPELL_HELLFIRE);
                            _events.RescheduleEvent(EVENT_TACTICAL_BLINK, 12000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_ymirjar_flamebearerAI(creature);
        }
};

class mob_iceborn_protodrake : public CreatureScript
{
    public:
        mob_iceborn_protodrake() : CreatureScript("mob_iceborn_protodrake") { }

        struct mob_iceborn_protodrakeAI: public ScriptedAI
        {
            mob_iceborn_protodrakeAI(Creature *creature) : ScriptedAI(creature), _vehicle(creature->GetVehicleKit())
            {
                //ASSERT(_vehicle);
            }

            void Reset() override
            {
                _frostBreathCooldown = 5000;
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _vehicle->RemoveAllPassengers();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (_frostBreathCooldown < diff)
                {
                    DoCastVictim(SPELL_FROST_BREATH);
                    _frostBreathCooldown = 10000;
                }
                else
                    _frostBreathCooldown -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            Vehicle* _vehicle;
            uint32 _frostBreathCooldown;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_iceborn_protodrakeAI(creature);
        }
};

class mob_wrathbone_laborer : public CreatureScript
{
    public:
        mob_wrathbone_laborer() : CreatureScript("mob_wrathbone_laborer") { }

        struct mob_wrathbone_laborerAI: public ScriptedAI
        {
            mob_wrathbone_laborerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset() override
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _events.ScheduleEvent(EVENT_BLINDING_DIRT, 8000);
                _events.ScheduleEvent(EVENT_PUNCTURE_WOUND, 9000);
                _events.ScheduleEvent(EVENT_SHOVELLED, 5000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_BLINDING_DIRT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                                DoCast(target, SPELL_BLINDING_DIRT);
                            _events.RescheduleEvent(EVENT_BLINDING_DIRT, 10000);
                            return;
                        case EVENT_PUNCTURE_WOUND:
                            DoCastVictim(SPELL_PUNCTURE_WOUND);
                            _events.RescheduleEvent(EVENT_PUNCTURE_WOUND, 9000);
                            return;
                        case EVENT_SHOVELLED:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -5.0f))
                                DoCast(target, SPELL_SHOVELLED);
                            _events.RescheduleEvent(EVENT_SHOVELLED, 7000);
                            return;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_wrathbone_laborerAI(creature);
        }
};

class mob_geist_ambusher : public CreatureScript
{
    public:
        mob_geist_ambusher() : CreatureScript("mob_geist_ambusher") { }

        struct mob_geist_ambusherAI: public ScriptedAI
        {
            mob_geist_ambusherAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset() override
            {
                _leapingFaceMaulCooldown = 9000;
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (me->IsWithinDistInMap(who, 30.0f))
                    DoCast(who, SPELL_LEAPING_FACE_MAUL);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (_leapingFaceMaulCooldown < diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 5.0f, true))
                        DoCast(target, SPELL_LEAPING_FACE_MAUL);
                    _leapingFaceMaulCooldown = urand(9000, 14000);
                }
                else
                    _leapingFaceMaulCooldown -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            uint32 _leapingFaceMaulCooldown;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_geist_ambusherAI(creature);
        }
};

class spell_trash_mob_glacial_strike : public SpellScriptLoader
{
    public:
        spell_trash_mob_glacial_strike() : SpellScriptLoader("spell_trash_mob_glacial_strike") { }

        class spell_trash_mob_glacial_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_trash_mob_glacial_strike_AuraScript);

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (GetTarget()->IsFullHealth())
                {
                    GetTarget()->RemoveAura(GetId(), ObjectGuid::Empty, 0U, AURA_REMOVE_BY_ENEMY_SPELL);
                    PreventDefaultAction();
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_trash_mob_glacial_strike_AuraScript::PeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_trash_mob_glacial_strike_AuraScript();
        }
};

struct LocationsXY
{
    float x, y, z, o;
    uint32 id;
};

static LocationsXY SummonLoc[]=
{
    {428.344f, 209.151f, 528.708f}, // right
    {429.887f, 212.632f, 528.708f}, // midle
    {428.505f, 215.999f, 528.708f}, // left
};

static LocationsXY SummonLocOutro[]=
{
    {1060.955f, 107.274f, 628.424f},
    {1052.122f, 103.916f, 628.454f},
    {1068.363f, 110.432f, 629.009f},
};

static LocationsXY MoveallLoc[]=
{
    {436.146f, 197.076f, 530.275f}, // right 1
    {437.490f, 205.392f, 528.739f}, // right 2
    {443.415f, 207.468f, 528.706f}, // right 2
    {443.498f, 210.536f, 528.708f}, // midle 1
    {446.442f, 213.136f, 528.710f}, // midle 2
    {443.738f, 217.716f, 528.708f}, // midle 3
    {439.485f, 221.760f, 528.708f}, // left 1
    {440.447f, 227.049f, 528.708f}, // left 2
    {432.713f, 230.449f, 528.706f}, // left 3
};

static LocationsXY MoveLoc[]=
{
    {485.361f, 210.737f, 528.710f}, // right 1
    {483.985f, 223.756f, 528.710f}, // midle 1
    {485.593f, 231.117f, 528.710f}, // left 1
};

static LocationsXY MoveLocOutro[]=
{
    {1019.006f, 129.684f, 628.156f}, 
    {1003.889f, 159.652f, 628.159f},
    {1015.389f, 183.650f, 628.156f},
    {1065.827f, 210.836f, 628.156f},
    {1072.659f, 204.432f, 628.156f},
};

static LocationsXY SummonSkull[]=
{
    {485.173f, 219.414f, 528.709f}, // summons undead and moveLoc 2 points for champions
};

enum {
SAY_TYRANNUS1           = -1658090,
SAY_TYRANNUS2           = -1658093,
SAY_TYRANNUS3           = -1658094,
SAY_TYRANNUS4            = -1658095,
SAY_TYRANNUS5            = -1658098,

NPC_TYRANNUS_INTRO              = 36794,
NPC_CHAMPION_3_ALLIANCE         = 37498,
NPC_SKELETAL_SLAVE              = 36881,
NPC_TYRANNUS_DELL              = 36658,


SPELL_RESURECT          = 32343,
SPELL_NECROTIC          = 55719,
SPELL_STRANGULATING     = 69413, // not support 70569...
SPELL_FLY               = 59553,
SPELL_FREEZE            = 70132,
SPELL_SHOT                = 38384,


SAY_SPEECH_SYLVANAS1    = -1658091,
SAY_SPEECH_JAINA1       = -1658092,
SAY_SPEECH_JAINA2       = -1658096,
SAY_SPEECH_SYLVANAS2    = -1658097,
SAY_SPEECH_JAINA3        = -1658099,
SAY_SPEECH_JAINA4        = -1658100,
SAY_SPEECH_SYLVANAS3    = -1658101,
SAY_SPEECH_JAINA5        = -1658102,
SAY_SPEECH_SYLVANAS4    = -1658103,
};


class npc_jaina_POSintro : public CreatureScript
{
public:
    npc_jaina_POSintro() : CreatureScript("npc_jaina_POSintro") { }

    struct npc_jaina_POSintroAI: public ScriptedAI
    {
        npc_jaina_POSintroAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 m_uiSpeech_TimerAlliance;
        ObjectGuid m_uiTyrannusGuid;
        ObjectGuid m_uiMageLeaftGuid;
        ObjectGuid m_uiMageRightGuid;
        ObjectGuid m_uiSkeletGuid[12];
        ObjectGuid muiCastNpcStart;
        ObjectGuid m_uiDeathGuid;
        uint8 m_uiIntro_Phase;
        bool m_bIsIntro;
        bool m_bIsIntroEnd;
        ObjectGuid m_uiChampAlianceGuid[12];
        uint32 creatureEntry;

        void Reset() override
        {
            m_uiIntro_Phase = 0;
            m_uiSpeech_TimerAlliance = 1000;
            m_uiTyrannusGuid.Clear();
            m_uiMageLeaftGuid.Clear();
            m_uiMageRightGuid.Clear();
            muiCastNpcStart.Clear();
            m_uiDeathGuid.Clear();
            m_bIsIntro = false;
            m_bIsIntroEnd = false;

            for (int32 i = 0; i < 13; ++i)
            {
                m_uiChampAlianceGuid[i].Clear();
                m_uiSkeletGuid[i].Clear();
            }
            creatureEntry = me->GetEntry();
        }

        void SummonAlyChampions01()
        {
            if (Creature* pChampAliance01 = me->SummonCreature(NPC_CHAMPION_1_ALLIANCE, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_TIMED_DESPAWN, 37800))
            {
                pChampAliance01->GetMotionMaster()->MovePoint(0, MoveallLoc[0].x, MoveallLoc[0].y, MoveallLoc[0].z);
                m_uiChampAlianceGuid[0] = pChampAliance01->GetGUID();
            }
            if (Creature* pChampAliance02 = me->SummonCreature(NPC_CHAMPION_1_ALLIANCE, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_TIMED_DESPAWN, 37800))
            {
                pChampAliance02->GetMotionMaster()->MovePoint(0, MoveallLoc[1].x, MoveallLoc[1].y, MoveallLoc[1].z);
                m_uiChampAlianceGuid[1] = pChampAliance02->GetGUID();
            }
            if (Creature* pChampAliance03 = me->SummonCreature(NPC_CHAMPION_1_ALLIANCE, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_TIMED_DESPAWN, 37800))
            {
                pChampAliance03->GetMotionMaster()->MovePoint(0, MoveallLoc[2].x, MoveallLoc[2].y, MoveallLoc[2].z);
                m_uiChampAlianceGuid[2] = pChampAliance03->GetGUID();
            }
        }

        void SummonAlyChampions02()
        {
            if (Creature* pChampAliance04 = me->SummonCreature(NPC_CHAMPION_2_ALLIANCE, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_TIMED_DESPAWN, 32000))
            {
                pChampAliance04->GetMotionMaster()->MovePoint(0, MoveallLoc[3].x, MoveallLoc[3].y, MoveallLoc[3].z);
                m_uiChampAlianceGuid[3] = pChampAliance04->GetGUID();
            }
            if (Creature* pChampAliance05 = me->SummonCreature(NPC_CHAMPION_2_ALLIANCE, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_TIMED_DESPAWN, 32000))
            {
                pChampAliance05->GetMotionMaster()->MovePoint(0, MoveallLoc[4].x, MoveallLoc[4].y, MoveallLoc[4].z);
                m_uiChampAlianceGuid[4] = pChampAliance05->GetGUID();
            }
            if (Creature* pChampAliance06 = me->SummonCreature(NPC_CHAMPION_2_ALLIANCE, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_TIMED_DESPAWN, 32000))
            {
                pChampAliance06->GetMotionMaster()->MovePoint(0, MoveallLoc[5].x, MoveallLoc[5].y, MoveallLoc[5].z);
                m_uiChampAlianceGuid[5] = pChampAliance06->GetGUID();
            }
        }

        void SummonAlyChampions03()
        {
            if (Creature* pChampAliance07 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance07->GetMotionMaster()->MovePoint(0, MoveallLoc[6].x, MoveallLoc[6].y, MoveallLoc[6].z);
                m_uiChampAlianceGuid[6] = pChampAliance07->GetGUID();
            }
            if (Creature* pChampAliance08 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance08->GetMotionMaster()->MovePoint(0, MoveallLoc[7].x, MoveallLoc[7].y, MoveallLoc[7].z);
                m_uiChampAlianceGuid[7] = pChampAliance08->GetGUID();
            }
            if (Creature* pChampAliance09 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance09->GetMotionMaster()->MovePoint(0, MoveallLoc[8].x, MoveallLoc[8].y, MoveallLoc[8].z);
                m_uiChampAlianceGuid[8] = pChampAliance09->GetGUID();
            }
        }

        void SummonAlyChampions04()
        {
            if (Creature* pChampAliance10 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance10->GetMotionMaster()->MovePoint(0, 439.0f, 225.0f, 528.709181f);
                m_uiChampAlianceGuid[9] = pChampAliance10->GetGUID();
            }
            if (Creature* pChampAliance11 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance11->GetMotionMaster()->MovePoint(0, 444.0f, 220.0f, 528.709181f);
                m_uiChampAlianceGuid[10] = pChampAliance11->GetGUID();
            }
            if (Creature* pChampAliance12 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance12->GetMotionMaster()->MovePoint(0, 435.0f, 230.0f, 528.709181f);
                m_uiChampAlianceGuid[11] = pChampAliance12->GetGUID();
            }
            
            if (Creature* pChampAliance13 = me->SummonCreature(NPC_CHAMPION_3_ALLIANCE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
            {
                pChampAliance13->GetMotionMaster()->MovePoint(0, 433.0f, 221.0f, 528.709181f);
                m_uiChampAlianceGuid[12] = pChampAliance13->GetGUID();
            }
        }

        void MovieChampions01()
        {
            // Aliance Champions movie
            if(Creature* pAChampion01 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[0]))
            {
                pAChampion01->GetMotionMaster()->MovePoint(0, MoveLoc[0].x + 5, MoveLoc[0].y - 2, MoveLoc[0].z);
            }
            if(Creature* pAChampion02 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[1]))
            {
                pAChampion02->GetMotionMaster()->MovePoint(0, MoveLoc[0].x + 3, MoveLoc[0].y + 2, MoveLoc[0].z);
            }
            if(Creature* pAChampion03 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[2]))
            {
                pAChampion03->GetMotionMaster()->MovePoint(0, MoveLoc[0].x - 5, MoveLoc[0].y + 2, MoveLoc[0].z);
            }
            if(Creature* pAChampion04 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[3]))
            {
                pAChampion04->GetMotionMaster()->MovePoint(0, MoveLoc[1].x + 5, MoveLoc[1].y - 2, MoveLoc[1].z);
            }
            if(Creature* pAChampion05 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[4]))
            {
                pAChampion05->GetMotionMaster()->MovePoint(0, MoveLoc[1].x + 3, MoveLoc[1].y + 2, MoveLoc[1].z);
            }
            if(Creature* pAChampion06 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[5]))
            {
                pAChampion06->GetMotionMaster()->MovePoint(0, MoveLoc[1].x - 5, MoveLoc[1].y + 2, MoveLoc[1].z);
            }
            if(Creature* pAChampion07 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[6]))
            {
                pAChampion07->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 5, MoveLoc[2].y - 2, MoveLoc[2].z);
            }
            if(Creature* pAChampion08 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[7]))
            {
                pAChampion08->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 3, MoveLoc[2].y + 2, MoveLoc[2].z);
            }
            if(Creature* pAChampion09 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[8]))
            {
                pAChampion09->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
            }
            if(Creature* pAChampion10 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[9]))
            {
                pAChampion10->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 3, MoveLoc[2].y + 2, MoveLoc[2].z);
            }
            if(Creature* pAChampion11 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[10]))
            {
                pAChampion11->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
            }
            if(Creature* pAChampion12 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[11]))
            {
                pAChampion12->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
            }
            if(Creature* pAChampion13 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[12]))
            {
                pAChampion13->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
            }
        }

        void TyrannusTargetAura()
        {
            // Aliance Champions movie
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[0]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[1]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[2]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[3]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[4]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[5]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[6]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[7]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[8]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[9]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[10]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[11]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
            if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[12]))
            {
                pAChampions->GetMotionMaster()->MovePoint(0, pAChampions->GetPositionX(),pAChampions->GetPositionY(),535.0f);
                pAChampions->CastSpell(pAChampions, SPELL_STRANGULATING, false);
            }
        }

        void NeedTele()
        {
            if(Creature* pHChampions01 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[0]))
            {
                pHChampions01->NearTeleportTo(pHChampions01->GetPositionX(),pHChampions01->GetPositionY(),528.70918f, pHChampions01->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[1]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[2]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if (Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[3]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[4]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[5]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[6]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[7]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[8]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[9]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[10]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[11]))
            {
                pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
            }
            if(Creature* pHChampions13 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[12]))
            {
                pHChampions13->NearTeleportTo(pHChampions13->GetPositionX(),pHChampions13->GetPositionY(),528.70918f, pHChampions13->GetOrientation());
            }
        }

        void MoveSkeletAlliance()
        {
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[0]))
            {
                if (Creature *pTempSkelet1 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet1->CastSpell(pTempSkelet1, SPELL_RESURECT, false);
                    pTempSkelet1->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[0] = pTempSkelet1->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[1]))
            {
                if (Creature *pTempSkelet2 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet2->CastSpell(pTempSkelet2, SPELL_RESURECT, false);
                    pTempSkelet2->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[1] = pTempSkelet2->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[2]))
            {
                if (Creature *pTempSkelet3 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet3->CastSpell(pTempSkelet3, SPELL_RESURECT, false);
                    pTempSkelet3->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[2] = pTempSkelet3->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[3]))
            {
                if (Creature *pTempSkelet4 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet4->CastSpell(pTempSkelet4, SPELL_RESURECT, false);
                    pTempSkelet4->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[3] = pTempSkelet4->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[4]))
            {
                if (Creature *pTempSkelet5 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet5->CastSpell(pTempSkelet5, SPELL_RESURECT, false);
                    pTempSkelet5->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[4] = pTempSkelet5->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[5]))
            {
                if (Creature *pTempSkelet6 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet6->CastSpell(pTempSkelet6, SPELL_RESURECT, false);
                    pTempSkelet6->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[5] = pTempSkelet6->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[6]))
            {
                if (Creature *pTempSkelet7 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet7->CastSpell(pTempSkelet7, SPELL_RESURECT, false);
                    pTempSkelet7->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[6] = pTempSkelet7->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[7]))
            {
                if (Creature *pTempSkelet8 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet8->CastSpell(pTempSkelet8, SPELL_RESURECT, false);
                    pTempSkelet8->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[7] = pTempSkelet8->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[8]))
            {
                if (Creature *pTempSkelet9 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet9->CastSpell(pTempSkelet9, SPELL_RESURECT, false);
                    pTempSkelet9->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[8] = pTempSkelet9->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[9]))
            {
                if (Creature *pTempSkelet10 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet10->CastSpell(pTempSkelet10, SPELL_RESURECT, false);
                    pTempSkelet10->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[9] = pTempSkelet10->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[10]))
            {
                if (Creature *pTempSkelet11 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet11->CastSpell(pTempSkelet11, SPELL_RESURECT, false);
                    pTempSkelet11->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[10] = pTempSkelet11->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[11]))
            {
                if (Creature *pTempSkelet12 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet12->CastSpell(pTempSkelet12, SPELL_RESURECT, false);
                    pTempSkelet12->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[11] = pTempSkelet12->GetGUID();
                }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[12]))
            {
                if (Creature *pTempSkelet13 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                {
                    pTempSkelet13->CastSpell(pTempSkelet13, SPELL_RESURECT, false);
                    pTempSkelet13->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                    m_uiSkeletGuid[12] = pTempSkelet13->GetGUID();
                }
            }
        }

        void TyrannusIntroEndAlliance()
        {
            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
            {
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[0]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[1]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[2]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[3]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[4]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[5]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[6]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[7]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[8]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[9]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[10]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[11]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
                if(Creature* pAChampions = me->GetMap()->GetCreature(m_uiChampAlianceGuid[12]))
                {
                    pTyrannus->CastSpell(pAChampions, SPELL_NECROMANTIC_POWER, false);
                    pAChampions->DespawnOrUnsummon();
                }
            }
        }
    
        void CastSpellDiedSkelet()
        {
            if(Creature* pSkelet1 = me->GetMap()->GetCreature(m_uiSkeletGuid[0]))
            {
                me->CastSpell(pSkelet1, 70277, true);
                me->CastSpell(pSkelet1, 70277, true);
                me->CastSpell(pSkelet1, 70277, true);
                me->CastSpell(pSkelet1, 70277, true);
                me->CastSpell(pSkelet1, SPELL_NECROMANTIC_POWER, true);
                
            }
            if(Creature* pSkelet2 = me->GetMap()->GetCreature(m_uiSkeletGuid[1]))
            {
                me->CastSpell(pSkelet2, 70277, true);
                me->CastSpell(pSkelet2, 70277, true);
                me->CastSpell(pSkelet2, 70277, true);
                me->CastSpell(pSkelet2, 70277, true);
                me->CastSpell(pSkelet2, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet3 = me->GetMap()->GetCreature(m_uiSkeletGuid[2]))
            {
                me->CastSpell(pSkelet3, 70277, true);
                me->CastSpell(pSkelet3, 70277, true);
                me->CastSpell(pSkelet3, 70277, true);
                me->CastSpell(pSkelet3, 70277, true);
                me->CastSpell(pSkelet3, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet4 = me->GetMap()->GetCreature(m_uiSkeletGuid[3]))
            {
                me->CastSpell(pSkelet4, 70277, true);
                me->CastSpell(pSkelet4, 70277, true);
                me->CastSpell(pSkelet4, 70277, true);
                me->CastSpell(pSkelet4, 70277, true);
                me->CastSpell(pSkelet4, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet5 = me->GetMap()->GetCreature(m_uiSkeletGuid[4]))
            {
                me->CastSpell(pSkelet5, 70277, true);
                me->CastSpell(pSkelet5, 70277, true);
                me->CastSpell(pSkelet5, 70277, true);
                me->CastSpell(pSkelet5, 70277, true);
                me->CastSpell(pSkelet5, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet6 = me->GetMap()->GetCreature(m_uiSkeletGuid[5]))
            {
                me->CastSpell(pSkelet6, 70277, true);
                me->CastSpell(pSkelet6, 70277, true);
                me->CastSpell(pSkelet6, 70277, true);
                me->CastSpell(pSkelet6, 70277, true);
                me->CastSpell(pSkelet6, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet7 = me->GetMap()->GetCreature(m_uiSkeletGuid[6]))
            {
                me->CastSpell(pSkelet7, 70277, true);
                me->CastSpell(pSkelet7, 70277, true);
                me->CastSpell(pSkelet7, 70277, true);
                me->CastSpell(pSkelet7, 70277, true);
                me->CastSpell(pSkelet7, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet8 = me->GetMap()->GetCreature(m_uiSkeletGuid[7]))
            {
                me->CastSpell(pSkelet8, 70277, true);
                me->CastSpell(pSkelet8, 70277, true);
                me->CastSpell(pSkelet8, 70277, true);
                me->CastSpell(pSkelet8, 70277, true);
                me->CastSpell(pSkelet8, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet9 = me->GetMap()->GetCreature(m_uiSkeletGuid[8]))
            {
                me->CastSpell(pSkelet9, 70277, true);
                me->CastSpell(pSkelet9, 70277, true);
                me->CastSpell(pSkelet9, 70277, true);
                me->CastSpell(pSkelet9, 70277, true);
                me->CastSpell(pSkelet9, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet10 = me->GetMap()->GetCreature(m_uiSkeletGuid[9]))
            {
                me->CastSpell(pSkelet10, 70277, true);
                me->CastSpell(pSkelet10, 70277, true);
                me->CastSpell(pSkelet10, 70277, true);
                me->CastSpell(pSkelet10, 70277, true);
                me->CastSpell(pSkelet10, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet11 = me->GetMap()->GetCreature(m_uiSkeletGuid[10]))
            {
                me->CastSpell(pSkelet11, 70277, true);
                me->CastSpell(pSkelet11, 70277, true);
                me->CastSpell(pSkelet11, 70277, true);
                me->CastSpell(pSkelet11, 70277, true);
                me->CastSpell(pSkelet11, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet12 = me->GetMap()->GetCreature(m_uiSkeletGuid[11]))
            {
                me->CastSpell(pSkelet12, 70277, true);
                me->CastSpell(pSkelet12, 70277, true);
                me->CastSpell(pSkelet12, 70277, true);
                me->CastSpell(pSkelet12, 70277, true);
                me->CastSpell(pSkelet12, SPELL_NECROMANTIC_POWER, true);
            }
            if(Creature* pSkelet13 = me->GetMap()->GetCreature(m_uiSkeletGuid[12]))
            {
                me->CastSpell(pSkelet13, 70277, true);
                me->CastSpell(pSkelet13, 70277, true);
                me->CastSpell(pSkelet13, 70277, true);
                me->CastSpell(pSkelet13, 70277, true);
                me->CastSpell(pSkelet13, SPELL_NECROMANTIC_POWER, true);
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (instance->GetBossState(DATA_TYRANNUS) == DONE || instance->GetBossState(DATA_ICK) == DONE || instance->GetBossState(DATA_GARFROST) == DONE
            || instance->GetBossState(DATA_TYRANNUS) == IN_PROGRESS || instance->GetBossState(DATA_ICK) == IN_PROGRESS || instance->GetBossState(DATA_GARFROST) == IN_PROGRESS)
            {            
                m_bIsIntro = false;
            }
            if (instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED && instance->GetBossState(DATA_ICK) == NOT_STARTED
            &&  instance->GetBossState(DATA_TYRANNUS) != IN_PROGRESS && instance->GetBossState(DATA_ICK) != IN_PROGRESS && instance->GetBossState(DATA_GARFROST) != IN_PROGRESS
            &&  instance->GetBossState(DATA_TYRANNUS) != DONE && instance->GetBossState(DATA_ICK) != DONE && instance->GetBossState(DATA_GARFROST) != DONE)
                m_bIsIntro = true;
        }
        
        void SummonMageStart()
        {
            if(Creature* pMageLeaft = me->SummonCreature(36788, 497.0f, 245.639f, 528.686f, 3.431f, TEMPSUMMON_MANUAL_DESPAWN))
            {
                pMageLeaft->CastSpell(pMageLeaft, 8674, false);    
                m_uiMageLeaftGuid = pMageLeaft->GetGUID();
            }

            if(Creature* pMageRight = me->SummonCreature(36788, 510.0f, 221.639f, 528.686f, 3.431f, TEMPSUMMON_MANUAL_DESPAWN))
            {
                pMageRight->CastSpell(pMageRight, 8674, false);
                m_uiMageRightGuid = pMageRight->GetGUID();
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if(m_bIsIntro && !m_bIsIntroEnd && instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED && instance->GetBossState(DATA_ICK) == NOT_STARTED
            &&  instance->GetBossState(DATA_TYRANNUS) != IN_PROGRESS && instance->GetBossState(DATA_ICK) != IN_PROGRESS && instance->GetBossState(DATA_GARFROST) != IN_PROGRESS
            &&  instance->GetBossState(DATA_TYRANNUS) != DONE && instance->GetBossState(DATA_ICK) != SPECIAL && instance->GetBossState(DATA_ICK) != DONE && instance->GetBossState(DATA_GARFROST) != DONE)
            {
                if(m_uiSpeech_TimerAlliance < diff)
                {
                    switch(m_uiIntro_Phase)
                    {
                        case 0:
                            SummonMageStart();
                            if(Creature* pTyrannus = me->SummonCreature(NPC_TYRANNUS_INTRO, 526.501f, 237.639f, 543.686f, 3.431f, TEMPSUMMON_MANUAL_DESPAWN))
                            {
                                pTyrannus->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                pTyrannus->SetGuidValue(UNIT_FIELD_TARGET, me->GetGUID());
                                pTyrannus->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                pTyrannus->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
                                pTyrannus->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
                                pTyrannus->MonsterMoveWithSpeed(526.501f, 237.639f, 543.686f, 1);
                                pTyrannus->GetMap()->CreatureRelocation(pTyrannus, 526.501f, 237.639f, 543.686f, 3.431f);
                                DoScriptText(SAY_TYRANNUS1, pTyrannus);
                                m_uiTyrannusGuid = pTyrannus->GetGUID();
                                SummonAlyChampions01();
                            }
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3000;
                            break;
                        case 1:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 2000;
                            break;
                        case 2:
                            SummonAlyChampions02();
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3999;
                            break;
                        case 3:
                            SummonAlyChampions03();
                            SummonAlyChampions04();
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 2999;
                            break;
                        case 4:
                            if(Creature* pMageLeaft =  me->GetMap()->GetCreature(m_uiMageLeaftGuid))
                            {
                                pMageLeaft->CastSpell(pMageLeaft, 8674, false);            
                            }
                            if(Creature* pMageRight =  me->GetMap()->GetCreature(m_uiMageRightGuid))
                            {
                                pMageRight->CastSpell(pMageRight, 8674, false);            
                            }
            
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                DoScriptText(SAY_TYRANNUS2, pTyrannus);
                            }
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 7500;
                            break;
                        case 5:
                            DoScriptText(SAY_SPEECH_JAINA1, me);
                            MovieChampions01();
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3999;
                            break;
                        case 6:
                            if(Creature* pMageLeaft =  me->GetMap()->GetCreature(m_uiMageLeaftGuid))
                            {
                                pMageLeaft->SetSpeed(MOVE_RUN, 0.5f, true);
                                pMageLeaft->GetMotionMaster()->MovePoint(0,513.924133f, 249.848083f, 528.291138f);            
                            }
                            if(Creature* pMageRight =  me->GetMap()->GetCreature(m_uiMageRightGuid))
                            {
                                pMageRight->SetSpeed(MOVE_RUN, 0.5f, true);
                                pMageRight->GetMotionMaster()->MovePoint(0,525.190918f, 227.157852f, 528.595032f);
                            }    
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 5000;
                            break;
                        case 7:
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                                TyrannusTargetAura();
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 5000;
                            break;
                        case 8:
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                DoScriptText(SAY_TYRANNUS3, pTyrannus);
                            }
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 5000;
                            break;
                        case 9:
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                DoScriptText(SAY_TYRANNUS4, pTyrannus);
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion01 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[0]))
                                {
                                    pTyrannus->CastSpell(pAChampion01, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion02 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[1]))
                                {
                                    pTyrannus->CastSpell(pAChampion02, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion03 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[2]))
                                {
                                    pTyrannus->CastSpell(pAChampion03, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion04 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[3]))
                                {
                                    pTyrannus->CastSpell(pAChampion04, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion05 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[4]))
                                {
                                    pTyrannus->CastSpell(pAChampion05, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion06 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[5]))
                                {
                                    pTyrannus->CastSpell(pAChampion06, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion07 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[6]))
                                {
                                    pTyrannus->CastSpell(pAChampion07, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion08 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[7]))
                                {
                                    pTyrannus->CastSpell(pAChampion08, SPELL_NECROTIC, false);
                                }
                            }
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                if(Creature* pAChampion09 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[8]))
                                {
                                    pTyrannus->CastSpell(pAChampion09, SPELL_NECROTIC, false);
                                }
                                if(Creature* pAChampion10 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[9]))
                                {
                                    pTyrannus->CastSpell(pAChampion10, SPELL_NECROTIC, false);
                                }
                                if(Creature* pAChampion11 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[10]))
                                {
                                    pTyrannus->CastSpell(pAChampion11, SPELL_NECROTIC, false);
                                }
                                if(Creature* pAChampion12 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[11]))
                                {
                                    pTyrannus->CastSpell(pAChampion12, SPELL_NECROTIC, false);
                                }
                                if(Creature* pAChampion13 = me->GetMap()->GetCreature(m_uiChampAlianceGuid[12]))
                                {
                                    pTyrannus->CastSpell(pAChampion13, SPELL_NECROTIC, false);
                                }
                            }
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 10:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 11:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 12:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 13:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 14:
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                            {
                                DoScriptText(SAY_TYRANNUS5, pTyrannus);
                            }
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 15:
                            MoveSkeletAlliance();
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 16:
                            NeedTele();
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 17:
                            TyrannusIntroEndAlliance();    
                            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                                instance->SetBossState(DATA_GARFROST, SPECIAL);
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 2000;
                            break;
                        case 18:
                            DoScriptText(SAY_SPEECH_JAINA2, me);
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 1500;
                            break;
                        case 19:
                            CastSpellDiedSkelet();    
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3500;
                            break;
                        case 20:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3000;
                            break;
                        case 21:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 2000;
                            break;
                        case 22:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 23:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 24:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 100;
                            break;
                        case 25:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 300;
                            break;
                        case 26:
                            DoScriptText(SAY_SPEECH_JAINA3, me);                            
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3500;
                            break;
                        case 27:
                            DoScriptText(SAY_SPEECH_JAINA4, me);
                            ++m_uiIntro_Phase;
                            m_uiSpeech_TimerAlliance = 3000;
                            break;
                        case 28:
                            DoScriptText(SAY_SPEECH_JAINA5, me);
                            ++m_uiIntro_Phase;
                            //m_bIsIntro = false;
                            m_bIsIntroEnd = true;
                            m_uiSpeech_TimerAlliance = 5000;
                            break;
                    }
                } else m_uiSpeech_TimerAlliance -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jaina_POSintroAI(creature);
    }
};
    
class npc_sylvanas_POSintro : public CreatureScript
{
public:
    npc_sylvanas_POSintro() : CreatureScript("npc_sylvanas_POSintro") { }

    struct npc_sylvanas_POSintroAI: public ScriptedAI
    {
        npc_sylvanas_POSintroAI(Creature* creature) : ScriptedAI(creature)
        {
        }
            uint32 m_uiSpeech_Timer;
            ObjectGuid m_uiTyrannusGuid;
            ObjectGuid m_uiMageLeaftGuid;
            ObjectGuid m_uiMageRightGuid;
            ObjectGuid m_uiSkeletGuid[12];
            ObjectGuid muiCastNpcStart;
            ObjectGuid m_uiDeathGuid;
            uint8 m_uiIntro_Phase;
            bool m_bIsIntro;
            bool m_bIsIntroEnd;
            ObjectGuid m_uiChampHordeGuid[12];
            uint32 creatureEntry;
    

        void Reset() override
        {
            InstanceScript* instance = me->GetInstanceScript();
            m_uiIntro_Phase     = 0;
            m_uiSpeech_Timer    = 1000;
            m_uiTyrannusGuid.Clear();
            m_uiMageLeaftGuid.Clear();
            m_uiMageRightGuid.Clear();
            muiCastNpcStart.Clear();
            m_uiDeathGuid.Clear();
            m_bIsIntro = false;
            m_bIsIntroEnd = false;

            for (int32 i = 0; i < 11; ++i)
            {
                m_uiChampHordeGuid[i].Clear();
                m_uiSkeletGuid[i].Clear();
            }

            creatureEntry = me->GetEntry();
        }
        
        void SummonHordeChampions01()
    {
        if (Creature* pChampHorde01 = me->SummonCreature(NPC_CHAMPION_1_HORDE, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_TIMED_DESPAWN, 37800))
        {
            pChampHorde01->GetMotionMaster()->MovePoint(0, MoveallLoc[0].x, MoveallLoc[0].y, MoveallLoc[0].z);
            m_uiChampHordeGuid[0] = pChampHorde01->GetGUID();
        }
        if (Creature* pChampHorde02 = me->SummonCreature(NPC_CHAMPION_1_HORDE, SummonLoc[0].x + 1, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_TIMED_DESPAWN, 37800))
        {
            pChampHorde02->GetMotionMaster()->MovePoint(0, MoveallLoc[1].x, MoveallLoc[1].y, MoveallLoc[1].z);
            m_uiChampHordeGuid[1] = pChampHorde02->GetGUID();
        }
        if (Creature* pChampHorde03 = me->SummonCreature(NPC_CHAMPION_1_HORDE, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z, SummonLoc[0].o, TEMPSUMMON_TIMED_DESPAWN, 37800))
        {
            pChampHorde03->GetMotionMaster()->MovePoint(0, MoveallLoc[2].x, MoveallLoc[2].y, MoveallLoc[2].z);
            m_uiChampHordeGuid[2] = pChampHorde03->GetGUID();
        }
    }
    
    
    
        void SummonHordeChampions02()
    {
        if (Creature* pChampHorde04 = me->SummonCreature(NPC_CHAMPION_2_HORDE, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_TIMED_DESPAWN, 32000))
        {
            pChampHorde04->GetMotionMaster()->MovePoint(0, MoveallLoc[3].x, MoveallLoc[3].y, MoveallLoc[3].z);
            m_uiChampHordeGuid[3] = pChampHorde04->GetGUID();
        }
        if (Creature* pChampHorde05 = me->SummonCreature(NPC_CHAMPION_2_HORDE, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_TIMED_DESPAWN, 32000))
        {
            pChampHorde05->GetMotionMaster()->MovePoint(0, MoveallLoc[4].x, MoveallLoc[4].y, MoveallLoc[4].z);
            m_uiChampHordeGuid[4] = pChampHorde05->GetGUID();
        }
        if (Creature* pChampHorde06 = me->SummonCreature(NPC_CHAMPION_2_HORDE, SummonLoc[1].x, SummonLoc[1].y, SummonLoc[1].z, SummonLoc[1].o, TEMPSUMMON_TIMED_DESPAWN, 32000))
        {
            pChampHorde06->GetMotionMaster()->MovePoint(0, MoveallLoc[5].x, MoveallLoc[5].y, MoveallLoc[5].z);
            m_uiChampHordeGuid[5] = pChampHorde06->GetGUID();
        }
    }
    
    void SummonHordeChampions03()
    {
        if (Creature* pChampHorde07 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
        {
            pChampHorde07->GetMotionMaster()->MovePoint(0, MoveallLoc[6].x, MoveallLoc[6].y, MoveallLoc[6].z);
            m_uiChampHordeGuid[6] = pChampHorde07->GetGUID();
        }
        if (Creature* pChampHorde08 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x + 1, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
        {
            pChampHorde08->GetMotionMaster()->MovePoint(0, MoveallLoc[7].x, MoveallLoc[7].y, MoveallLoc[7].z);
            m_uiChampHordeGuid[7] = pChampHorde08->GetGUID();
        }
        if (Creature* pChampHorde09 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
        {
            pChampHorde09->GetMotionMaster()->MovePoint(0, MoveallLoc[8].x, MoveallLoc[8].y, MoveallLoc[8].z);
            m_uiChampHordeGuid[8] = pChampHorde09->GetGUID();
        }
    }
    
    
        void SummonHordeChampions04()
    {
        if (Creature* pChampHorde10 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
        {
            pChampHorde10->GetMotionMaster()->MovePoint(0, 439.0f, 225.0f, 528.709181f);
            m_uiChampHordeGuid[9] = pChampHorde10->GetGUID();
        }
        if (Creature* pChampHorde11 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x + 1, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
        {
            pChampHorde11->GetMotionMaster()->MovePoint(0, 444.0f, 220.0f, 528.709181f);
            m_uiChampHordeGuid[10] = pChampHorde11->GetGUID();
        }
        if (Creature* pChampHorde12 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28500))
        {
            pChampHorde12->GetMotionMaster()->MovePoint(0, 435.0f, 230.0f, 528.709181f);
            m_uiChampHordeGuid[11] = pChampHorde12->GetGUID();
        }
        
        if (Creature* pChampHorde13 = me->SummonCreature(NPC_CHAMPION_3_HORDE, SummonLoc[2].x, SummonLoc[2].y, SummonLoc[2].z, SummonLoc[2].o, TEMPSUMMON_TIMED_DESPAWN, 28000))
        {
            pChampHorde13->GetMotionMaster()->MovePoint(0, 433.0f, 221.0f, 528.709181f);
            m_uiChampHordeGuid[12] = pChampHorde13->GetGUID();
        }
    }
    
    
    void MovieChampions01()
    {
        // Horde Soldiers movie
        if(Creature* pHChampion01 = me->GetMap()->GetCreature(m_uiChampHordeGuid[0]))
        {
            pHChampion01->GetMotionMaster()->MovePoint(0, MoveLoc[0].x + 5, MoveLoc[0].y - 2, MoveLoc[0].z);
        }
        if(Creature* pHChampion02 = me->GetMap()->GetCreature(m_uiChampHordeGuid[1]))
        {
            pHChampion02->GetMotionMaster()->MovePoint(0, MoveLoc[0].x + 3, MoveLoc[0].y + 2, MoveLoc[0].z);
        }
        if(Creature* pHChampion03 = me->GetMap()->GetCreature(m_uiChampHordeGuid[2]))
        {
            pHChampion03->GetMotionMaster()->MovePoint(0, MoveLoc[0].x - 5, MoveLoc[0].y + 2, MoveLoc[0].z);
        }
        
        if(Creature* pHChampion04 = me->GetMap()->GetCreature(m_uiChampHordeGuid[3]))
        {
            pHChampion04->GetMotionMaster()->MovePoint(0, MoveLoc[1].x + 5, MoveLoc[1].y - 2, MoveLoc[1].z);
        }
        if(Creature* pHChampion05 = me->GetMap()->GetCreature(m_uiChampHordeGuid[4]))
        {
            pHChampion05->GetMotionMaster()->MovePoint(0, MoveLoc[1].x + 3, MoveLoc[1].y + 2, MoveLoc[1].z);
        }
        if(Creature* pHChampion06 = me->GetMap()->GetCreature(m_uiChampHordeGuid[5]))
        {
            pHChampion06->GetMotionMaster()->MovePoint(0, MoveLoc[1].x - 5, MoveLoc[1].y + 2, MoveLoc[1].z);
        }
        if(Creature* pHChampion07 = me->GetMap()->GetCreature(m_uiChampHordeGuid[6]))
        {
            pHChampion07->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 5, MoveLoc[2].y - 2, MoveLoc[2].z);
        }
    
        if(Creature* pHChampion08 = me->GetMap()->GetCreature(m_uiChampHordeGuid[7]))
        {
            pHChampion08->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 3, MoveLoc[2].y + 2, MoveLoc[2].z);
        }
        if(Creature* pHChampion09 = me->GetMap()->GetCreature(m_uiChampHordeGuid[8]))
        {
            pHChampion09->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
        }
            if(Creature* pHChampion10 = me->GetMap()->GetCreature(m_uiChampHordeGuid[9]))
        {
            pHChampion10->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 3, MoveLoc[2].y + 2, MoveLoc[2].z);
        }
        if(Creature* pHChampion11 = me->GetMap()->GetCreature(m_uiChampHordeGuid[10]))
        {
            pHChampion11->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
        }
            if(Creature* pHChampion12 = me->GetMap()->GetCreature(m_uiChampHordeGuid[11]))
        {
            pHChampion12->GetMotionMaster()->MovePoint(0, MoveLoc[2].x + 3, MoveLoc[2].y + 2, MoveLoc[2].z);
        }
        if(Creature* pHChampion13 = me->GetMap()->GetCreature(m_uiChampHordeGuid[12]))
        {
            pHChampion13->GetMotionMaster()->MovePoint(0, MoveLoc[2].x - 5, MoveLoc[2].y + 2, MoveLoc[2].z);
        }

        
    }

    
        void TyrannusTargetAura()
    {

        for (uint8 i = 0; i < 13; ++i)
        {
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[i])) {
            pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),535.0f);
            pHChampions->CastSpell(pHChampions, SPELL_STRANGULATING, false);
        }
        }

    
    }
    
    void NeedTele() {

    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[0])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
    
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[1])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[2])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[3])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[4])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[5])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[6])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[7])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[8])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[9])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[10])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[11])) {
            pHChampions->NearTeleportTo(pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f, pHChampions->GetOrientation());
                }
                
    if(Creature* pHChampions13 = me->GetMap()->GetCreature(m_uiChampHordeGuid[12])) {
            pHChampions13->NearTeleportTo(pHChampions13->GetPositionX(),pHChampions13->GetPositionY(),528.70918f, pHChampions13->GetOrientation());
                }

    
    }
    

    void TyrannusIntroEndHorde()
    {
            if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
       {
            
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[0])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[1])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[2])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[3])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[4])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[5])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[6])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[7])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[8])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[9])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[10])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[11])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            
             if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[12])) {
            //pHChampions->GetMotionMaster()->MovePoint(0, pHChampions->GetPositionX(),pHChampions->GetPositionY(),528.70918f);
            pTyrannus->CastSpell(pHChampions, SPELL_NECROMANTIC_POWER, false);
            pHChampions->DespawnOrUnsummon();
            }
            }
    }
    
    void MoveSkeletHorde() {
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[0])) {
            if (Creature *pTempSkelet1 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[0] = pTempSkelet1->GetGUID();
                pTempSkelet1->CastSpell(pTempSkelet1, SPELL_RESURECT, false);
                pTempSkelet1->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
                
            }
            }
            
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[1])) {
            if (Creature *pTempSkelet2 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[1] = pTempSkelet2->GetGUID();
                pTempSkelet2->CastSpell(pTempSkelet2, SPELL_RESURECT, false);
                pTempSkelet2->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[2])) {
            if (Creature *pTempSkelet3 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[2] = pTempSkelet3->GetGUID();
                pTempSkelet3->CastSpell(pTempSkelet3, SPELL_RESURECT, false);
                pTempSkelet3->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[3])) {
            if (Creature *pTempSkelet4 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[3] = pTempSkelet4->GetGUID();
                pTempSkelet4->CastSpell(pTempSkelet4, SPELL_RESURECT, false);
                pTempSkelet4->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[4])) {
            if (Creature *pTempSkelet5 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[4] = pTempSkelet5->GetGUID();
                pTempSkelet5->CastSpell(pTempSkelet5, SPELL_RESURECT, false);
                pTempSkelet5->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[5])) {
            if (Creature *pTempSkelet6 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[5] = pTempSkelet6->GetGUID();
                pTempSkelet6->CastSpell(pTempSkelet6, SPELL_RESURECT, false);
                pTempSkelet6->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[6])) {
            if (Creature *pTempSkelet7 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[6] = pTempSkelet7->GetGUID();
                pTempSkelet7->CastSpell(pTempSkelet7, SPELL_RESURECT, false);
                pTempSkelet7->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[7])) {
            if (Creature *pTempSkelet8 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[7] = pTempSkelet8->GetGUID();
                pTempSkelet8->CastSpell(pTempSkelet8, SPELL_RESURECT, false);
                pTempSkelet8->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[8])) {
            if (Creature *pTempSkelet9 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[8] = pTempSkelet9->GetGUID();
                pTempSkelet9->CastSpell(pTempSkelet9, SPELL_RESURECT, false);
                pTempSkelet9->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[9])) {
            if (Creature *pTempSkelet10 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[9] = pTempSkelet10->GetGUID();
                pTempSkelet10->CastSpell(pTempSkelet10, SPELL_RESURECT, false);
                pTempSkelet10->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[10])) {
            if (Creature *pTempSkelet11 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[10] = pTempSkelet11->GetGUID();
                pTempSkelet11->CastSpell(pTempSkelet11, SPELL_RESURECT, false);
                pTempSkelet11->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[11])) {
            if (Creature *pTempSkelet12 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[11] = pTempSkelet12->GetGUID();
                pTempSkelet12->CastSpell(pTempSkelet12, SPELL_RESURECT, false);
                pTempSkelet12->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
            if(Creature* pHChampions = me->GetMap()->GetCreature(m_uiChampHordeGuid[12])) {
            if (Creature *pTempSkelet13 = me->SummonCreature(NPC_SKELETAL_SLAVE, pHChampions->GetPositionX(),pHChampions->GetPositionY(), 528.70918f, pHChampions->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
            {
                m_uiSkeletGuid[12] = pTempSkelet13->GetGUID();
                pTempSkelet13->CastSpell(pTempSkelet13, SPELL_RESURECT, false);
                pTempSkelet13->GetMotionMaster()->MovePoint(0, SummonLoc[0].x, SummonLoc[0].y, SummonLoc[0].z);    
            }
            }
    }
    
    void CastSpellDiedSkelet() {
    if(Creature* pSkelet1 = me->GetMap()->GetCreature(m_uiSkeletGuid[0])) {
    me->CastSpell(pSkelet1, 20463, true);
            me->CastSpell(pSkelet1, 70512, true);
            me->CastSpell(pSkelet1, 20463, true);
            me->CastSpell(pSkelet1, 70512, true);
            me->CastSpell(pSkelet1, SPELL_NECROMANTIC_POWER, true);
            
    }
    if(Creature* pSkelet2 = me->GetMap()->GetCreature(m_uiSkeletGuid[1])) {
        me->CastSpell(pSkelet2, 20463, true);
            me->CastSpell(pSkelet2, 70512, true);
            me->CastSpell(pSkelet2, 20463, true);
            me->CastSpell(pSkelet2, 70512, true);
            me->CastSpell(pSkelet2, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet3 = me->GetMap()->GetCreature(m_uiSkeletGuid[2])) {
        me->CastSpell(pSkelet3, 20463, true);
            me->CastSpell(pSkelet3, 70512, true);
            me->CastSpell(pSkelet3, 20463, true);
            me->CastSpell(pSkelet3, 70512, true);
            me->CastSpell(pSkelet3, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet4 = me->GetMap()->GetCreature(m_uiSkeletGuid[3])) {
        me->CastSpell(pSkelet4, 20463, true);
            me->CastSpell(pSkelet4, 70512, true);
            me->CastSpell(pSkelet4, 20463, true);
            me->CastSpell(pSkelet4, 70512, true);
            me->CastSpell(pSkelet4, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet5 = me->GetMap()->GetCreature(m_uiSkeletGuid[4])) {
        me->CastSpell(pSkelet5, 20463, true);
            me->CastSpell(pSkelet5, 70512, true);
            me->CastSpell(pSkelet5, 20463, true);
            me->CastSpell(pSkelet5, 70512, true);
            me->CastSpell(pSkelet5, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet6 = me->GetMap()->GetCreature(m_uiSkeletGuid[5])) {
        me->CastSpell(pSkelet6, 20463, true);
            me->CastSpell(pSkelet6, 70512, true);
            me->CastSpell(pSkelet6, 20463, true);
            me->CastSpell(pSkelet6, 70512, true);
            me->CastSpell(pSkelet6, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet7 = me->GetMap()->GetCreature(m_uiSkeletGuid[6])) {
    me->CastSpell(pSkelet7, 20463, true);
            me->CastSpell(pSkelet7, 70512, true);
            me->CastSpell(pSkelet7, 20463, true);
            me->CastSpell(pSkelet7, 70512, true);
            me->CastSpell(pSkelet7, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet8 = me->GetMap()->GetCreature(m_uiSkeletGuid[7])) {
    me->CastSpell(pSkelet8, 20463, true);
            me->CastSpell(pSkelet8, 70512, true);
            me->CastSpell(pSkelet8, 20463, true);
            me->CastSpell(pSkelet8, 70512, true);
            me->CastSpell(pSkelet8, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet9 = me->GetMap()->GetCreature(m_uiSkeletGuid[8])) {
    me->CastSpell(pSkelet9, 20463, true);
            me->CastSpell(pSkelet9, 70512, true);
            me->CastSpell(pSkelet9, 20463, true);
            me->CastSpell(pSkelet9, 70512, true);
            me->CastSpell(pSkelet9, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet10 = me->GetMap()->GetCreature(m_uiSkeletGuid[9])) {
    me->CastSpell(pSkelet10, 20463, true);
            me->CastSpell(pSkelet10, 70512, true);
            me->CastSpell(pSkelet10, 20463, true);
            me->CastSpell(pSkelet10, 70512, true);
            me->CastSpell(pSkelet10, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet11 = me->GetMap()->GetCreature(m_uiSkeletGuid[10])) {
    me->CastSpell(pSkelet11, 20463, true);
            me->CastSpell(pSkelet11, 70512, true);
            me->CastSpell(pSkelet11, 20463, true);
            me->CastSpell(pSkelet11, 70512, true);
            me->CastSpell(pSkelet11, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet12 = me->GetMap()->GetCreature(m_uiSkeletGuid[11])) {
    me->CastSpell(pSkelet12, 20463, true);
            me->CastSpell(pSkelet12, 70512, true);
            me->CastSpell(pSkelet12, 20463, true);
            me->CastSpell(pSkelet12, 70512, true);
            me->CastSpell(pSkelet12, SPELL_NECROMANTIC_POWER, true);
    }
    if(Creature* pSkelet13 = me->GetMap()->GetCreature(m_uiSkeletGuid[12])) {
    me->CastSpell(pSkelet13, 20463, true);
            me->CastSpell(pSkelet13, 70512, true);
            me->CastSpell(pSkelet13, 20463, true);
            me->CastSpell(pSkelet13, 70512, true);
            me->CastSpell(pSkelet13, SPELL_NECROMANTIC_POWER, true);
    
    }
    }
        void MoveInLineOfSight(Unit* who) override
        {
            InstanceScript* instance = me->GetInstanceScript();
        if (instance->GetBossState(DATA_TYRANNUS) == DONE || instance->GetBossState(DATA_ICK) == DONE || instance->GetBossState(DATA_GARFROST) == DONE
        || instance->GetBossState(DATA_TYRANNUS) == IN_PROGRESS || instance->GetBossState(DATA_ICK) == IN_PROGRESS || instance->GetBossState(DATA_GARFROST) == IN_PROGRESS)
        {            
            m_bIsIntro = false;
        }
        if (instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED && instance->GetBossState(DATA_ICK) == NOT_STARTED
        &&  instance->GetBossState(DATA_TYRANNUS) != IN_PROGRESS && instance->GetBossState(DATA_ICK) != IN_PROGRESS && instance->GetBossState(DATA_GARFROST) != IN_PROGRESS
        &&  instance->GetBossState(DATA_TYRANNUS) != DONE && instance->GetBossState(DATA_ICK) != DONE && instance->GetBossState(DATA_GARFROST) != DONE)
            m_bIsIntro = true;
             
        }
        
        void SummonMageStart() {
        
        if(Creature* pMageLeaft = me->SummonCreature(36788, 497.0f, 245.639f, 528.686f, 3.431f, TEMPSUMMON_MANUAL_DESPAWN))
                {
                pMageLeaft->CastSpell(pMageLeaft, 8674, false);    
                m_uiMageLeaftGuid = pMageLeaft->GetGUID();
                }
                
        if(Creature* pMageRight = me->SummonCreature(36788, 510.0f, 221.639f, 528.686f, 3.431f, TEMPSUMMON_MANUAL_DESPAWN))
                {
                pMageRight->CastSpell(pMageRight, 8674, false);
                m_uiMageRightGuid = pMageRight->GetGUID();
                }
                
        }

        void UpdateAI(uint32 diff) override
        {
                InstanceScript* instance = me->GetInstanceScript();
                if(m_bIsIntro && !m_bIsIntroEnd && instance->GetBossState(DATA_TYRANNUS) == NOT_STARTED && instance->GetBossState(DATA_ICK) == NOT_STARTED
                &&  instance->GetBossState(DATA_TYRANNUS) != IN_PROGRESS && instance->GetBossState(DATA_ICK) != IN_PROGRESS && instance->GetBossState(DATA_GARFROST) != IN_PROGRESS
                &&  instance->GetBossState(DATA_TYRANNUS) != DONE && instance->GetBossState(DATA_ICK) != DONE && instance->GetBossState(DATA_ICK) != SPECIAL && instance->GetBossState(DATA_GARFROST) != DONE) {
                if(m_uiSpeech_Timer < diff)
                {
                switch(m_uiIntro_Phase)
                {
                case 0:
                SummonMageStart();
                if(Creature* pTyrannus = me->SummonCreature(NPC_TYRANNUS_INTRO, 526.501f, 237.639f, 543.686f, 3.431f, TEMPSUMMON_MANUAL_DESPAWN))
                {
                pTyrannus->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                pTyrannus->SetGuidValue(UNIT_FIELD_TARGET, me->GetGUID());
                pTyrannus->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                pTyrannus->SetUInt32Value(UNIT_FIELD_BYTES_0, 50331648);
                pTyrannus->SetUInt32Value(UNIT_FIELD_BYTES_1, 50331648);
                pTyrannus->MonsterMoveWithSpeed(526.501f, 237.639f, 543.686f, 1);
                pTyrannus->GetMap()->CreatureRelocation(pTyrannus, 526.501f, 237.639f, 543.686f, 3.431f);
                DoScriptText(SAY_TYRANNUS1, pTyrannus);
                m_uiTyrannusGuid = pTyrannus->GetGUID();
                }
                SummonHordeChampions01(); 
            ++m_uiIntro_Phase;
            m_uiSpeech_Timer = 3000;
            break;
            case 1:
                            ++m_uiIntro_Phase;
                            m_uiSpeech_Timer = 2000;
                            break;
                            case 2:
                        SummonHordeChampions02();
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 3999;
                        break;
                        case 3:
                        SummonHordeChampions03();
                        SummonHordeChampions04();                        
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 2999;
                        break;
                    case 4:
                                    if(Creature* pMageLeaft =  me->GetMap()->GetCreature(m_uiMageLeaftGuid)) {
            pMageLeaft->CastSpell(pMageLeaft, 8674, false);            
            }
            if(Creature* pMageRight =  me->GetMap()->GetCreature(m_uiMageRightGuid)) {
            pMageRight->CastSpell(pMageRight, 8674, false);            
            }
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid)) {
                            DoScriptText(SAY_TYRANNUS2, pTyrannus);
                            }
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 7500;
                        break;
                    case 5:
                         DoScriptText(SAY_SPEECH_SYLVANAS1, me);
                         MovieChampions01();
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 3999;
                        break;
                        case 6:
                        if(Creature* pMageLeaft =  me->GetMap()->GetCreature(m_uiMageLeaftGuid)) {
                        pMageLeaft->SetSpeed(MOVE_RUN, 0.5f, true);
                        pMageLeaft->GetMotionMaster()->MovePoint(0,513.924133f, 249.848083f, 528.291138f);            
                            }
                        if(Creature* pMageRight =  me->GetMap()->GetCreature(m_uiMageRightGuid)) {
                        pMageRight->SetSpeed(MOVE_RUN, 0.5f, true);
                        pMageRight->GetMotionMaster()->MovePoint(0,525.190918f, 227.157852f, 528.595032f);
                        }    
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 5000;
                        case 7:
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        TyrannusTargetAura();
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 5000;
                        break;
                        case 8:
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid)){
                        DoScriptText(SAY_TYRANNUS3, pTyrannus);
                        }
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 5000;
                        break;
                         case 9:
                         if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid)){
                        DoScriptText(SAY_TYRANNUS4, pTyrannus);
                        }
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion01 = me->GetMap()->GetCreature(m_uiChampHordeGuid[0])) {
                                pTyrannus->CastSpell(pHChampion01, SPELL_NECROTIC, false);
                                }
                        }
                        
                                                if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            
                            if(Creature* pHChampion02 = me->GetMap()->GetCreature(m_uiChampHordeGuid[1])) {
                            pTyrannus->CastSpell(pHChampion02, SPELL_NECROTIC, false);
                                }

                        }
                        
                                                if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion03 = me->GetMap()->GetCreature(m_uiChampHordeGuid[2])) {
                            pTyrannus->CastSpell(pHChampion03, SPELL_NECROTIC, false);
                                }
                        }
                        
                                                if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion04 = me->GetMap()->GetCreature(m_uiChampHordeGuid[3])) {
                               pTyrannus->CastSpell(pHChampion04, SPELL_NECROTIC, false);
                                }
                        }
                        
                                                if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion05 = me->GetMap()->GetCreature(m_uiChampHordeGuid[4])) {
                             pTyrannus->CastSpell(pHChampion05, SPELL_NECROTIC, false);
                                }
                        }
                        
                                                if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                               if(Creature* pHChampion06 = me->GetMap()->GetCreature(m_uiChampHordeGuid[5])) {
                             pTyrannus->CastSpell(pHChampion06, SPELL_NECROTIC, false);
                                }

                        }
                                                if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion07 = me->GetMap()->GetCreature(m_uiChampHordeGuid[6])) {
                            pTyrannus->CastSpell(pHChampion07, SPELL_NECROTIC, false);
                                }
                        }
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion08 = me->GetMap()->GetCreature(m_uiChampHordeGuid[7])) {
                            pTyrannus->CastSpell(pHChampion08, SPELL_NECROTIC, false);
                                }
                        }
                         if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        {
                            if(Creature* pHChampion09 = me->GetMap()->GetCreature(m_uiChampHordeGuid[8])) {
                            pTyrannus->CastSpell(pHChampion09, SPELL_NECROTIC, false);
                                }
                                
                              if(Creature* pHChampion10 = me->GetMap()->GetCreature(m_uiChampHordeGuid[9])) {
                              pTyrannus->CastSpell(pHChampion10, SPELL_NECROTIC, false);
                                }
                              if(Creature* pHChampion11 = me->GetMap()->GetCreature(m_uiChampHordeGuid[10])) {
                              pTyrannus->CastSpell(pHChampion11, SPELL_NECROTIC, false);
                                }
                                  if(Creature* pHChampion12 = me->GetMap()->GetCreature(m_uiChampHordeGuid[11])) {
                                  pTyrannus->CastSpell(pHChampion12, SPELL_NECROTIC, false);
                                }
                                  if(Creature* pHChampion13 = me->GetMap()->GetCreature(m_uiChampHordeGuid[12])) {
                                  pTyrannus->CastSpell(pHChampion13, SPELL_NECROTIC, false);
                                }
                        }
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 10:
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 11:

                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 12:

                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 13:
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 14:
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid)){
                        DoScriptText(SAY_TYRANNUS5, pTyrannus);
                        }
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 15:
                        MoveSkeletHorde();
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 1;
                        break;    
                    case 16:
                        NeedTele();
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                    case 17:
                        TyrannusIntroEndHorde();
                        if(Creature* pTyrannus =  me->GetMap()->GetCreature(m_uiTyrannusGuid))
                        //DoScriptText(SAY_TYRANNUS5, pTyrannus);

                        instance->SetBossState(DATA_GARFROST, SPECIAL);
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 2000;
                        break;
                    case 18:
                        DoScriptText(SAY_SPEECH_SYLVANAS2, me);
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 1500;
                        break;
                    case 19:
                        CastSpellDiedSkelet();                        
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 3500;
                        break;
                                            case 20:
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 3000;
                        break;
                    case 21:
                        
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 2000;
                        break;
                            case 22:
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                            case 23:
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                                    case 24:
        
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 100;
                        break;
                                    case 25:
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 300;
                        break;
                case 26:
                        DoScriptText(SAY_SPEECH_SYLVANAS3, me);
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 3500;
                        break;
                    case 27:
                        DoScriptText(SAY_SPEECH_SYLVANAS4, me);
                        ++m_uiIntro_Phase;
                        m_uiSpeech_Timer = 5000;
                        break;
                    case 28:
                        ++m_uiIntro_Phase;
                        //m_bIsIntro = false;
                        m_bIsIntroEnd = true;
                        m_uiSpeech_Timer = 5000;
                        break;
        }    
        }else m_uiSpeech_Timer -= diff;
        
        }
        }
        
        
    };
    
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sylvanas_POSintroAI(creature);
    }    
    
    };

void AddSC_pit_of_saron()
{
    new mob_ymirjar_flamebearer();
    new mob_wrathbone_laborer();
    new mob_iceborn_protodrake();
    new mob_geist_ambusher();
    new spell_trash_mob_glacial_strike();
    //new npc_sylvanas_POSintro();
    //new npc_jaina_POSintro();
}
