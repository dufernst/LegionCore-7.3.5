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

#include "mogu_shan_vault.h"

enum eSpells
{
    SPELL_BANISHMENT            = 116272,
    SPELL_SOUL_CUT_SUICIDE      = 116278,
    SPELL_SOUL_CUT_DAMAGE       = 117337,

    SPELL_VOODOO_DOLL_VISUAL    = 122151,
    SPELL_VOODOO_DOLL_SHARE     = 116000,
    SPELL_SUMMON_SPIRIT_TOTEM   = 116174,
    SPELL_SUMMON_MINION         = 118087,

    // attaques ombreuses
    SPELL_RIFHT_CROSS           = 117215,
    SPELL_LEFT_HOOK             = 117218,
    SPELL_HAMMER_FIST           = 117219,
    SPELL_SWEEPING_KICK         = 117222,

    SPELL_FRENESIE              = 117752,

    // Shadowy Minion
    SPELL_SHADOW_BOLT           = 122118,
    SPELL_SPIRITUAL_GRASP       = 118421,

    // Misc
    SPELL_CLONE                 = 119051,
    SPELL_CLONE_VISUAL          = 119053,
    SPELL_LIFE_FRAGILE_THREAD   = 116227,
    SPELL_CROSSED_OVER          = 116161, // Todo : virer le summon

    SPELL_FRAIL_SOUL            = 117723,
};

enum eEvents
{
    EVENT_SECONDARY_ATTACK      = 1,
    EVENT_SUMMON_TOTEM          = 2,
    EVENT_SUMMON_SHADOWY_MINION = 3,
    EVENT_VOODOO_DOLL           = 4,
    EVENT_BANISHMENT            = 5,

    // Shadowy Minion
    EVENT_SHADOW_BOLT           = 6,
    EVENT_SPIRITUAL_GRASP       = 7,
};

float const minpullpos = 4238.0410f; //x
float const maxpullpos = 1380.4799f; //y

class boss_garajal : public CreatureScript
{
    public:
        boss_garajal() : CreatureScript("boss_garajal") {}

        struct boss_garajalAI : public BossAI
        {
            boss_garajalAI(Creature* creature) : BossAI(creature, DATA_GARAJAL)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            ObjectGuid voodooTargets[4];
            uint32 checkvictim;

