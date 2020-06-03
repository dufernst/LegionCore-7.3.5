#include "the_vortex_pinnacle.h"


// Перепрыгивания в вихрях пока не реализованы

enum Creatures
{
    NPC_SLIPSTREAM                  = 45455,
    NPC_SLIPSTREAM_LANDING_ZONE     = 45504,
    NPC_SKYFALL_LAND                = 45981,

    NPC_CLOUD_PRINCE                = 45917,
    NPC_WIPPING_WIND                = 47238,
    NPC_YOUNG_STORM_DRAGON          = 45919,
    NPC_ARMORED_MISTRAL             = 45915,
    NPC_EMPYREAN_ASSASSIN           = 45922,
    NPC_EXECUTOR_OF_THE_CALIPH      = 45928,
    NPC_GUST_SOLDIER                = 45477,
    NPC_LURKING_TEMPEST             = 45704,
    NPC_MINISTER_OF_AIR             = 45930,
    NPC_SERVANT_OF_ASAAD            = 45926,
    NPC_TEMPEST_ADEPT               = 45935,
    NPC_TURBULENT_SQUALL            = 45924,
    NPC_HOWLING_GALE                = 45572,
    NPC_WILD_VORTEX                 = 45912,
    NPC_SKYFALL_STAR                = 45932,
    NPC_GOLDEN_ORB                  = 51157,
    NPC_ITESH                       = 49943
};

enum Spells
{
    //slipstream
    SPELL_SLIPSTREAM_AURA       = 85021,
    SPELL_SLIPSTREAM_VEH        = 87742,

    //cloud prince
    SPELL_TYPHOON               = 88074,
    SPELL_STARFALL              = 88073,

    //whipping wind
    SPELL_WHW_LIGHTNING_BOLT    = 88080,

    //young storm dragon
    SPELL_BRUTAL_STRIKES        = 88192,
    SPELL_BRUTAL_STRIKES_DMG    = 88188,
    SPELL_CHILLING_BLAST        = 88194,
    SPELL_HEALING_WELL          = 88201,

    //armored mistral
    SPELL_GALE_STRIKE           = 88061,
    SPELL_RISING_WINDS          = 88057,
    SPELL_STORM_SURGE           = 88055,

    //empyrean assassin
    SPELL_LETHARGIC_POISON      = 88184,
    SPELL_VAPOR_FORM            = 88186,

    //executor of the caliph
    SPELL_DEVASTATE             = 78660,
    SPELL_RALLY                 = 87761,
    SPELL_SHOCKWAVE             = 87759,
    
    //gust soldier
    SPELL_AIR_NOVA              = 87933,
    SPELL_AIR_NOVA_H            = 92753,
    SPELL_CHARGE                = 87930,
    SPELL_WIND_BLAST            = 87923,

    //lurking tempest
    SPELL_LT_LIGHTNING_BOLT     = 89105,
    SPELL_FEIGN_DEATH           = 85267,
    SPELL_LURK                  = 85467,

    //minister of air
    SPELL_LIGHTNING_LASH        = 87762,
    SPELL_LIGHTNING_LASH_DUMMY  = 87765,
    SPELL_LIGHTNING_LASH_DMG    = 88963,
    SPELL_LIGHTNING_NOVA        = 87768,

    //servant of asaad
    SPELL_CRUSADER_STRIKE       = 87771,
    SPELL_DIVINE_STORM          = 58112,
    SPELL_HAND_OF_PROTECTION    = 87772,

    //temple adept
    SPELL_DESPERATE_SPEED       = 87780,
    SPELL_GREATER_HEAL          = 87779,
    SPELL_HOLY_SMITE            = 88959,

    //turbulent squall
    SPELL_ASPHYXIATE            = 88175,
    SPELL_CLOUDBURST            = 88170,
    SPELL_HURRICANE             = 88171,

    //wild vortex
    SPELL_WV_LIGHTNING_BOLT     = 88032,
    SPELL_WIND_SHOCK            = 88029,
    SPELL_CYCLONE               = 88010,

    //skyfall star
    SPELL_ARCANE_BARRAGE        = 87854,

    //howling gale
    SPELL_HOWLING_GALE          = 85084,
    SPELL_HOWLING_GALE_1        = 85086,
    SPELL_HOWLING_GALE_2        = 85136,
    SPELL_HOWLING_GALE_3        = 85086,
    SPELL_HOWLING_GALE_DMG      = 85159
};

