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

#include "heart_of_fear.h"

enum eSpells
{
    //Lord Meljarak
    SPELL_RECKLESSNESS          = 122354,
    SPELL_RECKLESSNESS_H        = 125873,
    SPELL_RAIN_OF_BLADES        = 122406,
    SPELL_WHIRLING_BLADE        = 121896,
    SPELL_WHIRLING_BLADE_PLR    = 124850,
    SPELL_WHIRLING_BLADE_SUM    = 124851,
    SPELL_WIND_BOMB             = 131813,
    SPELL_WATCHFUL_EYE_1        = 125933, //3 soldiers
    SPELL_WATCHFUL_EYE_2        = 125935, //2 soldiers
    SPELL_WATCHFUL_EYE_3        = 125936, //1 soldiers

    //Zarthik spells
    SPELL_HEAL                  = 122193,
    SPELL_HEAL_TR_EF            = 122147,
    SPELL_HASTE                 = 122149,

    //Sra'thik spells
    SPELL_AMBER_PRISON          = 121876,
    SPELL_AMBER_PRISON_PERIODIC = 121881,
    SPELL_AMBER_PRISON_STUN     = 121885,
    SPELL_RESIDUE               = 122055,
    SPELL_CORROSIVE_RESIN       = 122064,
    SPELL_CORROSIVE_RESIN_SUM   = 122123,
    SPELL_CORROSIVE_RESIN_A_DMG = 129005,

    //Korthik spells
    SPELL_KORTHIK_STRIKE        = 122409,
    SPELL_KORTHIK_STRIKE_FIND_T = 123963,

    //Bomb
    SPELL_WIND_BOMB_SPAWN_DMG   = 131830,
    SPELL_WIND_BOMB_VISUAL      = 131835,
    SPELL_WIND_BOMB_EXPLOSION   = 131842,
};

enum eEvents
{
    //Lord Meljarak
    EVENT_RAIN_BLADES           = 1,
    EVENT_WHIRLING_BLADE        = 2,
    EVENT_WHIRLING_BLADE_CAST   = 3,
    EVENT_WIND_BOMB             = 4,
    EVENT_CHECK_CONTROL         = 5,
    EVENT_KORTHIK_STRIKE        = 6,
    EVENT_HEROIC_SUM_ZARTHIK    = 7,
    EVENT_HEROIC_SUM_SRATHIK    = 8,
    EVENT_HEROIC_SUM_KORTHIK    = 9,

    //Soldiers
    EVENT_HEAL                  = 1, //Zarthik
    EVENT_HASTE                 = 2,
    EVENT_AMBER_PRISON          = 3, //Sra'thik
    EVENT_CORROSIVE_RESIN       = 4,
};

const AuraType auratype[6] = 
{
    SPELL_AURA_MOD_STUN,
    SPELL_AURA_MOD_FEAR,
    SPELL_AURA_MOD_CHARM,
    SPELL_AURA_MOD_CONFUSE,
    SPELL_AURA_MOD_SILENCE,
    SPELL_AURA_TRANSFORM,
};

uint32 fightSpells[8] =
{
    SPELL_WATCHFUL_EYE_1,
    SPELL_WATCHFUL_EYE_2,
    SPELL_WATCHFUL_EYE_3,
    SPELL_RECKLESSNESS,
    SPELL_RECKLESSNESS_H,
    SPELL_HASTE,
    SPELL_AMBER_PRISON_PERIODIC,
    SPELL_AMBER_PRISON_STUN
};

Position const soldiersPos[9] =
{
    {-2059.94f, 481.40f, 503.57f, 3.07f}, // 62408
    {-2058.86f, 485.71f, 503.57f, 3.06f},
    {-2062.94f, 466.99f, 503.57f, 3.13f},
    {-2063.09f, 488.55f, 503.57f, 3.25f}, // 65499
    {-2064.01f, 484.50f, 503.57f, 3.12f},
    {-2064.49f, 480.21f, 503.57f, 3.13f},
    {-2067.37f, 470.12f, 503.57f, 3.12f}, // 65500
    {-2067.95f, 465.90f, 503.57f, 3.10f},
    {-2069.22f, 461.29f, 503.57f, 3.02f},
};

