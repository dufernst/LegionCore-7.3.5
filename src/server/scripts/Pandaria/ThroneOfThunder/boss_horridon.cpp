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

#include "throne_of_thunder.h"

enum eSpells
{
    //Horridon
    SPELL_RAMPAGE            = 136821,
    SPELL_TRIPLE_PUNCTURE    = 136767,
    SPELL_DOUBLE_SWIPE       = 136741,
    SPELL_HORRIDON_CHARGE    = 136769,
    //Jalak
    SPELL_BESTIAL_CRY        = 136817,

    //Gate Adds
    //Farrak
    SPELL_BLAZING_SUNLIGHT   = 136719,
    SPELL_SAND_TRAP          = 136724,
    SPELL_STONE_GAZE         = 136708,
    //Gurubashi
    SPELL_VENOM_BOLT_VOLLEY  = 136587,
    SPELL_LIVING_POISON      = 136645,
    SPELL_RENDING_CHARGE     = 136653,
    SPELL_RENDING_CHARGE_DMG = 136654,
    //Drakkari
    SPELL_MORTAL_STRIKE      = 136670,
    SPELL_FROZEN_ORB_SUM     = 136564,
    SPELL_FROZEN_BOLT_AURA   = 136572,
    SPELL_UNCONTROLLED_ABOM  = 136709, //Uncontrolled Abomination
    //Amani
    SPELL_SWIPE              = 136463,
    SPELL_CHAIN_LIGHTNING    = 136480,
    SPELL_LIGHTNING_NOVA_T_S = 136487,
    SPELL_LIGHTNING_NOVA     = 136489,
    SPELL_HEX_OF_CONFUSION   = 136512,
    SPELL_FIREBALL           = 136465,

    SPELL_DINO_FORM          = 137237,
    SPELL_DINO_MENDING       = 136797,

    SPELL_HEADACHE           = 137294,
    SPELL_CRACKED_SHELL      = 137240,
    SPELL_SUM_ORB_OF_CONTROL = 137447,
    SPELL_CONTROL_HORRIDON   = 137442,
    SPELL_VENOMOUS_EFFUSIONS = 136644,
};

enum sEvents
{
    //Horridon
    EVENT_TRIPLE_PUNCTURE    = 1,
    EVENT_DOUBLE_SWIPE       = 2,
    EVENT_CHARGES            = 3,
    //Jalak
    EVENT_INTRO              = 5,
    EVENT_BESTIAL_CRY        = 6,
    //Add events
    EVENT_BLAZING_SUNLIGHT   = 7,
    EVENT_SAND_TRAP          = 8,
    EVENT_STONE_GAZE         = 9,
    EVENT_VENOM_BOLT_VOLLEY  = 10,
    EVENT_RENDING_CHARGE     = 11,
    EVENT_MORTAL_STRIKE      = 12,
    EVENT_SUMMON_FROZEN_ORB  = 13,
    EVENT_SWIPE              = 14,
    EVENT_CHAIN_LIGHTNING    = 15,
    EVENT_SUMMON_TOTEM       = 16,
    EVENT_HEX_OF_CONFUSION   = 17,
    EVENT_FIREBALL           = 18,
    EVENT_DINO_MENDING       = 19,
    EVENT_VENOMOUS_EFFUSION  = 20,
    EVENT_CHANGE_PHASE       = 21,

    EVENT_RE_ATTACK          = 35,
    EVENT_OPEN_GATE          = 36,
    EVENT_UPDATE_WAVE        = 37,
    EVENT_UPDATE_WAVE2       = 38,
    EVENT_UPDATE_WAVE3       = 39,
    EVENT_UPDATE_WAVE4       = 40,
};

enum sAction
{
    //Jalak
    ACTION_INTRO             = 1,
    ACTION_RE_ATTACK         = 2,
    ACTION_SHAMAN_DISMAUNT   = 3,
    ACTION_RESET             = 4,
    ACTION_ACTIVE_GATE_EVENT = 5,
    ACTION_ENTERCOMBAT       = 6,
    ACTION_ATTACK_STOP       = 7,
    ACTION_CHARGE_TO_GATE    = 8,
    ACTION_STOP_GATE_EVENT   = 9,
    ACTION_NEW_PHASE         = 10, 
    ACTION_NEW_GATE_EVENT    = 11,
    ACTION_ACHIEVE_FAIL      = 12,
};

enum Phase
{
    PHASE_NULL,
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE,
    PHASE_FOUR,
    PHASE_FIVE,
};

//Farrak
Position farrakspawnpos[3] =
{
    {5523.08f, 5844.93f, 130.1096f, 3.9444f}, //center
    {5531.18f, 5837.05f, 130.0269f, 3.9444f}, //left
    {5514.92f, 5852.99f, 130.0348f, 3.9444f}, //right
};

Position farrakdestpos[3] =
{
    {5497.93f, 5819.61f, 130.0380f, 3.9036f},
    {5503.82f, 5812.21f, 130.0380f, 3.9036f},
    {5490.60f, 5826.27f, 130.0380f, 3.9036f},
};

//Gurubashi
Position gurubashispawnpos[3] =
{
    {5523.46f, 5661.59f, 130.0168f, 2.3265f},
    {5514.13f, 5652.66f, 130.0282f, 2.3262f},
    {5532.19f, 5669.82f, 130.0172f, 2.3265f},
};

Position gurubashidestpos[3] =
{
    {5498.91f, 5687.65f, 130.0377f, 2.3147f},
    {5490.29f, 5679.63f, 130.0377f, 2.3147f},
    {5506.46f, 5695.52f, 130.0377f, 2.3147f},
};

