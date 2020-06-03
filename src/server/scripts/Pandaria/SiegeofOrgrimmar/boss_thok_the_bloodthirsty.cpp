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

enum eSpells
{
    //Phase 1
    SPELL_FEARSOME_ROAR      = 143426, 
    SPELL_DEAFENING_SCREECH  = 143343,
    SPELL_TAIL_LASH          = 143428, 
    SPELL_SHOCK_BLAST        = 143707,
    SPELL_BLOODIED           = 143452,
    SPELL_POWER_REGEN        = 143345,
    SPELL_ACCELERATION       = 143411,
    //
    SPELL_ACID_BREATH        = 143780,
    SPELL_CORROSIVE_BLOOD    = 143791,
    //
    SPELL_FREEZING_BREATH    = 143773,
    SPELL_ICY_BLOOD          = 143800,
    SPELL_SUMMON_ICE_TOMB    = 136929,
    SPELL_FROZEN_SOLID       = 143777,
    //
    SPELL_SCORCHING_BREATH   = 143767,
    SPELL_BURNING_BLOOD      = 143783,
    SPELL_BURNING_BLOOD_ADMG = 143784,

    //Phase 2
    SPELL_BLOOD_FRENZY       = 143440,
    SPELL_BLOOD_FRENZY_TE    = 143442,
    SPELL_BLOOD_FRENZY_KB    = 144067, 
    SPELL_FIXATE_PL          = 143445,
    SPELL_FIXATE_IM          = 146540,
    SPELL_FIXATE_PR          = 146581,

    SPELL_ENRAGE_KJ          = 145974,
    SPELL_UNLOCKING          = 146589, 

    SPELL_ENRAGE             = 26662,

    SPELL_VAMPIRIC_FRENZY    = 147980,

    SPELL_CANNON_BALL        = 147906,
    SPELL_CANNON_BALL_ATDMG  = 147607,
    SPELL_CANNON_BALL_AT_A   = 147609,
    SPELL_CANNON_BALL_DESTD  = 147662,
    SPELL_FLAME_COATING      = 144115,
    SPELL_R_WATERS           = 144117,
    SPELL_HEAL               = 149232,
};

enum Events
{
    //Default events
    EVENT_FEARSOME_ROAR      = 1,
    EVENT_TAIL_LASH          = 2,
    EVENT_SHOCK_BLAST        = 3,
    //Extra events
    EVENT_ACID_BREATH        = 4,
    EVENT_CORROSIVE_BLOOD    = 5,
    //
    EVENT_FREEZING_BREATH    = 6,
    EVENT_ICY_BLOOD          = 7,
    //
    EVENT_SCORCHING_BREATH   = 8,
    EVENT_BURNING_BLOOD      = 9,
    //Special events
    EVENT_GO_TO_PRISONER     = 10,
    EVENT_FIXATE             = 11,
    EVENT_KILL_PRISONER      = 12,
    EVENT_MOVING             = 13,

    //Summon events
    EVENT_ENRAGE_KJ          = 14,
    EVENT_MOVE_TO_CENTER     = 15,
    EVENT_MOVE_TO_THOK       = 16,
    EVENT_CHECK_TPLAYER      = 17,
    EVENT_Y_CHARGE           = 18,
    EVENT_PRE_Y_CHARGE       = 19,
    EVENT_VAMPIRIC_FRENZY    = 20,
    EVENT_R_WATERS           = 21,
    EVENT_CHECK_PROGRESS     = 22,
};

enum Action
{
    ACTION_PHASE_TWO           = 1,
    ACTION_PHASE_ONE_ACID      = 2,
    ACTION_PHASE_ONE_FROST     = 3,
    ACTION_PHASE_ONE_FIRE      = 4,
    ACTION_FIXATE              = 5,
    ACTION_START_FIXATE        = 6,
    ACTION_FREEDOM             = 7,
    ACTION_SUMMON_CAPTIVE_BAT  = 8,
    ACTION_SUMMON_STARVED_EYTI = 9,
};

uint32 prisonersentry[3] =
{
    NPC_AKOLIK, 
    NPC_MONTAK, 
    NPC_WATERSPEAKER_GORAI, 
};

Position fpos[3] =
{
    {1273.30f, -5123.47f, -290.4582f, 2.9432f},
    {1138.35f, -5100.04f, -290.4619f, 6.1209f},
    {1220.41f, -5045.36f, -290.4579f, 4.5030f},
};

