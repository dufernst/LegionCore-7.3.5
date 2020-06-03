#include "blackwing_descent.h"

enum ScriptTexts
{
    // lord victor nefarius
    SAY_INTRO_1                 = 0,
    SAY_INTRO_2                 = 1,

    SAY_OMNOTRON_DEATH_2        = 2,
    SAY_OMNOTRON_INTRO          = 3,
    SAY_OMNOTRON_SPELL_1        = 4,
    SAY_OMNOTRON_SPELL_2        = 5,
    SAY_OMNOTRON_SPELL_3        = 6,
    SAY_OMNOTRON_SPELL_4        = 7,
    SAY_OMNOTRON_DEATH_1        = 8,

    SAY_MAGMAW_INTRO            = 9,
    SAY_MAGMAW_DEATH_1          = 10,
    SAY_MAGMAW_DEATH_2          = 11,
    SAY_MAGMAW_LOW_HEALTH       = 12,
    SAY_MAGMAW_SUMMON           = 13,

    SAY_MALORIAK_INTRO          = 14,
    SAY_MALORIAK_DARK           = 15,
    SAY_MALORIAK_DEATH_2        = 16,
    SAY_MALORIAK_DEATH_1        = 17,

    SAY_CHIMAERON_DEATH_1       = 23,
    SAY_CHIMAERON_DEATH_2       = 24,
    SAY_CHIMAERON_INTRO         = 25,
    SAY_CHIMAERON_SPELL         = 26,
    SAY_CHIMAERON_FEUD          = 27,
    SAY_CHIMAERON_LOW           = 28,
};

enum Spells
{
    //golem sentry
    SPELL_LASER_STRIKE_SUM              = 81063,
    SPELL_LASER_STRIKE_AURA             = 81066,
    SPELL_LASER_STRIKE_DUMMY            = 81065,
    SPELL_FLASH_BOMB_SUM                = 81056,
    SPELL_FLASH_BOMB_DMG                = 81060,
    SPELL_FLASH_BOMB_DUMMY              = 81058,
    SPELL_ELECTRICAL_DISCHARGE          = 81055,

    //drakonid drudge
    SPELL_DRAKONID_RUSH                 = 79630,
    SPELL_DRUDGE_THUNDERCLAP            = 79604,
    SPELL_DRUDGE_THUNDERCLAP_25         = 91905,
    SPELL_VENGEFUL_RAGE                 = 80035,

    //drakonid chainwielder
    SPELL_OVERHEAD_SMASH                = 79580,
    SPELL_GRIEVOUS_WOUND                = 80051,
    SPELL_CONSTRICTING_CHAINS           = 79589,
    SPELL_CONSTRICTING_CHAINS_25        = 91911,

    //maimgor
    SPELL_MAIMGOR_BERSERK               = 80084,
    SPELL_PIERCING_GRIP                 = 80145,
    SPELL_PIERCING_GRIP_VEHICLE         = 80155,
    SPELL_SHADOWFLAME                   = 79921,
    SPELL_TAIL_LASH                     = 80130,

    //ivoroc
    SPELL_CURSE_OF_MENDING              = 80295,

    //pyreclaw
    SPELL_FLAME_BUFFET                  = 80127,

    //drakonid slayer
    SPELL_BLAST_WAVE                    = 80391,
    SPELL_CLEAVE                        = 80392,
    SPELL_MORTAL_STRIKE                 = 80390,

    //drakeadon mongrel
    SPELL_BROOD_POWER_RED               = 80368,
    SPELL_BROOD_POWER_GREEN             = 80369,
    SPELL_BROOD_POWER_BLACK             = 80370,
    SPELL_BROOD_POWER_BLUE              = 80371,
    SPELL_BROOD_POWER_BRONZE            = 80372,
    SPELL_MONGREL_CHARGE                = 79904,
    SPELL_CORROSIVE_ACID                = 80345,
    SPELL_IGNITE_FLESH_DUMMY            = 80347,
    SPELL_IGNITE_FLESH                  = 80341,
    SPELL_IGNITE_FLESH_DMG              = 80342,
    SPELL_INSENERATE_DUMMY              = 80349,
    SPELL_INSENERATE                    = 80331,
    SPELL_INSENERATE_SELF               = 80332,
    SPELL_TIME_LAPSE_DUMMY              = 80348,
    SPELL_TIME_LAPSE                    = 80329,
    SPELL_TIME_LAPSE_SELF               = 80330,
    SPELL_FROST_BURN_DUMMY              = 80346,
    SPELL_FROST_BURN                    = 80336,
    SPELL_FROST_BURN_25                 = 91896,
    SPELL_FROST_BURN_SELF               = 80338,

    //spirit of angerforge
    SPELL_BESTOWAL_OF_ANGERFORGE        = 80878,
    SPELL_SPIRIT_OF_ANGERFORGE          = 80762,
    SPELL_STONEBLOOD                    = 80655,

    //spirit of thaurissan
    SPELL_BESTOWAL_OF_THAURISSAN        = 80871,
    SPELL_SPIRIT_OF_THAURISSAN          = 80766,
    SPELL_AVATAR                        = 80645,

    //spirit of ironstar
    SPELL_BESTOWAL_OF_IRONSTAR          = 80875,
    SPELL_SPIRIT_OF_IRONSTAR            = 80767,
    SPELL_EXECUTION_SENTENSE            = 80727,
    SPELL_SHIELD_OF_LIGHT               = 80747,

    //spirit of burningeye
    SPELL_BESTOWAL_OF_BURNINGEYE        = 80872,
    SPELL_SPIRIT_OF_BURNINGEYE          = 80770,
    SPELL_WHIRLWIND                     = 80652,

    //spirit of corehamer
    SPELL_BESTOWAL_OF_COREHAMMER        = 80877,
    SPELL_SPIRIT_OF_COREHAMMER          = 80763,
    SPELL_BURDEN_OF_THE_CROWN           = 80718,

    //spirit of anvilrage
    SPELL_BESTOWAL_OF_ANVILRAGE         = 80874,
    SPELL_SPIRIT_OF_ANVILRAGE           = 80768,
    SPELL_STORMBOLT                     = 80648, //править
    SPELL_STORMBOLT_25                  = 91890,

    //spirit of moltenfist
    SPELL_BESTOWAL_OF_MOLTENFIST        = 80876,
    SPELL_SPIRIT_OF_MOLTENFIST          = 80764,
    SPELL_DWARVEN_THUNDERCLAP           = 80649,

    //spirit of shadowforge
    SPELL_BESTOWAL_OF_SHADOWFORGE       = 80873,
    SPELL_SPIRIT_OF_SHADOWFORGE         = 80769,
    SPELL_DWARVEN_CHAIN_LIGHTNING       = 80646, //править
    SPELL_DWARVEN_CHAIN_LIGHTNING_25    = 91891,

