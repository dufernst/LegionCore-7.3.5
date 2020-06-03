/*
    Legion
    World boss
*/
#include "AreaTriggerAI.h"

enum eSpells
{
    // flotsam
    SPELL_STALKER           = 220277,
    SPELL_BREAKSAM          = 223317,
    SPELL_YAKSAM            = 223373,
    SPELL_GETSAM            = 220340,
    SPELL_JETSAM            = 220295,
    SPELL_REGEN             = 223304,

    //levantus
    SPELL_GUST_OF_WIND      = 217211,
    SPELL_RENDING_WHIRL     = 217235,
    SPELL_MASSIVE_SPOUL     = 217249,
    SPELL_ELECTRIFY         = 217344,
    SPELL_TURBULENT_VORTEX  = 217360,
    SPELL_RAMPAGING_TORRENT = 217229,
    
    // nithogg
    SPELL_TAIL_LASH         = 212835,
    SPELL_STORM_BREATH      = 212852,
    SPELL_ELECTRICAL_STORM  = 212867,
    SPELL_STATIC_CHARGE     = 212887,
    SPELL_CRACKLING_JOLT    = 212837,
    
    // humongris
    SPELL_FIRE_BOOM         = 216427,
    SPELL_EARTHSHAKE_STOMP  = 216430,
    SPELL_ICE_FIST          = 216432,
    SPELL_YOU_GO_BANG       = 216817,
    SPELL_MAKE_THE_SNOW     = 216467,
    
    //calamir
    SPELL_PHASE_FIRE        = 217563,
    SPELL_PHASE_FROST       = 217831,
    SPELL_PHASE_ARCANE      = 217834,
    
    SPELL_BURNING_BOMB      = 217874,
    SPELL_HOWLING_GALE      = 217966,
    SPELL_ARCANOPULSE       = 218005,
    
    SPELL_WRATHFUL_FLAMES   = 217893,
    SPELL_ICY_COMET         = 217919,
    SPELL_ARCANE_DESOLATION = 217986,
    
    // jim
    SPELL_MORE_MORE_MORE        = 223715,
    // General
    SPELL_NIGHTSHIFTED_BOLTS    = 223623,
    SPELL_RESONANCE             = 223614,
    SPELL_NIGHTSTABLE_ENERGY    = 223689,
    
    // shartos
    SPELL_TAIL_LASH_SHARTOS     = 215806,
    SPELL_CRY_OF_THE_TORMENTED  = 216044,
    SPELL_DREAD_FLAME           = 216043,
    SPELL_NIGHTMARE_BREATH      = 215821,
    SPELL_BURNING_EARTH         = 215856,
    
    // drugon
    SPELL_ICE_HURL              = 219803,
    SPELL_SNOW_CRASH            = 219493,
    SPELL_AVALANCHE             = 219542,
    SPELL_SNOW_PLOW             = 219601,
    SPELL_SNOW_PLOW_AURA        = 219602,
    SPELL_SNOW_PLOW_STUN        = 219610,
    // add
    SPELL_AVALANCHE_1           = 219687, // point
    SPELL_AVALANCHE_2           = 219689, // target
    
    // SOULTAKERS
    // capitan
    SPELL_TENTACLE_BASH         = 213420,
    SPELL_SHATTER_CREWMEN       = 213532,
    SPELL_CURSED_CREW           = 213522,
    //ydorn
    SPELL_MARAUDING_MISTS       = 213665,
    SPELL_SEADOG_SCUTTLE_COST   = 213580,
    SPELL_SEADOG_SCUTTLE        = 213584,
    SPELL_SEADOG_SCUTTLE_TRIG   = 213588,
    // mevra
    SPELL_TELEPORT              = 215681,
    SPELL_EXPEL_SOUL            = 213625,
    SPELL_SOUL_REND             = 213605,
    SPELL_SOUL_REND_TRIG        = 213638,

    //add
    SPELL_SHATTER_CREWMEN_ADD   = 213533,
    
    // Ana-Muz
    SPELL_FEL_GEYZER            = 218823, // 22 sec + filter + script to geyzer (197376) + npc 109946 ?  197217
    SPELL_IMPISH_FLAMES         = 218888, // 22 sec after spell above
    SPELL_GASEOUS_BREATH        = 219255, // 31 sec + at + adds
    SPELL_MOTHERS_EMBRACE       = 219045, // 65 sec + script for charm
    SPELL_REMOVE_CHARM          = 227351, // reset or dead
    
    // Nazak
    SPELL_CORRODING_SPRAY       = 219349, // 23
    SPELL_WEB_WRAP              = 219861, // 42
    SPELL_FOUNDATIONAL_COLLAPSE = 219591, // 57 + cast. + script for crystalls
    SPELL_ABSORB_LEYSTONES      = 219813, // after above. Script for stacks   
    SPELL_LEY_INFUSION          = 219836, 
};



// 99929
class boss_flotsam : public CreatureScript
{
public:
    boss_flotsam() : CreatureScript("boss_flotsam") {}

    struct boss_flotsamAI : public ScriptedAI
    {
        boss_flotsamAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;

        void Reset() override
        {
            events.Reset();
            me->SetMaxPower(POWER_MANA, 100);
            me->SetPower(POWER_MANA, 0);
            summons.DespawnAll();
            me->RemoveAura(SPELL_REGEN);
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, urand(8000, 9000)); //триггер на рандома SPELL_STALKER линк SPELL_JETSAM
            events.RescheduleEvent(EVENT_2, urand (16000, 19000)); // SPELL_BREAKSAM
            events.RescheduleEvent(EVENT_3, 40000); //SPELL_YAKSAM
            events.RescheduleEvent(EVENT_4, 1000); //проверка маны. Если 100 - каст SPELL_GETSAM
            me->SetPower(POWER_MANA, 0);
            DoCast(SPELL_REGEN);
        }
        