//Drakkari
Position drakkarispawnpos[3] =
{
    {5340.36f, 5662.42f, 130.1075f, 0.7604f},
    {5331.84f, 5671.37f, 130.0962f, 0.7604f},
    {5349.24f, 5653.73f, 130.0990f, 0.7604f},
};

Position drakkaridestpos[3] =
{
    {5365.96f, 5687.19f, 130.0361f, 0.7290f},
    {5357.18f, 5695.09f, 130.0361f, 0.7290f},
    {5374.31f, 5678.31f, 130.0361f, 0.7290f},
};

//Amani
Position amanispawnpos[3] =
{
    {5340.68f, 5845.15f, 130.1072f, 5.4524f},
    {5349.75f, 5853.64f, 130.0978f, 5.4524f},
    {5331.79f, 5835.75f, 130.0992f, 5.4524f},
};

Position amanidestpos[3] =
{
    {5365.37f, 5819.05f, 130.0378f, 5.4790f},
    {5374.40f, 5828.42f, 130.0378f, 5.4790f},
    {5356.72f, 5810.77f, 130.0378f, 5.4790f},
};

Position hchargedestpos[4] =
{
    {5497.175f, 5818.986f, 130.0373f}, //Farrak
    {5497.503f, 5687.455f, 130.0373f}, //Gurubashi
    {5365.979f, 5687.588f, 130.0371f}, //Drakkari
    {5365.813f, 5818.714f, 130.0371f}, //Amani
};

uint32 const gateentry[4] =
{
    GO_FARRAK_GATE,
    GO_GURUBASHI_GATE,
    GO_DRAKKARI_GATE,
    GO_AMANI_GATE,
};

enum achieve
{
    DINOMANCER_NOT_DIED = 1,
};

class boss_horridon : public CreatureScript
{
public:
    boss_horridon() : CreatureScript("boss_horridon") {}
    
    struct boss_horridonAI : public BossAI
    {
        boss_horridonAI(Creature* creature) : BossAI(creature, DATA_HORRIDON)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        Phase phase;
        bool dinomNotKilled;

        void Reset()
        {
            _Reset();
            ActiveOrOfflineGateEvent(false);
            DespawnSpecialSummons();
            ResetJalak();
            phase = PHASE_NULL;
            me->RemoveAurasDueToSpell(SPELL_HEADACHE);
            me->RemoveAurasDueToSpell(SPELL_CRACKED_SHELL);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_AGGRESSIVE);
            dinomNotKilled = true;
        }

        void ActiveOrOfflineGateEvent(bool state)
        {
            if (Creature* gatecontroller = me->GetCreature(*me, instance->GetGuidData(NPC_H_GATE_CONTROLLER)))
            {
                if (state)
                    gatecontroller->AI()->DoAction(ACTION_ACTIVE_GATE_EVENT);
                else
                    gatecontroller->AI()->DoAction(ACTION_RESET);
            }
        }

        void DespawnSpecialSummons()
        {
            std::list<Creature*>_sumlist;
            _sumlist.clear();
            GetCreatureListWithEntryInGrid(_sumlist, me, NPC_FROZEN_ORB, 200.0f);
            GetCreatureListWithEntryInGrid(_sumlist, me, NPC_LIGHTNING_NOVA_TOTEM, 200.0f);
            GetCreatureListWithEntryInGrid(_sumlist, me, NPC_LIVING_POISON, 200.0f);
            GetCreatureListWithEntryInGrid(_sumlist, me, NPC_VENOMOUS_EFFUSION, 200.0f);
            if (!_sumlist.empty())
                for (std::list<Creature*>::const_iterator itr = _sumlist.begin(); itr != _sumlist.end(); itr++)
                    (*itr)->DespawnOrUnsummon();

            std::list<GameObject*>_orbcontrolist;
            _orbcontrolist.clear();
            GetGameObjectListWithEntryInGrid(_orbcontrolist, me, GO_ORB_OF_CONTROL, 200.0f);
            for (std::list<GameObject*>::const_iterator Itr = _orbcontrolist.begin(); Itr != _orbcontrolist.end(); Itr++)
                (*Itr)->Delete();
        }