    //lord victor nefarius
    //omnotron
    SPELL_SHADOW_INFUSION               = 92048,
    SPELL_SHADOW_INFUSION_AURA          = 92050,
    SPELL_GRIP_OF_DEATH                 = 91849,
    SPELL_ENCASING_SHADOWS              = 92023,
    SPELL_OVERCHARGE                    = 91881,
    SPELL_SHADOW_CONDUCTOR              = 92053,
    SPELL_OVERCHARGED_POWER_GENERATOR   = 91857,
    SPELL_LIGHTNING_CONDUCTOR           = 79888,
    SPELL_ACQUIRING_TARGET              = 79501,
    //magmaw
    SPELL_BLAZING_INFERNO_M             = 92153,
    SPELL_BLAZING_INFERNO_DMG           = 92154,
    SPELL_SHADOW_BREATH                 = 92173,
    SPELL_FIERY_SLASH                   = 92144,
    SPELL_ARMAGEDDON                    = 92177,
    SPELL_IGNITION                      = 92131,
    SPELL_IGNITION_TRIGGER_SPAWN        = 92121,
    SPELL_IGNITION_TRIGGER_M            = 92119,
    //chimaeron
    SPELL_SHADOW_WHIP                   = 91304,
    SPELL_MOCKING_SHADOWS               = 91307,
};

enum CreaturesIds
{
    NPC_GOLEM_SENTRY                = 42800,
    NPC_LASER_STRIKE                = 43362,
    NPC_FLASH_BOMB                  = 43361,

    NPC_DRAKONID_DRUDGE             = 42362,

    NPC_DRAKONID_CHAINWIELDER       = 42649,

    NPC_MAIMGOR                     = 42768,
    NPC_PYRECLAW                    = 42764,
    NPC_IVOROC                      = 42767,

    NPC_DRAKONID_SLAYER             = 42802,
    NPC_DRAKEADON_MONGREL_1         = 46083,
    NPC_DRAKEADON_MONGREL_2         = 42803,

    NPC_SPIRIT_OF_ANGERFORGE        = 43119,
    NPC_SPIRIT_OF_THAURISSAN        = 43126,
    NPC_SPIRIT_OF_IRONSTAR          = 43127,
    NPC_SPIRIT_OF_BURNINGEYE        = 43130,
    NPC_SPIRIT_OF_COREHAMMER        = 43122,
    NPC_SPIRIT_OF_ANVILRAGE         = 43128,
    NPC_SPIRIT_OF_MOLTENFIST        = 43125,
    NPC_SPIRIT_OF_SHADOWFORGE       = 43129,

    NPC_POWER_GENERATOR             = 42733,
    NPC_CHEMICAL_CLOUD              = 42934,

    NPC_IGNITION_TRIGGER            = 49447,
    NPC_BLAZING_BONE_CONSTRUCT      = 49416,
};

enum Events
{
    //golem sentry
    EVENT_ELECTRICAL_DISCHARGE      = 1,
    EVENT_FLASH_BOMB                = 2,
    EVENT_FLASH_BOMB_DMG            = 3,
    EVENT_LASER_STRIKE              = 4,
    EVENT_LASER_STRIKE_DMG          = 5,

    //drakonid drudge
    EVENT_DRAKONID_RUSH             = 6,
    EVENT_THUNDERCLAP               = 7,

    //drakonid chainwielder
    EVENT_OVERHEAD_SMASH            = 8,
    EVENT_GRIEVOUS_WOUND            = 9,
    EVENT_CONSTRICTING_CHAINS       = 10,

    //maimgor
    EVENT_PIERCING_GRIP             = 11,
    EVENT_SHADOWFLAME               = 12,
    EVENT_TAIL_LASH                 = 13,
    EVENT_MAIMGOR_BERSERK           = 14,

    //ivoroc
    EVENT_CURSE_OF_MENDING          = 15,

    //pyreclaw
    EVENT_FLAME_BUFFET              = 16,

    //drakonid slayer
    EVENT_BLAST_WAVE                = 17,
    EVENT_CLEAVE                    = 18,
    EVENT_MORTAL_STRIKE             = 19,

    //drakeadon mongrel
    EVENT_MONGREL_CHARGE            = 20,
    EVENT_CORROSIVE_ACID            = 21,
    EVENT_IGNITE_FLESH              = 22,
    EVENT_INSENERATE                = 23,
    EVENT_FROST_BURN                = 24,
    EVENT_TIME_LAPSE                = 25,

    //spirit of angerforge
    EVENT_STONEBLOOD                = 26,

    //spirit of thaurissan
    EVENT_AVATAR                    = 27,

    //spirit of ironstar
    EVENT_EXECUTION_SENTENSE        = 28,
    EVENT_SHIELD_OF_LIGHT           = 29,

    //spirit of burningeye
    EVENT_WHIRLWIND                 = 30,

    EVENT_STORMBOLT                 = 31,

    EVENT_BURDEN_OF_THE_CROWN       = 32,

    EVENT_CHAIN_LIGHTNING           = 33,

    EVENT_DWARVEN_THUNDERCLAP       = 34,

    //lord victor nefarius
    EVENT_NEFARIUS_DESPAWN          = 35,
    //omnotron
    EVENT_SHADOW_INFUSION           = 36,
    EVENT_JUMP_TO                   = 37,
    EVENT_GRIP_OF_DEATH             = 38,
    EVENT_JUMP_OUT                  = 39,
    EVENT_OVERCHARGE                = 40,
    EVENT_ENCASING_SHADOWS          = 41,
    EVENT_NEXT_SKILL                = 42,
    EVENT_OMNOTRON_INTRO            = 43,
    EVENT_OMNOTRON_DEATH            = 44,
    EVENT_OMNOTRON_SPELL_1          = 45,
    EVENT_OMNOTRON_SPELL_2          = 46,
    EVENT_OMNOTRON_SPELL_3          = 47,
    EVENT_OMNOTRON_SPELL_4          = 48,
    //magmaw
    EVENT_BLAZING_INFERNO           = 49,
    EVENT_SHADOW_BREATH             = 50,
    EVENT_FIERY_SLASH               = 51,
    EVENT_MAGMAW_INTRO              = 52,
    EVENT_MAGMAW_DEATH              = 53,
    EVENT_MAGMAW_SUMMON             = 54,
    EVENT_MAGMAW_LOW_HEALTH         = 55,
    //maloriak
    EVENT_MALORIAK_DARK_MAGIC       = 56,
    EVENT_MALORIAK_DEATH            = 57,
    EVENT_MALORIAK_INTRO            = 58,
    //chimaeron
    EVENT_CHIMAERON_INTRO           = 59,
    EVENT_CHIMAERON_DEATH           = 60,
    EVENT_CHIMAERON_FEUD            = 61,
    EVENT_CHIMAERON_LOW             = 62,
};