class boss_lord_meljarak : public CreatureScript
{
    public:
        boss_lord_meljarak() : CreatureScript("boss_lord_meljarak") {}

        struct boss_lord_meljarakAI : public BossAI
        {
            boss_lord_meljarakAI(Creature* creature) : BossAI(creature, DATA_MELJARAK), summons(me) {}

            SummonList summons;
            uint8 soldierDied, soldierControlCount;
            bool windBomb, checkSoldier;

            void Reset()
            {
                _Reset();
                events.Reset();
                summons.DespawnAll();
                soldierDied = 0;
                windBomb = false;
                SummonSoldiers();

                for (uint8 i = 0; i < 6; i++)
                    me->RemoveAurasDueToSpell(fightSpells[i]);

                for (uint8 i = 6; i < 8; i++)
                    instance->DoRemoveAurasDueToSpellOnPlayers(fightSpells[i]);

                DespawnPrison();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                DoCast(me, SPELL_WATCHFUL_EYE_1, true);
                events.ScheduleEvent(EVENT_RAIN_BLADES, 50000); //19:05
                events.ScheduleEvent(EVENT_WHIRLING_BLADE, 35000);
                events.ScheduleEvent(EVENT_CHECK_CONTROL, 1000);
                events.ScheduleEvent(EVENT_KORTHIK_STRIKE, 30000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                summons.DespawnAll();
                DespawnPrison();
                for (uint8 i = 6; i < 8; i++)
                    instance->DoRemoveAurasDueToSpellOnPlayers(fightSpells[i]);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (me->HealthBelowPct(75) && !windBomb)
                {
                    windBomb = true;
                    events.ScheduleEvent(EVENT_WIND_BOMB, 15000);
                }
            }

            void DoAction(const int32 action)
            {
                if (IsHeroic() && action == 1 || !IsHeroic() && action > 1)
                    return;

                switch (action)
                {
                    case ACTION_1: 
                        if (soldierDied < 2)
                        {
                            me->RemoveAurasDueToSpell(fightSpells[soldierDied]);
                            soldierDied++;
                            DoCast(me, fightSpells[soldierDied], true);
                        }
                        break;
                    case ACTION_2:
                        events.ScheduleEvent(EVENT_HEROIC_SUM_ZARTHIK, 45000);
                        break;
                    case ACTION_3:
                        events.ScheduleEvent(EVENT_HEROIC_SUM_SRATHIK, 45000);
                        break;
                    case ACTION_4:
                        events.ScheduleEvent(EVENT_HEROIC_SUM_KORTHIK, 45000);
                        break;
                }
            }

            void JustSummoned(Creature* summoned)
            {
                summons.Summon(summoned);

                if (me->isInCombat())
                {
                    switch (summoned->GetEntry())
                    {
                        case NPC_ZARTHIK:
                        case NPC_SRATHIK:
                        case NPC_KORTHIK:
                            summoned->AI()->DoZoneInCombat(summoned, 100.0f);
                            break;
                    }
                }
            }

            void SummonSoldiers()
            {
                for (uint8 i = 0; i < 3; i++)
                {
                    me->SummonCreature(NPC_ZARTHIK, soldiersPos[i]);
                    me->SummonCreature(NPC_SRATHIK, soldiersPos[i+3]);
                    me->SummonCreature(NPC_KORTHIK, soldiersPos[i+6]);
                }
            }

            void DespawnPrison()
            {
                std::list<Creature*> prison;
                GetCreatureListWithEntryInGrid(prison, me, NPC_AMBER_PRISON, 120.0f);
                for (std::list<Creature*>::iterator itr = prison.begin(); itr != prison.end(); ++itr)
                    (*itr)->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->GetDistance(me->GetHomePosition()) > 70.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RAIN_BLADES:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                                DoCast(target, SPELL_RAIN_OF_BLADES);
                            events.ScheduleEvent(EVENT_RAIN_BLADES, 60000);
                            break;
                        case EVENT_WHIRLING_BLADE:
                            DoCast(SPELL_WHIRLING_BLADE_PLR);
                            events.ScheduleEvent(EVENT_WHIRLING_BLADE_CAST, 500);
                            events.ScheduleEvent(EVENT_WHIRLING_BLADE, 35000);
                            break;
                        case EVENT_WHIRLING_BLADE_CAST:
                            DoCast(SPELL_WHIRLING_BLADE);
                            break;
                        case EVENT_WIND_BOMB:
                            DoCast(SPELL_WIND_BOMB);
                            events.ScheduleEvent(EVENT_WIND_BOMB, 15000);
                            break;
                        case EVENT_KORTHIK_STRIKE:
                        {
                            DoCast(me, SPELL_KORTHIK_STRIKE_FIND_T, true);
                            EntryCheckPredicate pred(NPC_KORTHIK);
                            summons.DoAction(ACTION_1, pred);
                            events.ScheduleEvent(EVENT_KORTHIK_STRIKE, 30000);
                            break;
                        }
                        case EVENT_CHECK_CONTROL:
                        {
                            soldierControlCount = 0;
                            checkSoldier = false;
                            std::list<Creature*> soldiers;
                            GetCreatureListWithEntryInGrid(soldiers, me, NPC_SRATHIK, 100.0f);
                            GetCreatureListWithEntryInGrid(soldiers, me, NPC_ZARTHIK, 100.0f);
                            GetCreatureListWithEntryInGrid(soldiers, me, NPC_KORTHIK, 100.0f);
                            for (std::list<Creature*>::iterator itr = soldiers.begin(); itr != soldiers.end(); ++itr)
                            {
                                if ((*itr)->isAlive() && (*itr)->AI()->IsInControl())
                                {
                                    soldierControlCount++;
                                    if (IsHeroic())
                                    {
                                        if (soldierControlCount > 3)
                                            checkSoldier = true;
                                    }
                                    else
                                    {
                                        switch (soldierDied)
                                        {
                                            case 0:
                                                if (soldierControlCount > 4)
                                                    checkSoldier = true;
                                                break;
                                            case 1:
                                                if (soldierControlCount > 2)
                                                    checkSoldier = true;
                                                break;
                                            case 2:
                                                if (soldierControlCount > 0)
                                                    checkSoldier = true;
                                                break;
                                            default:
                                                break;
                                        }
                                    }
                                }
                            }
                            if (checkSoldier)
                                for (std::list<Creature*>::iterator itr = soldiers.begin(); itr != soldiers.end(); ++itr)
                                    if ((*itr)->isAlive())
                                        (*itr)->RemoveAurasWithMechanic(IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK);
                            events.ScheduleEvent(EVENT_CHECK_CONTROL, 1000);
                            break;
                        }
                        case EVENT_HEROIC_SUM_ZARTHIK:
                            for (uint8 i = 0; i < 3; i++)
                                me->SummonCreature(NPC_ZARTHIK, soldiersPos[i]);
                            break;
                        case EVENT_HEROIC_SUM_SRATHIK:
                            for (uint8 i = 3; i < 6; i++)
                                me->SummonCreature(NPC_SRATHIK, soldiersPos[i]);
                            break;
                        case EVENT_HEROIC_SUM_KORTHIK:
                            for (uint8 i = 6; i < 9; i++)
                                me->SummonCreature(NPC_KORTHIK, soldiersPos[i]);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_lord_meljarakAI(creature);
        }
};

struct npc_soldierAI : ScriptedAI
{
    npc_soldierAI(Creature* creature) : ScriptedAI(creature){}