        void ResetJalak()
        {
            if (Creature* jalak = me->GetCreature(*me, instance->GetGuidData(NPC_JALAK)))
            {
                if (!jalak->isAlive())
                {
                    jalak->Respawn();
                    jalak->GetMotionMaster()->MoveTargetedHome();
                }
                else if (jalak->isInCombat())
                    jalak->AI()->EnterEvadeMode();
            }
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_ATTACK_STOP:
                me->InterruptNonMeleeSpells(true);
                events.Reset();
                me->StopAttack();
                if (Creature* gc = me->GetCreature(*me, instance->GetGuidData(NPC_H_GATE_CONTROLLER)))
                    gc->AI()->DoAction(ACTION_STOP_GATE_EVENT);
                break;
            case ACTION_CHARGE_TO_GATE:
                if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[phase - 1])))
                {
                    me->SetFacingToObject(gate);
                    me->GetMotionMaster()->MoveCharge(hchargedestpos[phase - 1].GetPositionX(), hchargedestpos[phase - 1].GetPositionY(), hchargedestpos[phase - 1].GetPositionZ(), 42.0f, 5);
                }
                break;
            case ACTION_NEW_PHASE:
                switch (phase)
                {
                case PHASE_ONE:
                    phase = PHASE_TWO;
                    break;
                case PHASE_TWO:
                    phase = PHASE_THREE;
                    break;
                case PHASE_THREE:
                    phase = PHASE_FOUR;
                    break;
                case PHASE_FOUR:
                    phase = PHASE_FIVE;
                    break;
                default:
                    break;
                }
                me->SetReactState(REACT_AGGRESSIVE);
                if (phase != PHASE_FIVE)
                {
                    if (Creature* gc = me->GetCreature(*me, instance->GetGuidData(NPC_H_GATE_CONTROLLER)))
                        gc->AI()->DoAction(ACTION_NEW_GATE_EVENT);
                }
                else if (phase == PHASE_FIVE)
                {
                    if (Creature* jalak = me->GetCreature(*me, instance->GetGuidData(NPC_JALAK)))
                        jalak->AI()->DoAction(ACTION_INTRO);
                }
                events.RescheduleEvent(EVENT_TRIPLE_PUNCTURE, urand(11000, 15000));
                events.RescheduleEvent(EVENT_DOUBLE_SWIPE, urand(17000, 20000));
                events.RescheduleEvent(EVENT_CHARGES, urand(50000, 60000));
                break;
            case ACTION_RE_ATTACK:
                me->SetReactState(REACT_AGGRESSIVE);
                ActiveOrOfflineGateEvent(true);
                events.RescheduleEvent(EVENT_TRIPLE_PUNCTURE, urand(11000, 15000));
                events.RescheduleEvent(EVENT_DOUBLE_SWIPE, urand(17000, 20000));
                events.RescheduleEvent(EVENT_CHARGES, urand(50000, 60000));
                break;
            case ACTION_ACHIEVE_FAIL:
                dinomNotKilled = false;
                break;
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE && pointId == 5)
            {
                if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[phase - 1])))
                    gate->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                DoCast(me, SPELL_HEADACHE, true);
            }
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            phase = PHASE_ONE;
            dinomNotKilled = true;
            ActiveOrOfflineGateEvent(true);
            events.RescheduleEvent(EVENT_TRIPLE_PUNCTURE, urand(11000, 15000));
            events.RescheduleEvent(EVENT_DOUBLE_SWIPE, urand(17000, 20000));
            events.RescheduleEvent(EVENT_CHARGES, urand(50000, 60000));
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(30) && phase != PHASE_FIVE)
            {
                phase = PHASE_FIVE;
                if (Creature* jalak = me->GetCreature(*me, instance->GetGuidData(NPC_JALAK)))
                    jalak->AI()->DoAction(ACTION_INTRO);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Creature* jalak = me->GetCreature(*me, instance->GetGuidData(NPC_JALAK)))
            {
                if (jalak->isAlive())
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                else
                {
                    _JustDied();
                    DespawnSpecialSummons();
                    ActiveOrOfflineGateEvent(false);
                }
            }
        }

        uint32 GetData(uint32 type) const
        {
            if (type == DATA_GET_PHASE)
                return uint32(phase);

            if (type == DINOMANCER_NOT_DIED)
                return dinomNotKilled;

            return 0;
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
                case EVENT_TRIPLE_PUNCTURE:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_TRIPLE_PUNCTURE);
                    events.RescheduleEvent(EVENT_TRIPLE_PUNCTURE, urand(11000, 15000));
                    break;
                case EVENT_DOUBLE_SWIPE:
                    DoCast(me, SPELL_DOUBLE_SWIPE);
                    events.RescheduleEvent(EVENT_DOUBLE_SWIPE, urand(17000, 20000));
                    break;
                case EVENT_CHARGES:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                        DoCast(target, SPELL_HORRIDON_CHARGE);
                    events.RescheduleEvent(EVENT_CHARGES, urand(50000, 60000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_horridonAI(creature);
    }
};

class boss_jalak : public CreatureScript
{
public:
    boss_jalak() : CreatureScript("boss_jalak") {}
    
    struct boss_jalakAI : public CreatureAI
    {
        boss_jalakAI(Creature* creature) : CreatureAI(creature)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
            ResetHorridon();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void ResetHorridon()
        {
            if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
            {
                if (!horridon->isAlive())
                {
                    horridon->Respawn();
                    horridon->GetMotionMaster()->MoveTargetedHome();
                }
            }
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_BESTIAL_CRY, 5000);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
                return;

            if (pointId == 0)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 150.0f);
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_INTRO)
                me->GetMotionMaster()->MoveJump(5433.32f, 5745.32f, 129.6066f, 25.0f, 25.0f, 0);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
            {
                if (horridon->isAlive())
                    horridon->AddAura(SPELL_RAMPAGE, horridon);
                else
                {
                    horridon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    instance->SetBossState(DATA_HORRIDON, DONE);
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_BESTIAL_CRY)
                {
                    DoCast(me, SPELL_BESTIAL_CRY);
                    events.RescheduleEvent(EVENT_BESTIAL_CRY, 10000);
                }
            }
            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jalakAI(creature);
    }
};

//90010
class npc_horridon_gate_controller : public CreatureScript
{
public:
    npc_horridon_gate_controller() : CreatureScript("npc_horridon_gate_controller") {}

    struct npc_horridon_gate_controllerAI : public ScriptedAI
    {
        npc_horridon_gate_controllerAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* instance;
        SummonList summon;
        EventMap events;
        uint8 gatenum;

