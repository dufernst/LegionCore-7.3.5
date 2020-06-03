#include "AchievementMgr.h"
#include "BrawlersGuild.h"

enum eSpells
{
    //! 1 rank
    
    // ooliss
    SPELL_HORRIFIC_PURSUIT      = 233224, 
    
    // grandpa
    SPELL_THROW_TOY_1           = 140967, //urand 140967 140962
    SPELL_THROW_TOY_2           = 140962, //urand 140967 140962
    SPELL_THROW_TOY_BAD         = 141020,
    SPELL_SONG_OF_FLEIT         = 140982, // 21 ?
    SPELL_KANTATA_OF_FLEIT      = 140983, // 48 ?   (heal)
    SPELL_GRUMMKEPACK           = 140950, // on 50% and after 140986
    SPELL_BAD_LUCKYDO           = 140986,
    SPELL_THROW_LUCKY_FLEIT     = 140988, // on 7%
    
    // oso
    SPELL_SHOTGUN_ROAR          = 234492, // 11 ?
    SPELL_CLAWSTROPHOBIC        = 234313, // 21 ?
    SPELL_JUMP_GRIZZLY          = 234305, // 21 ? (3 times)
    
    // gnomes
    SPELL_PREPARED_TO_SPELL     = 229758, // ?????
    SPELL_WHIRLWIND             = 234046, // 20
    
    SPELL_LAVA_BURST            = 229393, // 4
    SPELL_MOLTEN_SLAG           = 229394, // 10
    
    SPELL_LIGHTING_CRASH        = 229471, // 7    
};

// 117133
class boss_brawguild_ooliss : public CreatureScript
{
public:
    boss_brawguild_ooliss() : CreatureScript("boss_brawguild_ooliss") {}

    struct boss_brawguild_oolissAI : public BrawlersBossAI
    {
        boss_brawguild_oolissAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {           
            events.Reset();
            me->ApplySpellImmune(0, IMMUNITY_ID, 236110, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INSTAKILL, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 25000);
        } 
        
        void UpdateAI(uint32 diff) override
        {            
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (me->HasAura(SPELL_HORRIFIC_PURSUIT))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                    {
                        me->CastSpell(me, 233226);
                        Position pos;
                        me->GetNearPoint2D(pos, 15.0f, frand(-M_PI/3, M_PI/3));
                        me->NearTeleportTo(pos);
                        me->AddUnitState(UNIT_STATE_ROOT);
                        events.RescheduleEvent(EVENT_2, 500);
                        events.RescheduleEvent(EVENT_1, 25500);
                        break;
                    }
                    case EVENT_2:
                    {
                        //>>>Hack
                        me->AddDelayedEvent(2400, [this]() -> void
                        {
                            if (me && me->isAlive())
                                me->ClearUnitState(UNIT_STATE_ROOT);
                        });
                        //<<<Hack

                        if (Unit* owner = me->GetAnyOwner())
                            me->CastSpell(owner, SPELL_HORRIFIC_PURSUIT);
                        
                        me->SetWalk(true);
                        me->AddDelayedEvent(2400 + 7500, [this]() -> void
                        {
                            me->SetWalk(false);
                        });
                        break;
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_oolissAI (creature);
    }
};

// 70678
class boss_brawguild_grandpa : public CreatureScript
{
public:
    boss_brawguild_grandpa() : CreatureScript("boss_brawguild_grandpa") {}

    struct boss_brawguild_grandpaAI : public BrawlersBossAI
    {
        boss_brawguild_grandpaAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        uint32 healthPct;       
        
        void Reset() override
        {           
            events.Reset();
            healthPct = 50;
            summons.DespawnAll();
            me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_GRUMMKEPACK, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 4000); // throw
            events.RescheduleEvent(EVENT_2, 18000); // song
            events.RescheduleEvent(EVENT_3, 8000); // kantata urand(40, 48)
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            switch(summon->GetEntry())
            {
                case 70735:
                case 70693:
                    summon->CastSpell(summon, 140975);
                    break;
                case 70692:
                    summon->CastSpell(summon, 140971);
                    break;
            }
        }
        
        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPct(healthPct))
            {
                if (healthPct == 50)
                {
                    healthPct -= 43;
                    events.RescheduleEvent(EVENT_5, 500);
                    events.RescheduleEvent(EVENT_4, urand(5000, 8000));
                }
                else
                {
                    healthPct = 0;
                    Talk(1);
                    DoCast(SPELL_THROW_LUCKY_FLEIT);
                }
            }
        }
        
