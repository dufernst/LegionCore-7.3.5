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
    SPELL_STATIC_BURST              = 137162,
    SPELL_STATIC_WOUND              = 138349,
    SPELL_BALL_LIGHTNING            = 136620,
    SPELL_STORM                     = 137313,
    SPELL_STORM_SUMMON              = 137283,

    //Lightning ball
    SPELL_LIGHTNING_BALL_VISUAL     = 137425,
    SPELL_LIGHTNING_BALL_AURA_INC_S = 137389, //increase speed
    SPELL_LIGHTNING_BALL_DUMMY      = 137429,
    SPELL_LIGHTNING_BALL_TARGET     = 137422,
    SPELL_EXPLOSE_LIGHTNING_BALL    = 137374, //finilize pursuit player
    SPELL_LIGHTNING_BALL_DMG        = 137423,

    //Lightning Fissure
    SPELL_LIGHTNING_FISSURE_VISUAL  = 137480,
    SPELL_LIGHTNING_FISSURE_DMG     = 137484,

    SPELL_LIGHTNING_FISSURE_DMG_EX  = 138133,
    SPELL_LIGHTNING_FISSURE_DMG_EX2 = 137530, //if player

    SPELL_WATER_POOL_VISUAL         = 137277, //Visual water pool
    SPELL_WATER_POOL_SCALE_AURA     = 137676,
    SPELL_STATIC_WATER_VISUAL       = 137978, //Visual static water pool
    SPELL_WATER_FONT_VISUAL         = 137340, //Visual water from font's head

    //Thundering throw
    SPELL_THUNDERING_THROW          = 137180,
    SPELL_THUNDERING_THROW_AURA     = 137161,
    SPELL_THUNDERING_THROW_JUMP     = 137173,
    SPELL_THUNDERING_THROW_DMG      = 137370,
    SPELL_THUNDERING_THROW_AOE_DMG  = 137167,
    SPELL_THUNDERING_THROW_STUN     = 137371,

    SPELL_CONDUCTIVE_WATER_DEBUFF   = 138470,
    SPELL_FLUIDITY                  = 138002,
    SPELL_ELECTRIFIED_WATERS        = 138006,

    SPELL_FOCUSED_LIGHTNING_CONDUCTION2 = 137530,
    SPELL_FOCUSED_LIGHTNING_VIOLENT_DETONATION = 138990,

    //Storm Visual
    SPELL_LIGHTNING_STORM_VISUAL    = 140811,
    SPELL_CONDUCTIVE_WATER_STROM_A  = 138568, //periodic aura
    SPELL_CONDUCTION_WATER_STORM_V  = 138647, //storm visual
    SPELL_BERSERK                   = 47008,
};

enum Events
{
    EVENT_LIGHTNING_BALL            = 1,
    EVENT_STATIC_BURST              = 3,
    EVENT_THUNDERING_THROW          = 4,
    EVENT_STORM_PREPARE             = 5,
    EVENT_STORM                     = 6,

    EVENT_RE_ATTACK                 = 19,
    EVENT_ACTIVE                    = 20,
    EVENT_SEARCH_PLAYERS            = 21,
};

enum Actions
{
    ACTION_RE_ATTACK_AFTER_STORM,
    ACTION_EXPLOSE,
    ACTION_SET_STORM_TIMER,
    ACTION_CHARGE_CONDUCTIVE_WATER,
};

uint32 mogufontsentry[4] =
{
    GO_MOGU_SR,
    GO_MOGU_NR,
    GO_MOGU_NL,
    GO_MOGU_SL,
};

Position mogufontdestpos[4] =
{
    {5842.01f, 6314.41f, 158.45f},
    {5841.02f, 6213.49f, 158.47f},
    {5942.02f, 6212.31f, 158.47f},
    {5944.27f, 6315.71f, 158.48f},
};

Position fontspos[4] =
{
    {5838.10f, 6317.60f, 156.99f, 5.43f},
    {5837.58f, 6210.22f, 156.53f, 0.82f},
    {5945.87f, 6208.96f, 156.68f, 2.32f},
    {5947.25f, 6318.95f, 156.97f, 3.90f},
};

