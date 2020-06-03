/* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
    // Jasper
    SPELL_JASPER_OVERLOAD               = 115843,
    SPELL_JASPER_PETRIFICATION          = 116036,
    SPELL_JASPER_PETRIFICATION_BAR      = 131270,
    SPELL_JASPER_TRUE_FORM              = 115828,
    SPELL_JASPER_CHAINS                 = 130395,
    SPELL_JASPER_CHAINS_VISUAL          = 130403,
    SPELL_JASPER_CHAINS_DAMAGE          = 130404,

    // Jade
    SPELL_JADE_OVERLOAD                 = 115842,
    SPELL_JADE_PETRIFICATION            = 116006,
    SPELL_JADE_PETRIFICATION_BAR        = 131269,
    SPELL_JADE_TRUE_FORM                = 115827,
    SPELL_JADE_SHARDS                   = 116223,

    // Amethyst
    SPELL_AMETHYST_OVERLOAD             = 115844,
    SPELL_AMETHYST_PETRIFICATION        = 116057,
    SPELL_AMETHYST_PETRIFICATION_BAR    = 131255,
    SPELL_AMETHYST_TRUE_FORM            = 115829,
    SPELL_AMETHYST_POOL                 = 132301,

    // Cobalt
    SPELL_COBALT_OVERLOAD               = 115840,
    SPELL_COBALT_PETRIFICATION          = 115852,
    SPELL_COBALT_PETRIFICATION_BAR      = 131268,
    SPELL_COBALT_TRUE_FORM              = 115771,
    SPELL_COBALT_MINE                   = 129460,

    // Shared
    SPELL_BERSERK                       = 26662,
    SPELL_SOLID_STONE                   = 115745,
    SPELL_REND_FLESH                    = 125206,
    SPELL_ANIM_SIT                      = 128886,
    SPELL_ZERO_ENERGY                   = 72242,
    SPELL_TOTALY_PETRIFIED              = 115877,
    
    SPELL_ACHIEV_STONE_GUARD_KILL       = 128288,
};

enum eEvents
{
    // Controler
    EVENT_CHECK_WIPE                    = 1,
    EVENT_PETRIFICATION                 = 2,

    // Guardians
    EVENT_CHECK_NEAR_GUARDIANS          = 3,
    EVENT_CHECK_ENERGY                  = 4,
    EVENT_REND_FLESH                    = 5,
    EVENT_MAIN_ATTACK                   = 6
};

uint32 guardiansEntry[4] =
{
    NPC_JASPER,
    NPC_JADE,
    NPC_AMETHYST,
    NPC_COBALT
};

class boss_stone_guard_controler : public CreatureScript
{
    public:
        boss_stone_guard_controler() : CreatureScript("boss_stone_guard_controler") {}

        struct boss_stone_guard_controlerAI : public ScriptedAI
        {
            boss_stone_guard_controlerAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                if (me->isAlive() && pInstance)
                    ResetStoneGuards();
            }

            InstanceScript* pInstance;
            EventMap events;
            uint32 lastPetrifierEntry;
            bool fightInProgress;

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetVisible(false);
                fightInProgress = false;
                lastPetrifierEntry = 0;
                pInstance->SetBossState(DATA_STONE_GUARD, NOT_STARTED);
                events.RescheduleEvent(EVENT_CHECK_WIPE, 2500);
                events.RescheduleEvent(EVENT_PETRIFICATION, 15000);
            }

            void RemovePlayerBar()
            {
                if (pInstance)
                {
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_JASPER_PETRIFICATION_BAR);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_JADE_PETRIFICATION_BAR);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_COBALT_PETRIFICATION_BAR);
                    pInstance->DoRemoveAurasDueToSpellOnPlayers(SPELL_COBALT_PETRIFICATION_BAR);
                }
            }

            void ResetStoneGuards()
            {
                for (uint8 n = 0; n < 4; n++)
                {
                    if (Creature* guardian = me->GetCreature(*me, pInstance->GetGuidData(guardiansEntry[n])))
                    {
                        if (!guardian->isAlive())
                            guardian->Respawn(true);
                    }
                }

                if (!me->GetMap()->Is25ManRaid())
                {
                    uint8 randomdespawn = urand(0, 3);
                    for (uint8 n = 0; n < 4; n++)
                    {
                        if (n == randomdespawn)
                        {
                            if (Creature* guardian = me->GetCreature(*me, pInstance->GetGuidData(guardiansEntry[n])))
                            {
                                guardian->DespawnOrUnsummon();
                                break;
                            }
                        }
                    }
                }
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case ACTION_ENTER_COMBAT:
                    {
                        for (uint8 i = 0; i < 4; ++i)
                        {
                            if (Creature* gardian = me->GetMap()->GetCreature(pInstance->GetGuidData(guardiansEntry[i])))
                            {
                                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, gardian);
                                if (gardian->isAlive() && !gardian->isInCombat())
                                    DoZoneInCombat(gardian, 150.0f);
                            }
                        }
                        events.RescheduleEvent(EVENT_PETRIFICATION, 15000);
                        fightInProgress = true;
                        pInstance->SetBossState(DATA_STONE_GUARD, IN_PROGRESS);
                        break;
                    }
                    case ACTION_GUARDIAN_DIED:
                    {
                        if (pInstance)
                        {
                            RemovePlayerBar();
                            for (uint8 i = 0; i < 4; ++i)
                            {
                                if (Creature* gardian = me->GetMap()->GetCreature(pInstance->GetGuidData(guardiansEntry[i])))
                                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, gardian);
                            }
                            
                            Map::PlayerList const& players = me->GetMap()->GetPlayers();
                            if (!players.isEmpty())
                            {
                                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                {
                                    if (Player* pPlayer = itr->getSource())
                                        me->GetMap()->ToInstanceMap()->PermBindAllPlayers(pPlayer);
                                }
                            }
                            pInstance->SetBossState(DATA_STONE_GUARD, DONE);
                            fightInProgress = false;
                            DoCast(SPELL_ACHIEV_STONE_GUARD_KILL);
                            me->Kill(me, true);
                        }
                        break;
                    }
                }
            } 
            
            void UpdateAI(uint32 diff) override
            {
                if (!fightInProgress)
                    return;

                if (!pInstance)
                    return;

                events.Update(diff);

                switch (events.ExecuteEvent())
                {
                    case EVENT_CHECK_WIPE:
                        if (pInstance->IsWipe())
                        {
                            fightInProgress = false;
                            pInstance->SetBossState(DATA_STONE_GUARD, FAIL);

                            // Their might be a wipe with player still alive but petrified, we must kill them
                            Map::PlayerList const &PlayerList = pInstance->instance->GetPlayers();
                            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                                if (Player* player = i->getSource())
                                    if (player->HasAura(SPELL_TOTALY_PETRIFIED))
                                        me->Kill(player);

                            for (uint8 i = 0; i < 4; ++i)
                                if (Creature* gardian = me->GetMap()->GetCreature(pInstance->GetGuidData(guardiansEntry[i])))
                                    pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, gardian);

                            events.Reset();
                            events.RescheduleEvent(EVENT_CHECK_WIPE, 2500);
                        }
                        else
                            events.RescheduleEvent(EVENT_CHECK_WIPE, 2500);
                        break;
                    case EVENT_PETRIFICATION:
                    {
                        if (fightInProgress)
                        {
                            bool alreadyOnePetrificationInProgress = false;

                            for (uint8 i = 0; i < 4; ++i)
                                if (ObjectGuid stoneGuardGuid = pInstance->GetGuidData(guardiansEntry[i]))
                                    if (Creature* stoneGuard = pInstance->instance->GetCreature(stoneGuardGuid))
                                        if (stoneGuard->HasAura(SPELL_JASPER_PETRIFICATION)   || stoneGuard->HasAura(SPELL_JADE_PETRIFICATION) ||
                                            stoneGuard->HasAura(SPELL_AMETHYST_PETRIFICATION) || stoneGuard->HasAura(SPELL_COBALT_PETRIFICATION))
                                        {
                                            alreadyOnePetrificationInProgress = true;
                                            break;
                                        }

                            if (alreadyOnePetrificationInProgress)
                            {
                                events.RescheduleEvent(EVENT_PETRIFICATION, 20000);
                                break;
                            }

                            uint32 nextPetrifierEntry = 0;
                            do
                            {
                                nextPetrifierEntry = guardiansEntry[rand() % 4];
                            }
                            while (nextPetrifierEntry == lastPetrifierEntry);

                            if (ObjectGuid stoneGuardGuid = pInstance->GetGuidData(nextPetrifierEntry))
                            {
                                if (Creature* stoneGuard = pInstance->instance->GetCreature(stoneGuardGuid))
                                {
                                    if (stoneGuard->isAlive())
                                    {
                                        stoneGuard->AI()->DoAction(ACTION_PETRIFICATION);
                                        lastPetrifierEntry = nextPetrifierEntry;
                                        events.RescheduleEvent(EVENT_PETRIFICATION, 90000);
                                    }
                                    else
                                        events.RescheduleEvent(EVENT_PETRIFICATION, 2000);
                                }
                                else
                                    events.RescheduleEvent(EVENT_PETRIFICATION, 2000);
                            }
                            else
                                events.RescheduleEvent(EVENT_PETRIFICATION, 2000);
                        }
                        break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_stone_guard_controlerAI(creature);
        }
};

struct boss_stone_guardianAI : ScriptedAI
{
    boss_stone_guardianAI(Creature* creature) : ScriptedAI(creature){}

    void DamageManager(InstanceScript* pInstance, Creature* caller, uint32 callerEntry, uint32 damage)
    {
        if (caller && pInstance)
        {
            for (uint8 n = 0; n < 4; n++)
                if (Creature* guardian = caller->GetCreature(*caller, pInstance->GetGuidData(guardiansEntry[n])))
                    if (guardian->isAlive() && guardian->GetEntry() != callerEntry)
                        guardian->SetHealth(guardian->GetHealth() - damage);
        }
    }

    void DiedManager(InstanceScript* pInstance, Creature* caller, uint32 callerEntry)
    {
        if (caller && pInstance)
        {
            for (uint8 n = 0; n < 4; n++)
            {
                if (Creature* guardian = caller->GetCreature(*caller, pInstance->GetGuidData(guardiansEntry[n])))
                {
                    if (guardian->isAlive() && guardian->GetEntry() != callerEntry)
                    {
                        guardian->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        guardian->Kill(guardian, true);
                        guardian->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
                    }
                }
            }
            if (Creature* controller = caller->GetCreature(*caller, pInstance->GetGuidData(NPC_STONE_GUARD_CONTROLER)))
                controller->AI()->DoAction(ACTION_GUARDIAN_DIED);
        }
    }
};

class boss_generic_guardian : public CreatureScript
{
    public:
        boss_generic_guardian() : CreatureScript("boss_generic_guardian") {}

        struct boss_generic_guardianAI : public boss_stone_guardianAI
        {
            boss_generic_guardianAI(Creature* creature) : boss_stone_guardianAI(creature), summons(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
            SummonList summons;
            uint32 spellOverloadId;
            uint32 spellPetrificationId;
            uint32 spellPetrificationBarId;
            uint32 spellTrueFormId;
            uint32 spellMainAttack;
            bool isInTrueForm; 

            Creature* GetController()
            {
                if (pInstance)
                    return me->GetMap()->GetCreature(pInstance->GetGuidData(NPC_STONE_GUARD_CONTROLER)); else return NULL;
            }

            void Reset() override
            {
                me->LowerPlayerDamageReq(me->GetMaxHealth());
                isInTrueForm = false;
                me->SetReactState(REACT_DEFENSIVE);
                me->SetPower(POWER_ENERGY, 0);
                DoCast(me, SPELL_ZERO_ENERGY, true);
                DoCast(me, SPELL_SOLID_STONE, true);
                DoCast(me, SPELL_ANIM_SIT,    true);
                summons.DespawnAll();

                switch (me->GetEntry())
                {
                    case NPC_JASPER:
                        spellOverloadId             = SPELL_JASPER_OVERLOAD;
                        spellPetrificationId        = SPELL_JASPER_PETRIFICATION;
                        spellPetrificationBarId     = SPELL_JASPER_PETRIFICATION_BAR;
                        spellTrueFormId             = SPELL_JASPER_TRUE_FORM;
                        spellMainAttack             = SPELL_JASPER_CHAINS;
                        break;
                    case NPC_JADE:
                        spellOverloadId             = SPELL_JADE_OVERLOAD;
                        spellPetrificationId        = SPELL_JADE_PETRIFICATION;
                        spellPetrificationBarId     = SPELL_JADE_PETRIFICATION_BAR;
                        spellTrueFormId             = SPELL_JADE_TRUE_FORM;
                        spellMainAttack             = SPELL_JADE_SHARDS;
                        break;
                    case NPC_AMETHYST:
                        spellOverloadId             = SPELL_AMETHYST_OVERLOAD;
                        spellPetrificationId        = SPELL_AMETHYST_PETRIFICATION;
                        spellPetrificationBarId     = SPELL_AMETHYST_PETRIFICATION_BAR;
                        spellTrueFormId             = SPELL_AMETHYST_TRUE_FORM;
                        spellMainAttack             = SPELL_AMETHYST_POOL;
                        break;
                    case NPC_COBALT:
                        spellOverloadId             = SPELL_COBALT_OVERLOAD;
                        spellPetrificationId        = SPELL_COBALT_PETRIFICATION;
                        spellPetrificationBarId     = SPELL_COBALT_PETRIFICATION_BAR;
                        spellTrueFormId             = SPELL_COBALT_TRUE_FORM;
                        spellMainAttack             = SPELL_COBALT_MINE;
                        break;
                    default:
                        spellOverloadId             = 0;
                        spellPetrificationId        = 0;
                        spellPetrificationBarId     = 0;
                        spellTrueFormId             = 0;
                        spellMainAttack             = 0;
                        break;
                }
                pInstance->DoRemoveAurasDueToSpellOnPlayers(spellPetrificationBarId);
                me->RemoveAurasDueToSpell(spellTrueFormId);
                events.Reset();
                events.RescheduleEvent(EVENT_CHECK_NEAR_GUARDIANS, 2500);
                events.RescheduleEvent(EVENT_CHECK_ENERGY, 1000);
                events.RescheduleEvent(EVENT_REND_FLESH, 5000);
                events.RescheduleEvent(EVENT_MAIN_ATTACK, 10000);
            }

            void EnterCombat(Unit* attacker) override
            {
                if (Creature* controller = GetController())
                    controller->AI()->DoAction(ACTION_ENTER_COMBAT);
                me->RemoveAurasDueToSpell(SPELL_SOLID_STONE);
                me->RemoveAurasDueToSpell(SPELL_ANIM_SIT);
            }

            void JustSummoned(Creature* summon)
            {
                summons.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (attacker->GetTypeId() == TYPEID_PLAYER)
                {
                    if (damage >= me->GetHealth())
                        DiedManager(pInstance, me, me->GetEntry());
                    else
                        DamageManager(pInstance, me, me->GetEntry(), damage);
                }
            }

            void DoAction(int32 const action) override
            {
                switch (action)
                {
                    case ACTION_PETRIFICATION:
                        me->CastSpell(me, spellPetrificationId, true);
                        pInstance->DoCastSpellOnPlayers(spellPetrificationBarId);
                        break;
                    case ACTION_FAIL:
                        EnterEvadeMode();
                        break;
                    default:
                        break;
                }
            }
            
            /* void RegeneratePower(Powers power, float &value)
            {
                if (!me->isInCombat())
                {
                    value = 0;
                    return;
                }
                
                if (CheckNearGuardians())
                   value = 1; // Creature regen every 2 seconds, and guardians must regen at 2/sec
                else
                   value = 0;
            } */

            bool CheckNearGuardians()
            {
                for (uint8 i = 0; i < 4; ++i)
                    if (guardiansEntry[i] != me->GetEntry())
                        if (Creature* gardian = GetClosestCreatureWithEntry(me, guardiansEntry[i], 12.0f, true))
                            return true;

                return false;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                switch(events.ExecuteEvent())
                {
                    case EVENT_CHECK_NEAR_GUARDIANS:
                    {
                        bool hasNearGardian = CheckNearGuardians();

                        if (!isInTrueForm && hasNearGardian)
                        {
                            me->RemoveAurasDueToSpell(SPELL_SOLID_STONE);
                            me->CastSpell(me, spellTrueFormId, true);
                            isInTrueForm = true;
                        }
                        else if (isInTrueForm && !hasNearGardian)
                        {
                            me->CastSpell(me, SPELL_SOLID_STONE, true);
                            me->RemoveAurasDueToSpell(spellTrueFormId);
                            isInTrueForm = false;
                        }
                        events.RescheduleEvent(EVENT_CHECK_NEAR_GUARDIANS, 1000);
                        break;
                    }
                    case EVENT_CHECK_ENERGY:                           
                        if (me->GetPower(POWER_ENERGY) >= 100)
                        {
                            me->CastSpell(me, spellOverloadId, false);
                            me->RemoveAurasDueToSpell(spellPetrificationId);
                            pInstance->DoRemoveAurasDueToSpellOnPlayers(spellPetrificationBarId);
                        }
                        events.RescheduleEvent(EVENT_CHECK_ENERGY, 1000);
                        break;
                    case EVENT_REND_FLESH:
                        if (Unit* victim = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        {
                            if (Aura* aura = victim->GetAura(SPELL_REND_FLESH))
                                aura->RefreshTimers();
                            else
                                me->CastSpell(victim, SPELL_REND_FLESH, false);
                        }
                        events.RescheduleEvent(EVENT_REND_FLESH, urand(4000, 6000));
                        break;
                    case EVENT_MAIN_ATTACK:
                        if (isInTrueForm)
                        {
                            switch (me->GetEntry())
                            {
                                case NPC_JADE:
                                {
                                    if (Unit* victim = SelectTarget(SELECT_TARGET_TOPAGGRO))
                                        me->CastSpell(victim, SPELL_JADE_SHARDS, false);

                                    events.RescheduleEvent(EVENT_MAIN_ATTACK, urand(15000, 19000));
                                    break;
                                }
                                case NPC_COBALT:
                                case NPC_AMETHYST:
                                {
                                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM))
                                        me->CastSpell(victim, spellMainAttack, false);

                                    events.RescheduleEvent(EVENT_MAIN_ATTACK, urand(15000, 19000));
                                    break;
                                }
                                case NPC_JASPER:
                                {
                                   /* for (uint8 i = 0; i < 2; ++i)
                                    {
                                        std::list<Player*> playerList;
                                        std::list<Player*> tempPlayerList;
                                        GetPlayerListInGrid(playerList, me, 100.0f);

										for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                                            if ((*itr)->isAlive() && !(*itr)->HasAura(SPELL_JASPER_CHAINS))
                                                tempPlayerList.push_back((*itr));

                                        if (tempPlayerList.size() < 2)
                                            break;

                                        Trinity::Containers::RandomResizeList(tempPlayerList, 2);
                                    
                                        Player* firstPlayer  = *tempPlayerList.begin();
                                        Player* SecondPlayer = *(++(tempPlayerList.begin()));

                                        if (!firstPlayer || !SecondPlayer)
                                            break;

                                        if (Aura* aura = me->AddAura(SPELL_JASPER_CHAINS, firstPlayer))
                                            aura->SetScriptGuid(0, SecondPlayer->GetGUID());

                                        if (Aura* aura = me->AddAura(SPELL_JASPER_CHAINS, SecondPlayer))
                                            aura->SetScriptGuid(0, firstPlayer->GetGUID());
                                    }

                                    events.RescheduleEvent(EVENT_MAIN_ATTACK, urand(45000, 75000));
                                    break;*/
                                    events.CancelEvent(EVENT_MAIN_ATTACK);
                                    break;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_generic_guardianAI(creature);
        }
};

enum eMineSpell
{
    SPELL_COBALT_MINE_VISUAL    = 129455,
    SPELL_COBALT_MINE_EXPLOSION = 116281
};

class mob_cobalt_mine : public CreatureScript
{
    public:
        mob_cobalt_mine() : CreatureScript("mob_cobalt_mine") {}

        struct mob_cobalt_mineAI : public ScriptedAI
        {
            mob_cobalt_mineAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetDisplayId(11686);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            uint32 active;
            bool explose;

            void Reset() override
            {
                me->AddAura(SPELL_COBALT_MINE_VISUAL, me);
                explose = false;
                active = 3000;
            }

            void EnterEvadeMode() override {}

            void EnterCombat(Unit* who) override {}

            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                if (spell->Id == 129428 && target->GetTypeId() == TYPEID_PLAYER && !explose)
                {
                    explose = true;
                    me->CastSpell(me, SPELL_COBALT_MINE_EXPLOSION, false);
                    me->DespawnOrUnsummon();
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (active)
                {
                    if (active <= diff)
                    {
                        active = 0;
                        me->AddAura(129426, me); //Aura trigger searcher dummy
                    }
                    else
                        active -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new mob_cobalt_mineAI(creature);
        }
};

// Petrification - 115852 / 116006 / 116036 / 116057
class spell_petrification : public SpellScriptLoader
{
    public:
        spell_petrification() : SpellScriptLoader("spell_petrification") { }

        class spell_petrification_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_petrification_AuraScript);

            void HandleTriggerSpell(AuraEffect const* aurEff)
            {
                PreventDefaultAction();

                Unit* caster = GetCaster();

                if (!caster)
                    return;

                std::list<Player*> playerList;
                GetPlayerListInGrid(playerList, caster, 200.0f);

				for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
					Player* target = *itr;
                    if (target->HasAura(SPELL_TOTALY_PETRIFIED))
                        continue;

                    uint32 triggeredSpell = GetSpellInfo()->Effects[0]->TriggerSpell;

                    if (!target->HasAura(triggeredSpell))
                        caster->AddAura(triggeredSpell, target);

                    if (Aura* triggeredAura = target->GetAura(triggeredSpell))
                    {
                        uint8 stackCount = triggeredAura->GetStackAmount();
                        uint8 newStackCount = stackCount + 4 > 100 ? 100: stackCount + 4;
                        triggeredAura->SetStackAmount(newStackCount);
                        triggeredAura->RefreshDuration();
                        triggeredAura->RecalculateAmountOfEffects();

                        target->SetPower(POWER_ALTERNATE, newStackCount);

                        if (newStackCount >= 100)
                            caster->AddAura(SPELL_TOTALY_PETRIFIED, target);
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_petrification_AuraScript::HandleTriggerSpell, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_petrification_AuraScript();
        }
};

// Jasper Chains - 130395
class spell_jasper_chains : public SpellScriptLoader
{
    public:
        spell_jasper_chains() : SpellScriptLoader("spell_jasper_chains") { }

        class spell_jasper_chains_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_jasper_chains_AuraScript);
            ObjectGuid playerLinkedGuid;

            bool Load()
            {
                playerLinkedGuid.Clear();
                return true;
            }

            void SetGuid(uint32 type, ObjectGuid const& guid)
            {
                playerLinkedGuid = guid;
            }

            void HandlePeriodic(AuraEffect const* aurEff)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                const SpellInfo* spell = GetSpellInfo();
                Player* linkedPlayer = sObjectAccessor->GetPlayer(*target, playerLinkedGuid);

                if (!caster || !target || !spell || !linkedPlayer || !linkedPlayer->isAlive() || !linkedPlayer->HasAura(spell->Id))
                {
                    if (Aura* myaura = GetAura())
                    {
                        myaura->Remove();
                        return;
                    }

                    return;
                }

                if (target->GetDistance(linkedPlayer) > spell->Effects[EFFECT_0]->BasePoints)
                {
                    if (Aura* aura = target->GetAura(spell->Id))
                    {
                        if (aura->GetStackAmount() >= 15)
                        {
                            aura->Remove();
                            return;
                        }
                    }
                    
                    caster->AddAura(spell->Id, target);
                    target->CastSpell(linkedPlayer, SPELL_JASPER_CHAINS_DAMAGE, true);
                }
                else
                    target->CastSpell(linkedPlayer, SPELL_JASPER_CHAINS_VISUAL, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_jasper_chains_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_jasper_chains_AuraScript();
        }
};

void AddSC_boss_stone_guard()
{
    new boss_stone_guard_controler();
    new boss_generic_guardian();
    new mob_cobalt_mine();
    new spell_petrification();
    new spell_jasper_chains();
}
