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
    //Tortos
    SPELL_FURIOS_STONE_BREATH  = 133939,
    SPELL_SNAPPING_BITE        = 135251,
    SPELL_QUAKE_STOMP          = 134920,
    SPELL_CALL_OF_TORTOS       = 136294,
    SPELL_SUM_ROCKFALL         = 134365,
    SPELL_ROCKFALL             = 134475,
    SPELL_ROCKFALL_AURA        = 134915,
    SPELL_ROCKFALL_P_DMG       = 134539,
    SPELL_GROWING_FURY         = 136010,

    //Whirl turtle
    SPELL_KICK_SHELL           = 134030,
    SPELL_KICK_SHELL_KICK_AURA = 134031,
    SPELL_KICK_SHELL_I_AURA    = 134092,
    SPELL_KICK_SHELL_T_AURA    = 136431,
    SPELL_KICK_SHELL_ROOT      = 134073,
    SPELL_SHELL_BLOCK          = 133971,
    SPELL_SHELL_BLOCK_DUMMY    = 140054,
    SPELL_SPINNING_SHELL_V     = 133974,
    SPELL_SPINNING_SHELL_DMG   = 134011,

    //Vampiric cave bat
    SPELL_DRAIN_THE_WEAK       = 135103,
    SPELL_DRAIN_THE_WEAK_HEAL  = 135102,
    SPELL_DRAIN_THE_WEAK_DMG   = 135101,
};

enum eEvents
{
    //Tortos
    EVENT_SNAPPING_BITE        = 1, 
    EVENT_QUAKE_STOMP          = 2, 
    EVENT_CALL_OF_TORTOS       = 3,
    EVENT_SUMMON_BATS          = 4,
    EVENT_ROCKFALL             = 5,

    //Whirl turtle
    EVENT_DESPAWN              = 6,
    EVENT_CHANGE_POSITION      = 7,

    //Rockfall
    EVENT_ACTIVE               = 8,
};

const float maxpullpos = 4988.0f;

Position batspawnpos[5] =
{
    { 6020.11f, 4974.91f, -47.3888f, 0.3043f},
    { 6038.12f, 4966.69f, -47.3888f, 1.4588f},
    { 6062.46f, 4964.91f, -47.3888f, 1.7690f},
    { 6054.69f, 4992.02f, -47.3888f, 4.3569f},
    { 6033.30f, 4994.31f, -47.3888f, 4.4826f},
};

class boss_tortos : public CreatureScript
{
public:
    boss_tortos() : CreatureScript("boss_tortos") {}

    struct boss_tortosAI : public BossAI
    {
        boss_tortosAI(Creature* creature) : BossAI(creature, DATA_TORTOS)
        {
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;
        uint32 checkvictim;
        uint32 updatepower;
        uint32 checkmelee;

        void Reset()
        {
            _Reset();
            AddOrRemoveSpellKickShellOnPlayers(false);
            me->RemoveAurasDueToSpell(SPELL_KICK_SHELL_T_AURA);
            me->setPowerType(POWER_ENERGY);
            me->SetPower(POWER_ENERGY, 0);
            checkvictim = 0;
            updatepower = 0;
            checkmelee = 0;
        }

        void AddOrRemoveSpellKickShellOnPlayers(bool state)
        {
            if (state)
                instance->DoCastSpellOnPlayers(SPELL_KICK_SHELL);
            else
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_KICK_SHELL);
        }

        void OnInterruptCast(Unit* /*caster*/, uint32 /*spellId*/, uint32 curSpellID, uint32 /*schoolMask*/)
        {
            if (curSpellID == SPELL_FURIOS_STONE_BREATH)
                me->SetPower(POWER_ENERGY, 0);
        }

        void JustReachedHome()
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            AddOrRemoveSpellKickShellOnPlayers(true);
            updatepower = 450;
            checkvictim = 1500;
            checkmelee = 4000;
            events.RescheduleEvent(EVENT_ROCKFALL, 9000);
            events.RescheduleEvent(EVENT_SUMMON_BATS, 30000);
            events.RescheduleEvent(EVENT_CALL_OF_TORTOS, 22000);
            events.RescheduleEvent(EVENT_SNAPPING_BITE, urand(8000, 10000));
            events.RescheduleEvent(EVENT_QUAKE_STOMP, 28000);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            AddOrRemoveSpellKickShellOnPlayers(false);
        }

