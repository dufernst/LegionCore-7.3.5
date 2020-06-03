/*
    The Seat of the Triumvirate: Zuraal the Ascended [heroic & mythic]

*/

#include "the_seat_of_the_triumvirate.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"

enum Says
{
    SAY_AGGRO               = 0,
    SAY_VOID_REALM          = 1,
    SAY_DEATH               = 2,
    SAY_KILL                = 3,

    //alleria
    SAY_1                   = 0,
    SAY_2                   = 1,
    SAY_3                   = 2,
};

enum Spells
{
    //Zuraal
    SPELL_NULL_PALM         = 246134,
    SPELL_DECIMATE          = 244579,
    SPELL_UMBRA_SHIFT       = 244433,
    SPELL_FIXATE_CHECK      = 244646,
    SPELL_FIXATE            = 244653,
    SPELL_FIXATE_DUMMY      = 244657,
    SPELL_MADDENED_FRENZY   = 247038,
    SPELL_VOID_REALM        = 244061,
    SPELL_VOID_REALM_1      = 244620,
    SPELL_VOID_REALM_2      = 244624,
    SPELL_VOID_TEAR         = 244621,

    //Coalesced Void
    SPELL_C_V_SPAWN         = 244603,
    SPELL_UMBRAL_EJECTION   = 244731,
    SPELL_DARK_EXPULSION    = 244599,

    //Shadowguard Subjugator
    SPELL_VOID_CONTAINMENT  = 246922,
    SPELL_NEGATING_BRAND    = 246697,
    SPELL_SUPPRESSION_FIELD = 246677,
    //alleria
    SPELL_DESTROY_WALL      = 249952,
    SPELL_CONVERSATION_LURA = 248451,
    SPELL_OPEN_DOOR_VISUAL  = 248460,

};

enum eEvents
{
    EVENT_NULL_PALM         = 1,
    EVENT_DECIMATE          = 2,
    EVENT_COALESCED_VOID    = 3,
    EVENT_UMBRA_SHIFT       = 4,
    EVENT_FIXATE            = 5,
};

enum eEventsTrash
{
    EVENT_NB                = 1,
    EVENT_FIELD             = 2,
};

Position const zurHome = { 5510.40f, 10800.15f, 17.16f, 1.27f };

Position const addPos[4] =
{
    { 5522.39f, 10810.70f, 17.26f, 3.85f },
    { 5512.43f, 10783.59f, 17.24f, 1.69f },
    { 5507.47f, 10815.09f, 17.24f, 4.90f },
    { 5497.77f, 10788.79f, 17.24f, 0.74f },
};

Position const voidPos[8] =
{
    { 5483.56f, 10765.00f, 18.26f, 1.21f },
    { 5494.43f, 10762.32f, 17.16f, 1.39f },
    { 5502.32f, 10760.47f, 17.20f, 1.34f },
    { 5519.31f, 10756.85f, 17.83f, 1.41f },
    { 5537.28f, 10821.35f, 17.16f, 4.52f },
    { 5522.23f, 10824.61f, 17.16f, 4.53f },
    { 5510.59f, 10827.11f, 17.16f, 4.54f },
    { 5497.86f, 10829.84f, 17.24f, 4.51f },
};

Position const assaultStart[4] =
{
    { 5570.56f, 10786.76f, 19.15f, 3.02f },
    { 5569.80f, 10783.51f, 19.38f, 3.02f },
    { 5606.06f, 10778.18f, 23.48f, 3.02f },
    { 5676.65f, 10760.53f, 23.49f, 3.02f },
};

Position const openDoor[2] =
{
    { 6096.06f, 10350.91f, 19.29f, 3.99f }, //alleria
    { 6105.29f, 10342.85f, 19.29f, 3.90f }, //walker
};

//122313
class boss_zuraal_the_ascended : public CreatureScript
{
public:
    boss_zuraal_the_ascended() : CreatureScript("boss_zuraal_the_ascended") {}

    struct boss_zuraal_the_ascendedAI : public BossAI
    {
        boss_zuraal_the_ascendedAI(Creature* creature) : BossAI(creature, DATA_ZURAAL)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisableGravity(true);
            AddsDiedCount = 0;
            phase = 0;
            if (me->isAlive())
                SummonAdds();
        }

