#include "AchievementMgr.h"
#include "BrawlersGuild.h"

enum eSpells
{
    //! 4 rank
    
    // burnstachio
    SPELL_WATER_ORB             = 228977, 
    SPELL_BURNING_GROUND        = 228910, // at    
    
    // meatball
    SPELL_MEATBALL_MAD          = 134911,
    SPELL_OUTRO                 = 134864,
    SPELL_OUTRO_1               = 134914,
    
    // GG
    // 67488
    SPELL_GOBLIN_ROCKET         = 133212, // 8
    
    // 67487
    SPELL_DEATH_RAY             = 133172, // 4?
    
    // both
    SPELL_EMERGENCY_TELEPORT    = 133161, // try to find save area ?   (33)
    SPELL_SHARE_DMG             = 133304,
    
    SPELL_READY_TO_ACTIVATE     = 133136, 
    SPELL_ACTIVATE_TOTEM        = 133137,
    SPELL_SHIELD                = 133249,
    
    // stitches
    SPELL_AURA_OF_ROT           = 236152,
    SPELL_AURA_OF_ROT_TICK      = 236154,
    SPELL_AURA_OF_ROT_DMG       = 236155,
    SPELL_STITCHES_HOOK         = 236159,
};

float const coordinates[8]
{
    -97.12f, // x AL
    2515.84f,  // y AL
    
    -142.58f, // x AL
    2482.71f, // y AL
    
    2050.20f, // x H
    -4731.81f, // y H
    
    2011.34f, // x H
    -4773.83f // y H
};

uint32 wp_rain_A[4]
{
    11854514,
    11854515,
    11854516,
    11854517    
};

uint32 wp_rain_H[4]
{
    11854518,
    11854519,
    11854520,
    11854521
};

// 115023
// + 114976 (rain) 114975 (burn)
class boss_brawguild_burnstachio : public CreatureScript
{
public:
    boss_brawguild_burnstachio() : CreatureScript("boss_brawguild_burnstachio") {}

    struct boss_brawguild_burnstachioAI : public BrawlersBossAI
    {
        boss_brawguild_burnstachioAI(Creature* creature) : BrawlersBossAI(creature)
        {
            me->setRegeneratingHealth(false);
        }
        
        uint8 count_sum;
        
        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);      
            events.Reset();
            if (me->isAlive() && me->GetAnyOwner())
                AttackStart(me->GetAnyOwner());
        }
        
        void EnterCombat(Unit* who) override
        {
            if (Unit* unit = me->GetAnyOwner())
            if (Player* player = unit->ToPlayer())
            {
                count_sum = 0;
                if (player->GetTeamId() == TEAM_ALLIANCE)
                { 
                    me->SummonCreature(114976, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 6.25f);
                    me->SummonCreature(114976, -131.83f, 2492.57f, -49.10f, 6.25f);
                    me->SummonCreature(114976, -108.83f, 2492.57f, -49.10f, 6.25f);
                    me->SummonCreature(114976, -108.83f, 2507.57f, -49.10f, 6.25f);
                    for (float y = coordinates[1]; y > coordinates[3]; y -= 4.0f)
                        for (float x = coordinates[0]; x > coordinates[2]; x -= 4.0f)
                            me->SummonCreature(114975, x, y, me->GetPositionZ(), 0.0f);
                }
                else
                {                        
                    me->SummonCreature(114976, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 6.25f);
                    me->SummonCreature(114976, 2023.94f, -4744.51f, 88.77f, 4.66f);
                    me->SummonCreature(114976, 2023.94f, -4763.51f, 88.77f, 4.66f);
                    me->SummonCreature(114976, 2042.94f, -4763.51f, 88.77f, 4.66f);
                    
                    for (float x = coordinates[4]; x >= coordinates[6]; x -= 4.0f)
                        for (float y = coordinates[5]; y >= coordinates[7]; y -= 4.0f)
                            me->SummonCreature(114975, x, y, me->GetPositionZ(), 0.0f);
                }
            }
            
            events.RescheduleEvent(EVENT_1, 8000); 
        }
        
        void JustDied(Unit* who) override
        {
			me->RemoveAllAreaObjects();
            summons.KillAll();
                                
            if (!who)
                return;
            
            _WinRound();
        }    
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (summon->GetEntry() == 114976)
            {
                if (me->GetAnyOwner() && me->GetAnyOwner()->IsPlayer())
                    if (Player* player = me->GetAnyOwner()->ToPlayer())
                        summon->GetMotionMaster()->MovePath(player->GetTeamId() == TEAM_ALLIANCE ? wp_rain_A[count_sum] :  wp_rain_H[count_sum], true);
                count_sum++;
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
                    Position pos;
                    me->GetNearPoint2D(pos, 15.0f, frand(-M_PI/3, M_PI/3));
                    me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_WATER_ORB);
                    events.RescheduleEvent(EVENT_1, urand (17000, 19000));
                    break;
                }
            }

            // DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_burnstachioAI (creature);
    }
};

