#include "AchievementMgr.h"
#include "BrawlersGuild.h"

enum eSpells
{
    //! 3 rank
    
    //splat
    SPELL_SPLIT                 = 141034,
    
    // shadowmaster
    SPELL_SHADOW_TORCH          = 232504, // 6
    SPELL_SHADOW_BLAST          = 232502, // 6
    SPELL_SHADOW_DETONATION_DMG = 232514,
    SPELL_SHADOW_DETONATION_TRIG = 232512,
    
    // johnny
    SPELL_SUMMON_PET            = 229065,
    SPELL_SHOOT                 = 205689, // 2
    SPELL_VOLLEY                = 229137, // 6-8
    SPELL_REVIVE_PET            = 229111,
    SPELL_POWERSHOOT_FIX        = 229123,
    SPELL_POWERSHOOT            = 229124,
    SPELL_POWERSHOOT_PLR        = 229127,
    SPELL_POWERSHOOT_NPC        = 229125,      
};

// 117145
class boss_brawguild_doomflipper : public CreatureScript
{
public:
    boss_brawguild_doomflipper() : CreatureScript("boss_brawguild_doomflipper") {}

    struct boss_brawguild_doomflipperAI : public BrawlersBossAI
    {
        boss_brawguild_doomflipperAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void UpdateAI(uint32 diff) override
        {            
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_doomflipperAI (creature);
    }
};

// 70736, 70737
class boss_brawguild_splat : public CreatureScript
{
public:
    boss_brawguild_splat() : CreatureScript("boss_brawguild_splat") {}

    struct boss_brawguild_splatAI : public BrawlersBossAI
    {
        boss_brawguild_splatAI(Creature* creature) : BrawlersBossAI(creature) {}

        
        void Reset() override
        {           
            summons.DespawnAll();
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 4000);
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->SetHealth(me->GetHealth());
            summon->AI()->AttackStart(me->getVictim());
        }
               
        void JustDied(Unit* who) override
        {
            if (me->GetEntry() != 70736)
                return;
            
			_Reset();
            
            if (!who)
                return;
            
            _WinRound();
        }
        
        void EnterEvadeMode() override
        {
            if (me->GetEntry() != 70736)
                return;
            
            _Reset();
            _LoseRound();
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
                    case EVENT_1:
                        if(me->GetEntry() == 70736)
                            DoCast(SPELL_SPLIT);
                        me->SetObjectScale(me->GetHealthPct() * 0.006 + 0.4);
                        events.RescheduleEvent(EVENT_1, 4000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_splatAI (creature);
    }
};

// 116743
// torch 116747    ball 116746
class boss_brawguild_shadowmaster : public CreatureScript
{
public:
    boss_brawguild_shadowmaster() : CreatureScript("boss_brawguild_shadowmaster") {}

    struct boss_brawguild_shadowmasterAI : public BrawlersBossAI
    {
        boss_brawguild_shadowmasterAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {           
            summons.DespawnAll();
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(0);
            events.RescheduleEvent(EVENT_1, 2000);
            events.RescheduleEvent(EVENT_2, 2000+6000);
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (summon->GetEntry() == 116746)
                if (Creature* last_torch = me->GetMap()->GetCreature(summon->GetTargetGUID()))
                {
                    // summon->CastSpell(last_torch, 232509);
                    summon->GetMotionMaster()->MovePoint(1, last_torch->GetPositionX(), last_torch->GetPositionY(), last_torch->GetPositionZ());
                    // summon->DespawnOrUnsummon(3000);
                }
        }
               
        void JustDied(Unit* who) override
        {
            summons.KillAll();
            
            if (!who)
                return;
            
            _WinRound();
        }
        
        void EnterEvadeMode() override
        {
            summons.KillAll();
            _LoseRound();
        }
        
        void KilledUnit(Unit* who) override
        {
            if (Unit* owner = me->GetAnyOwner())
                if (who->GetGUID() == owner->GetGUID())
                {
                    summons.KillAll();
                    _LoseRound();
                }
        }
        
        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_SHADOW_TORCH)
                me->SummonCreature(116747, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation());
            
            if (spell->Id == 232509)
            {
                for (auto guid : summons)
                    if (Creature* last_torch = me->GetMap()->GetCreature(guid))
                        if (last_torch->GetEntry() == 116747 && last_torch->GetGUID() != target->GetGUID())
                            target->CastSpell(last_torch, SPELL_SHADOW_BLAST, true);
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
                    case EVENT_1:
                        DoCast(SPELL_SHADOW_TORCH);
                        events.RescheduleEvent(EVENT_1, 6000);
                        break;
                    case EVENT_2:
                        if (Creature* last_torch = ObjectAccessor::FindCreature(summons.back()))
                            me->CastSpell(last_torch, SPELL_SHADOW_BLAST);
                        events.RescheduleEvent(EVENT_2, 6000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_shadowmasterAI (creature);
    }
};


// 116747, 116746
class npc_brawguild_shadowmaster_adds : public CreatureScript
{
public:
    npc_brawguild_shadowmaster_adds() : CreatureScript("npc_brawguild_shadowmaster_adds") { }

    struct npc_brawguild_shadowmaster_addsAI : public ScriptedAI
    {
        npc_brawguild_shadowmaster_addsAI(Creature* creature) : ScriptedAI(creature), summons(me)  {}
        
