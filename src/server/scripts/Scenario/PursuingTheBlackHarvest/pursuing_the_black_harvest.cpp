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

#include "MiscPackets.h"
#include "pursuing_the_black_harvest.h"

Position const atPos[]
{
    {703.721f, 574.901f, 112.628f} //< 8696
};

Position const akamaWP[]
{
    {721.4444f, 355.3133f, 125.3953f}, //<  +0
    {734.1794f, 343.3147f, 125.4458f}, //<  +2
    {749.3447f, 321.5173f, 125.4249f}, //<  +3
    {763.5404f, 314.8681f, 125.3804f}, //<  +3
    {792.8096f, 306.7504f, 113.0866f}, //<  +4
    {802.1400f, 278.0078f, 112.9999f}, //<  +4
    {804.4133f, 246.3474f, 113.0019f}, //<  +5
    {804.2388f, 128.3550f, 112.5224f}, //<  +13
    {792.6276f, 99.26649f, 113.0114f}, //<  +17
    {757.0466f, 65.21021f, 112.9879f}, //<  +6
    {754.4785f, 64.58849f, 112.9997f}, //<  +5
    {694.0858f, 67.66933f, 112.8861f}, //<  +3
    //< s 6 / 7 points
    {777.0364f, 71.70834f, 112.8220f}, //<  +0
    {778.7274f, 92.57291f, 112.7408f}, //<  +5
    {772.1349f, 110.3631f, 112.7561f}, //<  +2
    {753.3282f, 148.8767f, 112.9570f},
};

Position const soulsPositions[]
{
    {449.8871f, 198.7309f, 95.17565f},
    {541.7239f, 156.0469f, 95.00831f},
    {445.8160f, 136.8576f, 94.91354f},
    {457.3507f, 122.5260f, 94.83651f},
    {458.5434f, 204.1754f, 95.42035f},
    {469.0729f, 215.0104f, 94.84925f},
    {531.6129f, 218.2778f, 94.89791f},
    {546.2726f, 200.2431f, 94.89764f},
    {543.2257f, 135.7830f, 95.17218f},
};

