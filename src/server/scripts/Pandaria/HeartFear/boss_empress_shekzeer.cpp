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

#include "Group.h"
#include "heart_of_fear.h"

enum eSpells
{
    //Empress
    //phase 1
    SPELL_CRY_OF_TERROR          = 123788,
    SPELL_DREAD_SCREECH          = 123735,
    SPELL_EYES_OF_EMPRESS        = 123707,
    SPELL_SERVANT_OF_THE_EMPRESS = 123713,
    //phase 3
    SPELL_CALAMITY               = 124845,
    SPELL_SHA_ENERGY             = 125464,
    SPELL_CONSUMING_TERROR       = 124849,
    SPELL_VISIONS_OF_DEMISE      = 124862,
    SPELL_AMASSING_DARKNESS      = 124842,

    SPELL_FIXATE                 = 129149,
    //Sentinels
    //Setthik
    SPELL_TOXIC_SLIME            = 124807,
    SPELL_DISPATCH               = 129154,
    SPELL_SONIC_BLADE            = 125888, //125886 ?
    //Korthik
    SPELL_BAND_OF_VALOR          = 125417,
    SPELL_TOXIC_BOMB             = 124777,
    SPELL_POISON_DRENCHED_ARMOR  = 124838,
};

enum eEvents
{
    //Empress
    EVENT_CHECK_POWER           = 1,
    EVENT_GO_IN_CASE_WORM       = 2,
    EVENT_TERROR                = 3,
    EVENT_SCREECH               = 4,
    EVENT_CALAMITY              = 5,
    EVENT_SHA_ENERGY            = 6,
    EVENT_CONSUMING_TERROR      = 7,
    EVENT_VISIONS_OF_DEMISE     = 8,
    EVENT_EYES_OF_EMPRESS       = 9,
    EVENT_AMASSING_DARKNESS     = 10,
    EVENT_FIXATE                = 11,
    EVENT_DISPATCH              = 12,

    //Sentinels
    EVENT_TOXIC_SLIME           = 1,
    EVENT_TOXIC_BOMB            = 2,
    EVENT_SONIC_BLADE           = 4,
};

enum Actions
{
    ACTION_PHASE_1           = 1,
    ACTION_PHASE_2           = 2,
    ACTION_PHASE_3           = 3,
    ACTION_SENTINEL_DIED     = 4,
};

enum Phase
{
    PHASE_NONE               = 0, 
    PHASE_ONE                = 1, 
    PHASE_TWO                = 2, 
    PHASE_THREE              = 3, 
};

enum eSentinels
{
    NPC_SETTHIK_WINDBLADE    = 63589,
    NPC_KORTHIK_REAVER       = 63591,
};

uint32 sumwave[4] = 
{
    NPC_SETTHIK_WINDBLADE,
    NPC_SETTHIK_WINDBLADE,
    NPC_SETTHIK_WINDBLADE,
    NPC_KORTHIK_REAVER,
};

Position const cocoonpos  = {-2478.709f, 1068.170f, 584.953f};
Position const lsummonpos = {-2520.776f, 983.256f, 569.630f};
Position const rsummonpos = {-2435.437f, 984.290f, 569.630f};

class boss_empress_shekzeer : public CreatureScript
{
    public:
        boss_empress_shekzeer() : CreatureScript("boss_empress_shekzeer") {}

        struct boss_empress_shekzeerAI : public BossAI
        {
            boss_empress_shekzeerAI(Creature* creature) : BossAI(creature, DATA_SHEKZEER)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            Phase phase;
            uint8 sdiedval;

