#include "AchievementMgr.h"
#include "AreaTriggerAI.h"
#include "BrawlersGuild.h"

enum eSpells
{
    //! 7 rank
    // nibleah
    SPELL_ACID_SPIT             = 140914, // 3
    SPELL_ACIT_AT               = 141013, // 20
    
    // epicus
    SPELL_INTRO_INVIS           = 69676,
    SPELL_INTRO_SCALE           = 142758,
    // shark
    SPELL_BLUE_CRUSH            = 133262, // 28 ?
    SPELL_OUTRO_DIED            = 133259,
    // gun
    SPELL_SUMMON_TARGET         = 133222, // 30
    SPELL_DESTRUCTION_LASER     = 133233,
    SPELL_LASER_DMG             = 133230, // on trigger (67538)
    // guitar
    SPELL_ROCK_N_ROLL           = 133205, // 5
    
    // rayd
    SPELL_SHOCKWAVE             = 229012, 
    SPELL_GRASPING_HAND         = 229022, 
    SPELL_ACIDIC_WOUND          = 229035,
    SPELL_DEATH_THROES          = 229026,
    SPELL_CRUSHING_DARKNESS     = 229162,
    SPELL_FEL_CRYSTAL           = 229165, // doing AT
    SPELL_GAVEL                 = 229037,
    SPELL_BLACK_HOLE            = 229042, 
    SPELL_FEL_IMPIOSION         = 229051, 
    SPELL_SHADOWFEL             = 229060

};



// 70656
class boss_brawguild_nibleah : public CreatureScript
{
public:
    boss_brawguild_nibleah() : CreatureScript("boss_brawguild_nibleah") {}

    struct boss_brawguild_nibleahAI : public BrawlersBossAI
    {
        boss_brawguild_nibleahAI(Creature* creature) : BrawlersBossAI(creature) {}

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 3000);
            events.RescheduleEvent(EVENT_2, 20000);
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
                        DoCast(SPELL_ACID_SPIT);
                        events.RescheduleEvent(EVENT_1, 3000);
                        break;
                    case EVENT_2:
                    {
                        Position pos;
                        me->GetNearPoint2D(pos, 15.0f, frand(-M_PI/3, M_PI/3));
                        me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_ACIT_AT);
                        events.RescheduleEvent(EVENT_2, 20000);
                        break;
                    }
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_nibleahAI (creature);
    }
};

Position totems_A[4]
{
    {-130.856f, 2488.31f, -49.1092f, 1.71907f},
    {-130.685f, 2508.9f, -49.1092f, 1.57377f},
    {-106.998f, 2508.99f, -49.1095f, 3.14456f},
    {-106.88f, 2489.1f, -49.1095f, 3.11707f}
};

Position totems_H[4]
{
    {2041.97f, -4764.33f, 86.7753f, 1.47468f},
    {2019.42f, -4764.33f, 86.7757f, 1.47468f},
    {2019.42f, -4742.36f, 86.7766f, 1.47468f},
    {2041.97f, -4742.36f, 86.7752f, 1.47468f}
};

uint32 totems[4]
{
    116701, 116707, 116706, 116708
};

// 116692
class boss_brawguild_serpent : public CreatureScript
{
public:
    boss_brawguild_serpent() : CreatureScript("boss_brawguild_serpent") {}

    struct boss_brawguild_serpentAI : public BrawlersBossAI
    {
        boss_brawguild_serpentAI(Creature* creature) : BrawlersBossAI(creature) {}

        void EnterCombat(Unit* /*who*/) override
        {
            if (Unit* unit = me->GetAnyOwner())
            if (Player* owner = unit->ToPlayer())
                if (owner->GetTeamId() == TEAM_ALLIANCE)
                {
                    for (uint8 i = 0; i < 4; ++i)
                        me->SummonCreature(totems[i], totems_A[i].GetPositionX(), totems_A[i].GetPositionY(), totems_A[i].GetPositionZ(), 0.0f);
                }
                else
                {
                    for (uint8 i = 0; i < 4; ++i)
                        me->SummonCreature(totems[i], totems_H[i].GetPositionX(), totems_H[i].GetPositionY(), totems_H[i].GetPositionZ(), 0.0f);
                }
        }     

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (me->isInCombat())
                DoZoneInCombat(summon); 
            
