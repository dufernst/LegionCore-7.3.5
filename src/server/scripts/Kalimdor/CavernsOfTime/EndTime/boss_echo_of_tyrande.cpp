#include "end_time.h"

enum Yells
{
    SAY_AGGRO       = 0,
    SAY_80          = 1,
    SAY_80_EMOTE    = 2,
    SAY_55          = 3,
    SAY_30          = 4,
    SAY_30_EMOTE    = 5,
    SAY_DEATH       = 6,
    SAY_INTRO_1     = 7,
    SAY_INTRO_2     = 8,
    SAY_INTRO_3     = 9,
    SAY_INTRO_4     = 10,
    SAY_INTRO_5     = 11,
    SAY_KILL        = 12,
    SAY_EYES        = 13,
    SAY_MOONLANCE   = 14,
    SAY_LIGHT_1     = 15,
    SAY_LIGHT_2     = 16,
    SAY_LIGHT_3     = 17,
    SAY_LIGHT_4     = 18,
    SAY_LIGHT_5     = 19,
    SAY_LIGHT_6     = 20,
    SAY_LIGHT_LEFT  = 21
};

enum Spells
{
    SPELL_MOONBOLT                      = 102193,
    SPELL_DARK_MOONLIGHT                = 102414,
    SPELL_MOONLIGHT_COSMETIC            = 108642,
    SPELL_STARDUST                      = 102173,
    SPELL_MOONLANCE_AURA                = 102150,
    SPELL_MOONLANCE_DMG                 = 102149,
    SPELL_MOONLANCE_SUMMON_1            = 102151,
    SPELL_MOONLANCE_SUMMON_2            = 102152,
    SPELL_LUNAR_GUIDANCE                = 102472,
    SPELL_TEARS_OF_ELUNE                = 102241,
    SPELL_TEARS_OF_ELUNE_SCRIPT         = 102242,
    SPELL_TEARS_OF_ELUNE_MISSILE        = 102243,
    SPELL_TEARS_OF_ELUNE_DMG            = 102244,
    SPELL_EYES_OF_GODDESS               = 102181,
    SPELL_EYES_OF_GODDESS_STUN          = 102248,

    SPELL_PIERCING_GAZE_OF_ELUNE_AURA   = 102182,
    SPELL_PIERCING_GAZE_OF_ELUNE_DMG    = 102183,

    // event
    SPELL_IN_SHADOW                     = 101841,
    SPELL_MOONLIGHT                     = 101946,
    SPELL_SHRINK                        = 102002,

    SPELL_ACHIEVEMENT_CHECK             = 102491,
    SPELL_ACHIEVEMENT_FAIL              = 102539,
    SPELL_ACHIEVEMENT                   = 102542
};

enum Events
{
    EVENT_MOONBOLT          = 1,
    EVENT_MOONLANCE         = 2,
    EVENT_STARDUST          = 3,
    EVENT_EYES_OF_GODDESS   = 4,

    EVENT_START_EVENT       = 5,
    EVENT_CHECK_PLAYERS     = 6,
    EVENT_SUMMON_ADDS       = 7,
    EVENT_SUMMON_POOL       = 8,
    EVENT_STOP_EVENT        = 9,
    EVENT_STOP_EVENT_1      = 10
};

enum Adds
{
    NPC_MOONLANCE_1                 = 54574,
    NPC_MOONLANCE_2_1               = 54580,
    NPC_MOONLANCE_2_2               = 54581,
    NPC_MOONLANCE_2_3               = 54582,
    NPC_EYE_OF_ELUNE_1              = 54939,
    NPC_EYE_OF_ELUNE_2              = 54940,
    NPC_EYE_OF_ELUNE_3              = 54941,
    NPC_EYE_OF_ELUNE_4              = 54942,

    // for aura
    NPC_STALKER                     = 45979,

    // event
    NPC_POOL_OF_MOONLIGHT           = 54508,
    NPC_TIME_TWISTED_SENTINEL       = 54512,
    NPC_TIME_TWISTED_HUNTRESS       = 54701,
    NPC_TIME_TWISTED_NIGHTSABER_1   = 54688,
    NPC_TIME_TWISTED_NIGHTSABER_2   = 54699,
    NPC_TIME_TWISTED_NIGHTSABER_3   = 54700
};

enum Other
{
    POINT_MOONLANCE     = 1,
    ACTION_START_EVENT  = 2
};

const Position poolPos[5] = 
{
    { 2903.26f, 63.1788f, 3.2449f, 0.0f },
    { 2862.83f, 131.462f, 3.18436f, 0.0f },
    { 2756.57f, 129.971f, 5.58215f, 0.0f },
    { 2695.44f, 28.7969f, 1.2324f, 0.0f },
    { 2792.82f, 1.93924f, 2.46328f, 0.0f }
};

