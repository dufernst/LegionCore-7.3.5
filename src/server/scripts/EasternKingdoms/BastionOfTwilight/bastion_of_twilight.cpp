#include "bastion_of_twilight.h"

enum ScriptTexts
{
    SAY_CHOGALL_0  = 0,
    SAY_CHOGALL_1  = 1,
    SAY_CHOGALL_2  = 2,
    SAY_CHOGALL_3  = 3,
    SAY_CHOGALL_4  = 4,
    SAY_CHOGALL_5  = 5,
    SAY_CHOGALL_6  = 6,
    SAY_CHOGALL_7  = 7,
    SAY_CHOGALL_8  = 8,
    SAY_CHOGALL_9  = 9,
    SAY_CHOGALL_10 = 10,
    SAY_CHOGALL_11 = 11,
    SAY_CHOGALL_12 = 12,
    SAY_CHOGALL_13 = 13,
    SAY_CHOGALL_14 = 14,
};

enum Spells
{
    //twilight portal shaper
    SPELL_SHADOW_BOLT               = 85544,
    SPELL_SHAPE_PORTAL              = 85529,
    SPELL_SHAPE_PORTAL_SUM          = 85528,
    
    //twilight shifter
    SPELL_TWILIGHT_SHIFT            = 85556,

    //twilight shadow mender
    SPELL_MIND_SEAR                 = 85643,
    SPELL_MIND_SEAR_DMG             = 85647,
    SPELL_UMBRAL_FLAMES             = 85664,
    SPELL_UMBRAL_FLAMES_DMG         = 85679,  
    SPELL_SHADOW_MENDING            = 85575,
    SPELL_SHADOW_MENDING_HEAL       = 85577,
};

enum CreaturesIds
{
    NPC_TWILIGHT_PORTAL_SHAPER      = 45700,
    NPC_TWILIGHT_PORTAL             = 45885,
    NPC_FACELESS_MINION             = 45703,
    NPC_TWILIGHT_SHIFTER            = 45687,
    NPC_TWILIGHT_SHADOW_MENDER      = 45699,
};

enum Events
{
    //twilight portal shaper
    EVENT_SHADOW_BOLT               = 1,
    EVENT_SHAPE_PORTAL              = 2,

    //twilight shifter
    EVENT_TWILIGHT_SHIFT            = 3,
    EVENT_NEXT_TARGET_ON            = 4,
    EVENT_NEXT_TARGET_OFF           = 5,

    //twilight shadow mender
    EVENT_MIND_SEAR                 = 6,
    EVENT_UMBRAL_FLAMES             = 7,

    //chogall dlg
    EVENT_ENTRANCE_DLG              = 111,
    EVENT_HALFUS_DLG_1              = 112,
    EVENT_HALFUS_DLG_2              = 113,
    EVENT_VALIONA_THERALION_DLG_1   = 114,
    EVENT_VALIONA_THERALION_DLG_2   = 115,
    EVENT_COUNCIL_DLG_1             = 116,
    EVENT_COUNCIL_DLG_2             = 117,
    EVENT_COUNCIL_DLG_3             = 118,
    EVENT_CHOGALL_DLG               = 119,
};