        SummonList summons;
        
        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
            if (me->GetEntry() == 116746)
                DoCast(232503);
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (summon->GetEntry() == 116746)
                if (Creature* last_torch = me->GetMap()->GetCreature(summon->GetTargetGUID()))
                {
                    // summon->CastSpell(last_torch, 232509);
                    summon->GetMotionMaster()->MovePoint(1, last_torch->GetPositionX(), last_torch->GetPositionY(), last_torch->GetPositionZ());
                    // summon->DespawnOrUnsummon(3000);
                }
        }
        
        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
        }
        
        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (Unit* owner = me->GetAnyOwner())
                    if (Creature* last_torch = me->GetMap()->GetCreature(me->GetTargetGUID()))
                        owner->ToCreature()->AI()->SpellHitTarget(last_torch, sSpellMgr->GetSpellInfo(232509));
                me->DespawnOrUnsummon();
            }
        }

        void SpellHit(Unit* /*caster */, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_SHADOW_DETONATION_TRIG)
                DoCast(SPELL_SHADOW_DETONATION_DMG);
        }
              
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_brawguild_shadowmaster_addsAI(creature);
    }
};

// 115058
class boss_brawguild_johnny : public CreatureScript
{
public:
    boss_brawguild_johnny() : CreatureScript("boss_brawguild_johnny") {}

    struct boss_brawguild_johnnyAI : public BrawlersBossAI
    {
        boss_brawguild_johnnyAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {         
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);        
            summons.DespawnAll();
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            DoCast(SPELL_SUMMON_PET);
            events.RescheduleEvent(EVENT_1, urand(6000, 7000)); 
            events.RescheduleEvent(EVENT_2, urand(9000, 12000));
            events.RescheduleEvent(EVENT_4, 2000); 
        }
               
        void JustDied(Unit* who) override
        {
			_Reset();
            
            if (!who)
                return;
            Talk(0);
            
            _WinRound();
        }

        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_POWERSHOOT_NPC)
            {
                events.Reset();
                events.RescheduleEvent(EVENT_3, 500);
            }
            
            if (spell->Id == SPELL_REVIVE_PET)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                if (target->IsCreature())
                {
                    target->ToCreature()->Respawn(false);
                    if (Unit* owner = me->GetAnyOwner())
                        target->ToCreature()->AI()->AttackStart(owner);
                }
                events.RescheduleEvent(EVENT_1, urand(6000, 7000)); 
                events.RescheduleEvent(EVENT_2, urand(9000, 12000));
                events.RescheduleEvent(EVENT_4, 2000); 
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
                    case EVENT_1:
                        DoCastVictim(SPELL_VOLLEY);
                        events.RescheduleEvent(EVENT_1, urand(6000, 7000));
                        break;
                    case EVENT_2:
                        DoCastVictim(SPELL_POWERSHOOT_FIX);
                        DoCastVictim(SPELL_POWERSHOOT);
                        events.RescheduleEvent(EVENT_4, 2002);
                        break;
                    case EVENT_3:
                        me->SetReactState(REACT_PASSIVE);
                        if (Creature* pet = me->GetMap()->GetCreature(summons.back()))
                        {
                            me->CastSpell(me, 229143);
                            me->CastSpell(pet, SPELL_REVIVE_PET);
                        }
                        else
                        {
                            DoCast(SPELL_SUMMON_PET);
                            me->SetReactState(REACT_AGGRESSIVE);
                            events.RescheduleEvent(EVENT_1, urand(6000, 7000)); 
                            events.RescheduleEvent(EVENT_2, urand(9000, 12000));
                            events.RescheduleEvent(EVENT_4, 2000); 
                        }
                        break;                            
                    case EVENT_4:
                        DoCast(SPELL_SHOOT);
                        events.RescheduleEvent(EVENT_4, 2002);
                        break;
                }
            }
            // DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_johnnyAI (creature);
    }
};

// 229124
class spell_brawling_powershot : public SpellScriptLoader
{
    public:
        spell_brawling_powershot() : SpellScriptLoader("spell_brawling_powershot") { }

        class spell_brawling_powershot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_brawling_powershot_SpellScript);
            
            void HandleOnHit()
            {
                if (!GetCaster())
                    return;
                
                Creature* caster = GetCaster()->ToCreature();
                if (!caster)
                    return;

                Unit* owner = caster->GetAnyOwner();
                if (!owner)
                    return;

                Player* pPlayer = owner->ToPlayer();
                if (!pPlayer)
                    return;
                
                 WorldObject* objTarget = nullptr;
                 
                if (Creature* target = pPlayer->FindNearestCreature(115085, 100.0f, true))
                {
                    if (target->IsInBetween(caster, pPlayer, 3.0f))
                            objTarget = target;
                        else
                            objTarget = pPlayer;
                }
                else
                    objTarget = pPlayer;

                if (objTarget->GetTypeId() != TYPEID_PLAYER)
                    GetCaster()->CastSpell(objTarget->ToCreature(), SPELL_POWERSHOOT_NPC, true);
                else
                    GetCaster()->CastSpell(objTarget->ToPlayer(), SPELL_POWERSHOOT_PLR, true);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_brawling_powershot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_brawling_powershot_SpellScript();
        }
};


void AddSC_brawlers_guild_bosses_rank_three()
{
    new boss_brawguild_doomflipper();
    new boss_brawguild_splat();
    new boss_brawguild_shadowmaster();
    new npc_brawguild_shadowmaster_adds();
    new boss_brawguild_johnny();
    new spell_brawling_powershot();
};