            void Reset() override
            {
                _Reset();
                me->RemoveAllAuras();
                checkvictim = 0;
                pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_VISUAL);
                pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_SHARE);
                pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BANISHMENT);

                for (uint8 n = 0; n < 4; n++) 
                    voodooTargets[n].Clear();
            }

            void JustReachedHome() override
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
            }

             bool CheckPullPlayerPos(Unit* who)
            {
                if (!who->ToPlayer() || who->GetPositionX() < minpullpos || who->GetPositionY() > maxpullpos)
                    return false;

                return true;
            }

            void EnterCombat(Unit* who) override
            {
                if (instance)
                {
                    if (!CheckPullPlayerPos(who))
                    {
                        EnterEvadeMode();
                        return;
                    }
                }
                _EnterCombat();
                checkvictim = 1500;
                events.RescheduleEvent(EVENT_SECONDARY_ATTACK,      urand(5000, 10000));
                events.RescheduleEvent(EVENT_SUMMON_TOTEM,          35000);
                events.RescheduleEvent(EVENT_SUMMON_SHADOWY_MINION, urand(10000, 15000));
                events.RescheduleEvent(EVENT_BANISHMENT,            90000);
                events.RescheduleEvent(EVENT_VOODOO_DOLL,           3000);
            }

            void JustDied(Unit* attacker) override
            {
                _JustDied();
                pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_VISUAL);
                pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_SHARE);
                pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BANISHMENT);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (me->HealthBelowPctDamaged(20, damage) && !me->HasAura(SPELL_FRENESIE))
                {
                    events.CancelEvent(EVENT_SUMMON_TOTEM);
                    DoCast(me, SPELL_FRENESIE, true);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

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

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_SECONDARY_ATTACK:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            {
                                uint32 spellId = RAND(SPELL_RIFHT_CROSS, SPELL_LEFT_HOOK, SPELL_HAMMER_FIST, SPELL_SWEEPING_KICK);
                                me->CastSpell(target, spellId, true);
                            }
                            events.RescheduleEvent(EVENT_SECONDARY_ATTACK, urand(5000, 10000));
                            break;
                        }
                        case EVENT_SUMMON_TOTEM:
                        {
                            /* float x = 0.0f, y = 0.0f;
                            GetRandPosFromCenterInDist(4277.08f, 1341.35f, frand(0.0f, 30.0f), x, y);
                            me->SummonCreature(NPC_SPIRIT_TOTEM, x, y, 454.55f, 0.0f, TEMPSUMMON_CORPSE_DESPAWN); */
                            DoCast(me, SPELL_SUMMON_SPIRIT_TOTEM);
                            events.RescheduleEvent(EVENT_SUMMON_TOTEM, 35000);
                            break;
                        }
                        case EVENT_SUMMON_SHADOWY_MINION:
                        {
                            float x = 0.0f, y = 0.0f;
                            GetRandPosFromCenterInDist(4277.08f, 1341.35f, frand(0.0f, 30.0f), x, y);
                            me->SummonCreature(NPC_SHADOWY_MINION_REAL, x, y, 454.55f, 0.0f, TEMPSUMMON_CORPSE_DESPAWN);
                            events.RescheduleEvent(EVENT_SUMMON_SHADOWY_MINION,     urand(10000, 15000));
                            break;
                        }
                        case EVENT_VOODOO_DOLL:
                        {
                            pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_VISUAL);
                            pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_SHARE);

                            uint8 mobCount = Is25ManRaid() ? 4: 3;
                            for (uint8 i = 0; i < mobCount; ++i)
                            {
                                if (Unit* target = SelectTarget(i == 0 ? SELECT_TARGET_TOPAGGRO : SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                {
                                    if (!target->HasAura(SPELL_VOODOO_DOLL_VISUAL))
                                    {
                                        voodooTargets[i] = target->GetGUID();
                                        target->AddAura(SPELL_VOODOO_DOLL_VISUAL, target);
                                    }
                                }
                            }

                            if (Player* plc = me->GetPlayer(*me, voodooTargets[0]))
                                if (Player* plt = me->GetPlayer(*me, voodooTargets[1]))
                                    plc->CastSpell(plt, SPELL_VOODOO_DOLL_SHARE);

                            if (Player* plc = me->GetPlayer(*me, voodooTargets[1]))
                                if (Player* plt = me->GetPlayer(*me, voodooTargets[2]))
                                    plc->CastSpell(plt, SPELL_VOODOO_DOLL_SHARE);

                            if (Is25ManRaid())
                            {
                                if (Player* plc = me->GetPlayer(*me, voodooTargets[2]))
                                    if (Player* plt = me->GetPlayer(*me, voodooTargets[3]))
                                        plc->CastSpell(plt, SPELL_VOODOO_DOLL_SHARE);
                            }

                            for (uint8 n = 0; n < 4; n++)
                                voodooTargets[n].Clear();

                            break;
                        }
                        case EVENT_BANISHMENT:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            {
                                me->AddAura(SPELL_BANISHMENT,       target);
                                me->AddAura(SPELL_SOUL_CUT_SUICIDE, target);
                                me->AddAura(SPELL_SOUL_CUT_DAMAGE,  target);

                                Difficulty difficulty = me->GetMap()->GetDifficultyID();
                                ObjectGuid viewerGuid;
                                if (difficulty != DIFFICULTY_LFR)
                                    viewerGuid = target->GetGUID();
                                uint8 mobCount = IsHeroic() ? 3: 1;

                                for (uint8 i = 0; i < mobCount; ++i)
                                {
                                    if (Creature* soulCutter = me->SummonCreature(NPC_SOUL_CUTTER, target->GetPositionX() + 2.0f, target->GetPositionY() + 2.0f, target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000, i == 0 ? viewerGuid : ObjectGuid::Empty))
                                    {
                                        soulCutter->SetPhaseMask(2, true);
                                        soulCutter->AI()->AttackStart(target);
                                        soulCutter->SetInCombatWith(target);
                                        soulCutter->getThreatManager().addThreat(target, 10000.0f);
                                    }
                                }
                                me->getThreatManager().resetAllAggro();
                            }
                            pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_VISUAL);
                            pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOODOO_DOLL_SHARE);
                            events.RescheduleEvent(EVENT_VOODOO_DOLL, 5000);
                            events.RescheduleEvent(EVENT_BANISHMENT, 90000);
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_garajalAI(creature);
        }
};
//Creature 60240
class mob_spirit_totem : public CreatureScript
{
    public:
        mob_spirit_totem() : CreatureScript("mob_spirit_totem") {}

        struct mob_spirit_totemAI : public ScriptedAI
        {
            mob_spirit_totemAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;

            void Reset() override
            {
                me->AddAura(116827, me);
                me->SetReactState(REACT_PASSIVE);
            }

            void JustDied(Unit* attacker) override
            {
                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, me, 5.0f);

                uint8 count = 0;
				for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
					Player* player = *itr;
                    
                    if (player->HasAura(SPELL_FRAIL_SOUL))
                    {
                        me->Kill(player);
                        continue;
                    }

                    if (player->HasAura(SPELL_VOODOO_DOLL_VISUAL))
                        continue;

                    if (++count > 3)
                        break;

                    if (Creature* clone = me->SummonCreature(56405, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation()))
                    {
                        if (player->isAlive())
                            player->SetHealth(player->GetHealth() * 0.3);

                        player->CastSpell(player, SPELL_CLONE_VISUAL, true);
                        player->CastSpell(player, SPELL_CROSSED_OVER, true);
                        player->CastSpell(clone,  SPELL_CLONE, true);
                        clone->CastSpell(clone, SPELL_LIFE_FRAGILE_THREAD, true);
                        clone->GetMotionMaster()->MoveTakeoff(1, clone->GetPositionX(), clone->GetPositionY(), clone->GetPositionZ() + 10.0f);
                        player->AddAura(SPELL_LIFE_FRAGILE_THREAD, player);
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (pInstance)
                    if (pInstance->GetBossState(DATA_GARAJAL) != IN_PROGRESS)
                        me->DespawnOrUnsummon();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_spirit_totemAI(creature);
        }
};
//Creature 60940, 60184
class mob_shadowy_minion : public CreatureScript
{
    public:
        mob_shadowy_minion() : CreatureScript("mob_shadowy_minion") {}

