#include "AchievementMgr.h"
#include "AreaTriggerAI.h"
#include "BrawlersGuild.h"

enum eSpells
{
    //! 6 rank
    // topps
    SPELL_CHARGE            = 232252,
    SPELL_STUN              = 232285,  
    SPELL_FAKE_STUN         = 234501,

    // mille
    SPELL_PHOTOPLASM_BUSTER_RAY =133357,  // 8
    SPELL_ELECTRIC_DYNAMITE = 133363, // 9
    SPELL_TRANSFORM         = 133362, // 33
    
    // carl
    SPELL_LAVA_SPIT         = 229398, // 2
    SPELL_TELEPORT          = 229358, // 33
    SPELL_LAVA_AT           = 229269,  // 16
    SPELL_LAVA_WAVES        = 229328,  // 115275
    
    //! ogrewatch
    
    // hudson
    SPELL_PROTECT_SHIELD    = 230366, 
    SPELL_PROTECT_SHIELD_BUFF = 229893,
    SPELL_JUMP_PACK         = 230373, // 13
    SPELL_JUMP_PACK_DMG     = 230374,
    SPELL_TESLA_GUN         = 230372, // after pack
    // dupree
    SPELL_HIGH_NOON         = 229154,
    //stuffshrew
    SPELL_FRAG_LAUNCHER     = 234683, // 6
    SPELL_MANICAL_LAUGH     = 229998, // 20
};


Position walls_pos_a[4]
{
    {-95.949471f, 2481.682617f, -49.10f, 0.0f},
    {-143.886841f, 2481.684082f, -49.10f, 0.0f},
    {-143.886703f, 2517.563721f, -49.10f, 0.0f},
    {-95.949265f, 2517.562744f, -49.10f, 0.0f}            
    
};

Position walls_pos_h[4]
{
    {2010.239258f, -4774.575684f, 86.77f, 0.0f},
    {2009.239624f, -4732.524902f, 86.77f, 0.0f},
    {2051.645752f, -4730.982422f, 86.77f, 0.0f},
    {2052.84f, -4773.460449f, 86.77f, 0.0f}

};


// 116539
class boss_brawguild_topps : public CreatureScript
{
public:
    boss_brawguild_topps() : CreatureScript("boss_brawguild_topps") {}

    struct boss_brawguild_toppsAI : public BrawlersBossAI
    {
        boss_brawguild_toppsAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        Position pos;
        bool need_stop;

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 3000);
        }     

        void SpellFinishCast(const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_CHARGE)
            {
                need_stop = false;
                events.RescheduleEvent(EVENT_2, 500);
                if (Unit* target = me->GetAnyOwner())
                    if (target->IsInBetween(&pos, me, 3.0f))
                        me->CastSpell(target, 232267);
            }
                
            if (spell->Id == 232261)
            {
                need_stop = true;
                if (urand(1, 2) == 2)
                {
                    DoCast(SPELL_STUN);
                    events.RescheduleEvent(EVENT_1, 13000);
                }
                else
                {
                    DoCast(SPELL_FAKE_STUN);
                    events.RescheduleEvent(EVENT_1, 2000);
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
                        if (Unit* unit = me->GetAnyOwner())
                        if (Player* owner = unit->ToPlayer())
                        {
                            pos = owner->GetPosition();
                            float switch_x = pos.GetPositionX() - me->GetPositionX();
                            float abs_switch_x = (switch_x < 0 ? switch_x * -1 : switch_x); 
                            float switch_y = pos.GetPositionY() - me->GetPositionY();
                            float abs_switch_y = (switch_y < 0 ? switch_y * -1 : switch_y); 
                            pos.Relocate(pos.GetPositionX() + (switch_x/abs_switch_x)*70.0f,  pos.GetPositionY() + (switch_y/abs_switch_y)*70.0f);
                            bool can_cast = false;
                            if (owner->GetTeamId() == TEAM_ALLIANCE)
                            {
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_a[0], walls_pos_a[1], &pos);
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_a[1], walls_pos_a[2], &pos);
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_a[2], walls_pos_a[3], &pos);
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_a[3], walls_pos_a[0], &pos);
                                // todo bleat
                            }
                            else
                            {
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_h[0], walls_pos_h[1], &pos);
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_h[1], walls_pos_h[2], &pos);
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_h[2], walls_pos_h[3], &pos);
                                if (!can_cast)
                                    can_cast = pos.IsLinesCross(me->GetPosition(), pos, walls_pos_h[3], walls_pos_h[0], &pos);
                            }
                            if (can_cast)
                                me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_CHARGE);
                        }
                        break;
                    case EVENT_2:
                        DoCast(232261);
                        if (!need_stop)
                            events.RescheduleEvent(EVENT_2, 400);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_toppsAI (creature);
    }
};

