#include "throne_of_the_four_winds.h"

enum Texts
{
    SAY_AGGRO                       = 1, // Your challenge is accepted, mortals! Know that you face Al'Akir, Elemental Lord of Air! You have no hope of defeating me!
    SAY_WIND_BURST                  = 2, // Winds! Obey my command!
    SAY_PHASE_2                     = 3, // Your futile persistence angers me.
    SAY_SQUALL_LINE                 = 4, // Storms! I summon you to my side!
    SAY_PHASE_3                     = 5, // ENOUGH! I WILL NO LONGER BE CONTAINED!
    SAY_KILL                        = 6, // Like swatting insects...
    SAY_KILL_2                      = 7, // This little one will vex me no longer.
    SAY_BERSERK                     = 8, // Enough! I tire of these games!
    SAY_DEFEAT                      = 9, // After every storm... comes the calm...
};

enum Spells
{
    // Al'Akir
    SPELL_WIND_BURST_N_10           = 87770, // PHASE ONE
    SPELL_WIND_BURST_H_10           = 93261,
    SPELL_WIND_BURST_N_25           = 93262,
    SPELL_WIND_BURST_H_25           = 93263,
    SPELL_LIGHTNING_STRIKE_N_10     = 88214,
    SPELL_LIGHTNING_STRIKE_H_10     = 93255,
    SPELL_LIGHTNING_STRIKE_N_25     = 93256,
    SPELL_LIGHTNING_STRIKE_H_25     = 93257,
    SPELL_ICE_STORM_SUMMON          = 88239,

    SPELL_ACID_RAIN_N_10            = 88301, // PHASE TWO
    SPELL_ACID_RAIN_H_10            = 93279,
    SPELL_ACID_RAIN_N_25            = 93280,
    SPELL_ACID_RAIN_H_25            = 93281,

    SPELL_RENTLESS_STORM            = 88866, // PHASE THREE
    SPELL_EYE_OFTHE_STORM           = 82724,        
    SPELL_LIGHTING_ROD_N_10         = 89667,
    SPELL_LIGHTING_ROD_H_10         = 93293,
    SPELL_LIGHTING_ROD_N_25         = 93294,
    SPELL_LIGHTING_ROD_H_25         = 93295,
    SPELL_WIND_BURST2_N_10          = 88858, 
    SPELL_WIND_BURST2_H_10          = 93286,
    SPELL_WIND_BURST2_N_25          = 93287,
    SPELL_WIND_BURST2_H_25          = 93288,
    SPELL_LIGHTNING_N_10            = 89641,
    SPELL_LIGHTNING_H_10            = 93290,
    SPELL_LIGHTNING_N_25            = 93291,
    SPELL_LIGHTNING_H_25            = 93292,

    SPELL_ELECTROCUE                = 88427, // PHASE ONE & TWO
    SPELL_STATIC_SHOCK              = 87873,

    SPELL_BLAST_OF_AIR              = 55912, // COMBI WITH WIND BURST
    SPELL_RELENTLESS_STORM          = 88875, // AURA ON PHASE THREE
    SPELL_BERSERK                   = 95211, // BERSERK I HOPE ITS THE RIGHT BERSERK

    // Icestorm npc
    SPELL_ICE_STORM_N_10            = 91020,
    SPELL_ICE_STORM_H_10            = 93258,
    SPELL_ICE_STORM_N_25            = 93259,
    SPELL_ICE_STORM_H_25            = 93260,
    SPELL_ICE_STORM_AURA_SPAWN      = 87472,
    SPELL_ICE_STORM_AURA2           = 87469,

    // Squall line
    SPELL_SQUALL_LINE_AURA          = 87621,
    SPELL_SQUALL_LINE_DISMOUNT      = 95757,
    SPELL_SQUALL_LINE_DAMAGE_N_10   = 87856,
    SPELL_SQUALL_LINE_DAMAGE_H_10   = 93283,
    SPELL_SQUALL_LINE_DAMAGE_N_25   = 93284,
    SPELL_SQUALL_LINE_DAMAGE_H_25   = 87855,

