#include "halls_of_origination.h"

enum ScriptTexts
{
    // Brann Bronzebeard
    SAY_0   = 0,
    SAY_1   = 1,
    SAY_2   = 2,
    SAY_3   = 3,
    SAY_4   = 4,
    SAY_5   = 5,
    SAY_6   = 6,
    SAY_7   = 7,
    SAY_8   = 8,
    SAY_9   = 9,
    SAY_10  = 10
};

enum Spells
{
    // Air Warden
    SPELL_WIND_SNEAR            = 77334,

    // Flame Warden
    SPELL_RAGING_INFERNO        = 77241,
    SPELL_RAGING_INFERNO_DMG    = 77262,
    SPELL_LAVA_ERUPTION         = 77273,

    // Water Warden
    SPELL_BUBBLE_BOUND          = 77336,

    // Earth Warden
    SPELL_IMPALE                = 77235,
    SPELL_ROCKWAVE              = 77234
};

enum Events
{
    // Air Warden
    EVENT_WIND_SNEAR        = 1,

    // Flame Warden
    EVENT_LAVA_ERUPTION     = 2,
    EVENT_RAGING_INFERNO    = 3,

    // Water Warden         
    EVENT_BUBBLE_BOUND      = 4,

    // Earth Warden
    EVENT_IMPALE            = 5,
    EVENT_ROCKWAVE          = 6,

    // Brann Bronzebeard
    EVENT_TALK_0            = 7,
    EVENT_TALK_1            = 8,
    EVENT_TALK_2            = 9,
    EVENT_TALK_3            = 10,
    EVENT_TALK_4            = 11,
    EVENT_TALK_5            = 12,
    EVENT_TALK_6            = 13,
    EVENT_TALK_7            = 14,
    EVENT_TALK_8            = 15,
    EVENT_TALK_9            = 16,
    EVENT_TALK_10           = 17,
    EVENT_TALK_11           = 18
};

enum Adds
{
    NPC_WHIRLING_WINDS  = 41245 // 77321
};

enum Actions
{
    ACTION_TALK_1       = 1,
    ACTION_TALK_2       = 2,
    ACTION_TALK_3       = 3,
    ACTION_TALK_4       = 4,
    ACTION_START_EVENT  = 5
};

#define GOSSIP_BRANN_START_EVENT "Let's go"

class npc_air_warden : public CreatureScript
{
    public:
        npc_air_warden() : CreatureScript("npc_air_warden") {}
 
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_air_wardenAI(creature);
        }

        struct npc_air_wardenAI : public ScriptedAI
        {
            npc_air_wardenAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }
 
            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_WIND_SNEAR, urand(2000, 6000));
                DoZoneInCombat();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (instance)
                    instance->SetData(DATA_WARDENS, 1);
            }
        
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                                       
                events.Update(diff);
 
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                    case EVENT_WIND_SNEAR:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_WIND_SNEAR);
                        events.ScheduleEvent(EVENT_WIND_SNEAR, urand(7000, 10000));
                        break;                                                 
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_flame_warden : public CreatureScript
{
    public:
        npc_flame_warden() : CreatureScript("npc_flame_warden") {}
 
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flame_wardenAI(creature);
        }

        struct npc_flame_wardenAI : public ScriptedAI
        {
            npc_flame_wardenAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }
 
            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_LAVA_ERUPTION, urand(4000, 7000));
                events.ScheduleEvent(EVENT_RAGING_INFERNO, urand(10000, 12000));
                DoZoneInCombat();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (instance)
                    instance->SetData(DATA_WARDENS, 1);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                                       
                events.Update(diff);
 
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_LAVA_ERUPTION:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_LAVA_ERUPTION);
                            events.ScheduleEvent(EVENT_LAVA_ERUPTION, urand(8000, 12000));
                            break;
                        case EVENT_RAGING_INFERNO:
                            DoCastAOE(SPELL_RAGING_INFERNO);
                            events.ScheduleEvent(EVENT_RAGING_INFERNO, urand(20000, 25000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_water_warden : public CreatureScript
{
    public:
        npc_water_warden() : CreatureScript("npc_water_warden") {}
 
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_water_wardenAI(creature);
        }

        struct npc_water_wardenAI : public ScriptedAI
        {
            npc_water_wardenAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }
 
            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_BUBBLE_BOUND, urand(10000, 15000));
                DoZoneInCombat();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (instance)
                    instance->SetData(DATA_WARDENS, 1);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                                       
                events.Update(diff);
 
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_BUBBLE_BOUND:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_BUBBLE_BOUND);
                            events.ScheduleEvent(EVENT_BUBBLE_BOUND, urand(10000, 15000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };
};

