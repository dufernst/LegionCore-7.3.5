#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum eSpells
{
    // drov
    SPELL_COLOSSAL_SLAM         = 175791, // 18
    SPELL_GROUND_PUNCH          = 155294, // 10 
    SPELL_CALL_OF_EARTH         = 175827, // 90 + script adds
    SPELL_SUMMON_GORENS         = 175911, // 45 + script adds
    
    // tarlna
    SPELL_COLOSSAL_SLAM_TARLNA  = 175973, // 16
    SPELL_SUMMON_MANDRAGORA     = 176013, // 46 + script adds
    SPELL_SAVAGE_VINES          = 176001, // 16
    SPELL_SUMMON_MAIN_ADDS      = 175975, // ticks ? + script adds
    SPELL_GENESIS               = 175979, // 49
    SPELL_WAITED_FINISH         = 175123,
    
    // ruhamar
    SPELL_SOLAR_RADIATION       = 167708,
    SPELL_SHARP_BEAK            = 167614, // 12
    SPELL_SOLAR_BREATH          = 167679, // 29
    SPELL_LOOSE_QUILLS          = 167647, // 115 (fly)
    SPELL_BLOOD_FEATHER         = 167625, // 15 + script adds
    SPELL_SLEEP_ADDS            = 160641,
    SPELL_FIX_TARGET            = 167757,
    
    EVENT_GROUP_GROUND          = 1,
    
    // kazzak
    SPELL_MARK_OF_KAZZAR        = 187667, // 10
    SPELL_FEL_BREATH            = 187664, // 23
    SPELL_SUPREME_DOOM          = 187466, // 52 + trigger 187471
    SPELL_SUPREME_DOOM_TRIGGER  = 187471, 
    SPELL_TWISTED_REFLECTION    = 187702, // 33% 66%  
};


// 81252
class boss_drov : public CreatureScript
{
public:
    boss_drov() : CreatureScript("boss_drov") { }

    struct boss_drovAI : public ScriptedAI
    {
        boss_drovAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
        }
        
        void EnterCombat(Unit* unit)
        {
            events.RescheduleEvent(EVENT_1, 18000);
            events.RescheduleEvent(EVENT_2, 10000);
            events.RescheduleEvent(EVENT_3, 90000);
            events.RescheduleEvent(EVENT_4, 45000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (Player* pl = summon->FindNearestPlayer(70.0f, true))
                summon->AI()->AttackStart(pl);
        }
        
        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply)
        {
            if (spellId == SPELL_CALL_OF_EARTH && !apply)
				summons.DespawnEntry(88106);
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_COLOSSAL_SLAM);
                        events.RescheduleEvent(EVENT_1, 18000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_GROUND_PUNCH);
                        events.RescheduleEvent(EVENT_2, 10000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_CALL_OF_EARTH);
                        events.RescheduleEvent(EVENT_3, 90000);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_SUMMON_GORENS);
                        events.RescheduleEvent(EVENT_4, 45000);  
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_drovAI(creature);
    }
};

// 81535
class boss_tarlna : public CreatureScript
{
public:
    boss_tarlna() : CreatureScript("boss_tarlna") { }

    struct boss_tarlnaAI : public ScriptedAI
    {
        boss_tarlnaAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
        }
        
        void EnterCombat(Unit* unit)
        {
            events.RescheduleEvent(EVENT_1, 16000);
            events.RescheduleEvent(EVENT_2, 36000);
            events.RescheduleEvent(EVENT_3, 26000); // 16
            events.RescheduleEvent(EVENT_4, 49000); 
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            DoZoneInCombat(summon, 150.0f);
            if (summon->GetEntry() == 88142)
                summon->CastSpell(summon, SPELL_WAITED_FINISH);
        }
        
        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply)
        {
            if (!apply)
                return;
            
            if (spellId == SPELL_GENESIS)
            {
                for (uint8 i = urand(2,4); i > 0; --i)
                    me->CastSpell(me->GetPositionX() + irand(-16, 16), me->GetPositionY() + irand(-16, 16), me->GetPositionZ(), SPELL_SUMMON_MAIN_ADDS);
                            
                std::list<HostileReference*> threat_list = me->getThreatManager().getThreatList();
                
                if (!threat_list.empty())
                    for (std::list<HostileReference*>::const_iterator itr = threat_list.begin(); itr!= threat_list.end(); ++itr)
                    {
                        if(Unit* target = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                            for (uint8 i = urand(2,4); i > 0; --i)
                                me->CastSpell(target->GetPositionX() + irand(-6, 6), target->GetPositionY() + irand(-6, 6), target->GetPositionZ(), SPELL_SUMMON_MAIN_ADDS);                                
                    }
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_COLOSSAL_SLAM_TARLNA);
                        events.RescheduleEvent(EVENT_1, 16000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_SUMMON_MANDRAGORA);
                        events.RescheduleEvent(EVENT_2, 46000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_SAVAGE_VINES);
                        events.RescheduleEvent(EVENT_3, 16000); // 16
                        break;
                    case EVENT_4:
                        DoCast(SPELL_GENESIS);
                        events.RescheduleEvent(EVENT_4, 49000); 
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_tarlnaAI(creature);
    }
};