        void JustDied(Unit* who) override
        {
            Talk(1);
        }

        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            DoZoneInCombat(summon, 150.0f);
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            events.Update(diff);
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            DoCast(target, SPELL_STALKER, true);
                        events.RescheduleEvent(EVENT_1, urand(8000, 9000)); //триггер на рандома SPELL_STALKER ЛИНК SPELL_JETSAM
                        break;
                    case EVENT_2:
                        DoCast(SPELL_BREAKSAM);
                        events.RescheduleEvent(EVENT_2, urand (16000, 19000)); // SPELL_BREAKSAM
                        break;
                    case EVENT_3:
                        DoCast(SPELL_YAKSAM);
                        Talk(0);
                        events.RescheduleEvent(EVENT_3, 40000); //SPELL_YAKSAM
                        events.RescheduleEvent(EVENT_5, 6000); // adds
                        break;
                    case EVENT_4:
                        if (me->GetPower(POWER_MANA) == 100) //не понятно, когда и как копится энергия и почему стоит МАНАкост. (снифы нужны каста)
                            DoCast(SPELL_GETSAM);
                        events.RescheduleEvent(EVENT_4, 1000);
                        break;
                    case EVENT_5:
                        DoCast(223391);
                        DoCast(223403);
                        DoCast(223404);
                        break;
                    default:
                        break;                        
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_flotsamAI(creature);
    }
};

// 110772
class npc_jetsam_stalker : public CreatureScript
{
public:
    npc_jetsam_stalker() : CreatureScript("npc_jetsam_stalker") {}

    struct npc_jetsam_stalkerAI : public ScriptedAI
    {
        npc_jetsam_stalkerAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
           // me->ApplySpellImmune(220354, IMMUNITY_DAMAGE, 0, true);
           first = false;
        }
        
        bool first;
        
        void IsSummonedBy(Unit* summoner) override
        {
            summoner->CastSpell(me, SPELL_JETSAM);
        }
        
        void SpellHit(Unit* /*caster */, SpellInfo const* spell) override
        {                           
            if (spell->Id == 220340 && !first)
            {
                first = true;
                DoCast(220367);
                me->DespawnOrUnsummon(100);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jetsam_stalkerAI(creature);
    }
};

// 108829
class boss_levantus : public CreatureScript
{
public:
    boss_levantus() : CreatureScript("boss_levantus") {}

    struct boss_levantusAI : public ScriptedAI
    {
        boss_levantusAI(Creature* creature) : ScriptedAI(creature), summons(me) 
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        
        EventMap events;
        SummonList summons;
        void Reset() override
        {
            events.Reset();
            summons.DespawnAll();
            me->RemoveAllAreaObjects();
            me->SetReactState(REACT_AGGRESSIVE);
        }
        
        void EnterCombat(Unit* unit) override
        {
           // DoCast(SPELL_GUST_OF_WIND);
            events.RescheduleEvent(EVENT_1, urand(7000, 8000)); //SPELL_RENDING_WHIRL + 40
            events.RescheduleEvent(EVENT_2, urand(25000, 27000)); //SPELL_MASSIVE_SPOUL + 43
            events.RescheduleEvent(EVENT_3, urand(50000, 51000)); //SPELL_ELECTRIFY + 43  (доделать)
            events.RescheduleEvent(EVENT_4, 11000); // SPELL_TURBULENT_VORTEX
            events.RescheduleEvent(EVENT_5, 1000); //check
            events.RescheduleEvent(EVENT_6, 3000); // not repeat
        }
        
        void EnterEvadeMove()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            summons.DespawnAll();
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
        }
        
        void SpellHit(Unit* /*caster */, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_ELECTRIFY)
            {
                std::list<HostileReference*> threatlist = me->getThreatManager().getThreatList();
                if (!threatlist.empty())
                {
                    for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                    {
                        if (Player* pl = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                        {
                            if (pl && pl->GetPositionZ() < 0)
                                me->CastSpell(pl, 217352, true);
                        }
                    }
                }
            }
        }
                
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_RENDING_WHIRL);
                        events.RescheduleEvent(EVENT_1, urand(39000, 41000)); //SPELL_RENDING_WHIRL + 40
                        break;
                    case EVENT_2:
                        DoCast(SPELL_MASSIVE_SPOUL);
                        events.RescheduleEvent(EVENT_2, urand(39000, 41000)); //SPELL_MASSIVE_SPOUL + 43
                        break;
                    case EVENT_3:
                        DoCast(SPELL_ELECTRIFY);
                        events.RescheduleEvent(EVENT_3, urand (39000, 41000));
                        break;
                    case EVENT_4:
                        DoCast(SPELL_TURBULENT_VORTEX);
                        events.RescheduleEvent(EVENT_4, 10000);
                        break;        
                    case EVENT_5:
                        if (auto target = me->FindNearestPlayer(100.0f))
                            if (!me->IsWithinMeleeRange(target))
                                DoCast(SPELL_RAMPAGING_TORRENT);
                        events.RescheduleEvent(EVENT_5, 1000); //check
                        break;
                    case EVENT_6:
                        for (int8 i = 0; i < 25; i++)
                            me->SummonCreature(109211, me->GetPositionX() + irand (-30, 30), me->GetPositionY() + irand (-30, 30), 0.0f, 0.0f);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_levantusAI(creature);
    }
};

// 217249
class spell_massive_spoul : public SpellScriptLoader
{
    public:
        spell_massive_spoul() : SpellScriptLoader("spell_massive_spoul") {}

        class spell_massive_spoul_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_massive_spoul_AuraScript);
            
            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetCaster()->ToCreature())
                {
                    GetCaster()->GetMotionMaster()->MoveRotate(20000, ROTATE_DIRECTION_RIGHT);
                    GetCaster()->ToCreature()->AttackStop();
                    GetCaster()->ToCreature()->SetReactState(REACT_PASSIVE);
                }
            }

            void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetCaster()->ToCreature())
                {
                    GetCaster()->StopMoving();
                    GetCaster()->GetMotionMaster()->Clear(false);
                    GetCaster()->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    GetCaster()->ToCreature()->AI()->DoZoneInCombat(GetCaster()->ToCreature(), 150.0f);
                    
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_massive_spoul_AuraScript::OnApply, EFFECT_0, SPELL_AURA_CREATE_AREATRIGGER, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_massive_spoul_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_CREATE_AREATRIGGER, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_massive_spoul_AuraScript();
        }
};

