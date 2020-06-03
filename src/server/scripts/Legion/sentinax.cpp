#include "sentinax.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "OutdoorPvPMgr.h"
#include "GameEventMgr.h"

class spell_sentinax_call_portal : public SpellScriptLoader
{
public:
    spell_sentinax_call_portal() : SpellScriptLoader("spell_sentinax_call_portal") { }

    class spell_sentinax_call_portal_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_sentinax_call_portal_SpellScript);
        
        SpellCastResult CheckRequirement()
        {
            Unit* caster = GetCaster();
            if (!caster)
                return SPELL_FAILED_BAD_TARGETS;
            Player* player = caster->ToPlayer();
            if (!player)
                return SPELL_FAILED_BAD_TARGETS;
            
            if (sGameEventMgr->IsActiveEvent(153))
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            
            if (ZoneScript* zone_script = sOutdoorPvPMgr->GetZoneScript(player->GetCurrentZoneID()))
            {
                if (!zone_script->GetData(true)) // limit of portals
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            }
            
            
            if (!player->FindNearestGameObject(GO_SENTINAX, DIST_SENTINAX/ 2)) // not on area sentinax
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                
            std::list<Creature*> targets;
            GetCreatureListWithEntryInGrid(targets, player, NPC_TARGET_PORTAL, 50.0f);
            
            if (targets.empty()) //! send about wrong area ?
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                
            return SPELL_CAST_OK;
        }


        void HandleOnCast()
        {
            Unit* caster = GetCaster();
            if (!caster)
                return;
            Player* player = caster->ToPlayer();
            if (!player)
                return;
                
            std::list<Creature*> targets;
            GetCreatureListWithEntryInGrid(targets, player, NPC_TARGET_PORTAL, 50.0f);
            
            if (targets.empty()) //! send about wrong area ?
                return; 
            
            Creature* owner = player->SummonCreature(NPC_OWNER_PORTALS);
            if (!owner)
                return;
            
            uint32 find_Spell = portalForEmpowered[GetSpellInfo()->Id];
            if (find_Spell != 0) // if empowered
            {
                uint32 portalsUsed = 0;
                for (std::list<Creature*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                {
                    if((*itr)->HasAura(SPELL_NOT_FREE_PORTAL) && (*itr)->AI()->GetData(true) == find_Spell && (*itr)->AI()->GetData(false) == 0)
                    {
                        (*itr)->AI()->SetData(GetSpellInfo()->Id, 1);
                        ++portalsUsed;
                    }
                }
                
                if (portalsUsed)
                    if (Item* item = GetCastItem())
                        player->DestroyItemCount(item->GetEntry(), 1, true);
                return;
            }
            
            uint8 max_portals = GetMaxPortals(GetSpellInfo()->Id);
            
            for (uint8 i = 0; i < max_portals; ++i) // we need to do it 2-3 times
            {
                Creature* target = nullptr;
                
                std::list<Creature*>::const_iterator itr = targets.begin();
                std::advance(itr, urand(0, targets.size() - 1)); // try to find it randomly
                
                if (*itr && !(*itr)->HasAura(SPELL_NOT_FREE_PORTAL))
                    target = *itr;
                else
                { // bruteforce 
                    for (std::list<Creature*>::iterator itr2 = targets.begin(); itr2 != targets.end(); ++itr2)
                    {
                        if ((*itr2)->HasAura(SPELL_NOT_FREE_PORTAL))
                            continue;
                        
                        target = *itr2;
                        break;
                    }
                }
                
                if (target == nullptr)
                {
                    if (i == 0)
                    {
                        GetSpell()->SendCastResult(SPELL_FAILED_NOT_READY);
                        return;
                    }
                    else
                        break; // we just summoned 1 portal and we need delete item
                }
                
                if (i == 0) // only first time we send laser
                    player->CastSpell(target, SPELL_SUMMON_LASER, true);  // we use target for moving laser's trigger
                    
               
                owner->AI()->JustSummoned(target); // we need to combine group of portals
                target->AddAura(SPELL_NOT_FREE_PORTAL, target);                
                target->AI()->DoAction(GetSpellInfo()->Id); // send info about portal and adds and start event                
            }
            
            //! delete item after cast
            if (Item* item = GetCastItem())
                player->DestroyItemCount(item->GetEntry(), 1, true);
            
            // to-do emotions
        }

        uint8 GetMaxPortals(uint32 spell)
        {
            switch(spell)
            {
                case TORMENT_PORTAL:
                case ENGINEERING_PORTAL:
                case WARBEAST_PORTAL:
                case CARNAGE_PORTAL:
                case FIRESTORM_PORTAL:
                case DOMINATION_PORTAL:
                    return urand(2, 3);
                case GREATER_TORMENT_PORTAL:
                case GREATER_ENGINEERING_PORTAL:
                case GREATER_WARBEAST_PORTAL:
                case GREATER_CARHAGE_PORTAL:
                case GREATER_FIRESTORM_PORTAL:
                case GREATER_DOMINATION_PORTAL:
                    return urand(1, 2);
                case BOSS_TORMENT_PORTAl:
                case BOSS_WARBEAST_PORTAL:
                case BOSS_CARNAGE_PORTAL:
                case BOSS_DOMITAION_PORTAL:
                case BOSS_FIRESTORM_PORTAL:
                case BOSS_ENGINEERING_PORTAL:
                    return 1;
            }
            
            return 0;
            
        }
        
        
        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_sentinax_call_portal_SpellScript::CheckRequirement);
            OnCast += SpellCastFn(spell_sentinax_call_portal_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_sentinax_call_portal_SpellScript();
    }
};