class npc_akama : public CreatureScript
{
public:
    npc_akama() : CreatureScript("npc_akama") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            player->ADD_GOSSIP_ITEM(1, "King Rynn is looking for members of the Grim Harvest Council who were recently here....", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        return false;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                if (IsNextStageAllowed(creature->GetInstanceScript(), STAGE_3))
                    player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 34543, 1); //< set stage 3
                player->ADD_GOSSIP_ITEM(1, "Akama, show me the way!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                creature->AI()->DoAction(ACTION_2);
                break;
            default:
                break;
        }
        return true;
    }

    struct npc_akamaAI : ScriptedAI
    {
        npc_akamaAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
            timer = 0;
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 2 * IN_MILLISECONDS, 1);
                    break;
                case ACTION_2:
                    events.RescheduleEvent(EVENT_5, 3 * IN_MILLISECONDS, 1);
                    break;
                default:
                    break;
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER && (me->GetDistance2d(who) < 10.0f) && instance->GetData(DATA_ALLOWED_STAGE) == STAGE_6)
                events.RescheduleEvent(EVENT_21, 3 * IN_MILLISECONDS);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (pointId)
            {
                case EVENT_22:
                    me->GetMotionMaster()->MovePoint(EVENT_23, akamaWP[14]);
                    Talk(7);
                    break;
                case EVENT_23:
                    events.RescheduleEvent(EVENT_28, 2 * IN_MILLISECONDS, 1);
                    break;
                case EVENT_24:
                    events.RescheduleEvent(EVENT_25, 4 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_2, 3 * IN_MILLISECONDS, 1);
                        if (Player* plr = me->FindNearestPlayer(50.0f))
                            me->AddAura(SPELL_SAP, plr);
                        me->SetVisible(true);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_3, 3 * IN_MILLISECONDS, 1);
                        me->RemoveAura(SPELL_STEALTH);
                        if (Player* plr = me->FindNearestPlayer(50.0f))
                        {
                            Talk(8);
                            plr->CastSpell(plr, SPELL_UPDATE_PLAYER_PHASE_AURAS);
                            plr->AddAura(SPELL_TRUSTED_BY_THE_ASHTONGUE, plr);
                            plr->AddAura(SPELL_INVISIBILITY_DETECTION, plr);
                        }
                        break;
                    case EVENT_3:
                        events.RescheduleEvent(EVENT_4, 3 * IN_MILLISECONDS, 1);
                        Talk(0);
                        break;
                    case EVENT_4:
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        events.CancelEventGroup(1);
                        break;
                    case EVENT_5:
                        events.RescheduleEvent(EVENT_6, 4 * IN_MILLISECONDS);
                        Talk(1);
                        timer = 5;
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case EVENT_6:
                        events.RescheduleEvent(EVENT_7, timer += 0 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_8, timer += 2 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_9, timer += 3 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_10, timer += 3 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_11, timer += 4 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_12, timer += 4 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_13, timer += 5 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_14, timer += 13 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_15, timer += 17 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_16, timer += 6 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_17, timer += 5 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_18, timer += 3 * IN_MILLISECONDS);
                        break;
                    case EVENT_7:
                        Talk(2);
                        me->GetMotionMaster()->MovePoint(EVENT_7, akamaWP[0]);
                        break;
                    case EVENT_8:
                        me->GetMotionMaster()->MovePoint(EVENT_8, akamaWP[1]);
                        break;
                    case EVENT_9:
                        me->GetMotionMaster()->MovePoint(EVENT_9, akamaWP[2]);
                        break;
                    case EVENT_10:
                        me->GetMotionMaster()->MovePoint(EVENT_10, akamaWP[3]);
                        break;
                    case EVENT_11:
                        me->GetMotionMaster()->MovePoint(EVENT_11, akamaWP[4]);
                        break;
                    case EVENT_12:
                        me->GetMotionMaster()->MovePoint(EVENT_12, akamaWP[5]);
                        break;
                    case EVENT_13:
                        Talk(3);
                        me->GetMotionMaster()->MovePoint(EVENT_13, akamaWP[6]);
                        break;
                    case EVENT_14:
                        me->GetMotionMaster()->MovePoint(EVENT_14, akamaWP[7]);
                        break;
                    case EVENT_15:
                        Talk(4);
                        me->GetMotionMaster()->MovePoint(EVENT_15, akamaWP[8]);
                        break;
                    case EVENT_16:
                        me->GetMotionMaster()->MovePoint(EVENT_16, akamaWP[9]);
                        break;
                    case EVENT_17:
                        me->GetMotionMaster()->MovePoint(EVENT_17, akamaWP[10]);
                        break;
                    case EVENT_18:
                        events.RescheduleEvent(EVENT_19, 11 * IN_MILLISECONDS);
                        me->GetMotionMaster()->MovePoint(EVENT_18, akamaWP[11]);
                        break;
                    case EVENT_19:
                        events.RescheduleEvent(EVENT_20, 5 * IN_MILLISECONDS);
                        Talk(5);
                        break;
                    case EVENT_20:
                        Talk(6);
                        break;
                    case EVENT_21:
                        events.RescheduleEvent(EVENT_22, 5 * IN_MILLISECONDS);
                        me->GetMotionMaster()->MovePoint(EVENT_21, akamaWP[12]);
                        me->SetOrientation(3.266473f);
                        break;
                    case EVENT_22:
                        me->GetMotionMaster()->MovePoint(EVENT_22, akamaWP[13]);
                        break;
                    case EVENT_28:
                        me->GetMotionMaster()->MovePoint(EVENT_24, akamaWP[15]);
                        break;
                    case EVENT_25:
                        events.RescheduleEvent(EVENT_26, 3 * IN_MILLISECONDS);
                        if (Creature* imp = me->FindNearestCreature(NPC_FEL_IMP, 150.0f))
                            imp->AI()->Talk(0);
                        break;
                    case EVENT_26:
                        if (Creature* imp = me->FindNearestCreature(NPC_FEL_IMP, 150.0f))
                            imp->AI()->Talk(1);
                        me->DespawnOrUnsummon(3 * IN_MILLISECONDS);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        InstanceScript* instance;
        EventMap events;
        uint32 timer;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_akamaAI(creature);
    }
};

class npc_asthongue_primalist : public CreatureScript
{
public:
    npc_asthongue_primalist() : CreatureScript("npc_asthongue_primalist") { }

    struct npc_asthongue_primalistAI : ScriptedAI
    {
        npc_asthongue_primalistAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
            talk = false;
        }

        void EnterCombat(Unit* who) override
        {
            Talk(2);
            events.RescheduleEvent(EVENT_1, 3 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, 6 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_5, 3 * IN_MILLISECONDS);
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (who->HasAura(SPELL_TRUSTED_BY_THE_ASHTONGUE))
                return;

            if ((me->GetDistance(atPos[0]) < 30.0f) && !talk)
            {
                events.CancelEvent(EVENT_1);
                events.CancelEvent(EVENT_2);
                me->AttackStop();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                me->CombatStop();
                Talk(1);
                talk = true;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_4, 5 * IN_MILLISECONDS);
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0, true))
                            me->CastSpell(target, SPELL_NETTED);
                        break;
                    case EVENT_4:
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0, true, SPELL_NETTED))
                            me->CastSpell(target, SPELL_BLACKOUT);
                        break;
                    case EVENT_5:
                        events.RescheduleEvent(EVENT_5, 8 * IN_MILLISECONDS);
                        if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 0, true))
                            me->CastSpell(target, SPELL_MULTI_SHOOT);
                        break;
                    case EVENT_2:
                        Talk(0);
                        break;
                    case EVENT_3:
                        events.RescheduleEvent(EVENT_2, 25 * IN_MILLISECONDS);
                        DoCast(SPELL_BLACKOUT);
                        break;
                    default:
                        break;
                }
            }

            DoSpellAttackIfReady(SPELL_SHOOT);
        }

    private:
        InstanceScript* instance;
        EventMap events;
        bool talk;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_asthongue_primalistAI(creature);
    }
};