class npc_twilight_portal_shaper: public CreatureScript
{
public:
    npc_twilight_portal_shaper() : CreatureScript("npc_twilight_portal_shaper") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_twilight_portal_shaperAI(pCreature);
    }

    struct npc_twilight_portal_shaperAI : public ScriptedAI
    {
        npc_twilight_portal_shaperAI(Creature* pCreature) : ScriptedAI(pCreature), summons(me)
        {
        }

        SummonList summons;
        EventMap events;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (summon->GetEntry() == NPC_FACELESS_MINION)
                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    summon->AI()->AttackStart(pTarget);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void JustDied(Unit* who)
        {
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_SHADOW_BOLT, 1000);
            events.RescheduleEvent(EVENT_SHAPE_PORTAL, urand(10000, 15000));
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
                case EVENT_SHADOW_BOLT:
                    DoCast(me->getVictim(), SPELL_SHADOW_BOLT);
                    events.RescheduleEvent(EVENT_SHADOW_BOLT, 2000);
                    break;
                case EVENT_SHAPE_PORTAL:
                    DoCast(me, SPELL_SHAPE_PORTAL);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_twilight_shifter: public CreatureScript
{
public:
    npc_twilight_shifter() : CreatureScript("npc_twilight_shifter") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_twilight_shifterAI(pCreature);
    }

    struct npc_twilight_shifterAI : public ScriptedAI
    {
        npc_twilight_shifterAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_TWILIGHT_SHIFT, urand(5000, 15000));
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
                case EVENT_TWILIGHT_SHIFT:
                    DoCast(me, SPELL_TWILIGHT_SHIFT);
                    events.RescheduleEvent(EVENT_TWILIGHT_SHIFT, urand(30000, 35000));
                    events.RescheduleEvent(EVENT_NEXT_TARGET_ON, 3000);
                    break;
                case EVENT_NEXT_TARGET_ON:
                    DoResetThreat();
                    events.RescheduleEvent(EVENT_NEXT_TARGET_ON, 3000);
                    break;
                case EVENT_NEXT_TARGET_OFF:
                    events.CancelEvent(EVENT_NEXT_TARGET_ON);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_twilight_shadow_mender: public CreatureScript
{
public:
    npc_twilight_shadow_mender() : CreatureScript("npc_twilight_shadow_mender") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_twilight_shadow_menderAI(pCreature);
    }

    struct npc_twilight_shadow_menderAI : public ScriptedAI
    {
        npc_twilight_shadow_menderAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            DoCast(me, SPELL_SHADOW_MENDING);
            events.RescheduleEvent(EVENT_UMBRAL_FLAMES, urand(10000, 13000));
            events.RescheduleEvent(EVENT_MIND_SEAR, urand(3000, 5000));
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
                case EVENT_UMBRAL_FLAMES:
                    DoCast(me, SPELL_UMBRAL_FLAMES);
                    events.RescheduleEvent(EVENT_UMBRAL_FLAMES, urand(15000, 20000));
                    break;
                case EVENT_MIND_SEAR:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        DoCast(pTarget, SPELL_MIND_SEAR);
                    events.RescheduleEvent(EVENT_MIND_SEAR, urand(10000, 12000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_chogall_dlg: public CreatureScript
{
public:
    npc_chogall_dlg() : CreatureScript("npc_chogall_dlg") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_chogall_dlgAI(pCreature);
    }

    struct npc_chogall_dlgAI : public ScriptedAI
    {
        npc_chogall_dlgAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
        }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
            case ACTION_AT_ENTRANCE:
                events.RescheduleEvent(EVENT_ENTRANCE_DLG, 5000);
                break;
            case ACTION_AT_HALFUS_START:
                events.RescheduleEvent(EVENT_HALFUS_DLG_1, 1000);
                break;
            case ACTION_AT_HALFUS_END:
                events.RescheduleEvent(EVENT_HALFUS_DLG_2, 7000);
                break;
            case ACTION_AT_VALIONA_THERALION_START:
                events.RescheduleEvent(EVENT_VALIONA_THERALION_DLG_1, 1000);
                break;
            case ACTION_AT_VALIONA_THERALION_END:
                events.RescheduleEvent(EVENT_VALIONA_THERALION_DLG_2, 7000);
                break;
            case ACTION_AT_COUNCIL_1:
                events.RescheduleEvent(EVENT_COUNCIL_DLG_1, 1000);
                break;
            case ACTION_AT_COUNCIL_2:
                events.RescheduleEvent(EVENT_COUNCIL_DLG_2, 1000);
                break;
            case ACTION_AT_COUNCIL_3:
                events.RescheduleEvent(EVENT_COUNCIL_DLG_3, 1000);
                break;
            case ACTION_AT_CHOGALL:
                events.RescheduleEvent(EVENT_CHOGALL_DLG, 1000);
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ENTRANCE_DLG:
                    Talk(SAY_CHOGALL_0);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_HALFUS_DLG_1:
                    Talk(SAY_CHOGALL_1);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_HALFUS_DLG_2:
                    Talk(SAY_CHOGALL_2);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_VALIONA_THERALION_DLG_1:
                    Talk(SAY_CHOGALL_9);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_VALIONA_THERALION_DLG_2:
                    Talk(SAY_CHOGALL_10);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_COUNCIL_DLG_1:
                    Talk(SAY_CHOGALL_11);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_COUNCIL_DLG_2:
                    Talk(SAY_CHOGALL_12);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_COUNCIL_DLG_3:
                    Talk(SAY_CHOGALL_13);
                    me->DespawnOrUnsummon();
                case EVENT_CHOGALL_DLG:
                    Talk(SAY_CHOGALL_14);
                    me->DespawnOrUnsummon();
                    break;
                }
            }
        }
    };
};

