/*
* Copyright (C) 2012-2013 JadeCore <http://www.pandashan.com/>
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

#include "ScriptedCreature.h"
#include "terrace_of_endless_spring.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"
#include "GridNotifiers.h"

enum eAshaniGossips
{
    ASHANI_LAST_GOSSIP = 60010,
    ASHANI_LAST_OPTION = 51
};

// Springtender Ashani - 64846
class npc_springtender_ashani : public CreatureScript
{
public:
    npc_springtender_ashani() : CreatureScript("npc_springtender_ashani") { }

    struct npc_springtender_ashaniAI : public ScriptedAI
    {
        npc_springtender_ashaniAI(Creature* creature) : ScriptedAI(creature) { }

        void sGossipSelect(Player* player, uint32 sender, uint32 action)
        {
            if (sender == ASHANI_LAST_GOSSIP && action == ASHANI_LAST_OPTION)
            {
                if (InstanceScript* pInstance = me->GetInstanceScript())
                {
                    pInstance->SetData(SPELL_RITUAL_OF_PURIFICATION, 0);
                    me->DespawnOrUnsummon(2000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_springtender_ashaniAI(creature);
    }
};

enum eApparitionsSpells
{
    // Shared
    SPELL_CHANNELING_CORRUPTION = 125706,
    SPELL_CORRUPTION_BOLT = 125713,

    // Fear
    SPELL_OVERWHELMING_FEAR = 125758,
    SPELL_NIGHT_TERRORS_MISSILE = 125736,
    SPELL_NIGHT_TERRORS_PERIODIC = 125745,
    SPELL_ENVELOPING_DARKNESS = 125760,

    // Terror
    SPELL_GRIP_OF_FEAR = 130115,
    SPELL_UNLEASHED_TERROR = 130120
};

enum eApparitionsEvents
{
    // Fear
    EVENT_OVERWHELMING_FEAR = 1,
    EVENT_NIGHT_TERRORS_MISSILE = 2,
    EVENT_NIGHT_TERRORS_AREA = 3,
    EVENT_ENVELOPING_DARKNESS = 4,

    // Terror
    EVENT_GRIP_OF_FEAR = 5,
    EVENT_UNLEASHED_TERROR = 6
};

// Apparition of Fear - 64368
class npc_apparition_of_fear : public CreatureScript
{
public:
    npc_apparition_of_fear() : CreatureScript("npc_apparition_of_fear") { }

    struct npc_apparition_of_fearAI : public ScriptedAI
    {
        npc_apparition_of_fearAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;
        EventMap events;
        SummonList summons;

        uint32 corruptionBoltTimer;

        void Reset()
        {
            me->CastSpell(me, SPELL_CHANNELING_CORRUPTION, true);

            events.Reset();

            corruptionBoltTimer = 3000;

            summons.DespawnAll();

            if (pInstance)
                pInstance->SetData(INTRO_DONE, 0);
        }

        void EnterCombat(Unit* attacker)
        {
            // Schedule Combat event here
            events.Reset();

            events.ScheduleEvent(EVENT_OVERWHELMING_FEAR, 8000);
            events.ScheduleEvent(EVENT_NIGHT_TERRORS_MISSILE, 13000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void JustDied(Unit* killer)
        {
            std::list<Creature*> fear;
            me->GetCreatureListWithEntryInGrid(fear, NPC_APPARITION_OF_FEAR, 100.0f);
            std::list<Creature*> terror;
            me->GetCreatureListWithEntryInGrid(fear, NPC_APPARITION_OF_TERROR, 100.0f);

            bool done = true;
            for (auto itr : fear)
            {
                if (itr->isAlive())
                {
                    done = false;
                    break;
                }
            }

            for (auto itr : terror)
            {
                if (itr->isAlive())
                {
                    done = false;
                    break;
                }
            }

            if (done && pInstance)
                pInstance->SetData(INTRO_DONE, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
            {
                if (corruptionBoltTimer)
                {
                    if (corruptionBoltTimer <= diff)
                    {
                        corruptionBoltTimer = 3000;

                        if (pInstance)
                        {
                            Creature* asani = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_ANCIENT_ASANI));
                            Creature* regail = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_ANCIENT_REGAIL));
                            Creature* kaolan = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_PROTECTOR_KAOLAN));
                            std::vector<ObjectGuid> targets;

                            if (asani)
                                targets.push_back(asani->GetGUID());
                            if (regail)
                                targets.push_back(regail->GetGUID());
                            if (kaolan)
                                targets.push_back(kaolan->GetGUID());

                            if (targets.empty())
                                return;

                            std::random_shuffle(targets.begin(), targets.end());
                            if (Unit* target = sObjectAccessor->FindUnit((*targets.begin())))
                                me->CastSpell(target, SPELL_CORRUPTION_BOLT, true);
                        }
                    }
                    else
                        corruptionBoltTimer -= diff;
                }
                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_OVERWHELMING_FEAR:
            {
                me->CastSpell(me, SPELL_OVERWHELMING_FEAR, false);
                events.ScheduleEvent(EVENT_OVERWHELMING_FEAR, 12000);
                break;
            }
            case EVENT_NIGHT_TERRORS_MISSILE:
            {
                me->CastSpell(me, SPELL_NIGHT_TERRORS_MISSILE, false);
                events.ScheduleEvent(EVENT_NIGHT_TERRORS_MISSILE, 20000);
                break;
            }
            default:
                break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_apparition_of_fearAI(creature);
    }
};

// Apparition of Terror - 66100
class npc_apparition_of_terror : public CreatureScript
{
public:
    npc_apparition_of_terror() : CreatureScript("npc_apparition_of_terror") { }

    struct npc_apparition_of_terrorAI : public ScriptedAI
    {
        npc_apparition_of_terrorAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }
        SummonList summons;
        InstanceScript* pInstance;
        EventMap events;
        uint32 corruptionBoltTimer;

        void Reset()
        {
            me->CastSpell(me, SPELL_CHANNELING_CORRUPTION, true);

            events.Reset();

            corruptionBoltTimer = 3000;

            summons.DespawnAll();

            if (pInstance)
                pInstance->SetData(INTRO_DONE, 0);
        }

        void EnterCombat(Unit* attacker)
        {
            // Schedule Combat event here
            events.Reset();
            events.ScheduleEvent(EVENT_GRIP_OF_FEAR, 5000);
            events.ScheduleEvent(EVENT_UNLEASHED_TERROR, 9000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void JustDied(Unit* killer)
        {
            std::list<Creature*> fear;
            me->GetCreatureListWithEntryInGrid(fear, NPC_APPARITION_OF_FEAR, 100.0f);
            std::list<Creature*> terror;
            me->GetCreatureListWithEntryInGrid(fear, NPC_APPARITION_OF_TERROR, 100.0f);

            bool done = true;
            for (auto itr : fear)
            {
                if (itr->isAlive())
                {
                    done = false;
                    break;
                }
            }

            for (auto itr : terror)
            {
                if (itr->isAlive())
                {
                    done = false;
                    break;
                }
            }

            if (done && pInstance)
                pInstance->SetData(INTRO_DONE, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
            {
                if (corruptionBoltTimer)
                {
                    if (corruptionBoltTimer <= diff)
                    {
                        corruptionBoltTimer = 3000;

                        if (pInstance)
                        {
                            Creature* asani = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_ANCIENT_ASANI));
                            Creature* regail = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_ANCIENT_REGAIL));
                            Creature* kaolan = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_PROTECTOR_KAOLAN));
                            std::vector<ObjectGuid> targets;

                            if (asani)
                                targets.push_back(asani->GetGUID());
                            if (regail)
                                targets.push_back(regail->GetGUID());
                            if (kaolan)
                                targets.push_back(kaolan->GetGUID());

                            if (targets.empty())
                                return;

                            std::random_shuffle(targets.begin(), targets.end());
                            if (Unit* target = sObjectAccessor->FindUnit((*targets.begin())))
                                me->CastSpell(target, SPELL_CORRUPTION_BOLT, true);
                        }
                    }
                    else
                        corruptionBoltTimer -= diff;
                }
                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_GRIP_OF_FEAR:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SPELL_GRIP_OF_FEAR, false);
                events.ScheduleEvent(EVENT_GRIP_OF_FEAR, 12000);
                break;
            }
            case EVENT_UNLEASHED_TERROR:
            {
                me->CastSpell(me, SPELL_UNLEASHED_TERROR, false);
                events.ScheduleEvent(EVENT_UNLEASHED_TERROR, 18000);
                break;
            }
            default:
                break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_apparition_of_terrorAI(creature);
    }
};

// Night Terrors - 64390
class mob_night_terrors : public CreatureScript
{
public:
    mob_night_terrors() : CreatureScript("mob_night_terrors") { }

    struct mob_night_terrorsAI : public ScriptedAI
    {
        mob_night_terrorsAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset()
        {
            me->CastSpell(me, SPELL_NIGHT_TERRORS_PERIODIC, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void UpdateAI(const uint32 diff) { }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_night_terrorsAI(creature);
    }
};

// Night Terror (summon) - 64393
class mob_night_terror_summon : public CreatureScript
{
public:
    mob_night_terror_summon() : CreatureScript("mob_night_terror_summon") { }

    struct mob_night_terror_summonAI : public ScriptedAI
    {
        mob_night_terror_summonAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
            events.ScheduleEvent(EVENT_ENVELOPING_DARKNESS, 2000);

            me->SetReactState(REACT_AGGRESSIVE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_NEAREST))
                    AttackStart(target);

                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_ENVELOPING_DARKNESS:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SPELL_ENVELOPING_DARKNESS, false);
                events.ScheduleEvent(EVENT_ENVELOPING_DARKNESS, 8000);
                break;
            }
            default:
                break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_night_terror_summonAI(creature);
    }
};

// Night Terrors (missile) - 125736
class spell_night_terrors_missile : public SpellScriptLoader
{
public:
    spell_night_terrors_missile() : SpellScriptLoader("spell_night_terrors_missile") { }

    class spell_night_terrors_missile_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_night_terrors_missile_SpellScript);

        void SummonNightTerrors(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(EFFECT_0);

            if (!GetExplTargetDest())
                return;

            GetCaster()->SummonCreature(NPC_NIGHT_TERRORS, GetExplTargetDest()->GetPositionX(), GetExplTargetDest()->GetPositionY(),
                GetExplTargetDest()->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 15000);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_night_terrors_missile_SpellScript::SummonNightTerrors, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_night_terrors_missile_SpellScript();
    }
};

// Night Terrors (periodic) - 125745
class spell_night_terrors_periodic : public SpellScriptLoader
{
public:
    spell_night_terrors_periodic() : SpellScriptLoader("spell_night_terrors_periodic") { }

    class spell_night_terrors_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_night_terrors_periodic_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                Position pos;
                caster->GetPosition(&pos);
                caster->SummonCreature(NPC_NIGHT_TERROR_SUMMON, pos);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_night_terrors_periodic_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_night_terrors_periodic_AuraScript();
    }
};

enum QuestsLegendaryVendor
{
    // 1st
    QuestFearItself = 31482,

    // 2nd 
    QuestTheSoulOfTheHorde = 32432,
    QuestCallThePackMaster = 32390,

    // 3d 
    QuestTheCrownOfHeaven = 32595,

    // 4th
    QuestCloakOfVirtue = 32861,

    // 5th
    QuestPandarenLegend = 33104,
};
enum ItemsLegendaryVendor
{

};

class creature_blacktalon_quartermaster : public CreatureScript
{
public:
    creature_blacktalon_quartermaster() : CreatureScript("creature_blacktalon_quartermaster") { }

    bool OnGossipHello(Player * player, Creature * Creature)
    {
        if (player->GetQuestRewardStatus(QuestFearItself))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Sha Touched Weapon Gems", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if (player->GetQuestRewardStatus(QuestTheCrownOfHeaven))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Sha Touched Head Gems", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        if (player->GetQuestRewardStatus(QuestTheSoulOfTheHorde) || player->GetQuestRewardStatus(QuestCallThePackMaster))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Eye of the Black Prince [2500 gold coins]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

        if (player->GetQuestRewardStatus(QuestCloakOfVirtue))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Cloaks", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        if (player->GetQuestRewardStatus(QuestPandarenLegend))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Legenday enchant [1000 gold coins]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);


        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "No thank you..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);

        player->SEND_GOSSIP_MENU(1, Creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player * player, Creature * Creature, uint32 sender, uint32 actions)
    {
        player->PlayerTalkClass->ClearMenus();

        if (sender == GOSSIP_SENDER_MAIN)
        {
            uint32 EntriesShaTouchedWeapon[3] = { 89873, 89882, 89881 }; //dread  horror terror
            uint32 EntriesShaTouchedHead[4] = { 95344, 95345, 95346, 95347 }; // indomitable, courageous, capactive, sinister
            uint32 EntriesCloaks[6] = { 98146, 98147, 98148, 98149, 98150, 98335 }; // oxhorn, tigerclaw, tigerfang, cranewing, jadefire, oxhoof

            switch (actions)
            {
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(1000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (1000 * GOLD));
                    player->DestroyItemCount(102457, 3, true);


                    player->AddItem(102457, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;

            case GOSSIP_ACTION_INFO_DEF + 5:
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(2500 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (2500 * GOLD));
                    player->DestroyItemCount(93403, 3, true);


                    player->AddItem(93403, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;

            case GOSSIP_ACTION_INFO_DEF + 1: // Sha Touched Weapon gems
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Crystalized Dread [10k gold coins]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 51);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Crystalized Horror [10k gold coins]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 52);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Crystalized Terror [10k gold coins]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 53);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Exit..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);

                player->SEND_GOSSIP_MENU(1, Creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 51: // Dread
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(15000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (15000 * GOLD));

                    for (int i = 0; i < 3; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedWeapon[i], 3, true);
                    }

                    player->AddItem(89873, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");


                break;
            case GOSSIP_ACTION_INFO_DEF + 52: // Horror
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(15000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (15000 * GOLD));

                    for (int i = 0; i < 3; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedWeapon[i], 3, true);
                    }

                    player->AddItem(89882, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;
            case GOSSIP_ACTION_INFO_DEF + 53: // Terror
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(15000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (15000 * GOLD));

                    for (int i = 0; i < 3; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedWeapon[i], 3, true);
                    }

                    player->AddItem(89881, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;

            case GOSSIP_ACTION_INFO_DEF + 2: // Sha Touched Weapon gems
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Indomitable Primal Diamond", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 20);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Courageous Primal Diamond", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Capacitive Primal Diamond", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Sinister Primal Diamond", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Exit..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);

                player->SEND_GOSSIP_MENU(1, Creature->GetGUID());
                break;

                /*
                uint32 EntriesShaTouchedHead[4]   = { 95344, 95345, 95346, 95347 }; // indomitable, courageous, capactive, sinister
                */

            case GOSSIP_ACTION_INFO_DEF + 20: // Indomitable
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(20000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (20000 * GOLD));

                    for (int i = 0; i < 4; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(95344, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");


                break;
            case GOSSIP_ACTION_INFO_DEF + 21: // Courageous
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(20000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (20000 * GOLD));

                    for (int i = 0; i < 3; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(95345, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;
            case GOSSIP_ACTION_INFO_DEF + 22: // Capacitive
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(20000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (20000 * GOLD));

                    for (int i = 0; i < 3; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(95346, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;
            case GOSSIP_ACTION_INFO_DEF + 23: // Sinister
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(20000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (20000 * GOLD));

                    for (int i = 0; i < 3; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(95347, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");

                break;

                // uint32 EntriesCloaks[6] = { 98146, 98147, 98148, 98149, 98150, 98335 }; // oxhorn, tigerclaw, tigerfang, cranewing, jadefire, oxhoof
            case GOSSIP_ACTION_INFO_DEF + 3: // Cloaks
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Oxhorn Bladebreaker", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 31);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tigerclaw Cape", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 32);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tigerfang Wrap", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 33);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Cranewing Cloak", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 34);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Jadefire Drape", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 35);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Oxhoof Greatcloak", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 36);

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Exit..", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);

                player->SEND_GOSSIP_MENU(1, Creature->GetGUID());
                break;

            case GOSSIP_ACTION_INFO_DEF + 31: // Oxhorn Bladebreaker
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(25000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (25000 * GOLD));

                    for (int i = 0; i < 6; i++)
                    {
                        player->DestroyItemCount(EntriesCloaks[i], 3, true);
                    }

                    player->AddItem(98146, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");
                break;

            case GOSSIP_ACTION_INFO_DEF + 32: // Tigerclaw Cape
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(25000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (25000 * GOLD));

                    for (int i = 0; i < 6; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(98147, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");
                break;

            case GOSSIP_ACTION_INFO_DEF + 33: // Tigerfang Wrap
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(25000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (25000 * GOLD));

                    for (int i = 0; i < 6; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(98148, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");
                break;

            case GOSSIP_ACTION_INFO_DEF + 34: // Cranewing Cloak
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(25000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (25000 * GOLD));

                    for (int i = 0; i < 6; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(98149, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");
                break;

            case GOSSIP_ACTION_INFO_DEF + 35: // Jadefire Drape
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(25000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (25000 * GOLD));

                    for (int i = 0; i < 6; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(98150, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");
                break;

            case GOSSIP_ACTION_INFO_DEF + 36: // Oxhoof Greatcloak
                player->CLOSE_GOSSIP_MENU();

                if (player->HasEnoughMoney(uint64(25000 * GOLD)))
                {
                    player->SetMoney(player->GetMoney() - (25000 * GOLD));

                    for (int i = 0; i < 6; i++)
                    {
                        player->DestroyItemCount(EntriesShaTouchedHead[i], 3, true);
                    }

                    player->AddItem(98335, 1);
                }
                else
                    player->GetSession()->SendNotification("You don't have enough gold, Friend.");
                break;

            case GOSSIP_ACTION_INFO_DEF + 60: // Exit
                player->CLOSE_GOSSIP_MENU();
                break;
            }
        }
        return true;
    }
};

void AddSC_terrace_of_endless_spring()
{
    new npc_springtender_ashani();
    new npc_apparition_of_fear();
    new npc_apparition_of_terror();
    new mob_night_terrors();
    new mob_night_terror_summon();
    new spell_night_terrors_missile();
    new spell_night_terrors_periodic();
    new creature_blacktalon_quartermaster();
}