class npc_ashtongue_worker : public CreatureScript
{
public:
    npc_ashtongue_worker() : CreatureScript("npc_ashtongue_worker") { }

    struct npc_ashtongue_workerAI : ScriptedAI
    {
        npc_ashtongue_workerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            me->CallForHelp(50.0f);
        }

        void UpdateAI(uint32 diff) override
        {
            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashtongue_workerAI(creature);
    }
};

class npc_suffering_soul_fragment : public CreatureScript
{
public:
    npc_suffering_soul_fragment() : CreatureScript("npc_suffering_soul_fragment") { }

    struct npc_suffering_soul_fragmentAI : ScriptedAI
    {
        npc_suffering_soul_fragmentAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 10 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, 6 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->DespawnOrUnsummon(3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, 6 * IN_MILLISECONDS);
                        DoCast(SPELL_SHADOW_BOLT_2);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, 15 * IN_MILLISECONDS);
                        DoCast(SPELL_SOUL_BLAST);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_suffering_soul_fragmentAI(creature);
    }
};

class npc_hungering_soul_fragment2 : public CreatureScript
{
public:
    npc_hungering_soul_fragment2() : CreatureScript("npc_hungering_soul_fragment2") { }

    struct npc_hungering_soul_fragment2AI : ScriptedAI
    {
        npc_hungering_soul_fragment2AI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        { }

        void JustDied(Unit* /*killer*/) override
        {
            me->DespawnOrUnsummon(3 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            //if (uint32 eventId = events.ExecuteEvent())
            //{
            //    switch (eventId)
            //    {
            //        default:
            //            break;
            //    }
            //}

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hungering_soul_fragment2AI(creature);
    }
};

class npc_essence_of_order : public CreatureScript
{
public:
    npc_essence_of_order() : CreatureScript("npc_essence_of_order") { }

    struct npc_essence_of_orderAI : ScriptedAI
    {
        npc_essence_of_orderAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetVisible(false);

            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 2 * MINUTE * IN_MILLISECONDS + 40 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void Reset() override
        {
            events.Reset();

            summons.DespawnAll();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            instance->SetData(DATA_ESSENCE_OF_ORDER_EVENT, IN_PROGRESS);

            events.RescheduleEvent(EVENT_4, 5 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_5, 10 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_6, 15 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(2);
            summons.DespawnAll();

            instance->SetData(DATA_ESSENCE_OF_ORDER_EVENT, DONE);
        }

        void EnterEvadeMode() override
        {
            summons.DespawnAll();

            instance->SetData(DATA_ESSENCE_OF_ORDER_EVENT, TO_BE_DECIDED);

            me->Respawn(true);
            me->SetVisible(true);
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);

            switch (summon->GetEntry())
            {
                case NPC_LOST_SOULS:
                    if (Unit* target = summon->FindNearestPlayer(200.0f))
                    {
                        summon->AddThreat(target, 10.0f);
                        summon->Attack(target, true);
                        summon->GetMotionMaster()->MoveChase(target);
                    }
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        if (Player* plr = me->FindNearestPlayer(300.0f))
                            if (IsNextStageAllowed(instance, STAGE_5))
                                plr->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT, 34552, 1); //< set stage 5
                        events.RescheduleEvent(EVENT_2, 2 * IN_MILLISECONDS);
                        me->SetVisible(true);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_3, 2 * IN_MILLISECONDS);
                        Talk(0);
                        break;
                    case EVENT_3:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                        break;
                    case EVENT_4:
                        events.RescheduleEvent(EVENT_4, 15 * IN_MILLISECONDS);
                        events.DelayEvents(3 * IN_MILLISECONDS);
                        DoCast(SPELL_SPELLFLAME_DUMMY);
                        break;
                    case EVENT_5:
                        events.RescheduleEvent(EVENT_5, 20 * IN_MILLISECONDS);
                        events.DelayEvents(2 * IN_MILLISECONDS);
                        DoCast(SPELL_HELLFIRE);
                        break;
                    case EVENT_6:
                        Talk(1);
                        events.RescheduleEvent(EVENT_6, 25 * IN_MILLISECONDS);
                        for (uint8 i = 0; i < 8; i++)
                            me->SummonCreature(NPC_LOST_SOULS, soulsPositions[i]);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
        SummonList summons;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_essence_of_orderAI(creature);
    }
};

class npc_demonic_gateway_scen : public CreatureScript
{
public:
    npc_demonic_gateway_scen() : CreatureScript("npc_demonic_gateway_scen") { }