// 107023
class boss_nithogg : public CreatureScript
{
public:
    boss_nithogg() : CreatureScript("boss_nithogg") {}

    struct boss_nithoggAI : public ScriptedAI
    {
        boss_nithoggAI(Creature* creature) : ScriptedAI(creature), summons(me), healthPct(0)
        {
        }

        EventMap events;
        SummonList summons;
        uint32 healthPct;
        
        void Reset() override
        {
            events.Reset();
            summons.DespawnAll();
            me->RemoveAllAreaObjects();
            healthPct = 80;
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, urand(2000, 4000)); // SPELL_TAIL_LASH  скилл проверяет перед, а не зад. Должен зад
            events.RescheduleEvent(EVENT_2, urand(20000, 21000)); // SPELL_STORM_BREATH
            events.RescheduleEvent(EVENT_3, 30000); // SPELL_ELECTRICAL_STORM
            events.RescheduleEvent(EVENT_4, 41000); // SPELL_STATIC_CHARGE
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);
            summon->SetReactState(REACT_PASSIVE);
            if (summon->GetEntry() == 107338)
                summon->GetMotionMaster()->MoveRandom(7.0f);
        }
        
        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPct(healthPct))
            {
                healthPct -= 20;
                events.RescheduleEvent(EVENT_5, 10);
            }
        }
        
                
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_TAIL_LASH);
                        events.RescheduleEvent(EVENT_1, 5000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_STORM_BREATH);
                        events.RescheduleEvent(EVENT_2, urand(20000, 21000));
                        break;
                    case EVENT_3:
                        DoCast(SPELL_ELECTRICAL_STORM);
                        events.RescheduleEvent(EVENT_3, 30000);
                        break;
                    case EVENT_4:
                        {                               
                            for (int8 i = 0; i < 5; i++)
                            {                                        
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40, true))    
                                    if (!target->HasAura(SPELL_STATIC_CHARGE))
                                        me->CastSpell(target, SPELL_STATIC_CHARGE);
                            }
                            events.RescheduleEvent(EVENT_4, 41000);
                        }
                        break;
                    case EVENT_5:
                        DoCast(SPELL_CRACKLING_JOLT);
                        std::list<HostileReference*> threatlist = me->getThreatManager().getThreatList();
                        if (!threatlist.empty())
                        {
                            for (std::list<HostileReference*>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                            {
                                    if (Player* pl = me->GetPlayer(*me, (*itr)->getUnitGuid()))
                                        Talk(0, pl->GetGUID());
                            }
                        }
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_nithoggAI(creature);
    }
};

// 107353
class npc_static_orb : public CreatureScript
{
public:
    npc_static_orb() : CreatureScript("npc_static_orb") {}

    struct npc_static_orbAI : public ScriptedAI
    {
        npc_static_orbAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            CheckComplete = false;
            checkTimer = 2000;
            moveComplete = true;
            DoCast(212915);  
        }
        
        ObjectGuid targetMove;
        bool moveComplete;
        uint32 moveTimer;
        bool CheckComplete;
        uint32 checkTimer;
        
        void UpdateAI(uint32 diff) override
        {
            if (!moveComplete)
            {
                if (moveTimer <= diff)
                {
                    Player* target = ObjectAccessor::GetPlayer(*me, targetMove);
                    if (!target)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }

                    if (me->GetDistance(target) < 3.0f)
                    {
                        moveComplete = true;
                        me->CastStop();
                        DoCast(target, 212948, true);
                        me->DespawnOrUnsummon(1000);
                    }
                    moveTimer = 1000;
                }
                else
                    moveTimer -= diff;
            }
            
            if (!CheckComplete)
            {
                if (checkTimer <= diff)
                {
                    std::list<Player*> targets;
                    targets.clear();
                    GetPlayerListInGrid(targets, me, 70.0f);
                    if (!targets.empty())
                    {
                        std::list<Player*>::const_iterator itr = targets.begin();
                        std::advance(itr, urand(0, targets.size() - 1));
                        if (*itr)
                            targetMove = (*itr)->GetGUID();
                        
                        if ((*itr) && !(*itr)->HasAura(212943) && !(*itr)->HasAura(212887))
                        {
                            moveTimer = 1000;
                            moveComplete = false;
                            CheckComplete = true;
                            if (!me->HasUnitState(UNIT_STATE_STUNNED))
                            {
                                me->GetMotionMaster()->MoveFollow((*itr), PET_FOLLOW_DIST, (*itr)->GetFollowAngle());
                                me->CastSpell((*itr), 212943);
                            }
                        }
                        else
                        {
                            checkTimer = 500;
                        }
                    }
                }
                else
                    checkTimer -= diff;
            }
        }        
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_static_orbAI(creature);
    }
};

// 212887
class spell_static_charge : public SpellScriptLoader
{
    public:
        spell_static_charge() : SpellScriptLoader("spell_static_charge") {}

        class spell_static_charge_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_static_charge_AuraScript);

            void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster() && GetCaster()->ToCreature() && GetTarget())
                {
                    Unit* caster = GetCaster()->ToCreature();
                    if (caster && caster->GetEntry() == 107023)
                    {
                        caster->SummonCreature(107353, GetTarget()->GetPositionX(), GetTarget()->GetPositionY(), GetTarget()->GetPositionZ(), GetTarget()->GetOrientation());
                    }
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_static_charge_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_static_charge_AuraScript();
        }
};

// 108879
class boss_humongris : public CreatureScript
{
public:
    boss_humongris() : CreatureScript("boss_humongris") {}