        bool CheckPullPlayerPos(Unit* who)
        {
            if (!who->ToPlayer() || who->GetPositionY() > maxpullpos)
                return false;
            return true;
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (updatepower)
            {
                if (updatepower <= diff)
                {
                    if (me->GetPower(POWER_ENERGY) < 100)
                    {
                        me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) + 1);
                        if (me->GetPower(POWER_ENERGY) == 100)
                            DoCast(me, SPELL_FURIOS_STONE_BREATH);
                        updatepower = 450;
                    }
                    else if (me->GetPower(POWER_ENERGY) == 100 && !me->HasUnitState(UNIT_STATE_CASTING))
                    {
                        DoCast(me, SPELL_FURIOS_STONE_BREATH);
                        updatepower = 450;
                    }
                }
                else
                    updatepower -= diff;
            }

            if (checkvictim)
            {
                if (checkvictim <= diff)
                {
                    if (me->getVictim())
                    {
                        if (!CheckPullPlayerPos(me->getVictim()))
                        {
                            me->SetFullHealth();
                            EnterEvadeMode();
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
                case EVENT_ROCKFALL:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                        DoCast(target, SPELL_SUM_ROCKFALL);
                    events.RescheduleEvent(EVENT_ROCKFALL, 9000);
                    break;
                case EVENT_SNAPPING_BITE:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_SNAPPING_BITE);
                    events.RescheduleEvent(EVENT_SNAPPING_BITE, urand(8000, 10000));
                    break;
                case EVENT_QUAKE_STOMP:
                    DoCastAOE(SPELL_QUAKE_STOMP);
                    events.RescheduleEvent(EVENT_QUAKE_STOMP, 46000);
                    break;
                case EVENT_CALL_OF_TORTOS:
                {
                    DoCastAOE(SPELL_CALL_OF_TORTOS);
                    uint8 num = 0;
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        {
                            if (!(*itr)->isInTankSpec())
                            {
                                num++;
                                if (Creature* wt = me->SummonCreature(NPC_WHIRL_TURTLE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f))
                                    wt->GetMotionMaster()->MoveJump((*itr)->GetPositionX(), (*itr)->GetPositionY(), me->GetPositionZ(), 8.0f, 0.0f, 2);
                                if (num == 3)
                                    break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_CALL_OF_TORTOS, 60000);
                    break;
                }
                case EVENT_SUMMON_BATS:
                    for (uint8 n = 0; n < 5; n++)
                        if (Creature* vb = me->SummonCreature(NPC_VAMPIRIC_CAVE_BAT, batspawnpos[n]))
                            vb->AI()->DoZoneInCombat(vb, 100.0f);
                    events.RescheduleEvent(EVENT_SUMMON_BATS, 30000);
                    break;
                }
            }

            if (checkmelee <= diff)
            {
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                        DoCast(me, SPELL_GROWING_FURY);
                checkmelee = 4000;
            }
            else
                checkmelee -= diff;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_tortosAI(creature);
    }
};

//67966
class npc_whirl_turtle : public CreatureScript
{
public:
    npc_whirl_turtle() : CreatureScript("npc_whirl_turtle") {}

    struct npc_whirl_turtleAI : public ScriptedAI
    {
        npc_whirl_turtleAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* pInstance;
        EventMap events;
        bool kick;
        bool done;

        void Reset()
        {
            kick = false;
            done = false;
            DoCast(me, SPELL_SPINNING_SHELL_V, true);
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            if (id && !kick)
            {
                kick = true;
                if (Player* pl = me->GetPlayer(*me, guid))
                {
                    float x, y;
                    float ang = pl->GetOrientation();
                    GetPositionWithDistInOrientation(pl, 100.0f, ang, x, y);
                    me->GetMotionMaster()->MoveJump(x, y, -61.2176f, 42.0f, 0.0f, 1);
                    me->AddAura(SPELL_KICK_SHELL_I_AURA, me);
                }
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE)
            {
                switch (pointId)
                {
                case 1:
                    events.RescheduleEvent(EVENT_DESPAWN, 5000);
                    break;
                case 2:
                    events.RescheduleEvent(EVENT_CHANGE_POSITION, 6000);
                    break;
                }
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;

            if (HealthBelowPct(10) && !done)
            {
                done = true;
                events.Reset();
                me->StopMoving();
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveIdle();
                me->RemoveAurasDueToSpell(SPELL_SPINNING_SHELL_V);
                DoCast(me, SPELL_SHELL_BLOCK, true);
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_CHANGE_POSITION:
                    if (!me->HasAura(SPELL_SHELL_BLOCK))
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                            me->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), me->GetPositionZ(), 8.0f, 0.0f, 2);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_whirl_turtleAI(creature);
    }
};

//69352
class npc_vampiric_cave_bat : public CreatureScript
{
public:
    npc_vampiric_cave_bat() : CreatureScript("npc_vampiric_cave_bat") {}

    struct npc_vampiric_cave_batAI : public ScriptedAI
    {
        npc_vampiric_cave_batAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;

        void Reset(){}

        void EnterCombat(Unit* who)
        {
            DoCast(me, SPELL_DRAIN_THE_WEAK, true);
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
        return new npc_vampiric_cave_batAI(creature);
    }
};

//68219
class npc_rockfall : public CreatureScript
{
public:
    npc_rockfall() : CreatureScript("npc_rockfall") {}