class npc_golem_sentry : public CreatureScript
{
public:
    npc_golem_sentry() : CreatureScript("npc_golem_sentry") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_golem_sentryAI(pCreature);
    }

    struct npc_golem_sentryAI : public ScriptedAI
    {
        npc_golem_sentryAI(Creature* pCreature) : ScriptedAI(pCreature), summons(me)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        SummonList summons;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
            summons.DespawnAll();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void JustDied(Unit* who)
        {
            summons.DespawnAll();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;
            std::list<Creature*> _golems;
            GetCreatureListWithEntryInGrid(_golems, me, NPC_GOLEM_SENTRY, 200.0f);
            for (std::list<Creature*>::iterator itr = _golems.begin(); itr != _golems.end(); ++itr)
                if ((*itr)->GetGUID() != me->GetGUID())
                    (*itr)->AI()->AttackStart(who);

            events.RescheduleEvent(EVENT_ELECTRICAL_DISCHARGE, urand(3000, 15000));
            events.RescheduleEvent(EVENT_FLASH_BOMB, urand(10000, 15000));
            events.RescheduleEvent(EVENT_LASER_STRIKE, urand(7000, 12000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ELECTRICAL_DISCHARGE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_ELECTRICAL_DISCHARGE);
                    events.RescheduleEvent(EVENT_ELECTRICAL_DISCHARGE, urand(10000, 15000));
                    break;
                case EVENT_LASER_STRIKE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_LASER_STRIKE_SUM);
                    events.RescheduleEvent(EVENT_LASER_STRIKE, urand(15000, 20000));
                    break;
                case EVENT_FLASH_BOMB:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_FLASH_BOMB_SUM);
                    events.RescheduleEvent(EVENT_FLASH_BOMB, urand(15000, 20000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class npc_flash_bomb : public CreatureScript
{
public:
    npc_flash_bomb() : CreatureScript("npc_flash_bomb") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_flash_bombAI(pCreature);
    }

    struct npc_flash_bombAI : public Scripted_NoMovementAI
    {
        npc_flash_bombAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            DoCast(me, SPELL_FLASH_BOMB_DUMMY);
        }

        void IsSummonedBy(Unit* owner)
        {
            events.RescheduleEvent(EVENT_FLASH_BOMB_DMG, 3000);
        }

        void JustDied(Unit* who)
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FLASH_BOMB_DMG:
                    me->RemoveAurasDueToSpell(SPELL_FLASH_BOMB_DUMMY);
                    DoCast(me, SPELL_FLASH_BOMB_DMG);
                    break;
                }
            }
        }
    };
};

class npc_laser_strike : public CreatureScript
{
public:
    npc_laser_strike() : CreatureScript("npc_laser_strike") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_laser_strikeAI(pCreature);
    }

    struct npc_laser_strikeAI : public Scripted_NoMovementAI
    {
        npc_laser_strikeAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            DoCast(me, SPELL_LASER_STRIKE_DUMMY);
        }

        void IsSummonedBy(Unit* owner)
        {
            events.RescheduleEvent(EVENT_LASER_STRIKE_DMG, 3000);
        }

        void JustDied(Unit* who)
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_LASER_STRIKE_DMG:
                    me->RemoveAurasDueToSpell(SPELL_LASER_STRIKE_DUMMY);
                    DoCast(me, SPELL_LASER_STRIKE_AURA);
                    break;
                }
            }
        }
    };
};

