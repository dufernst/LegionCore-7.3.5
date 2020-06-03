#include "lost_city_of_the_tolvir.h"
#include "Vehicle.h"

enum eSpells
{
    SPELL_HAMMER_FIST                              = 83655,
    // Mystic Trap
    SPELL_MYSTIC_TRAP_FIND_TARGET                  = 83644,
    SPELL_TOLVIR_MINE_PLAYER_SEARCH_TRIGGER        = 83111,
    SPELL_THROW_LAND_MINES                         = 83122,
    SPELL_TOLVIR_LAND_MINE_VISUAL                  = 83110,
    SPELL_TOLVIR_LAND_MIVE_PERIODIC                = 85523,
    SPELL_MYSTIC_TRAP                              = 83171,
    SPELL_DETONATE_TRAPS                           = 91263,
    // Bad Intentions
    SPELL_BAD_INTENTIONS                           = 83113,
    SPELL_THROW_VISUAL                             = 83371,
    SPELL_HARD_IMPACT                              = 83339,
    SPELL_RIDE_VEHICLE_HARDCODED                   = 46598,
    SPELL_HURL_SCRIPT                              = 83236,
    SPELL_HURL_RIDE                                = 83235,
    // Shockwave
    SPELL_SHOCKWAVE                                = 83445,
    SPELL_SHOCKWAVE_VISUAL                         = 83130,
    SPELL_SHOCKWAVE_STALKER_VISUAL                 = 83127,
    SPELL_SHOCKWAVE_DAMAGE                         = 83454,
    SPELL_SUMMON_SHOCKWAVE                         = 83128
};

enum eCreatures
{
    NPC_MYSTIC_TRAP_TARGET                         = 44840,
    NPC_BAD_INTENTIONS_TARGET                      = 44586
};

enum eEvents
{
    EVENT_SUMMON_LAND_MINES                        = 1,
    EVENT_COUNTDOWN_LAND_MINES                     = 2,
    EVENT_SUMMON_SHOCKWAVE                         = 3,
    EVENT_HAMMER_FIST                              = 4,
    EVENT_BAD_INTENTIONS                           = 5,
    EVENT_AFTER_SHOCKWAVE                          = 6
};

enum ePhases
{
    LAND_MINE_STATE_JUSTADDED                      = 0x1,
    LAND_MINE_STATE_ACTIVATED                      = 0x2,
    LAND_MINE_STATE_COUNTDOWN                      = 0x4,
    LAND_MINE_STATE_DETONATED                      = 0x8
};

enum Texts
{
    SAY_START                                      = 0,
    SAY_CAST_SHOCKVAWE                             = 1,
    SAY_DEATH                                      = 2,
    SAY_KILL                                       = 3
};

struct boss_general_husam : public ScriptedAI
{
    explicit boss_general_husam(Creature* creature) : ScriptedAI(creature), summons(me)
    {
        instance = creature->GetInstanceScript();
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
    }

    SummonList summons;
    InstanceScript* instance;
    uint8 uiHammerFistCount;
    EventMap events;

    void Reset() override
    {
        if (Vehicle* vehicle = me->GetVehicleKit())
            vehicle->RemoveAllPassengers();

        if (instance)
            instance->SetData(DATA_GENERAL_HUSAM, NOT_STARTED);

        summons.DespawnAll();
        events.Reset();
        uiHammerFistCount = 0;
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!who)
            return;