    struct npc_demonic_gateway_scenAI : ScriptedAI
    {
        npc_demonic_gateway_scenAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
            me->AddAura(SPELL_DEMONIC_GATEWAY, me);
            start = false;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (!start)
            {
                events.RescheduleEvent(EVENT_1, 3 * IN_MILLISECONDS);
                start = true;
            }

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        if (Creature* cre = me->SummonCreature(NPC_KANRETHAD_EBONLOCKE, 641.84f, 306.321f, 353.411f, 6.2351f))
                            cre->AI()->DoAction(ACTION_1);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        InstanceScript* instance;
        EventMap events;
        bool start;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_demonic_gateway_scenAI(creature);
    }
};

class npc_kanrethad_ebonlocke : public CreatureScript
{
public:
    npc_kanrethad_ebonlocke() : CreatureScript("npc_kanrethad_ebonlocke") { }

    struct npc_kanrethad_ebonlockeAI : ScriptedAI
    {
        npc_kanrethad_ebonlockeAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();

            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_TURN, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
        }

        void Reset() override
        {
            events.Reset();

            me->SetReactState(REACT_PASSIVE);
            completed = false;
            start = false;
        }

        void EnterCombat(Unit* /*who*/) override
        { }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 1 * IN_MILLISECONDS);
                    break;
                case ACTION_2:
                    events.RescheduleEvent(EVENT_26, 2 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        void EnterEvadeMode() override
        {
            instance->SetData(DATA_KANRETHAD, TO_BE_DECIDED);

            me->Respawn(true);
        }

        void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType dmgType) override
        {
            if (damage && HealthBelowPct(2) && !completed)
            {
                damage = 0;

                events.CancelEvent(EVENT_6);
                events.CancelEvent(EVENT_7);
                events.CancelEvent(EVENT_8);
                events.CancelEvent(EVENT_9);
                events.CancelEvent(EVENT_10);
                events.CancelEvent(EVENT_11);
                events.CancelEvent(EVENT_12);
                events.CancelEvent(EVENT_13);
                events.CancelEvent(EVENT_14);
                events.CancelEvent(EVENT_15);
                events.CancelEvent(EVENT_16);

                events.RescheduleEvent(EVENT_25, 5 * IN_MILLISECONDS);

                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SEED_OF_TERRIBLE_DESTRUCTION);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CURSE_OF_ULTIMATE_DOOM);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_EXCRUCIATING_AGONY);

                DoCast(SPELL_ANNIHILATE_DEMONS);
                completed = true;
                instance->SetData(DATA_KANRETHAD, DONE);
                me->SetReactState(REACT_PASSIVE);
                Talk(13);

