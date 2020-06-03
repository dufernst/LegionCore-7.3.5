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

enum Spells
{
    SPELL_BATTLE_STANCE        = 143589,
    SPELL_BERSERKER_STANCE     = 143594,
    SPELL_DEFENSIVE_STANCE     = 143593,
    SPELL_HEROIC_SHOCKWAVE     = 143500,
    SPELL_KORKRON_BANNER_SUM   = 143591,
    SPELL_KORKRON_BANNER       = 143536,
    SPELL_WAR_SONG             = 143503,
    SPELL_RAVAGER_SUM          = 143872,
    SPELL_RAVAGER_AURA         = 143874,
    SPELL_SUNDERING_BLOW       = 143494,
    SPELL_BONECRACKER          = 143638,
    SPELL_COOLING_OFF          = 143484,
    SPELL_BERSERK              = 26662,
    SPELL_AFTERSHOCK           = 143712,
    SPELL_EXECUTE              = 143502,
    SPELL_IRONSTORM            = 143420,
    SPELL_LASTSTAND            = 143427,
    SPELL_ARCANE_SHOCK         = 143432,
    SPELL_MAGI_STRIKE          = 143431,
    SPELL_UNSTABLE_BLINK       = 143433,
    SPELL_STEALTH              = 118969,
    SPELL_ASSASINS_MARK        = 143480,
    SPELL_BACKSTAB             = 143481,
    SPELL_EARTH_SHIELD         = 143475,
    SPELL_EMPOWERED_CHAIN_HEAL = 143473,
    SPELL_HEALING_TIDE         = 143477,
    SPELL_HEALING_TOTEM_SUM    = 145558,
    SPELL_HUNTERS_MARK         = 143882,
    SPELL_SHOOT                = 143884,
    SPELL_MULTI_SHOT           = 143887,
};

enum Events
{
    EVENT_SUNDERING_BLOW      = 1,
    EVENT_BONECRACKER         = 2,
    EVENT_RE_ATTACK           = 3,
    EVENT_SUMMON              = 4,
    EVENT_BERSERK             = 5,
    EVENT_IN_POINT            = 6,
    EVENT_ARCANE_SHOCK        = 7,
    EVENT_MAGI_STRIKE         = 8,
    EVENT_UNSTABLE_BLINK      = 9,
    EVENT_EARTH_SHIELD        = 10,
    EVENT_CHAIN_HEAL          = 11,
    EVENT_HEALING_TOTEM       = 12,
    EVENT_CHECK_DIST          = 13,
    EVENT_BACKSTAB            = 14,
    EVENT_EXECUTE             = 15,
    EVENT_SHOOT               = 16,
    EVENT_MULTI_SHOT          = 17,
    EVENT_CHECK_PROGRESS      = 18,
};

enum Actions
{
    ACTION_SET_NEXT_STANCE    = 1,
    ACTION_RE_ATTACK          = 2,
};

enum CreatureText
{
    SAY_PULL                  = 1,
    SAY_SUMMON                = 2,
    SAY_SUMMON_2              = 3,
    SAY_SUMMON_3              = 4,
    SAY_DIED                  = 5,
};

Position sumpos[3] =
{
    {1572.98f, -4634.50f, -66.7077f, 6.1815f},
    {1609.19f, -4605.83f, -66.7174f, 5.1880f},
    {1582.42f, -4616.40f, -66.6944f, 5.5729f},
};

uint32 stances[3] =
{
    SPELL_BATTLE_STANCE,
    SPELL_BERSERKER_STANCE,
    SPELL_DEFENSIVE_STANCE,
};

struct AfterShockAngle
{
    float ang;
    float ang2;
};

static AfterShockAngle angmod[6] =
{
    {M_PI, M_PI + 1.570796326795f},
    {M_PI, 0.7853981633975f},
    {M_PI, 1.570796326795f + 0.7853981633975f},
    {M_PI + 1.570796326795f, 0.7853981633975f},
    {M_PI + 1.570796326795f, 1.570796326795f + 0.7853981633975f},
    {0.7853981633975f, 1.570796326795f + 0.7853981633975f},
};

class boss_general_nazgrim : public CreatureScript
{
    public:
        boss_general_nazgrim() : CreatureScript("boss_general_nazgrim") {}