struct boss_echo_of_tyrande : public BossAI
{
    explicit boss_echo_of_tyrande(Creature* creature) : BossAI(creature, DATA_ECHO_OF_TYRANDE)
    {
        me->setActive(true);
        phase = 0;
        eventphase = 0;
        curPool = 0;
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, 8);
        me->AddAura(SPELL_IN_SHADOW, me);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
    }

    uint8 phase;
    ObjectGuid moonlanceGUID;
    uint8 eventphase;
    Unit* curPool;

    void Reset() override
    {
        _Reset();

        moonlanceGUID.Clear();
        phase = 0;

        if (instance->GetData(DATA_TYRANDE_EVENT) == DONE)
        {
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->RemoveAura(SPELL_IN_SHADOW);

            if (auto stalker = me->SummonCreature(NPC_STALKER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f))
            {
                stalker->RemoveAllAuras();
                stalker->CastSpell(stalker, SPELL_MOONLIGHT_COSMETIC, true);
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);

        // Quest
        instance->instance->ApplyOnEveryPlayer([&](Player* player)
        {
            if (me->GetDistance2d(player) <= 50.0f && player->GetQuestStatus(30097) == QUEST_STATUS_INCOMPLETE)
                DoCast(player, SPELL_ARCHIVED_TYRANDE, true);
        });
    }

    void JustSummoned(Creature* summon) override
    {
        if (summon->GetEntry() != NPC_STALKER)
            BossAI::JustSummoned(summon);

        if (summon->GetEntry() == NPC_MOONLANCE_1)
        {
            if (Player* target = ObjectAccessor::GetPlayer(*me, moonlanceGUID))
            {
                summon->SetOrientation(me->GetAngle(target));
                Position pos;
                summon->GetNearPosition(pos, 15.0f, 0.0f);
                summon->GetMotionMaster()->MovePoint(POINT_MOONLANCE, pos);
            }
            else
                summon->DespawnOrUnsummon();
        }
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->HasEffect(SPELL_EFFECT_INTERRUPT_CAST))
            if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_MOONBOLT)
                    me->InterruptSpell(CURRENT_GENERIC_SPELL);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        Talk(SAY_AGGRO);

        phase = 0;

        DoCast(me, SPELL_DARK_MOONLIGHT, true);

        events.ScheduleEvent(EVENT_MOONBOLT, 1000);
        events.ScheduleEvent(EVENT_MOONLANCE, urand(12000, 13000));
        events.ScheduleEvent(EVENT_STARDUST, urand(7000, 8000));
        events.ScheduleEvent(EVENT_EYES_OF_GODDESS, urand(30000, 33000));

        instance->SetBossState(DATA_ECHO_OF_TYRANDE, IN_PROGRESS);
        DoZoneInCombat();
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        BossAI::SummonedCreatureDespawn(summon);
        if (summon->GetEntry() == NPC_POOL_OF_MOONLIGHT)
            Talk(SAY_LIGHT_LEFT);
    }

    void AttackStart(Unit* who) override
    {
        if (who)
            me->Attack(who, false);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_START_EVENT)
        {
            eventphase = 1;
            summons.DespawnAll();
            events.ScheduleEvent(EVENT_START_EVENT, 10000);
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (phase == 0 && me->HealthBelowPct(80))
        {
            phase = 1;
            Talk(SAY_80);
            DoCast(SPELL_LUNAR_GUIDANCE);
            return;
        }
        else if (phase == 1 && me->HealthBelowPct(55))
        {
            phase = 2;
            Talk(SAY_55);
            DoCast(SPELL_LUNAR_GUIDANCE);
            return;
        }
        else if (phase == 2 && me->HealthBelowPct(30))
        {
            phase = 3;
            Talk(SAY_30);
            DoCast(SPELL_TEARS_OF_ELUNE);
            return;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && !eventphase)
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_START_EVENT:
                events.ScheduleEvent(EVENT_SUMMON_POOL, 2000);
                events.ScheduleEvent(EVENT_SUMMON_ADDS, 5000);
                events.ScheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                break;
            case EVENT_SUMMON_POOL:
                switch (eventphase)
                {
                case 1:
                    Talk(SAY_LIGHT_1);
                    break;
                case 2:
                    Talk(SAY_LIGHT_2);
                    break;
                case 3:
                    Talk(SAY_LIGHT_3);
                    break;
                case 4:
                    Talk(SAY_LIGHT_4);
                    break;
                case 5:
                    Talk(SAY_LIGHT_5);
                    break;
                default:
                    break;
                }
                curPool = 0;
                curPool = me->SummonCreature(NPC_POOL_OF_MOONLIGHT, poolPos[eventphase - 1], TEMPSUMMON_TIMED_DESPAWN, 40000);
                if (eventphase < 5)
                {
                    switch (eventphase)
                    {
                    case 2:
                        Talk(SAY_INTRO_2);
                        break;
                    case 3:
                        Talk(SAY_INTRO_3);
                        break;
                    case 4:
                        Talk(SAY_INTRO_4);
                        break;
                    default:
                        break;
                    }
                    eventphase++;
                    events.ScheduleEvent(EVENT_SUMMON_POOL, 45000);
                }
                else
                {
                    Talk(SAY_INTRO_5);
                    events.ScheduleEvent(EVENT_STOP_EVENT, 30000);
                    events.ScheduleEvent(EVENT_STOP_EVENT_1, 40000);
                }
                break;
            case EVENT_SUMMON_ADDS:
            {
                if (auto player = me->FindNearestPlayer(500.0f, true))
                {
                    Position pos;
                    player->GetRandomNearPosition(pos, frand(15.0f, 20.0f));
                    uint32 entry = NPC_TIME_TWISTED_NIGHTSABER_1;
                    switch (urand(1, eventphase))
                    {
                    case 1:
                        entry = NPC_TIME_TWISTED_NIGHTSABER_1;
                        break;
                    case 2:
                        entry = NPC_TIME_TWISTED_NIGHTSABER_2;
                        break;
                    case 3:
                        entry = NPC_TIME_TWISTED_NIGHTSABER_3;
                        break;
                    case 4:
                        entry = NPC_TIME_TWISTED_SENTINEL;
                        break;
                    case 5:
                        entry = NPC_TIME_TWISTED_HUNTRESS;
                        break;
                    }

                    if (auto creature = me->SummonCreature(entry, pos))
                        creature->AI()->AttackStart(player);
                }

                events.ScheduleEvent(EVENT_SUMMON_ADDS, urand(5000, 10000));
                break;
            }
            case EVENT_CHECK_PLAYERS:
            {
                uint8 num = 0;
                Map::PlayerList const &playerList = instance->instance->GetPlayers();
                if (!playerList.isEmpty())
                    for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                        if (Player* player = itr->getSource())
                            if (player->GetCurrentAreaID() == AREA_EMERALD && player->isAlive())
                                num++;

                if (!num)
                {
                    summons.DespawnAll();
                    events.CancelEvent(EVENT_SUMMON_ADDS);
                    events.CancelEvent(EVENT_SUMMON_POOL);
                    events.CancelEvent(EVENT_CHECK_PLAYERS);
                    events.CancelEvent(EVENT_STOP_EVENT);
                    events.CancelEvent(EVENT_STOP_EVENT_1);
                    eventphase = 0;
                    instance->SetData(DATA_TYRANDE_EVENT, NOT_STARTED);
                }
                else
                    events.ScheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                break;
            }
            case EVENT_STOP_EVENT:
                events.CancelEvent(EVENT_SUMMON_ADDS);
                events.CancelEvent(EVENT_SUMMON_POOL);
                break;
            case EVENT_STOP_EVENT_1:
                Talk(SAY_LIGHT_6);
                DoCast(SPELL_ACHIEVEMENT);
                events.CancelEvent(EVENT_CHECK_PLAYERS);
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAura(SPELL_IN_SHADOW);
                instance->SetData(DATA_TYRANDE_EVENT, DONE);

                if (auto stalker = me->SummonCreature(NPC_STALKER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f))
                {
                    stalker->RemoveAllAuras();
                    stalker->CastSpell(stalker, SPELL_MOONLIGHT_COSMETIC, true);
                }
                break;
            case EVENT_MOONBOLT:
                if (auto victim = me->getVictim())
                    DoCast(victim, SPELL_MOONBOLT, false);

                events.ScheduleEvent(EVENT_MOONBOLT, 3000);
                break;
            case EVENT_MOONLANCE:
            {
                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                {
                    if (roll_chance_i(20))
                        Talk(SAY_MOONLANCE);

                    moonlanceGUID = target->GetGUID();
                    DoCast(target, SPELL_MOONLANCE_SUMMON_1, false);
                }
                events.ScheduleEvent(EVENT_MOONLANCE, urand(12000, 13000));
                break;
            }
            case EVENT_STARDUST:
                DoCast(SPELL_STARDUST);
                events.ScheduleEvent(EVENT_STARDUST, urand(7000, 8000));
                break;
            case EVENT_EYES_OF_GODDESS:
                Talk(SAY_EYES);
                break;
            default:
                break;
            }
        }
    }
};