class npc_earth_warden : public CreatureScript
{
    public:
        npc_earth_warden() : CreatureScript("npc_earth_warden") {}
 
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_earth_wardenAI(creature);
        }

        struct npc_earth_wardenAI : public ScriptedAI
        {
            npc_earth_wardenAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            }
 
            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_IMPALE, urand(6000, 10000));
                events.ScheduleEvent(EVENT_ROCKWAVE, urand(12000, 15000));
                DoZoneInCombat();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (instance)
                    instance->SetData(DATA_WARDENS, 1);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                                       
                events.Update(diff);
 
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_IMPALE:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_IMPALE);
                            events.ScheduleEvent(EVENT_IMPALE, urand(10000, 15000));
                            break;
                        case EVENT_ROCKWAVE:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_ROCKWAVE);
                            events.ScheduleEvent(EVENT_ROCKWAVE, urand(15000, 20000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_water_warden_water_bubble : public CreatureScript
{
    public:
        npc_water_warden_water_bubble() : CreatureScript("npc_water_warden_water_bubble") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_water_warden_water_bubbleAI(creature);
        }

        struct npc_water_warden_water_bubbleAI : public Scripted_NoMovementAI
        {
            npc_water_warden_water_bubbleAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (auto pOwner = me->GetOwner())
                {
                    pOwner->RemoveAurasDueToSpell(SPELL_BUBBLE_BOUND);
                }
                me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!me->GetOwner())
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (me->GetOwner()->isAlive())
                    me->DespawnOrUnsummon();

                if (!me->GetOwner()->HasAura(SPELL_BUBBLE_BOUND))
                    me->DespawnOrUnsummon();
            }
        };
};

class npc_halls_of_origination_brann_bronzebeard : public CreatureScript
{
    public:
        npc_halls_of_origination_brann_bronzebeard() : CreatureScript("npc_halls_of_origination_brann_bronzebeard") {}
 
        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_halls_of_origination_brann_bronzebeardAI(creature);
        }

        bool OnGossipHello(Player* pPlayer, Creature* pCreature)
        {
            if (pCreature->isQuestGiver())
                pPlayer->PrepareQuestMenu(pCreature->GetGUID());

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_BRANN_START_EVENT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());

            return true;
        }

        bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
        {
            pPlayer->PlayerTalkClass->ClearMenus();

            if (uiAction == (GOSSIP_ACTION_INFO_DEF + 1))
            {
                pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                pCreature->AI()->DoAction(ACTION_START_EVENT);
            }

            return true;
        }

        struct npc_halls_of_origination_brann_bronzebeardAI : public ScriptedAI
        {
            npc_halls_of_origination_brann_bronzebeardAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
                pCreature->setActive(true);
            }
 
            InstanceScript* instance;
            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void DoAction(const int32 action) override
            {
                switch (action)
                {
                    case ACTION_TALK_1:
                        Talk(SAY_7);
                        break;
                    case ACTION_TALK_2:
                        Talk(SAY_8);
                        break;
                    case ACTION_TALK_3:
                        Talk(SAY_9);
                        break;
                    case ACTION_TALK_4:
                        Talk(SAY_10);
                        break;
                    case ACTION_START_EVENT:
                        events.ScheduleEvent(EVENT_TALK_0, 1000);
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {        
                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_TALK_0:
                            Talk(SAY_0);
                            events.ScheduleEvent(EVENT_TALK_1, 8000);
                            break;
                        case EVENT_TALK_1:
                            if (instance)
                            {
                                instance->HandleGameObject(instance->GetGuidData(DATA_ANRAPHET_ENTRANCE_DOOR), true);
                                instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, EVENT_FASTER_THAN_LIGHT);
                            }
                            Talk(SAY_1);
                            events.ScheduleEvent(EVENT_TALK_2, 6000);
                            break;
                        case EVENT_TALK_2:
                            Talk(SAY_2);
                            events.ScheduleEvent(EVENT_TALK_3, 10000);
                            break;
                        case EVENT_TALK_3:
                            Talk(SAY_3);
                            events.ScheduleEvent(EVENT_TALK_4, 5000);
                            break;
                        case EVENT_TALK_4:
                            Talk(SAY_4);
                            events.ScheduleEvent(EVENT_TALK_5, 4000);
                            break;
                        case EVENT_TALK_5:
                            Talk(SAY_5);
                            events.ScheduleEvent(EVENT_TALK_6, 4000);
                            break;
                        case EVENT_TALK_6:
                            Talk(SAY_6);
                            break;
                    }
                }
            }
        };
};

class go_halls_of_origination_transit_device : public GameObjectScript
{
    public:
        go_halls_of_origination_transit_device() : GameObjectScript("go_halls_of_origination_transit_device") {}

        bool OnGossipHello(Player* pPlayer, GameObject* pGo)
        {
            if (pPlayer->isInCombat())
                return true;
            return false;
        }
};

void AddSC_halls_of_origination()
{
    new npc_air_warden();
    new npc_flame_warden();
    new npc_water_warden();
    new npc_earth_warden();
    new npc_water_warden_water_bubble();
    new npc_halls_of_origination_brann_bronzebeard();
    new go_halls_of_origination_transit_device();
}
