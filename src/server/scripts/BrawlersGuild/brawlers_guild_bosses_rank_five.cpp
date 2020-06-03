#include "AchievementMgr.h"
#include "AreaTriggerAI.h"
#include "BrawlersGuild.h"

enum eSpells
{
    //! 5 rank
    // thwack
    SPELL_FIRE_BALL         = 228867, // 4
    SPELL_SECOND_DIE        = 229736,
    SPELL_SIT_DOWN          = 229152, // 16-18
    
    
    // general
    SPELL_CALL_WAVE         = 231711,  // 60 duration 36
    SPELL_CALL_WAVE_2       = 231713,  // 60 duration 15
    
    // razorgrin
    SPELL_BITE              = 142734,
    
    // 5 gnomes
    SPELL_LEPEROUS_SPEW     = 133157,
    SPELL_SPRINT            = 133171,
    
    // blackmange
    SPELL_CHARRRGE          = 228826,
    SPELL_SHOT_VISUAL       = 228815,
};

float coordinates[4]
{
    -142.39f, // x line AL
    2485.23f,  // y start AL 
    
    2011.79f, // x start H
    -4734.03f // y line H
    
};

uint32 timer_for_summon[5]
{
    6000,
    10000,
    10000,
    30000,
    16000
};

// 115432 controller
class npc_brawguild_thwack_u_controller : public CreatureScript
{
public:
    npc_brawguild_thwack_u_controller() : CreatureScript("npc_brawguild_thwack_u_controller") {}

    struct npc_brawguild_thwack_u_controllerAI : public BrawlersBossAI
    {
        npc_brawguild_thwack_u_controllerAI(Creature* creature) : BrawlersBossAI(creature), summonsGO(me) {}
        
        SummonListGO summonsGO;
        GuidList special_summons;
        uint8 phase;
        