        struct boss_general_nazgrimAI : public BossAI
        {
            boss_general_nazgrimAI(Creature* creature) : BossAI(creature, DATA_GENERAL_NAZGRIM)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            uint8 laststance;
            uint32 checkpower, checkvictim;
            uint8 wavenum;
            bool lowhp;

            void Reset()
            {
                _Reset();
                for (uint8 n = 0; n < 3; n++)
                    me->RemoveAurasDueToSpell(stances[n]);
                me->RemoveAurasDueToSpell(SPELL_BERSERK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ASSASINS_MARK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HUNTERS_MARK);
                me->SetCreateMana(100);
                me->SetMaxPower(POWER_MANA, 100);
                me->SetPower(POWER_MANA, 0);
                laststance = 4; //default
                checkpower = 1000;
                checkvictim = 0;
                lowhp = false;
            }

            void SummonWave(uint8 count)
            {
                std::vector<uint32>sumlist;
                sumlist.clear();
                if (me->GetMap()->IsHeroic())
                {
                    switch (count)
                    {
                    case 0:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_IRONBLADE);
                        break;
                    case 1:
                        sumlist.push_back(NPC_WARSHAMAN);
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_SNIPER);
                        break;
                    case 2:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_WARSHAMAN);
                        sumlist.push_back(NPC_IRONBLADE);
                        break;
                    case 3:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_SNIPER);
                        break;
                    case 4:
                        sumlist.push_back(NPC_WARSHAMAN);
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_IRONBLADE);
                        break;
                    case 5:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_WARSHAMAN);
                        sumlist.push_back(NPC_SNIPER);
                        break;
                    case 6:
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_SNIPER);
                        sumlist.push_back(NPC_IRONBLADE);
                        break;
                    case 7:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_WARSHAMAN);
                        sumlist.push_back(NPC_ASSASIN);
                        break;
                    case 8:
                        sumlist.push_back(NPC_WARSHAMAN);
                        sumlist.push_back(NPC_SNIPER);
                        sumlist.push_back(NPC_IRONBLADE);
                        break;
                    case 9:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_SNIPER);
                        sumlist.push_back(NPC_IRONBLADE);
                        break;
                    default:
                        break;
                    }
                }
                else
                { 
                    switch (count)
                    {
                    case 0:
                        sumlist.push_back(NPC_IRONBLADE);
                        sumlist.push_back(NPC_ARCHWEAVER);
                        break;
                    case 1:
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_WARSHAMAN);
                        break;
                    case 2:
                        sumlist.push_back(NPC_IRONBLADE);
                        sumlist.push_back(NPC_ASSASIN);
                        break;
                    case 3:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_WARSHAMAN);
                        break;
                    case 4:
                        sumlist.push_back(NPC_IRONBLADE);
                        sumlist.push_back(NPC_WARSHAMAN);
                        break;
                    case 5:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_ASSASIN);
                        break;
                    case 6:
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_WARSHAMAN);
                        break;
                    case 7:
                        sumlist.push_back(NPC_IRONBLADE);
                        sumlist.push_back(NPC_ASSASIN);
                        sumlist.push_back(NPC_WARSHAMAN);
                        break;
                    case 8:
                        sumlist.push_back(NPC_IRONBLADE);
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_WARSHAMAN);
                        break;
                    case 9:
                        sumlist.push_back(NPC_IRONBLADE);
                        sumlist.push_back(NPC_ARCHWEAVER);
                        sumlist.push_back(NPC_ASSASIN);
                        break;
                    default:
                        break;
                    }
                }
                if (!sumlist.empty())
                {
                    uint8 pos = urand(2, 4);
                    Talk(pos);
                    uint8 mod;
                    for (std::vector<uint32>::const_iterator itr = sumlist.begin(); itr != sumlist.end(); itr++)
                    {
                        mod = itr == sumlist.begin() ? 0 : urand(1, 2);
                        if (Creature* s = me->SummonCreature(*itr, sumpos[mod].GetPositionX(), sumpos[mod].GetPositionY(), sumpos[mod].GetPositionZ(), sumpos[mod].GetOrientation()))
                        {
                            switch (s->GetEntry())
                            {
                            case NPC_ASSASIN:
                                s->AI()->SetGUID(GetTargetGUIDForRogue(), 1);
                                break;
                            case NPC_SNIPER:
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 80.0f, true))
                                    s->AI()->SetGUID(target->GetGUID(), 2);
                                break;
                            default:
                                s->AI()->DoZoneInCombat(s, 150.0f);
                                break;
                            }
                        }
                    }
                }
                wavenum++;
            }

            ObjectGuid GetTargetGUIDForRogue()
            {
                std::list<HostileReference*>tlist = me->getThreatManager().getThreatList();
                if (!tlist.empty())
                {
                    for (std::list<HostileReference*>::const_iterator itr = tlist.begin(); itr != tlist.end(); itr++)
                    {
                        if (itr != tlist.begin())
                        {
                            if (Player* pl = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                            {
                                if (CheckPlayerForRogue(pl))
                                    return pl->GetGUID();
                            }
                        }
                    }
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                        return target->GetGUID();
                }
                return ObjectGuid::Empty;
            }

            bool CheckPlayerForRogue(Player* pl)
            {
                if (pl->getClass() == CLASS_PRIEST || pl->getClass() == CLASS_MAGE || pl->getClass() == CLASS_WARLOCK)
                    return true;
                else if ((pl->getPowerType() == POWER_MANA && pl->getClass() == CLASS_DRUID) || pl->getClass() == CLASS_SHAMAN)
                    return true;

                return false;
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (HealthBelowPct(10) && !lowhp)
                {
                    lowhp = true;
                    SummonWave(wavenum);
                }
            }

            void JustReachedHome()
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
            }

            bool CheckPullPlayerPos(Unit* who)
            {
                if (who->GetPositionY() >= -4586.20f)
                    return false;

                return true;
            }

            void EnterCombat(Unit* who)
            {
                if (instance)
                {
                    if (!CheckPullPlayerPos(who))
                    {
                        EnterEvadeMode();
                        return;
                    }
                }
                Talk(SAY_PULL);
                _EnterCombat();
                SetStance(0);
                wavenum = 0;
                checkvictim = 1500;
                events.RescheduleEvent(EVENT_CHECK_PROGRESS, 5000);
                events.RescheduleEvent(EVENT_SUMMON, 45000);
                events.RescheduleEvent(EVENT_SUNDERING_BLOW, 30000);
                events.RescheduleEvent(EVENT_BONECRACKER, 15000);
                events.RescheduleEvent(EVENT_BERSERK, 600000);
                if (me->GetMap()->IsHeroic())
                    events.RescheduleEvent(EVENT_EXECUTE, 18000);
            }

            void SetStance(uint8 stance)
            {
                if (stance > 2)
                    return;

                for (uint8 n = 0; n < 3; n++)
                    if (me->HasAura(stances[n]))
                        return;

                DoCast(me, stances[stance], true);
                laststance = stance;
            }

            uint8 GetNextStance()
            {
                uint8 num = laststance == 2 ? 0 : laststance + 1;
                return num;
            }

            void DoAction(int32 const action)
            {
                switch (action)
                { 
                case ACTION_SET_NEXT_STANCE:
                    if (me->isInCombat())
                        SetStance(GetNextStance());
                    break;
                case ACTION_RE_ATTACK:
                    events.RescheduleEvent(EVENT_RE_ATTACK, 1000);
                    break;
                }
            }

            void CastExtraSpellsFromPower(int32 power)
            {
                if (power == 100)
                {
                    if (me->getVictim())
                        DoCastVictim(SPELL_RAVAGER_SUM);
                }
                else if (power >= 70 && power < 100)
                    DoCastAOE(SPELL_WAR_SONG);
                else if (power >= 50 && power < 70)
                {
                    DoCast(me, SPELL_KORKRON_BANNER_SUM);
                    float x, y;
                    GetPosInRadiusWithRandomOrientation(me, 8.0f, x, y);
                    me->SummonCreature(NPC_KORKRON_BANNER, x, y, me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);
                }
                else if (power >= 30 && power < 50)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                    {
                        me->StopAttack();
                        DoCast(target, SPELL_HEROIC_SHOCKWAVE);
                    }
                }
                checkpower = 5000;
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (checkpower <= diff)
                {
                    if (!me->HasAura(SPELL_COOLING_OFF))
                    {
                        if (me->GetPower(POWER_MANA) >= 30) //need check or not
                        {
                            checkpower = 15000; //for safe
                            events.DelayEvents(5000);
                            CastExtraSpellsFromPower(me->GetPower(POWER_MANA));
                            return;
                        }
                    }
                    checkpower = 1000;
                }
                else
                    checkpower -= diff;

                if (checkvictim && instance)
                {
                    if (checkvictim <= diff)
                    {
                        if (me->getVictim())
                        {
                            if (!CheckPullPlayerPos(me->getVictim()))
                            {
                                me->AttackStop();
                                me->SetReactState(REACT_PASSIVE);
                                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                                EnterEvadeMode();
                                checkvictim = 0;
                            }
                            else
                                checkvictim = 1500;
                        }
                    }
                    else
                        checkvictim -= diff;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CHECK_PROGRESS:
                        if (instance && instance->GetBossState(DATA_KORKRON_D_SHAMAN) != DONE)
                        {
                            me->AttackStop();
                            me->SetReactState(REACT_PASSIVE);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            EnterEvadeMode();
                        }
                        break;
                    //Default events
                    case EVENT_SUNDERING_BLOW:
                        if (me->getVictim())
                            DoCastVictim(SPELL_SUNDERING_BLOW, true);
                        events.RescheduleEvent(EVENT_SUNDERING_BLOW, 8000);
                        break;
                    case EVENT_BONECRACKER:
                        DoCastAOE(SPELL_BONECRACKER);
                        events.RescheduleEvent(EVENT_BONECRACKER, 30000);
                        break;             
                    //Other events
                    case EVENT_RE_ATTACK:
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    case EVENT_SUMMON:
                        SummonWave(wavenum);
                        events.RescheduleEvent(EVENT_SUMMON, 45000);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        break;
                    case EVENT_EXECUTE:
                        if (me->getVictim())
                            DoCastVictim(SPELL_EXECUTE);
                        events.RescheduleEvent(EVENT_EXECUTE, 18000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*killer*/)
            {
                Talk(SAY_DIED);
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ASSASINS_MARK);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HUNTERS_MARK);
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_general_nazgrimAI(creature);
        }
};