// 88142
class npc_giant_lasher : public CreatureScript
{
public:
    npc_giant_lasher() : CreatureScript("npc_giant_lasher") { }

    struct npc_giant_lasherAI : public ScriptedAI
    {
        npc_giant_lasherAI(Creature* creature) : ScriptedAI(creature) 
        {
            justCasted = false;
        }
        
        EventMap events;
        bool justCasted;

        void Reset()
        {
            events.Reset();
        }
        
        void EnterCombat(Unit* unit)
        {
            events.RescheduleEvent(EVENT_1, 4000);
        }
        
        void IsSummonedBy(Unit* summoner)
        {
            me->AddDelayedEvent(13000, [this] () -> void
            {
                me->RemoveAurasDueToSpell(SPELL_WAITED_FINISH);
                if (Player* pl = me->FindNearestPlayer(70.0f, true))
                    AttackStart(pl);
            });
        }
        
        void MoveInLineOfSight(Unit* who)
        {      
            if (who->GetTypeId() == TYPEID_PLAYER && !justCasted && me->IsWithinDistInMap(who, 2.0f) && me->HasAura(SPELL_WAITED_FINISH))
            {
                justCasted = true;
                who->CastSpell(me, 175986);
                who->Kill(me);
                me->DespawnOrUnsummon(2000);
            }
        }


        void UpdateAI(uint32 diff)
        {
            if (me->HasAura(SPELL_WAITED_FINISH))  
                return;
            
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(176000);
                        events.RescheduleEvent(EVENT_1, 4000);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_giant_lasherAI(creature);
    }
};

// 83746
class boss_ruhamar : public CreatureScript
{
public:
    boss_ruhamar() : CreatureScript("boss_ruhamar") { }

    struct boss_ruhamarAI : public ScriptedAI
    {
        boss_ruhamarAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;
        uint32 healthPct;
        Position oldpos;

        void Reset()
        {
            me->GetMotionMaster()->MovePath(439156, true);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            me->SetReactState(REACT_AGGRESSIVE);
            
            events.Reset();
            summons.DespawnAll();
            healthPct = 98;
        }
        
        void EnterCombat(Unit* unit)
        {
            me->SetDisableGravity(false);
            me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3,  UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            me->CastSpell(me, SPELL_SOLAR_RADIATION);
            events.RescheduleEvent(EVENT_1, 12000, EVENT_GROUP_GROUND);
            events.RescheduleEvent(EVENT_2, 29000, EVENT_GROUP_GROUND);
            events.RescheduleEvent(EVENT_3, 85000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            DoZoneInCombat(summon, 150.0f);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPct(healthPct))
            {
                healthPct -= urand(2,4);
                events.RescheduleEvent(EVENT_4, 10, EVENT_GROUP_GROUND);
            }
        }
        
        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply)
        {
            if (!apply && spellId == SPELL_LOOSE_QUILLS)
                me->GetMotionMaster()->MovePoint(2, oldpos.GetPositionX(), oldpos.GetPositionY(), oldpos.GetPositionZ());
        }
        