        struct mob_shadowy_minionAI : public Scripted_NoMovementAI
        {
            mob_shadowy_minionAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;

            ObjectGuid spiritGuid;
            EventMap events;

            void Reset() override
            {
                events.Reset();
                spiritGuid.Clear();

                if (me->GetEntry() == NPC_SHADOWY_MINION_REAL)
                {
                    if (Creature* spirit = me->SummonCreature(NPC_SHADOWY_MINION_SPIRIT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN))
                    {
                        spiritGuid = spirit->GetGUID();
                        spirit->SetPhaseMask(2, true);
                    }

                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                    events.RescheduleEvent(EVENT_SPIRITUAL_GRASP, urand(2000, 5000));
                }
                else
                    events.RescheduleEvent(EVENT_SHADOW_BOLT, urand(2000, 5000));

                DoZoneInCombat();
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_SHADOWY_MINION_SPIRIT)
                    me->DespawnOrUnsummon();
            }

            void JustDied(Unit* attacker) override
            {
                if (me->GetEntry() == NPC_SHADOWY_MINION_SPIRIT)
                    if (me->ToTempSummon())
                        if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                            if (summoner->ToCreature())
                                summoner->ToCreature()->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) override
            {
                if (pInstance)
                    if (pInstance->GetBossState(DATA_GARAJAL) != IN_PROGRESS)
                        me->DespawnOrUnsummon();

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        // Spirit World
                        case EVENT_SHADOW_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_SHADOW_BOLT);
                            events.RescheduleEvent(EVENT_SHADOW_BOLT, urand(2000, 3000));
                            break;
                        // Real World
                        case EVENT_SPIRITUAL_GRASP:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_SPIRITUAL_GRASP);
                            events.RescheduleEvent(EVENT_SPIRITUAL_GRASP, urand(5000, 8000));
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_shadowy_minionAI(creature);
        }
};
//Creature 62003
class mob_soul_cutter : public CreatureScript
{
    public:
        mob_soul_cutter() : CreatureScript("mob_soul_cutter") {}

        struct mob_soul_cutterAI : public ScriptedAI
        {
            mob_soul_cutterAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;

            void Reset() override
            {}

            void JustDied(Unit* attacker) override
            {
                std::unordered_set<ObjectGuid> playerList;
                me->GetMustBeVisibleForPlayersList(playerList);

				for (std::unordered_set<ObjectGuid>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
                    if (Player* player = ObjectAccessor::FindPlayer(*itr))
                    {
                        player->RemoveAurasDueToSpell(SPELL_BANISHMENT);
                        player->RemoveAurasDueToSpell(SPELL_SOUL_CUT_SUICIDE);
                        player->RemoveAurasDueToSpell(SPELL_SOUL_CUT_DAMAGE);
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (pInstance)
                    if (pInstance->GetBossState(DATA_GARAJAL) != IN_PROGRESS)
                        me->DespawnOrUnsummon();

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_soul_cutterAI(creature);
        }
};

// Life Fragile 116227
class spell_life_fragile : public SpellScriptLoader
{
    public:
        spell_life_fragile() : SpellScriptLoader("spell_life_fragile") { }

        class spell_life_fragile_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_life_fragile_AuraScript);

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                if (GetTarget()->GetTypeId() == TYPEID_PLAYER)
                {
                    if (GetTarget()->GetHealth() == GetTarget()->GetMaxHealth())
                        GetTarget()->CastSpell(GetTarget(), 120717); //Bar, if use, return target in real world
                }
            }
            
            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_life_fragile_AuraScript::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_life_fragile_AuraScript();
        }
};

// Soul Back - 120715
class spell_soul_back : public SpellScriptLoader
{
    public:
        spell_soul_back() : SpellScriptLoader("spell_soul_back") { }

        class spell_soul_back_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_soul_back_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    // SPELL_LIFE_FRAGILE_THREAD removed by default effect
                    target->RemoveAurasDueToSpell(SPELL_CLONE_VISUAL);
                    target->RemoveAurasDueToSpell(SPELL_CROSSED_OVER);
                    if (target->GetMap()->IsHeroic())
                        target->AddAura(SPELL_FRAIL_SOUL, target);
                    target->SetHealth(target->GetHealth() * 0.3);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_soul_back_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_REMOVE_AURA);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_soul_back_SpellScript();
        }
};


void AddSC_boss_garajal()
{
    new boss_garajal();
    new mob_spirit_totem();
    new mob_shadowy_minion();
    new mob_soul_cutter();
    new spell_life_fragile();
    new spell_soul_back();
}