Position kjspawnpos = {1285.03f, -5059.10f, -290.9505f, 3.6988f}; 
Position cpos = {1208.61f, -5106.27f, -289.939f, 0.526631f};

Position ccbatspawnpos[7] =
{
    {1257.04f, -5169.60f, -280.0894f, 2.2238f},
    {1251.81f, -5162.37f, -280.0894f, 2.2238f},
    {1251.15f, -5174.29f, -280.0894f, 2.2238f},
    {1245.83f, -5166.56f, -280.0894f, 2.2238f},
    {1238.66f, -5181.14f, -280.0894f, 2.2238f},
    {1233.69f, -5172.25f, -280.0894f, 2.2238f},
    {1244.47f, -5158.36f, -280.0894f, 2.2238f},
};

Position sumyetipos[4] = 
{
    {1217.50f, -5041.30f, -290.4328f, 4.4818f},
    {1272.52f, -5124.51f, -290.4575f, 2.8089f},
    {1218.18f, -5181.76f, -290.4609f, 1.7683f},
    {1135.86f, -5098.79f, -290.4617f, 6.0448f},
};

enum CreatureText
{
    SAY_PULL
};

class boss_thok_the_bloodthirsty : public CreatureScript
{
    public:
        boss_thok_the_bloodthirsty() : CreatureScript("boss_thok_the_bloodthirsty") {}

        struct boss_thok_the_bloodthirstyAI : public BossAI
        {
            boss_thok_the_bloodthirstyAI(Creature* creature) : BossAI(creature, DATA_THOK)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
                me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_CORROSIVE_BLOOD, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
            }
            
            InstanceScript* instance;
            ObjectGuid fplGuid, jGuid, pGuid;
            uint32 enrage;
            uint32 findtargets; //find and kill player in front of boss
            uint8 phasecount;
            bool phasetwo;
            bool summonbat;
            bool summoneyti;