// 114975, 114976
class npc_brawguild_burnstachio_ground : public CreatureScript
{
public:
    npc_brawguild_burnstachio_ground() : CreatureScript("npc_brawguild_burnstachio_ground") { }

    struct npc_brawguild_burnstachio_groundAI : public ScriptedAI
    {
        npc_brawguild_burnstachio_groundAI(Creature* creature) : ScriptedAI(creature) {}
                
        EventMap events;
        
        void Reset() override
        {
            if (me->GetEntry() != 114976)
            {
                events.Reset();
                // me->SetReactState(REACT_PASSIVE);
                if (me->FindNearestCreature(114976, 5.0f))
                    events.RescheduleEvent(EVENT_1, 8000);
                else
                    me->CastSpell(me, SPELL_BURNING_GROUND);
            }
            events.RescheduleEvent(EVENT_2, 1000);
        }
        
        void JustDied(Unit* who) override
        {
            me->RemoveAllAreaObjects();
        }

        void MoveInLineOfSight(Unit* who) override
        {  
            if (!who->IsCreature())
                return;
            
            if (who->GetEntry() != 114976)
                return;
            
            if (me->GetEntry() == 114976)
                return;
            
            if (me->GetExactDist2d(who) <= 5.0f && me->HasAura(SPELL_BURNING_GROUND))
            {
                me->RemoveAurasDueToSpell(SPELL_BURNING_GROUND);
                events.RescheduleEvent(EVENT_1, 8000);
            }
            
        }  

        void UpdateAI(uint32 diff) override
        {            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {        
                switch(eventId)
                {
                    case EVENT_1:
                        if (me->FindNearestCreature(114976, 7.0f))
                            events.RescheduleEvent(EVENT_1, 8000);
                        else
                            me->CastSpell(me, SPELL_BURNING_GROUND);
                        break;
                    case EVENT_2:
                        if (!me->GetAnyOwner() || !me->GetAnyOwner()->isAlive())
                            me->DespawnOrUnsummon();
                        else
                            events.RescheduleEvent(EVENT_2, 1000);
                        break;
                }
            }
        }
        
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_brawguild_burnstachio_groundAI(creature);
    }
};

// 229025
class spell_brawguild_burnstachio_absorb : public SpellScriptLoader
{
    public:
        spell_brawguild_burnstachio_absorb() : SpellScriptLoader("spell_brawguild_burnstachio_absorb") { }

        class spell_brawguild_burnstachio_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_brawguild_burnstachio_absorb_AuraScript);

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                if (Unit* target = dmgInfo.GetAttacker())
                    if (target->HasAura(228981))
                        absorbAmount = 0.f;
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_brawguild_burnstachio_absorb_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_brawguild_burnstachio_absorb_AuraScript();
        }
};

// 67573
class boss_brawguild_meatball : public CreatureScript
{
public:
    boss_brawguild_meatball() : CreatureScript("boss_brawguild_meatball") {}

    struct boss_brawguild_meatballAI : public BrawlersBossAI
    {
        boss_brawguild_meatballAI(Creature* creature) : BrawlersBossAI(creature) {}

        bool outro, outro1;
        
        void Reset() override
        {           
            outro = false;
            outro1 = false;
            events.Reset();
            me->ApplySpellImmune(0, IMMUNITY_ID, 134851, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 60000);
            events.RescheduleEvent(EVENT_2, 10000);
            events.RescheduleEvent(EVENT_3, 20000);
            Talk(0);
        }
                       
        void JustDied(Unit* who) override
        {
            _Reset();
            
            if (!who)
                return;
            
            Talk(5);

            DoCast(SPELL_OUTRO_1);
            ObjectGuid guid = ObjectGuid::Empty;
            if (Unit* owner = me->GetAnyOwner())
                guid = owner->GetGUID();
            me->AddDelayedEvent(1000, [this, guid] () -> void
            {
                if (Creature* gnome = me->FindNearestCreature(68475, 10.0f, true))
                {
                    gnome->AddDelayedEvent(1000, [gnome, this, guid] () -> void
                    {
                        gnome->AI()->Talk(0, guid);
                    });

                    gnome->AddDelayedEvent(5000, [gnome] () -> void
                    {
                        gnome->AI()->Talk(1);
                    });

                    gnome->AddDelayedEvent(5000, [gnome] () -> void
                    {
                        gnome->CastSpell(gnome, 134917);
                        gnome->DespawnOrUnsummon(4000);
                    });
                }
            });

            me->AddDelayedEvent(10000, [this] () -> void
            {
                _WinRound();
            });
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPct(10) && !outro)
            {
                outro = true;
                DoCast(SPELL_OUTRO);
                Talk(4);
            }
            