        void JustDied(Unit* who) override
        {
            _Reset(); 
            
            if (who) 
                _WinRound(); 
            
            Talk(3);
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
                    {
                        Position pos;
                        me->GetNearPoint2D(pos, 20.0f, frand(-M_PI/3, M_PI/3));
                        if (me->HasAura(SPELL_BAD_LUCKYDO))
                            me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),  SPELL_THROW_TOY_BAD);
                        else
                            me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),  (urand(1,2) == 1 ? SPELL_THROW_TOY_1 : SPELL_THROW_TOY_2));
                        events.RescheduleEvent(EVENT_1, 4000);
                        break;
                    }
                    case EVENT_2:
                        DoCast(SPELL_SONG_OF_FLEIT);
                        events.RescheduleEvent(EVENT_2, urand(18000, 25000));
                        break;
                    case EVENT_3:
                        DoCast(SPELL_KANTATA_OF_FLEIT);
                        events.RescheduleEvent(EVENT_3, urand(35000, 48000));
                        break;
                    case EVENT_4:
                    {
                        if (Unit* owner = me->GetAnyOwner())
                        {
                            std::list<Creature*> adds;
                            GetCreatureListWithEntryInGrid(adds, me, 70735, 60.0f);
                            if (!adds.empty())
                                for (std::list<Creature*>::iterator itr = adds.begin(); itr != adds.end(); ++itr)
                                {
                                    (*itr)->SetFacingTo(owner);
                                    (*itr)->CastSpell(owner, 141053);
                                }
                        }
                        break;
                    }
                    case EVENT_5:
                        Talk(0);
                        if (Unit* owner = me->GetAnyOwner())
                            me->CastSpell(owner, SPELL_GRUMMKEPACK);
                        DoCast(SPELL_BAD_LUCKYDO);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_grandpaAI (creature);
    }
};

// 117753
class boss_brawguild_oso : public CreatureScript
{
public:
    boss_brawguild_oso() : CreatureScript("boss_brawguild_oso") {}

    struct boss_brawguild_osoAI : public BrawlersBossAI
    {
        boss_brawguild_osoAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {           
            events.Reset();
            if (Creature* morozec = me->SummonCreature(117759, me->GetPositionX() + 3.0f, me->GetPositionY() + 3.0f, me->GetPositionZ(), me->GetOrientation()))
            {
                me->SetReactState(REACT_PASSIVE);
                if (me->GetAnyOwner())
                    morozec->AI()->Talk(0, me->GetAnyOwner()->GetGUID());
                morozec->AddDelayedEvent(6000, [morozec, this] () -> void
                {
                    me->SetFacingTo(morozec);
                    me->CastSpell(morozec, 93330);
                    morozec->CastSpell(morozec, 234213);
                    morozec->DespawnOrUnsummon(1000);
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (me->GetAnyOwner())
                        AttackStart(me->GetAnyOwner());
                    if (Creature* moroz = me->FindNearestCreature(117860, 30.0f, true))
                        moroz->AI()->Talk(0);
                });
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 12000); 
            events.RescheduleEvent(EVENT_2, urand(20000, 21000));
            events.RescheduleEvent(EVENT_3, urand(21000, 22000)); 
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
                        DoCast(SPELL_SHOTGUN_ROAR);
                        if (urand(1, 4) != 1)
                            if (Creature* moroz = me->FindNearestCreature(117860, 30.0f, true))
                                moroz->AI()->Talk(1);
                        events.RescheduleEvent(EVENT_1, 12000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_CLAWSTROPHOBIC);
                        events.RescheduleEvent(EVENT_2, urand(20000, 22000));
                        break;
                    case EVENT_3:
                    {
                        Position pos;
                        me->GetNearPoint2D(pos, 20.0f, frand(-M_PI / 3, M_PI / 3));
                        me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_JUMP_GRIZZLY);
                        me->AddDelayedEvent(2200, [this]() -> void
                        {
                            Position pos;
                            me->GetNearPoint2D(pos, 20.0f, frand(-M_PI / 3, M_PI / 3));
                            me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_JUMP_GRIZZLY);
                            me->AddDelayedEvent(2200, [this]() -> void
                            {
                                Position pos;
                                me->GetNearPoint2D(pos, 20.0f, frand(-M_PI / 3, M_PI / 3));
                                me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_JUMP_GRIZZLY);
                            });
                        });
                        events.RescheduleEvent(EVENT_1, urand(6000, 12000));
                        events.RescheduleEvent(EVENT_3, urand(21000, 22000)); 
                        break;
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_osoAI (creature);
    }
};

