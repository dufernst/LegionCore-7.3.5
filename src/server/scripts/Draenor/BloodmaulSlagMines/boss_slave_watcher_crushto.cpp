/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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
#include "ScriptedCreature.h"
#include "bloodmaul_slag_mines.h"
#include "Map.h"

enum BroadcastTexts
{
    TEXT_0, // 'Стражи! Хватит зевать, пора убивать.'
    TEXT_1, // '...Мои рабы! Собственность Кровавого Молота!'
    TEXT_2, // 'Идите сюда! Я закую вас в цепи.'
    TEXT_3, // 'Хватайте этого!'
    TEXT_4, // 'Рабы! Бейтесь или умрите!
    TEXT_5, // 'У-у… мы вас всех убьем…'
};

enum Spells
{
    SPELL_EARTH_CRUSH                   = 153679, //< cast this after SPELL_EARTH_CRUSH_2
    SPELL_EARTH_CRUSH_2                 = 153732, //< cast this after SPELL_EARTH_CRUSH_4 - target = NPC_EARTH_CRUSH_STALKER
    SPELL_EARTH_CRUSH_3                 = 167246, // visual
    SPELL_EARTH_CRUSH_4                 = 153735, //< summoning NPC_EARTH_CRUSH_STALKER

    SPELL_FEROCIOUS_YELL                = 150759,

    SPELL_WILD_SLAM                     = 150753,

    SPELL_CRUSHING_LEAP                 = 150745,
    SPELL_CRUSHING_LEAP_TRIGGER_1       = 150746,
    SPELL_CRUSHING_LEAP_TRIGGER_2       = 150751,

    SPELL_RAISE_THE_MINERS              = 150801,
    SPELL_RAISE_THE_MINERS_TRIGGER_1    = 150775,
    SPELL_RAISE_THE_MINERS_TRIGGER_2    = 150779,
    SPELL_RAISE_THE_MINERS_TRIGGER_3    = 150780,

    SPELL_SLAYER_OF_THE_SLAVER          = 163577,

    //< Miners
    SPELL_TRAUMATIC_STRIKE              = 150807,
    SPELL_WEAKENED_WILL                 = 150811,
};

enum NPCs
{
    NPC_CAPTURED_MINER                  = 83725, //< for SPELL_RAISE_THE_MINERS
    NPC_EARTH_CRUSH_STALKER             = 83650, //< for SPELL_EARTH_CRUSH
    NPC_BLOODMAUL_WARDER                = 75210,
};

Position const warderPoint[] =
{
    {2042.93f, -305.80f, 226.56f},
    {2053.88f, -307.19f, 226.61f}
};

class boss_slave_watcher_crushto : public CreatureScript
{
public:
    boss_slave_watcher_crushto() : CreatureScript("boss_slave_watcher_crushto") { }

    struct boss_slave_watcher_crushtoAI : public BossAI
    {
        boss_slave_watcher_crushtoAI(Creature* creature) : BossAI(creature, DATA_SLAVE_WATCHER_CRUSHTO) { }

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
            _Reset();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();

            Talk(TEXT_2);

            events.RescheduleEvent(EVENT_1, 11 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, urand(14, 26) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_3, urand(3, 8) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_4, 5 * IN_MILLISECONDS);
        }

        void EnterEvadeMode()
        {
            BossAI::EnterEvadeMode();
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            me->DeleteThreatList();

            std::list<Player*> players;
            GetPlayerListInGrid(players, me, 100.0f);

            for (auto const& player : players)
            {
                if (!player->HasAura(SPELL_SLAYER_OF_THE_SLAVER))
                    player->AddAura(SPELL_SLAYER_OF_THE_SLAVER, player);
            }

            Talk(TEXT_5);
        }

        void DamageTaken(Unit* /*attacker*/, uint32 &/*damage*/, DamageEffectType dmgType) { }