//71770
class npc_korkron_ironblade : public CreatureScript
{
public:
    npc_korkron_ironblade() : CreatureScript("npc_korkron_ironblade") {}

    struct npc_korkron_ironbladeAI : public ScriptedAI
    {
        npc_korkron_ironbladeAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        bool laststand;

        void Reset()
        {
            laststand = false;
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 150.0f);
            DoCast(me, SPELL_IRONSTORM);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (HealthBelowPct(50) && !laststand)
            {
                laststand = true;
                me->AddAura(SPELL_LASTSTAND, me);
            }
        }

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_ironbladeAI(creature);
    }
};

//71771
class npc_korkron_archweaver : public CreatureScript
{
public:
    npc_korkron_archweaver() : CreatureScript("npc_korkron_archweaver") {}

    struct npc_korkron_archweaverAI : public ScriptedAI
    {
        npc_korkron_archweaverAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 150.0f);
            events.RescheduleEvent(EVENT_ARCANE_SHOCK, 5000);
            events.RescheduleEvent(EVENT_MAGI_STRIKE, 10000);
            events.RescheduleEvent(EVENT_UNSTABLE_BLINK, 15000);
        }

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (IsInControl() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ARCANE_SHOCK:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_ARCANE_SHOCK);
                    events.RescheduleEvent(EVENT_ARCANE_SHOCK, 5000);
                    break;
                case EVENT_MAGI_STRIKE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_MAGI_STRIKE);
                    events.RescheduleEvent(EVENT_MAGI_STRIKE, 10000);
                    break;
                case EVENT_UNSTABLE_BLINK:
                    DoCast(me, SPELL_UNSTABLE_BLINK);
                    events.RescheduleEvent(EVENT_UNSTABLE_BLINK, 15000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_archweaverAI(creature);
    }
};