// 950010
class npc_sentinax_portal_helper : public CreatureScript
{
public:
    npc_sentinax_portal_helper() : CreatureScript("npc_sentinax_portal_helper") { }

    struct npc_sentinax_portal_helperAI : public ScriptedAI
    {
        npc_sentinax_portal_helperAI(Creature* creature) : ScriptedAI(creature), summons(me), _action(0), secondAction(0)
        {
            me->SetReactState(REACT_PASSIVE);
            portal_guid.Clear();
            second_wave = false;
        }

        ObjectGuid portal_guid;
        SummonList summons;
        bool second_wave;
        int32 _action, secondAction;

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->GetMotionMaster()->MovePoint(0, me->GetPositionX() + irand(-5, 5), me->GetPositionY() + irand(-5, 5), me->GetPositionZ());
        }
        
        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            summons.Despawn(summon);
            
            if (summons.empty())
            {
                if (second_wave) // reset state
                {
                     me->AddDelayedEvent(10000, [this] () -> void {
                        summons.DespawnAll();
                     });
                     
                    if (GameObject* go = Unit::GetGameObjectOnMap(*me, portal_guid))
                        go->Delete();
                    portal_guid.Clear();
                    second_wave = false;
                    me->RemoveAura(SPELL_NOT_FREE_PORTAL);
                    
                    if (me->GetAnyOwner())
                        me->GetAnyOwner()->ToCreature()->AI()->DoAction(1); // for remove all portals by group
                    
                    secondAction = 0;
                    return;
                }
                
                // start second wave
                second_wave = true;
                
                for (int32 id : {_action, secondAction})
                    me->AddDelayedEvent(6000, [this, id]() -> void
                    {
                        secondWave(id);
                    });
            }
        }
        
        uint32 GetData(uint32 id) const override
        {
            if (id)
                return _action;    
            else
                return secondAction;
        }
        
        void SetData(uint32 spell, uint32 id) override
        {
            secondAction = spell;
            
            firstWave(secondAction);
            
            if (second_wave)
                secondWave(secondAction);
        }
        
        void DoAction(int32 const action) override
        {
            secondAction = 0;
            if (action == 1)
            {
                me->KillAllDelayedEvents();
                me->AddDelayedEvent(10000, [this] () -> void {
                  summons.DespawnAll();
                });
                
                if (GameObject* go = Unit::GetGameObjectOnMap(*me, portal_guid))
                    go->Delete();
                portal_guid.Clear();
                second_wave = true;
                me->RemoveAura(SPELL_NOT_FREE_PORTAL);
                
                if (ZoneScript* zone_script = sOutdoorPvPMgr->GetZoneScript(me->GetCurrentZoneID()))
                    zone_script->SetData(0, 1);
                return;
            }
            
            _action = action;
            second_wave = false;
            
            if (ZoneScript* zone_script = sOutdoorPvPMgr->GetZoneScript(me->GetCurrentZoneID()))
            {
                switch (action)
                {
                    case BOSS_TORMENT_PORTAl:
                    case BOSS_WARBEAST_PORTAL:
                    case BOSS_CARNAGE_PORTAL:
                    case BOSS_DOMITAION_PORTAL:
                    case BOSS_FIRESTORM_PORTAL:
                    case BOSS_ENGINEERING_PORTAL:
                        zone_script->SetData(2, 1);
                        break;
                    default:
                        zone_script->SetData(1, 1);
                        break;
                    
                }
            }
            
            me->AddDelayedEvent(3000, [this, action]() -> void
            {
                firstWave(action);
                if (GameObject* go = me->SummonGameObject(GOB_PORTAL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 0))
                {
                    go->SetLootState(GO_READY);
                    // go->UseDoorOrButton(10000, false);
                    portal_guid = go->GetGUID();
                }
            });
        }

        void secondWave(int32 action)
        {
            switch (action)
            {
            case GREATER_TORMENT_PORTAL:            // uncommon
                me->SummonCreature(NPC_WARDEN, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case TORMENT_PORTAL:
                for (uint8 i = 0; i< urand(2, 3); ++i)
                    me->SummonCreature(Npcs[urand(0, 1)], TEMPSUMMON_DEAD_DESPAWN);
                for (uint8 i = 0; i< urand(1, 2); ++i)
                    me->SummonCreature(Npcs[2], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case GREATER_ENGINEERING_PORTAL:        // uncommon
                me->SummonCreature(NPC_MOARG, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case ENGINEERING_PORTAL:
                for (uint8 i = 0; i < urand(2, 3); ++i)
                    me->SummonCreature(Npcs[urand(3, 4)], TEMPSUMMON_DEAD_DESPAWN);
                for (uint8 i = 0; i< urand(1, 2); ++i)
                    me->SummonCreature(Npcs[5], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case GREATER_WARBEAST_PORTAL:           // uncommon
                me->SummonCreature(Npcs[urand(20, 21)], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case WARBEAST_PORTAL:
                for (uint8 i = 0; i < urand(4, 7); ++i)
                    me->SummonCreature(Npcs[urand(6, 8)], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case GREATER_CARHAGE_PORTAL:            // uncommon
                me->SummonCreature(NPC_FIRECALLER, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case CARNAGE_PORTAL:
                for (uint8 i = 0; i < urand(4, 6); ++i)
                    me->SummonCreature(Npcs[urand(9, 11)], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case GREATER_FIRESTORM_PORTAL:          // uncommon
                me->SummonCreature(NPC_VILE_MOTHER, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case FIRESTORM_PORTAL:
                for (uint8 i = 0; i < urand(6, 8); ++i)
                    me->SummonCreature(Npcs[urand(12, 14)], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case GREATER_DOMINATION_PORTAL:         // uncommon
            case DOMINATION_PORTAL:
                for (uint8 i = 0; i < urand(6, 8); ++i)
                    me->SummonCreature(Npcs[urand(15, 17)], TEMPSUMMON_DEAD_DESPAWN);
                break;
            }

        };

        void firstWave(int32 action)
        {
            switch (action)
            {
            case GREATER_TORMENT_PORTAL:            // uncommon
                for (uint8 i = 0; i < 2; ++i)
                    me->SummonCreature(NPC_WARDEN, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case BOSS_TORMENT_PORTAl:              // boss
                me->SummonCreature(NPC_IILLISTHYNDRIA, TEMPSUMMON_DEAD_DESPAWN);
                second_wave = true; // only one wave
                // no break;
            case TORMENT_PORTAL:
                for (uint8 i = 0; i< urand(3, 5); ++i)
                    me->SummonCreature(Npcs[urand(0, 1)], TEMPSUMMON_DEAD_DESPAWN);
                break;

            case GREATER_ENGINEERING_PORTAL:        // uncommon
                for (uint8 i = 0; i < 2; ++i)
                    me->SummonCreature(NPC_MOARG, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case BOSS_ENGINEERING_PORTAL:           // boss
                me->SummonCreature(NPC_OBLITERATOR, TEMPSUMMON_DEAD_DESPAWN);
                second_wave = true;
                // no break; 
            case ENGINEERING_PORTAL:
                for (uint8 i = 0; i < urand(2, 3); ++i)
                    me->SummonCreature(Npcs[urand(3, 4)], TEMPSUMMON_DEAD_DESPAWN);
                break;


            case GREATER_WARBEAST_PORTAL:           // uncommon
                for (uint8 i = 0; i < urand(3, 4); ++i)
                    me->SummonCreature(Npcs[urand(20, 21)], TEMPSUMMON_DEAD_DESPAWN);
                break;
            case BOSS_WARBEAST_PORTAL:              // boss
                me->SummonCreature(NPC_ANTHYNA, TEMPSUMMON_DEAD_DESPAWN);
                second_wave = true;
                // no break
            case WARBEAST_PORTAL:
                for (uint8 i = 0; i < urand(4, 6); ++i)
                    me->SummonCreature(Npcs[urand(6, 8)], TEMPSUMMON_DEAD_DESPAWN);
                break;


            case GREATER_CARHAGE_PORTAL:            // uncommon
                for (uint8 i = 0; i < 3; ++i)
                    me->SummonCreature(NPC_FIRECALLER, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case BOSS_CARNAGE_PORTAL:               // boss
                me->SummonCreature(NPC_XILLIOUS, TEMPSUMMON_DEAD_DESPAWN);
                second_wave = true;   // only one wave
                // no break;
            case CARNAGE_PORTAL:
                for (uint8 i = 0; i < urand(4, 5); ++i)
                    me->SummonCreature(Npcs[urand(9, 11)], TEMPSUMMON_DEAD_DESPAWN);
                break;

            case GREATER_FIRESTORM_PORTAL:            // uncommon
                for (uint8 i = 0; i < 2; ++i)
                    me->SummonCreature(NPC_INFERNAL, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case BOSS_FIRESTORM_PORTAL:               // boss
                me->SummonCreature(NPC_SKULGULOTH, TEMPSUMMON_DEAD_DESPAWN);
                second_wave = true;
                // no break
            case FIRESTORM_PORTAL:
                for (uint8 i = 0; i < urand(5, 7); ++i)
                    me->SummonCreature(Npcs[urand(12, 14)], TEMPSUMMON_DEAD_DESPAWN);
                break;


            case GREATER_DOMINATION_PORTAL:          // uncommon
                me->SummonCreature(NPC_DRAINING_EYE, TEMPSUMMON_DEAD_DESPAWN);
                break;
            case BOSS_DOMITAION_PORTAL:              // boss    
                me->SummonCreature(NPC_THANOTALION, TEMPSUMMON_DEAD_DESPAWN);
                second_wave = true;   // only one wave
                // no break
            case DOMINATION_PORTAL:
                for (uint8 i = 0; i < urand(4, 6); ++i)
                    me->SummonCreature(Npcs[urand(15, 17)], TEMPSUMMON_DEAD_DESPAWN);
                break;

            }
        };
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sentinax_portal_helperAI(creature);
    }
};

// 121084
class npc_sentinax_laser : public CreatureScript
{
public:
    npc_sentinax_laser() : CreatureScript("npc_sentinax_laser") { }

    struct npc_sentinax_laserAI : public ScriptedAI
    {
        npc_sentinax_laserAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }
        
        void IsSummonedBy(Unit* owner) override
        {
            if (Unit* target = me->GetTargetUnit())
                me->GetMotionMaster()->MovePoint(1, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            else
            {
                me->DespawnOrUnsummon();
                return;
            }
            
            if (Creature* sent = me->FindNearestCreature(NPC_SENTINAX, DIST_SENTINAX, true))
                me->CastSpell(sent, SPELL_GREEN_BEAM);
        }
        
        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == 1)
                me->GetMotionMaster()->MoveCirclePath(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 3.0f, true, 6);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sentinax_laserAI(creature);
    }
};

//  950011
class npc_owner_portal : public CreatureScript
{
public:
    npc_owner_portal() : CreatureScript("npc_owner_portal") { }

    struct npc_owner_portalAI : public ScriptedAI
    {
        npc_owner_portalAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            me->SetReactState(REACT_PASSIVE);
        }
        
        SummonList summons;
        
        void JustSummoned(Creature* summon) override
        {
            if (!summon)
                return;
            
            summons.Summon(summon);
            summon->SetCharmerGUID(me->GetGUID());
        }
   
        void DoAction(int32 const action) override
        {
            DummyEntryCheckPredicate pred;
            summons.DoAction(1, pred);
            summons.clear();
            me->DespawnOrUnsummon(3000);
        }

   };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_owner_portalAI(creature);
    }
};


// 950012
class npc_sentinax : public CreatureScript
{
public:
    npc_sentinax() : CreatureScript("npc_sentinax") { }

    struct npc_sentinaxAI : public ScriptedAI
    {
        npc_sentinaxAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            events.Reset();
            events.RescheduleEvent(EVENT_1, 65000);
        }
        
        EventMap events;
        
   
        void UpdateAI(uint32 diff) override
        {                       
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                    
                        std::list<Creature*> targets;
                        GetCreatureListWithEntryInGrid(targets, me, NPC_TARGET_LASER, DIST_SENTINAX);
                        
                        if (targets.empty())
                        {
                            events.RescheduleEvent(EVENT_1, 15000);
                            break; 
                        }
                        
                        std::list<Creature*>::const_iterator itr = targets.begin();
                        std::advance(itr, urand(0, targets.size() - 1)); 
                        
                        if (*itr)
                            me->CastSpell(*itr, SPELL_LASER_DMG);
                        
                        events.RescheduleEvent(EVENT_1, 180000);
                }
            }
        }

   };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sentinaxAI(creature);
    }
};

class OutdoorPvP_Sentinax : public OutdoorPvPScript
{
    public:

        OutdoorPvP_Sentinax()
            : OutdoorPvPScript("outdoorpvp_sentinax")
        {
        }

        OutdoorPvP* GetOutdoorPvP() const override
        {
            return new OutdoorPVPSentinax();
        }
};

void AddSC_sentinax()
{
    new spell_sentinax_call_portal();
    new npc_sentinax_portal_helper();
    new npc_sentinax_laser();
    new npc_owner_portal();
    new npc_sentinax();
    new OutdoorPvP_Sentinax();
};