// 67591
class boss_brawguild_millie : public CreatureScript
{
public:
    boss_brawguild_millie() : CreatureScript("boss_brawguild_millie") {}

    struct boss_brawguild_millieAI : public BrawlersBossAI
    {
        boss_brawguild_millieAI(Creature* creature) : BrawlersBossAI(creature) {}

        void Reset() override
        {
            
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);      
            events.Reset();  
        }
        
        void EnterCombat(Unit* /*who*/) override
        {
            Talk(0);
            events.RescheduleEvent(EVENT_1, 4000);
            events.RescheduleEvent(EVENT_3, 33000);
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
                        DoCast(SPELL_PHOTOPLASM_BUSTER_RAY);
                        events.RescheduleEvent(EVENT_2, 5200);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_ELECTRIC_DYNAMITE);
                        events.RescheduleEvent(EVENT_1, 1100);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_TRANSFORM);
                        Talk(1);
                        events.RescheduleEvent(EVENT_3, 33000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_millieAI (creature);
    }
};

Position carl_triggers_a[29]
{
    {-135.789f, 2487.17f, -49.1092f, 0.165552f},
    {-128.897f, 2488.32f, -49.1092f, 0.544114f},
    {-124.453f, 2485.53f, -49.1092f, 5.97907f},
    {-118.105f, 2488.39f, -49.1092f, 0.736539f},
    {-117.762f, 2494.25f, -49.1092f, 2.40944f},
    {-126.628f, 2500.62f, -49.1092f, 3.1163f},
    {-133.444f, 2503.56f, -49.1092f, 2.10706f},
    {-132.926f, 2510.4f, -49.1092f, 0.960378f},
    {-127.796f, 2511.66f, -49.1092f, 6.2304f},
    {-121.269f, 2509.18f, -49.1092f, 5.05623f},
    {-117.108f, 2507.36f, -49.1092f, 0.650147f},
    {-112.544f, 2510.85f, -49.1092f, 0.292791f},
    {-107.053f, 2511.66f, -49.1092f, 0.265302f},
    {-102.265f, 2511.46f, -49.1092f, 5.05231f},
    {-102.696f, 2507.0f, -49.1092f, 3.85065f},
    {-108.629f, 2504.03f, -49.1092f, 3.71321f},
    {-109.969f, 2498.69f, -49.1092f, 5.79451f},
    {-104.273f, 2494.73f, -49.1092f, 5.53533f},
    {-101.358f, 2488.89f, -49.1092f, 4.30619f},
    {-109.798f, 2488.7f, -49.1092f, 2.37803f},
    {-129.522f, 2496.73f, -49.1092f, 2.47621f},
    {-118.082f, 2515.04f, -49.1092f, 1.40414f},
    {-139.94f, 2495.33f, -49.1092f, 3.13987f},
    {-140.715f, 2508.98f, -49.1092f, 2.97101f},
    {-126.587f, 2515.96f, -49.1092f, 6.26968f},
    {-134.189f, 2493.67f, -49.1092f, 5.67277f},
    {-124.755f, 2493.05f, -49.1092f, 0.43416f},
    {-99.1994f, 2500.49f, -49.1092f, 0.182832f},
    {-110.271f, 2515.95f, -49.1092f, 2.56652f}
};