//71518
class npc_korkron_assasin : public CreatureScript
{
public:
    npc_korkron_assasin() : CreatureScript("npc_korkron_assasin") {}

    struct npc_korkron_assasinAI : public ScriptedAI
    {
        npc_korkron_assasinAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid plGuid;
        bool visible;

        void Reset()
        {
            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetPower(POWER_ENERGY, 100);
            plGuid.Clear();
            DoCast(me, SPELL_STEALTH);
            visible = false;
        }

        Player* GetPlayerTarget()
        {
            if (Player* pl = me->GetPlayer(*me, plGuid))
                if (pl->isAlive())
                    return pl;

            return NULL;
        }

        void SetGUID(ObjectGuid const& guid, int32 type) override
        {
            if (type == 1)
            {
                if (guid)
                    SetTarget(guid);
                else
                    me->DespawnOrUnsummon();
            }
        }

        void SetTarget(ObjectGuid Guid)
        {
            if (Player* pl = me->GetPlayer(*me, Guid))
            {
                plGuid = Guid;
                me->AddAura(SPELL_ASSASINS_MARK, pl);
                me->SetReactState(REACT_AGGRESSIVE);
                me->AddThreat(pl, 5000000.0f);
                DoZoneInCombat(me, 150.0f);
            }
        }