Position jumpdestpos[4] =
{
    {5863.90f, 6291.79f, 125.0353f, 5.41f},
    {5864.57f, 6236.31f, 125.0353f, 0.78f},
    {5919.86f, 6235.83f, 125.0353f, 2.34f},
    {5919.53f, 6290.47f, 125.0353f, 3.89f},
};

class boss_jinrokh : public CreatureScript
{
public:
    boss_jinrokh() : CreatureScript("boss_jinrokh") {}

    struct boss_jinrokhAI : public BossAI
    {
        boss_jinrokhAI(Creature* creature) : BossAI(creature, DATA_JINROKH)
        {
            instance = creature->GetInstanceScript();
        }
        InstanceScript* instance;
        ObjectGuid lastvictimGuid;
        uint32 berserk;

        void Reset()
        {
            _Reset();
            berserk = 0;
            lastvictimGuid.Clear();
            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_STATIC_BURST, 12000);
            events.RescheduleEvent(EVENT_LIGHTNING_BALL, 25000);
            events.RescheduleEvent(EVENT_THUNDERING_THROW, 30000);
            if (me->GetMap()->IsHeroic())
                berserk = 360000;
        }

        void JustDied(Unit* killer)
        {
            _JustDied();
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LIGHTNING_FISSURE_DMG);
        }

        void ChargeConductionWater()
        {
            std::list<Creature*>cwlist;
            cwlist.clear();
            GetCreatureListWithEntryInGrid(cwlist, me, NPC_CONDUCTIVE_WATER, 150.0f);
            if (!cwlist.empty())
            {
                for (std::list<Creature*>::const_iterator itr = cwlist.begin(); itr != cwlist.end(); itr++)
                {
                    if ((*itr)->HasAura(SPELL_WATER_POOL_VISUAL))
                    {
                        (*itr)->RemoveAurasDueToSpell(SPELL_WATER_POOL_VISUAL);
                        (*itr)->CastSpell(*itr, SPELL_STATIC_WATER_VISUAL, true);
                    }
                }
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
                if (pointId == 1)
                    events.RescheduleEvent(EVENT_STORM, 500);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_CHARGE_CONDUCTIVE_WATER:
                ChargeConductionWater();
                break;
            case ACTION_RE_ATTACK_AFTER_STORM:
                me->SetReactState(REACT_AGGRESSIVE);
                events.RescheduleEvent(EVENT_STATIC_BURST, 12000);
                events.RescheduleEvent(EVENT_LIGHTNING_BALL, 10000);
                break;
            case ACTION_SET_STORM_TIMER:
                events.RescheduleEvent(EVENT_STORM_PREPARE, 60000);
                break;
            }
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                DoCast(who, SPELL_THUNDERING_THROW);
                events.RescheduleEvent(EVENT_RE_ATTACK, 2500);
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (berserk)
            {
                if (berserk <= diff)
                {
                    berserk = 0;
                    DoCast(me, SPELL_BERSERK, true);
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
                case EVENT_STATIC_BURST:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_STATIC_BURST);
                    events.RescheduleEvent(EVENT_STATIC_BURST, 20000);
                    break;
                case EVENT_LIGHTNING_BALL:
                {
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->isInTankSpec())
                            {
                                if (Creature* ball = me->SummonCreature(NPC_LIGHTNING_BALL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()))
                                {
                                    ball->AI()->SetGUID((*itr)->GetGUID(), 0);
                                    break;
                                }
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_LIGHTNING_BALL, 25000);
                    break;
                }
                case EVENT_THUNDERING_THROW:
                    if (instance && instance->GetData(DATA_CHECK_VALIDATE_THUNDERING_THROW))
                    {
                        if (me->getVictim())
                        {
                            if (Player* ttpl = me->getVictim()->ToPlayer())
                            {
                                me->StopAttack();
                                lastvictimGuid = ttpl->GetGUID();
                                ttpl->EnterVehicle(me, 0);
                                events.DelayEvents(5000);
                            }
                        }
                    }
                    break;
                case EVENT_STORM_PREPARE:
                    events.Reset();
                    me->StopAttack();
                    me->GetMotionMaster()->MoveCharge(5891.61f, 6263.18f, 124.035f, 15.0f, 1);
                    break;
                case EVENT_STORM:
                    DoCast(me, SPELL_STORM);
                    events.RescheduleEvent(EVENT_THUNDERING_THROW, 32500);
                    break;
                case EVENT_RE_ATTACK: //After Thundering Throw
                {
                    bool havetarget = false;
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 80.0f);
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    {
                        if ((*itr)->GetGUID() != lastvictimGuid)
                        {
                            havetarget = true;
                            me->getThreatManager().resetAllAggro();
                            me->AddThreat(*itr, 5000.0f);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->Attack(*itr, true);
                            me->GetMotionMaster()->MoveChase(*itr);
                        }
                    }
                    lastvictimGuid.Clear();
                    if (!havetarget)
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat(me, 150.0f);
                    }
                }
                break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jinrokhAI(creature);
    }
};