        void DoAction(int32 const action)
        {
            switch (action)
            {
                case ACTION_1:
                {
                    Talk(TEXT_0);

                    uint8 p = urand(0, 1);
                    std::list<Creature*> creList;
                    GetCreatureListWithEntryInGrid(creList, me, NPC_BLOODMAUL_WARDER, 90.0f);
                    for (auto const& warder : creList)
                        if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST, 0, 120.0f, true))
                            warder->AI()->AttackStart(target);
                        else
                            warder->GetMotionMaster()->MovePoint(p + 1, warderPoint[p]);
                    break;
                }
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->GetDistance(me->GetHomePosition()) >= 120.0f)
            {
                EnterEvadeMode();
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, urand(15, 25) * IN_MILLISECONDS);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                            me->CastSpell(target, SPELL_CRUSHING_LEAP);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, urand(10, 15) * IN_MILLISECONDS);
                        Talk(TEXT_4);
                        me->CastSpell(me, SPELL_RAISE_THE_MINERS);
                        break;
                    case EVENT_3:
                        events.RescheduleEvent(EVENT_3, 20 * IN_MILLISECONDS);
                        me->CastSpell(me, SPELL_WILD_SLAM);
                        break;
                    case EVENT_4:
                    {
                        events.RescheduleEvent(EVENT_4, urand(15, 21) * IN_MILLISECONDS);
                        float x = 0.0f, y = 0.0f;
                        GetRandPosFromCenterInDist(me->GetPositionX(), me->GetPositionY(), 5.0f, x, y);
                        me->CastSpell(x, y, me->GetPositionZ(), SPELL_EARTH_CRUSH_4);
                        if (Creature* stalker = me->SummonCreature(NPC_EARTH_CRUSH_STALKER, x, y, me->GetPositionZ(), 0.0f, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            me->AddAura(SPELL_EARTH_CRUSH_3, stalker);
                            me->CastSpell(stalker, SPELL_EARTH_CRUSH_2);
                            me->CastSpell(stalker, SPELL_EARTH_CRUSH);
                            stalker->DespawnOrUnsummon(2.5 * IN_MILLISECONDS);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
        bool intro;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_slave_watcher_crushtoAI(creature);
    }
};

class npc_captured_miner : public CreatureScript
{
public:
    npc_captured_miner() : CreatureScript("npc_captured_miner") { }

    struct npc_captured_minerAI : public ScriptedAI
    {
        npc_captured_minerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();

            events.RescheduleEvent(EVENT_5, urand(4, 7) * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_6, 10 * IN_MILLISECONDS);

            DoAttackerAreaInCombat(me, 100.0f);
        }

        void EnterCombat(Unit* /*who*/) { }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_5:
                        events.RescheduleEvent(EVENT_5, urand(2, 5) * IN_MILLISECONDS);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 20.0f, true, SPELL_CRUSHING_LEAP_TRIGGER_2))
                            me->CastSpell(target, SPELL_TRAUMATIC_STRIKE);
                        else
                            me->CastSpell(SelectTarget(SELECT_TARGET_TOPAGGRO), SPELL_TRAUMATIC_STRIKE);
                        break;
                    case EVENT_6:
                        events.RescheduleEvent(EVENT_6, 3 * IN_MILLISECONDS);
                        if (Aura* aura = me->GetAura(SPELL_FEROCIOUS_YELL))
                            if (aura->GetStackAmount() == 3)
                                me->CastSpell(me, SPELL_WEAKENED_WILL, true);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_captured_minerAI(creature);
    }
};

class spell_crushing_leap : public SpellScriptLoader
{
public:
    spell_crushing_leap() : SpellScriptLoader("spell_crushing_leap") { }

    class spell_crushing_leap_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_crushing_leap_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitUnit();
            if (!caster || !target)
                return;

            caster->AddAura(SPELL_CRUSHING_LEAP_TRIGGER_1, target);
            caster->AddAura(SPELL_CRUSHING_LEAP_TRIGGER_2, target);
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_crushing_leap_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_crushing_leap_SpellScript();
    }
};

class spell_raise_the_miners : public SpellScriptLoader
{
public:
    spell_raise_the_miners() : SpellScriptLoader("spell_raise_the_miners") { }

    class spell_raise_the_miners_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_raise_the_miners_SpellScript);

        void HandleDummy(SpellEffIndex effIndex)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            float x = 0.0f, y = 0.0f;
            for (uint8 i = 0; i < 3; ++i)
            {
                GetRandPosFromCenterInDist(caster->GetPositionX(), caster->GetPositionY(), frand(3.0f, 8.0f), x, y);

                caster->CastSpell(x + float(i), y, caster->GetPositionZ(), SPELL_RAISE_THE_MINERS_TRIGGER_1, true);
                caster->CastSpell(x - float(i), y - float(i), caster->GetPositionZ(), SPELL_RAISE_THE_MINERS_TRIGGER_2, true);
                caster->CastSpell(x, y + float(i), caster->GetPositionZ(), SPELL_RAISE_THE_MINERS_TRIGGER_3, true);
            }
        }

        void Register()
        {
            OnEffectLaunch += SpellEffectFn(spell_raise_the_miners_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_raise_the_miners_SpellScript();
    }
};

void AddSC_boss_slave_watcher_crushto()
{
    new boss_slave_watcher_crushto();

    new npc_captured_miner();

    new spell_crushing_leap();
    new spell_raise_the_miners();
}