// 115292, 115294, 115295
class boss_brawguild_gnomes : public CreatureScript
{
public:
    boss_brawguild_gnomes() : CreatureScript("boss_brawguild_gnomes") {}

    struct boss_brawguild_gnomesAI : public BrawlersBossAI
    {
        boss_brawguild_gnomesAI(Creature* creature) : BrawlersBossAI (creature) {}

        uint32 healthPct;
        
        void Reset() override
        {           
            events.Reset();
            switch(me->GetEntry())
            {
                case 115292:
                    me->SummonCreature(115294, me->GetPositionX(), me->GetPositionY() + 5.0f, me->GetPositionZ(), me->GetOrientation());
                    me->SummonCreature(115295, me->GetPositionX(), me->GetPositionY() - 5.0f, me->GetPositionZ(), me->GetOrientation());
                    me->SummonCreature(115346, me->GetPositionX() - 5.0f, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    break;
                case 115294:
                    me->SummonCreature(115347, me->GetPositionX() - 5.0f, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    break;
                case 115295:
                    me->SummonCreature(115348, me->GetPositionX() - 5.0f, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/) override
        {
            switch(me->GetEntry())
            {
                case 115292:
                    events.RescheduleEvent(EVENT_1, 20000);
                    break;
                case 115294:
                    events.RescheduleEvent(EVENT_2, 4000);
                    events.RescheduleEvent(EVENT_3, 10000);
                    break;
                case 115295:
                    events.RescheduleEvent(EVENT_4, 7000);
                    break;
            }
        }
        
        void JustDied(Unit* who) override
        {
            // summons.KillAll(); // Crashed
            me->RemoveAllAreaObjects();
            
            if (!who)
                return;

            if (Unit* unit = me->GetAnyOwner())
                if (Creature* owner = unit->ToCreature())
                    if (owner->isAlive())
                        owner->Kill(owner);
            
            _WinRound();
        }
        
        void EnterEvadeMode() override
        {
            // summons.KillAll(); // Crashed
            me->RemoveAllAreaObjects();
			_LoseRound();
        }
        
        void KilledUnit(Unit* who) override
        {
            if (Unit* owner = me->GetAnyOwner())
                if (who->GetGUID() == owner->GetGUID())
                {
                    _LoseRound();
                     // summons.KillAll(); // Crashed
                    me->RemoveAllAreaObjects();
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
                        DoCast(SPELL_PREPARED_TO_SPELL);
                        DoCast(SPELL_WHIRLWIND);
                        events.RescheduleEvent(EVENT_1, 20000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_LAVA_BURST);
                        events.RescheduleEvent(EVENT_2, 4000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_MOLTEN_SLAG);
                        events.RescheduleEvent(EVENT_3, 10000);
                        break;
                    case EVENT_4:
                        me->CastSpell(me->getVictim(), SPELL_LIGHTING_CRASH);
                        events.RescheduleEvent(EVENT_4, 7500);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_gnomesAI (creature);
    }
};

void AddSC_brawlers_guild_bosses_rank_one()
{
    new boss_brawguild_ooliss();    
    new boss_brawguild_grandpa();
    new boss_brawguild_oso();
    new boss_brawguild_gnomes();
};