            void Reset()
            {
                _Reset();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetVisible(true);
                me->SetReactState(REACT_DEFENSIVE);
                phase = PHASE_NONE;
                events.SetPhase(PHASE_NONE);
                me->setPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 150);
                me->SetPower(POWER_ENERGY, 150);
                sdiedval = 0;
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EYES_OF_EMPRESS);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SERVANT_OF_THE_EMPRESS);
            }

            void RegeneratePower(Powers power, float &value)
            {
                if (!me->isInCombat() || phase == PHASE_ONE)
                    value = 0;
                else if (phase == PHASE_TWO)
                    value = 1;
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                events.SetPhase(PHASE_ONE);
                phase = PHASE_ONE;
                events.ScheduleEvent(EVENT_CHECK_POWER, 1000);
                events.ScheduleEvent(EVENT_SCREECH, 8000, 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_TERROR,  urand(25000, 35000), 0, PHASE_ONE);
                events.ScheduleEvent(EVENT_EYES_OF_EMPRESS, 10000);
                events.ScheduleEvent(EVENT_FIXATE, 10000);
                events.ScheduleEvent(EVENT_DISPATCH, 20000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EYES_OF_EMPRESS);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SERVANT_OF_THE_EMPRESS);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (HealthBelowPct(30) && phase != PHASE_THREE)
                {
                    phase = PHASE_THREE;
                    events.SetPhase(PHASE_THREE);
                    DoAction(ACTION_PHASE_3);
                }
            }

            void SummonRoyalSentinels()
            {
                for (uint8 n = 0; n < 4; n++)
                {
                    if (Creature* sentinel = me->SummonCreature(sumwave[n], lsummonpos.GetPositionX(), lsummonpos.GetPositionY(), lsummonpos.GetPositionZ()))
                        sentinel->AI()->DoZoneInCombat(sentinel, 150.0f);
                }

                for (uint8 n = 0; n < 4; n++)
                {
                    if (Creature* sentinel = me->SummonCreature(sumwave[n], rsummonpos.GetPositionX(), rsummonpos.GetPositionY(), rsummonpos.GetPositionZ()))
                        sentinel->AI()->DoZoneInCombat(sentinel, 150.0f);
                }
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_PHASE_2:
                        DoStopAttack();
                        events.CancelEvent(EVENT_TERROR);
                        events.CancelEvent(EVENT_SCREECH);
                        events.CancelEvent(EVENT_EYES_OF_EMPRESS);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        me->SetVisible(false);
                        sdiedval = 0;
                        SummonRoyalSentinels();
                        break;
                    case ACTION_PHASE_1:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        me->SetVisible(true);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat(me, 150.0f);
                        events.ScheduleEvent(EVENT_SCREECH, 8000, 0, PHASE_ONE);
                        events.ScheduleEvent(EVENT_TERROR,  urand(25000, 35000), 0, PHASE_ONE);
                        events.ScheduleEvent(EVENT_EYES_OF_EMPRESS, 10000);
                        break;
                    case ACTION_SENTINEL_DIED:
                        if (sdiedval++ >= 7 && phase == PHASE_TWO)
                        {
                            sdiedval = 0;
                            me->SetPower(POWER_ENERGY, 150);
                        }
                        break;
                    case ACTION_PHASE_3:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        me->SetVisible(true);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat(me, 150.0f);
                        me->SetPower(POWER_ENERGY, 150);
                        events.CancelEvent(EVENT_CHECK_POWER);
                        events.ScheduleEvent(EVENT_CALAMITY,          urand(60000, 90000), 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_SHA_ENERGY,        urand(20000, 30000), 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_CONSUMING_TERROR,  urand(40000, 50000), 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_VISIONS_OF_DEMISE, urand(25000, 30000), 0, PHASE_THREE);
                        events.ScheduleEvent(EVENT_EYES_OF_EMPRESS, 10000);
                        events.ScheduleEvent(EVENT_AMASSING_DARKNESS, 8000);
                        break;
                }
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_EYES_OF_EMPRESS)
                    if (Aura* aura = target->GetAura(SPELL_EYES_OF_EMPRESS))
                        if (aura->GetStackAmount() > 4)
                        {
                            target->CastSpell(target, 123713, true);
                            DoResetThreat();
                        }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventid = events.ExecuteEvent())
                {
                    switch (eventid)
                    {
                        case EVENT_CHECK_POWER:
                        {
                            switch (phase)
                            {
                                case PHASE_ONE:
                                    {
                                        if (me->GetPower(POWER_ENERGY) >= 1)
                                            me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) - 1);
                                        else if (me->GetPower(POWER_ENERGY) == 0)
                                        {
                                            phase = PHASE_TWO;
                                            events.SetPhase(PHASE_TWO);
                                            DoAction(ACTION_PHASE_2);
                                        }
                                        break;
                                    }
                                case PHASE_TWO:
                                    {
                                        if (me->GetPower(POWER_ENERGY) == 150)
                                        {
                                            phase = PHASE_ONE;
                                            events.SetPhase(PHASE_ONE);
                                            DoAction(ACTION_PHASE_1);
                                        }
                                        break;
                                    }
                                default:
                                    break;
                            }
                            events.ScheduleEvent(EVENT_CHECK_POWER, 1000);
                            break;
                        }
                        case EVENT_SCREECH:
                            DoCast(SPELL_DREAD_SCREECH);
                            events.ScheduleEvent(EVENT_SCREECH, 8000, 0, PHASE_ONE);
                            break;
                        case EVENT_TERROR:
                            DoCast(SPELL_CRY_OF_TERROR);
                            events.ScheduleEvent(EVENT_TERROR,  urand(25000, 35000), 0, PHASE_ONE);
                            break;
                        case EVENT_CALAMITY:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_CALAMITY);//AOE
                            events.ScheduleEvent(EVENT_CALAMITY, urand(60000, 90000), 0, PHASE_THREE);
                            break;
                        case EVENT_SHA_ENERGY:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_SHA_ENERGY);//AOE
                            events.ScheduleEvent(EVENT_SHA_ENERGY, urand(20000, 30000), 0, PHASE_THREE);
                            break;
                        case EVENT_CONSUMING_TERROR:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_CONSUMING_TERROR);//Cone AOE
                            events.ScheduleEvent(EVENT_CONSUMING_TERROR, urand(40000, 50000), 0, PHASE_THREE);
                            break;
                        case EVENT_VISIONS_OF_DEMISE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                                target->AddAura(SPELL_VISIONS_OF_DEMISE, target);
                            events.ScheduleEvent(EVENT_VISIONS_OF_DEMISE, urand(25000, 30000), 0, PHASE_THREE);
                            break;
                        case EVENT_EYES_OF_EMPRESS:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_EYES_OF_EMPRESS);
                            events.ScheduleEvent(EVENT_EYES_OF_EMPRESS, 10000);
                            break;
                        case EVENT_AMASSING_DARKNESS:
                            DoCast(SPELL_AMASSING_DARKNESS);
                            events.ScheduleEvent(EVENT_AMASSING_DARKNESS, 60000);
                            break;
                        case EVENT_FIXATE:
                            DoCast(SPELL_FIXATE);
                            events.ScheduleEvent(EVENT_FIXATE, 22000);
                            break;
                        case EVENT_DISPATCH:
                            DoCast(SPELL_DISPATCH);
                            events.ScheduleEvent(EVENT_DISPATCH, 20000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_empress_shekzeerAI(creature);
        }
};