    struct boss_humongrisAI : public ScriptedAI
    {
        boss_humongrisAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, urand(15000, 16000)); // SPELL_FIRE_BOOM
            events.RescheduleEvent(EVENT_2, urand(25000, 26000)); // SPELL_EARTHSHAKE_STOMP
            events.RescheduleEvent(EVENT_3, urand(28000, 30000)); // SPELL_ICE_FIST
            events.RescheduleEvent(EVENT_4, urand(33000, 35000)); // SPELL_YOU_GO_BANG 25  rand
            events.RescheduleEvent(EVENT_5, urand(37000, 38000)); // SPELL_MAKE_THE_SNOW 34

        }        
                
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_FIRE_BOOM);
                        SayHelper(0);
                        events.RescheduleEvent(EVENT_1, 15000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_EARTHSHAKE_STOMP);
                        events.RescheduleEvent(EVENT_2, 25000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_ICE_FIST);
                        SayHelper(2);
                        events.RescheduleEvent(EVENT_3, urand(28000, 30000));
                        break;
                    case EVENT_4:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            DoCast(target, SPELL_YOU_GO_BANG, false);
                        events.RescheduleEvent(EVENT_4, urand(24000, 26000));
                        break;
                    case EVENT_5:
                        DoCast(SPELL_MAKE_THE_SNOW);
                        SayHelper(1);
                        events.RescheduleEvent(EVENT_5, urand(34000, 36000));
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
        
        void SayHelper(uint32 text)
        {
            Talk(text);
            if (Creature* padosen = me->FindNearestCreature(108880, 50.0f, true))
                padosen->AI()->Talk(text);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_humongrisAI(creature);
    }
};

uint32 const phaseSpell[3]
{
    SPELL_PHASE_FIRE,
    SPELL_PHASE_FROST,
    SPELL_PHASE_ARCANE
};

// 109331
class boss_calamir : public CreatureScript
{
public:
    boss_calamir() : CreatureScript("boss_calamir") {}

    struct boss_calamirAI : public ScriptedAI
    {
        boss_calamirAI(Creature* creature) : ScriptedAI(creature) 
        {
            timerphase = 100;
            phase = 0;
        }
        
        EventMap events;
        uint32 timerphase;
        uint8 phase;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, urand(5000, 6000)); // SPELL_1 13
            events.RescheduleEvent(EVENT_2, urand(15000, 16000)); // SPELL_2 25
        }        
                
        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (timerphase <= diff)
            {
                DoCast(me, phaseSpell[phase], true);
                
                if (phase >= 2)
                    phase = 0;
                else
                    phase++;
                
                timerphase = 25000;
                
            } else timerphase -= diff;
            
            if (!UpdateVictim())
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        if (me->HasAura(SPELL_PHASE_FIRE) || me->HasAura(SPELL_PHASE_FROST) || me->HasAura(SPELL_PHASE_ARCANE))
                        {
                            if (me->HasAura(SPELL_PHASE_FIRE))
                                DoCast(SPELL_BURNING_BOMB);                            
                            else if (me->HasAura(SPELL_PHASE_FROST))
                                DoCast(SPELL_HOWLING_GALE);
                            // else if (me->HasAura(SPELL_PHASE_ARCANE))
                            //    DoCast(SPELL_ARCANOPULSE);
                            
                            events.RescheduleEvent(EVENT_1, 13000);   
                        } 
                        else 
                            events.RescheduleEvent(EVENT_1, 1000);   
                        
                        break;
                    case EVENT_2:
                         if (me->HasAura(SPELL_PHASE_FIRE) || me->HasAura(SPELL_PHASE_FROST) || me->HasAura(SPELL_PHASE_ARCANE))
                        {
                            if (me->HasAura(SPELL_PHASE_FIRE))
                                DoCast(SPELL_WRATHFUL_FLAMES);                            
                            else if (me->HasAura(SPELL_PHASE_FROST))
                                DoCast(SPELL_ICY_COMET);
                            else if (me->HasAura(SPELL_PHASE_ARCANE))
                                DoCast(SPELL_ARCANE_DESOLATION);
                            
                            events.RescheduleEvent(EVENT_2, 25000);   
                        } 
                        else 
                            events.RescheduleEvent(EVENT_2, 1000);   
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_calamirAI(creature);
    }
};

// 218003
/*  Ждем Блека
class spell_arcanopulse : public SpellScriptLoader
{
    public:
        spell_arcanopulse() : SpellScriptLoader("spell_arcanopulse") { }

        class spell_arcanopulse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_arcanopulse_SpellScript);

            void OnCast()
            {
                PreventHitDefaultEffect(EFFECT_0);

                if (!GetCaster())
                    return;

                Position pos;
                GetCaster()->GetRandomNearPosition(pos, 3.0f);
                WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());
                dest->Relocate(pos);
            }

            void Register()
            {
                OnCast += SpellHitFn(spell_arcanopulse_SpellScript::OnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_arcanopulse_SpellScript();
        }
};  */

class boss_withered_jim : public CreatureScript
{
public:
    boss_withered_jim() : CreatureScript("boss_withered_jim") {}

    struct boss_withered_jimAI : public ScriptedAI
    {
        boss_withered_jimAI(Creature* creature) : ScriptedAI(creature), summons(me), countclons(0)
        {
        }

        EventMap events;
        SummonList summons;
        uint8 countclons;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
            summons.DespawnAll();
            countclons=0;
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 18000);
            events.RescheduleEvent(EVENT_2, 24000);
            events.RescheduleEvent(EVENT_3, 22000);
            if (me->GetEntry() == 102075)
                events.RescheduleEvent(EVENT_4, 30000);
            DoCast(223632); // AT
            DoCast(223599);
        }  
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);  
            DoZoneInCombat(me, 150.0f);
            if (summon->GetEntry() == 112350)
                summon->CastSpell(summon, 223599);
            if (summon->GetEntry() == 112342)
                summon->DespawnOrUnsummon(9000);
        }

        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
        }
                
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker->GetTypeId() != TYPEID_PLAYER)
                return;
            
            if (attacker->GetPositionZ() >= 60.0f)
                me->Kill(attacker); //cheaters and others
        }
        
        
        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (!UpdateVictim())
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_NIGHTSHIFTED_BOLTS);
                        events.RescheduleEvent(EVENT_1, urand(25000, 30000));
                        break;
                    case EVENT_2:
                        DoCast(SPELL_RESONANCE);
                        events.RescheduleEvent(EVENT_1, urand(24000, 30000));
                        break;
                    case EVENT_3:
                        DoCast(SPELL_NIGHTSTABLE_ENERGY);
                        events.RescheduleEvent(EVENT_1, urand(29000, 34000));
                        break;
                    case EVENT_4:
                        ++countclons;
                        DoCast(SPELL_MORE_MORE_MORE);
                        if (countclons < 5)
                            events.RescheduleEvent(EVENT_4, 30000);
                        events.RescheduleEvent(EVENT_5, 1000);
                        break;
                    case EVENT_5:
                        if (me->HasAura(SPELL_MORE_MORE_MORE))
                            events.RescheduleEvent(EVENT_5, 1000);
                        else
                        {
                            if (auto add = me->SummonCreature(112350, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation()))
                                add->CastSpell(add, 223723);
                        }
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_withered_jimAI(creature);
    }
};

