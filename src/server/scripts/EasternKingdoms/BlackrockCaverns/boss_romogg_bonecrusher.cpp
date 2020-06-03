#include "blackrock_caverns.h"

//todo: реализовать призыв о помощи

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_CHAINS  = 2,
    SAY_DEATH   = 3
};

enum Spells
{
    SPELL_CALL_FOR_HELP         = 82137,
    SPELL_CHAINS_OF_WOE         = 75539,
    SPELL_WOUNDING_STRIKE       = 75571,
    SPELL_THE_SKULLCRACKER      = 75543,
    SPELL_QUAKE                 = 75272,
    SPELL_SKULLCRACKER          = 95324,
    SPELL_CHAINS_OF_WOE_0       = 75441,
    SPELL_CHAINS_OF_WOE_1       = 82189,
    SPELL_CHAINS_OF_WOE_ROOT    = 82189,
    SPELL_CHAINS_OF_WOE_TELE    = 75437,
    SPELL_CHAINS_OF_WOE_TELE_0  = 75464
};
 
enum Adds
{
    NPC_QUAKE                   = 40401,
    NPC_CHAINS_OF_WOE           = 40447,
    NPC_ANGERED_EARTH           = 50376
}; 

enum Events
{
    EVENT_WOUNDING_STRIKE       = 1,
    EVENT_QUAKE                 = 2,
    EVENT_CHAINS_OF_WOE         = 3,
    EVENT_SKULLCRACKER          = 4
};
 
class boss_romogg_bonecrusher : public CreatureScript
{
    public:
        boss_romogg_bonecrusher() : CreatureScript("boss_romogg_bonecrusher") {}
     
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_romogg_bonecrusherAI (pCreature);
        }
     
        struct boss_romogg_bonecrusherAI : public ScriptedAI
        {
            boss_romogg_bonecrusherAI(Creature* c) : ScriptedAI(c), summons(me)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);   
                instance = (InstanceScript*)c->GetInstanceScript();
            }
     
            InstanceScript* instance;
            EventMap events;
            SummonList summons;
            uint8 stage;

            void Reset() override
            {
                instance->DoRemoveAurasDueToSpellOnPlayers(76575);

                stage = 0;
                events.Reset();
                summons.DespawnAll();
                if (instance)
                    instance->SetData(DATA_ROMOGG, NOT_STARTED);
            }
     
            void DespawnAllSummons()
            {
                std::list<Creature*> list;
                list.clear();
                me->GetCreatureListWithEntryInGrid(list, 40447, 200.0f);
                if (!list.empty())
                    for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                        (*itr)->DespawnOrUnsummon();
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                switch (summon->GetEntry())
                {
                case NPC_ANGERED_EARTH:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        summon->AddThreat(target, 10.0f);
                        summon->Attack(target, true);
                        summon->GetMotionMaster()->MoveChase(target);
                    }
                    break;
                }        
            }

            void EnterCombat(Unit* /*who*/) override
            {
                DoCast(me, SPELL_CALL_FOR_HELP);
                Talk(SAY_AGGRO);
                events.RescheduleEvent(EVENT_WOUNDING_STRIKE, urand(5000, 7000));
                events.RescheduleEvent(EVENT_QUAKE, urand(18000, 20000));
                events.RescheduleEvent(EVENT_CHAINS_OF_WOE, urand(22000, 32000));
                DoZoneInCombat();
                if (instance)
                    instance->SetData(DATA_ROMOGG, IN_PROGRESS);
                
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
                    case EVENT_WOUNDING_STRIKE:
                        DoCast(me->getVictim(), SPELL_WOUNDING_STRIKE);
                        events.RescheduleEvent(EVENT_WOUNDING_STRIKE, urand(6000, 7000));
                        break;
                    case EVENT_QUAKE:
                        DoCast(me, SPELL_QUAKE);
                        if (IsHeroic())
                        {
                            Map::PlayerList const& players = me->GetMap()->GetPlayers();
                            if (me->GetMap()->IsDungeon() && !players.isEmpty())
                            {
                                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                {
                                    Player* pPlayer = itr->getSource();
                                     me->SummonCreature(NPC_ANGERED_EARTH, pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
                                }
                              }  
                        }
                        events.RescheduleEvent(EVENT_QUAKE, urand(18000, 20000));
                        break;
                    case EVENT_CHAINS_OF_WOE:
                        Talk(SAY_CHAINS);
                        DoCast(me, SPELL_CHAINS_OF_WOE);
                        events.RescheduleEvent(EVENT_CHAINS_OF_WOE, urand(22000, 32000));
                        events.RescheduleEvent(EVENT_SKULLCRACKER, 3000);
                        break;
                    case EVENT_SKULLCRACKER:
                        DoCast(me, SPELL_SKULLCRACKER);
                        break;
                    }
                }
                DoMeleeAttackIfReady();  
            }
     
            void SpellHit(Unit* /*owner*/, SpellInfo const* spell) override
            {
                switch (spell->Id)
                {
                case SPELL_SKULLCRACKER:
                    DespawnAllSummons();
                    break;
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
                summons.DespawnAll();
                if (instance)
                    instance->SetData(DATA_ROMOGG, DONE);
            }
     
            void KilledUnit(Unit* victim) override
            {
                Talk(SAY_KILL);
            }
        };
};
 
class npc_chains_of_woe : public CreatureScript
{
    public:
        npc_chains_of_woe() : CreatureScript("npc_chains_of_woe") {}
     
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_chains_of_woeAI (pCreature);
        }
     
        struct npc_chains_of_woeAI : public ScriptedAI
        {
            npc_chains_of_woeAI(Creature *c) : ScriptedAI(c) 
            {
                instance = c->GetInstanceScript();
            }
     
            InstanceScript* instance;

            void Reset() override
            {
                me->SetDisplayId(38330);
                me->AddUnitState(UNIT_STATE_ROOT);
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_CHAINS_OF_WOE_TELE);
                DoCast(me, SPELL_CHAINS_OF_WOE_0);
            }
        };
};

class spell_romoogg_chains_of_woe_root : public SpellScript
{
    PrepareSpellScript(spell_romoogg_chains_of_woe_root);

    void HandleScript(SpellEffIndex effIndex)
    {
        if (GetHitUnit())
            GetHitUnit()->CastSpell(GetHitUnit(), 82192);
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_romoogg_chains_of_woe_root::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_romoogg_chains_of_woe_tele_root : public SpellScript
{
    PrepareSpellScript(spell_romoogg_chains_of_woe_tele_root);

    void HandleScript(SpellEffIndex effIndex)
    {
        if (GetHitUnit())
        {
            if (!GetHitUnit()->IsPlayer())
                return;

            GetHitUnit()->ToPlayer()->TeleportTo(645,
                GetCaster()->GetPositionX(),
                GetCaster()->GetPositionY(),
                GetCaster()->GetPositionZ(),
                0.0f);
        }
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_romoogg_chains_of_woe_tele_root::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_boss_romogg_bonecrusher()
{
    new boss_romogg_bonecrusher();
    new npc_chains_of_woe();
    RegisterSpellScript(spell_romoogg_chains_of_woe_root);
    RegisterSpellScript(spell_romoogg_chains_of_woe_tele_root);
}