        uint8 AddsDiedCount;
        uint8 phase = 0;

        void Reset() override 
        {
            _Reset();
            phase = 0;
            me->SetReactState(REACT_DEFENSIVE);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
            me->RemoveAurasDueToSpell(SPELL_FIXATE_CHECK);
            AbberationsReset();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOID_REALM);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOID_REALM_1);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOID_REALM_2);
        } 

        void EnterCombat(Unit* /*who*/) override 
        {
            _EnterCombat();
            Talk(SAY_AGGRO);
            events.RescheduleEvent(EVENT_NULL_PALM, 11000);
            events.RescheduleEvent(EVENT_DECIMATE, 18000);
            events.RescheduleEvent(EVENT_COALESCED_VOID, 20000);
            events.RescheduleEvent(EVENT_UMBRA_SHIFT, 41000);
        }

        void JustDied(Unit* /*killer*/) override 
        {
            _JustDied();
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOID_REALM);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOID_REALM_1);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOID_REALM_2);
            me->SummonCreature(NPC_ALLERIA_E1, assaultStart[0]);
            me->SummonCreature(NPC_LOCUSWALKER_E1, assaultStart[1]);
            instance->UpdatePhasing();
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->IsPlayer())
                Talk(SAY_KILL);
        }

        void SummonAdds() 
        {
            for (uint8 i = 0; i < 4; ++i)
                if (auto iadds = me->SummonCreature(NPC_SHADOWGUARD, addPos[i]))
                {
                    DoCast(me, 244087, true);
                    DoCast(me, 246913, true);
                }
        }

        void AbberationsReset()
        {
            std::list<Creature*> creaList;
            GetCreatureListWithEntryInGrid(creaList, me, NPC_DARK_ABBERATION, 100.0f);
            for (auto const& abberations : creaList)
                if (!creaList.empty())
                    abberations->Respawn();
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override 
        {
            switch (summon->GetEntry())
            {
            case NPC_SHADOWGUARD:
                AddsDiedCount++;
                break;
            }
            if (AddsDiedCount == 4 && !me->isInCombat())  
            {
                me->SetHomePosition(zurHome);
                me->SetDisableGravity(false);
                me->GetMotionMaster()->MovePoint(0, zurHome);
                me->RemoveAura(244087);
                me->RemoveAura(246913);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
                me->SetReactState(REACT_DEFENSIVE);
            }
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == 244621)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->getThreatManager().resetAllAggro();
            }
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_NULL_PALM)
            {
                Position pos;
                float angle = me->GetRelativeAngle(target);
                float _range = 0.0f;
                for (uint8 i = 0; i < 12; ++i)
                {
                    _range += 2;
                    me->GetNearPosition(pos, _range, angle + frand(-0.7f, 0.7f));
                    me->AddDelayedEvent(i * 100, [this, pos]() -> void
                    {
                        me->CastSpell(pos, 246135, true);
                    });
                }
            }

            if (spell->Id == SPELL_FIXATE)
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(target, 50000000.0f);
                me->GetMotionMaster()->MoveChase(target);
                events.RescheduleEvent(EVENT_FIXATE, 10000);
            }
        }

        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
        {
            if (spellId == SPELL_VOID_TEAR)
            {
                if (apply)
                {
                    phase = 0;
                    events.Reset();
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAurasDueToSpell(SPELL_MADDENED_FRENZY);
                    me->RemoveAurasDueToSpell(SPELL_FIXATE_CHECK);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                }
                else
                {
                    events.RescheduleEvent(EVENT_NULL_PALM, 11000);
                    events.RescheduleEvent(EVENT_DECIMATE, 18000);
                    events.RescheduleEvent(EVENT_COALESCED_VOID, 20000);
                    events.RescheduleEvent(EVENT_UMBRA_SHIFT, 41000);
                }
            }
        }

        void SpellFinishCast(SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_DECIMATE)
                DoCast(me, SPELL_FIXATE_CHECK, true);
        }

        Player* DPSSelector()
        {
            std::vector<Player*> DPSSelector;
            DPSSelector.clear();

            Map::PlayerList const &players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                if (Player* player = itr->getSource()->ToPlayer())
                    if (!player->isGameMaster() && player->isAlive())
                        if (player->IsRangedDamageDealer(false) || player->IsMeleeDamageDealer())
                            DPSSelector.push_back(player);
                    
            if (DPSSelector.empty())
                return NULL;
            return Trinity::Containers::SelectRandomContainerElement(DPSSelector);
        }

        void UpdateAI(uint32 diff) override 
        {
            if (!UpdateVictim())
                return;

            if (CheckHomeDistToEvade(diff, 60.0f))
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_NULL_PALM:
                    if (!phase)
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 39.0f, true))
                            DoCast(target, SPELL_NULL_PALM);
                        
                    events.RescheduleEvent(EVENT_NULL_PALM, 55000);
                    break;
                case EVENT_DECIMATE:
                    if (!phase)
                        DoCast(SPELL_DECIMATE);

                    events.RescheduleEvent(EVENT_DECIMATE, 12000);
                    break;
                case EVENT_COALESCED_VOID:
                    if (!phase)
                    {
                        for (uint8 i = 0; i < 8; i++)
                            if (auto voids = me->SummonCreature(NPC_COALESCED_VOID, voidPos[i]))
                                voids->AI()->DoZoneInCombat(voids, 100.0f);
                    }
                    events.RescheduleEvent(EVENT_COALESCED_VOID, 55000);
                    break;
                case EVENT_UMBRA_SHIFT:
                    if (!phase)
                    {
                        if (auto plr = DPSSelector())
                        {
                            Talk(SAY_VOID_REALM);
                            plr->CastSpell(plr, SPELL_UMBRA_SHIFT);
                            phase = 1;
                        }
                        events.RescheduleEvent(EVENT_FIXATE, 1000);
                    }
                    break;
                case EVENT_FIXATE:
                    if (phase)
                    {
                        DoCast(SPELL_FIXATE);
                        me->SetReactState(REACT_PASSIVE);
                        DoCast(me, SPELL_MADDENED_FRENZY, true);
                    }
                    break;
                }
            }
            if (!phase)
                DoMeleeAttackIfReady();
            else
                DoSpellAttackIfReady(249298);
        }

    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_zuraal_the_ascendedAI (creature);
    }
};

