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
    SPELL_DECAPITATE_T_A    = 134912, 
    SPELL_DECAPITATE_TR_M   = 134990, 
    SPELL_STATIC_SHOCK      = 135695,
    SPELL_LIGHTNING_WHIP    = 136850,
};

enum eEvents
{
    EVENT_DECAPITATE        = 1,
    EVENT_STATIC_SHOCK      = 2,
    EVENT_LIGHTNING_WHIP    = 3,
    EVENT_RE_ATTACK         = 4,
};

enum sActions
{
    ACTION_RE_ATTACK        = 1,
};

class boss_lei_shen : public CreatureScript
{
    public:
        boss_lei_shen() : CreatureScript("boss_lei_shen") {}

        struct boss_lei_shenAI : public BossAI
        {
            boss_lei_shenAI(Creature* creature) : BossAI(creature, DATA_LEI_SHEN)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_DEFENSIVE);
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                events.RescheduleEvent(EVENT_DECAPITATE,     30000);
                events.RescheduleEvent(EVENT_LIGHTNING_WHIP, 40000);
                events.RescheduleEvent(EVENT_STATIC_SHOCK,   50000);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_RE_ATTACK)
                    events.RescheduleEvent(EVENT_RE_ATTACK, 1000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
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
                    case EVENT_DECAPITATE:
                        if (me->getVictim())
                            DoCast(me->getVictim(), SPELL_DECAPITATE_T_A);
                        events.RescheduleEvent(EVENT_DECAPITATE, 30000);
                        break;
                    case EVENT_LIGHTNING_WHIP:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        {
                            me->StopAttack();
                            me->SetFacingToObject(target);
                            DoCast(target, SPELL_LIGHTNING_WHIP);
                        }
                        events.RescheduleEvent(EVENT_LIGHTNING_WHIP, 40000);
                        break;
                    case EVENT_STATIC_SHOCK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true))
                            DoCast(target, SPELL_STATIC_SHOCK);
                        events.RescheduleEvent(EVENT_STATIC_SHOCK, 50000);
                        break;
                    case EVENT_RE_ATTACK:
                        me->SetReactState(REACT_AGGRESSIVE);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_lei_shenAI(creature);
        }
};

//134912
class spell_decapitate_aura : public SpellScriptLoader
{
    public:
        spell_decapitate_aura() : SpellScriptLoader("spell_decapitate_aura") { }

        class spell_decapitate_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_decapitate_aura_AuraScript);

            void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetTarget())
                    GetCaster()->CastSpell(GetTarget(), SPELL_DECAPITATE_TR_M, true);
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_decapitate_aura_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_decapitate_aura_AuraScript();
        }
};

//134916
class spell_decapitate : public SpellScriptLoader
{
    public:
        spell_decapitate() : SpellScriptLoader("spell_decapitate") { }

        class spell_decapitate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_decapitate_SpellScript);

            void DealDamage()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;
                
                float dist = GetCaster()->GetExactDist2d(GetHitUnit());

                if (dist >= 0 && dist <= 100)
                {
                    if (GetCaster()->IsWithinMeleeRange(GetHitUnit()))
                        SetHitDamage(3000000);
                    else
                        SetHitDamage((3000000/dist)*5);
                }
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_decapitate_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_decapitate_SpellScript();
        }
};

//136850
class spell_lightning_whip : public SpellScriptLoader
{
    public:
        spell_lightning_whip() : SpellScriptLoader("spell_lightning_whip") { }
        
        class spell_lightning_whip_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_lightning_whip_SpellScript);
            
            void OnAfterCast()
            {
                if (GetCaster() && GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_lightning_whip_SpellScript::OnAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_lightning_whip_SpellScript();
        }
};

//218417
class go_lei_shen_tp_platform : public GameObjectScript
{
    public:
        go_lei_shen_tp_platform() : GameObjectScript("go_lei_shen_tp_platform") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            InstanceScript* pInstance = (InstanceScript*)go->GetInstanceScript();

            if (!pInstance)
                return false;
            
            if (pInstance->GetBossState(DATA_LEI_SHEN) != IN_PROGRESS)
                player->NearTeleportTo(5670.09f, 4094.19f, 156.4627f, 0.0755f);
            
            return true;
        }
};

//218418
class go_tp_platform_to_ra_den : public GameObjectScript
{
    public:
        go_tp_platform_to_ra_den() : GameObjectScript("go_tp_platform_to_ra_den") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            InstanceScript* pInstance = (InstanceScript*)go->GetInstanceScript();

            if (!pInstance || !player->GetMap()->IsHeroic())
                return false;
            
            if (pInstance->GetBossState(DATA_LEI_SHEN) == DONE)
                player->NearTeleportTo(5591.90f, 4692.89f, 55.7776f, 4.75735f);
            
            return true;
        }
};

void AddSC_boss_lei_shen()
{
    new boss_lei_shen();
    new spell_decapitate_aura();
    new spell_decapitate();
    new spell_lightning_whip();
    new go_lei_shen_tp_platform();
    new go_tp_platform_to_ra_den();
}