            void Reset()
            {
                _Reset();
                DespawnObjects();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                me->SetReactState(REACT_DEFENSIVE);
                me->RemoveAllAuras();
                me->SetCreateMana(100);
                me->SetMaxPower(POWER_MANA, 100);
                me->SetPower(POWER_MANA, 0);
                findtargets = 0;
                fplGuid.Clear();  
                jGuid.Clear();   
                pGuid.Clear();    
                phasecount = 0;
                phasetwo = false;
                summonbat = false;
                summoneyti = false;
                enrage = 0;
                if (instance)
                {
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_PL);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOODIED);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNLOCKING);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BURNING_BLOOD_ADMG);
                }
            }

            void JustReachedHome()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void OnInterruptCast(Unit* /*caster*/, uint32 spellId, uint32 curSpellID, uint32 /*schoolMask*/)
            {
                if (curSpellID == SPELL_DEAFENING_SCREECH)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    EnterEvadeMode();
                }
            }

            void DespawnObjects()
            {
                std::list<AreaTrigger*> atlist;
                atlist.clear();
                me->GetAreaTriggersWithEntryInRange(atlist, 4890, me->GetGUID(), 200.0f);
                if (!atlist.empty())
                {
                    for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                        (*itr)->RemoveFromWorld();
                }
            }

            //Debug (for testing)
            void SpellHit(Unit* caster, SpellInfo const *spell)
            {
                if (spell->Id == SPELL_BLOODIED && me->HasAura(SPELL_POWER_REGEN))
                {
                    me->RemoveAurasDueToSpell(SPELL_BLOODIED);
                    me->RemoveAurasDueToSpell(SPELL_POWER_REGEN);
                    me->ToCreature()->AI()->DoAction(ACTION_PHASE_TWO);
                }
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                enrage = 600000;
                DoCast(me, SPELL_POWER_REGEN, true);
                events.RescheduleEvent(EVENT_SHOCK_BLAST, 4000);
                events.RescheduleEvent(EVENT_CHECK_PROGRESS, 6000);
                events.RescheduleEvent(EVENT_TAIL_LASH, 12000);
                events.RescheduleEvent(EVENT_FEARSOME_ROAR, 15000);
            }

            void SetGUID(ObjectGuid const& guid, int32 type) override
            {
                if (type == 2 && instance)
                {   //End phase two, go to kill prisoner
                    fplGuid.Clear();
                    events.Reset();
                    findtargets = 0;
                    pGuid = guid;
                    me->InterruptNonMeleeSpells(true);
                    me->RemoveAurasDueToSpell(SPELL_FIXATE_PL);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_PL);
                    if (Creature* prisoner = me->GetCreature(*me, pGuid))
                        DoCast(prisoner, SPELL_FIXATE_PR, true);
                    me->StopAttack();
                    me->getThreatManager().resetAllAggro();
                    me->GetMotionMaster()->MoveCharge(cpos.GetPositionX(), cpos.GetPositionY(), cpos.GetPositionZ(), 15.0f, 0);
                }
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_GET_THOK_PHASE_COUNT)
                    return phasecount;
                return 0;
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                case ACTION_PHASE_ONE_ACID: 
                case ACTION_PHASE_ONE_FROST: 
                case ACTION_PHASE_ONE_FIRE:
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    events.Reset();
                    phasetwo = false;
                    me->StopMoving();
                    me->GetMotionMaster()->Clear(false);
                    me->getThreatManager().resetAllAggro();
                    me->RemoveAurasDueToSpell(SPELL_BLOOD_FRENZY);
                    me->RemoveAurasDueToSpell(SPELL_BLOOD_FRENZY_TE);
                    me->RemoveAurasDueToSpell(SPELL_FIXATE_PL);
                    if (instance)
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_PL);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoCast(me, SPELL_POWER_REGEN, true);
                    DoZoneInCombat(me, 150.0f);
                    if (me->getVictim())
                        me->GetMotionMaster()->MoveChase(me->getVictim());
                    else
                    {
                        EnterEvadeMode();
                        return;
                    }
                    events.RescheduleEvent(EVENT_TAIL_LASH, 12000);
                    switch (action)
                    {
                    case ACTION_PHASE_ONE_ACID: 
                        events.RescheduleEvent(EVENT_ACID_BREATH, 15000);
                        events.RescheduleEvent(EVENT_CORROSIVE_BLOOD, 4000);
                        break;
                    case ACTION_PHASE_ONE_FROST: 
                        events.RescheduleEvent(EVENT_FREEZING_BREATH, 15000);
                        events.RescheduleEvent(EVENT_ICY_BLOOD, 4000);
                        break;
                    case ACTION_PHASE_ONE_FIRE:
                        DoCastAOE(SPELL_FLAME_COATING, true);
                        events.RescheduleEvent(EVENT_SCORCHING_BREATH, 15000);
                        events.RescheduleEvent(EVENT_BURNING_BLOOD, 4000);
                        break;
                    }
                    if (me->GetMap()->IsHeroic())
                    {
                        phasecount++;
                        me->CastCustomSpell(SPELL_HEAL, SPELLVALUE_BASE_POINT0, 8.0f, me, true);
                    }
                    break;
                case ACTION_PHASE_TWO:
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    phasetwo = true;
                    events.Reset();
                    events.RescheduleEvent(EVENT_SHOCK_BLAST, 3000);
                    if (instance)
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOODIED);
                    me->StopAttack();
                    me->getThreatManager().resetAllAggro();
                    me->RemoveAurasDueToSpell(SPELL_POWER_REGEN);
                    me->RemoveAurasDueToSpell(SPELL_ACCELERATION);
                    me->SetPower(POWER_MANA, 0);
                    DoCast(me, SPELL_BLOOD_FRENZY_KB, true);
                    DoCast(me, SPELL_BLOOD_FRENZY, true);
                    if (Creature* kj = me->SummonCreature(NPC_KORKRON_JAILER, kjspawnpos))
                    {
                        kj->AI()->DoZoneInCombat(kj, 250.0f);
                        jGuid = kj->GetGUID();
                    }
                    events.RescheduleEvent(EVENT_FIXATE, 2000);
                    break;
                //Special actions
                case ACTION_FIXATE:
                    fplGuid.Clear();
                    findtargets = 0;
                    me->StopAttack();
                    me->getThreatManager().resetAllAggro();
                    events.RescheduleEvent(EVENT_FIXATE, 1000);
                    break;
                case ACTION_START_FIXATE:
                    events.RescheduleEvent(EVENT_MOVING, 2000);
                    break;
                case ACTION_SUMMON_CAPTIVE_BAT:
                    if (!summonbat)
                    {
                        summonbat = true;
                        uint8 mod = urand(0, 5);
                        for (uint8 n = 0; n < 6; n++)
                        {
                            if (Creature* bat = me->SummonCreature(NPC_CAPTIVE_CAVE_BAT, ccbatspawnpos[n]))
                            {
                                if (mod == n)
                                    bat->AddAura(SPELL_VAMPIRIC_FRENZY, bat);
                                bat->AI()->DoZoneInCombat(bat, 200.0f);
                            }
                        }
                    }
                    break;
                case ACTION_SUMMON_STARVED_EYTI:
                    if (!summoneyti)
                    {
                        summoneyti = true;
                        me->SummonCreature(NPC_STARVED_YETI, sumyetipos[urand(0, 3)]);
                    }
                    break;
                }
            }

            void MovementInform(uint32 type, uint32 pointId)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                    case 0:
                        events.RescheduleEvent(EVENT_GO_TO_PRISONER, 500);
                        break;
                    case 1:
                        events.RescheduleEvent(EVENT_KILL_PRISONER, 2000);
                        break;
                    }       
                }
            }

            void KilledUnit(Unit* unit)
            {
                if (phasetwo)
                    if (unit->ToPlayer())
                        if (unit->GetGUID() == fplGuid)
                            DoAction(ACTION_FIXATE);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (findtargets)
                {
                    if (findtargets <= diff)
                    {
                        std::list<Player*> plist;
                        plist.clear();
                        GetPlayerListInGrid(plist, me, 20.0f);
                        if (!plist.empty())
                            for (std::list<Player*>::const_iterator itr = plist.begin(); itr != plist.end(); itr++)
                                if (me->isInFront(*itr, M_PI/6) && me->GetDistance(*itr) <= 8.0f)
                                    me->Kill(*itr, true);
                        findtargets = 750;
                    }
                    else
                        findtargets -= diff;
                }

                if (enrage)
                {
                    if (enrage <= diff)
                    {
                        DoCast(me, SPELL_ENRAGE, true);
                        enrage = 0;
                    }
                    else
                        enrage -= diff;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CHECK_PROGRESS:
                        if (instance && instance->GetBossState(DATA_SPOILS_OF_PANDARIA) != DONE)
                        {
                            me->AttackStop();
                            me->SetReactState(REACT_PASSIVE);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            EnterEvadeMode();
                        }
                        break;
                    //Default events
                    case EVENT_SHOCK_BLAST:
                        DoCastAOE(SPELL_SHOCK_BLAST, true);
                        events.RescheduleEvent(EVENT_SHOCK_BLAST, 4000);
                        break;
                    case EVENT_TAIL_LASH:
                        DoCast(me, SPELL_TAIL_LASH, true);
                        events.RescheduleEvent(EVENT_TAIL_LASH, 12000);
                        break;
                    case EVENT_FEARSOME_ROAR:
                        DoCast(me, SPELL_FEARSOME_ROAR, true);
                        events.RescheduleEvent(EVENT_FEARSOME_ROAR, 15000);
                        break;
                    case EVENT_GO_TO_PRISONER:
                        if (Creature* pr = me->GetCreature(*me, pGuid))
                            me->GetMotionMaster()->MoveCharge(pr->GetPositionX(), pr->GetPositionY(), pr->GetPositionZ(), 15.0f, 1);
                        break;
                    //Extra events
                    //
                    case EVENT_ACID_BREATH:
                        DoCast(me, SPELL_ACID_BREATH, true);
                        events.RescheduleEvent(EVENT_ACID_BREATH, 15000);
                        break;
                    case EVENT_CORROSIVE_BLOOD:
                        DoCastAOE(SPELL_CORROSIVE_BLOOD, true);
                        events.RescheduleEvent(EVENT_CORROSIVE_BLOOD, 4000);
                        break;
                    //
                    case EVENT_FREEZING_BREATH:
                        DoCast(me, SPELL_FREEZING_BREATH, true);
                        events.RescheduleEvent(EVENT_FREEZING_BREATH, 15000);
                        break;
                    case EVENT_ICY_BLOOD:
                    {
                        std::list<HostileReference*> tlist = me->getThreatManager().getThreatList();
                        if (!tlist.empty())
                        {
                            uint8 num = 0;
                            uint8 maxnum = me->GetMap()->Is25ManRaid() ? 8 : 3;
                            for (std::list<HostileReference*>::const_iterator itr = tlist.begin(); itr != tlist.end(); itr++)
                            {
                                if (itr != tlist.begin())
                                {
                                    if (Player* pl = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                                    {
                                        if (!pl->HasAura(SPELL_FROZEN_SOLID))
                                        {
                                            pl->AddAura(SPELL_ICY_BLOOD, pl);
                                            num++;
                                            if (num == maxnum)
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_ICY_BLOOD, 4000);
                        break;
                    }
                    //
                    case EVENT_SCORCHING_BREATH:
                        DoCast(me, SPELL_SCORCHING_BREATH, true);
                        events.RescheduleEvent(EVENT_SCORCHING_BREATH, 15000);
                        break;
                    case EVENT_BURNING_BLOOD:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 150.0f, true))
                            DoCast(target, SPELL_BURNING_BLOOD);
                        events.RescheduleEvent(EVENT_BURNING_BLOOD, 4000);
                        break; 
                    //
                    //Special events
                    case EVENT_KILL_PRISONER:
                        if (Creature* pr = me->GetCreature(*me, pGuid))
                        {
                            me->RemoveAurasDueToSpell(SPELL_FIXATE_PR);
                            pr->Kill(pr, true);
                            switch (pr->GetEntry())
                            {
                            case NPC_AKOLIK:
                                DoAction(ACTION_PHASE_ONE_ACID);
                                break;
                            case NPC_MONTAK:
                                DoAction(ACTION_PHASE_ONE_FIRE);
                                break;
                            case NPC_WATERSPEAKER_GORAI:
                                DoAction(ACTION_PHASE_ONE_FROST);
                                break;
                            }
                            pGuid.Clear();
                        }
                        break;
                    case EVENT_FIXATE:
                        me->InterruptNonMeleeSpells(true);
                        if (Player* pl = me->GetPlayer(*me, GetFixateTargetGuid()))
                        {
                            DoCast(pl, SPELL_FIXATE_PL);
                            fplGuid = pl->GetGUID();
                        }
                        else
                        {
                            me->MonsterTextEmote("Not found new fixate target, EnterEvadeMode", ObjectGuid::Empty, true);
                            EnterEvadeMode();
                        }
                        break;
                    case EVENT_MOVING:
                        if (Player* pl = me->GetPlayer(*me, fplGuid))
                        {
                            me->AddThreat(pl, 50000000.0f);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->Attack(pl, true);
                            me->GetMotionMaster()->MoveChase(pl);
                            findtargets = 700;
                        }
                        else
                        {
                            me->MonsterTextEmote("Not found my fixate target, EnterEvadeMode", ObjectGuid::Empty, true);
                            EnterEvadeMode();
                        }
                        break;
                    }
                }
                if (!phasetwo)
                    DoMeleeAttackIfReady();
            }

            ObjectGuid GetJailerVictimGuid()
            {
                if (Creature* kj = me->GetCreature(*me, jGuid))
                    if (kj->isAlive() && kj->isInCombat())
                        return kj->getVictim() ? kj->getVictim()->GetGUID() : ObjectGuid::Empty;
                return ObjectGuid::Empty;
            }

            ObjectGuid GetFixateTargetGuid()
            {
                ObjectGuid jvGuid = GetJailerVictimGuid();
                std::vector<ObjectGuid>_pllist;
                _pllist.clear();
                std::list<HostileReference*>ThreatList = me->getThreatManager().getThreatList();
                if (!ThreatList.empty())
                {
                    for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); itr++)
                        if (Player* target = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                            if (!target->HasAura(SPELL_UNLOCKING))
                                if (target->GetGUID() != jvGuid)
                                    _pllist.push_back(target->GetGUID());

                    if (!_pllist.empty())
                    {
                        std::random_shuffle(_pllist.begin(), _pllist.end());
                        std::vector<ObjectGuid>::const_iterator itr = _pllist.begin();
                        std::advance(itr, urand(0, _pllist.size() - 1));
                        return *itr;
                    }
                }
                return ObjectGuid::Empty;
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                DespawnObjects();
                if (instance)
                {
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FIXATE_PL);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLOODIED);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNLOCKING);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BURNING_BLOOD_ADMG);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_thok_the_bloodthirstyAI(creature);
        }
};