        if (apply && who->IsPlayer())
            me->CastSpell(who, SPELL_HURL_SCRIPT, false);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance)
            instance->SetData(DATA_GENERAL_HUSAM, IN_PROGRESS);

        Talk(SAY_START);
        events.ScheduleEvent(EVENT_SUMMON_LAND_MINES, 3000);
        events.ScheduleEvent(EVENT_HAMMER_FIST, 5000);
        events.ScheduleEvent(EVENT_SUMMON_SHOCKWAVE, urand(12000, 17000));
        // To do: fix client crash
        //events.ScheduleEvent(EVENT_BAD_INTENTIONS, urand(7000, 11000));

        if (IsHeroic())
            events.ScheduleEvent(EVENT_COUNTDOWN_LAND_MINES, 15000);
    }

    void JustSummoned(Creature* summoned) override
    {
        summons.Summon(summoned);

        if (summoned->GetEntry() == 44798)
        {
            float x, y, z;
            summoned->GetPosition(x, y, z);
            summoned->SetDisplayId(27823);

            if (auto mine = me->SummonCreature(44796, x, y, z))
            {
                mine->EnterVehicle(summoned);
                mine->ClearUnitState(UNIT_STATE_ONVEHICLE);
                mine->SetDisplayId(34086);
            }
        }

        switch (summoned->GetEntry())
        {
        case NPC_MYSTIC_TRAP_TARGET:
        case 44712:
        case 44586:
        case 44711:
            summoned->SetDisplayId(27823);
            break;
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim)
            return;

        if (victim->IsPlayer())
            Talk(SAY_KILL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (instance)
            instance->SetData(DATA_GENERAL_HUSAM, DONE);

        Talk(SAY_DEATH);
        summons.DespawnAll();
        events.Reset();
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
            case EVENT_COUNTDOWN_LAND_MINES:
                Talk(SAY_CAST_SHOCKVAWE);
                events.ScheduleEvent(EVENT_COUNTDOWN_LAND_MINES, 15000);
                DoCast(SPELL_DETONATE_TRAPS);
                break;
            case EVENT_SUMMON_LAND_MINES:
            {
                DoCast(SPELL_MYSTIC_TRAP_FIND_TARGET);
                std::list<Creature*> triggers;
                me->GetCreatureListWithEntryInGrid(triggers, NPC_MYSTIC_TRAP_TARGET, 50.0f);

                if (!triggers.empty())
                    for (auto& cre : triggers)
                        DoCast(cre, SPELL_THROW_LAND_MINES, false);

                events.ScheduleEvent(EVENT_SUMMON_LAND_MINES, 21000);
                break;
            }
            case EVENT_SUMMON_SHOCKWAVE:
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                Talk(SAY_CAST_SHOCKVAWE);

                float _x, _y, x, y, z, o;
                me->GetPosition(x, y, z, o);

                for (int i = 0; i < 4; ++i)
                {
                    _x = x + 3.0f * cos(o);
                    _y = y + 3.0f * sin(o);
                    me->SummonCreature(44711, _x, _y, z, o);
                    o += M_PI / 2;
                }

                DoCast(SPELL_SHOCKWAVE);
                events.ScheduleEvent(EVENT_AFTER_SHOCKWAVE, 5500);
                break;
            case EVENT_AFTER_SHOCKWAVE:
                me->SetReactState(REACT_AGGRESSIVE);
                events.ScheduleEvent(EVENT_SUMMON_SHOCKWAVE, urand(25000, 50000));
                break;
            case EVENT_HAMMER_FIST:
                ++uiHammerFistCount;

                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_HAMMER_FIST, false);

                if (uiHammerFistCount < 4)
                    events.ScheduleEvent(EVENT_HAMMER_FIST, 500);
                else
                {
                    uiHammerFistCount = 0;
                    events.ScheduleEvent(EVENT_HAMMER_FIST, urand(5000, 15000));
                }
                break;
            case EVENT_BAD_INTENTIONS:
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                    DoCast(target, SPELL_BAD_INTENTIONS, false);

                events.ScheduleEvent(EVENT_BAD_INTENTIONS, urand(15000, 20000));
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//44796
struct npc_land_mine : public ScriptedAI
{
    explicit npc_land_mine(Creature* creature) : ScriptedAI(creature)
    {
        uiState = LAND_MINE_STATE_JUSTADDED;
        uiDespawnTimer = 1000;
        uiActivationTimer = 2000;
        uiCountdownTimer = urand(20000, 35000);
        me->SetInCombatWithZone();
    }

    uint32 uiActivationTimer;
    uint32 uiCountdownTimer;
    uint32 uiDespawnTimer;
    uint8 uiState;

    void AddMineState(uint32 state)
    {
        uiState |= state;
    }

    bool HasMineState(const uint32 state) const
    {
        return (uiState & state);
    }

    void ClearMineState(uint32 state)
    {
        uiState &= ~state;
    }

    void StartCountDown()
    {
        if (HasMineState(LAND_MINE_STATE_DETONATED | LAND_MINE_STATE_COUNTDOWN) || !HasMineState(LAND_MINE_STATE_ACTIVATED))
            return;

        AddMineState(LAND_MINE_STATE_DETONATED);
        AddMineState(LAND_MINE_STATE_COUNTDOWN);
        me->AddAura(SPELL_TOLVIR_LAND_MIVE_PERIODIC, me);
        uiDespawnTimer = 5500;
    }

    void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_DETONATE_TRAPS)
            StartCountDown();
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == 83112)
        {
            float x, y, _x, _y;
            me->GetPosition(x, y);
            target->GetPosition(_x, _y);

            if (sqrt(pow(x - _x, 2) + pow(y - _y, 2)) < 1.4f)
            {
                if (!HasMineState(LAND_MINE_STATE_ACTIVATED))
                    return;

                ClearMineState(LAND_MINE_STATE_ACTIVATED);
                AddMineState(LAND_MINE_STATE_DETONATED);
                uiDespawnTimer = 1000;
                DoCast(target, SPELL_MYSTIC_TRAP, true);
                me->RemoveAura(SPELL_TOLVIR_LAND_MINE_VISUAL);
                me->RemoveAura(SPELL_TOLVIR_MINE_PLAYER_SEARCH_TRIGGER);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (HasMineState(LAND_MINE_STATE_JUSTADDED))
        {
            if (uiActivationTimer <= diff)
            {
                ClearMineState(LAND_MINE_STATE_JUSTADDED);
                AddMineState(LAND_MINE_STATE_ACTIVATED);
                me->AddAura(SPELL_TOLVIR_LAND_MINE_VISUAL, me);
                me->AddAura(SPELL_TOLVIR_MINE_PLAYER_SEARCH_TRIGGER, me);
            }
            else
                uiActivationTimer -= diff;
        }

        if (HasMineState(LAND_MINE_STATE_ACTIVATED) && !HasMineState(LAND_MINE_STATE_COUNTDOWN))
        {
            if (uiCountdownTimer <= diff)
                StartCountDown();
            else
                uiCountdownTimer -= diff;
        }

        if (HasMineState(LAND_MINE_STATE_DETONATED))
        {
            if (uiDespawnTimer <= diff)
            {
                if (auto pMineVehicle = me->GetVehicleCreatureBase())
                    pMineVehicle->DespawnOrUnsummon();

                me->DespawnOrUnsummon();
            }
            else
                uiDespawnTimer -= diff;
        }
    }
};