enum Events
{
    EVENT_ARCANE_BARRAGE        = 1,
    EVENT_TYPHOON               = 2,
    EVENT_STARFALL              = 3,
    EVENT_WHW_LIGHTNING_BOLT    = 4,
    EVENT_CHILLING_BLAST        = 5,
    EVENT_GALE_STRIKE           = 6,
    EVENT_STORM_SURGE           = 7,
    EVENT_VAPOR_FORM            = 8,
    EVENT_DEVASTATE             = 9,
    EVENT_SHOCKWAVE             = 10,
    EVENT_RALLY                 = 11,
    EVENT_AIR_NOVA              = 12,
    EVENT_WIND_BLAST            = 13,
    EVENT_LIGHTNING_NOVA        = 14,
    EVENT_LIGHTNING_LASH        = 15,
    EVENT_CRUSADER_STRIKE       = 16,
    EVENT_HAND_OF_PROTECTION    = 17,
    EVENT_DESPERATE_SPEED       = 18,
    EVENT_GREATER_HEAL          = 19,
    EVENT_HOLY_SMITE            = 20,
    EVENT_ASPHYXIATE            = 21,
    EVENT_HURRICANE             = 22,
    EVENT_CLOUDBURST            = 23,
    EVENT_CYCLONE               = 24,
    EVENT_WIND_SHOCK            = 25,
    EVENT_WV_LIGHTNING_BOLT     = 26,
    EVENT_HOWLING_GALE          = 27
};

const Position teleportPos[2] = 
{
    {-906.08f, -176.51f, 664.50f, 2.86f},
    {-1193.67f, 472.83f, 634.86f, 0.50f}
};

enum Other
{
    TYPE_SLIPSTREAM    = 1
};

class npc_vortex_pinnacle_slipsteam : public CreatureScript
{
    public:
        npc_vortex_pinnacle_slipsteam() : CreatureScript("npc_vortex_pinnacle_slipsteam") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_vortex_pinnacle_slipsteamAI(pCreature);
        }

        bool OnGossipHello(Player* pPlayer, Creature* creature)
        {
            if (InstanceScript* instance = creature->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_ALTAIRUS) == DONE)
                {
                    pPlayer->NearTeleportTo(
                        teleportPos[1].GetPositionX(),
                        teleportPos[1].GetPositionY(),
                        teleportPos[1].GetPositionZ()+10.0f,
                        teleportPos[1].GetOrientation());
                    return true;
                }
                else if (instance->GetBossState(DATA_ERTAN) == DONE)
                {
                    pPlayer->NearTeleportTo(
                        teleportPos[0].GetPositionX(),
                        teleportPos[0].GetPositionY(),
                        teleportPos[0].GetPositionZ()+10.0f,
                        teleportPos[0].GetOrientation());
                    return true;
                }
            }
            return false;
        }

        struct npc_vortex_pinnacle_slipsteamAI : public Scripted_NoMovementAI
        {
            npc_vortex_pinnacle_slipsteamAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetCanFly(true);
            }

            void Reset() override
            {
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                DoCast(SPELL_SLIPSTREAM_AURA);
            }

            void UpdateAI(uint32 diff) override {}
        };
};

class npc_skyfall_star : public CreatureScript
{
    public:
        npc_skyfall_star() : CreatureScript("npc_skyfall_star") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_skyfall_starAI(pCreature);
        }

        struct npc_skyfall_starAI : public Scripted_NoMovementAI
        {
            npc_skyfall_starAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetCanFly(true);
                me->SetDisableGravity(true);
            }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustDied(Unit* /*who*/) override
            {
                me->SetCanFly(false);
                me->SetDisableGravity(false);
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(5000, 6000));
            }

            void UpdateAI(uint32 diff) override
            { 
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARCANE_BARRAGE:
                            DoCast(SPELL_ARCANE_BARRAGE);
                            events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(10000, 15000));
                            break;
                    }
                }
            }
        };
};