                if (Player* plr = me->FindNearestPlayer(150.0f))
                    me->AddAura(SPELL_DEMONIC_GRASP, plr);
            }
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (pointId)
            {
                case EVENT_1:
                    instance->SetData(DATA_KANRETHAD, IN_PROGRESS);
                    Talk(0);
                    break;
                case EVENT_2:
                    events.RescheduleEvent(EVENT_6, 1 * MINUTE * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_7, 2 * MINUTE * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_8, 30 * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_11, 1 * MINUTE * IN_MILLISECONDS + 10 * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_13, 15 * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_14, 1 * MINUTE * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_15, 10 * IN_MILLISECONDS);
                    events.RescheduleEvent(EVENT_16, 25 * IN_MILLISECONDS);

                    me->StopMoving();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1);
                    me->SetReactState(REACT_AGGRESSIVE);
                    Talk(1);
                    if (Player* plr = me->FindNearestPlayer(150.0f))
                        me->AddAura(SPELL_FACE_PLAYER, plr);

                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_2, 3 * IN_MILLISECONDS);
                        me->GetMotionMaster()->MovePoint(EVENT_1, 660.0f, 305.5f, 354.1158f);
                        break;
                    case EVENT_2:
                        me->AddAura(SPELL_BURNING_EMBERS, me);
                        me->AddAura(SPELL_SOULSHARDS, me);
                        me->AddAura(SPELL_METAMORPHOSIS, me);
                        events.RescheduleEvent(EVENT_3, 5 * IN_MILLISECONDS);
                        me->GetMotionMaster()->MovePoint(EVENT_2, 662.0f, 315.0f, 354.0903f);
                        break;
                    case EVENT_3:
                        events.RescheduleEvent(EVENT_4, 10 * IN_MILLISECONDS);
                        Talk(2);
                        break;
                    case EVENT_4:
                        me->SetReactState(REACT_AGGRESSIVE);
                        events.RescheduleEvent(EVENT_5, 6 * IN_MILLISECONDS);
                        Talk(3);
                        break;
                    case EVENT_5:
                        Talk(4);
                        break;
                    case EVENT_6:
                        events.RescheduleEvent(EVENT_6, 1 * MINUTE * IN_MILLISECONDS);
                        DoCast(SPELL_CHAOS_BOLT);
                        Talk(9);
                        break;
                    case EVENT_7:
                        Talk(6);
                        break;
                    case EVENT_8:
                        events.RescheduleEvent(EVENT_9, 4 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_10, 15 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_8, 45 * IN_MILLISECONDS);
                        DoCast(SPELL_SEED_OF_TERRIBLE_DESTRUCTION);
                        Talk(7);
                        break;
                    case EVENT_9:
                        DoCast(SPELL_CURSE_OF_ULTIMATE_DOOM);
                        break;
                    case EVENT_10:
                        DoCast(SPELL_EXCRUCIATING_AGONY);
                        break;
                    case EVENT_11:
                        events.RescheduleEvent(EVENT_12, 5 * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_11, 1 * MINUTE * IN_MILLISECONDS + 40 * IN_MILLISECONDS);
                        Talk(10);
                        break;
                    case EVENT_12:
                        DoCast(SPELL_CATACLYSM);
                        Talk(11);
                        break;
                    case EVENT_13:
                        events.RescheduleEvent(EVENT_13, 10 * IN_MILLISECONDS);
                        switch (urand(1, 4))
                        {
                            case 1:
                                DoCast(SPELL_AURA_OF_OMNIPOTENCE);
                                break;
                            case 2:
                                DoCast(SPELL_BACKFIRE);
                                break;
                            case 3:
                                Talk(8);
                                DoCast(SPELL_RAID_OF_FIRE);
                                break;
                            case 4:
                                DoCast(SPELL_FEL_FLAME_BREATH);
                                break;
                            default:
                                break;
                        }
                        break;
                    case EVENT_16:
                        events.RescheduleEvent(EVENT_16, 5 * IN_MILLISECONDS);
                        DoCast(SPELL_SOULFIRE);
                        break;

                    case EVENT_14:
                        events.RescheduleEvent(EVENT_14, 4 * MINUTE * IN_MILLISECONDS);
                        events.RescheduleEvent(EVENT_17, 1 * MINUTE * IN_MILLISECONDS);
                        Talk(12);
                        if (Creature* target = me->FindNearestCreature(NPC_DEMONIC_GATEWAY, 100.0f))
                            me->CastSpell(target, SPELL_SUMMON_WILD_IMPS);
                        break;
                    case EVENT_17:
                        events.RescheduleEvent(EVENT_18, 1 * MINUTE * IN_MILLISECONDS);
                        if (Creature* target = me->FindNearestCreature(NPC_DEMONIC_GATEWAY, 100.0f))
                            me->CastSpell(target, SPELL_SUMMON_FELHUNTERS);
                        break;
                    case EVENT_18:
                        if (Creature* target = me->FindNearestCreature(NPC_DEMONIC_GATEWAY, 100.0f))
                            me->CastSpell(target, SPELL_SUMMON_DOOM_LORD);
                        break;
                    case EVENT_15:
                        if (Creature* target = me->FindNearestCreature(NPC_DEMONIC_GATEWAY, 100.0f))
                            me->CastSpell(target, SPELL_SUMMONING_PIT_LORD);
                        break;
                    case EVENT_25:
                        Talk(14);
                        break;
                    case EVENT_26:
                        events.RescheduleEvent(EVENT_27, 5 * IN_MILLISECONDS);
                        Talk(15);
                        break;
                    case EVENT_27:
                        me->setFaction(35);
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

                        if (Player* plr = me->FindNearestPlayer(150.0f))
                            plr->RemoveAura(SPELL_DEMONIC_GRASP);

                        Talk(16);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        InstanceScript* instance;
        EventMap events;
        bool completed;
        bool start;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_kanrethad_ebonlockeAI(creature);
    }
};

class npc_jubeka_shadowbreaker : public CreatureScript
{
public:
    npc_jubeka_shadowbreaker() : CreatureScript("npc_jubeka_shadowbreaker") { }

    struct npc_jubeka_shadowbreakerAI : ScriptedAI
    {
        npc_jubeka_shadowbreakerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
        }

        void DoAction(int32 const action) override
        {
            switch (action)
            {
                case ACTION_1:
                    me->SetVisible(true);
                    events.RescheduleEvent(EVENT_2, 4 * IN_MILLISECONDS);
                    Talk(0);
                    me->GetMotionMaster()->MovePoint(EVENT_1, 655.026f, 307.8343f, 354.2209f);
                    break;
                default:
                    break;
            }
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (pointId)
            {
                case EVENT_1:
                    if (Creature* target = me->FindNearestCreature(NPC_KANRETHAD_EBONLOCKE, 100.0f))
                    {
                        me->AddAura(SPELL_ETERNAL_BANISHMENT, target);
                        target->AI()->DoAction(ACTION_2);
                    }

                    events.RescheduleEvent(EVENT_1, 3 * IN_MILLISECONDS);
                    Talk(1);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, 5 * IN_MILLISECONDS);
                        Talk(2);
                        break;
                    case EVENT_2:
                        Talk(3);
                        if (Player* plr = me->FindNearestPlayer(200.0f))
                            plr->CastSpell(plr, SPELL_PURGE_XERRATH);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jubeka_shadowbreakerAI(creature);
    }
};

class npc_wild_imp_scenario : public CreatureScript
{
public:
    npc_wild_imp_scenario() : CreatureScript("npc_wild_imp_scenario") { }