//71658
class npc_korkron_jailer : public CreatureScript
{
public:
    npc_korkron_jailer() : CreatureScript("npc_korkron_jailer") {}

    struct npc_korkron_jailerAI : public ScriptedAI
    {
        npc_korkron_jailerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset(){}
        
        void EnterCombat(Unit* who)
        {
            Talk(SAY_PULL);
            events.RescheduleEvent(EVENT_ENRAGE_KJ, 1000);
        }

        void OnSpellClick(Unit* clicker)
        {
            clicker->CastSpell(clicker, SPELL_UNLOCKING);
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* killer)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ENRAGE_KJ)
                {
                    DoCast(me, SPELL_ENRAGE_KJ, true);
                    events.RescheduleEvent(EVENT_ENRAGE_KJ, urand(15000, 20000));
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_jailerAI(creature);
    }
};

//71742, 71763, 71749
class npc_generic_prisoner : public CreatureScript
{
public:
    npc_generic_prisoner() : CreatureScript("npc_generic_prisoner") {}

    struct npc_generic_prisonerAI : public ScriptedAI
    {
        npc_generic_prisonerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (me->GetEntry() == NPC_WATERSPEAKER_GORAI)
                me->setFaction(35);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_FREEDOM:
                if (Creature* thok = me->GetCreature(*me, instance->GetGuidData(NPC_THOK)))
                    thok->AI()->SetGUID(me->GetGUID(), 2);
                if (me->GetEntry() == NPC_WATERSPEAKER_GORAI)
                {
                    DoCastAOE(SPELL_R_WATERS);
                    events.RescheduleEvent(EVENT_R_WATERS, 11000);
                }
                break;
            case ACTION_RESET:
                me->InterruptNonMeleeSpells(true);
                events.Reset();
                break;
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_R_WATERS)
                {
                    DoCastAOE(SPELL_R_WATERS);
                    events.RescheduleEvent(EVENT_R_WATERS, 11000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_generic_prisonerAI(creature);
    }
};

//69398
class npc_thok_ice_tomb : public CreatureScript
{
public:
    npc_thok_ice_tomb() : CreatureScript("npc_thok_ice_tomb") {}