//122716
class npc_zuraal_coalesced_void : public CreatureScript
{
public:
    npc_zuraal_coalesced_void() : CreatureScript("npc_zuraal_coalesced_void") {}

    struct npc_zuraal_coalesced_voidAI : public ScriptedAI
    {
        npc_zuraal_coalesced_voidAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        EventMap events;
        uint16 movetoboss;

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            if (!summoner->isInCombat())
            {
                me->DespawnOrUnsummon();
                return;
            }
            me->SetSpeed(MOVE_RUN, 0.3f);
            me->SetSpeed(MOVE_WALK, 0.3f);
            DoCast(me, SPELL_C_V_SPAWN);

            if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                DoCast(me, SPELL_UMBRAL_EJECTION);

            movetoboss = 1000;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (movetoboss <= diff)
            {
                movetoboss = 1000;

                if (Creature* target = me->FindNearestCreature(NPC_ZURAAL, 60.0f, true))
                    me->GetMotionMaster()->MoveChase(target);

                if (Creature* target = me->FindNearestCreature(NPC_ZURAAL, 1.7f, true))
                {
                    DoCast(me, SPELL_DARK_EXPULSION, true);
                    me->DespawnOrUnsummon(100);
                }

            }
            else movetoboss -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_zuraal_coalesced_voidAI(creature);
    }
};

// 124171
class npc_sott_shadowguard_subjugator : public CreatureScript
{
public:
    npc_sott_shadowguard_subjugator() : CreatureScript("npc_sott_shadowguard_subjugator") {}

    struct npc_sott_shadowguard_subjugatorAI : public ScriptedAI
    {
        npc_sott_shadowguard_subjugatorAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;