struct npc_echo_of_tyrande_moonlance : public ScriptedAI
{
    explicit npc_echo_of_tyrande_moonlance(Creature*  creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetSpeed(MOVE_RUN, (me->GetEntry() == NPC_MOONLANCE_1 ? 0.6f : 0.9f), true);
    }

    void MovementInform(uint32 type, uint32 data) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (data == POINT_MOONLANCE)
        {
            if (me->GetEntry() == NPC_MOONLANCE_1)
            {
                Position pos1_1, pos1_2, pos2_1, pos2_2, pos3_1, pos3_2;
                me->GetNearPosition(pos1_1, 3.0f, -(M_PI / 4.0f));
                me->GetNearPosition(pos1_2, 30.0f, -(M_PI / 4.0f));
                me->GetNearPosition(pos2_1, 3.0f, 0.0f);
                me->GetNearPosition(pos2_2, 30.0f, 0.0f);
                me->GetNearPosition(pos3_1, 3.0f, (M_PI / 4.0f));
                me->GetNearPosition(pos3_2, 30.0f, (M_PI / 4.0f));

                if (auto lance1 = me->SummonCreature(NPC_MOONLANCE_2_1, pos1_1, TEMPSUMMON_TIMED_DESPAWN, 30000))
                    lance1->GetMotionMaster()->MovePoint(POINT_MOONLANCE, pos1_2);
                if (auto lance2 = me->SummonCreature(NPC_MOONLANCE_2_2, pos2_1, TEMPSUMMON_TIMED_DESPAWN, 30000))
                    lance2->GetMotionMaster()->MovePoint(POINT_MOONLANCE, pos2_2);
                if (auto lance3 = me->SummonCreature(NPC_MOONLANCE_2_3, pos3_1, TEMPSUMMON_TIMED_DESPAWN, 30000))
                    lance3->GetMotionMaster()->MovePoint(POINT_MOONLANCE, pos3_2);
            }

            me->DespawnOrUnsummon(500);
        }
    }
};