    // Clouds
    SPELL_LIGHTING_CLOUD_N_10       = 89588,
    SPELL_LIGHTING_CLOUD_H_10       = 93297,
    SPELL_LIGHTING_CLOUD_N_25       = 93298,
    SPELL_LIGHTING_CLOUD_H_25       = 93299,

    // Stormling
    SPELL_FEEDBACK                  = 87904,
    SPELL_STORMLING_H               = 87908,    // only in herioc

};

Position const StormlingListPositions[5] =
{
    {0.0f,          0.0f,           0.0f,       0.0f},
    {0.0f,          0.0f,           0.0f,       0.0f},
    {0.0f,          0.0f,           0.0f,       0.0f},
    {0.0f,          0.0f,           0.0f,       0.0f},
    {0.0f,          0.0f,           0.0f,       0.0f}
};

Position const StormlineswListPositions[7] =
{
    {-25.299749f,   875.881104f,    189.983994f,    0.0f},
    {0.0f,          0.0f,           189.983994f,    0.0f},
    {0.0f,          0.0f,           189.983994f,    0.0f},
    {0.0f,          0.0f,           189.983994f,    0.0f},
    {0.0f,          0.0f,           189.983994f,    0.0f},
    {0.0f,          0.0f,           189.983994f,    0.0f},
    {-38.400715f,   844.311096f,    189.983994f,    0.0f}
};

enum Events
{
    EVENT_PHASE_THREE                = 0,
    EVENT_BERSERK                   = 1,
    EVENT_ICE_STORM                    = 2,
    EVENT_STATIC_SHOCK                = 3,
    EVENT_WIND_BURST                = 4,
    EVENT_LIGHTNING_STRIKE            = 5,
    EVENT_ELECTROCUE                = 6,
    EVENT_SQUALL_LINE                = 7,
    EVENT_ACID_RAIN                    = 8,
    EVENT_STATIC_SHOCK2                = 9,
    EVENT_ELECTROCUE2                = 10,
    EVENT_LIGHTING_ROD                = 11,
    EVENT_SUMMON_STORMLING            = 12,
    EVENT_EYE_OF_THE_STORM            = 13,
    EVENT_RENTLESS_STORM            = 14,
    EVENT_WIND_BURST2                = 15,
    EVENT_LIGHTNING_CLOUD            = 16,
    EVENT_SQUALL_LINE2                = 17,
    EVENT_LIGHTING                    = 18,
};

enum AlakirEvents
{
    DATA_PHASE,
};

enum Phases
{
    PHASE_NONE                      = 0,
    PHASE_ONE                       = 1,
    PHASE_TWO                       = 2,
    PHASE_THREE                     = 3,
};

enum MiscData
{
    NORMAL                            = 2768,
    DARK                            = 2810,
};

enum Achievements
{
    ACHIEVEMENT_ALAKIR_DEAD         = 5123,
    ACHIEVEMENT_FOUR_PLAY           = 5305,
    ACHIEVEMENT_KILLED_ALAKIR       = 5463,
};