        void MovementInform(uint32 type, uint32 point)
        {
            if (type == WAYPOINT_MOTION_TYPE)
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            
            if (type != POINT_MOTION_TYPE)
                return;

            me->GetMotionMaster()->MovementExpired();
            if (point == 1)
                me->CastSpell(me, SPELL_LOOSE_QUILLS);
            else if (point == 2)
            {
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
                me->SetReactState(REACT_AGGRESSIVE);
                
                events.RescheduleEvent(EVENT_1, 12000, EVENT_GROUP_GROUND);
                events.RescheduleEvent(EVENT_2, 29000, EVENT_GROUP_GROUND);
                events.RescheduleEvent(EVENT_3, 85000);
            }
        }
            
    
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
				switch (eventId)
				{
				case EVENT_1:
					DoCastVictim(SPELL_SHARP_BEAK);
					events.RescheduleEvent(EVENT_1, 12000, EVENT_GROUP_GROUND);
					break;
				case EVENT_2:
					DoCastVictim(SPELL_SOLAR_BREATH);
					events.RescheduleEvent(EVENT_2, 29000, EVENT_GROUP_GROUND);
					break;
				case EVENT_3:
				{
					events.CancelEventGroup(EVENT_GROUP_GROUND);

					me->SetCanFly(true);
					me->SetDisableGravity(true);
					me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
					me->SetReactState(REACT_PASSIVE);
					me->AttackStop();
					Position pos;
					pos.Relocate(me);
					oldpos = pos;
					pos.m_positionZ += 10.0f;
					me->GetMotionMaster()->MovePoint(1, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
					break;
				}
                    case EVENT_4:
                        DoCastVictim(SPELL_BLOOD_FEATHER);
                        break;
                }   
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ruhamarAI(creature);
    }
};

// 83769
class npc_energized_phoenix : public CreatureScript
{
public:
    npc_energized_phoenix() : CreatureScript("npc_energized_phoenix") { }

    struct npc_energized_phoenixAI : public ScriptedAI
    {
        npc_energized_phoenixAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        bool moveComplete;
        uint32 moveTimer;
		ObjectGuid targetMove;

        void Reset()
        {
            events.Reset();
        }
        
        void EnterCombat(Unit* unit)
        {
        }


        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                
                me->CastStop();
                me->SetHealth(me->GetMaxHealth());
                me->CastSpell(me, 167629, true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->CastSpell(me, SPELL_SLEEP_ADDS, true);
                me->GetMotionMaster()->MovementExpired(true);
                me->SetDisplayId(66325);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
            }
        }
        
        void IsSummonedBy(Unit* owner)
        {
            if (owner->GetGUID() != me->GetGUID())
            {
                std::list<Creature*> adds;
                GetCreatureListWithEntryInGrid(adds, me, me->GetEntry(), 5.0f);
                if (!adds.empty())
                    for (std::list<Creature*>::iterator itr = adds.begin(); itr != adds.end(); ++itr)
                    {
                        if ((*itr)->HasAura(SPELL_SLEEP_ADDS))
                        {
                            (*itr)->RemoveAurasDueToSpell(SPELL_SLEEP_ADDS);
                            (*itr)->AI()->IsSummonedBy((*itr));
                        }
                    }
            }
            
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetDisplayId(me->GetNativeDisplayId());
            
            moveComplete = false;
            moveTimer = 2000;
            
            std::list<Player*> targets;
            targets.clear();
            GetPlayerListInGrid(targets, me, 70.0f);
            if (!targets.empty())
            {
                std::list<Player*>::const_iterator itr = targets.begin();
                std::advance(itr, urand(0, targets.size() - 1));
                if (*itr)
                    targetMove = (*itr)->GetGUID();
                
                me->GetMotionMaster()->MoveFollow((*itr), PET_FOLLOW_DIST, (*itr)->GetFollowAngle());
                me->CastSpell((*itr), SPELL_FIX_TARGET);
            }
            else
                me->DespawnOrUnsummon();
        }
           
    
        void UpdateAI(uint32 diff)
        {
            if (!moveComplete)
            {
                if (moveTimer <= diff)
                {
                    Player* target = ObjectAccessor::GetPlayer(*me, targetMove);
                    if (!target)
                    {
                        IsSummonedBy(me); // tru refind
                        return;
                    }

                    if (me->GetDistance(target) < 3.0f)
                    {
                        moveComplete = true;
                        
                        me->CastStop();
                        me->SetHealth(me->GetMaxHealth());
                        me->CastSpell(me, 167629, true);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        me->CastSpell(me, SPELL_SLEEP_ADDS, true);
                        me->GetMotionMaster()->MovementExpired(true);
                        me->SetDisplayId(66325);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                    }
                    moveTimer = 1000;
                }
                else
                    moveTimer -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_energized_phoenixAI(creature);
    }
};

// 94015
class boss_kazzak_legion : public CreatureScript
{
public:
    boss_kazzak_legion() : CreatureScript("boss_kazzak_legion") { }

    struct boss_kazzak_legionAI : public ScriptedAI
    {
        boss_kazzak_legionAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;
        uint32 healthPct;

        void Reset()
        {
            events.Reset();
            summons.DespawnAll();
            healthPct = 66;
        }
        
