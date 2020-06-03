#include "zulaman.h"

enum Spells
{
    SPELL_REMOVE_AMANI_CURSE        = 43732,
    SPELL_PUSH_MOJO                 = 43923,
    SPELL_SUMMON_MONEY_BAG          = 43774,
    SPELL_SUMMON_AMANI_CHARM_CHEST1 = 43835,
    SPELL_SUMMON_AMANI_CHARM_CHEST2 = 43756,

    // Amani'shi Tempest
    SPELL_THUNDERCLAP               = 44033,
    SPELL_CHAIN_LIGHTNING           = 97496,

    // Amani Eagle
    SPELL_TALON                     = 43517,

    // Amani'shi Warrior
    SPELL_CHARGE                    = 43519,
    SPELL_KICK                      = 43518,

    // Achievement Hex Mix
    SPELL_MELISSA_ACHIEVEMENT       = 97905,
    SPELL_MAWAGO_ACHIEVEMENT        = 97906,
    SPELL_TYLLAN_ACHIEVEMENT        = 97907,
    SPELL_MICAH_ACHIEVEMENT         = 97908,
    SPELL_ARINOTH_ACHIEVEMENT       = 97909,
    SPELL_KALDRICK_ACHIEVEMENT      = 97910,
    SPELL_LENZO_ACHIEVEMENT         = 97911,
    SPELL_MELASONG_ACHIEVEMENT      = 97912,
    SPELL_HARALD_ACHIEVEMENT        = 97913,
    SPELL_RELISSA_ACHIEVEMENT       = 97914,
    SPELL_EULINDA_ACHIEVEMENT       = 97915,
    SPELL_ROSA_ACHIEVEMENT          = 97916,

};

enum Events
{
    // Amani'shi Tempest
    EVENT_SUMMON_EAGLES     = 1,
    EVENT_SUMMON_MOBS       = 2,
    EVENT_THUNDERCLAP       = 3,
    EVENT_CHAIN_LIGHTNING   = 4,

    // Amani'shi Warrior
    EVENT_CHARGE1            = 5,
    EVENT_KICK              = 6,

    // Amani Eagle
    SPELL_TALON1             = 7,
};

enum Points
{
    POINT_TURN  = 1,
    POINT_DOWN  = 2,
};

enum Actions
{
    ACTION_START_EVENT  = 1,
};

const Position posTurnPoint = {232.83f, 1367.78f, 48.58f, 1.79f};
const Position posDownPoint = {227.75f, 1460.83f, 25.98f, 4.75f};
const Position posUpPoint   = {280.12f, 1380.63f, 49.35f, 3.46f};