// 223614
class spell_resonance : public SpellScriptLoader
{
public:
    spell_resonance() : SpellScriptLoader("spell_resonance") {}

    class spell_resonance_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_resonance_AuraScript);

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            PreventDefaultAction();

            Unit* target = GetTarget();
            if (!target)
                return;
            target->CastSpell(target, 223616);
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_resonance_AuraScript::OnProc, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_resonance_AuraScript();
    }
};

// 108678
class boss_shartos : public CreatureScript
{
public:
    boss_shartos() : CreatureScript("boss_shartos") {}

    struct boss_shartosAI : public ScriptedAI
    {
        boss_shartosAI(Creature* creature) : ScriptedAI(creature){}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 5000);
            events.RescheduleEvent(EVENT_2, 45000);
            events.RescheduleEvent(EVENT_3, 20000);
            events.RescheduleEvent(EVENT_5, 24000);
        }  

                
        void UpdateAI(uint32 diff) override
        {
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (!UpdateVictim())
                return;

            events.Update(diff);
            
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_TAIL_LASH_SHARTOS);
                        events.RescheduleEvent(EVENT_1, urand(7000, 10000));
                        break;
                    case EVENT_2:
                        DoCast(SPELL_CRY_OF_THE_TORMENTED);
                        events.RescheduleEvent(EVENT_2, 50000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_DREAD_FLAME);
                        events.RescheduleEvent(EVENT_3, 20000);
                        events.RescheduleEvent(EVENT_4, 10000);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_NIGHTMARE_BREATH);
                        break;
                    case EVENT_5:
                        DoCast(SPELL_BURNING_EARTH);  // �� �������...
                        events.RescheduleEvent(EVENT_5, 25000);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_shartosAI(creature);
    }
};

// 110378
class boss_drugon_the_frostblood : public CreatureScript
{
public:
    boss_drugon_the_frostblood() : CreatureScript("boss_drugon_the_frostblood") {}

    struct boss_drugon_the_frostbloodAI : public ScriptedAI
    {
        boss_drugon_the_frostbloodAI(Creature* creature) : ScriptedAI(creature), summons(me), timer_phase(0), inphase(false)
        {
        }

        EventMap events;
        SummonList summons;
        uint32 timer_phase;
        bool inphase;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
            summons.DespawnAll();
            timer_phase = 3000;
            inphase = false;
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 6000);
            events.RescheduleEvent(EVENT_2, 11000);
            events.RescheduleEvent(EVENT_3, 20000);
            events.RescheduleEvent(EVENT_4, 33000);
        }  
        
        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);  
            if (summon->GetEntry() == 110452)
            {
                for (uint8 i = 0; i < urand(3,5); ++i)
                    summon->CastSpell(summon, SPELL_AVALANCHE_1);
                
                std::list<Player*> targets;
                targets.clear();
                GetPlayerListInGrid(targets, summon, 30.0f);
                if (!targets.empty())
                    for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    {
                        if (urand(0, 1) == 1)
                            summon->CastSpell((*itr), SPELL_AVALANCHE_2);
                    }
                
            }
        }
        
        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_SNOW_PLOW)
            {
                me->StopAttack();
                AttackStart(target);
                inphase = true;
            }
            if (spell->Id == SPELL_SNOW_CRASH)
                target->CastSpell(target, 219497, true);
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            if (inphase)
            {
                if (timer_phase <= diff)
                {
                    if (me->HasAura(SPELL_SNOW_PLOW_AURA))
                    {
                        if (auto target = me->getVictim())
                        {
                            if (me->IsWithinMeleeRange(target, me->GetAttackDist()))
                            {
                                me->RemoveAura(SPELL_SNOW_PLOW_AURA);
                                me->CastSpell(target, SPELL_SNOW_PLOW_STUN);
                                me->SetReactState(REACT_AGGRESSIVE);
                                Talk(1);
                                inphase = false;
                            }
                        }
                        else
                        {
                            me->RemoveAura(SPELL_SNOW_PLOW_AURA);
                            me->SetReactState(REACT_AGGRESSIVE);
                        }
                    }
                    else
                    {
                        inphase = false;
                        me->SetReactState(REACT_AGGRESSIVE);
                    }
                    timer_phase = 1000;   
                }
                else
                    timer_phase -= diff;
                
                return;
            }
            
            events.Update(diff);
                        
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(SPELL_ICE_HURL);
                        events.RescheduleEvent(EVENT_1, 10000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_SNOW_CRASH);
                        events.RescheduleEvent(EVENT_2, 11000);
                        break;
                    case EVENT_3:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                            me->CastSpell(target, SPELL_AVALANCHE);
                        events.RescheduleEvent(EVENT_3, 20000);
                        break;
                    case EVENT_4:
                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                            me->CastSpell(target, SPELL_SNOW_PLOW);
                        Talk(0);
                        inphase = true;
                        timer_phase = 3000;
                        events.RescheduleEvent(EVENT_4, 34000);
                        break;
                        
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_drugon_the_frostbloodAI(creature);
    }
};

enum Bosses
{
    NPC_CAPITAN     = 106981,
    NPC_YDORN       = 106982,
    NPC_MEVRA       = 106984,
};

uint32 const _Bosses[3]
{
    NPC_CAPITAN,
    NPC_YDORN,
    NPC_MEVRA
};