class at_bt_entrance : public AreaTriggerScript
{
public:
    at_bt_entrance() : AreaTriggerScript("at_bt_entrance") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_ENTRANCE) != DONE)
            {
                if (Creature* pChogall    = pPlayer->SummonCreature(NPC_CHOGALL_DLG,
                    pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                {
                    pChogall->AI()->DoAction(ACTION_AT_ENTRANCE);
                }
                instance->SetData(DATA_DLG_ENTRANCE, DONE);
            }
        }
        return true;
    }
};

class at_bt_halfus: public AreaTriggerScript
{
public:
    at_bt_halfus() : AreaTriggerScript("at_bt_halfus") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_HALFUS) != DONE)
            {
                if (Creature* pChogall = pPlayer->SummonCreature(NPC_CHOGALL_DLG, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                    if (pChogall->AI())
                        pChogall->AI()->DoAction(ACTION_AT_HALFUS_START);
                instance->SetData(DATA_DLG_HALFUS, DONE);
            }
        }
        return true;
    }
};

class at_bt_valiona_theralion : public AreaTriggerScript
{
public:
    at_bt_valiona_theralion() : AreaTriggerScript("at_bt_valiona_theralion") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_VALIONA_THERALION) != DONE)
            {
                if (Creature* pChogall    = pPlayer->SummonCreature(NPC_CHOGALL_DLG,
                    pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                    pChogall->AI()->DoAction(ACTION_AT_VALIONA_THERALION_START);
                instance->SetData(DATA_DLG_VALIONA_THERALION, DONE);
            }
        }
        return true;
    }
};

class at_bt_council_1 : public AreaTriggerScript
{
public:
    at_bt_council_1() : AreaTriggerScript("at_bt_council_1") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_COUNCIL_1) != DONE)
            {
                if (Creature* pChogall    = pPlayer->SummonCreature(NPC_CHOGALL_DLG,
                    pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                    pChogall->AI()->DoAction(ACTION_AT_COUNCIL_1);
                instance->SetData(DATA_DLG_COUNCIL_1, DONE);
            }
        }
        return true;
    }
};

class at_bt_council_2 : public AreaTriggerScript
{
public:
    at_bt_council_2() : AreaTriggerScript("at_bt_council_2") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_COUNCIL_2) != DONE)
            {
                if (Creature* pChogall    = pPlayer->SummonCreature(NPC_CHOGALL_DLG,
                    pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                    pChogall->AI()->DoAction(ACTION_AT_COUNCIL_2);
                instance->SetData(DATA_DLG_COUNCIL_2, DONE);
            }
        }
        return true;
    }
};

class at_bt_council_3 : public AreaTriggerScript
{
public:
    at_bt_council_3() : AreaTriggerScript("at_bt_council_3") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_COUNCIL_3) != DONE)
            {
                if (Creature* pChogall    = pPlayer->SummonCreature(NPC_CHOGALL_DLG,
                    pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                    pChogall->AI()->DoAction(ACTION_AT_COUNCIL_3);
                instance->SetData(DATA_DLG_COUNCIL_3, DONE);
            }
        }
        return true;
    }
};

class at_bt_chogall : public AreaTriggerScript
{
public:
    at_bt_chogall() : AreaTriggerScript("at_bt_chogall") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_CHOGALL) != DONE)
            {
                if (Creature* pChogall    = pPlayer->SummonCreature(NPC_CHOGALL_DLG,
                    pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f))
                    pChogall->AI()->DoAction(ACTION_AT_CHOGALL);
                instance->SetData(DATA_DLG_CHOGALL, DONE);
            }
        }
        return true;
    }
};

class at_bt_sinestra : public AreaTriggerScript
{
public:
    at_bt_sinestra() : AreaTriggerScript("at_bt_sinestra") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/)
    {
        if (InstanceScript* instance = pPlayer->GetInstanceScript())
        {
            if (instance->GetData(DATA_DLG_SINESTRA) != DONE)
            {
                instance->SetData(DATA_DLG_SINESTRA, DONE);
            }
        }
        return true;
    }
};

void AddSC_bastion_of_twilight()
{
    new npc_twilight_portal_shaper();
    new npc_twilight_shifter();
    new npc_twilight_shadow_mender();
    new npc_chogall_dlg();
    new at_bt_entrance();
    new at_bt_halfus();
    new at_bt_valiona_theralion();
    new at_bt_council_1();
    new at_bt_council_2();
    new at_bt_council_3();
    new at_bt_chogall();
}