class npc_zulaman_forest_frog : public CreatureScript
{
    public:
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_zulaman_forest_frogAI>(pCreature);
        }

        npc_zulaman_forest_frog() : CreatureScript("npc_zulaman_forest_frog") { }

        struct npc_zulaman_forest_frogAI : public ScriptedAI
        {
            npc_zulaman_forest_frogAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                instance = pCreature->GetInstanceScript();
                reward = 0;
            }

            InstanceScript *instance;
            uint8 reward;

            void DoSpawnRandom()
            {
                if (instance)
                {
                    uint32 cEntry = 0;
                    uint32 spellId = 0; // For Achievement
                    switch(urand(0, 11))
                    {
                        case 0: cEntry = NPC_HARALD; spellId = SPELL_HARALD_ACHIEVEMENT; break;
                        case 1: cEntry = NPC_EULINDA; spellId = SPELL_EULINDA_ACHIEVEMENT; break;
                        case 2: cEntry = NPC_ARINOTH; reward = 1; spellId = SPELL_ARINOTH_ACHIEVEMENT; break;
                        case 3: cEntry = NPC_LENZO; reward = 1; spellId = SPELL_LENZO_ACHIEVEMENT; break;
                        case 4: cEntry = NPC_MELISSA; reward = 2; spellId = SPELL_MELISSA_ACHIEVEMENT; break;
                        case 5: cEntry = NPC_MAWAGO; reward = 2; spellId = SPELL_MAWAGO_ACHIEVEMENT; break;
                        case 6: cEntry = NPC_MELASONG; reward = 2; spellId = SPELL_MELASONG_ACHIEVEMENT;; break;
                        case 7: cEntry = NPC_ROSA; reward = 2; spellId = SPELL_ROSA_ACHIEVEMENT; break;
                        case 8: cEntry = NPC_RELISSA; reward = 2; spellId = SPELL_RELISSA_ACHIEVEMENT; break;
                        case 9: cEntry = NPC_TYLLAN; reward = 2; spellId = SPELL_TYLLAN_ACHIEVEMENT; break;
                        case 10: cEntry = NPC_KALDRICK; reward = 2; spellId = SPELL_KALDRICK_ACHIEVEMENT; break;
                        case 11: cEntry = NPC_MICAH; reward = 2; spellId = SPELL_MICAH_ACHIEVEMENT; break;
                    }

                    if (cEntry == NPC_HARALD && instance->GetData(DATA_VENDOR_1))
                        cEntry = NPC_TYLLAN;
                    if (cEntry == NPC_EULINDA && instance->GetData(DATA_VENDOR_2))
                        cEntry = NPC_ARINOTH;

                    if (cEntry) 
                        me->UpdateEntry(cEntry);

                    if (spellId)
                        DoCastAOE(spellId, true);
                    
                    // There must be only one vendor per instance
                    if (cEntry == NPC_HARALD)
                        instance->SetData(DATA_VENDOR_1, DONE);
                    else if (cEntry == NPC_EULINDA)
                        instance->SetData(DATA_VENDOR_2, DONE);
                    else
                    {
                        if (reward == 1)
                            me->CastSpell(me, SPELL_SUMMON_MONEY_BAG, true);
                        else if (reward == 2)
                            me->CastSpell(me, urand(0, 1)? SPELL_SUMMON_AMANI_CHARM_CHEST1: SPELL_SUMMON_AMANI_CHARM_CHEST2, true);

                        me->DespawnOrUnsummon(5000);
                    }
                }
            }

            void SpellHit(Unit *caster, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_REMOVE_AMANI_CURSE && caster->GetTypeId() == TYPEID_PLAYER && me->GetEntry() == NPC_FOREST_FROG)
                {
                    if (roll_chance_i(6))
                    {
                        if (!caster->ToPlayer()->HasItemCount(33993) && !caster->HasSpell(43918))
                        {
                            DoCast(caster, SPELL_PUSH_MOJO, true);
                            me->DespawnOrUnsummon();
                        }
                        else
                            DoSpawnRandom();
                    }
                    else
                        DoSpawnRandom();
                }
            }
        };
};

class go_strange_gong : public GameObjectScript
{
    public:
        go_strange_gong() : GameObjectScript("go_strange_gong") {}
        
        bool OnGossipHello(Player* pPlayer, GameObject* pGo)
        {
            if (InstanceScript* instance = pGo->GetInstanceScript())
            {
                instance->SetData(DATA_MAIN_GATE, 1);
            }
            return false;
        }
};

class npc_amanishi_tempest : public CreatureScript
{
    public:
        npc_amanishi_tempest() : CreatureScript("npc_amanishi_tempest") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_amanishi_tempestAI>(pCreature);
        }

        struct npc_amanishi_tempestAI : public ScriptedAI
        {
            npc_amanishi_tempestAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_IMMUNE_TO_PC);
            }

            EventMap events;

            void Reset()
            {
                events.Reset();
            }

            void EnterCombat(Unit* attacker)
            {
                events.RescheduleEvent(EVENT_THUNDERCLAP, urand(5000, 10000));
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(6000, 12000));
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
                        case EVENT_THUNDERCLAP:
                            DoCastAOE(SPELL_THUNDERCLAP);
                            events.RescheduleEvent(EVENT_THUNDERCLAP, urand(14000, 18000));
                            break;
                        case EVENT_CHAIN_LIGHTNING:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(pTarget, SPELL_CHAIN_LIGHTNING);
                            events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(10000, 12000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
     };
};

void AddSC_zulaman()
{
    new npc_zulaman_forest_frog();
    new go_strange_gong();
    //new npc_amanishi_tempest();
    //new npc_amanishi_lookout();
    //new npc_amani_eagle();
    //new npc_amanishi_warrior();
}