class npc_generic_royal_sentinel : public CreatureScript
{
    public:
        npc_generic_royal_sentinel() : CreatureScript("npc_generic_royal_sentinel") {}

        struct npc_generic_royal_sentinelAI : public ScriptedAI
        {
            npc_generic_royal_sentinelAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_AGGRESSIVE);
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset(){}
            
            void EnterCombat(Unit* attacker)
            {
                switch (me->GetEntry())
                {
                    case NPC_SETTHIK_WINDBLADE:
                        DoCast(me, SPELL_BAND_OF_VALOR, true);
                        events.ScheduleEvent(EVENT_SONIC_BLADE, 8000);
                        break;
                    case NPC_KORTHIK_REAVER:
                        DoCast(me, SPELL_POISON_DRENCHED_ARMOR, true);
                        events.ScheduleEvent(EVENT_TOXIC_SLIME, urand(10000, 15000));
                        events.ScheduleEvent(EVENT_TOXIC_BOMB,  urand(20000, 25000));
                        break;
                }
            }

            void JustDied(Unit* killer)
            {
                if (pInstance)
                {
                    if (Creature* shekzeer = me->GetCreature(*me, pInstance->GetGuidData(NPC_SHEKZEER)))
                        shekzeer->AI()->DoAction(ACTION_SENTINEL_DIED);
                }
            }