        void EnterCombat(Unit* unit)
        {
            events.RescheduleEvent(EVENT_1, 10000);
            events.RescheduleEvent(EVENT_2, 23000);
            events.RescheduleEvent(EVENT_3, 52000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            DoZoneInCombat(summon, 150.0f);
        }       

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPct(healthPct))
            {
                healthPct -= 33;
                events.RescheduleEvent(EVENT_4, 10);
            }
        }     
    
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
				switch (eventId)
				{
                    case EVENT_1:
                        DoCast(SPELL_MARK_OF_KAZZAR);
                        events.RescheduleEvent(EVENT_1, 10000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_FEL_BREATH);
                        events.RescheduleEvent(EVENT_2, 23000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_SUPREME_DOOM);
                        events.RescheduleEvent(EVENT_3, 52000);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_TWISTED_REFLECTION);
                        break;
                }   
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_kazzak_legionAI(creature);
    }
};

// 94712
class npc_twisted_reflection : public CreatureScript
{
public:
    npc_twisted_reflection() : CreatureScript("npc_twisted_reflection") { }

    struct npc_twisted_reflectionAI : public ScriptedAI
    {
        npc_twisted_reflectionAI(Creature* creature) : ScriptedAI(creature) 
        {
            moveComplete = true;
        }
        
        bool moveComplete;
        uint32 moveTimer;
		ObjectGuid targetMove;

        
        void EnterCombat(Unit* unit)
        {
        }

        void IsSummonedBy(Unit* targ)
        {
            if (Creature* owner = me->FindNearestCreature(94015, 100.0f, true))
            {
                owner->AI()->JustSummoned(me); // hack
                me->AddAura(187709, me);
                targetMove = owner->GetGUID();
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->MoveFollow(owner, 0, 0);
                moveTimer = 2000;
                moveComplete = false;
            }
            else
                me->DespawnOrUnsummon();
        }
           
    
        void UpdateAI(uint32 diff)
        {
            if (!moveComplete)
            {
                if (moveTimer <= diff)
                {
                    Unit* target = Unit::GetUnit(*me, targetMove);
                    if (!target)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }

                    if (me->GetDistance(target) < 1.0f)
                    {
                        moveComplete = true;
                        target->CastSpell(target, 187711);
                        me->DespawnOrUnsummon();
                    }
                    moveTimer = 1000;
                }
                else
                    moveTimer -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_twisted_reflectionAI(creature);
    }
};

// 187668
class spell_mark_of_kazzak_legion : public SpellScriptLoader
{
    public:
        spell_mark_of_kazzak_legion() : SpellScriptLoader("spell_mark_of_kazzak_legion") { }

        class spell_mark_of_kazzak_legion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mark_of_kazzak_legion_AuraScript);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                if (Player* _player = caster->ToPlayer())
                {
                    float damage = int32(eventInfo.GetDamageInfo()->GetDamage());

                    if (damage)
                        _player->CastCustomSpell(_player, 187671, &damage, NULL, NULL, false);
                }
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_mark_of_kazzak_legion_AuraScript::OnProc, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mark_of_kazzak_legion_AuraScript();
        }
};

// 187471
class spell_mark_of_kazzak_legion_trigger : public SpellScriptLoader
{
    public:
        spell_mark_of_kazzak_legion_trigger() : SpellScriptLoader("spell_mark_of_kazzak_legion_trigger") { }

        class spell_mark_of_kazzak_legion_trigger_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mark_of_kazzak_legion_trigger_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                
                if (Unit* target = GetTarget())
                {
                    if (target->GetHealthPct() < 50)
                        target->CastSpell(target, 187632);
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_mark_of_kazzak_legion_trigger_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mark_of_kazzak_legion_trigger_AuraScript();
        }
};


// 187466
class spell_supreme_doom : public SpellScriptLoader
{
    public:
        spell_supreme_doom() : SpellScriptLoader("spell_supreme_doom") { }

        class spell_supreme_doom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_supreme_doom_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!targets.empty())
                {
                    if (targets.size() >= 4)
                        Trinity::Containers::RandomResizeList(targets, targets.size()/ 4);
                    else
                        Trinity::Containers::RandomResizeList(targets, 1);
                }
            }
            
            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        float damage = target->GetHealth() - 1;
                        caster->CastCustomSpell(target, SPELL_SUPREME_DOOM_TRIGGER, &damage, NULL, NULL, false);                            
                    }      
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_supreme_doom_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_supreme_doom_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_supreme_doom_SpellScript();
        }
};

void AddSC_world_bossess_draenor()
{
    new boss_drov();
    new boss_tarlna();
    new npc_giant_lasher();
    new boss_ruhamar();
    new npc_energized_phoenix();
    new boss_kazzak_legion();
    new npc_twisted_reflection();
    new spell_mark_of_kazzak_legion();
    new spell_mark_of_kazzak_legion_trigger();
    new spell_supreme_doom();
}