            if (me->HealthBelowPct(20) && !outro1)
            {
                outro1 = true;
                Talk(3);
            }
        }
        
        void EnterEvadeMode() override
        {          
            if (outro)
                return;

            _Reset();
            _LoseRound();
        }
        
        void KilledUnit(Unit* who) override
        {
            if (outro)
                return;

            if (Unit* owner = me->GetAnyOwner())
                if (who->GetGUID() == owner->GetGUID())
                {
					_Reset();
                    _LoseRound();
                }  
        }
        
        void UpdateAI(uint32 diff) override
        {            
            if (!UpdateVictim())
                return;

            events.Update(diff);

            // if (me->HasUnitState(UNIT_STATE_CASTING))
                // return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_MEATBALL_MAD);
                        break;
                    case EVENT_2:
                        Talk(1);
                        break;
                    case EVENT_3:
                        Talk(2);
                        break;
                    case EVENT_4:
                        me->RemoveAurasDueToSpell(134851);
                        events.RescheduleEvent(EVENT_4, 500);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_meatballAI (creature);
    }
};


Position totemPos[8]
{
    // A
    {-139.94f, 2485.23f, -49.10f, 0.56f},  
    {-140.82f, 2515.23f, -49.10f, 5.62f}, 
    {-98.45f, 2515.23f, -49.10f, 3.78f}, 
    {-98.45f, 2485.23f, -49.10f, 2.50f}, 
    
    {2051.12f, -4770.54f, 86.77f, 2.42f},
    {2014.44f, -4770.54f, 86.77f, 0.90f},
    {2014.44f, -4733.71f, 86.77f, 5.47f},
    {2051.44f, -4733.71f, 86.77f, 3.98f},

    // H    
};
// 67488, 67487
class boss_brawguild_gg_engineering : public CreatureScript
{
public:
    boss_brawguild_gg_engineering() : CreatureScript("boss_brawguild_gg_engineering") {}

    struct boss_brawguild_gg_engineeringAI : public BrawlersBossAI
    {
        boss_brawguild_gg_engineeringAI(Creature* creature) : BrawlersBossAI(creature) {}

        uint8 count_teleports;
        