        void DamageDealt(Unit* doneTo, uint32& damage, DamageEffectType damagetype)
        {
            if (!visible)
            {
                visible = true;
                doneTo->RemoveAurasDueToSpell(SPELL_ASSASINS_MARK);
                me->RemoveAurasDueToSpell(SPELL_STEALTH);
                events.RescheduleEvent(EVENT_BACKSTAB, 1000);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {         
            if (!visible)
            {
                visible = true;
                if (Player* pl = GetPlayerTarget())
                    pl->RemoveAurasDueToSpell(SPELL_ASSASINS_MARK);
                me->RemoveAurasDueToSpell(SPELL_STEALTH);
                events.RescheduleEvent(EVENT_BACKSTAB, 1000);
            }

            if (attacker->GetGUID() != plGuid)
                me->getThreatManager().addThreat(attacker, NULL);
        }

        void JustDied(Unit* killer)
        {
            if (Player* pl = GetPlayerTarget())
                pl->RemoveAurasDueToSpell(SPELL_ASSASINS_MARK);
            me->DespawnOrUnsummon();
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_BACKSTAB)
                {
                    if (me->getVictim())
                    {
                        if (me->getVictim()->isInBack(me))
                        {
                            DoCast(me->getVictim(), SPELL_BACKSTAB);
                            events.RescheduleEvent(EVENT_BACKSTAB, 4000);
                        }
                        else
                            events.RescheduleEvent(EVENT_BACKSTAB, 1000);
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_assasinAI(creature);
    }
};

//71773
class npc_korkron_warshaman : public CreatureScript
{
public:
    npc_korkron_warshaman() : CreatureScript("npc_korkron_warshaman") {}

    struct npc_korkron_warshamanAI : public ScriptedAI
    {
        npc_korkron_warshamanAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        SummonList summon;

        void Reset()
        {
            events.Reset();
            summon.DespawnAll();
        }

        void EnterCombat(Unit* who)
        {
            DoZoneInCombat(me, 150.0f);
            events.RescheduleEvent(EVENT_EARTH_SHIELD, 10000);
            events.RescheduleEvent(EVENT_CHAIN_HEAL, 15000);
            events.RescheduleEvent(EVENT_HEALING_TOTEM, 8000);
        }

        void JustDied(Unit* killer)
        {
            summon.DespawnAll();
            me->DespawnOrUnsummon();
        }

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
            if (sum->GetEntry() == NPC_HEALING_TIDE_TOTEM)
            {
                sum->SetReactState(REACT_PASSIVE);
                sum->AttackStop();
                sum->AddAura(SPELL_HEALING_TIDE, sum);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (IsInControl() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_EARTH_SHIELD:
                    if (Unit* ftarget = DoSelectLowestHpFriendly(20.0f))
                    {
                        if (ftarget->HealthBelowPct(90))
                            DoCast(ftarget, SPELL_EARTH_SHIELD);
                    }
                    else
                        DoCast(me, SPELL_EARTH_SHIELD);
                    events.RescheduleEvent(EVENT_EARTH_SHIELD, 15000);
                    break;
                case EVENT_CHAIN_HEAL:
                    if (Unit* ftarget = DoSelectLowestHpFriendly(20.0f))
                    {
                        if (ftarget->HealthBelowPct(90))
                            DoCast(ftarget, SPELL_EMPOWERED_CHAIN_HEAL);
                    }
                    else
                        DoCast(me, SPELL_EMPOWERED_CHAIN_HEAL);
                    events.RescheduleEvent(EVENT_CHAIN_HEAL, 20000);
                    break;
                case EVENT_HEALING_TOTEM:
                    DoCast(me, SPELL_HEALING_TOTEM_SUM);
                    events.RescheduleEvent(EVENT_HEALING_TOTEM, 25000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_warshamanAI(creature);
    }
};

//71656
class npc_korkron_sniper : public CreatureScript
{
public:
    npc_korkron_sniper() : CreatureScript("npc_korkron_sniper") {}

    struct npc_korkron_sniperAI : public ScriptedAI
    {
        npc_korkron_sniperAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        EventMap events;
        SummonList summon;
        ObjectGuid targetGuid;

        void Reset()
        {
            events.Reset();
            targetGuid.Clear();
        }

        Unit* GetTarget()
        {
            if (Unit* target = me->GetUnit(*me, targetGuid))
                if (target->isAlive())
                    return target;
            return 0;
        }

        void SetGUID(ObjectGuid const& guid, int32 type) override
        {
            if (type == 2)
            {
                if (Unit* target = me->GetUnit(*me, guid))
                {
                    if (target->isAlive())
                    {
                        targetGuid = guid;
                        me->AddAura(SPELL_HUNTERS_MARK, target);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->AddThreat(target, 5000000.0f);
                        events.RescheduleEvent(EVENT_SHOOT, 2000);
                        events.RescheduleEvent(EVENT_MULTI_SHOT, 5000);
                    }
                }
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
            {
                if (pointId == 3)
                {
                    me->GetMotionMaster()->Clear(false);
                    events.RescheduleEvent(EVENT_SHOOT, 2000);
                    events.RescheduleEvent(EVENT_MULTI_SHOT, 5000);
                }
            }
        }
        
        void JustDied(Unit* killer)
        {
            if (Unit* target = GetTarget())
                target->RemoveAurasDueToSpell(SPELL_HUNTERS_MARK);
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SHOOT:
                {
                    Unit* target = GetTarget() ? GetTarget() : SelectTarget(SELECT_TARGET_FARTHEST, 0, 80.0f, true);
                    if (me->GetDistance(target) > 30.0f)
                    {
                        events.Reset();
                        float x, y;
                        GetPositionWithDistInOrientation(target, 30.0f, me->GetFollowAngle(), x, y);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 15.0f, 3);
                        return;
                    }
                    DoCast(target, SPELL_SHOOT);
                    events.RescheduleEvent(EVENT_SHOOT, 4000);
                }
                break;
                case EVENT_MULTI_SHOT:
                {
                    Unit* target = GetTarget() ? GetTarget() : SelectTarget(SELECT_TARGET_FARTHEST, 0, 80.0f, true);
                    if (me->GetDistance(target) > 40.0f)
                    {
                        events.Reset();
                        float x, y;
                        GetPositionWithDistInOrientation(target, 30.0f, me->GetFollowAngle(), x, y);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 15.0f, 3);
                        return;
                    }
                    DoCast(target, SPELL_MULTI_SHOT);
                    events.RescheduleEvent(EVENT_MULTI_SHOT, 10000);
                }
                break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_sniperAI(creature);
    }
};

//71697
class npc_after_shock : public CreatureScript
{
public:
    npc_after_shock() : CreatureScript("npc_after_shock") {}

    struct npc_after_shockAI : public ScriptedAI
    {
        npc_after_shockAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->SetDisplayId(11686);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->ToCreature() && summoner->GetEntry() != me->GetEntry())
            {
                uint8 mod = urand(0, 5);
                float x, y;

                GetPositionWithDistInOrientation(me, 20.0f, angmod[mod].ang, x, y);
                if (Creature* as = me->SummonCreature(NPC_AFTER_SHOCK, x, y, me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000))
                {
                    as->SetFacingToObject(me);
                    as->CastSpell(me, SPELL_AFTERSHOCK);
                }

                GetPositionWithDistInOrientation(me, 20.0f, angmod[mod].ang2, x, y);
                if (Creature* as = me->SummonCreature(NPC_AFTER_SHOCK, x, y, me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000))
                {
                    as->SetFacingToObject(me);
                    as->CastSpell(me, SPELL_AFTERSHOCK);
                }
            }
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_after_shockAI(creature);
    }
};

//71626
class npc_korkron_banner : public CreatureScript
{
public:
    npc_korkron_banner() : CreatureScript("npc_korkron_banner") {}

    struct npc_korkron_bannerAI : public ScriptedAI
    {
        npc_korkron_bannerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->AttackStop();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->CastSpell(me, SPELL_KORKRON_BANNER, true);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}
        
        void UpdateAI(uint32 diff){}  

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_korkron_bannerAI(creature);
    }
};

//71762
class npc_ravager : public CreatureScript
{
public:
    npc_ravager() : CreatureScript("npc_ravager") {}

    struct npc_ravagerAI : public ScriptedAI
    {
        npc_ravagerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->AddAura(SPELL_RAVAGER_AURA, me);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
            GetAndMoveRandomPoint();
        }

        Unit* GetGeneralNazgrim()
        {
            if (me->ToTempSummon())
            {
                if (Unit* gn = me->ToTempSummon()->GetSummoner())
                {
                    if (gn->isAlive() && gn->isInCombat())
                        return gn;
                }
            }
            return NULL;
        }

        void GetAndMoveRandomPoint()
        {
            if (Unit* gn = GetGeneralNazgrim())
            {
                me->GetMotionMaster()->Clear(false);
                float x, y;
                float ang = float(urand(0, 6));
                GetPositionWithDistInOrientation(gn, float(urand(5, 40)), ang, x, y);
                me->GetMotionMaster()->MovePoint(1, x, y, gn->GetPositionZ(), true, 4.0f);
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
                if (pointId == 1)
                    events.RescheduleEvent(EVENT_IN_POINT, urand(5000, 10000));
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_IN_POINT)
                    GetAndMoveRandomPoint();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ravagerAI(creature);
    }
};

//143716
class spell_heroic_shockwave : public SpellScriptLoader
{
public:
    spell_heroic_shockwave() : SpellScriptLoader("spell_heroic_shockwave") { }

    class spell_heroic_shockwave_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_heroic_shockwave_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature())
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_heroic_shockwave_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_heroic_shockwave_SpellScript();
    }
};

//143589, 143594, 143593,
class spell_generic_stance : public SpellScriptLoader
{
public:
    spell_generic_stance() : SpellScriptLoader("spell_generic_stance") { }

    class spell_generic_stance_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_generic_stance_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
            {
                if (GetCaster() && GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_SET_NEXT_STANCE);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_generic_stance_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_ENERGIZE, AURA_EFFECT_HANDLE_REAL);        //143589
            OnEffectRemove += AuraEffectRemoveFn(spell_generic_stance_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);  //143594
            OnEffectRemove += AuraEffectRemoveFn(spell_generic_stance_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL); //143593
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_generic_stance_AuraScript();
    }
};

//143716, 143501, 143503, 143872
class spell_after_extra_spell_effect : public SpellScriptLoader
{
public:
    spell_after_extra_spell_effect() : SpellScriptLoader("spell_after_extra_spell_effect") { }