//69232
class npc_lightning_ball : public CreatureScript
{
public:
    npc_lightning_ball() : CreatureScript("npc_lightning_ball") {}

    struct npc_lightning_ballAI : public ScriptedAI
    {
        npc_lightning_ballAI(Creature* creature) : ScriptedAI(creature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK_DEST, true);
            InstanceScript* pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            DoCast(me, SPELL_LIGHTNING_BALL_VISUAL, true);
            targetGuid.Clear();
        }

        InstanceScript* pInstance;
        EventMap events;
        ObjectGuid targetGuid;

        void Reset(){}

        void DoAction(int32 const action)
        {
            if (action == ACTION_EXPLOSE)
            {
                me->StopAttack();
                DoCast(me, SPELL_EXPLOSE_LIGHTNING_BALL, true);
                if (me->ToTempSummon())
                    if (Unit* jinrokh = me->ToTempSummon()->GetSummoner())
                        jinrokh->SummonCreature(NPC_LIGHTNING_FISSURE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                me->DespawnOrUnsummon();
            }
        }
        
        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            targetGuid = guid;
            events.RescheduleEvent(EVENT_ACTIVE, 3000);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE)
                {
                    if (Player* pl = me->GetPlayer(*me, targetGuid))
                    {
                        if (pl->isAlive())
                        {
                            DoCast(me, SPELL_LIGHTNING_BALL_AURA_INC_S, true);
                            DoCast(pl, SPELL_LIGHTNING_BALL_TARGET, true);
                            me->AddThreat(pl, 50000000.0f);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->Attack(pl, true);
                            me->GetMotionMaster()->MoveChase(pl);
                            return;
                        }
                    }
                    me->DespawnOrUnsummon();
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lightning_ballAI(creature);
    }
};

//69609
class npc_lightning_fissure : public CreatureScript
{
public:
    npc_lightning_fissure() : CreatureScript("npc_lightning_fissure") {}

    struct npc_lightning_fissureAI : public ScriptedAI
    {
        npc_lightning_fissureAI(Creature* creature) : ScriptedAI(creature)
        {
            InstanceScript* pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetDisplayId(11686);
        }

        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            me->SetFloatValue(OBJECT_FIELD_SCALE, 0.5f);
            DoCast(me, SPELL_LIGHTNING_FISSURE_VISUAL, true);
            events.RescheduleEvent(EVENT_SEARCH_PLAYERS, 2000);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_SEARCH_PLAYERS)
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 40.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        {
                            if (me->GetExactDist2d(*itr) <= 3.0f)
                                (*itr)->CastSpell(*itr, SPELL_LIGHTNING_FISSURE_DMG, true);
                            else
                                (*itr)->RemoveAurasDueToSpell(SPELL_LIGHTNING_FISSURE_DMG);
                        }
                    }
                    events.RescheduleEvent(EVENT_SEARCH_PLAYERS, 1000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lightning_fissureAI(creature);
    }
};