Position carl_triggers_h[22]
{
    {2015.38f, -4769.42f, 86.775f, 1.81692f},
    {2021.86f, -4768.03f, 86.775f, 0.493525f},
    {2031.05f, -4769.64f, 86.775f, 6.23479f},
    {2043.8f, -4769.7f, 86.775f, 0.159728f},
    {2042.17f, -4760.11f, 86.775f, 1.7855f},
    {2049.1f, -4753.57f, 86.775f, 1.49098f},
    {2040.52f, -4749.27f, 86.775f, 3.211f},
    {2032.81f, -4755.05f, 86.775f, 3.92179f},
    {2024.28f, -4751.45f, 86.775f, 2.89292f},
    {2016.96f, -4754.97f, 86.775f, 3.5448f},
    {2014.17f, -4745.47f, 86.775f, 1.12577f},
    {2021.18f, -4744.01f, 86.775f, 6.05022f},
    {2027.07f, -4739.76f, 86.775f, 0.13224f},
    {2032.29f, -4743.5f, 86.775f, 6.09341f},
    {2039.53f, -4741.84f, 86.775f, 0.524938f},
    {2044.74f, -4742.26f, 86.775f, 6.17588f},
    {2048.24f, -4734.57f, 86.775f, 2.6141f},
    {2040.81f, -4734.18f, 86.775f, 3.42306f},
    {2033.38f, -4733.84f, 86.775f, 2.84579f},
    {2025.06f, -4735.86f, 86.775f, 3.65082f},
    {2013.83f, -4735.98f, 86.775f, 4.70326f},
    {2022.66f, -4759.84f, 86.775f, 5.36692f}
};


// 115233
class boss_brawguild_carl : public CreatureScript
{
public:
    boss_brawguild_carl() : CreatureScript("boss_brawguild_carl") {}

    struct boss_brawguild_carlAI : public BrawlersBossAI
    {
        boss_brawguild_carlAI(Creature* creature) : BrawlersBossAI(creature) {}

        void Reset() override
        {
            
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);      
            events.Reset();  
        }
        
        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_1, 2000);
            events.RescheduleEvent(EVENT_2, 33000);
            events.RescheduleEvent(EVENT_3, 16000);
            if (Unit* unit = me->GetAnyOwner())
            if (Player* owner = unit->ToPlayer())
            {
                if (owner->GetTeamId() == TEAM_ALLIANCE)
                {
                    for (uint8 i = 0; i < 29; ++i)
                        me->SummonCreature(115275, carl_triggers_a[i].GetPositionX(), carl_triggers_a[i].GetPositionY(), carl_triggers_a[i].GetPositionZ(), 0.0f);
                }
                else
                {
                    for (uint8 i = 0; i < 22; ++i)
                        me->SummonCreature(115275, carl_triggers_h[i].GetPositionX(), carl_triggers_h[i].GetPositionY(), carl_triggers_h[i].GetPositionZ(), 0.0f);
                }
            }
        }     
        
        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id == 229253)
                if (target->GetEntry() == 115275)
                   {
                       target->SetOrientation(0);
                       target->CastSpell(target, SPELL_LAVA_WAVES);
                       
                       target->AddDelayedEvent(10, [target] () -> void
                       {
                           
                            target->SetOrientation(1.57f);
                            target->CastSpell(target, SPELL_LAVA_WAVES);
                       });
                       
                       target->AddDelayedEvent(20, [target] () -> void
                       {
                            target->SetOrientation(3.14f);
                            target->CastSpell(target, SPELL_LAVA_WAVES);
                       });
                       
                       target->AddDelayedEvent(30, [target] () -> void
                       {
                            target->SetOrientation(4.71f);
                            target->CastSpell(target, SPELL_LAVA_WAVES);
                       });
                   }
                
            if (spell->Id == SPELL_TELEPORT)
            {
                // hack
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);      
                me->NearTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);  
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
                        DoCast(SPELL_LAVA_SPIT);
                        events.RescheduleEvent(EVENT_1, 2000);
                        break;
                    case EVENT_2:
                    {
                        std::list<Creature*> adds;
                        GetCreatureListWithEntryInGrid(adds, me, 115275, 70.0f);
                        if (!adds.empty())
                            for (std::list<Creature*>::iterator itr = adds.begin(); itr != adds.end(); ++itr)
                                if (me->GetDistance2d(*itr) > 15.0f)
                                {
                                    if (!(*itr)->HasAura(229253))
                                    {
                                        DoCast(*itr, SPELL_TELEPORT);
                                        (*itr)->DespawnOrUnsummon(5000);
                                        break;
                                    }
                                }
                        events.RescheduleEvent(EVENT_2, 33000);        
                        break;
                    }
                    case EVENT_3:
                        DoCast(SPELL_LAVA_AT);
                        events.RescheduleEvent(EVENT_3, 16000);
                        break;
                        
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_carlAI (creature);
    }
};

uint32 ogrewatch[3]
{
    114951,
    114955,
    114941
};