Position const tpPos[6] =
{
    {4930.69f, 489.92f, -53.34f, 2.71f},
    {4904.75f, 471.04f, -53.34f, 2.21f},
    {4863.42f, 449.40f, -53.34f, 1.36f},
    {4840.75f, 476.77f, -53.34f, 0.60f},
    {4820.06f, 510.35f, -53.34f, 0.11f},
    {4878.81f, 524.31f, -48.90f, 4.52f}
};

// 106981 106982 106984
class boss_soultakers : public CreatureScript
{
public:
    boss_soultakers() : CreatureScript("boss_soultakers") {}

    struct boss_soultakersAI : public ScriptedAI
    {
        boss_soultakersAI(Creature* creature) : ScriptedAI(creature), timer_for_energy(0)
        {
        }

        EventMap events;
        uint32 timer_for_energy;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
           // me->setPowerType(POWER_ENERGY);
            me->SetPower(POWER_ENERGY, 0);
            timer_for_energy = 1000;
        }
        
        void EnterCombat(Unit* unit) override
        {
            if (me->GetEntry() == NPC_CAPITAN || me->GetEntry() == NPC_YDORN)
                events.RescheduleEvent(EVENT_1, urand(13000, 16000));
            if (me->GetEntry() == NPC_MEVRA)
                events.RescheduleEvent(EVENT_3, 1000); // 9000
            
            events.RescheduleEvent(EVENT_10, 1000); // check for energy
            
            for (int32 n = 0; n < 3; n++)
                if (me->GetEntry() != _Bosses[n])
                    if (Creature* targ = me->FindNearestCreature(_Bosses[n], 100.0f))
                    {
                        if (targ->isAlive())
                            targ->AI()->AttackStart(unit);
                        else
                            targ->Respawn(false);
                    }
        }  

        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (me->GetEntry() == NPC_CAPITAN)
            {
                if (spell->Id == SPELL_SHATTER_CREWMEN)
                {
                    if (target->GetTypeId() == TYPEID_UNIT)
                    {
                        if (target->ToCreature()->GetEntry() == 107569)
                        {
                            if (target->isAlive())
                            {
                                target->CastSpell(target, SPELL_SHATTER_CREWMEN_ADD);
                                target->ToCreature()->DespawnOrUnsummon(500);
                            }
                        }
                    }
                }
            }
            
            if (me->GetEntry() == NPC_MEVRA)
            {
                if (spell->Id == SPELL_SOUL_REND_TRIG)
                {
                    me->CastSpell(target, 213648, true);
                }
            } 
        }
        
        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (attacker != me)
                for (int32 n = 0; n < 3; n++)
                    if (me->GetEntry() != _Bosses[n])
                        if (Creature* targ = me->FindNearestCreature(_Bosses[n], 100.0f, true))
                        {
                            if (damage >= me->GetHealth())
                                attacker->Kill(targ);
                            else
                                targ->DealDamage(targ, damage); // targ->GetHealth() 
                        }
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            if (timer_for_energy <= diff)
            {
                uint32 power = me->GetPower(POWER_ENERGY);
                if (power < 100)
                    me->SetPower(POWER_ENERGY, ++power);
                timer_for_energy = urand(1000, 1300);
            }
            else
                timer_for_energy -= diff;
            
            events.Update(diff);
                        
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        if (me->GetEntry() == NPC_CAPITAN)
                            DoCast(SPELL_TENTACLE_BASH);
                        else
                            DoCast(SPELL_MARAUDING_MISTS);
                        events.RescheduleEvent(EVENT_1, urand(15000, 17000));
                        break;
                    case EVENT_2:
                        DoCast(SPELL_SHATTER_CREWMEN);
                        break;
                    case EVENT_3:
                        {
                            uint32 rand = urand(0, 5);
                            me->CastSpell(tpPos[rand].GetPositionX(), tpPos[rand].GetPositionY(), tpPos[rand].GetPositionZ(), SPELL_TELEPORT);
                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                                me->CastSpell(target, SPELL_EXPEL_SOUL);
                            events.RescheduleEvent(EVENT_3, 8500);
                        }
                        break;
                    case EVENT_10:
                        {
                            uint32 power = me->GetPower(POWER_ENERGY);
                            if (power == 100)
                            {
                                switch(me->GetEntry())
                                {
                                    case NPC_CAPITAN:
                                        DoCast(SPELL_CURSED_CREW);
                                        events.RescheduleEvent(EVENT_2, 4000);
                                        break;
                                    case NPC_YDORN:
                                        // repeat this 3 time
                                        me->StopAttack();
                                        me->SetReactState(REACT_PASSIVE);
                                        DoCast(SPELL_SEADOG_SCUTTLE_COST);
                                        if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                                        {
                                            me->CastSpell(target, SPELL_SEADOG_SCUTTLE);
                                            me->CastSpell(target, SPELL_SEADOG_SCUTTLE_TRIG);
                                        }
                                        
                                        me->AddDelayedEvent(5000, [this]() -> void
                                        {
                                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                                            {
                                                me->CastSpell(target, SPELL_SEADOG_SCUTTLE);
                                                me->CastSpell(target, SPELL_SEADOG_SCUTTLE_TRIG);
                                            }
                                        });
                                        
                                        me->AddDelayedEvent(10000, [this]() -> void
                                        {
                                            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 70.0f, true))
                                            {
                                                me->CastSpell(target, SPELL_SEADOG_SCUTTLE);
                                                me->CastSpell(target, SPELL_SEADOG_SCUTTLE_TRIG);
                                            }
                                        });
                                        
                                        me->AddDelayedEvent(15000, [this] () -> void
                                        {
                                            me->SetReactState(REACT_AGGRESSIVE);
                                        });
                                        break;
                                    case NPC_MEVRA:
                                        DoCast(SPELL_SOUL_REND);
                                        break;
                                }
                             //  me->SetPower(POWER_ENERGY, 0);
                            }
                            // else
                                // me->SetPower(POWER_ENERGY, ++power);
                            events.RescheduleEvent(EVENT_10, urand(1000, 1100));
                        }
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_soultakersAI(creature);
    }
};

// 109943
class boss_ana_muz : public CreatureScript
{
public:
    boss_ana_muz() : CreatureScript("boss_ana_muz") {}