    void SendDamageSoldiers(InstanceScript* instance, Creature* caller, uint32 callerEntry, ObjectGuid callerGuid, uint32 damage)
    {
        if (caller && instance)
        {
            switch (callerEntry)
            {
            case NPC_SRATHIK:
                for (uint32 n = NPC_SRATHIK_1; n <= NPC_SRATHIK_3; n++)
                    if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                        if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                            soldier->SetHealth(soldier->GetHealth() - damage);
                break;
            case NPC_ZARTHIK:
                for (uint32 n = NPC_ZARTHIK_1; n <= NPC_ZARTHIK_3; n++)
                    if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                        if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                            soldier->SetHealth(soldier->GetHealth() - damage);
                break;
            case NPC_KORTHIK:
                for (uint32 n = NPC_KORTHIK_1; n <= NPC_KORTHIK_3; n++)
                    if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                        if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                            soldier->SetHealth(soldier->GetHealth() - damage);
                break;
            }
        }
    }

    void SendDiedSoldiers(InstanceScript* instance, Creature* caller, uint32 callerEntry, ObjectGuid callerGuid)
    {
        if (caller && instance)
        {
            Creature* meljarak = caller->GetCreature(*caller, instance->GetGuidData(NPC_MELJARAK));
            if (!meljarak || !meljarak->isAlive())
                return;

            meljarak->AI()->DoAction(ACTION_1);
            meljarak->CastSpell(meljarak, meljarak->GetMap()->IsHeroic() ? SPELL_RECKLESSNESS_H : SPELL_RECKLESSNESS, true);

            switch (callerEntry)
            {
                case NPC_ZARTHIK:
                    meljarak->AI()->DoAction(ACTION_2);
                    for (uint32 n = NPC_ZARTHIK_1; n <= NPC_ZARTHIK_3; n++)
                        if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                            if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                                soldier->Kill(soldier, true);
                    break;
                case NPC_SRATHIK:
                    meljarak->AI()->DoAction(ACTION_3);
                    for (uint32 n = NPC_SRATHIK_1; n <= NPC_SRATHIK_3; n++)
                        if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                            if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                                soldier->Kill(soldier, true);
                    break;
                case NPC_KORTHIK:
                    meljarak->AI()->DoAction(ACTION_4);
                    for (uint32 n = NPC_KORTHIK_1; n <= NPC_KORTHIK_3; n++)
                        if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                            if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                                soldier->Kill(soldier, true);
                    break;
            }
        }
    }