//90005
class npc_mogu_font : public CreatureScript
{
public:
    npc_mogu_font() : CreatureScript("npc_mogu_font") { }

    struct npc_mogu_fontAI : public ScriptedAI
    {
        npc_mogu_fontAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }

        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_ACTIVE, 1000);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE)
                    DoCast(me, SPELL_WATER_FONT_VISUAL, true);
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_mogu_fontAI(pCreature);
    }
};

//69469
class npc_conductive_water : public CreatureScript
{
public:
    npc_conductive_water() : CreatureScript("npc_conductive_water") { }

    struct npc_conductive_waterAI : public ScriptedAI
    {
        npc_conductive_waterAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }

        void Reset()
        {
            me->SetFloatValue(OBJECT_FIELD_SCALE, 1.0f);
            DoCast(me, SPELL_WATER_POOL_VISUAL, true);
            DoCast(me, SPELL_WATER_POOL_SCALE_AURA, true);
            if (me->ToTempSummon())
                if (Unit* jinrokh = me->ToTempSummon()->GetSummoner())
                    if (jinrokh->ToCreature())
                        if (jinrokh->isAlive() && jinrokh->isInCombat())
                            jinrokh->ToCreature()->AI()->DoAction(ACTION_SET_STORM_TIMER);
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_conductive_waterAI(pCreature);
    }
};

//69676
class npc_storm_stalker : public CreatureScript
{
public:
    npc_storm_stalker() : CreatureScript("npc_storm_stalker") { }

    struct npc_storm_stalkerAI : public ScriptedAI
    {
        npc_storm_stalkerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }

        void Reset()
        {
            DoCast(me, SPELL_CONDUCTIVE_WATER_STROM_A, true);
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_storm_stalkerAI(pCreature);
    }
};

class spell_static_burst : public SpellScriptLoader
{
public:
    spell_static_burst() : SpellScriptLoader("spell_static_burst") { }

    class spell_static_burst_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_static_burst_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            {
                if (GetTarget() && GetCaster())
                    GetCaster()->CastCustomSpell(SPELL_STATIC_WOUND, SPELLVALUE_AURA_STACK, 10, GetTarget());
            }
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_static_burst_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_static_burst_AuraScript();
    }
};

//138389
class spell_static_wound : public SpellScriptLoader
{
public:
    spell_static_wound() : SpellScriptLoader("spell_static_wound") { }

    class spell_static_wound_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_static_wound_SpellScript);

        void _HandleHit()
        {
            if (GetHitUnit())
                if (!GetHitUnit()->HasAura(SPELL_STATIC_WOUND))
                    SetHitDamage(GetHitDamage() / 3);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_static_wound_SpellScript::_HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_static_wound_SpellScript();
    }
};

//138349
class spell_static_wound_periodic : public SpellScriptLoader
{
public:
    spell_static_wound_periodic() : SpellScriptLoader("spell_static_wound_periodic") { }

    class spell_static_wound_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_static_wound_periodic_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetTarget())
                if (Aura* aura = GetTarget()->GetAura(SPELL_STATIC_WOUND))
                    if (aura->GetStackAmount() > 1)
                        aura->SetStackAmount(aura->GetStackAmount() - 1);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_static_wound_periodic_AuraScript::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_static_wound_periodic_AuraScript();
    }
};

//137180
class spell_thundering_throw : public SpellScriptLoader
{
public:
    spell_thundering_throw() : SpellScriptLoader("spell_thundering_throw") { }