    struct boss_ana_muzAI : public ScriptedAI
    {
        boss_ana_muzAI(Creature* creature) : ScriptedAI(creature), summons(me) 
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        
        EventMap events;
        SummonList summons;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
            summons.DespawnAll();
            DoCast(SPELL_REMOVE_CHARM);
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 4000); // SPELL_FEL_GEYZER
            events.RescheduleEvent(EVENT_3, 7000); // SPELL_GASEOUS_BREATH
            events.RescheduleEvent(EVENT_4, 31000); // SPELL_MOTHERS_EMBRACE
        }  
        
        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
            DoCast(SPELL_REMOVE_CHARM);
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);  
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
                        DoCast(SPELL_FEL_GEYZER);
                        events.RescheduleEvent(EVENT_1, 22000);
                        events.RescheduleEvent(EVENT_2, 3000);
                        break;        
                    case EVENT_2:
                        DoCast(SPELL_IMPISH_FLAMES);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_GASEOUS_BREATH);
                        events.RescheduleEvent(EVENT_3, 31000);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_MOTHERS_EMBRACE);
                        events.RescheduleEvent(EVENT_4, 65000);
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_ana_muzAI(creature);
    }
};

// 219045
class spell_mothers_embrace : public SpellScriptLoader
{
    public:
        spell_mothers_embrace() : SpellScriptLoader("spell_mothers_embrace") {}

        class spell_mothers_embrace_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mothers_embrace_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster() || !GetTarget() || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                    return;
                if (GetCaster()->isAlive() && GetTarget()->isAlive())
                    GetCaster()->CastSpell(GetTarget(), 219068, true);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mothers_embrace_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mothers_embrace_AuraScript();
        }
};

// 110321
class boss_nazak_the_fiend : public CreatureScript
{
public:
    boss_nazak_the_fiend() : CreatureScript("boss_nazak_the_fiend") {}

    struct boss_nazak_the_fiendAI : public ScriptedAI
    {
        boss_nazak_the_fiendAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
            summons.DespawnAll();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 23000);
            events.RescheduleEvent(EVENT_2, 42000);
            events.RescheduleEvent(EVENT_3, 57000);
        }  
        
        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
        }
        
        void JustSummoned(Creature* summon) override
        {
            summons.Summon(summon);  
        }
        
        void SpellHitTarget(Unit* /*target*/, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_ABSORB_LEYSTONES)
            {
                std::list<Creature*> adds;
                GetCreatureListWithEntryInGrid(adds, me, 110417, 60.0f);
                if (!adds.empty())
                    for (std::list<Creature*>::iterator itr = adds.begin(); itr != adds.end(); ++itr)
                    {
                        if ( (*itr)->isAlive())
                        {
                            (*itr)->DespawnOrUnsummon(1000);
                            DoCast(me, SPELL_LEY_INFUSION, true);
                        }
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
                        DoCast(SPELL_CORRODING_SPRAY);
                        events.RescheduleEvent(EVENT_1, 23000);
                        break;
                    case EVENT_2:
                        DoCast(SPELL_WEB_WRAP);
                        events.RescheduleEvent(EVENT_2, 42000);
                        break;
                    case EVENT_3:
                        DoCast(SPELL_FOUNDATIONAL_COLLAPSE);
                        events.RescheduleEvent(EVENT_3, 57000);
                        events.RescheduleEvent(EVENT_4, 2000);
                        break;
                    case EVENT_4:
                        DoCast(SPELL_ABSORB_LEYSTONES);
                        break;                    
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_nazak_the_fiendAI(creature);
    }
};

// 219591
class spell_foundational_collapse : public SpellScriptLoader
{
    public:                                                      
        spell_foundational_collapse() : SpellScriptLoader("spell_foundational_collapse") {}

        class spell_foundational_collapse_AuraScript : public AuraScript 
        {
            PrepareAuraScript(spell_foundational_collapse_AuraScript) 

            void OnPereodic(AuraEffect const* aurEff) 
            {
                PreventDefaultAction();

                if (!GetCaster())
                    return;

                Position pos;
                GetCaster()->GetRandomNearPosition(pos, 27.0f);
                GetCaster()->CastSpell(pos, GetSpellInfo()->Effects[EFFECT_0]->TriggerSpell, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_foundational_collapse_AuraScript::OnPereodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_foundational_collapse_AuraScript();
        }
};

// 117470
class boss_sivash : public CreatureScript
{
public:
    boss_sivash() : CreatureScript("boss_sivash") {}

    struct boss_sivashAI : public ScriptedAI
    {
        boss_sivashAI(Creature* creature) : ScriptedAI(creature), summons(me) {}
        
        EventMap events;
        SummonList summons;
        
        void Reset() override
        {
            events.Reset();
            me->RemoveAllAreaObjects();
            summons.DespawnAll();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 25000); // summons 233968
            events.RescheduleEvent(EVENT_2, 13000); // 241433
            events.RescheduleEvent(EVENT_3, 23000); // 233996 sea with help 233977
        }  
        
        void JustDied(Unit* who) override
        {
            summons.DespawnAll();
        }
        
        void JustSummoned(Creature* summon) override
        {
            if (summon->GetEntry() != 117487)
                return;
            
            summons.Summon(summon);  
            if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                summon->AI()->AttackStart(target);
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
                        me->SummonCreature(117487, me->GetPositionX() + irand(-35, 35), me->GetPositionY() + irand(-35, 35), me->GetPositionZ());
                        me->SummonCreature(117487, me->GetPositionX() + irand(-35, 35), me->GetPositionY() + irand(-35, 35), me->GetPositionZ());
                        events.RescheduleEvent(EVENT_1, 25000);
                        break;
                    case EVENT_2:
                        DoCast(241433);
                        events.RescheduleEvent(EVENT_2, 13000);
                        break;
                    case EVENT_3:
                        DoCast(233996);
                        for (uint8 i = 0; i < urand(1, 2); ++i)
                            me->CastSpell(me->GetPositionX() + irand(-35, 35), me->GetPositionY() + irand(-35, 35), me->GetPositionZ(), 233970, true);
                        events.RescheduleEvent(EVENT_3, 23000); 
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_sivashAI(creature);
    }
};

// 13481
class areatrigger_at_tidal_wave : public AreaTriggerScript
{
    public:
        areatrigger_at_tidal_wave() : AreaTriggerScript("areatrigger_at_tidal_wave") {}