    class spell_after_extra_spell_effect_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_after_extra_spell_effect_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                if (GetCaster()->isInCombat())
                    GetCaster()->CastSpell(GetCaster(), SPELL_COOLING_OFF, true);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_after_extra_spell_effect_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_after_extra_spell_effect_SpellScript();
    }
};

//143494
class spell_sundering_blow : public SpellScriptLoader
{
public:
    spell_sundering_blow() : SpellScriptLoader("spell_sundering_blow") { }

    class spell_sundering_blow_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sundering_blow_SpellScript);

        void _HandleHit()
        {
            if (GetCaster() &&  GetHitUnit())
            {
                if (!GetHitUnit()->HasAura(SPELL_SUNDERING_BLOW))
                {
                    if (GetCaster()->GetPower(POWER_MANA) <= 95)
                        GetCaster()->SetPower(POWER_MANA, GetCaster()->GetPower(POWER_MANA) + 5);
                }
                else
                {
                    uint8 mod = (GetHitUnit()->GetAura(SPELL_SUNDERING_BLOW)->GetStackAmount()) * 5;
                    if ((GetCaster()->GetPower(POWER_MANA) + mod) <= 100)
                        GetCaster()->SetPower(POWER_MANA, GetCaster()->GetPower(POWER_MANA) + mod);
                    else
                        GetCaster()->SetPower(POWER_MANA, 100);
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_sundering_blow_SpellScript::_HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_sundering_blow_SpellScript();
    }
};

void AddSC_boss_general_nazgrim()
{
    new boss_general_nazgrim();
    new npc_korkron_ironblade();
    new npc_korkron_archweaver();
    new npc_korkron_assasin();
    new npc_korkron_warshaman();
    new npc_korkron_sniper();
    new npc_after_shock();
    new npc_korkron_banner();
    new npc_ravager();
    new spell_heroic_shockwave();
    new spell_generic_stance();
    new spell_after_extra_spell_effect();
    new spell_sundering_blow();
}