    struct npc_wild_imp_scenarioAI : ScriptedAI
    {
        npc_wild_imp_scenarioAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            me->CallForHelp(50.0f);
        }

        void Reset() override
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoSpellAttackIfReady(SPELL_FEL_FIREBOLT);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wild_imp_scenarioAI(creature);
    }
};

class npc_doom_lord_scenario : public CreatureScript
{
public:
    npc_doom_lord_scenario() : CreatureScript("npc_doom_lord_scenario") { }

    struct npc_doom_lord_scenarioAI : ScriptedAI
    {
        npc_doom_lord_scenarioAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void Reset() override
        { }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoSpellAttackIfReady(SPELL_DOOM_BOLT2);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_doom_lord_scenarioAI(creature);
    }
};

class npc_felhunter_scenario : public CreatureScript
{
public:
    npc_felhunter_scenario() : CreatureScript("npc_felhunter_scenario") { }

    struct npc_felhunter_scenarioAI : ScriptedAI
    {
        npc_felhunter_scenarioAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void Reset() override
        { 
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 5 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, 15 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 /*diff*/) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, 15 * IN_MILLISECONDS);
                        DoCast(SPELL_DEVOUR_ENSLAVEMENT);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, 25 * IN_MILLISECONDS);
                        DoCast(SPELL_DEVOUR_MAGIC);
                        break;
                    default:
                        break;
                }
            }

            DoSpellAttackIfReady(SPELL_SHADOW_BITE);
        }
    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_felhunter_scenarioAI(creature);
    }
};

class npc_unbound_night_lord : public CreatureScript
{
public:
    npc_unbound_night_lord() : CreatureScript("npc_unbound_night_lord") { }

    struct npc_unbound_night_lordAI : ScriptedAI
    {
        npc_unbound_night_lordAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 15 * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_2, 7 * IN_MILLISECONDS);

        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        events.RescheduleEvent(EVENT_1, 25 * IN_MILLISECONDS);
                        me->AddAura(SPELL_SHADOW_INFERNO2, me);
                        break;
                    case EVENT_2:
                        events.RescheduleEvent(EVENT_2, 30 * IN_MILLISECONDS);
                        DoCast(SPELL_SUMMON_SHADOWFIENDS);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_unbound_night_lordAI(creature);
    }
};

class at_pursuing_the_black_harvest_main : public AreaTriggerScript
{
public:
    at_pursuing_the_black_harvest_main() : AreaTriggerScript("at_pursuing_the_black_harvest_main")
    { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* at, bool enter) override
    {
        instance = player->GetInstanceScript();
        if (enter && instance)
        {
            switch (at->ID)
            {
                case 8696:
                    if (IsNextStageAllowed(instance, STAGE_2))
                        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 34539, 1);  //< set stage 2
                    return true;
                case 8699:
                    if (IsNextStageAllowed(instance, STAGE_4))
                        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT, 34545, 1);  //< set stage 4
                    return true;
                case 8698:
                    if (instance->GetData(DATA_ESSENCE_OF_ORDER_EVENT) != DONE || instance->GetData(DATA_ESSENCE_OF_ORDER_EVENT) != TO_BE_DECIDED)
                    {
                        if (instance->GetData(DATA_SCENE_EVENT) == DONE)
                            return false;

                        instance->SetData(DATA_SCENE_EVENT, DONE);
                        player->CastSpell(player, SPELL_MEMORY_OF_THE_RELIQUARY);
                        player->CastSpell(player, SPELL_SPAWN_THE_RELIQUARY);
                        player->AddAura(SPELL_INVISIBILITY_DETECTION, player);
                        if (Creature* essence = player->FindNearestCreature(NPC_ESSENCE_OF_ORDER, 200.0f))
                            essence->AI()->DoAction(ACTION_1);
                        return true;
                    }
                case 8701:
                    if (IsNextStageAllowed(instance, STAGE_7))
                        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 34554, 1); //< set stage 7 
                    return true;
                case 8706:
                    if (instance->GetData(DATA_NOBEL_EVENT) == NOT_STARTED && instance->GetData(DATA_ESSENCE_OF_ORDER_EVENT) == DONE)
                    {
                        player->AddAura(SPELL_INTERRUPTED, player);
                        player->CastSpell(player, SPELL_SUMMON_FEL_IMP, true);
                        instance->SetData(DATA_NOBEL_EVENT, DONE);
                    }
                    return true;
                case 8702:
                    if (IsNextStageAllowed(instance, STAGE_7))
                    {
                        std::vector<WorldPackets::Misc::PhaseShiftDataPhase> phaseIds;
                        std::vector<uint16> terrainswaps;
                        std::vector<uint16> WorldMapAreaIds;
                        WorldMapAreaIds.push_back(992);
                        WorldMapAreaIds.push_back(683);
                        phaseIds.emplace_back(1982);
                        phaseIds.emplace_back(2387);
                        player->GetSession()->SendSetPhaseShift(phaseIds, terrainswaps, WorldMapAreaIds, {}, 16);
                        player->AddAura(SPELL_UPDATE_PHASE_SHIFT, player);
                        instance->HandleGameObject(instance->GetGuidData(DATA_SECOND_DOOR), true);
                        player->AddAura(SPELL_PLUNDER, player);
                        player->PlayerTalkClass->SendQuestQueryResponse(32340); //< i hope it's right way... SMSG_QUEST_GIVER_QUEST_DETAILS in sniffs
                        return true;
                    }
                case 8708:
                    if (IsNextStageAllowed(instance, STAGE_8))
                        player->TeleportTo(1112, 786.0955f, 304.3524f, 319.7598f, 0.0f);
                    return true;
                case 8908:
                    if (IsNextStageAllowed(instance, STAGE_LAST))
                        player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 34559, 1); //< set final stage
                    return true;
                default:
                    return false;
            }
        }

        return false;
    }