class boss_alakir : public CreatureScript
{
public:
    boss_alakir() : CreatureScript("boss_alakir") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_alakirAI(pCreature);
    }

    struct boss_alakirAI : public BossAI
    {
        boss_alakirAI(Creature* pCreature) : BossAI(pCreature, DATA_ALAKIR)
        {
            instance = pCreature->GetInstanceScript();
            _phase = PHASE_NONE;
            me->ApplySpellImmune(0, IMMUNITY_EFFECT,    SPELL_EFFECT_KNOCK_BACK,    true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC,  MECHANIC_GRIP,              true);
        }

        void Reset()
        {
            _Reset();

            instance->DoSendNotifyToInstance("INSTANCE MESSAGE: Al'Akir _Reset");

            SetPhase(PHASE_ONE, true);

            if (GameObject* Go = GetClosestGameObjectWithEntry(me, GO_FLOOR_ALAKIR, 1000.0f))
                Go->SetDestructibleState(GO_DESTRUCTIBLE_REBUILDING);

            Map* pMap = me->GetMap();
            Map::PlayerList const &PlayerList = pMap->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)                            
            {
                Player* plr = i->getSource();
                if (plr->isAlive())
                {
                    plr->RemoveAurasDueToSpell(SPELL_EYE_OFTHE_STORM);
                }
            }
            me->GetMap()->SetZoneWeather(me->GetZoneId(), WEATHER_STATE_FINE, 0.5f);
        }

        void KilledUnit(Unit* /*who*/)
        {
            Talk (RAND(SAY_KILL, SAY_KILL_2));
        }

        uint32 GetData(uint32 data) const
        {
            if (data == DATA_PHASE)
                return _phase;

            return 0;
        }

        void SetPhase(uint8 phase, bool setEvents = false)
        {
            events.Reset();

            events.SetPhase(phase);
            _phase = phase;

            if (setEvents)
                SetPhaseEvents();
        }

        void SetPhaseEvents()
        {
            switch (_phase)
            {
            case PHASE_ONE:
                events.ScheduleEvent(EVENT_STATIC_SHOCK,        12000,  0, _phase);
                events.ScheduleEvent(EVENT_ELECTROCUE,          20000,  0, _phase);
                events.ScheduleEvent(EVENT_WIND_BURST,          3000,   0, _phase);
                events.ScheduleEvent(EVENT_LIGHTNING_STRIKE,    18000,  0, _phase);
                break;

            case PHASE_TWO:
                events.ScheduleEvent(EVENT_STATIC_SHOCK2,       8000,   0, _phase);
                events.ScheduleEvent(EVENT_ACID_RAIN,           2000,   0, _phase);
                events.ScheduleEvent(EVENT_ELECTROCUE2,         5000,   0, _phase);
                break;

            case PHASE_THREE:
                events.ScheduleEvent(EVENT_WIND_BURST2,         25000,  0, _phase);
                events.ScheduleEvent(EVENT_LIGHTING_ROD,        20000,  0, _phase);
                events.ScheduleEvent(EVENT_LIGHTING,            25000,  0, _phase);
                break;
            default:
                break;
            }
        }

        void StartPhaseTwo()
        {
            SetPhase(PHASE_TWO, true);
            me->GetMap()->SetZoneOverrideLight(me->GetZoneId(), NORMAL, 5 * IN_MILLISECONDS);
            me->GetMap()->SetZoneWeather(me->GetZoneId(), WEATHER_STATE_MEDIUM_RAIN, 0.5f);
        }

        void StartPhaseThree()
        {
            SetPhase(PHASE_THREE, true);

            GameObject* floor = me->FindNearestGameObject(GO_FLOOR_ALAKIR, 200);
            if (floor)
                floor->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);    

            me->AddAura(SPELL_RELENTLESS_STORM, me);    

            me->GetMap()->SetZoneWeather(me->GetZoneId(), WEATHER_STATE_FINE, 0.5f);
            me->GetMap()->SetZoneOverrideLight(me->GetZoneId(), DARK, 5 * IN_MILLISECONDS);

            Map* pMap = me->GetMap();
            Map::PlayerList const &PlayerList = pMap->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)                            
            {
                Player* plr = i->getSource();
                if (plr->isAlive())
                {
                    plr->CastSpell(plr, SPELL_EYE_OFTHE_STORM, false);
                }
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_AGGRO);
            _EnterCombat();
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            events.SetPhase(PHASE_ONE);
            events.ScheduleEvent(EVENT_BERSERK, 10 * MINUTE * IN_MILLISECONDS);
        }

        void JustDied(Unit* killer)
        {
            _JustDied();
            me->SummonGameObject(GO_HEART_OF_THE_WIND, 25.359699f, 777.276733f, 200.264008f, 0, 0, 0, 0, 0, 100000);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPctDamaged(80, damage) && (_phase == PHASE_ONE))
                StartPhaseTwo();

            if (me->HealthBelowPctDamaged(25, damage) && (_phase == PHASE_TWO))
                StartPhaseThree();
        }

        void UpdateAI(uint32 diff)
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
                /*case EVENT_BERSERK:
                    DoCast(me, SPELL_BERSERK);
                    break;*/
                case EVENT_WIND_BURST:
                    Talk(SAY_WIND_BURST);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    {
                        DoCast(pTarget, SPELL_BLAST_OF_AIR);
                        DoCast(pTarget, RAID_MODE(SPELL_WIND_BURST_N_10, SPELL_WIND_BURST_H_10, SPELL_WIND_BURST_N_25, SPELL_WIND_BURST_H_25));
                    }
                    events.ScheduleEvent(EVENT_WIND_BURST, 25000, 0, PHASE_ONE);
                    break;

                case EVENT_LIGHTNING_STRIKE:
                    DoCastVictim(RAID_MODE(SPELL_LIGHTNING_STRIKE_N_10, SPELL_LIGHTNING_STRIKE_H_10, SPELL_LIGHTNING_STRIKE_N_25, SPELL_LIGHTNING_STRIKE_H_25));
                    events.ScheduleEvent(EVENT_LIGHTNING_STRIKE, 20000, 0, PHASE_ONE);
                    break;

                case EVENT_ELECTROCUE:
                    if (Unit* pTarget = me->getVictim())
                        if (me->IsWithinDistInMap(pTarget, 5.0f))
                            DoCast(me->getVictim(), SPELL_ELECTROCUE);
                    events.ScheduleEvent(EVENT_ELECTROCUE, 17000, 0, PHASE_ONE);
                    break;

                case EVENT_STATIC_SHOCK:
                    DoCast(me->getVictim(), SPELL_STATIC_SHOCK);
                    events.ScheduleEvent(EVENT_STATIC_SHOCK, 17000, 0, PHASE_ONE);
                    break;

                case EVENT_STATIC_SHOCK2:
                    DoCast(me->getVictim(), SPELL_STATIC_SHOCK);
                    events.ScheduleEvent(EVENT_STATIC_SHOCK2, 17000, 0, PHASE_TWO);
                    break;

                case EVENT_ELECTROCUE2:
                    if (Unit* pTarget = me->getVictim())
                        if (me->IsWithinDistInMap(pTarget, 5.0f))
                            DoCast(me->getVictim(), SPELL_ELECTROCUE);
                    events.ScheduleEvent(EVENT_ELECTROCUE2, 17000, 0, PHASE_TWO);
                    break;

                case EVENT_ACID_RAIN:
                    DoCastAOE(RAID_MODE(SPELL_ACID_RAIN_N_10, SPELL_ACID_RAIN_H_10, SPELL_ACID_RAIN_N_25, SPELL_ACID_RAIN_H_25));
                    events.ScheduleEvent(EVENT_ACID_RAIN, 15000, 0, PHASE_TWO);
                    break;

                case EVENT_WIND_BURST2:
                    Talk(SAY_WIND_BURST);
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    {
                        DoCast(pTarget, RAID_MODE(SPELL_WIND_BURST2_N_10, SPELL_WIND_BURST2_H_10, SPELL_WIND_BURST2_N_25, SPELL_WIND_BURST2_H_25));
                    }
                    events.ScheduleEvent(EVENT_WIND_BURST2, 5000, 0, PHASE_THREE);
                    break;

                case EVENT_LIGHTING_ROD:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    {
                        DoCast(pTarget, RAID_MODE(SPELL_LIGHTING_ROD_N_10, SPELL_LIGHTING_ROD_H_10, SPELL_LIGHTING_ROD_N_25, SPELL_LIGHTING_ROD_H_25));
                    }
                    events.ScheduleEvent(EVENT_LIGHTING_ROD, 15000, 0, PHASE_THREE);
                    break;

                case EVENT_LIGHTING:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
                    {
                        DoCast(pTarget, RAID_MODE(SPELL_LIGHTNING_N_10, SPELL_LIGHTNING_H_10, SPELL_LIGHTNING_N_25, SPELL_LIGHTNING_H_25));
                    }
                    events.ScheduleEvent(EVENT_LIGHTING, 20000, 0, PHASE_THREE);
                    break;
                default:
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* instance;
        uint8 _phase;
        uint32 phase;
    };
};