    void SendHealSoldiers(InstanceScript* instance, Creature* caller, uint32 callerEntry, ObjectGuid callerGuid, uint32 modhealth)
    {
        if (caller && instance)
        {
            switch (callerEntry)
            {
                case NPC_SRATHIK:
                    for (uint32 n = NPC_SRATHIK_1; n <= NPC_SRATHIK_3; n++)
                        if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                            if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                                soldier->SetHealth(soldier->GetHealth() + modhealth);
                    break;
                case NPC_ZARTHIK:
                    for (uint32 n = NPC_ZARTHIK_1; n <= NPC_ZARTHIK_3; n++)
                        if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                            if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                                soldier->SetHealth(soldier->GetHealth() + modhealth);
                    break;
                case NPC_KORTHIK:
                    for (uint32 n = NPC_KORTHIK_1; n <= NPC_KORTHIK_3; n++)
                        if (Creature* soldier = caller->GetCreature(*caller, instance->GetGuidData(n)))
                            if (soldier->GetGUID() != callerGuid && soldier->isAlive())
                                soldier->SetHealth(soldier->GetHealth() + modhealth);
                    break;
            }
        }
    }
};

class npc_generic_soldier : public CreatureScript
{
    public:
        npc_generic_soldier() : CreatureScript("npc_generic_soldier") {}

        struct npc_generic_soldierAI : public npc_soldierAI
        {
            npc_generic_soldierAI(Creature* creature) : npc_soldierAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_AGGRESSIVE);
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset()
            {
                me->RemoveAurasDueToSpell(SPELL_HASTE);
            }
            
            void EnterCombat(Unit* attacker)
            {
                if (pInstance)
                {
                    if (Creature* meljarak = me->GetCreature(*me, pInstance->GetGuidData(NPC_MELJARAK)))
                    {
                        if (meljarak->isAlive() && !meljarak->isInCombat())
                            meljarak->AI()->AttackStart(attacker);
                    }
                }

                switch (me->GetEntry())
                {
                    case NPC_SRATHIK:
                        events.ScheduleEvent(EVENT_AMBER_PRISON, urand(35000, 90000));
                        events.ScheduleEvent(EVENT_CORROSIVE_RESIN, urand(35000, 90000));
                        break;
                    case NPC_ZARTHIK:
                        events.ScheduleEvent(EVENT_HEAL, urand(60000,  120000));
                        events.ScheduleEvent(EVENT_HASTE, urand(50000, 110000));
                        break;
                    case NPC_KORTHIK:
                        break;
                }
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_1)
                {
                    DoResetThreat();
                    DoCast(SPELL_KORTHIK_STRIKE);
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                    SendDiedSoldiers(pInstance, me, me->GetEntry(), me->GetGUID());
                else
                    SendDamageSoldiers(pInstance, me, me->GetEntry(), me->GetGUID(), damage);
            }