    class spell_thundering_throw_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_thundering_throw_SpellScript);

        uint8 GetFontNum(uint32 entry)
        {
            switch (entry)
            {
            case GO_MOGU_SR:
                return 0;
            case GO_MOGU_NR:
                return 1;
            case GO_MOGU_NL:
                return 2;
            case GO_MOGU_SL:
                return 3;
            }
            return 0;
        }

        void _HandleHit()
        {
            if (GetHitUnit() && GetCaster())
            {
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_THUNDERING_THROW_AURA, true);
                std::list<GameObject*>mogufonts;
                mogufonts.clear();
                uint8 num = 0;

                for (uint8 n = 0; n < 4; n++)
                    if (GameObject* go = GetHitUnit()->FindNearestGameObject(mogufontsentry[n], 150.0f))
                        if (go->GetGoState() == GO_STATE_READY)
                            mogufonts.push_back(go);

                if (!mogufonts.empty())
                {
                    for (std::list<GameObject*>::const_iterator itr = mogufonts.begin(); itr != mogufonts.end(); itr++)
                    {
                        if (GetCaster()->isInFront(*itr, 2.5f))
                        {
                            num = GetFontNum((*itr)->GetEntry());
                            GetCaster()->SetFacingToObject(*itr);
                            GetHitUnit()->ExitVehicle();
                            GetHitUnit()->CastSpell(mogufontdestpos[num].GetPositionX(), mogufontdestpos[num].GetPositionY(), mogufontdestpos[num].GetPositionZ(), SPELL_THUNDERING_THROW_JUMP);
                            return;
                        }
                    }

                    //if Go not found isinfront at boss, use random
                    for (std::list<GameObject*>::const_iterator itr = mogufonts.begin(); itr != mogufonts.end(); itr++)
                    {
                        num = GetFontNum((*itr)->GetEntry());
                        GetCaster()->SetFacingToObject(*itr);
                        GetHitUnit()->ExitVehicle();
                        GetHitUnit()->CastSpell(mogufontdestpos[num].GetPositionX(), mogufontdestpos[num].GetPositionY(), mogufontdestpos[num].GetPositionZ(), SPELL_THUNDERING_THROW_JUMP);
                        break;
                    }
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_thundering_throw_SpellScript::_HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_thundering_throw_SpellScript();
    }
};

//137161
class spell_thundering_throw_aura : public SpellScriptLoader
{
public:
    spell_thundering_throw_aura() : SpellScriptLoader("spell_thundering_throw_aura") { }