            if (summon->GetEntry() == 119281)
            {
                summon->SetReactState(REACT_PASSIVE);
                summon->CastSpell(summon, 236809);
                summon->AddDelayedEvent(2000, [summon] () -> void
                {
                    summon->CastSpell(summon, 236794);
                    summon->DespawnOrUnsummon(500);
                });
            }
        }
        void UpdateAI(uint32 diff) override
        {            
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_serpentAI (creature);
    }
};

// 232383
class spell_brawling_healing_totem : public SpellScriptLoader
{
    public:
        spell_brawling_healing_totem() : SpellScriptLoader("spell_brawling_healing_totem") { }

        class spell_brawling_healing_totem_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_brawling_healing_totem_AuraScript);

            void OnTick(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                float healValue = caster->GetAuraCount(232383);
                caster->CastCustomSpell(caster, 232396, &healValue, NULL, NULL, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_brawling_healing_totem_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_brawling_healing_totem_AuraScript();
        }
};

// 67490 -> 67492 (67495)
class boss_brawguild_epicus : public CreatureScript
{
public:
    boss_brawguild_epicus() : CreatureScript("boss_brawguild_epicus") {}

    struct boss_brawguild_epicusAI : public BrawlersBossAI
    {
        boss_brawguild_epicusAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {
            if (me->GetEntry() != 67490)
                return;
            
            me->AddDelayedEvent(1000, [this] () -> void
            {
                if (Creature* razrubait = me->SummonCreature(67526, me->GetPositionX() + irand(-5, 5), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                {
                    razrubait->SetReactState(REACT_PASSIVE);
                    razrubait->AI()->Talk(0);
                    razrubait->AddDelayedEvent(2000, [razrubait] () -> void
                    {
                        razrubait->AI()->Talk(1);
                    });
                    
                    razrubait->AddDelayedEvent(5000, [razrubait] () -> void
                    {
                        razrubait->AI()->Talk(2);
                    });
                    
                    razrubait->AddDelayedEvent(8000, [razrubait] () -> void
                    {
                        razrubait->AI()->Talk(3);
                        razrubait->CastSpell(razrubait, 84768);
                        razrubait->DespawnOrUnsummon(1500);
                    });
                }
                DoCast(SPELL_INTRO_INVIS);
                DoCast(SPELL_INTRO_SCALE);
            });
        }

        void EnterCombat(Unit* /*who*/) override
        {
            switch(me->GetEntry())
            {
                case 67490:
                    events.RescheduleEvent(EVENT_1, 28000);
                    break;
                case 67492:
                    events.RescheduleEvent(EVENT_2, urand(29000, 30000));
                    break;
                case 67495:
                    events.RescheduleEvent(EVENT_4, 5000);
                    break;
            }
        }     
        
        void JustDied(Unit* who) override
        {
            _Reset(); 
            if (who && me->GetEntry() == 67490) 
            {
                DoCast(SPELL_OUTRO_DIED);
                _WinRound();
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
                        DoCast(SPELL_BLUE_CRUSH);
                        events.RescheduleEvent(EVENT_1, 28000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_SUMMON_TARGET);
                        events.RescheduleEvent(EVENT_3, 1000);
                        events.RescheduleEvent(EVENT_2, urand(29000, 30000));
                        break;
                    case EVENT_3:
                        if (Creature* target = me->FindNearestCreature(67538, 100.0f, true))
                            me->CastSpell(target, SPELL_DESTRUCTION_LASER);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_ROCK_N_ROLL);
                        events.RescheduleEvent(EVENT_4, 5000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_epicusAI (creature);
    }
};


uint32 Spells_Rayd[10]
{
    SPELL_SHOCKWAVE, 
    SPELL_GRASPING_HAND,
    SPELL_ACIDIC_WOUND,
    SPELL_DEATH_THROES,
    SPELL_CRUSHING_DARKNESS,
    SPELL_FEL_CRYSTAL,
    SPELL_GAVEL,
    SPELL_BLACK_HOLE,
    SPELL_FEL_IMPIOSION,
    SPELL_SHADOWFEL 
};

uint32 Spells_Morph[10]
{
    229008,
    229014,
    229036,
    229033,
    229167,
    229172,
    229172,
    229046,
    229057,
    229058    
};

// 115040 115044
class boss_brawguild_rayd : public CreatureScript
{
public:
    boss_brawguild_rayd() : CreatureScript("boss_brawguild_rayd") {}

    struct boss_brawguild_raydAI : public BrawlersBossAI
    {
        boss_brawguild_raydAI(Creature* creature) : BrawlersBossAI(creature) {}


        uint8 phase;
        uint32 last_spell;
        void Reset() override
        {
            me->setRegeneratingHealth(false);
        }
        
        void EnterCombat(Unit* /*who*/) override
        {
            phase = 0;
            events.RescheduleEvent(EVENT_1, 7000);
        }     

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (me->isInCombat())
                DoZoneInCombat(summon); 

            if (Unit* owner = me->GetAnyOwner())
            {
                switch(summon->GetEntry())
                {
                    case 115052:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->CastSpell(owner, 188080);
                        summon->CastSpell(owner, 229024);
                        break;
                    case 115220:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->CastSpell(owner, 229166);
                        break;
                }
            }
        }
        
        void EnterEvadeMode() override {}

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
                        DoCast(Spells_Rayd[phase]);
                        if (Creature* visual = me->FindNearestCreature(115044, 10.0f, true))
                        {
                            last_spell = Spells_Morph[phase];
                            visual->CastSpell(visual, last_spell);
                            visual->AddDelayedEvent(6000, [visual, this] () -> void
                            {
                                visual->RemoveAura(last_spell);
                            });
                        }
                        
                        phase++;
                        events.RescheduleEvent(EVENT_1, urand(11000, 12000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_raydAI (creature);
    }
};

// 229042
class spell_brawling_black_hole : public SpellScriptLoader
{
    public:
        spell_brawling_black_hole() : SpellScriptLoader("spell_brawling_black_hole") { }

        class spell_brawling_black_hole_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_brawling_black_hole_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    GetSpell()->AddDestTarget(GetSpell()->getDestTarget(0), 1);
            }

            void Register() override
            {
               OnCast += SpellCastFn(spell_brawling_black_hole_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_brawling_black_hole_SpellScript();
        }
};


// 115220
class npc_brawguild_fel_crystal : public CreatureScript
{
public:
    npc_brawguild_fel_crystal() : CreatureScript("npc_brawguild_fel_crystal") {}

    struct npc_brawguild_fel_crystalAI : public ScriptedAI
    {
        npc_brawguild_fel_crystalAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
            events.RescheduleEvent(EVENT_1, 10000);
        }
        
        void UpdateAI(uint32 diff) override
        {                       
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        
                        for (float angle = 0.0f; angle <= 6.24f; angle += 0.51f)
                        {
                            me->SetOrientation(angle);
                            DoCast(234808);
                            
                        }
                        events.RescheduleEvent(EVENT_1, 10000);
                        break;
                }
            }

        }
        
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_brawguild_fel_crystalAI (creature);
    }
};

void AddSC_brawlers_guild_bosses_rank_seven()
{
    new boss_brawguild_nibleah();
    new boss_brawguild_serpent();
    new spell_brawling_healing_totem();
    new boss_brawguild_epicus();
    new boss_brawguild_rayd();
    new spell_brawling_black_hole();
    new npc_brawguild_fel_crystal();
};