            bool CheckMeIsInControl()
            {
                for (uint8 n = 0; n < 6; n++)
                {
                    if (me->HasAuraType(auratype[n]))
                        return true;
                }
                return false;
            }

            void SpellHit(Unit* caster, SpellInfo const* spell)
            {
                if (spell->Id == SPELL_HEAL_TR_EF)
                {
                    uint32 modhealth = me->GetMaxHealth()/4;
                    SendHealSoldiers(pInstance, me, me->GetEntry(), me->GetGUID(), modhealth);
                }
            }

            void FindSoldierWithLowHealt()
            {
                if (!CheckMeIsInControl())
                {
                    for (uint32 n = NPC_SRATHIK_1; n <= NPC_KORTHIK_3; n++)
                    {
                        if (Creature* soldier = me->GetCreature(*me, pInstance->GetGuidData(n)))
                        {
                            if (soldier->GetGUID() != me->GetGUID())
                            {
                                if (soldier->isAlive() && soldier->HealthBelowPct(75))
                                {
                                    DoCast(soldier, SPELL_HEAL);
                                    break;
                                }
                            }
                        }
                    }
                }
                events.ScheduleEvent(EVENT_HEAL, urand(60000, 120000));
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
                        case EVENT_HEAL:
                            if (pInstance)
                                FindSoldierWithLowHealt();
                            break;
                        case EVENT_HASTE:
                            if (!CheckMeIsInControl())
                                DoCast(me, SPELL_HASTE);
                            events.ScheduleEvent(EVENT_HASTE, urand(50000, 110000));
                            break;
                        case EVENT_AMBER_PRISON:
                            DoCast(SPELL_AMBER_PRISON);
                            events.ScheduleEvent(EVENT_AMBER_PRISON, urand(30000, 90000));
                            break;
                        case EVENT_CORROSIVE_RESIN:
                            DoCast(SPELL_CORROSIVE_RESIN);
                            events.ScheduleEvent(EVENT_CORROSIVE_RESIN, urand(35000, 90000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_generic_soldierAI(creature);
        }
};

//67053
class npc_meljarak_wind_bomb : public CreatureScript
{
    public:
        npc_meljarak_wind_bomb() : CreatureScript("npc_meljarak_wind_bomb") {}

        struct npc_meljarak_wind_bombAI : public ScriptedAI
        {
            npc_meljarak_wind_bombAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetDisplayId(45684); //Bomb morph
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            InstanceScript* pInstance;
            EventMap events;
            bool active;

            void Reset() {}
            
            void EnterCombat(Unit* attacker) {}

            void IsSummonedBy(Unit* summoner)
            {
                active = false;
                DoCast(SPELL_WIND_BOMB_SPAWN_DMG);
                DoCast(me, SPELL_WIND_BOMB_VISUAL, true);
                events.ScheduleEvent(EVENT_1, 3000);
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (who->GetTypeId() != TYPEID_PLAYER || me->GetDistance(who) > 3.0f || !active)
                    return;

                active = false;
                DoCast(SPELL_WIND_BOMB_EXPLOSION); 
                me->DespawnOrUnsummon(500);
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_1:
                            active = true;
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_meljarak_wind_bombAI(creature);
        }
};

//62531
class npc_meljarak_amber_prison : public CreatureScript
{
    public:
        npc_meljarak_amber_prison() : CreatureScript("npc_meljarak_amber_prison") {}

        struct npc_meljarak_amber_prisonAI : public ScriptedAI
        {
            npc_meljarak_amber_prisonAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* pInstance;
            bool click;

            void Reset() 
            {
                click = false;
            }

            void OnSpellClick(Unit* clicker)
            {
                Unit* summoner = me->ToTempSummon()->GetSummoner();
                if (!summoner)
                    return;

                if (clicker->HasAura(SPELL_RESIDUE) || click || clicker == summoner)
                    return;

                click = true;

                clicker->CastSpell(clicker, SPELL_RESIDUE, true);

                summoner->RemoveAurasDueToSpell(SPELL_AMBER_PRISON_STUN);

                me->DespawnOrUnsummon();
            }

            void EnterCombat(Unit* attacker) {}

            void UpdateAI(uint32 diff) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_meljarak_amber_prisonAI(creature);
        }
};

//121898
class spell_meljarak_whirling_blade : public SpellScriptLoader
{
    public:
        spell_meljarak_whirling_blade() : SpellScriptLoader("spell_meljarak_whirling_blade") { }

        class spell_meljarak_whirling_blade_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_meljarak_whirling_blade_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;

                Unit* caster = GetCaster();
                if (!caster)
                    return;

                AuraEffect const* aurEff = GetSpell()->GetTriggeredAuraEff();
                if (!aurEff)
                {
                    targets.clear();
                    return;
                }

                uint32 tick = aurEff->GetTickNumber();
                Aura* auraTrigger = aurEff->GetBase();
                Creature* target = caster->FindNearestCreature(63930, 60.0f);

                float distanceintick = 6.0f * tick;
                if (distanceintick > 40.0f)
                    distanceintick = (40.0f * 2) - distanceintick;

                if (distanceintick < 0.0f || !target)
                {
                    targets.clear();
                    return;
                }

                float angle = caster->GetAngle(target);

                // expload at tick
                float x = caster->GetPositionX() + (caster->GetObjectSize() + distanceintick) * std::cos(angle);
                float y = caster->GetPositionY() + (caster->GetObjectSize() + distanceintick) * std::sin(angle);
                Trinity::NormalizeMapCoord(x);
                Trinity::NormalizeMapCoord(y);

                std::list<ObjectGuid> saveTargets = auraTrigger->GetEffectTargets();

                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
                {
                    ObjectGuid guid = (*itr)->GetGUID();
                    bool find = false;
                    if(!saveTargets.empty())
                    {
                        for (std::list<ObjectGuid>::iterator itrGuid = saveTargets.begin(); itrGuid != saveTargets.end();)
                        {
                            if(guid == (*itrGuid))
                            {
                                find = true;
                                break;
                            }
                            ++itrGuid;
                        }
                    }
                    if(find || ((*itr)->GetDistance2d(x, y) > 4.0f))
                        targets.erase(itr++);
                    else
                    {
                        auraTrigger->AddEffectTarget(guid);
                        ++itr;
                    }
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_meljarak_whirling_blade_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_meljarak_whirling_blade_SpellScript();
        }
};

//121896
class spell_meljarak_whirling_blade_visual : public SpellScriptLoader
{
    public:
        spell_meljarak_whirling_blade_visual() : SpellScriptLoader("spell_meljarak_whirling_blade_visual") { }

        class spell_meljarak_whirling_blade_visual_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_meljarak_whirling_blade_visual_AuraScript)

            Position pos, _ownPos;
            bool check;

            bool Load()
            {
                pos.Relocate(0, 0, 0, 0);
                _ownPos.Relocate(0, 0, 0, 0);
                check = false;
                return true;
            }

            void OnPereodic(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                Unit* caster = GetCaster();
                if (!caster || !check)
                    return;

                uint32 tick = aurEff->GetTickNumber() - 1;

                if (tick == 7)
                {
                    caster->SendMissileCancel(GetSpellInfo()->Effects[2]->TriggerSpell);
                    caster->SendMissileCancel(GetSpellInfo()->Effects[3]->TriggerSpell);
                    GetAura()->ClearEffectTarget();
                }

                float distanceintick = 6.0f * tick;
                if (distanceintick > 40.0f)
                    distanceintick = (40.0f * 2) - distanceintick;

                if (distanceintick < 0.0f)
                    return;

                // expload at tick
                float x = _ownPos.GetPositionX() + (caster->GetObjectSize() + distanceintick) * std::cos(_ownPos.GetOrientation());
                float y = _ownPos.GetPositionY() + (caster->GetObjectSize() + distanceintick) * std::sin(_ownPos.GetOrientation());
                Trinity::NormalizeMapCoord(x);
                Trinity::NormalizeMapCoord(y);

                caster->CastSpell(x, y, _ownPos.GetPositionZ(), GetSpellInfo()->Effects[0]->TriggerSpell, true, NULL, aurEff);
            }

            void HandleApplyEffect(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Creature* target = caster->FindNearestCreature(63930, 60.0f);
                if (!target)
                    return;

                check = true;
                float x, y;
                float angle = caster->GetAngle(target);
                
                caster->GetNearPoint2D(x, y, 40.0f, angle);
                pos.Relocate(x, y, caster->GetPositionZ(), angle);

                GetAura()->SetDuration(uint32(1000.0f * 4));
                _ownPos.Relocate(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), angle);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->SendMissileCancel(GetSpellInfo()->Effects[2]->TriggerSpell);
                    caster->SendMissileCancel(GetSpellInfo()->Effects[3]->TriggerSpell);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_meljarak_whirling_blade_visual_AuraScript::OnPereodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                OnEffectApply += AuraEffectApplyFn(spell_meljarak_whirling_blade_visual_AuraScript::HandleApplyEffect, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_meljarak_whirling_blade_visual_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        class spell_meljarak_whirling_blade_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_meljarak_whirling_blade_visual_SpellScript);
 
            void HandleTriggerEffect(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Creature* target = caster->FindNearestCreature(63930, 60.0f);
                if (!target)
                    return;

                float x, y;
                float angle = caster->GetAngle(target);
                caster->GetNearPoint2D(x, y, 40.0f, angle);

                caster->CastSpell(x, y, caster->GetPositionZ(), GetSpellInfo()->Effects[2]->TriggerSpell, true);
                caster->CastSpell(x, y, caster->GetPositionZ(), GetSpellInfo()->Effects[3]->TriggerSpell, true);
            }
            
            void Register()
            {
                OnEffectLaunch += SpellEffectFn(spell_meljarak_whirling_blade_visual_SpellScript::HandleTriggerEffect, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
                OnEffectLaunch += SpellEffectFn(spell_meljarak_whirling_blade_visual_SpellScript::HandleTriggerEffect, EFFECT_3, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };
        
        AuraScript* GetAuraScript() const
        {
            return new spell_meljarak_whirling_blade_visual_AuraScript();
        }

        SpellScript* GetSpellScript() const
        {
            return new spell_meljarak_whirling_blade_visual_SpellScript();
        }
};

//122064
class spell_meljarak_corrosive_resin : public SpellScriptLoader
{
    public:                                                      
        spell_meljarak_corrosive_resin() : SpellScriptLoader("spell_meljarak_corrosive_resin") { }

        class spell_meljarak_corrosive_resin_AuraScript : public AuraScript 
        {
            PrepareAuraScript(spell_meljarak_corrosive_resin_AuraScript) 

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                GetAura()->SetStackAmount(5);
            }

            void OnPereodic(AuraEffect const* /*aurEff*/) 
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                InstanceScript* pInstance = target->GetInstanceScript();
                if (!pInstance)
                    return;

                if (target->isMoving())
                {
                    if (Creature* meljarak = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_MELJARAK)))
                        target->CastSpell(target, SPELL_CORROSIVE_RESIN_SUM, true, NULL, NULL, meljarak->GetGUID());

                    GetAura()->SetStackAmount(GetAura()->GetStackAmount() - 1);
                    if (GetAura()->GetStackAmount() < 1)
                        GetAura()->Remove();
                }
            }

            void Register() 
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_meljarak_corrosive_resin_AuraScript::OnPereodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectApply += AuraEffectApplyFn(spell_meljarak_corrosive_resin_AuraScript::OnApply, EFFECT_1, SPELL_AURA_SCREEN_EFFECT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_meljarak_corrosive_resin_AuraScript();
        }
};

void AddSC_boss_lord_meljarak()
{
    new boss_lord_meljarak();
    new npc_generic_soldier();
    new npc_meljarak_wind_bomb();
    new npc_meljarak_amber_prison();
    new spell_meljarak_whirling_blade();
    new spell_meljarak_whirling_blade_visual();
    new spell_meljarak_corrosive_resin();
}