class npc_stormling : public CreatureScript
{
public:
    npc_stormling() : CreatureScript("npc_stormling") { }

    struct npc_stormlingAI : public ScriptedAI
    {
        npc_stormlingAI(Creature* pCreature) : ScriptedAI(pCreature) { }

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_stormlingAI(creature);
        }

        uint32 GravityTimer;

        void Reset()
        {
            GravityTimer = 10000;
        }

        void JustDied(Unit* killer)
        {
            DoCast(SPELL_FEEDBACK);
        }

        void UpdateAI(uint32 diff)
        {    
            if (IsHeroic())
                if (GravityTimer <= diff)
                {
                    DoCast(SPELL_STORMLING_H);
                    GravityTimer = 10000;
                }

                DoMeleeAttackIfReady();
        }
    };
};

class npc_tornado_moving : public CreatureScript
{
public:
    npc_tornado_moving() :  CreatureScript("npc_tornado_moving") { }

    struct npc_tornado_movingAI : public ScriptedAI
    {
        npc_tornado_movingAI(Creature* creature) : ScriptedAI(creature) { }

        bool MoveSide; // true = right, false = left
        float defaultDistX;
        float defaultDistY;

        void IsSummonedBy(Unit* /*summoner*/)
        {
            //me->AddAura(SPELL_TORNADO_VISUAL, me);

            defaultDistX = GetAlakir()->GetPositionX() > me->GetPositionX() ? GetAlakir()->GetPositionX() - me->GetPositionX() : me->GetPositionX() - GetAlakir()->GetPositionX();
            defaultDistY = GetAlakir()->GetPositionY() > me->GetPositionY() ? GetAlakir()->GetPositionY() - me->GetPositionY() : me->GetPositionY() - GetAlakir()->GetPositionY();

            std::list<Creature*> tornados;
            GetCreatureListWithEntryInGrid(tornados, me, me->GetEntry(), 40.0f);

            if (!tornados.empty() && tornados.size() < 4)
                //me->SummonCreature(me->GetEntry(),me->GetPositionX() - (cos(me->GetOrientation())*2),me->GetPositionY() - (sin(me->GetOrientation())*2),me->GetPositionZ());

                    if (me->GetEntry() == 48854)
                        MoveSide = true; // west
                    else
                        MoveSide = false; // east
        }

        void UpdateAI(uint32 diff)
        {
            if (GetAlakir() && GetAlakir()->isAlive())
            {
                float distanceX = GetAlakir()->GetPositionX() > me->GetPositionX() ? GetAlakir()->GetPositionX() - me->GetPositionX() : me->GetPositionX() - GetAlakir()->GetPositionX();
                float distanceY = GetAlakir()->GetPositionY() > me->GetPositionY() ? GetAlakir()->GetPositionY() - me->GetPositionY() : me->GetPositionY() - GetAlakir()->GetPositionY();
                me->GetMotionMaster()->MovePoint(0, (distanceX < defaultDistX) ? me->GetPositionX() + MoveSide ? 1 : - 1 : me->GetPositionX(),(distanceY < defaultDistY) ? me->GetPositionY() + MoveSide ? 1 : - 1 : me->GetPositionY(),me->GetPositionZ());
            }
        }

        Creature* GetAlakir()
        {
            return me->FindNearestCreature(BOSS_ALAKIR, 5000.0f, true);
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tornado_movingAI(creature);
    }
};

void AddSC_boss_alakir()
{
    new boss_alakir();
    new npc_stormling();
    new npc_tornado_moving();
}