// 115796
class npc_brawguild_ogrewatch_controller : public CreatureScript
{
public:
    npc_brawguild_ogrewatch_controller() : CreatureScript("npc_brawguild_ogrewatch_controller") {}

    struct npc_brawguild_ogrewatch_controllerAI : public BrawlersBossAI
    {
        npc_brawguild_ogrewatch_controllerAI(Creature* creature) : BrawlersBossAI(creature) {}
        
        uint8 count_adds;
        void Reset() override
        {  
            count_adds = 3;
            for (uint32 id : ogrewatch)
                me->SummonCreature(id, me->GetPositionX() + irand(-5, 5), me->GetPositionY() + irand(-5, 5), me->GetPositionZ(), me->GetOrientation());
        }
        
        void EnterCombat(Unit* who) override {}
                
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
        return new npc_brawguild_ogrewatch_controllerAI (creature);
    }
};

// 114951, 114955, 114941
class boss_brawguild_ogrewatch : public CreatureScript
{
public:
    boss_brawguild_ogrewatch() : CreatureScript("boss_brawguild_ogrewatch") {}

    struct boss_brawguild_ogrewatchAI : public BrawlersBossAI
    {
        boss_brawguild_ogrewatchAI(Creature* creature) : BrawlersBossAI(creature) {}

        void Reset() override
        {     
            events.Reset();  
            me->ApplySpellImmune(SPELL_MANICAL_LAUGH, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        }
        
        void EnterCombat(Unit* who) override
        {
            switch(me->GetEntry())
            {
                case 114951:
                    events.RescheduleEvent(EVENT_5, 1000);
                    events.RescheduleEvent(EVENT_1, 13000);
                    break;
                case 114955:
                    DoCast(who, SPELL_HIGH_NOON);
					events.RescheduleEvent(EVENT_7, 80000);
                    me->SetReactState(REACT_PASSIVE);
                    break;
                case 114941:
                    events.RescheduleEvent(EVENT_3, 6000);
                    events.RescheduleEvent(EVENT_4, 20000);
                    break;
            }
        }     
        
    
        void KilledUnit(Unit* who) override
        {
            if (Unit* owner = me->GetAnyOwner())
                if (owner->IsCreature())
                    owner->ToCreature()->AI()->KilledUnit(who); // some hack
        }
 
        void SpellFinishCast(const SpellInfo* spell) override
        {
            switch(spell->Id)
            {
                case SPELL_JUMP_PACK_DMG:
                    events.RescheduleEvent(EVENT_2, 1000);
                    break;
            }
        }
        
        
        void UpdateAI(uint32 diff) override
        {                        
            if (!UpdateVictim())
                return;

            if (me->HasAura(SPELL_PROTECT_SHIELD))
                return;
            
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_JUMP_PACK);
                        events.RescheduleEvent(EVENT_1, 13000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_TESLA_GUN);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_FRAG_LAUNCHER);
                        events.RescheduleEvent(EVENT_3, 6000);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_MANICAL_LAUGH);
                        events.RescheduleEvent(EVENT_4, 20000);
                        break;
                    case EVENT_5:
                        DoCast(SPELL_PROTECT_SHIELD);    
                        me->AddDelayedEvent(100, [this] () -> void
                        {
                            me->RemoveAura(SPELL_PROTECT_SHIELD_BUFF);
                        });
                        events.RescheduleEvent(EVENT_6, 1000);
                        break;
                    case EVENT_6:
                        if (me->HasAura(SPELL_PROTECT_SHIELD))
                            events.RescheduleEvent(EVENT_6, 1000);
                        else
                        {
                            me->RemoveAura(SPELL_PROTECT_SHIELD_BUFF);
                            for (uint32 id : ogrewatch)
                                if (id != me->GetEntry())
                                    if (Creature* targ = me->FindNearestCreature(id, 100.0f, true))
                                        targ->RemoveAura(SPELL_PROTECT_SHIELD_BUFF);
                        }
                        break;
                    case EVENT_7:
                        if (Unit* owner = me->GetAnyOwner())
                            owner->ToCreature()->AI()->EnterEvadeMode();
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_brawguild_ogrewatchAI (creature);
    }
};


void AddSC_brawlers_guild_bosses_rank_six()
{
    new boss_brawguild_topps();
    new boss_brawguild_millie();
    new boss_brawguild_carl();
    new npc_brawguild_ogrewatch_controller();
    new boss_brawguild_ogrewatch();
};