        void Reset() override
        {    
            phase = 0;
            events.Reset();
            events.RescheduleEvent(EVENT_1, 5000);
            
            events.RescheduleEvent(EVENT_3, timer_for_summon[phase++]);
            
            me->SetReactState(REACT_PASSIVE);

            if (Unit* unit = me->GetAnyOwner())
            if (Player* owner = unit->ToPlayer())
            {
                if (owner->GetTeamId() == TEAM_ALLIANCE)
                {                   
                    // dancers
                    if (Creature* cre = me->SummonCreature(115717, me->GetPositionX() + 5.0f, me->GetPositionY() + 5.0f, me->GetPositionZ(), 3.14f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115712, me->GetPositionX() + 5.0f, me->GetPositionY() - 5.0f, me->GetPositionZ(), 3.14f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115662, me->GetPositionX() + 6.0f, me->GetPositionY() - 8.0f, me->GetPositionZ(), 3.14f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115639, me->GetPositionX() + 6.0f, me->GetPositionY() + 8.0f, me->GetPositionZ(), 3.14f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115654, me->GetPositionX() + 7.0f, me->GetPositionY(), me->GetPositionZ(), 3.14f))
                        special_summons.push_back(cre->GetGUID());
                    
                    // bosses
                    if (Creature* add = owner->SummonCreature(114945, me->GetPositionX() + 4.0f, me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                    {
                        add->AI()->AttackStart(owner);
                        summons.Summon(add);
                    }
                    if (Creature* add = owner->SummonCreature(114944, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                    {
                        add->AI()->AttackStart(owner);
                        summons.Summon(add);
                    }
                    
                    // sounds
                    for (uint8 x = 0; x <= 6; ++x)
                        me->SummonCreature(114953, coordinates[0], coordinates[1] + 6*x, me->GetPositionZ(), me->GetOrientation());
                    
                    // decor
                    me->SummonGameObject(265513, me->GetPositionX() + 2.0f, me->GetPositionY(), me->GetPositionZ(), 3.14f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    me->SummonGameObject(265576, me->GetPositionX() - 5.0f, me->GetPositionY(), me->GetPositionZ(), 3.14f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    me->SummonGameObject(265555, me->GetPositionX() - 5.0f, me->GetPositionY() - 4.0f, me->GetPositionZ(), 3.14f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    me->SummonGameObject(265555, me->GetPositionX() - 5.0f, me->GetPositionY() + 4.0f, me->GetPositionZ(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                }
                else
                {
                    // dancers
                    if (Creature* cre = me->SummonCreature(115717, me->GetPositionX() + 5.0f, me->GetPositionY() - 5.0f, me->GetPositionZ(), 1.57f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115712, me->GetPositionX() - 5.0f, me->GetPositionY() - 5.0f, me->GetPositionZ(), 1.57f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre =  me->SummonCreature(115662, me->GetPositionX() - 8.0f, me->GetPositionY() - 6.0f, me->GetPositionZ(), 1.57f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115639, me->GetPositionX() + 8.0f, me->GetPositionY() - 6.0f, me->GetPositionZ(), 1.57f))
                        special_summons.push_back(cre->GetGUID());
                    if (Creature* cre = me->SummonCreature(115654, me->GetPositionX(), me->GetPositionY() - 7.0f, me->GetPositionZ(), 1.57f))
                        special_summons.push_back(cre->GetGUID());
                    
                    // bosses
                    if (Creature* add = owner->SummonCreature(114945, me->GetPositionX(), me->GetPositionY() - 4.0f, me->GetPositionZ(), me->GetOrientation()))
                    {
                        add->AI()->AttackStart(owner);
                        summons.Summon(add);
                    }
                    if (Creature* add = owner->SummonCreature(114944, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                    {
                        add->AI()->AttackStart(owner);
                        summons.Summon(add);
                    }
                    
                    // sounds
                    for (uint8 x = 0; x <= 6; ++x)
                        me->SummonCreature(114953, coordinates[2] + 6*x, coordinates[3] , me->GetPositionZ(), me->GetOrientation());
                    
                    // decor
                    me->SummonGameObject(265513, me->GetPositionX(), me->GetPositionY() - 2.0f, me->GetPositionZ(), 1.57f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    me->SummonGameObject(265576, me->GetPositionX(), me->GetPositionY() + 5.0f, me->GetPositionZ(), 1.57f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    me->SummonGameObject(265555, me->GetPositionX() - 4.0f, me->GetPositionY() + 5.0f, me->GetPositionZ(), 1.57f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                    me->SummonGameObject(265555, me->GetPositionX() + 4.0f, me->GetPositionY() + 5.0f, me->GetPositionZ(), 1.57f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                }

            }
        }
        
        void EnterEvadeMode() override {}
        void EnterCombat(Unit* who) override {}
                
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            
            if (summon->GetEntry() != 114945 && summon->GetEntry() != 114944)
                summon->AddDelayedEvent(1000, [summon] () -> void
                {
                    summon->SetReactState(REACT_PASSIVE);
                });
            else
                summon->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
        }
        
        void JustSummonedGO(GameObject* go) override
        {
            summonsGO.Summon(go);
        }
               
        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
            summonsGO.DespawnAll();
        }
        
        void UpdateAI(uint32 diff) override
        {                       
            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_CALL_WAVE);
                        events.RescheduleEvent(EVENT_2, 42000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_CALL_WAVE_2);
                        events.RescheduleEvent(EVENT_1, 20000);
                        break;
                    case EVENT_3:
                        {
                            if (Creature* dancer = me->GetMap()->GetCreature(special_summons.front()))
                            {
                                special_summons.remove(dancer->GetGUID());
                                dancer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
                                dancer->SetReactState(REACT_AGGRESSIVE);
                                if (Unit* owner = me->GetAnyOwner())
                                    dancer->AI()->AttackStart(owner);
                                dancer->AI()->Talk(0);
                                
                                if (Vehicle* veh = dancer->GetVehicleKit()) // it's need to remove flag from all passengers
                                {
                                    Unit* passenger = NULL;
                                    uint8 seat = 0;
                                    do
                                    {
                                        passenger = veh->GetPassenger(seat++);
                                        if (passenger)
                                        {
                                            passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
                                            if (Unit* owner = me->GetAnyOwner())
                                                passenger->ToCreature()->AI()->AttackStart(owner);
                                        }else
                                            break;
                                    } while (passenger != NULL);
                                }
                            }
                            if (phase < 5)
                                events.RescheduleEvent(EVENT_3, timer_for_summon[phase++]);
                        }
                        break;
                }
            }

        }
        
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_brawguild_thwack_u_controllerAI (creature);
    }
};

// 114944 (S) + 114945(W)
class boss_brawguild_thwack_u : public CreatureScript
{
public:
    boss_brawguild_thwack_u() : CreatureScript("boss_brawguild_thwack_u") {}

    struct boss_brawguild_thwack_uAI : public BrawlersBossAI
    {
        boss_brawguild_thwack_uAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {
            events.Reset();   
        }
        
        void EnterCombat(Unit* who) override
        {           
            if (me->GetEntry() == 114944)
                events.RescheduleEvent(EVENT_1, 4000);
            else 
                events.RescheduleEvent(EVENT_2, urand(16000, 18000));
        }
                
        void JustDied(Unit* who) override
        {                        
            if (!me->HasAura(SPELL_SECOND_DIE))
                if (Creature* other = me->FindNearestCreature((me->GetEntry() == 114944 ? 114945 : 114944), 200.0f, true))
                {
                    other->AddAura(SPELL_SECOND_DIE, other);
                    return;
                }
            
            _Reset();
           _WinRound();
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
                        DoCast(SPELL_FIRE_BALL);
                        events.RescheduleEvent(EVENT_1, 4000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_SIT_DOWN);
                        events.RescheduleEvent(EVENT_2, urand(16000, 18000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_thwack_uAI (creature);
    }
};

// 71085
class boss_brawguild_razorgrin : public CreatureScript
{
public:
    boss_brawguild_razorgrin() : CreatureScript("boss_brawguild_razorgrin") {}

    struct boss_brawguild_razorgrinAI : public BrawlersBossAI
    {
        boss_brawguild_razorgrinAI(Creature* creature) : BrawlersBossAI(creature) {}

        void EnterCombat(Unit* /*who*/) override {}
        
        void KilledUnit(Unit* who) override
        {
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsCreature())
                me->GetAnyOwner()->ToCreature()->AI()->KilledUnit(who); // some hack
            
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsPlayer())
            {
                Player* player = me->GetAnyOwner()->ToPlayer();
                player->AddDelayedEvent(1000, [player] () -> void
                {
                    if (player)
                        if (BrawlersGuild* brawlerGuild = player->GetBrawlerGuild())
                            brawlerGuild->BossReport(player->GetGUID(), false);
                });
            }
        }   

        void JustDied(Unit* who) override
        {
            if (!who)
                return;
            
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsCreature())
                me->GetAnyOwner()->ToCreature()->AI()->JustDied(who); // some hack
            
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsPlayer())
            {
                Player* player = me->GetAnyOwner()->ToPlayer();
                player->AddDelayedEvent(1000, [player] () -> void
                {
                    if (player)
                        if (BrawlersGuild* brawlerGuild = player->GetBrawlerGuild())
                            brawlerGuild->BossReport(player->GetGUID(), true);
                });
            }
        }   

        void EnterEvadeMode() override
        {            
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsCreature())
                me->GetAnyOwner()->ToCreature()->AI()->EnterEvadeMode(); // some hack
            
            if (me->GetAnyOwner() && me->GetAnyOwner()->IsPlayer())
            {
                Player* player = me->GetAnyOwner()->ToPlayer();
                player->AddDelayedEvent(1000, [player] () -> void
                {
                    if (player)
                        if (BrawlersGuild* brawlerGuild = player->GetBrawlerGuild())
                            brawlerGuild->BossReport(player->GetGUID(), false);
                });
            }
        }          
 
        void UpdateAI(uint32 diff) override
        {            
            if (!UpdateVictim())
                return;
            
            // DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_razorgrinAI (creature);
    }
};

uint32 gnomes[5]
{
    67515, 67514, 67513, 67516, 67511
};
// 67509
class boss_brawguild_five_gnomes_controller : public CreatureScript
{
public:
    boss_brawguild_five_gnomes_controller() : CreatureScript("boss_brawguild_five_gnomes_controller") {}

    struct boss_brawguild_five_gnomes_controllerAI : public BrawlersBossAI
    {
        boss_brawguild_five_gnomes_controllerAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        uint8 count_adds;
        void Reset() override
        {  
            count_adds = 0;
            for (uint32 id : gnomes)
                me->SummonCreature(id, me->GetPositionX() + irand(-5, 5), me->GetPositionY() + irand(-5, 5), me->GetPositionZ(), me->GetOrientation());
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->setRegeneratingHealth(false);
            summon->SetHealth(summon->GetMaxHealth()*(0.6+0.1*count_adds));
            count_adds++;
        }
        
        void EnterCombat(Unit* who) override {}
        
        void EnterEvadeMode() override {}
                
        void JustDied(Unit* who) override
        {                                   
            _Reset();
        }
        
        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            count_adds--;
            if (count_adds <= 0)
            {
                _Reset();
                _WinRound();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_five_gnomes_controllerAI (creature);
    }
};

// 67515, 67514, 67513, 67516, 67511
class boss_brawguild_five_gnomes : public CreatureScript
{
public:
    boss_brawguild_five_gnomes() : CreatureScript("boss_brawguild_five_gnomes") {}

    struct boss_brawguild_five_gnomesAI : public BrawlersBossAI
    {
        boss_brawguild_five_gnomesAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {
            events.Reset();   
        }
        
        void EnterCombat(Unit* who) override
        {           
            events.RescheduleEvent(EVENT_1, 4000);
            events.RescheduleEvent(EVENT_2, urand(3000, 8000));
        }
                
        void JustDied(Unit* who) override
        {                                    
            _Reset();
        }
        
        void KilledUnit(Unit* who) override
        {
            if (Unit* owner = me->GetAnyOwner())
                if (owner->IsCreature())
                    owner->ToCreature()->AI()->KilledUnit(who); // some hack
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
                        DoCast(SPELL_LEPEROUS_SPEW);
                        events.RescheduleEvent(EVENT_1, 4000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_SPRINT);
                        events.RescheduleEvent(EVENT_2, urand(3000, 8000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_five_gnomesAI (creature);
    }
};

// 114902
class boss_brawguild_blackmange : public CreatureScript
{
public:
    boss_brawguild_blackmange() : CreatureScript("boss_brawguild_blackmange") {}

    struct boss_brawguild_blackmangeAI : public BrawlersBossAI
    {
        boss_brawguild_blackmangeAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        void Reset() override
        {
            events.Reset();   
        }
        
        void EnterCombat(Unit* who) override
        {
            if (Unit* unit = me->GetAnyOwner())
                if (Player* owner = unit->ToPlayer())
                {
                    bool isAllance = owner->GetTeamId() == TEAM_ALLIANCE;
                    for (int8 x = -3; x <= 16; ++x)
                        if (isAllance)
                            me->SummonCreature(114907, coordinates[0], coordinates[1] + 2*x , me->GetPositionZ(), me->GetOrientation());        
                        else
                            me->SummonCreature(114907, coordinates[2] + 2*x + 10.0f, coordinates[3] , me->GetPositionZ(), me->GetOrientation());        
                }
            events.RescheduleEvent(EVENT_1, 21000);
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            if (me->isInCombat())
                DoZoneInCombat(summon);
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
                        DoCast(SPELL_CHARRRGE);
                        events.RescheduleEvent(EVENT_1, urand(19000, 21000));
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_blackmangeAI (creature);
    }
};

uint32 SPELL_FUSE[3]
{
    228795,
    228820,
    228821
};

// 114907
class boss_brawguild_blackmange_guns : public CreatureScript
{
public:
    boss_brawguild_blackmange_guns() : CreatureScript("boss_brawguild_blackmange_guns") {}

    struct boss_brawguild_blackmange_gunsAI : public ScriptedAI
    {
        boss_brawguild_blackmange_gunsAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();   
            events.RescheduleEvent(EVENT_1, 6000);
        }
                
        void SpellFinishCast(const SpellInfo* spell) override
        {
            if (spell->Id == 228812)
            {
                DoCast(SPELL_SHOT_VISUAL);
                events.RescheduleEvent(EVENT_1, 2000);
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
                        if (!me->GetAnyOwner() || !me->GetAnyOwner()->isAlive())
                            me->DespawnOrUnsummon();
                        else
                            DoCast(SPELL_FUSE[urand(0, 2)]);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_blackmange_gunsAI (creature);
    }
};

// 7813
class areatrigger_at_fuse : public AreaTriggerScript
{
    public:
        areatrigger_at_fuse() : AreaTriggerScript("areatrigger_at_fuse") { }

    struct areatrigger_at_fuseAI : AreaTriggerAI
    {
        areatrigger_at_fuseAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

        bool CalculateSpline(Position const* pos, Position& startPos, Position& endPos, std::vector<Position>& path) override
        {
            startPos.m_positionX = pos->GetPositionX();
            startPos.m_positionY = pos->GetPositionY();
            startPos.m_positionZ = pos->GetPositionZ();
            if (Unit* caster = at->GetCaster())
            {
                endPos.m_positionX = caster->GetPositionX();
                endPos.m_positionY = caster->GetPositionY();
                endPos.m_positionZ = caster->GetPositionZ();
            }
            return true;
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_fuseAI(areatrigger);
    }
};

void AddSC_brawlers_guild_bosses_rank_five()
{
    new npc_brawguild_thwack_u_controller();
    new boss_brawguild_thwack_u();
    new boss_brawguild_razorgrin();
    new boss_brawguild_five_gnomes_controller();
    new boss_brawguild_five_gnomes();
    new boss_brawguild_blackmange();
    new boss_brawguild_blackmange_guns();
    new areatrigger_at_fuse();
};