    class spell_thundering_throw_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_thundering_throw_aura_AuraScript);

        uint8 GetFontNum(uint32 entry)
        {
            switch (entry)
            {
            case GO_MOGU_SR:
                return 0;
            case GO_MOGU_NR:
                return 1;
            case GO_MOGU_NL:
                return 2;
            case GO_MOGU_SL:
                return 3;
            }
            return 0;
        }

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_CANCEL && GetTarget())
            {
                if (InstanceScript* instance = GetTarget()->GetInstanceScript())
                {
                    if (Creature* jinrokh = GetTarget()->GetCreature(*GetTarget(), instance->GetGuidData(NPC_JINROKH)))
                    {
                        if (jinrokh->isAlive() && jinrokh->isInCombat())
                        {
                            for (uint8 n = 0; n < 4; n++)
                            {
                                if (GameObject* go = GetTarget()->FindNearestGameObject(mogufontsentry[n], 60.0f))
                                {
                                    GetTarget()->CastSpell(GetTarget(), SPELL_THUNDERING_THROW_DMG, true);
                                    GetTarget()->CastSpell(GetTarget(), SPELL_THUNDERING_THROW_STUN, true);
                                    go->SetGoState(GO_STATE_ACTIVE);
                                    uint8 num = GetFontNum(go->GetEntry());
                                    GetTarget()->GetMotionMaster()->MoveJump(jumpdestpos[num].GetPositionX(), jumpdestpos[num].GetPositionY(), jumpdestpos[num].GetPositionZ(), 35.0f, 15.0f, 137161);
                                    jinrokh->SummonCreature(NPC_MOGU_FONT, fontspos[num].GetPositionX(), fontspos[num].GetPositionY(), fontspos[num].GetPositionZ(), fontspos[num].GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 50000);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_thundering_throw_aura_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_MOD_SILENCE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_thundering_throw_aura_AuraScript();
    }
};

//137167
class spell_thundering_throw_aoe : public SpellScriptLoader
{
public:
    spell_thundering_throw_aoe() : SpellScriptLoader("spell_thundering_throw_aoe") { }

    class spell_thundering_throw_aoe_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_thundering_throw_aoe_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
                if (Creature* jinrokh = GetCaster()->FindNearestCreature(NPC_JINROKH, 150.0f, true))
                    jinrokh->SummonCreature(NPC_CONDUCTIVE_WATER, GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ());
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_thundering_throw_aoe_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_thundering_throw_aoe_SpellScript();
    }
};

//137676
class spell_conductive_water : public SpellScriptLoader
{
public:
    spell_conductive_water() : SpellScriptLoader("spell_conductive_water") { }

    class spell_conductive_water_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_conductive_water_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                float dist = GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE)*3.8f;
                std::list<Player*>pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), dist);
                if (!pllist.empty())
                {
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                    {
                        if ((*itr)->isAlive() && GetCaster()->GetExactDist2d(*itr) <= dist)
                        {
                            (*itr)->CastSpell(*itr, SPELL_CONDUCTIVE_WATER_DEBUFF, true, 0, 0, GetCaster()->GetGUID());
                            (*itr)->CastSpell(*itr, GetCaster()->HasAura(SPELL_STATIC_WATER_VISUAL) ? SPELL_ELECTRIFIED_WATERS : SPELL_FLUIDITY, true);
                        }
                    }
                }
                std::list<Creature*>lflist;
                lflist.clear();
                GetCreatureListWithEntryInGrid(lflist, GetCaster(), NPC_LIGHTNING_FISSURE, dist);
                if (!lflist.empty())
                {
                    for (std::list<Creature*>::const_iterator itr = lflist.begin(); itr != lflist.end(); itr++)
                    {
                        (*itr)->DespawnOrUnsummon();
                        GetCaster()->CastSpell(GetCaster(), SPELL_LIGHTNING_FISSURE_DMG_EX, true);
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_conductive_water_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_conductive_water_AuraScript();
    }
};

//137313
class spell_jinrokh_storm : public SpellScriptLoader
{
public:
    spell_jinrokh_storm() : SpellScriptLoader("spell_jinrokh_storm") { }

    class spell_jinrokh_storm_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_jinrokh_storm_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster())
            {
                GetCaster()->SummonCreature(NPC_STORM_STALKER, 5891.61f, 6263.18f, 124.035f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 15000);
                if (GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_CHARGE_CONDUCTIVE_WATER);
            }
        }

        void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH && GetCaster())
                if (GetCaster()->ToCreature() && GetCaster()->isInCombat())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK_AFTER_STORM);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_jinrokh_storm_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_jinrokh_storm_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_jinrokh_storm_AuraScript();
    }
};

//138568
class spell_conductive_water_periodic : public SpellScriptLoader
{
public:
    spell_conductive_water_periodic() : SpellScriptLoader("spell_conductive_water_periodic") { }

    class spell_conductive_water_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_conductive_water_periodic_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster())
            {
                float x, y;
                GetPosInRadiusWithRandomOrientation(GetCaster(), float(urand(5, 55)), x, y);
                GetCaster()->CastSpell(x, y, GetCaster()->GetPositionZ(), SPELL_CONDUCTION_WATER_STORM_V, true);

                float x1, y1;
                GetPosInRadiusWithRandomOrientation(GetCaster(), float(urand(5, 55)), x1, y1);
                GetCaster()->CastSpell(x1, y1, GetCaster()->GetPositionZ(), SPELL_LIGHTNING_STORM_VISUAL, true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_conductive_water_periodic_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_conductive_water_periodic_AuraScript();
    }
};

//137389
class spell_focused_lightning : public SpellScriptLoader
{
public:
    spell_focused_lightning() : SpellScriptLoader("spell_focused_lightning") { }

    class spell_focused_lightning_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_focused_lightning_AuraScript);

        void OnPeriodic(AuraEffect const*aurEff)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_LIGHTNING_BALL_DUMMY, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_focused_lightning_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_focused_lightning_AuraScript();
    }
};

//137429
class spell_focused_lightning_dummy : public SpellScriptLoader
{
public:
    spell_focused_lightning_dummy() : SpellScriptLoader("spell_focused_lightning_dummy") { }