        void Reset() override
        {        
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            count_teleports = 0;
            events.Reset();
            // DoCast(SPELL_SHARE_DMG);
            if (me->GetEntry() == 67488)
            {
                me->AddDelayedEvent(700, [this] () -> void
                {
                    if (Unit* owner = me->GetAnyOwner())
                        AttackStart(owner);
                });
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            if (me->GetEntry() == 67488)
            {
                me->SummonCreature(67487, me->GetPositionX(), me->GetPositionY() + 3.0f, me->GetPositionZ(), me->GetOrientation());
                Talk(0);
                events.RescheduleEvent(EVENT_1, 3000);
                events.RescheduleEvent(EVENT_3, 18000);
                if (Unit* unit = me->GetAnyOwner())
                if (Player* owner = unit->ToPlayer())
                {
                    uint8 diff = owner->GetTeamId() == TEAM_ALLIANCE ? 0 : 4;
                    for (uint8 i = 0 + diff; i < 4 + diff; ++i)
                        me->SummonCreature(67500, totemPos[i].GetPositionX(), totemPos[i].GetPositionY(), totemPos[i].GetPositionZ(), totemPos[i].GetOrientation());
                }
            }
            else if (me->GetEntry() == 67487)
            {
                events.RescheduleEvent(EVENT_2, 4000);
                events.RescheduleEvent(EVENT_3, 9000);
            }
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (summon->GetEntry() == 67487)
                if (Unit* owner = me->GetAnyOwner())
                    summon->AI()->AttackStart(owner);
            else if (summon->GetEntry() == 67500)
                summon->SetReactState(REACT_PASSIVE);
        }
               
        void JustDied(Unit* who) override
        {
            if (me->GetEntry() != 67488)
                return;
            
            _Reset();
            
            if (!who)
                return;
            
            if (Creature* talker = me->FindNearestCreature(67487, 200.0f))
                talker->AI()->Talk(2);
            
            _WinRound();
        }
        
        void EnterEvadeMode() override
        {
            if (me->GetEntry() != 67488)
                return;
            _Reset();
            _LoseRound();
        }
        
        void KilledUnit(Unit* who) override
        {
            if (me->GetEntry() != 67488)
                return;
            
            if (Unit* owner = me->GetAnyOwner())
                if (who->GetGUID() == owner->GetGUID())
                {
					_Reset();
                    _LoseRound();
                }  
        }

        
        Creature* GetOtherEngineer()
        {
            if (Creature* oeng = me->FindNearestCreature((me->GetEntry() == 67488 ? 67487 : 67488), 200.0f, true))
                return oeng;
            return NULL;
        }
        
        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType) override
        {
            if (attacker->IsCreature())
                damage /= 2;
            
            if (attacker != me)
            {
                if (Creature* oeng = GetOtherEngineer())
                {
                    if (damage >= me->GetHealth())
                        oeng->Kill(oeng, true);
                    else
                        oeng->DealDamage(oeng, damage);
                }
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
                        DoCastVictim(SPELL_GOBLIN_ROCKET);
                        events.RescheduleEvent(EVENT_1, 8000);
                        break;
                    case EVENT_2:
                        DoCastVictim(SPELL_DEATH_RAY);
                        events.RescheduleEvent(EVENT_2, 4000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_EMERGENCY_TELEPORT);
                        if (count_teleports < 2)
                        {
                            Talk(me->GetEntry() == 67487 ? count_teleports : count_teleports+1);
                            count_teleports++;
                        }

                        me->AddDelayedEvent(700, [this] ()-> void
                        {
                            if (Creature* totem = me->FindNearestCreature(67500, 30.0f, true))
                            {
                                totem->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1);
                                totem->CastSpell(totem, SPELL_ACTIVATE_TOTEM);
                                totem->RemoveAurasDueToSpell(SPELL_READY_TO_ACTIVATE);
                            }
                        });
                        
                        events.RescheduleEvent(EVENT_3, 33000);
                        break;

                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_gg_engineeringAI (creature);
    }
};

// 117275
class boss_brawguild_stitches : public CreatureScript
{
public:
    boss_brawguild_stitches() : CreatureScript("boss_brawguild_stitches") {}

    struct boss_brawguild_stitchesAI : public BrawlersBossAI
    {
        boss_brawguild_stitchesAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {  
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);      
            events.Reset();
            me->CastSpell(me, SPELL_AURA_OF_ROT);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            if (Unit* owner = me->GetAnyOwner())
                me->CastSpell(owner, SPELL_AURA_OF_ROT_TICK);
        }
                       
        void JustDied(Unit* who) override
        {
			_Reset();
            
            if (!who)
                return;
            
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsPlayer())
            {
                Player* player = me->GetAnyOwner()->ToPlayer();
                player->RemoveAurasDueToSpell(SPELL_AURA_OF_ROT_TICK);
                player->RemoveAurasDueToSpell(SPELL_AURA_OF_ROT_DMG);
            }
            _WinRound();
        }               
        
        void EnterEvadeMode() override
        {           
			_Reset();
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsPlayer())
            {
                Player* player = me->GetAnyOwner()->ToPlayer();
                player->RemoveAurasDueToSpell(SPELL_AURA_OF_ROT_TICK);
                player->RemoveAurasDueToSpell(SPELL_AURA_OF_ROT_DMG);
            }
            _LoseRound();
        }
        
        void MoveInLineOfSight(Unit* who) override
        {  
            if (!who->IsPlayer())
                return;
            
            if (who != me->GetAnyOwner())
                return;
            
            if (me->GetExactDist2d(who) >= 10.0f )
                events.RescheduleEvent(EVENT_1, 1);
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
                        if (Unit* owner = me->GetAnyOwner())
                        {
                            me->CastSpell(owner, SPELL_STITCHES_HOOK, true);
                            owner->RemoveAurasDueToSpell(SPELL_AURA_OF_ROT_TICK);
                            owner->RemoveAurasDueToSpell(SPELL_AURA_OF_ROT_DMG);
                        }                  
                        me->AddDelayedEvent(1000, [this] () -> void
                        {
                            if (Unit* owner = me->GetAnyOwner())
                                me->CastSpell(owner, SPELL_AURA_OF_ROT_TICK);
                        });
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_stitchesAI (creature);
    }
};



void AddSC_brawlers_guild_bosses_rank_four()
{
    new boss_brawguild_burnstachio();
    new npc_brawguild_burnstachio_ground();
    new spell_brawguild_burnstachio_absorb();
    new boss_brawguild_meatball();
    new boss_brawguild_gg_engineering();
    new boss_brawguild_stitches();
};