class npc_drakonid_drudge : public CreatureScript
{
public:
    npc_drakonid_drudge() : CreatureScript("npc_drakonid_drudge") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_drakonid_drudgeAI(pCreature);
    }

    struct npc_drakonid_drudgeAI : public ScriptedAI
    {
        npc_drakonid_drudgeAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }

        void JustDied(Unit* who)
        {
            std::list<Creature*> _drudges;
            GetCreatureListWithEntryInGrid(_drudges, me, NPC_GOLEM_SENTRY, 200.0f);
            for (std::list<Creature*>::iterator itr = _drudges.begin(); itr != _drudges.end(); ++itr)
                if ((*itr)->GetGUID() != me->GetGUID())
                    DoCast((*itr), SPELL_VENGEFUL_RAGE);            
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            std::list<Creature*> _drudges;
            GetCreatureListWithEntryInGrid(_drudges, me, NPC_GOLEM_SENTRY, 200.0f);
            for (std::list<Creature*>::iterator itr = _drudges.begin(); itr != _drudges.end(); ++itr)
                if ((*itr)->GetGUID() != me->GetGUID())
                    (*itr)->AI()->AttackStart(who);

            events.RescheduleEvent(EVENT_DRAKONID_RUSH, 12000);
            events.RescheduleEvent(EVENT_THUNDERCLAP, urand(4000, 5000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_THUNDERCLAP:
                    DoCast(me, SPELL_DRUDGE_THUNDERCLAP);
                    events.RescheduleEvent(EVENT_THUNDERCLAP, urand(15000, 17000));
                    break;
                case EVENT_DRAKONID_RUSH:
                    std::list<Creature*> _drudges;
                    GetCreatureListWithEntryInGrid(_drudges, me, NPC_GOLEM_SENTRY, 200.0f);
                    for (std::list<Creature*>::iterator itr = _drudges.begin(); itr != _drudges.end(); ++itr)
                        if ((*itr)->GetGUID() != me->GetGUID() && (*itr)->isAlive())
                            if (Unit* target = (*itr)->AI()->SelectTarget(SELECT_TARGET_TOPAGGRO))
                                DoCast(target, SPELL_DRAKONID_RUSH);                    
                    events.RescheduleEvent(EVENT_DRAKONID_RUSH, 20000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_drakonid_chainwielder : public CreatureScript
{
public:
    npc_drakonid_chainwielder() : CreatureScript("npc_drakonid_chainwielder") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_drakonid_chainwielderAI(pCreature);
    }

    struct npc_drakonid_chainwielderAI : public ScriptedAI
    {
        npc_drakonid_chainwielderAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }


        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_OVERHEAD_SMASH, urand(5000, 10000));
            events.RescheduleEvent(EVENT_GRIEVOUS_WOUND, urand(8000, 10000));
            events.RescheduleEvent(EVENT_CONSTRICTING_CHAINS, urand(10000, 20000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_OVERHEAD_SMASH:
                    DoCast(me->getVictim(), SPELL_OVERHEAD_SMASH);
                    events.RescheduleEvent(EVENT_OVERHEAD_SMASH, urand(8000, 10000));
                    break;
                case EVENT_GRIEVOUS_WOUND:
                    if (me->getVictim())
                        if (!me->getVictim()->HasAura(SPELL_GRIEVOUS_WOUND))
                            DoCast(me->getVictim(), SPELL_GRIEVOUS_WOUND);
                    events.RescheduleEvent(EVENT_GRIEVOUS_WOUND, urand(15000, 20000));
                    break;
                case EVENT_CONSTRICTING_CHAINS:
                    DoCast(SPELL_CONSTRICTING_CHAINS);
                    events.RescheduleEvent(EVENT_CONSTRICTING_CHAINS, urand(14000, 18000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_maimgor: public CreatureScript
{
public:
    npc_maimgor() : CreatureScript("npc_maimgor") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_maimgorAI(pCreature);
    }

    struct npc_maimgorAI : public ScriptedAI
    {
        npc_maimgorAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_MAIMGOR_BERSERK, urand(5000, 10000));
            events.RescheduleEvent(EVENT_SHADOWFLAME, urand(8000, 12000));
            events.RescheduleEvent(EVENT_PIERCING_GRIP, urand(10000, 15000));
            events.RescheduleEvent(EVENT_TAIL_LASH, urand(4000, 6000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MAIMGOR_BERSERK:
                    DoCast(me, SPELL_MAIMGOR_BERSERK);
                    events.RescheduleEvent(EVENT_MAIMGOR_BERSERK, urand(20000, 25000));
                    break;
                case EVENT_TAIL_LASH:
                    DoCast(me, SPELL_TAIL_LASH);
                    events.RescheduleEvent(EVENT_TAIL_LASH, urand(5000, 6000));
                    break;
                case EVENT_SHADOWFLAME:
                    DoCast(me, SPELL_SHADOWFLAME);
                    events.RescheduleEvent(EVENT_SHADOWFLAME, urand(12000, 20000));
                    break;
                case EVENT_PIERCING_GRIP:
                    DoCast(me->getVictim(), SPELL_PIERCING_GRIP);
                    events.RescheduleEvent(EVENT_PIERCING_GRIP, urand(15000, 20000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_ivoroc: public CreatureScript
{
public:
    npc_ivoroc() : CreatureScript("npc_ivoroc") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_ivorocAI(pCreature);
    }

    struct npc_ivorocAI : public ScriptedAI
    {
        npc_ivorocAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_SHADOWFLAME, urand(8000, 12000));
            events.RescheduleEvent(EVENT_CURSE_OF_MENDING, urand(5000, 10000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SHADOWFLAME:
                    DoCast(me, SPELL_SHADOWFLAME);
                    events.RescheduleEvent(EVENT_SHADOWFLAME, urand(12000, 20000));
                    break;
                case EVENT_CURSE_OF_MENDING:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        DoCast(target, SPELL_CURSE_OF_MENDING);
                    events.RescheduleEvent(EVENT_CURSE_OF_MENDING, urand(15000, 17000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_pyreclaw: public CreatureScript
{
public:
    npc_pyreclaw() : CreatureScript("npc_pyreclaw") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_pyreclawAI(pCreature);
    }

    struct npc_pyreclawAI : public ScriptedAI
    {
        npc_pyreclawAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_SHADOWFLAME, urand(8000, 12000));
            events.RescheduleEvent(EVENT_FLAME_BUFFET, urand(6000, 7000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SHADOWFLAME:
                    DoCast(me, SPELL_SHADOWFLAME);
                    events.RescheduleEvent(EVENT_SHADOWFLAME, urand(12000, 20000));
                    break;
                case EVENT_FLAME_BUFFET:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        DoCast(target, SPELL_FLAME_BUFFET);
                    events.RescheduleEvent(EVENT_FLAME_BUFFET, urand(6000, 7000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_drakonid_slayer: public CreatureScript
{
public:
    npc_drakonid_slayer() : CreatureScript("npc_drakonid_slayer") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_drakonid_slayerAI(pCreature);
    }

    struct npc_drakonid_slayerAI : public ScriptedAI
    {
        npc_drakonid_slayerAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_CLEAVE, urand(5000, 8000));
            events.RescheduleEvent(EVENT_BLAST_WAVE, urand(10000, 15000));
            events.RescheduleEvent(EVENT_MORTAL_STRIKE, urand(3000, 10000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BLAST_WAVE:
                    DoCast(me, SPELL_BLAST_WAVE);
                    events.RescheduleEvent(EVENT_BLAST_WAVE, urand(15000, 20000));
                    break;
                case EVENT_CLEAVE:
                    DoCast(me->getVictim(), SPELL_CLEAVE);
                    events.RescheduleEvent(EVENT_CLEAVE, urand(8000, 10000));
                    break;
                case EVENT_MORTAL_STRIKE:
                    DoCast(me->getVictim(), SPELL_MORTAL_STRIKE);
                    events.RescheduleEvent(EVENT_MORTAL_STRIKE, urand(12000, 15000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_drakeadon_mongrel: public CreatureScript
{
public:
    npc_drakeadon_mongrel() : CreatureScript("npc_drakeadon_mongrel") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_drakeadon_mongrelAI(pCreature);
    }

    struct npc_drakeadon_mongrelAI : public ScriptedAI
    {
        npc_drakeadon_mongrelAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            events.RescheduleEvent(EVENT_FROST_BURN, urand(10000, 30000));
            events.RescheduleEvent(EVENT_INSENERATE, urand(10000, 30000));
            events.RescheduleEvent(EVENT_IGNITE_FLESH, urand(20000, 30000));
            events.RescheduleEvent(EVENT_TIME_LAPSE, urand(10000, 30000));
            events.RescheduleEvent(EVENT_CORROSIVE_ACID, urand(6000, 10000));
            events.RescheduleEvent(EVENT_MONGREL_CHARGE, urand(7000, 12000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MONGREL_CHARGE:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        DoCast(target, SPELL_MONGREL_CHARGE);
                    events.RescheduleEvent(EVENT_MONGREL_CHARGE, urand(12000, 15000));
                    break;
                case EVENT_IGNITE_FLESH:
                    DoCast(SPELL_IGNITE_FLESH);
                    events.RescheduleEvent(EVENT_IGNITE_FLESH, urand(30000, 40000));
                    break;
                case EVENT_FROST_BURN:
                    if (urand(0, 1))
                        DoCast(me, SPELL_FROST_BURN);
                    else
                        DoCast(me, SPELL_FROST_BURN_SELF);
                    events.RescheduleEvent(EVENT_FROST_BURN, urand(15000, 30000));
                    break;
                case EVENT_INSENERATE:
                    if (urand(0, 1))
                        DoCast(me, SPELL_INSENERATE);
                    else
                        DoCast(me, SPELL_INSENERATE_SELF);
                    events.RescheduleEvent(EVENT_INSENERATE, urand(15000, 30000));
                    break;
                case EVENT_TIME_LAPSE:
                    if (urand(0, 1))
                        DoCast(me, SPELL_TIME_LAPSE);
                    else
                        DoCast(me, SPELL_TIME_LAPSE_SELF);
                    events.RescheduleEvent(EVENT_TIME_LAPSE, urand(15000, 30000));
                    break;
                case EVENT_CORROSIVE_ACID:
                    DoCast(me, SPELL_CORROSIVE_ACID);
                    events.RescheduleEvent(EVENT_CORROSIVE_ACID, urand(20000, 30000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_bd_spirit_of_dwarf: public CreatureScript
{
public:
    npc_bd_spirit_of_dwarf() : CreatureScript("npc_bd_spirit_of_dwarf") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_bd_spirit_of_dwarfAI(pCreature);
    }

    struct npc_bd_spirit_of_dwarfAI : public ScriptedAI
    {
        npc_bd_spirit_of_dwarfAI(Creature* pCreature) : ScriptedAI(pCreature)
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
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            if (!instance)
                return;

            switch (me->GetEntry())
            {
            case NPC_SPIRIT_OF_ANGERFORGE:
                DoCast(me, SPELL_SPIRIT_OF_ANGERFORGE);
                break;
            case NPC_SPIRIT_OF_THAURISSAN:
                DoCast(me, SPELL_SPIRIT_OF_THAURISSAN);
                break;
            case NPC_SPIRIT_OF_IRONSTAR:
                DoCast(me, SPELL_SPIRIT_OF_IRONSTAR);
                break;
            case NPC_SPIRIT_OF_BURNINGEYE:
                DoCast(me, SPELL_SPIRIT_OF_BURNINGEYE);
                break;
            case NPC_SPIRIT_OF_COREHAMMER:
                DoCast(me, SPELL_SPIRIT_OF_COREHAMMER);
                break;
            case NPC_SPIRIT_OF_ANVILRAGE:
                DoCast(me, SPELL_SPIRIT_OF_ANVILRAGE);
                break;
            case NPC_SPIRIT_OF_MOLTENFIST:
                DoCast(me, SPELL_SPIRIT_OF_MOLTENFIST);
                break;
            case NPC_SPIRIT_OF_SHADOWFORGE:
                DoCast(me, SPELL_SPIRIT_OF_SHADOWFORGE);
                break;
            }
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (!instance)
                return;

            switch (me->GetEntry())
            {
            case NPC_SPIRIT_OF_ANGERFORGE:
                if (Creature* _thaurissan = me->FindNearestCreature(NPC_SPIRIT_OF_THAURISSAN, 200.0f))
                    _thaurissan->AI()->AttackStart(who);
                if (Creature* _ironstar = me->FindNearestCreature(NPC_SPIRIT_OF_IRONSTAR, 200.0f))
                    _ironstar->AI()->AttackStart(who);
                if (Creature* _burningeye = me->FindNearestCreature(NPC_SPIRIT_OF_BURNINGEYE, 200.0f))
                    _burningeye->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_STONEBLOOD, urand(10000, 20000));
                break;
            case NPC_SPIRIT_OF_THAURISSAN:
                if (Creature* _angerforge = me->FindNearestCreature(NPC_SPIRIT_OF_ANGERFORGE, 200.0f))
                    _angerforge->AI()->AttackStart(who);
                if (Creature* _ironstar = me->FindNearestCreature(NPC_SPIRIT_OF_IRONSTAR, 200.0f))
                    _ironstar->AI()->AttackStart(who);
                if (Creature* _burningeye = me->FindNearestCreature(NPC_SPIRIT_OF_BURNINGEYE, 200.0f))
                    _burningeye->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_AVATAR, urand(10000, 20000));
                break;
            case NPC_SPIRIT_OF_IRONSTAR:
                if (Creature* _thaurissan = me->FindNearestCreature(NPC_SPIRIT_OF_THAURISSAN, 200.0f))
                    _thaurissan->AI()->AttackStart(who);
                if (Creature* _angerforge = me->FindNearestCreature(NPC_SPIRIT_OF_ANGERFORGE, 200.0f))
                    _angerforge->AI()->AttackStart(who);
                if (Creature* _burningeye = me->FindNearestCreature(NPC_SPIRIT_OF_BURNINGEYE, 200.0f))
                    _burningeye->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_EXECUTION_SENTENSE, urand(15000, 20000));
                break;
            case NPC_SPIRIT_OF_BURNINGEYE:
                if (Creature* _thaurissan = me->FindNearestCreature(NPC_SPIRIT_OF_THAURISSAN, 200.0f))
                    _thaurissan->AI()->AttackStart(who);
                if (Creature* _angerforge = me->FindNearestCreature(NPC_SPIRIT_OF_ANGERFORGE, 200.0f))
                    _angerforge->AI()->AttackStart(who);
                if (Creature* _ironstar = me->FindNearestCreature(NPC_SPIRIT_OF_IRONSTAR, 200.0f))
                    _ironstar->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_WHIRLWIND, urand(10000, 20000));
                break;
            case NPC_SPIRIT_OF_COREHAMMER:
                if (Creature* _anvilrage = me->FindNearestCreature(NPC_SPIRIT_OF_ANVILRAGE, 200.0f))
                    _anvilrage->AI()->AttackStart(who);
                if (Creature* _moltenfist = me->FindNearestCreature(NPC_SPIRIT_OF_MOLTENFIST, 200.0f))
                    _moltenfist->AI()->AttackStart(who);
                if (Creature* _shadowforge = me->FindNearestCreature(NPC_SPIRIT_OF_SHADOWFORGE, 200.0f))
                    _shadowforge->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_BURDEN_OF_THE_CROWN, urand(10000, 20000));
                break;
            case NPC_SPIRIT_OF_ANVILRAGE:
                if (Creature* _corehammer = me->FindNearestCreature(NPC_SPIRIT_OF_COREHAMMER, 200.0f))
                    _corehammer->AI()->AttackStart(who);
                if (Creature* _moltenfist = me->FindNearestCreature(NPC_SPIRIT_OF_MOLTENFIST, 200.0f))
                    _moltenfist->AI()->AttackStart(who);
                if (Creature* _shadowforge = me->FindNearestCreature(NPC_SPIRIT_OF_SHADOWFORGE, 200.0f))
                    _shadowforge->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_STORMBOLT, urand(10000, 20000));
                break;
            case NPC_SPIRIT_OF_MOLTENFIST:
                if (Creature* _corehammer = me->FindNearestCreature(NPC_SPIRIT_OF_COREHAMMER, 200.0f))
                    _corehammer->AI()->AttackStart(who);
                if (Creature* _anvilrage = me->FindNearestCreature(NPC_SPIRIT_OF_ANVILRAGE, 200.0f))
                    _anvilrage->AI()->AttackStart(who);
                if (Creature* _shadowforge = me->FindNearestCreature(NPC_SPIRIT_OF_SHADOWFORGE, 200.0f))
                    _shadowforge->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_DWARVEN_THUNDERCLAP, urand(10000, 20000));
                break;
            case NPC_SPIRIT_OF_SHADOWFORGE:
                if (Creature* _corehammer = me->FindNearestCreature(NPC_SPIRIT_OF_COREHAMMER, 200.0f))
                    _corehammer->AI()->AttackStart(who);
                if (Creature* _anvilrage = me->FindNearestCreature(NPC_SPIRIT_OF_ANVILRAGE, 200.0f))
                    _anvilrage->AI()->AttackStart(who);
                if (Creature* _moltenfist = me->FindNearestCreature(NPC_SPIRIT_OF_MOLTENFIST, 200.0f))
                    _moltenfist->AI()->AttackStart(who);
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(10000, 20000));
                break;
            }
        }

        void JustDied(Unit* killer)
        {
            if (!instance)
                return;

            switch (me->GetEntry())
            {
            case NPC_SPIRIT_OF_ANGERFORGE:
                DoCast(SPELL_BESTOWAL_OF_ANGERFORGE);
                break;
            case NPC_SPIRIT_OF_THAURISSAN:
                DoCast(SPELL_BESTOWAL_OF_THAURISSAN);
                break;
            case NPC_SPIRIT_OF_IRONSTAR:
                DoCast(SPELL_BESTOWAL_OF_IRONSTAR);
                break;
            case NPC_SPIRIT_OF_BURNINGEYE:
                DoCast(SPELL_BESTOWAL_OF_BURNINGEYE);
                break;
            case NPC_SPIRIT_OF_COREHAMMER:
                DoCast(SPELL_BESTOWAL_OF_COREHAMMER);
                break;
            case NPC_SPIRIT_OF_ANVILRAGE:
                DoCast(SPELL_BESTOWAL_OF_ANVILRAGE);
                break;
            case NPC_SPIRIT_OF_MOLTENFIST:
                DoCast(SPELL_BESTOWAL_OF_MOLTENFIST);
                break;
            case NPC_SPIRIT_OF_SHADOWFORGE:
                DoCast(SPELL_BESTOWAL_OF_SHADOWFORGE);
                break;
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            switch (spell->Id)
            {
            case SPELL_SPIRIT_OF_ANGERFORGE:
                events.RescheduleEvent(EVENT_STONEBLOOD, urand(10000, 20000));
                break;
            case SPELL_SPIRIT_OF_THAURISSAN:
                events.RescheduleEvent(EVENT_AVATAR, urand(10000, 20000));
                break;
            case SPELL_SPIRIT_OF_IRONSTAR:
                events.RescheduleEvent(EVENT_EXECUTION_SENTENSE, urand(15000, 20000));
                break;
            case SPELL_SPIRIT_OF_BURNINGEYE:
                events.RescheduleEvent(EVENT_WHIRLWIND, urand(10000, 20000));
                break;
            case SPELL_SPIRIT_OF_COREHAMMER:
                events.RescheduleEvent(EVENT_BURDEN_OF_THE_CROWN, urand(10000, 20000));
                break;
            case SPELL_SPIRIT_OF_ANVILRAGE:
                events.RescheduleEvent(EVENT_STORMBOLT, urand(10000, 20000));
                break;
            case SPELL_SPIRIT_OF_MOLTENFIST:
                events.RescheduleEvent(EVENT_THUNDERCLAP, urand(10000, 20000));
                break;
            case SPELL_SPIRIT_OF_SHADOWFORGE:
                events.RescheduleEvent(EVENT_CHAIN_LIGHTNING, urand(10000, 20000));
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_STONEBLOOD:
                    DoCast(me, SPELL_STONEBLOOD);
                    events.RescheduleEvent(EVENT_STONEBLOOD, urand(15000, 25000));
                    break;
                case EVENT_AVATAR:
                    DoCast(me, SPELL_AVATAR);
                    events.RescheduleEvent(EVENT_AVATAR, urand(25000, 30000));
                    break;
                case EVENT_EXECUTION_SENTENSE:

                    events.RescheduleEvent(EVENT_EXECUTION_SENTENSE, urand(25000, 30000));
                    break;
                case EVENT_WHIRLWIND:
                    DoCast(me, SPELL_WHIRLWIND);
                    events.RescheduleEvent(EVENT_WHIRLWIND, urand(25000, 30000));
                    break;
                case EVENT_STORMBOLT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_STORMBOLT);
                    events.RescheduleEvent(EVENT_STORMBOLT, urand(18000, 23000));
                    break;
                case EVENT_DWARVEN_THUNDERCLAP:
                    DoCast(me, SPELL_DWARVEN_THUNDERCLAP);
                    events.RescheduleEvent(EVENT_DWARVEN_THUNDERCLAP, urand(19000, 30000));
                    break;
                case EVENT_CHAIN_LIGHTNING:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_DWARVEN_CHAIN_LIGHTNING);
                    break;
                case EVENT_BURDEN_OF_THE_CROWN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_BURDEN_OF_THE_CROWN);
                    events.RescheduleEvent(EVENT_BURDEN_OF_THE_CROWN, urand(30000, 40000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class spell_drakonid_chainwielder_grievous_wound : public SpellScriptLoader
{
public:
    spell_drakonid_chainwielder_grievous_wound() : SpellScriptLoader("spell_drakonid_chainwielder_grievous_wound") { }

    class spell_drakonid_chainwielder_grievous_wound_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_drakonid_chainwielder_grievous_wound_AuraScript)

            void OnPeriodic(AuraEffect const*aurEff)
        {
            if (!GetTarget())
                return;

            if (GetTarget()->HealthAbovePct(90))
                GetTarget()->RemoveAurasDueToSpell(aurEff->GetId());
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_drakonid_chainwielder_grievous_wound_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_drakonid_chainwielder_grievous_wound_AuraScript();
    }
};

class npc_lord_victor_nefarius_heroic : public CreatureScript
{
public:
    npc_lord_victor_nefarius_heroic() : CreatureScript("npc_lord_victor_nefarius_heroic") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lord_victor_nefarius_heroicAI(pCreature);
    }

    struct npc_lord_victor_nefarius_heroicAI : public Scripted_NoMovementAI
    {
        npc_lord_victor_nefarius_heroicAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature), summons(me)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            instance = pCreature->GetInstanceScript();
        }

        EventMap events;
        SummonList summons;
        InstanceScript* instance;
        bool bAllowCast;
        Position homepos;        

        void Reset()
        {
            if (!instance)
                return;

            me->SetCanFly(true);
            me->SetReactState(REACT_PASSIVE);
            me->setFaction(16);
            bAllowCast = true;
            summons.DespawnAll();
            events.Reset();
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (me->isInCombat())
                summon->SetInCombatWithZone();
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void DoAction(const int32 action)
        {
            switch (action)
            {
            case ACTION_OMNOTRON_INTRO:
                events.RescheduleEvent(EVENT_OMNOTRON_INTRO, 3000);
                break;
            case ACTION_OMNOTRON_DEATH:
                events.RescheduleEvent(EVENT_OMNOTRON_DEATH, 3000);
                break;
            case ACTION_MAGMAW_INTRO:
                events.RescheduleEvent(EVENT_MAGMAW_INTRO, 1000);
                break;
            case ACTION_MAGMAW_DEATH:
                events.RescheduleEvent(EVENT_MAGMAW_DEATH, 1000);
                break;
            case ACTION_MALORIAK_INTRO:
                events.RescheduleEvent(EVENT_MALORIAK_INTRO, 7000);
                break;
            case ACTION_MALORIAK_DEATH:
                events.RescheduleEvent(EVENT_MALORIAK_DEATH, 3000);
                break;
            case ACTION_CHIMAERON_INTRO:
                events.RescheduleEvent(EVENT_CHIMAERON_INTRO, 1000);
                break;
            case ACTION_CHIMAERON_DEATH:
                events.RescheduleEvent(EVENT_CHIMAERON_DEATH, 1000);
                break;
            }
            switch (action)
            {
            case ACTION_GRIP_OF_DEATH:
                if (roll_chance_i(50) && bAllowCast)
                {
                    bAllowCast = false;
                    events.RescheduleEvent(EVENT_NEXT_SKILL, 30000);
                    events.RescheduleEvent(EVENT_JUMP_TO, 1000);
                }
                break;
            case ACTION_SHADOW_INFUSION:
                if (roll_chance_i(50) && bAllowCast)
                {
                    bAllowCast = false;
                    events.RescheduleEvent(EVENT_NEXT_SKILL, 30000);
                    events.RescheduleEvent(EVENT_SHADOW_INFUSION, 1000);
                }
                break;
            case ACTION_OVERCHARGE:
                if (roll_chance_i(50) && bAllowCast)
                {
                    bAllowCast = false;
                    events.RescheduleEvent(EVENT_NEXT_SKILL, 30000);
                    events.RescheduleEvent(EVENT_OVERCHARGE, 1000);
                }
                break;
            case ACTION_ENCASING_SHADOWS:
                if (roll_chance_i(50) && bAllowCast)
                {
                    bAllowCast = false;
                    events.RescheduleEvent(EVENT_NEXT_SKILL, 30000);
                    events.RescheduleEvent(EVENT_ENCASING_SHADOWS, 1000);
                }
                break;
            case ACTION_BLAZING_INFERNO:
                events.RescheduleEvent(EVENT_BLAZING_INFERNO, 20000);
                break;
            case ACTION_SHADOW_BREATH:
                Talk(SAY_MAGMAW_LOW_HEALTH);
                events.CancelEvent(EVENT_BLAZING_INFERNO);
                events.RescheduleEvent(EVENT_SHADOW_BREATH, 4000);
                break;
            case ACTION_MALORIAK_DARK_MAGIC:
                events.RescheduleEvent(EVENT_MALORIAK_DARK_MAGIC, 1000);
                break;
            case ACTION_CHIMAERON_FEUD:
                events.RescheduleEvent(EVENT_CHIMAERON_FEUD, 500);
                break;
            case ACTION_CHIMAERON_LOW:
                events.RescheduleEvent(EVENT_CHIMAERON_LOW, 1000);
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_NEXT_SKILL:
                    bAllowCast = true;
                    break;
                case EVENT_SHADOW_INFUSION:
                    Talk(SAY_OMNOTRON_SPELL_3);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, SPELL_LIGHTNING_CONDUCTOR))
                    {
                        pTarget->RemoveAurasDueToSpell(SPELL_LIGHTNING_CONDUCTOR);
                        me->AddAura(SPELL_SHADOW_CONDUCTOR, pTarget);
                    }
                    break;
                case EVENT_OVERCHARGE:
                    Talk(SAY_OMNOTRON_SPELL_2);
                    DoCast(me, SPELL_OVERCHARGE);
                    break;
                case EVENT_ENCASING_SHADOWS:
                    Talk(SAY_OMNOTRON_SPELL_1);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, SPELL_ACQUIRING_TARGET))
                        me->AddAura(SPELL_ENCASING_SHADOWS, pTarget);
                    break;
                case EVENT_JUMP_TO:
                    if (Creature* cloud = me->FindNearestCreature(NPC_CHEMICAL_CLOUD, 200.0f))
                    {
                        me->GetPosition(&homepos);
                        me->JumpTo(cloud, 20.0f);
                        events.RescheduleEvent(EVENT_GRIP_OF_DEATH, 2000);
                    }
                    break;
                case EVENT_GRIP_OF_DEATH:
                    Talk(SAY_OMNOTRON_SPELL_4);
                    DoCast(me, SPELL_GRIP_OF_DEATH, true);
                    events.RescheduleEvent(EVENT_JUMP_OUT, 3000);
                    break;
                case EVENT_JUMP_OUT:
                    me->GetMotionMaster()->MoveJump(homepos.GetPositionX(),
                        homepos.GetPositionY(),
                        homepos.GetPositionZ(),
                        40.0f, 20.0f);
                    break;
                case EVENT_OMNOTRON_INTRO:
                    Talk(SAY_OMNOTRON_INTRO);
                    break;
                case EVENT_OMNOTRON_DEATH:
                    Talk(IsHeroic()? SAY_OMNOTRON_DEATH_2: SAY_OMNOTRON_DEATH_1);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_BLAZING_INFERNO:
                    if (Creature* pMagmaw = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW)))
                    {
                        Unit* pTarget;
                        pTarget = pMagmaw->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, -20.0f);
                        if (!pTarget)
                            pTarget = pMagmaw->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0);
                        if (!pTarget)
                            return;
                        DoCast(pTarget, SPELL_BLAZING_INFERNO_M);
                        Talk(SAY_MAGMAW_SUMMON);
                    }
                    events.RescheduleEvent(EVENT_BLAZING_INFERNO, urand(30000, 35000));
                    break;
                case EVENT_SHADOW_BREATH:
                    if (Creature* pMagmaw = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW)))
                    {
                        Unit* pTarget;
                        pTarget = pMagmaw->AI()->SelectTarget(SELECT_TARGET_RANDOM, 1, -20.0f);
                        if (!pTarget)
                            pTarget = pMagmaw->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0);
                        if (!pTarget)
                            return;
                        DoCast(pTarget, SPELL_SHADOW_BREATH);
                    }
                    events.RescheduleEvent(EVENT_SHADOW_BREATH, 4000);
                    break;
                case EVENT_MAGMAW_INTRO:
                    Talk(SAY_MAGMAW_INTRO);
                    break;
                case EVENT_MAGMAW_DEATH:
                    Talk(IsHeroic()? SAY_MAGMAW_DEATH_2: SAY_MAGMAW_DEATH_1);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_MALORIAK_INTRO:
                    Talk(SAY_MALORIAK_INTRO);
                    break;
                case EVENT_MALORIAK_DEATH:
                    Talk(IsHeroic()? SAY_MALORIAK_DEATH_2: SAY_MALORIAK_DEATH_1);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_MALORIAK_DARK_MAGIC:
                    if (GameObject* pGo = me->FindNearestGameObject(203306, 300.0f))
                        me->CastSpell(pGo, 92831, false);
                    Talk(SAY_MALORIAK_DARK);
                    break;
                case EVENT_CHIMAERON_INTRO:
                    Talk(SAY_CHIMAERON_INTRO);
                    break;
                case EVENT_CHIMAERON_DEATH:
                    Talk(IsHeroic()? SAY_CHIMAERON_DEATH_2: SAY_CHIMAERON_DEATH_1);
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_CHIMAERON_FEUD:
                    Talk(SAY_CHIMAERON_FEUD);
                    if (Creature* pChimaeron = me->FindNearestCreature(NPC_CHIMAERON, 300.0f))
                        DoCast(pChimaeron, SPELL_SHADOW_WHIP);
                    break;
                case EVENT_CHIMAERON_LOW:
                    Talk(SAY_CHIMAERON_LOW);
                    DoCast(me, SPELL_MOCKING_SHADOWS);
                    break;
                }
            }
        }
    };
};

class npc_blazing_bone_construct : public CreatureScript
{
public:
    npc_blazing_bone_construct() : CreatureScript("npc_blazing_bone_construct") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_blazing_bone_constructAI(pCreature);
    }

    struct npc_blazing_bone_constructAI : public ScriptedAI
    {
        npc_blazing_bone_constructAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        EventMap events;
        bool bArmageddon;


        void Reset()
        {
            events.Reset();
            bArmageddon = false;
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon();
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_FIERY_SLASH, urand(8000, 12000));
        }

        void UpdateAI(uint32 diff)
        {
            if (instance && instance->GetBossState(DATA_MAGMAW) != IN_PROGRESS)
                me->DespawnOrUnsummon();

            if (!UpdateVictim())
                return;

            if (me->HealthBelowPct(20) && !bArmageddon)
            {
                bArmageddon = true;
                DoCast(me, SPELL_ARMAGEDDON);
                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FIERY_SLASH:
                    DoCast(me->getVictim(), SPELL_FIERY_SLASH);
                    events.RescheduleEvent(EVENT_FIERY_SLASH, urand(8000, 12000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class spell_lord_victor_nefarius_shadow_conductor : public SpellScriptLoader
{
public:
    spell_lord_victor_nefarius_shadow_conductor() : SpellScriptLoader("spell_lord_victor_nefarius_shadow_conductor") { }


    class spell_lord_victor_nefarius_shadow_conductor_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lord_victor_nefarius_shadow_conductor_SpellScript);


        void HandleScript()
        {
            PreventHitDamage();

            if (!GetHitUnit())
                return;

            float distance;
            uint32 damage;
            distance = GetCaster()->GetDistance2d(GetHitUnit());
            damage = (int)(5000*(distance/25));
            if (damage < 5000)
                damage = 5000;
            SetHitDamage(damage);
        }

        void Register()
        {
            BeforeHit += SpellHitFn(spell_lord_victor_nefarius_shadow_conductor_SpellScript::HandleScript/*, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE*/);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lord_victor_nefarius_shadow_conductor_SpellScript();
    }
};

class spell_lord_victor_nefarius_encasing_shadows : public SpellScriptLoader
{
public:
    spell_lord_victor_nefarius_encasing_shadows() : SpellScriptLoader("spell_lord_victor_nefarius_encasing_shadows") { }


    class spell_lord_victor_nefarius_encasing_shadows_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lord_victor_nefarius_encasing_shadows_SpellScript);


        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster() || !GetHitUnit())
                return;

            if (!GetHitUnit()->HasAura(SPELL_LIGHTNING_CONDUCTOR))
                PreventHitEffect(EFFECT_0);

            GetHitUnit()->RemoveAurasDueToSpell(SPELL_LIGHTNING_CONDUCTOR);
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_SHADOW_CONDUCTOR, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_lord_victor_nefarius_encasing_shadows_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lord_victor_nefarius_encasing_shadows_SpellScript();
    }
};

class spell_lord_victor_nefarius_grip_of_death : public SpellScriptLoader
{
public:
    spell_lord_victor_nefarius_grip_of_death() : SpellScriptLoader("spell_lord_victor_nefarius_grip_of_death") { }


    class spell_lord_victor_nefarius_grip_of_death_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lord_victor_nefarius_grip_of_death_SpellScript);


        void HandleScript(SpellEffIndex effIndex)
        {
            PreventHitEffect(EFFECT_0);

            if (!GetCaster() || !GetHitUnit())
                return;

            float speedZ = float(GetSpellInfo()->Effects[effIndex]->CalcValue() / 10);
            float speedXY = float(GetSpellInfo()->Effects[effIndex]->MiscValue / 10);

            GetHitUnit()->GetMotionMaster()->MoveJump(
                GetCaster()->GetPositionX(),
                GetCaster()->GetPositionY(),
                GetCaster()->GetPositionZ(),
                speedXY, speedZ);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_lord_victor_nefarius_grip_of_death_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_PULL_TOWARDS_DEST);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lord_victor_nefarius_grip_of_death_SpellScript();
    }
};

class spell_lord_victor_nefarius_overcharge : public SpellScriptLoader
{
public:
    spell_lord_victor_nefarius_overcharge() : SpellScriptLoader("spell_lord_victor_nefarius_overcharge") { }


    class spell_lord_victor_nefarius_overcharge_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lord_victor_nefarius_overcharge_SpellScript);


        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (!GetHitUnit())
                return;

            GetHitUnit()->RemoveAllAuras();
            GetHitUnit()->CastSpell(GetHitUnit(), SPELL_OVERCHARGED_POWER_GENERATOR, true); 
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_lord_victor_nefarius_overcharge_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lord_victor_nefarius_overcharge_SpellScript();
    }
};

void AddSC_blackwing_descent()
{
    new npc_golem_sentry();
    new npc_laser_strike();
    new npc_flash_bomb();
    new npc_drakonid_drudge();
    new npc_drakonid_chainwielder();
    new npc_maimgor();
    new npc_ivoroc();
    new npc_pyreclaw();
    new npc_drakonid_slayer();
    new npc_drakeadon_mongrel();
    new npc_bd_spirit_of_dwarf();
    new spell_drakonid_chainwielder_grievous_wound();
    new npc_lord_victor_nefarius_heroic();
    new npc_blazing_bone_construct();
    new spell_lord_victor_nefarius_overcharge();
    new spell_lord_victor_nefarius_encasing_shadows();
    new spell_lord_victor_nefarius_shadow_conductor();
    new spell_lord_victor_nefarius_grip_of_death();
}