    struct npc_thok_ice_tombAI : public ScriptedAI
    {
        npc_thok_ice_tombAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid sGuid;

        void Reset()
        {
            events.Reset();
            sGuid.Clear();
        }

        void IsSummonedBy(Unit* summoner)
        {
            sGuid = summoner->GetGUID();
            events.RescheduleEvent(EVENT_CHECK_TPLAYER, 1000);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void JustDied(Unit* killer)
        {
            if (Player* pl = me->GetPlayer(*me, sGuid))
            {
                pl->RemoveAurasDueToSpell(SPELL_FROZEN_SOLID);
                if (GameObject* it = me->FindNearestGameObject(GO_ICE_TOMB, 10.0f))
                    it->Delete();
            }
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_CHECK_TPLAYER)
                {
                    if (Player* pl = me->GetPlayer(*me, sGuid))
                    {
                        if (!pl->isAlive())
                        {
                            if (GameObject* it = me->FindNearestGameObject(GO_ICE_TOMB, 10.0f))
                                it->Delete();
                            me->DespawnOrUnsummon();
                        }
                        else
                            events.RescheduleEvent(EVENT_CHECK_TPLAYER, 1000);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_thok_ice_tombAI(creature);
    }
};

//73522
class npc_captive_cave_bat : public CreatureScript
{
public:
    npc_captive_cave_bat() : CreatureScript("npc_captive_cave_bat") {}

    struct npc_captive_cave_batAI : public ScriptedAI
    {
        npc_captive_cave_batAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset(){}

        void EnterCombat(Unit* who){}
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_captive_cave_batAI(creature);
    }
};

//71787
class npc_body_stalker : public CreatureScript
{
public:
    npc_body_stalker() : CreatureScript("npc_body_stalker") {}

    struct npc_body_stalkerAI : public ScriptedAI
    {
        npc_body_stalkerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE |UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;

        void Reset(){}
        
        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_body_stalkerAI(creature);
    }
};

//71645
class npc_shock_collar : public CreatureScript
{
public:
    npc_shock_collar() : CreatureScript("npc_shock_collar") {}

    struct npc_shock_collarAI : public ScriptedAI
    {
        npc_shock_collarAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->AddAura(SPELL_CANNON_BALL_DESTD, me);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shock_collarAI(creature);
    }
};

//73184
class npc_starved_yeti : public CreatureScript
{
public:
    npc_starved_yeti() : CreatureScript("npc_starved_yeti") {}

    struct npc_starved_yetiAI : public ScriptedAI
    {
        npc_starved_yetiAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        InstanceScript* instance;
        EventMap events;
        SummonList summons;
        float x, y;

        void Reset(){}

        void JustSummoned(Creature* sum)
        {
            summons.Summon(sum);
            events.RescheduleEvent(EVENT_Y_CHARGE, 2000);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (pointId == 1)
                {
                    summons.DespawnAll();
                    DoCast(me, SPELL_CANNON_BALL, true);
                    if (Creature* bs = me->FindNearestCreature(NPC_BODY_STALKER, 100.0f, true))
                    {
                        float ang1 = me->GetAngle(bs);
                        float ang = ang1 + GetAngleMod();
                        me->SetFacingTo(ang);
                        GetPositionWithDistInOrientation(me, 135.0f, ang, x, y);
                        events.RescheduleEvent(EVENT_PRE_Y_CHARGE, 15000);
                    }
                }
            }
        }

        float GetAngleMod()
        {
            float mod = float(urand(0, 1));
            mod = !mod ? -1 : 1;
            float mod2 = float(urand(1, 5))/10;
            float modangle = mod*mod2;
            return modangle;
        }

        void IsSummonedBy(Unit* summoner)
        {
            GetPositionWithDistInOrientation(me, 135.0f, me->GetOrientation(), x, y);
            me->SummonCreature(NPC_SHOCK_COLLAR, x, y, me->GetPositionZ());
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_PRE_Y_CHARGE:
                    me->SummonCreature(NPC_SHOCK_COLLAR, x, y, me->GetPositionZ());
                    break;
                case EVENT_Y_CHARGE:
                    if (Creature* sc = me->FindNearestCreature(NPC_SHOCK_COLLAR, 135.0f, true))
                        me->SetFacingToObject(sc);
                    me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 30.0f, 1);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_starved_yetiAI(creature);
    }
};

//143345
class spell_power_regen : public SpellScriptLoader
{
public:
    spell_power_regen() : SpellScriptLoader("spell_power_regen") { }