private:
    InstanceScript* instance;
};

class go_cospicuous_illidari_scroll : public GameObjectScript
{
public:
    go_cospicuous_illidari_scroll() : GameObjectScript("go_cospicuous_illidari_scroll") { }

    void OnLootStateChanged(GameObject* go, uint32 /*state*/, Unit* /*unit*/) override
    {
        if (Creature* akama = go->FindNearestCreature(NPC_AKAMA, 50.0f))
            akama->AI()->DoAction(ACTION_1);

        return;
    }
};

class go_treasure_chest : public GameObjectScript
{
public:
    go_treasure_chest() : GameObjectScript("go_treasure_chest") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (InstanceScript* instance = player->GetInstanceScript())
        {
            if (IsNextStageAllowed(instance, STAGE_8))
            {
                player->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, 34558, 1); //< set stage 8
                player->CastSpell(player, SPELL_APPRAISAL);
            }

            //ServerToClient: SMSG_PHASE_SHIFT_CHANGE (0x00D1) Length: 39 ConnIdx: 0 Time: 01/29/2015 09:05:50.000 Number: 20113
            //PhaseShiftFlags: 16
            //PhaseShiftCount: 1
            //PersonalGUID: Full: 0x0
            //[0] PhaseFlags: 0
            //[0] Id: 1982
            //PreloadMapIDsCount: 0
            //UiWorldMapAreaIDSwap: 0
            //VisibleMapIDsCount: 4
            //[0] VisibleMapID: 992 (992)
            //[1] VisibleMapID: 683 (683)

            return true;
        }

        return false;
    }
};

class spell_place_empowered_soulcore : public SpellScriptLoader
{
public:
    spell_place_empowered_soulcore() : SpellScriptLoader("spell_place_empowered_soulcore") { }

    class spell_place_empowered_soulcore_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_place_empowered_soulcore_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetTarget())
                return;

            if (Creature* summoner = GetTarget()->FindNearestCreature(NPC_DEMONIC_SOULWELL, 150.0f))
                summoner->SummonCreature(NPC_DEMONIC_GATEWAY, 641.84f, 306.321f, 353.411f, 6.2351f);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_place_empowered_soulcore_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_place_empowered_soulcore_AuraScript();
    }
};

class CreatureTargetFilter
{
public:
    bool operator()(WorldObject* target) const
    {
        if (Unit* unit = target->ToCreature())
            if (unit->GetEntry() != NPC_WILD_IMP || unit->GetEntry() != NPC_PIT_LORD)
                return false;

        return true;
    }
};

class spell_anihilate_demons : public SpellScriptLoader
{
public:
    spell_anihilate_demons() : SpellScriptLoader("spell_anihilate_demons") { }

    class spell_anihilate_demons_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_anihilate_demons_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            unitList.remove_if(CreatureTargetFilter());
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anihilate_demons_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anihilate_demons_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_anihilate_demons_SpellScript();
    }
};

class spell_drain_fel_enegry : public SpellScriptLoader
{
public:
    spell_drain_fel_enegry() : SpellScriptLoader("spell_drain_fel_enegry") { }

    class spell_drain_fel_enegry_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_drain_fel_enegry_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* target = GetCaster()->ToPlayer())
            {
                //if (Creature* veh = target->SummonCreature(70504, 660.1176f, 313.6469f, 355.0672f, 3.764802f))
                //    veh->EnterVehicle(target);

                //target->CastSpell(target, SPELL_FEL_ENERGY_DUMMY, true);

                target->CastSpell(target, SPELL_THE_CODEX_OF_XERRATH);
                target->CastSpell(target, SPELL_THE_CODEX_OF_XERRATH_2);

                if (Creature* caster = GetTarget()->ToCreature())
                    caster->DespawnOrUnsummon(3 * IN_MILLISECONDS);
            }
        }

        void Register() override
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_drain_fel_enegry_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_drain_fel_enegry_AuraScript();
    }
};

class spell_fel_enery : public SpellScriptLoader
{
public:
    spell_fel_enery() : SpellScriptLoader("spell_fel_enery") { }

