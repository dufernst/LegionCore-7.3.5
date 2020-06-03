#include "blackrock_caverns.h"

//todo: сделать спелл трансформации

enum ScriptTexts
{
    SAY_AGGRO               = 0,
    SAY_KILL                = 1,
    SAY_TRANSFORMATION      = 2,
    SAY_DEATH               = 3
};
enum Spells
{
    SPELL_STONEBLOW                 = 76185,
    SPELL_TWILIGHT_CORRUPTION       = 76188,
    SPELL_THUNDERCLAP               = 76186,
    SPELL_TWITCHY                   = 76167,
    SPELL_SHADOW_OF_OBSIDIUS        = 76164,
    SPELL_CREPUSCULAR_VEIL          = 76189
};

enum Events
{
    EVENT_STONEBLOW             = 1,
    EVENT_TWILIGHT_CORRUPTION   = 2,
    EVENT_THUNDERCLAP           = 3,
    EVENT_CREPUSCULAR_VEIL      = 4
};

enum Adds
{
    NPC_SHADOW_OF_OBSIDIUS    = 40817
};

const Position shadowofobsidiusPos[3] = 
{
    {328.19f, 561.97f, 66.0f, 4.50f},
    {335.24f, 556.08f, 66.0f, 1.64f},
    {330.25f, 549.84f, 660.f, 3.10f}
};
class boss_ascendant_lord_obsidius : public CreatureScript
{
    public:
        boss_ascendant_lord_obsidius() : CreatureScript("boss_ascendant_lord_obsidius") {}
     
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_ascendant_lord_obsidiusAI (pCreature);
        }
     
        struct boss_ascendant_lord_obsidiusAI : public ScriptedAI
        {
            boss_ascendant_lord_obsidiusAI(Creature* c) : ScriptedAI(c), summons(me)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                instance = (InstanceScript*)c->GetInstanceScript();
            }
     
            InstanceScript* instance;
            EventMap events;
            SummonList summons;
            Creature* shadows[3];

            void Reset() override
            {
                summons.DespawnAll();
                events.Reset();
                if (instance)
                {
                    if (IsHeroic())
                        for (uint8 i = 0; i < 3; i++)
                            shadows[i] = me->SummonCreature(NPC_SHADOW_OF_OBSIDIUS, shadowofobsidiusPos[i]);
                    else
                        for (uint8 i = 1; i < 3; i++)
                            shadows[i] = me->SummonCreature(NPC_SHADOW_OF_OBSIDIUS, shadowofobsidiusPos[i]);
                }
                instance->SetData(DATA_ASCENDANT_LORD_OBSIDIUS, NOT_STARTED);
            }

            void EnterCombat(Unit* who) override
            {
                events.RescheduleEvent(EVENT_STONEBLOW, 6000);
                events.RescheduleEvent(EVENT_TWILIGHT_CORRUPTION, 20000);
                if (IsHeroic())
                    events.RescheduleEvent(EVENT_THUNDERCLAP, 6000);
                Talk(SAY_AGGRO);
                DoZoneInCombat();
                me->RemoveAura(75054);
                if (instance)
                    instance->SetData(DATA_ASCENDANT_LORD_OBSIDIUS, IN_PROGRESS);
            }
     
            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                
                if (me->GetDistance(me->GetHomePosition()) > 60.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;
                
                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_STONEBLOW:
                            DoCast(me->getVictim(), SPELL_STONEBLOW);
                            events.RescheduleEvent(EVENT_STONEBLOW, 6000);
                            break;
                        case EVENT_TWILIGHT_CORRUPTION:
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_TWILIGHT_CORRUPTION);
                            events.RescheduleEvent(EVENT_TWILIGHT_CORRUPTION, 20000);
                            break;
                        case EVENT_THUNDERCLAP:
                            DoCast(me, SPELL_THUNDERCLAP);
                            events.RescheduleEvent(EVENT_THUNDERCLAP, 15000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
     
            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
                summons.DespawnAll();
                if (instance)
                    instance->SetData(DATA_ASCENDANT_LORD_OBSIDIUS, DONE);
            }
     
            void KilledUnit(Unit * victim) override
            {
                Talk(SAY_KILL);
            }
        };
};

class npc_shadow_of_obsidius : public CreatureScript
{
    public:
        npc_shadow_of_obsidius() : CreatureScript("npc_shadow_of_obsidius") {}

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_shadow_of_obsidiusAI(creature);
        }

        struct npc_shadow_of_obsidiusAI : public ScriptedAI
        {
            npc_shadow_of_obsidiusAI(Creature* creature) : ScriptedAI(creature) 
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;
            
            void Reset() override
            {
                DoCast(me, SPELL_TWITCHY);
                DoCast(me, SPELL_SHADOW_OF_OBSIDIUS);
            }   

            void EnterCombat(Unit* /*attacker*/) override
            {
                me->RemoveAura(75054);
                events.RescheduleEvent(EVENT_CREPUSCULAR_VEIL, 3900);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
            {
                if (me->getVictim() != attacker)
                {
                    DoResetThreat();
                    me->AddThreat(attacker, 1000000.0f);
                    me->Attack(attacker, true);
                    me->GetMotionMaster()->MoveChase(attacker);
                }
            }

            void UpdateAI(uint32 diff) override
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
                    case EVENT_CREPUSCULAR_VEIL:
                        DoCast(me->getVictim(), SPELL_CREPUSCULAR_VEIL);
                        events.RescheduleEvent(EVENT_CREPUSCULAR_VEIL, 3900);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };           
};

void AddSC_boss_ascendant_lord_obsidius()
{
    new boss_ascendant_lord_obsidius();
    new npc_shadow_of_obsidius();
}