        void Reset()
        {
            events.Reset();
            summon.DespawnAll();
            gatenum = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_RESET:
                events.Reset();
                summon.DespawnAll();
                gatenum = 0;
                break;
            case ACTION_STOP_GATE_EVENT:
                events.Reset();
                break;
            case ACTION_ACTIVE_GATE_EVENT:
                events.RescheduleEvent(EVENT_OPEN_GATE, 30000);
                break;
            case ACTION_NEW_GATE_EVENT:
                events.RescheduleEvent(EVENT_OPEN_GATE, 40000);
                break;
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
                case EVENT_OPEN_GATE:
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                    {
                        gatenum = uint8(horridon->AI()->GetData(DATA_GET_PHASE) - 1);
                        if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[gatenum])))
                        {
                            gate->UseDoorOrButton(10);
                            switch (gatenum)
                            {
                            case 0: //Farrak
                                if (Creature* add = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_FARRAKI_SKIRMISHER, farrakspawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                break;
                            case 1: //Gurubasi
                                if (Creature* add = me->SummonCreature(NPC_GURUBASHI_BLOODLORD, gurubashispawnpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 0);
                                break;
                            case 2: //Drakkari
                                if (Creature* add = me->SummonCreature(NPC_RISEN_DRAKKARI_CHAMPION, drakkarispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_RISEN_DRAKKARI_WARRIOR, drakkarispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                break;
                            case 3: //Amani
                                if (Creature* add = me->SummonCreature(NPC_AMANISHI_FLAME_CASTER, amanispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_AMANISHI_PROTECTOR, amanispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_UPDATE_WAVE, 20000);
                    break;
                case EVENT_UPDATE_WAVE:
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                    {
                        gatenum = uint8(horridon->AI()->GetData(DATA_GET_PHASE) - 1);
                        if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[gatenum])))
                        {
                            gate->UseDoorOrButton(10);
                            switch (gatenum)
                            {
                            case 0: //Farrak
                                if (Creature* add = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* badd = me->SummonCreature(NPC_FARRAKI_WASTEWALKER, farrakdestpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            case 1: //Gurubashi
                                if (Creature* add = me->SummonCreature(NPC_GURUBASHI_BLOODLORD, gurubashispawnpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 0);
                                if (Creature* badd = me->SummonCreature(NPC_GURUBASHI_VENOM_PRIEST, gurubashidestpos[urand(1, 2)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            case 2: //Drakkari
                                if (Creature* add = me->SummonCreature(NPC_RISEN_DRAKKARI_CHAMPION, drakkarispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_RISEN_DRAKKARI_WARRIOR, drakkarispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* badd = me->SummonCreature(NPC_DRAKKARI_FROZEN_WARLORD, drakkaridestpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            case 3: //Amani
                                if (Creature* add = me->SummonCreature(NPC_AMANISHI_FLAME_CASTER, amanispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_AMANISHI_PROTECTOR, amanispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* badd = me->SummonCreature(NPC_AMANI_WARBEAR, amanidestpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_UPDATE_WAVE2, 20000);
                    break;
                case EVENT_UPDATE_WAVE2:
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                    {
                        gatenum = uint8(horridon->AI()->GetData(DATA_GET_PHASE) - 1);
                        if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[gatenum])))
                        {
                            gate->UseDoorOrButton(10);
                            switch (gatenum)
                            {
                            case 0: //Farrak
                                if (Creature* add = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* badd = me->SummonCreature(NPC_FARRAKI_WASTEWALKER, farrakdestpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                if (Creature* badd2 = me->SummonCreature(NPC_FARRAKI_WASTEWALKER, farrakdestpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd2->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            case 1: //Gurubashi
                                if (Creature* add = me->SummonCreature(NPC_GURUBASHI_BLOODLORD, gurubashispawnpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 0);
                                if (Creature* badd = me->SummonCreature(NPC_GURUBASHI_VENOM_PRIEST, gurubashidestpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                if (Creature* badd2 = me->SummonCreature(NPC_GURUBASHI_VENOM_PRIEST, gurubashidestpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd2->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            case 2: //Drakkari
                                if (Creature* add = me->SummonCreature(NPC_RISEN_DRAKKARI_CHAMPION, drakkarispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_RISEN_DRAKKARI_WARRIOR, drakkarispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* badd = me->SummonCreature(NPC_DRAKKARI_FROZEN_WARLORD, drakkaridestpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                if (Creature* badd2 = me->SummonCreature(NPC_DRAKKARI_FROZEN_WARLORD, drakkaridestpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd2->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            case 3: //Amani
                                if (Creature* add = me->SummonCreature(NPC_AMANISHI_FLAME_CASTER, amanispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_AMANISHI_PROTECTOR, amanispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* badd = me->SummonCreature(NPC_AMANI_WARBEAR, amanidestpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd->AI()->DoAction(ACTION_ENTERCOMBAT);
                                if (Creature* badd2 = me->SummonCreature(NPC_AMANI_WARBEAR, amanidestpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    badd2->AI()->DoAction(ACTION_ENTERCOMBAT);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_UPDATE_WAVE3, 20000);
                    break;
                case EVENT_UPDATE_WAVE3:
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                    {
                        gatenum = uint8(horridon->AI()->GetData(DATA_GET_PHASE) - 1);
                        if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[gatenum])))
                        {
                            gate->UseDoorOrButton(10);
                            switch (gatenum)
                            {
                            case 0: //Farrak
                                if (Creature* add = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_FARRAKI_SKIRMISHER, farrakspawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* dino = me->SummonCreature(NPC_ZANDALARI_DINOMANCER, farrakdestpos[urand(0, 2)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    dino->AI()->DoZoneInCombat(dino, 150.0f);
                                break;
                            case 1: //Gurubashi
                                if (Creature* add = me->SummonCreature(NPC_GURUBASHI_BLOODLORD, gurubashispawnpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 0);
                                if (Creature* dino = me->SummonCreature(NPC_ZANDALARI_DINOMANCER, gurubashidestpos[urand(1, 2)], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    dino->AI()->DoZoneInCombat(dino, 150.0f);
                                break;
                            case 2: //Drakkari
                                if (Creature* add = me->SummonCreature(NPC_RISEN_DRAKKARI_CHAMPION, drakkarispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_RISEN_DRAKKARI_WARRIOR, drakkarispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* dino = me->SummonCreature(NPC_ZANDALARI_DINOMANCER, drakkaridestpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    dino->AI()->DoZoneInCombat(dino, 150.0f);
                                break;
                            case 3: //Amani
                                if (Creature* add = me->SummonCreature(NPC_AMANISHI_FLAME_CASTER, amanispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_AMANISHI_PROTECTOR, amanispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                if (Creature* dino = me->SummonCreature(NPC_ZANDALARI_DINOMANCER, amanidestpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    dino->AI()->DoZoneInCombat(dino, 150.0f);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_UPDATE_WAVE4, 20000);
                    break;
                case EVENT_UPDATE_WAVE4:
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                    {
                        gatenum = uint8(horridon->AI()->GetData(DATA_GET_PHASE) - 1);
                        if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetGuidData(gateentry[gatenum])))
                        {
                            gate->UseDoorOrButton(10);
                            switch (gatenum)
                            {
                            case 0: //Farrak
                                if (Creature* add = me->SummonCreature(NPC_SULLITHUZ_STONEGAZER, farrakspawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_FARRAKI_SKIRMISHER, farrakspawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                break;
                            case 1: //Gurubashi
                                if (Creature* add = me->SummonCreature(NPC_GURUBASHI_BLOODLORD, gurubashispawnpos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 0);
                                break;
                            case 2: //Drakkari
                                if (Creature* add = me->SummonCreature(NPC_RISEN_DRAKKARI_CHAMPION, drakkarispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_RISEN_DRAKKARI_WARRIOR, drakkarispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                break;
                            case 3: //Amani
                                if (Creature* add = me->SummonCreature(NPC_AMANISHI_FLAME_CASTER, amanispawnpos[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add->AI()->SetData(DATA_SEND_DEST_POS, 1);
                                if (Creature* add2 = me->SummonCreature(NPC_AMANISHI_PROTECTOR, amanispawnpos[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000))
                                    add2->AI()->SetData(DATA_SEND_DEST_POS, 2);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_UPDATE_WAVE4, 20000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_horridon_gate_controllerAI(creature);
    }
};

//Farrak: big - 69175, 69173, small - 69172.
//Gurubashi: big - 69164, 69314, small - 69167.
//Drakkari: big - 69178, special summons - 69268, small - 69185.
//Amani: big - 69177, 69176, small - 69168, 69169
class npc_generic_gate_add: public CreatureScript
{
public:
    npc_generic_gate_add() : CreatureScript("npc_generic_gate_add") {}

    struct npc_generic_gate_addAI : public ScriptedAI
    {
        npc_generic_gate_addAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            if (me->GetEntry() == NPC_RISEN_DRAKKARI_CHAMPION)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            }
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
            switch (me->GetEntry())
            {
            case NPC_FROZEN_ORB:
                me->SetDisplayId(11686);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                DoZoneInCombat(me, 100.0f);
                DoCast(me, SPELL_FROZEN_BOLT_AURA, true);
                break;
            case NPC_RISEN_DRAKKARI_CHAMPION:
                FindAndAttackRandomPlayer();
                break;
            }
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (me->GetEntry() == NPC_VENOMOUS_EFFUSION)
                DoAction(ACTION_ENTERCOMBAT);
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_SEND_DEST_POS)
            {
                switch (me->GetEntry())
                {
                case NPC_FARRAKI_SKIRMISHER:
                case NPC_SULLITHUZ_STONEGAZER:
                    switch (data)
                    {
                    case 1:
                        me->GetMotionMaster()->MoveJump(farrakdestpos[1].GetPositionX(), farrakdestpos[1].GetPositionY(), farrakdestpos[1].GetPositionZ(), 7.0f, 0.0f, 1);
                        break;
                    case 2:
                        me->GetMotionMaster()->MoveJump(farrakdestpos[2].GetPositionX(), farrakdestpos[2].GetPositionY(), farrakdestpos[2].GetPositionZ(), 7.0f, 0.0f, 2);
                        break;
                    }
                    break;
                case NPC_GURUBASHI_BLOODLORD:
                    if (!data)
                        me->GetMotionMaster()->MoveJump(gurubashidestpos[0].GetPositionX(), gurubashidestpos[0].GetPositionY(), gurubashidestpos[0].GetPositionZ(), 7.0f, 0.0f, 0);
                    break;
                case NPC_RISEN_DRAKKARI_CHAMPION:
                case NPC_RISEN_DRAKKARI_WARRIOR:
                    switch (data)
                    {
                    case 1:
                        me->GetMotionMaster()->MoveJump(drakkaridestpos[1].GetPositionX(), drakkaridestpos[1].GetPositionY(), drakkaridestpos[1].GetPositionZ(), 7.0f, 0.0f, 1);
                        break;
                    case 2:
                        me->GetMotionMaster()->MoveJump(drakkaridestpos[2].GetPositionX(), drakkaridestpos[2].GetPositionY(), drakkaridestpos[2].GetPositionZ(), 7.0f, 0.0f, 2);
                        break;
                    }
                    break;
                case NPC_AMANISHI_FLAME_CASTER:
                case NPC_AMANISHI_PROTECTOR:
                    switch (data)
                    {
                    case 1:
                        me->GetMotionMaster()->MoveJump(amanidestpos[1].GetPositionX(), amanidestpos[1].GetPositionY(), amanidestpos[1].GetPositionZ(), 7.0f, 0.0f, 1);
                        break;
                    case 2:
                        me->GetMotionMaster()->MoveJump(amanidestpos[2].GetPositionX(), amanidestpos[2].GetPositionY(), amanidestpos[2].GetPositionZ(), 7.0f, 0.0f, 1);
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 100.0f);
            }
        }

        void FindAndAttackRandomPlayer()
        {
            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
            {
                me->AddThreat(target, 50000000.0f);
                me->SetReactState(REACT_AGGRESSIVE);
                me->Attack(target, true);
                me->GetMotionMaster()->MoveChase(target);
            }
        }

        void EnterCombat(Unit* who)
        {
            switch (me->GetEntry())
            {
            //Farrak gate
            case NPC_FARRAKI_WASTEWALKER:
                events.RescheduleEvent(EVENT_BLAZING_SUNLIGHT, 10000);
                events.RescheduleEvent(EVENT_SAND_TRAP, 7000);
                break;
            case NPC_SULLITHUZ_STONEGAZER:
                events.RescheduleEvent(EVENT_STONE_GAZE, 10000);
                break;
            //Gurubashi
            case NPC_GURUBASHI_VENOM_PRIEST:
            case NPC_VENOMOUS_EFFUSION:
                events.RescheduleEvent(EVENT_VENOM_BOLT_VOLLEY, 10000);
                events.RescheduleEvent(EVENT_VENOMOUS_EFFUSION, 20000);
                break;
            case NPC_GURUBASHI_BLOODLORD:
                events.RescheduleEvent(EVENT_RENDING_CHARGE, 10000);
                break;
            //Drakkari
            case NPC_DRAKKARI_FROZEN_WARLORD:
                events.RescheduleEvent(EVENT_MORTAL_STRIKE, 6000);
                events.RescheduleEvent(EVENT_SUMMON_FROZEN_ORB, 15000);
                break;
            case NPC_RISEN_DRAKKARI_CHAMPION:
                DoCast(me, SPELL_UNCONTROLLED_ABOM, true);
                break;
            //Amani
            case NPC_AMANI_WARBEAR:
                events.RescheduleEvent(EVENT_SWIPE, 5000);
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 10000);
                events.RescheduleEvent(EVENT_SUMMON_TOTEM, 20000);
                events.RescheduleEvent(EVENT_HEX_OF_CONFUSION, 30000);
                break;
            case NPC_AMANISHI_FLAME_CASTER:
                events.RescheduleEvent(EVENT_FIREBALL, 4000);
                break;
            }
        }

        void JustDied(Unit* killer)
        {
            if (me->GetEntry() == NPC_AMANI_WARBEAR)
                if (Vehicle* vehicle = me->GetVehicleKit())
                    if (Unit* passenger = vehicle->GetPassenger(0))
                        passenger->ToCreature()->AI()->DoAction(ACTION_SHAMAN_DISMAUNT);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_RE_ATTACK:
                events.RescheduleEvent(EVENT_RE_ATTACK, 1500);
                break;
            case ACTION_SHAMAN_DISMAUNT:
                me->ExitVehicle();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 100.0f);
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 10000);
                events.RescheduleEvent(EVENT_SUMMON_TOTEM, 20000);
                events.RescheduleEvent(EVENT_HEX_OF_CONFUSION, 30000);
                break;
            case ACTION_ENTERCOMBAT:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 100.0f);
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                //Farrak
                case EVENT_BLAZING_SUNLIGHT:
                    DoCast(me, SPELL_BLAZING_SUNLIGHT);
                    events.RescheduleEvent(EVENT_BLAZING_SUNLIGHT, 10000);
                    break;
                case EVENT_SAND_TRAP:
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                            horridon->SummonCreature(NPC_SAND_TRAP, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                    events.RescheduleEvent(EVENT_SAND_TRAP, 15000);
                    break;
                case EVENT_STONE_GAZE:
                    me->StopAttack();
                    DoCast(me, SPELL_STONE_GAZE);
                    break;
                //Gurubashi
                case EVENT_VENOM_BOLT_VOLLEY:
                    DoCast(me, SPELL_VENOM_BOLT_VOLLEY);
                    events.RescheduleEvent(EVENT_VENOM_BOLT_VOLLEY, 20000);
                    break;
                case EVENT_VENOMOUS_EFFUSION:
                    DoCast(me, SPELL_VENOMOUS_EFFUSIONS);
                    events.RescheduleEvent(EVENT_VENOMOUS_EFFUSION, 25000);
                    break;
                case EVENT_RENDING_CHARGE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 55.0f, true))
                    {
                        me->StopAttack();
                        DoCast(target, SPELL_RENDING_CHARGE);
                    }
                    break;
                //Drakkari
                case EVENT_MORTAL_STRIKE:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_MORTAL_STRIKE);
                    events.RescheduleEvent(EVENT_MORTAL_STRIKE, 10000);
                    break;
                case EVENT_SUMMON_FROZEN_ORB:
                    DoCast(me, SPELL_FROZEN_ORB_SUM);
                    events.RescheduleEvent(EVENT_SUMMON_FROZEN_ORB, 20000);
                    break;
                //Amani
                case EVENT_SWIPE:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_SWIPE);
                    events.RescheduleEvent(EVENT_SWIPE, 10000);
                    break;
                //Amani Beast Shaman
                case EVENT_CHAIN_LIGHTNING:
                    if (me->GetEntry() == NPC_AMANI_WARBEAR)
                    {
                        if (Vehicle* vehicle = me->GetVehicleKit())
                            if (Unit* passenger = vehicle->GetPassenger(0))
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                                    passenger->CastSpell(target, SPELL_CHAIN_LIGHTNING);
                    }
                    else
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            DoCast(target, SPELL_CHAIN_LIGHTNING);
                    }
                    events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, 10000);
                    break;
                case EVENT_SUMMON_TOTEM:
                    if (me->GetEntry() == NPC_AMANI_WARBEAR)
                    {
                        if (Vehicle* vehicle = me->GetVehicleKit())
                            if (Unit* passenger = vehicle->GetPassenger(0))
                                passenger->CastSpell(passenger, SPELL_LIGHTNING_NOVA_T_S);
                    }
                    else
                        DoCast(me, SPELL_LIGHTNING_NOVA_T_S);
                    events.RescheduleEvent(EVENT_SUMMON_TOTEM, 20000);
                    break;
                case EVENT_HEX_OF_CONFUSION:
                    if (me->GetEntry() == NPC_AMANI_WARBEAR)
                    {
                        if (Vehicle* vehicle = me->GetVehicleKit())
                            if (Unit* passenger = vehicle->GetPassenger(0))
                                passenger->CastSpell(passenger, SPELL_HEX_OF_CONFUSION);
                    }
                    else
                        DoCast(me, SPELL_HEX_OF_CONFUSION);
                    events.RescheduleEvent(EVENT_HEX_OF_CONFUSION, 30000);
                    break;
                //
                case EVENT_FIREBALL:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 45.0f, true))
                        DoCast(target, SPELL_FIREBALL);
                    events.RescheduleEvent(EVENT_FIREBALL, 6000);
                    break;
                //Special
                case EVENT_RE_ATTACK:
                    me->SetReactState(REACT_AGGRESSIVE);
                    switch (me->GetEntry())
                    {
                    case NPC_SULLITHUZ_STONEGAZER:
                        events.RescheduleEvent(EVENT_STONE_GAZE, 10000);
                        break;
                    case NPC_GURUBASHI_BLOODLORD:
                        events.RescheduleEvent(EVENT_RENDING_CHARGE, 12000);
                        break;
                    default:
                        break;
                    }
                    break;
                }
            }
            if (me->GetEntry() != NPC_AMANISHI_FLAME_CASTER)
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_generic_gate_addAI(creature);
    }
};

//69221
class npc_zandalari_dinomancer : public CreatureScript
{
public:
    npc_zandalari_dinomancer() : CreatureScript("npc_zandalari_dinomancer") {}

    struct npc_zandalari_dinomancerAI : public ScriptedAI
    {
        npc_zandalari_dinomancerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* instance;
        EventMap events;
        bool done;

        void Reset()
        {
            done = false;
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_DINO_MENDING, 5000);
        }

        void OnInterruptCast(Unit* /*caster*/, uint32 spellId, uint32 curSpellID, uint32 /*schoolMask*/)
        {
            if (curSpellID == SPELL_DINO_MENDING)
                events.RescheduleEvent(EVENT_DINO_MENDING, 5000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (auto horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                if (horridon->IsAIEnabled)
                    horridon->AI()->DoAction(ACTION_ACHIEVE_FAIL);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(50) && !done)
            {
                done = true;
                events.CancelEvent(EVENT_DINO_MENDING);
                me->InterruptNonMeleeSpells(true);
                DoCast(me, SPELL_SUM_ORB_OF_CONTROL, true);
                DoCast(me, SPELL_DINO_FORM, true);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat(me, 150.0f);
                if (me->getVictim())
                    me->GetMotionMaster()->MoveChase(me->getVictim());
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_DINO_MENDING)
                {
                    if (Creature* horridon = me->GetCreature(*me, instance->GetGuidData(NPC_HORRIDON)))
                        DoCast(horridon, SPELL_DINO_MENDING);
                    events.RescheduleEvent(EVENT_DINO_MENDING, 30000);
                }
            }
            if (me->HasAura(SPELL_DINO_FORM))
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_zandalari_dinomancerAI(creature);
    }
};

//69346
class npc_sand_trap : public CreatureScript
{
public:
    npc_sand_trap() : CreatureScript("npc_sand_trap") {}

    struct npc_sand_trapAI : public ScriptedAI
    {
        npc_sand_trapAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }
        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_SAND_TRAP, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sand_trapAI(creature);
    }
};

//69313
class npc_living_poison : public CreatureScript
{
public:
    npc_living_poison() : CreatureScript("npc_living_poison") {}

    struct npc_living_poisonAI : public ScriptedAI
    {
        npc_living_poisonAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }
        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_LIVING_POISON, true);
            me->GetMotionMaster()->MoveRandom(10.0f);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_living_poisonAI(creature);
    }
};

//69215
class npc_lightning_nova_totem : public CreatureScript
{
public:
    npc_lightning_nova_totem() : CreatureScript("npc_lightning_nova_totem") {}

    struct npc_lightning_nova_totemAI : public ScriptedAI
    {
        npc_lightning_nova_totemAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
        }
        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_LIGHTNING_NOVA, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lightning_nova_totemAI(creature);
    }
};

//218374
class go_orb_of_control : public GameObjectScript
{
public:
    go_orb_of_control() : GameObjectScript("go_orb_of_control") { }


    bool OnGossipHello(Player* player, GameObject* go)
    {
        InstanceScript* pInstance = (InstanceScript*)go->GetInstanceScript();

        if (!pInstance)
            return false;

        if (Creature* horridon = player->GetCreature(*player, pInstance->GetGuidData(NPC_HORRIDON)))
        {
            go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
            player->CastSpell(horridon, SPELL_CONTROL_HORRIDON);
            go->Delete();
        }
        return true;
    }
};

class spell_horridon_charge : public SpellScriptLoader
{
public:
    spell_horridon_charge() : SpellScriptLoader("spell_horridon_charge") { }
    
    class spell_horridon_charge_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_horridon_charge_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_DOUBLE_SWIPE);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_horridon_charge_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_horridon_charge_AuraScript();
    }
};

//136719
class spell_blazing_sunlight : public SpellScriptLoader
{
public:
    spell_blazing_sunlight() : SpellScriptLoader("spell_blazing_sunlight") { }

    class spell_blazing_sunlight_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_blazing_sunlight_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (targets.size() > 1)
                targets.resize(1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_blazing_sunlight_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_blazing_sunlight_SpellScript::_FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_blazing_sunlight_SpellScript();
    }
};

//136708
class spell_stone_gaze : public SpellScriptLoader
{
public:
    spell_stone_gaze() : SpellScriptLoader("spell_stone_gaze") { }

    class spell_stone_gaze_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_stone_gaze_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature())
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_stone_gaze_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_stone_gaze_SpellScript();
    }
};

//136653
class spell_rending_charge : public SpellScriptLoader
{
public:
    spell_rending_charge() : SpellScriptLoader("spell_rending_charge") { }

    class spell_rending_charge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rending_charge_SpellScript);

        ObjectGuid targetGuid;

        SpellCastResult CheckTarget()
        {
            if (GetExplTargetUnit())
                targetGuid = GetExplTargetUnit()->GetGUID();
            return SPELL_CAST_OK;
        }

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (Unit* _target = GetCaster()->GetUnit(*GetCaster(), targetGuid))
                    if (_target->isAlive())
                        GetCaster()->CastSpell(_target, SPELL_RENDING_CHARGE_DMG, true);
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK);
            }
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_rending_charge_SpellScript::CheckTarget);
            AfterCast += SpellCastFn(spell_rending_charge_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rending_charge_SpellScript();
    }
};

//137442
class spell_control_horridon : public SpellScriptLoader
{
public:
    spell_control_horridon() : SpellScriptLoader("spell_control_horridon") { }

    class spell_control_horridon_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_control_horridon_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->ToCreature())
                GetTarget()->ToCreature()->AI()->DoAction(ACTION_ATTACK_STOP);
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget() && GetTarget()->ToCreature())
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    GetTarget()->ToCreature()->AI()->DoAction(ACTION_CHARGE_TO_GATE);
                else if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_CANCEL || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                    GetTarget()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_control_horridon_AuraScript::OnApply, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_control_horridon_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_control_horridon_AuraScript();
    }
};

//137294
class spell_headache : public SpellScriptLoader
{
public:
    spell_headache() : SpellScriptLoader("spell_headache") { }

    class spell_headache_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_headache_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget() && GetTarget()->ToCreature())
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                    GetTarget()->ToCreature()->AI()->DoAction(ACTION_NEW_PHASE);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_headache_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_headache_AuraScript();
    }
};

class ExactDistanceCheck
{
public:
    ExactDistanceCheck(WorldObject* source, float dist) : _source(source), _dist(dist) {}

    bool operator()(WorldObject* unit)
    {
        return _source->GetExactDist2d(unit) > _dist;
    }
private:
    WorldObject* _source;
    float _dist;
};

//136723
class spell_sand_trap : public SpellScriptLoader
{
public:
    spell_sand_trap() : SpellScriptLoader("spell_sand_trap") { }

    class spell_sand_trap_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sand_trap_SpellScript);

        void ScaleRange(std::list<WorldObject*>& targets)
        {
            targets.remove_if(ExactDistanceCheck(GetCaster(), 2.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE)));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sand_trap_SpellScript::ScaleRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sand_trap_SpellScript();
    }
};

//22677
class achievement_cretaceous_collector : public AchievementCriteriaScript
{
public:
    achievement_cretaceous_collector() : AchievementCriteriaScript("achievement_cretaceous_collector") {}

    bool OnCheck(Player* /*player*/, Unit* target) override
    {
        if (!target)
            return false;

        if (auto boss = target->ToCreature())
            if (boss->IsAIEnabled && boss->AI()->GetData(DINOMANCER_NOT_DIED))
                return true;

        return false;
    }
};

void AddSC_boss_horridon()
{
    new boss_horridon();
    new boss_jalak();
    //new npc_horridon_gate_controller();
    new npc_generic_gate_add();
    new npc_zandalari_dinomancer();
    new npc_sand_trap();
    new npc_living_poison();
    new go_orb_of_control();
    new npc_lightning_nova_totem();
    new spell_horridon_charge();
    new spell_blazing_sunlight();
    new spell_stone_gaze();
    new spell_rending_charge();
    new spell_control_horridon();
    new spell_headache();
    new spell_sand_trap();
    new achievement_cretaceous_collector();
}