struct npc_shockwave_stalker : public ScriptedAI
{
    explicit npc_shockwave_stalker(Creature* creature) : ScriptedAI(creature)
    {
        summonedGUID.clear();
        CanCheck = true;
        uiCheckTimer = 250;
        instance = creature->GetInstanceScript();

        if (me->isSummon())
            if (Unit* summoner = me->ToTempSummon()->GetSummoner())
            {
                float x, y, _x, _y, z = me->GetPositionZ();
                summoner->GetPosition(_x, _y);
                pos.Relocate(_x, _y, z);
                me->GetNearPoint2D(x, y, 40.0f, M_PI - me->GetAngle(_x, _y));
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }
    }

    InstanceScript* instance;
    GuidList summonedGUID;
    Position pos;
    uint32 uiCheckTimer;
    bool CanCheck;

    void JustSummoned(Creature* summoned) override
    {
        summoned->SetReactState(REACT_PASSIVE);
        summoned->SetInCombatWithZone();
        summoned->AddAura(SPELL_SHOCKWAVE_STALKER_VISUAL, summoned);

        if (instance)
            if (auto husam = Unit::GetCreature(*me, instance->GetGuidData(DATA_GENERAL_HUSAM)))
                husam->AI()->JustSummoned(summoned);

        if (ObjectGuid uiGUID = summoned->GetGUID())
            summonedGUID.push_back(uiGUID);
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_SHOCKWAVE || spell->Id == 91257)
        {
            caster->CastSpell(me, SPELL_SHOCKWAVE_VISUAL, true);
            me->RemoveAllAuras();
            CanCheck = false;

            if (summonedGUID.empty())
                return;

            for (GuidList::const_iterator itr = summonedGUID.begin(); itr != summonedGUID.end(); ++itr)
            {
                if (auto shockwave = Unit::GetCreature(*me, (*itr)))
                {
                    shockwave->CastSpell(shockwave, SPELL_SHOCKWAVE_DAMAGE, true);
                    shockwave->RemoveAllAuras();
                }
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (CanCheck)
        {
            if (uiCheckTimer <= diff)
            {
                uiCheckTimer = 250;
                float x, y;
                me->GetPosition(x, y);

                if (pos.GetExactDist2d(x, y) >= 4.0f)
                {
                    float dist = me->GetDistance(pos);
                    float _x, _y, _z = me->GetPositionZ();
                    me->GetNearPoint2D(_x, _y, dist - 4.0f, me->GetAngle(&pos));
                    me->CastSpell(_x, _y, _z, SPELL_SUMMON_SHOCKWAVE, true);
                }
            }
            else
                uiCheckTimer -= diff;
        }
    }
};

struct npc_bad_intentios_target : public ScriptedAI
{
    explicit npc_bad_intentios_target(Creature* creature) : ScriptedAI(creature) {}

    uint32 uiExitTimer;
    bool Passenger;

    void Reset()
    {
        uiExitTimer = 1000;
        Passenger = false;

        if (Vehicle* vehicle = me->GetVehicleKit())
            vehicle->RemoveAllPassengers();
    }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        if (!who)
            return;

        if (apply)
        {
            DoCast(who, SPELL_HARD_IMPACT, false);
            Passenger = true;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (Passenger)
        {
            if (uiExitTimer <= diff)
                Reset();
            else
                uiExitTimer -= diff;
        }
    }
};

class spell_bad_intentions : public SpellScript
{
    PrepareSpellScript(spell_bad_intentions)

    void HandleScript(SpellEffIndex /*effect*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        if (!(caster && target))
            return;

        target->CastSpell(caster, SPELL_RIDE_VEHICLE_HARDCODED, false);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_bad_intentions::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_hurl : public SpellScript
{
    PrepareSpellScript(spell_hurl)

    void HandleScript(SpellEffIndex /*effect*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        if (!(caster && target))
            return;

        target->ExitVehicle();
        caster->CastSpell(caster, SPELL_THROW_VISUAL, false);
        target->CastSpell(target, SPELL_HURL_RIDE, false);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hurl::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

void AddSC_boss_general_husam()
{
    RegisterCreatureAI(boss_general_husam);
    RegisterCreatureAI(npc_land_mine);
    RegisterCreatureAI(npc_shockwave_stalker);
    RegisterCreatureAI(npc_bad_intentios_target);
    RegisterSpellScript(spell_bad_intentions);
    RegisterSpellScript(spell_hurl);
}