    struct areatrigger_at_tidal_waveAI : AreaTriggerAI
    {
        areatrigger_at_tidal_waveAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

        bool CalculateSpline(Position const* pos, Position& startPos, Position& endPos, std::vector<Position>& path) override
        {
            endPos.m_positionX = pos->GetPositionX() + irand(-30, 30);
            endPos.m_positionY = pos->GetPositionY() + 50;
            endPos.m_positionZ = pos->GetPositionZ() - 5;
            if (Unit* caster = at->GetCaster())
            {
                startPos.m_positionX = caster->GetPositionX() + irand(-55, 55);
                startPos.m_positionY = caster->GetPositionY() - 90;
                startPos.m_positionZ = caster->GetPositionZ() - 7;
                return true;
            }
            else
                return false;
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_tidal_waveAI(areatrigger);
    }
};

// 121124
class boss_apocron : public CreatureScript
{
public:
    boss_apocron() : CreatureScript("boss_apocron") {}

    struct boss_apocronAI : public ScriptedAI
    {
        boss_apocronAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 23000); // 241458
            events.RescheduleEvent(EVENT_2, 35000); // 241518 (random)
            events.RescheduleEvent(EVENT_3, 11000); // 241498 script (241507)
        }  

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            if (auto victim = me->getVictim())
            {
                if (abs(victim->GetPositionZ() - me->GetPositionZ()) >= 4.0f)
                {
                    EnterEvadeMode();
                    return;
                }
            }

            events.Update(diff);
                        
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:       
                        DoCast(241458);
                        events.RescheduleEvent(EVENT_1, 23000); // 241458
                        break;
                    case EVENT_2:
                        DoCast(241518);
                        events.RescheduleEvent(EVENT_2, 24000); // 241518 (random)
                        break;
                    case EVENT_3:
                        DoCast(241498);
                        me->AddDelayedEvent(2100, [this] () -> void
                        {
                            uint8 r = urand(5, 11);
                            for (uint8 i = 0; i < r; ++i)
                                DoCast(241507);
                        });
                        
                        me->AddDelayedEvent(3100, [this] () -> void
                        {
                            uint8 r = urand(5, 11);
                            for (uint8 i = 0; i < r; ++i)
                                DoCast(241507);
                        });
                        
                        me->AddDelayedEvent(4100, [this] () -> void
                        {
                            uint8 r = urand(8, 11);
                            for (uint8 i = 0; i < r; ++i)
                                DoCast(241507);
                        });
                        
                        events.RescheduleEvent(EVENT_3, 16000); // 241498 script (241507)
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_apocronAI(creature);
    }
};

// 117239
class boss_bs_brutallus : public CreatureScript
{
public:
    boss_bs_brutallus() : CreatureScript("boss_bs_brutallus") {}

    struct boss_bs_brutallusAI : public ScriptedAI
    {
        boss_bs_brutallusAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 19000); // 233484
            events.RescheduleEvent(EVENT_2, 30000); // 233566
            events.RescheduleEvent(EVENT_3, 20000); // 233515
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
                        DoCast(233484);
                        events.RescheduleEvent(EVENT_1, 19000); // 233484
                        break;
                    case EVENT_2:
                        DoCast(233566);
                        events.RescheduleEvent(EVENT_2, 18000); // 233566
                        break;
                    case EVENT_3:   
                        DoCast(233515);
                        events.RescheduleEvent(EVENT_3, 20+18000); // 233515
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_bs_brutallusAI(creature);
    }
};

// 117303
class boss_malificus : public CreatureScript
{
public:
    boss_malificus() : CreatureScript("boss_malificus") {}

    struct boss_malificusAI : public ScriptedAI
    {
        boss_malificusAI(Creature* creature) : ScriptedAI(creature) {}
        
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
        }
        
        void EnterCombat(Unit* unit) override
        {
            events.RescheduleEvent(EVENT_1, 17000); // 234452
            events.RescheduleEvent(EVENT_2, 15000); // 233614
            events.RescheduleEvent(EVENT_3, 16000); // 233569
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
                        DoCast(234452);
                        events.RescheduleEvent(EVENT_1, 19000); // 234452
                        break;
                    case EVENT_2:
                        DoCast(233614);
                        events.RescheduleEvent(EVENT_2, 16000); // 233614
                        break;
                    case EVENT_3:
                        DoCast(233569);
                        events.RescheduleEvent(EVENT_3, 18000); // 233569
                        break;
                }
            }
            
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_malificusAI(creature);
    }
};

//233484
class spell_meteor_slash : public SpellScript
{
    PrepareSpellScript(spell_meteor_slash);

    uint8 targetsCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targetsCount = targets.size();
    }

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (targetsCount)
            SetHitDamage(GetHitDamage() / targetsCount);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_meteor_slash::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
        OnEffectHitTarget += SpellEffectFn(spell_meteor_slash::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

void AddSC_world_bossess_legion()
{
    new boss_flotsam();
    new npc_jetsam_stalker();
    
    new boss_levantus();
    new spell_massive_spoul();
    
    new boss_nithogg();
    new npc_static_orb();
    new spell_static_charge();
    
    new boss_humongris();
    
    new boss_calamir();
 //   new spell_arcanopulse();
 
    new boss_withered_jim();
    new spell_resonance();
    
    new boss_shartos();
    new boss_drugon_the_frostblood();
    
    new boss_soultakers();
    
    new boss_ana_muz();
    new spell_mothers_embrace();
    
    new boss_nazak_the_fiend();
    new spell_foundational_collapse();
    
    //!  7.2.0
    
    new boss_sivash();
    new areatrigger_at_tidal_wave();
    
    new boss_apocron();
    
    new boss_bs_brutallus();
    RegisterSpellScript(spell_meteor_slash);

    new boss_malificus();
}