        void Reset() override 
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->SetReactState(REACT_DEFENSIVE);
            DoCast(summoner, SPELL_VOID_CONTAINMENT, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_NB, urand(10000,15000));
            events.RescheduleEvent(EVENT_FIELD, 20000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_NB:
                    DoCast(SPELL_NEGATING_BRAND);
                    events.RescheduleEvent(EVENT_NB, urand(10000, 15000));
                    break;
                case EVENT_FIELD:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 30.0f, true))
                        DoCast(target, SPELL_SUPPRESSION_FIELD);
                    events.RescheduleEvent(EVENT_FIELD, urand(20000, 25000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sott_shadowguard_subjugatorAI(creature);
    }
};

//123743
class npc_zuraal_alleria : public CreatureScript
{
public:
    npc_zuraal_alleria() : CreatureScript("npc_zuraal_alleria") {}

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (InstanceScript* instance = creature->GetInstanceScript())
            if (instance->GetBossState(DATA_ZURAAL) != DONE)
                if (!creature->HasPhaseId(8683))
                    return false;

        player->PlayerTalkClass->SendCloseGossip();
        creature->CastSpell(creature, SPELL_DESTROY_WALL);
        creature->AI()->DoAction(1);
        creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        if (Creature* target = creature->FindNearestCreature(NPC_ALLERIA_TARGET, 60.0f, true))
            creature->SetFacingTo(target);

        return true;
    }

    struct npc_zuraal_alleriaAI : public ScriptedAI
    {
        npc_zuraal_alleriaAI(Creature* creature) : ScriptedAI(creature) 
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        InstanceScript* instance;
        bool convers = false;

        void DoAction(int32 const action) override
        {
            switch (action)
            {
            case 1:
                me->AddDelayedEvent(6500, [this] {
                    Talk(SAY_3);
                    if (instance)
                        instance->SetBossState(DATA_ALLERIA1, DONE);
                });
                break;
            case 2:
                if (instance && !convers)
                {
                    convers = true;
                    Conversation* conversation = new Conversation;
                    if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 5632, me, NULL, *me))
                        delete conversation;
                    me->AddDelayedEvent(18000, [this] {
                        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    });
                }
                break;
            }
        }

        void IsSummonedBy(Unit* summoner) override
        {
            if (summoner->GetEntry() == NPC_ZURAAL)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->AddDelayedEvent(1000, [this] {
                    Talk(SAY_1);
                });

                me->AddDelayedEvent(3500, [this] {
                    Talk(SAY_2);
                });

                me->AddDelayedEvent(6500, [this]() -> void
                {
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(0, assaultStart[2]);
                });

                me->AddDelayedEvent(10000, [this]() -> void
                {
                    me->GetMotionMaster()->MovePoint(0, assaultStart[3]);
                });

                me->AddDelayedEvent(15000, [this]() -> void
                {
                    me->DespawnOrUnsummon();
                });
            }
            if (summoner->GetEntry() == NPC_LURA_INTROPORTAL)
            {
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->AddDelayedEvent(2000, [this] {
                    if (instance)
                    {
                        me->GetMotionMaster()->MovePoint(0, openDoor[0]);
                        Conversation* conversation = new Conversation;
                        if (!conversation->CreateConversation(sObjectMgr->GetGenerator<HighGuid::Conversation>()->Generate(), 5430, me, NULL, *me))
                            delete conversation;
                    }
                });
                me->AddDelayedEvent(26000, [this] {
                    if (Creature* doorSt = me->FindNearestCreature(NPC_DOOR_ST, 60.0f, true))
                        DoCast(doorSt, SPELL_OPEN_DOOR_VISUAL);
                });
                me->AddDelayedEvent(34000, [this] {
                    if (Creature* doorSt = me->FindNearestCreature(NPC_DOOR_ST, 60.0f, true))
                    {
                        me->InterruptSpell(CURRENT_CHANNELED_SPELL);
                        me->Kill(doorSt);
                    }
                });
                me->AddDelayedEvent(60000, [this] {
                    me->DespawnOrUnsummon();
                });
            }
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_zuraal_alleriaAI(creature);
    }
};

//125099
class npc_sott_walker_after_nezhar : public CreatureScript
{
public:
    npc_sott_walker_after_nezhar() : CreatureScript("npc_sott_walker_after_nezhar") {}