            void SpellHit(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == 125393) //Return Fixate
                    me->AddThreat(target, 100000.0f);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TOXIC_SLIME:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_TOXIC_SLIME);
                            events.ScheduleEvent(EVENT_TOXIC_SLIME, urand(10000, 15000));
                            break;
                        case EVENT_TOXIC_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_TOXIC_BOMB);
                            events.ScheduleEvent(EVENT_TOXIC_BOMB, urand(20000, 25000));
                            break;
                        case EVENT_SONIC_BLADE:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_SONIC_BLADE);
                            events.ScheduleEvent(EVENT_SONIC_BLADE, 6000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_generic_royal_sentinelAI(creature);
        }
};

class spell_calamity : public SpellScriptLoader
{
    public:
        spell_calamity() : SpellScriptLoader("spell_calamity") { }

        class spell_calamity_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_calamity_SpellScript);

            void DealDamage()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!caster || !target)
                    return;
                
                int32 curdmg = target->GetHealth()/2;

                if (curdmg)
                    SetHitDamage(curdmg);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_calamity_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_calamity_SpellScript();
        }
};

// 124843
class spell_shekzeer_amassing_darkness : public SpellScriptLoader
{
    public:
        spell_shekzeer_amassing_darkness() : SpellScriptLoader("spell_shekzeer_amassing_darkness") { }

        class spell_shekzeer_amassing_darkness_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_shekzeer_amassing_darkness_SpellScript);

            uint8 _targetCount;

            bool Load()
            {
                _targetCount = 0;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targetsList)
            {
                if (targetsList.empty() || !GetCaster())
                    return;

                if (AuraEffect const* aurEff = GetCaster()->GetAuraEffect(124842, 0))
                {
                    _targetCount = aurEff->GetTickNumber();

                    if (_targetCount < targetsList.size())
                        targetsList.resize(_targetCount);
                    else
                        aurEff->GetBase()->Remove();
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_shekzeer_amassing_darkness_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_shekzeer_amassing_darkness_SpellScript();
        }
};

// 123788
// 123792
class spell_cry_of_terror : public SpellScriptLoader
{
    public:
        spell_cry_of_terror() : SpellScriptLoader("spell_shekzeer_cry_of_terror") {}

        class spell_cry_of_terror_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_cry_of_terror_SpellScript);

            uint8 _targetCount;

            bool Load()
            {
                _targetCount = 0;
                return true;
            }

            void FilterTargets(std::list<WorldObject*>& targetsList)
            {
                targetsList.clear();
                std::list<WorldObject*> unitsToRemove;
                if (!GetSpell() || !GetSpell()->GetTriggeredAuraEff())
                    return;

                const AuraEffect* triggerEffect = GetSpell()->GetTriggeredAuraEff();
                if (!triggerEffect->GetBase() || !triggerEffect->GetBase()->GetUnitOwner())
                    return;

                Aura* aura = triggerEffect->GetBase();
                auto target = aura->GetUnitOwner();
                if (!target || !target->ToPlayer())
                    return;

                auto player = target->ToPlayer();
                auto group = player->GetGroup();

                targetsList.push_back(player);
                if (!group)
                    return;

                for (auto& obj : targetsList)
                {
                    if (obj->GetGUID() == player->GetGUID())
                        continue;

                    auto u = obj->ToUnit();
                    auto p = obj->ToPlayer();
                    if (!p && !u)
                    {
                        unitsToRemove.push_back(obj);
                        continue;
                    }

                    if (!p && u)
                    {
                        if (!u->ToPet()->GetOwner() || !u->ToPet()->GetOwner()->ToPlayer() || !u->ToPet()->GetOwner()->ToPlayer()->IsInSameRaidWith(player))
                            unitsToRemove.push_back(u);
                    }

                    if (!p->IsInSameRaidWith(player))
                    {
                        unitsToRemove.push_back(p);
                    }
                }
               
                for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    auto p = itr->getSource();
                    if (!p || p->GetGUID() == target->GetGUID())
                        continue;

                    if (p->IsInRange(target, 0.0f, 150.0f))
                        targetsList.push_back(p);
                    else
                        unitsToRemove.push_back(p);
                }

                for (auto& t : unitsToRemove)
                    targetsList.remove(t);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_cry_of_terror_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_cry_of_terror_SpellScript();
        }
};

void AddSC_boss_empress_shekzeer()
{
    new boss_empress_shekzeer();
    new npc_generic_royal_sentinel();
    new spell_calamity();
    new spell_shekzeer_amassing_darkness();
    new spell_cry_of_terror();
}