struct npc_echo_of_tyrande_pool_of_moonlight : public ScriptedAI
{
    explicit npc_echo_of_tyrande_pool_of_moonlight(Creature*  creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    uint32 uiShrinkTimer;
    uint32 uiDespawnTimer;

    void Reset() override
    {
        uiShrinkTimer = 5000;
        uiDespawnTimer = 60000;
    }

    void UpdateAI(uint32 diff) override
    {
        if (uiDespawnTimer <= diff)
        {
            uiDespawnTimer = 60000;
            me->DespawnOrUnsummon(500);
        }
        else
            uiDespawnTimer -= diff;

        if (uiShrinkTimer <= diff)
        {
            uiShrinkTimer = 2000;
            DoCast(me, SPELL_SHRINK, true);
        }
        else
            uiShrinkTimer -= diff;
    }
};

class spell_echo_of_tyrande_tears_of_elune_script : public SpellScript
{
    PrepareSpellScript(spell_echo_of_tyrande_tears_of_elune_script);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster)
            return;
        if (!target)
            return;

        if (roll_chance_i(50))
            caster->CastSpell(target, SPELL_TEARS_OF_ELUNE_MISSILE, true);
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_echo_of_tyrande_tears_of_elune_script::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class at_et_tyrande : public AreaTriggerScript
{
    public:
        at_et_tyrande() : AreaTriggerScript("at_et_tyrande") {}

        bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/)
        {
            if (!player)
                return true;

            if (player->IsBeingTeleported() || player->isBeingLoaded())
                return true;

            if (InstanceScript* instance = player->GetInstanceScript())
            {
                if (instance->GetData(DATA_TYRANDE_EVENT) != IN_PROGRESS && 
                    instance->GetData(DATA_TYRANDE_EVENT) != DONE)
                {
                    if (auto tyrande = ObjectAccessor::GetCreature(*player, instance->GetGuidData(DATA_ECHO_OF_TYRANDE)))
                    {
                        tyrande->AI()->Talk(SAY_INTRO_1);
                        tyrande->AI()->DoAction(ACTION_START_EVENT);
                        instance->SetData(DATA_TYRANDE_EVENT, IN_PROGRESS);
                    }
                }
            }
            return true;
        }
};

void AddSC_boss_echo_of_tyrande()
{
    RegisterCreatureAI(boss_echo_of_tyrande);
    RegisterCreatureAI(npc_echo_of_tyrande_moonlance);
    RegisterCreatureAI(npc_echo_of_tyrande_pool_of_moonlight);
    RegisterSpellScript(spell_echo_of_tyrande_tears_of_elune_script);
    new at_et_tyrande();
}