class npc_cloud_prince : public CreatureScript
{
    public:
        npc_cloud_prince() : CreatureScript("npc_cloud_prince") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_cloud_princeAI(pCreature);
        }

        struct npc_cloud_princeAI : public ScriptedAI
        {
            npc_cloud_princeAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_TYPHOON, urand(5000, 7000));
                events.ScheduleEvent(EVENT_STARFALL, urand(7000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_TYPHOON:
                        DoCast(SPELL_TYPHOON);
                        events.ScheduleEvent(EVENT_TYPHOON, urand(15000, 20000));
                        break;
                    case EVENT_STARFALL:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_STARFALL, false);
                        events.ScheduleEvent(EVENT_STARFALL, 20000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_whipping_wind : public CreatureScript
{
    public:
        npc_whipping_wind() : CreatureScript("npc_whipping_wind") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_whipping_windAI(pCreature);
        }

        struct npc_whipping_windAI : public ScriptedAI
        {
            npc_whipping_windAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_WHW_LIGHTNING_BOLT, 2000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_WHW_LIGHTNING_BOLT:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_WHW_LIGHTNING_BOLT, false);
                        events.ScheduleEvent(EVENT_WHW_LIGHTNING_BOLT, 2000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_young_storm_dragon : public CreatureScript
{
    public:
        npc_young_storm_dragon() : CreatureScript("npc_young_storm_dragon") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_young_storm_dragonAI(pCreature);
        }

        struct npc_young_storm_dragonAI : public ScriptedAI
        {
            npc_young_storm_dragonAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            }

            EventMap events;

            void Reset() override
            {
                DoCast(SPELL_BRUTAL_STRIKES);
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                DoCast(SPELL_HEALING_WELL);
                events.ScheduleEvent(EVENT_CHILLING_BLAST, urand(12000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CHILLING_BLAST:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_CHILLING_BLAST, false);
                        events.ScheduleEvent(EVENT_CHILLING_BLAST, urand(15000, 18000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_armored_mistral : public CreatureScript
{
    public:
        npc_armored_mistral() : CreatureScript("npc_armored_mistral") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_armored_mistralAI(pCreature);
        }

        struct npc_armored_mistralAI : public ScriptedAI
        {
            npc_armored_mistralAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_GALE_STRIKE, urand(2000, 4000));
                events.ScheduleEvent(EVENT_STORM_SURGE, urand(10000, 15000));    
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_GALE_STRIKE:
                        DoCast(SPELL_GALE_STRIKE);
                        events.ScheduleEvent(EVENT_GALE_STRIKE, urand(15000, 20000));
                        break;
                    case EVENT_STORM_SURGE:
                        DoCast(SPELL_STORM_SURGE);
                        events.ScheduleEvent(EVENT_STORM_SURGE, urand(15000, 20000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_empyrean_assassin : public CreatureScript
{
    public:
        npc_empyrean_assassin() : CreatureScript("npc_empyrean_assassin") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_empyrean_assassinAI(pCreature);
        }

        struct npc_empyrean_assassinAI : public ScriptedAI
        {
            npc_empyrean_assassinAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                DoCast(SPELL_LETHARGIC_POISON);
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_VAPOR_FORM, urand(15000, 20000));    
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_VAPOR_FORM:
                        DoCast(SPELL_VAPOR_FORM);
                        events.ScheduleEvent(EVENT_VAPOR_FORM, urand(20000, 25000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_executor_of_the_caliph : public CreatureScript
{
    public:
        npc_executor_of_the_caliph() : CreatureScript("npc_executor_of_the_caliph") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_executor_of_the_caliphAI(pCreature);
        }

        struct npc_executor_of_the_caliphAI : public ScriptedAI
        {
            npc_executor_of_the_caliphAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_RALLY, urand(5000, 20000));
                events.ScheduleEvent(EVENT_DEVASTATE, urand(2000, 8000));
                events.ScheduleEvent(EVENT_SHOCKWAVE, urand(12000, 20000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_RALLY:
                        DoCast(SPELL_RALLY);
                        events.ScheduleEvent(EVENT_RALLY, urand(20000, 30000));
                        break;
                    case EVENT_SHOCKWAVE:
                        DoCast(SPELL_SHOCKWAVE);
                        events.ScheduleEvent(EVENT_SHOCKWAVE, urand(16000, 30000));
                        break;
                    case EVENT_DEVASTATE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_DEVASTATE, false);

                        events.ScheduleEvent(EVENT_DEVASTATE, urand(15000, 18000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_gust_soldier : public CreatureScript
{
    public:
        npc_gust_soldier() : CreatureScript("npc_gust_soldier") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_gust_soldierAI(pCreature);
        }

        struct npc_gust_soldierAI : public ScriptedAI
        {
            npc_gust_soldierAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* attacker) override
            {
                DoCast(attacker, SPELL_CHARGE);
                events.ScheduleEvent(EVENT_AIR_NOVA, urand(5000, 15000));
                events.ScheduleEvent(EVENT_WIND_BLAST, urand(6000, 8000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_AIR_NOVA:
                        DoCast(SPELL_AIR_NOVA);
                        events.ScheduleEvent(EVENT_AIR_NOVA, urand(10000, 15000));
                        break;
                    case EVENT_WIND_BLAST:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_WIND_BLAST, false);

                        events.ScheduleEvent(EVENT_WIND_BLAST, urand(6000, 8000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

//45704
struct npc_lurking_tempest : public ScriptedAI
{
    npc_lurking_tempest(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    uint32 timer;
    bool firstUse;

    void Reset() override
    {
        timer = 0;
        firstUse = false;
    }

    void IsSummonedBy(Unit* owner) override
    {
        me->SetFacingTo(owner->GetOrientation());
        me->SetOrientation(owner->GetOrientation());
        DoCast(85294);
    }

    void SpellHit(Unit* /*owner*/, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_FEIGN_DEATH)
        {
            if (!firstUse)
            {
                firstUse = true;
                me->RemoveAura(SPELL_LURK);
                Talk(0);
            }
            else
            {
                if (me->HasAura(SPELL_LURK))
                {
                    me->RemoveAura(SPELL_LURK);
                    Talk(0);
                }
            }
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        timer = urand(1000, 3000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (timer)
        {
            if (timer <= diff)
            {
                timer = 6000;

                DoCast(89105);
            }
            else
                timer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

class npc_howling_gale : public CreatureScript
{
    public:
        npc_howling_gale() : CreatureScript("npc_howling_gale") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_howling_galeAI(pCreature);
        }

        struct npc_howling_galeAI : public Scripted_NoMovementAI
        {
            npc_howling_galeAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
                SetCombatMovement(false);
            }

            EventMap events;
            bool bCombat;

            void Reset() override
            {
                bCombat = false;
                events.Reset();
                DoCast(SPELL_HOWLING_GALE);
                events.ScheduleEvent(EVENT_HOWLING_GALE, 1000);
            }

            void JustReachedHome() override
            {
                bCombat = false;
            }

            void EnterEvadeMode() override
            {
                bCombat = false;
                CreatureAI::EnterEvadeMode();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                bCombat = true;
                events.CancelEvent(EVENT_HOWLING_GALE);
                me->RemoveAurasDueToSpell(SPELL_HOWLING_GALE);
            }

            void UpdateAI(uint32 diff) override
            {
                if (bCombat)
                {
                    if (!me->GetMap()->GetPlayers().isEmpty())
                    {
                        uint8 _attackersnum = 0;
                        for (Map::PlayerList::const_iterator itr = me->GetMap()->GetPlayers().begin(); itr != me->GetMap()->GetPlayers().end(); ++itr)
                        {
                            if (me->GetDistance2d(itr->getSource()) < 17.0f)
                            {
                                _attackersnum++;
                                DoCast(SPELL_HOWLING_GALE_DMG);
                            }
                        }

                        if (_attackersnum == 0)
                        {
                            EnterEvadeMode();
                            return;
                        }
                    }
                }

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_HOWLING_GALE:
                        DoCast(me, SPELL_HOWLING_GALE_1, true);
                        DoCast(me, SPELL_HOWLING_GALE_2, true);
                        DoCast(me, SPELL_HOWLING_GALE_3, true);
                        events.ScheduleEvent(EVENT_HOWLING_GALE, 2500);
                        break;
                    }
                }
            }
        };
};

class npc_minister_of_air : public CreatureScript
{
    public:
        npc_minister_of_air() : CreatureScript("npc_minister_of_air") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_minister_of_airAI(pCreature);
        }

        struct npc_minister_of_airAI : public ScriptedAI
        {
            npc_minister_of_airAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_LIGHTNING_LASH, urand(4000, 8000));
                events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(7000, 10000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_LIGHTNING_NOVA:
                        DoCast(SPELL_LIGHTNING_NOVA);
                        events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(10000, 20000));
                        break;
                    case EVENT_LIGHTNING_LASH:
                        DoCast(SPELL_LIGHTNING_LASH);
                        events.ScheduleEvent(EVENT_LIGHTNING_LASH, urand(8000, 15000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_servant_of_asaad : public CreatureScript
{
    public:
        npc_servant_of_asaad() : CreatureScript("npc_servant_of_asaad") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_servant_of_asaadAI(pCreature);
        }

        struct npc_servant_of_asaadAI : public ScriptedAI
        {
            npc_servant_of_asaadAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* attacker) override
            {
                events.ScheduleEvent(EVENT_CRUSADER_STRIKE, urand(3000, 6000));
                events.ScheduleEvent(EVENT_HAND_OF_PROTECTION, urand(10000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CRUSADER_STRIKE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_CRUSADER_STRIKE, false);

                        events.ScheduleEvent(EVENT_CRUSADER_STRIKE, urand(8000, 12000));
                        break;
                    case EVENT_HAND_OF_PROTECTION:
                        DoCast(SPELL_HAND_OF_PROTECTION);
                        events.ScheduleEvent(EVENT_HAND_OF_PROTECTION, urand(20000, 30000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_temple_adept : public CreatureScript
{
    public:
        npc_temple_adept() : CreatureScript("npc_temple_adept") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_temple_adeptAI(pCreature);
        }

        struct npc_temple_adeptAI : public ScriptedAI
        {
            npc_temple_adeptAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;
            Creature* _target;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_HOLY_SMITE, urand(5000, 6000));
                events.ScheduleEvent(EVENT_GREATER_HEAL, urand(5000, 6000));
                events.ScheduleEvent(EVENT_DESPERATE_SPEED, urand(10000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_HOLY_SMITE:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_HOLY_SMITE, false);
                        events.ScheduleEvent(EVENT_HOLY_SMITE, urand(5000, 6000));
                        break;
                    case EVENT_DESPERATE_SPEED:
                        DoCast(SPELL_DESPERATE_SPEED);
                        events.ScheduleEvent(EVENT_DESPERATE_SPEED, urand(20000, 30000));
                        break;
                    case EVENT_GREATER_HEAL:
                        if (_target = me->FindNearestCreature(NPC_EXECUTOR_OF_THE_CALIPH, 30.0f))
                            if (_target->GetHealthPct() < 50)
                                DoCast(_target, SPELL_GREATER_HEAL);
                        else if (_target = me->FindNearestCreature(NPC_MINISTER_OF_AIR, 30.0f))
                            if (_target->GetHealthPct() < 50)
                                DoCast(_target, SPELL_GREATER_HEAL);
                        else if (_target = me->FindNearestCreature(NPC_SERVANT_OF_ASAAD, 30.0f))
                            if (_target->GetHealthPct() < 50)
                                DoCast(_target, SPELL_GREATER_HEAL);
                        else if (_target = me->FindNearestCreature(NPC_TEMPEST_ADEPT, 30.0f))
                            if (_target->GetHealthPct() < 50)
                                DoCast(_target, SPELL_GREATER_HEAL);
                        events.ScheduleEvent(EVENT_GREATER_HEAL, urand(5000, 6000));
                        break;
                    }
                }
            }
        };
};

class npc_turbulent_squall : public CreatureScript
{
    public:
        npc_turbulent_squall() : CreatureScript("npc_turbulent_squall") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_turbulent_squallAI(pCreature);
        }

        struct npc_turbulent_squallAI : public ScriptedAI
        {
            npc_turbulent_squallAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_ASPHYXIATE, urand(3000, 10000));
                events.ScheduleEvent(EVENT_HURRICANE, urand(10000, 20000));
                events.ScheduleEvent(EVENT_CLOUDBURST, urand(5000, 30000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ASPHYXIATE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_ASPHYXIATE, false);
                        events.ScheduleEvent(EVENT_ASPHYXIATE, urand(10000, 20000));
                        break;
                    case EVENT_HURRICANE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(target, SPELL_HURRICANE, false);
                        events.ScheduleEvent(EVENT_HURRICANE, urand(15000, 30000));
                        break;
                    case EVENT_CLOUDBURST:
                        DoCast(SPELL_CLOUDBURST);
                        events.ScheduleEvent(EVENT_CLOUDBURST, urand(20000, 30000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };
};

class npc_wild_vortex : public CreatureScript
{
    public:
        npc_wild_vortex() : CreatureScript("npc_wild_vortex") {}
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_wild_vortexAI(pCreature);
        }

        struct npc_wild_vortexAI : public ScriptedAI
        {
            npc_wild_vortexAI(Creature* pCreature) : ScriptedAI(pCreature) {}

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void EnterCombat(Unit* /*attacker*/) override
            {
                events.ScheduleEvent(EVENT_WIND_SHOCK, urand(5000, 10000));
                events.ScheduleEvent(EVENT_WV_LIGHTNING_BOLT, 3000);
                events.ScheduleEvent(EVENT_CYCLONE, urand(5000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_CYCLONE:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                            DoCast(target, SPELL_CYCLONE, false);
                        events.ScheduleEvent(EVENT_CYCLONE, urand(10000, 20000));
                        break;
                    case EVENT_WIND_SHOCK:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                            DoCast(target, SPELL_WIND_SHOCK, false);
                        events.ScheduleEvent(EVENT_WIND_SHOCK, urand(10000, 20000));
                        break;
                    case EVENT_WV_LIGHTNING_BOLT:
                        if (auto victim = me->getVictim())
                            DoCast(victim, SPELL_WV_LIGHTNING_BOLT, false);
                        events.ScheduleEvent(EVENT_WV_LIGHTNING_BOLT, 3000);
                        break;
                    }
                }
            }
        };
};

class npc_golden_orb : public CreatureScript
{
public:
    npc_golden_orb() : CreatureScript("npc_golden_orb") {}

    bool OnGossipHello(Player* player, Creature* me) override
    {
        if (InstanceScript* instance = me->GetInstanceScript())
        {
            uint32 OrbsCount = instance->GetData(DATA_ORB) + 1;
            instance->DoUpdateWorldState(static_cast<WorldStates>(5649), OrbsCount);
            instance->SetData(DATA_ORB, OrbsCount);

            if (OrbsCount == 5)
                instance->DoCastSpellOnPlayers(94756);

            me->DespawnOrUnsummon();
        }
        return true;
    }
};

class spell_minister_of_air_lightning_lash : public SpellScriptLoader
{
    public:
        spell_minister_of_air_lightning_lash() : SpellScriptLoader("spell_minister_of_air_lightning_lash") {}

        class spell_minister_of_air_lightning_lash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_minister_of_air_lightning_lash_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_LIGHTNING_LASH_DMG, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_minister_of_air_lightning_lash_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_minister_of_air_lightning_lash_SpellScript();
        }
};

//85294
class spell_lurk_search : public AuraScript
{
    PrepareAuraScript(spell_lurk_search);

    void OnPeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, 86456, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_lurk_search::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//86456
class spell_lurk_search_dummy : public SpellScript
{
    PrepareSpellScript(spell_lurk_search_dummy);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit()->ToPlayer();
        Unit* caster = GetCaster();
        if (!caster || !target)
            return;

        if (target->isInFront(caster, M_PI / 2))
            caster->CastSpell(caster, SPELL_FEIGN_DEATH, false);
        else
        {
            if (!caster->HasAura(SPELL_LURK))
            {
                caster->CastSpell(caster, SPELL_LURK, false);
                caster->RemoveAura(SPELL_FEIGN_DEATH);
                caster->GetAI()->AttackStart(target);
            }
        }   
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_lurk_search_dummy::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_the_vortex_pinnacle()
{
    new npc_vortex_pinnacle_slipsteam();
    new npc_skyfall_star();
    new npc_cloud_prince();
    new npc_whipping_wind();
    new npc_young_storm_dragon();
    new npc_armored_mistral();
    new npc_empyrean_assassin();
    new npc_executor_of_the_caliph();
    new npc_gust_soldier();
    new npc_howling_gale();
    new npc_minister_of_air();
    new npc_servant_of_asaad();
    new npc_temple_adept();
    new npc_turbulent_squall();
    new npc_wild_vortex();
    new npc_golden_orb();
    new spell_minister_of_air_lightning_lash();
    RegisterCreatureAI(npc_lurking_tempest);
    RegisterAuraScript(spell_lurk_search);
    RegisterSpellScript(spell_lurk_search_dummy);
};