    class spell_fel_enery_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_fel_enery_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* player = GetTarget()->ToPlayer())
                player->CastSpell(player, SPELL_FEL_ENERGY_DUMMY_2);
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Player* player = GetTarget()->ToPlayer())
                player->SendOnCancelExpectedVehicleRideAura();
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_fel_enery_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_fel_enery_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_fel_enery_AuraScript();
    }
};

class spell_appraisal : public SpellScriptLoader
{
public:
    spell_appraisal() : SpellScriptLoader("spell_appraisal") { }

    class spell_appraisal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_appraisal_SpellScript);

        void HandleEffect(uint32 item, uint32 spellID, Player* player)
        {
            uint32 itemCount = player->GetItemCount(item);
            for (uint8 i = 0; i < itemCount; i++)
                player->CastSpell(player, spellID);

            player->DestroyItemCount(item, itemCount, true);
        }

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            Player* player = GetCaster()->ToPlayer();
            if (!player)
                return;

            HandleEffect(92582, SPELL_CHEAP_COLOGNE, player);
            HandleEffect(92590, SPELL_RUBY_NEACKABLE, player);
            HandleEffect(92620, SPELL_ROPE_BINDINGS, player);
            HandleEffect(92612, SPELL_YARN, player);
            HandleEffect(92604, SPELL_GOLDEN_GOBLET, player);
            HandleEffect(92596, SPELL_RUBY_RING, player);
            HandleEffect(92588, SPELL_JADE_KITTEN, player);
            HandleEffect(92580, SPELL_FRAGRANT_PERFUME, player);
            HandleEffect(92618, SPELL_ORNATE_PORTRAIT, player);
            HandleEffect(92610, SPELL_GOLDER_PLATTER, player);
            HandleEffect(92602, SPELL_LARGE_PILE_OF_GOLD, player);
            HandleEffect(92594, SPELL_DIAMONG_RING, player);
            HandleEffect(92586, SPELL_SPARKLING_SAPPHIRE, player);
            HandleEffect(92624, SPELL_RUNEBLADE, player);
            HandleEffect(92616, SPELL_FRUIT_BOWL, player);
            HandleEffect(92608, SPELL_GOLDON_POTION, player);
            HandleEffect(92600, SPELL_SMALL_PILE_OF_GOLD, player);
            HandleEffect(92592, SPELL_SPELLSTONE_NECKABLE, player);
            HandleEffect(92584, SPELL_EXPENSIVE_RUBY, player);
            HandleEffect(92622, SPELL_ANCIENT_ORC_SHIELD, player);
            HandleEffect(92614, SPELL_FAMILY_JEWELS, player);
            HandleEffect(92606, SPELL_HIGH_ELF_STATUE, player);
            HandleEffect(92598, SPELL_GOLD_RING, player);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_appraisal_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_appraisal_SpellScript();
    }
};

class spell_spellflame_dummy : public SpellScriptLoader
{
public:
    spell_spellflame_dummy() : SpellScriptLoader("spell_spellflame_dummy") { }

    class spell_spellflame_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_spellflame_dummy_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            Position offset;
            caster->GetNearPosition(offset, step, 0);
            caster->GetPosition(&pos);
            vel = G3D::Vector2(offset.m_positionX - pos.m_positionX, offset.m_positionY - pos.m_positionY).direction();
        }

        void PeriodicTick(AuraEffect const* aurEff)
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;

            uint32 tick = aurEff->GetTickNumber();
            if (tick > 19)
            {
                caster->RemoveAura(SPELL_SPELLFLAME_DUMMY);
                return;
            }

            caster->CastSpell(pos.m_positionX + vel.x * step * (tick - 1), pos.m_positionY + vel.y * step * (tick - 1), pos.m_positionZ, SPELL_SPELLFLAME_TRIGGER, true);
        }

        void Register() override
        {
            OnEffectApply += AuraEffectApplyFn(spell_spellflame_dummy_AuraScript::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_DEFAULT);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_spellflame_dummy_AuraScript::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }

    private:
        const float step = 2.0f;
        G3D::Vector2 vel;
        Position pos;
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_spellflame_dummy_AuraScript();
    }
};

void AddSC_pursing_the_black_harvest()
{
    new npc_akama();
    new npc_asthongue_primalist();
    new npc_ashtongue_worker();
    new npc_suffering_soul_fragment();
    new npc_hungering_soul_fragment2();
    new npc_essence_of_order();
    new npc_demonic_gateway_scen();
    new npc_kanrethad_ebonlocke();
    new npc_jubeka_shadowbreaker();
    new npc_wild_imp_scenario();
    new npc_doom_lord_scenario();
    new npc_felhunter_scenario();
    new npc_unbound_night_lord();

    new at_pursuing_the_black_harvest_main();

    new go_cospicuous_illidari_scroll();
    new go_treasure_chest();

    new spell_place_empowered_soulcore();
    new spell_anihilate_demons();
    new spell_drain_fel_enegry();
    new spell_fel_enery();
    new spell_appraisal();
    new spell_spellflame_dummy();
}