    class spell_focused_lightning_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_focused_lightning_dummy_SpellScript);

        void HandleHit()
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetHitUnit())
            {
                if (GetHitUnit()->HasAura(SPELL_LIGHTNING_BALL_TARGET))
                {
                    GetCaster()->RemoveAurasDueToSpell(SPELL_LIGHTNING_BALL_AURA_INC_S);
                    GetHitUnit()->RemoveAurasDueToSpell(SPELL_LIGHTNING_BALL_TARGET);
                    if (GetHitUnit()->HasAura(SPELL_CONDUCTIVE_WATER_DEBUFF))
                    {
                        GetCaster()->AttackStop();
                        GetCaster()->ToCreature()->SetReactState(REACT_PASSIVE);
                        GetCaster()->StopMoving();
                        GetCaster()->GetMotionMaster()->Clear(false);
                        GetCaster()->CastSpell(GetHitUnit(), SPELL_FOCUSED_LIGHTNING_VIOLENT_DETONATION, true);
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_FOCUSED_LIGHTNING_CONDUCTION2, true);
                        GetCaster()->ToCreature()->DespawnOrUnsummon();
                    }
                    else
                        GetCaster()->ToCreature()->AI()->DoAction(ACTION_EXPLOSE);
                }
                else
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_LIGHTNING_BALL_DMG, true);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_focused_lightning_dummy_SpellScript::HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_focused_lightning_dummy_SpellScript();
    }
};


class LightningFissureConductionFilter
{
public:
    LightningFissureConductionFilter(Unit* caster) : _caster(caster){}

    bool operator()(WorldObject* unit)
    {
        if (Player* pl = unit->ToPlayer())
            if (pl->HasAura(SPELL_CONDUCTIVE_WATER_DEBUFF))
                if (pl->GetAura(SPELL_CONDUCTIVE_WATER_DEBUFF)->GetCasterGUID() == _caster->GetGUID())
                    return false;
        return true;
    }
private:
    Unit* _caster;
};

//138133
class spell_lightning_fissure_conduction : public SpellScriptLoader
{
public:
    spell_lightning_fissure_conduction() : SpellScriptLoader("spell_lightning_fissure_conduction") { }

    class spell_lightning_fissure_conduction_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lightning_fissure_conduction_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
                targets.remove_if(LightningFissureConductionFilter(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_fissure_conduction_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lightning_fissure_conduction_SpellScript();
    }
};

//137530
class spell_lightning_fissure_conduction_extra : public SpellScriptLoader
{
public:
    spell_lightning_fissure_conduction_extra() : SpellScriptLoader("spell_lightning_fissure_conduction_extra") { }

    class spell_lightning_fissure_conduction_extra_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lightning_fissure_conduction_extra_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (GetCaster() && GetCaster()->HasAura(SPELL_CONDUCTIVE_WATER_DEBUFF))
            {
                if (Unit* caster = GetCaster()->GetAura(SPELL_CONDUCTIVE_WATER_DEBUFF)->GetCaster())
                    targets.remove_if(LightningFissureConductionFilter(caster));
                else
                    targets.clear();
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_lightning_fissure_conduction_extra_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_DEST_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lightning_fissure_conduction_extra_SpellScript();
    }
};

void AddSC_boss_jinrokh()
{
    new boss_jinrokh();
    new npc_lightning_ball();
    new npc_lightning_fissure();
    new npc_mogu_font();
    new npc_conductive_water();
    new npc_storm_stalker();
    new spell_static_burst();
    new spell_static_wound();
    new spell_static_wound_periodic();
    new spell_thundering_throw();
    new spell_thundering_throw_aura();
    new spell_thundering_throw_aoe();
    new spell_conductive_water();
    new spell_jinrokh_storm();
    new spell_conductive_water_periodic();
    new spell_focused_lightning();
    new spell_focused_lightning_dummy();
    new spell_lightning_fissure_conduction();
    new spell_lightning_fissure_conduction_extra();
}