    class spell_power_regen_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_power_regen_AuraScript);

        void OnPeriodic(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
                if (GetCaster()->GetPower(POWER_MANA) == 100)
                    if (!GetCaster()->HasUnitState(UNIT_STATE_CASTING))
                        GetCaster()->CastSpell(GetCaster(), SPELL_DEAFENING_SCREECH);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_power_regen_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_power_regen_AuraScript();
    }
};

//143430
class spell_clump_check : public SpellScriptLoader
{
public:
    spell_clump_check() : SpellScriptLoader("spell_clump_check") { }

    class spell_clump_check_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_clump_check_SpellScript);

        void HandleOnHit()
        {
            if (GetHitUnit() && GetCaster()->ToCreature())
            {
                if (!GetHitUnit()->HasAura(SPELL_BLOODIED))
                {
                    if (GetHitUnit()->HealthBelowPct(50))
                        GetHitUnit()->AddAura(SPELL_BLOODIED, GetHitUnit());
                }
                else
                {
                    if (GetHitUnit()->HealthAbovePct(50))
                    {
                        GetHitUnit()->RemoveAurasDueToSpell(SPELL_BLOODIED);
                        return;
                    }

                    if (GetCaster()->HasAura(SPELL_POWER_REGEN)) //for safe
                    {
                        std::list<Player*> pllist;
                        pllist.clear();
                        GetPlayerListInGrid(pllist, GetHitUnit(), 10.0f);
                        if (!pllist.empty())
                        {
                            uint8 maxcount = GetCaster()->GetMap()->Is25ManRaid() ? 15 : 5;
                            uint8 count = 0;
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            {
                                if ((*itr)->HasAura(SPELL_BLOODIED))
                                    count++;

                                if (count >= maxcount)
                                {
                                    if (GetCaster()->HasAura(SPELL_POWER_REGEN))
                                    {
                                        GetCaster()->RemoveAurasDueToSpell(SPELL_POWER_REGEN);
                                        GetCaster()->ToCreature()->AI()->DoAction(ACTION_PHASE_TWO);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_clump_check_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_clump_check_SpellScript();
    }
};

//143445
class spell_fixate : public SpellScriptLoader
{
public:
    spell_fixate() : SpellScriptLoader("spell_fixate") { }

    class spell_fixate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fixate_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                GetCaster()->CastSpell(GetCaster(), SPELL_FIXATE_IM, true);
                GetCaster()->ClearUnitState(UNIT_STATE_CASTING);
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_START_FIXATE);
            }
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (GetCaster() && GetCaster()->ToCreature() && GetCaster()->isAlive() && GetCaster()->HasAura(SPELL_BLOOD_FRENZY))
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_FIXATE);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_fixate_AuraScript::OnApply, EFFECT_0, SPELL_AURA_FIXATE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_fixate_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_FIXATE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_fixate_AuraScript();
    }
};

//146589
class spell_unlocking : public SpellScriptLoader
{
public:
    spell_unlocking() : SpellScriptLoader("spell_unlocking") { }

    class spell_unlocking_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_unlocking_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                for (uint8 n = 0; n < 3; n++)
                {
                    if (Creature* p = GetTarget()->FindNearestCreature(prisonersentry[n], 40.0f, true))
                    {
                        p->NearTeleportTo(fpos[n].GetPositionX(), fpos[n].GetPositionY(), fpos[n].GetPositionZ(), fpos[n].GetOrientation());
                        p->AI()->DoAction(ACTION_FREEDOM);
                        break;
                    }
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_unlocking_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_unlocking_AuraScript();
    }
};

//143800
class spell_icy_blood : public SpellScriptLoader
{
public:
    spell_icy_blood() : SpellScriptLoader("spell_icy_blood") { }

    class spell_icy_blood_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_icy_blood_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetTarget()->HasAura(SPELL_ICY_BLOOD))
            {
                if (GetTarget()->GetAura(SPELL_ICY_BLOOD)->GetStackAmount() >= 5 && !GetTarget()->HasAura(SPELL_FROZEN_SOLID))
                {
                    GetTarget()->AddAura(SPELL_FROZEN_SOLID, GetTarget());
                    GetTarget()->RemoveAurasDueToSpell(SPELL_ICY_BLOOD);
                    GetTarget()->CastSpell(GetTarget(), SPELL_SUMMON_ICE_TOMB, true);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_icy_blood_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_icy_blood_AuraScript();
    }
};

//143773
class spell_freezing_breath : public SpellScriptLoader
{
public:
    spell_freezing_breath() : SpellScriptLoader("spell_freezing_breath") { }

    class spell_freezing_breath_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_freezing_breath_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetTarget()->HasAura(SPELL_FREEZING_BREATH))
            {
                if (GetTarget()->GetAura(SPELL_FREEZING_BREATH)->GetStackAmount() >= 5 && !GetTarget()->HasAura(SPELL_FROZEN_SOLID))
                {
                    GetTarget()->AddAura(SPELL_FROZEN_SOLID, GetTarget());
                    GetTarget()->RemoveAurasDueToSpell(SPELL_FREEZING_BREATH);
                    GetTarget()->CastSpell(GetTarget(), SPELL_SUMMON_ICE_TOMB, true);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_freezing_breath_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_freezing_breath_AuraScript();
    }
};

//143343
class spell_thok_deafening_screech : public SpellScriptLoader
{
public:
    spell_thok_deafening_screech() : SpellScriptLoader("spell_thok_deafening_screech") { }

    class spell_thok_deafening_screech_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_thok_deafening_screech_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (GetCaster()->GetMap()->IsHeroic())
                {
                    if (Aura* aura = GetCaster()->GetAura(SPELL_ACCELERATION))
                    {
                        switch (aura->GetStackAmount())
                        {
                        case 2:
                            if (GetCaster()->ToCreature()->AI()->GetData(DATA_GET_THOK_PHASE_COUNT) == 1)
                                GetCaster()->ToCreature()->AI()->DoAction(ACTION_SUMMON_CAPTIVE_BAT);
                            else if (GetCaster()->ToCreature()->AI()->GetData(DATA_GET_THOK_PHASE_COUNT) == 2)
                                GetCaster()->ToCreature()->AI()->DoAction(ACTION_SUMMON_STARVED_EYTI);
                            break;
                        case 30:
                        {
                            Map::PlayerList const &PlayerList = GetCaster()->GetMap()->GetPlayers();
                            if (!PlayerList.isEmpty())
                                for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                                    if (Player* player = Itr->getSource())
                                        if (player->isAlive())
                                            player->Kill(player, true);
                        }
                        break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_thok_deafening_screech_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_thok_deafening_screech_SpellScript();
    }
};

void AddSC_boss_thok_the_bloodthirsty()
{
    new boss_thok_the_bloodthirsty();
    new npc_korkron_jailer();
    new npc_generic_prisoner();
    new npc_thok_ice_tomb();
    new npc_captive_cave_bat();
    new npc_body_stalker();
    new npc_shock_collar();
    new npc_starved_yeti();
    new spell_power_regen();
    new spell_clump_check();
    new spell_fixate();
    new spell_unlocking();
    new spell_icy_blood();
    new spell_freezing_breath();
    new spell_thok_deafening_screech();
}