    struct npc_sott_walker_after_nezharAI : public ScriptedAI
    {
        npc_sott_walker_after_nezharAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void IsSummonedBy(Unit* summoner) override
        {
            if (summoner->GetEntry() == NPC_LURA_INTROPORTAL)
            {
                me->AddDelayedEvent(2000, [this] {
                    me->GetMotionMaster()->MovePoint(0, openDoor[1]);
                });
                me->AddDelayedEvent(12000, [this] {
                    if (Creature* doorSt = me->FindNearestCreature(NPC_DOOR_ST, 60.0f, true))
                        DoCast(doorSt, SPELL_OPEN_DOOR_VISUAL);
                });
                me->AddDelayedEvent(60000, [this] {
                    me->DespawnOrUnsummon();
                });
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sott_walker_after_nezharAI(creature);
    }
};

//125104
class npc_sott_lura_door : public CreatureScript
{
public:
    npc_sott_lura_door() : CreatureScript("npc_sott_lura_door") {}

    struct npc_sott_lura_doorAI : public ScriptedAI
    {
        npc_sott_lura_doorAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            if (me->isAlive())
                instance->SetBossState(DATA_ALLERIA2, NOT_STARTED);
        }

        InstanceScript* instance;

        void Reset() override { }

        void JustDied(Unit* /*killer*/) override
        {
            if (instance)
                instance->SetBossState(DATA_ALLERIA2, DONE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sott_lura_doorAI(creature);
    }
};

//123744
class npc_zuraal_walker : public CreatureScript
{
public:
    npc_zuraal_walker() : CreatureScript("npc_zuraal_walker") {}

    struct npc_zuraal_walkerAI : public ScriptedAI
    {
        npc_zuraal_walkerAI(Creature* creature) : ScriptedAI(creature) {}

        void IsSummonedBy(Unit* summoner) override
        {
            me->AddDelayedEvent(7500, [this] {
                me->SetWalk(false);
                me->GetMotionMaster()->MovePoint(0, assaultStart[2]);
            });

            me->AddDelayedEvent(10500, [this]() -> void
            {
                me->GetMotionMaster()->MovePoint(0, assaultStart[3]);
            });

            me->AddDelayedEvent(17000, [this] {
                me->DespawnOrUnsummon();
            });
        }

        void UpdateAI(uint32 diff) override {}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_zuraal_walkerAI(creature);
    }
};

//1602  
class eventobject_sott_alleria_conversation : public EventObjectScript
{
public:
    eventobject_sott_alleria_conversation() : EventObjectScript("eventobject_sott_alleria_conversation") {}

    bool meeted = false;

    bool IsTriggerMeets(Player* player, EventObject* eo) override
    {
        if (InstanceScript* instance = eo->GetInstanceScript())
            if (instance->GetBossState(DATA_ZURAAL) == DONE)
                if (Creature* alleria = eo->FindNearestCreature(NPC_ALLERIA_E1, 20.0f, true))
                {
                    meeted = true;
                    if (alleria->IsAIEnabled)
                        alleria->GetAI()->DoAction(2);
                    return true;
                }

        return false;
    }
};

// 1064300 custom entry
class at_zuraal_decimate : public AreaTriggerScript
{
public:
    at_zuraal_decimate() : AreaTriggerScript("at_zuraal_decimate") { }

    struct at_zuraal_decimateAI : AreaTriggerAI
    {
        at_zuraal_decimateAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}
        
        bool dam = true;
        void ActionOnUpdate(GuidList& affectedPlayers) override
        {
            if (at->GetRadius() <= 15.0f)
                at->SetSphereScale(dam ? 0.015f : 0.2, 15);
        }

    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new at_zuraal_decimateAI(areatrigger);
    }
};



void AddSC_boss_zuraal_the_ascended()
{
    new boss_zuraal_the_ascended();
    new npc_zuraal_coalesced_void();
    new npc_sott_shadowguard_subjugator();
    new npc_zuraal_alleria();
    new npc_zuraal_walker();
    new npc_sott_walker_after_nezhar();
    new npc_sott_lura_door();
    new eventobject_sott_alleria_conversation();
    new at_zuraal_decimate();
}