    struct npc_rockfallAI : public ScriptedAI
    {
        npc_rockfallAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_ACTIVE, 500);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            damage = 0;
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
                case EVENT_ACTIVE:
                    if (Player* pl = me->FindNearestPlayer(150.0f, true))
                        DoCast(pl, SPELL_ROCKFALL, true);
                    events.RescheduleEvent(EVENT_DESPAWN, 6000);
                    break;
                case EVENT_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rockfallAI(creature);
    }
};

//134920
class spell_quake_stomp : public SpellScriptLoader
{
public:
    spell_quake_stomp() : SpellScriptLoader("spell_quake_stomp") { }

    class spell_quake_stomp_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_quake_stomp_SpellScript);

        void DealDamage()
        {
            if (GetHitUnit())
            {
                int32 curdmg = ((GetHitUnit()->GetMaxHealth() / 2) + (GetHitUnit()->GetMaxHealth() / 10));
                if (curdmg)
                    SetHitDamage(curdmg);
            }
        }

        void HandleAfterCast()
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_ROCKFALL_AURA, true);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_quake_stomp_SpellScript::DealDamage);
            AfterCast += SpellCastFn(spell_quake_stomp_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_quake_stomp_SpellScript();
    }
};

//135101
class spell_drain_the_weak : public SpellScriptLoader
{
public:
    spell_drain_the_weak() : SpellScriptLoader("spell_drain_the_weak") { }

    class spell_drain_the_weak_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_drain_the_weak_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                float heal = float(GetHitDamage() * 10);
                GetCaster()->CastCustomSpell(SPELL_DRAIN_THE_WEAK_HEAL, SPELLVALUE_BASE_POINT0, heal, GetCaster(), true);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_drain_the_weak_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_drain_the_weak_SpellScript();
    }
};

//134031
class spell_kick_shell_aura : public SpellScriptLoader
{
public:
    spell_kick_shell_aura() : SpellScriptLoader("spell_kick_shell_aura") { }

    class spell_kick_shell_aura_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kick_shell_aura_SpellScript);

        SpellCastResult CheckCast()
        {
            if (GetCaster())
            {
                Aura* aura = GetCaster()->GetAura(SPELL_SHELL_BLOCK_DUMMY);
                if (!aura)
                    return SPELL_FAILED_OUT_OF_RANGE;

                GetCaster()->RemoveAurasDueToSpell(SPELL_SHELL_BLOCK);
                return SPELL_CAST_OK;
            }
            return SPELL_FAILED_BAD_TARGETS;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_kick_shell_aura_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_kick_shell_aura_SpellScript();
    }

    class spell_kick_shell_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_kick_shell_aura_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget())
                GetTarget()->AddAura(SPELL_KICK_SHELL_ROOT, GetTarget());
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (Aura* aura = GetCaster()->GetAura(SPELL_SHELL_BLOCK_DUMMY))
                    if (Creature* wt = aura->GetCaster()->ToCreature())
                        wt->AI()->SetGUID(GetCaster()->GetGUID(), 1);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_kick_shell_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_kick_shell_aura_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_kick_shell_aura_AuraScript();
    }
};

class KickShellFilterTarget
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (unit->ToCreature() && unit->GetEntry() == NPC_TORTOS)
            return false;
        return true;
    }
};

//134091, 136431
class spell_kick_shell_dmg : public SpellScriptLoader
{
public:
    spell_kick_shell_dmg() : SpellScriptLoader("spell_kick_shell_dmg") { }

    class spell_kick_shell_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kick_shell_dmg_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(KickShellFilterTarget());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_kick_shell_dmg_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_kick_shell_dmg_SpellScript();
    }
};

class _TankFilter
{
public:
    bool operator()(WorldObject* unit)
    {
        if (Player* target = unit->ToPlayer())
            if (!target->isInTankSpec())
                return false;
        return true;
    }
};

//140431
class spell_rockfall_dummy : public SpellScriptLoader
{
public:
    spell_rockfall_dummy() : SpellScriptLoader("spell_rockfall_dummy") { }

    class spell_rockfall_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_rockfall_dummy_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
                GetCaster()->CastSpell(GetHitUnit(), SPELL_SUM_ROCKFALL, true);
        }

        void FilterTarget(std::list<WorldObject*>&targets)
        {
            targets.remove_if(_TankFilter());
            if (!targets.empty())
                if (targets.size() > 1)
                    targets.resize(1);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_rockfall_dummy_SpellScript::DealDamage);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rockfall_dummy_SpellScript::FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_rockfall_dummy_SpellScript();
    }
};

void AddSC_boss_tortos()
{
    new boss_tortos();
    new npc_whirl_turtle();
    new npc_vampiric_cave_bat();
    new npc_rockfall();
    new spell_quake_stomp();
    new spell_drain_the_weak();
    new spell_kick_shell_aura();
    new spell_kick_shell_dmg();
    new spell_rockfall_dummy();
}
