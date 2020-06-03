/*===================
=====================*/

#include "siege_of_the_niuzoa_temple.h"

enum Actions
{
    ACTION_BULWARK_BREAKS   = 10,
    POINT_BULWARK           = 10
};

enum Yells
{
    SAY_INTRO,
    SAY_AGGRO,
    SAY_BULWARK,
    SAY_SLAY,
    SAY_DEATH,
    EMOTE_BULWARK
};

enum Spells
{
    SPELL_BULWARK           = 119476,
    SPELL_BLADE_RUSH_SUMMON = 124278,
    SPELL_BLADE_RUSH_EFF    = 124283,
    SPELL_BLADE_RUSH_DUMMY  = 124291,
    SPELL_BLADE_RUSH_CHARGE = 124312,
    SPELL_BLADE_RUSH_DMG_C  = 124317,
    SPELL_BLADE_RUSH_DMG_A  = 124290,
    SPELL_TEMPEST           = 119875,

    SPELL_SIEGE_EXPLOSIVE_PARENT    = 119388,
    SPELL_ARMING_VISUAL_YELLOW      = 88315,
    SPELL_ARMING_VISUAL_ORANGE      = 88316,
    SPELL_ARMING_VISUAL_RED         = 88317,
    SPELL_BOMB_ARMED                = 119702,
    SPELL_DETONATE                  = 11970,
    SPELL_CARRYING_EXPLOSIVES       = 119698,
    SPELL_SIEGE_EXPLOSIVE_MISSILE   = 119376,
    SPEL_SIEGE_EXPLOSIVES_VISUAL    = 119380
};

enum Points
{
    POINT_TARGET = 10,
    POINT_RETURN
};

enum Events
{
    EVENT_GROUP_COMBAT  = 1,

    EVENT_BLADE_RUSH    = 1,
    EVENT_BLADE_RUSH_CAST,
    EVENT_BLADE_RUSH_CHARGE,
    EVENT_BLADE_RUSH_DAMAGE,
    EVENT_BLADE_RUSH_END,
    EVENT_TEMPEST,
    EVENT_PLAYER_CHECK,
    EVENT_BULWARK_END,

    EVENT_ARMING_YELLOW = 1,
    EVENT_ARMING_ORANGE,
    EVENT_ARMING_RED,
    EVENT_ARMED,
    EVENT_DETONATE
};

static const Position reinforcements[4] =
{
    { 1741.793f, 5338.747f, 136.2742f, 4.686225f },
    { 1727.012f, 5342.524f, 138.5872f, 4.686225f },
    { 1733.038f, 5332.926f, 136.088f, 4.686225f },
    { 1750.8f, 5332.092f, 134.8477f, 4.686225f }
};

enum Creatures
{
    NPC_AMBER_SAPPER            = 61484,
    NPC_REINFORCEMENTS_STALKER  = 61483,
    NPC_BLADE_RUSH_STALKER      = 63720
};

static const Position sapperPos = { 1837.330f, 5235.642f, 176.60643f, 2.62f };

struct boss_general_pavalak : public BossAI
{
    explicit boss_general_pavalak(Creature* creature) : BossAI(creature, DATA_PAVALAK)
    {
        introDone = false;
    }

    bool introDone;
    int8 phase;
    ObjectGuid rushTargetGUID;

    void Reset() override
    {
        phase = 0;
        rushTargetGUID.Clear();
        me->SetReactState(REACT_AGGRESSIVE);
        _Reset();
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        events.RescheduleEvent(EVENT_BLADE_RUSH, 10000, EVENT_GROUP_COMBAT);
        events.RescheduleEvent(EVENT_TEMPEST, 15000, EVENT_GROUP_COMBAT);
        _EnterCombat();
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_BULWARK_BREAKS)
        {
            events.RescheduleEvent(EVENT_BLADE_RUSH, 10000, EVENT_GROUP_COMBAT);
            events.RescheduleEvent(EVENT_TEMPEST, 15000, EVENT_GROUP_COMBAT);
            events.RescheduleEvent(EVENT_PLAYER_CHECK, 5000);
            summons.DespawnEntry(NPC_REINFORCEMENTS_STALKER);
            me->SetReactState(REACT_AGGRESSIVE);
            events.RescheduleEvent(EVENT_BULWARK_END, 200);
            me->RemoveAura(119798);
        }
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!introDone && who->IsPlayer() && me->IsWithinDist2d(who, 50.0f))
        {
            Talk(SAY_INTRO);
            introDone = true;
        }
        BossAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* /*who*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim && victim->IsPlayer())
            Talk(SAY_SLAY);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_BLADE_RUSH_DUMMY)
        {
            me->SetFacingToObject(target);
            target->CastSpell(target, 124307, true);
            target->CastSpell(target, SPELL_BLADE_RUSH_DMG_A, true);
            events.RescheduleEvent(EVENT_BLADE_RUSH_CHARGE, 200);
        }

    }

    void DamageTaken(Unit* /*owner*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (phase < 2)
        {
            uint32 healthChk = phase ? 35 : 65;

            if (me->HealthBelowPctDamaged(healthChk, damage))
            {
                damage = 0;
                me->SetHealth(me->CountPctFromMaxHealth(healthChk));

                ++phase;
                me->InterruptNonMeleeSpells(true);
                events.CancelEventGroup(EVENT_GROUP_COMBAT);
                events.CancelEvent(EVENT_TEMPEST);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->GetMotionMaster()->MovePoint(POINT_BULWARK, 1701.693f, 5242.439f, 123.9606f);
                DoCast(119798);

            }
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == EVENT_CHARGE)
            {
                me->StopMoving();
                me->AttackStop();
                events.RescheduleEvent(EVENT_BLADE_RUSH_DAMAGE, 200, EVENT_GROUP_COMBAT);
            }
            else if (id == POINT_BULWARK)
            {
                Talk(SAY_BULWARK);
                Talk(EMOTE_BULWARK);
                DoCast(SPELL_BULWARK);

                for (auto itr : reinforcements)
                    me->SummonCreature(NPC_REINFORCEMENTS_STALKER, itr);
            }

        }
    }

    void JustSummoned(Creature* summon) override
    {
        if (summon->GetEntry() == NPC_BLADE_RUSH_STALKER)
        {
            rushTargetGUID = summon->GetGUID();
            me->SetTarget(summon->GetGUID());
            me->SetFacingToObject(summon);
            events.RescheduleEvent(EVENT_BLADE_RUSH_CAST, 500, EVENT_GROUP_COMBAT);
        }

        BossAI::JustSummoned(summon);
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
            case EVENT_BLADE_RUSH:
            {
                me->SetReactState(REACT_PASSIVE);

                if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                {
                    events.DelayEvents(15000);
                    me->SummonCreature(NPC_BLADE_RUSH_STALKER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 15000);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    me->StopMoving();
                    me->AttackStop();
                    events.RescheduleEvent(EVENT_BLADE_RUSH_END, 30000, EVENT_GROUP_COMBAT);
                }
                events.RescheduleEvent(EVENT_BLADE_RUSH, 30000, EVENT_GROUP_COMBAT);
                break;
            }
            case EVENT_BLADE_RUSH_CAST:
                if (auto rushTarget = Creature::GetCreature(*me, rushTargetGUID))
                    DoCast(rushTarget, SPELL_BLADE_RUSH_EFF, false);
                break;
            case EVENT_BLADE_RUSH_CHARGE:
                if (auto rushTarget = Creature::GetCreature(*me, rushTargetGUID))
                    DoCast(rushTarget, SPELL_BLADE_RUSH_CHARGE, false);
                break;
            case EVENT_BLADE_RUSH_DAMAGE:
                if (auto rushTarget = Creature::GetCreature(*me, rushTargetGUID))
                    rushTarget->RemoveAllAuras();
                events.RescheduleEvent(EVENT_BLADE_RUSH_END, 1000, EVENT_GROUP_COMBAT);
                break;
            case EVENT_BLADE_RUSH_END:
                rushTargetGUID.Clear();
                me->SetReactState(REACT_AGGRESSIVE);

                if (auto victim = me->getVictim())
                    AttackStart(victim);
                break;
            case EVENT_TEMPEST:
                DoCast(SPELL_TEMPEST);
                events.RescheduleEvent(EVENT_TEMPEST, 20000, EVENT_GROUP_COMBAT);
                break;
            case EVENT_BULWARK_END:
                if (auto victim = me->getVictim())
                    me->GetMotionMaster()->MoveChase(victim);
                break;
            case EVENT_PLAYER_CHECK:
            {
                bool playersAlive = false;
                Map::PlayerList const &players = me->GetMap()->GetPlayers();
                for (auto itr = players.begin(); itr != players.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        if (player->isAlive() && !player->isGameMaster() && player->GetCurrentAreaID() == 6411)
                        {
                            playersAlive = true;
                            break;
                        }
                    }
                }

                if (!playersAlive)
                    EnterEvadeMode();
                else
                    events.RescheduleEvent(EVENT_PLAYER_CHECK, 5000);
                break;
            }
            }
        }
        DoMeleeAttackIfReady();
    }
};

// 119476
class spell_general_pavalak_bulwark : public AuraScript
{
    PrepareAuraScript(spell_general_pavalak_bulwark);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        if (auto creatureTarget = target->ToCreature())
            creatureTarget->AI()->DoAction(ACTION_BULWARK_BREAKS);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_general_pavalak_bulwark::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// 61452
struct npc_siege_explosive : public ScriptedAI
{
    explicit npc_siege_explosive(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->SetDisplayId(38796);
        me->SetReactState(REACT_PASSIVE);
    }

    EventMap events;

    void IsSummonedBy(Unit* /*who*/) override
    {
        DoCast(SPEL_SIEGE_EXPLOSIVES_VISUAL);
        events.RescheduleEvent(EVENT_ARMING_YELLOW, 1000);
    }

    void OnSpellClick(Unit* /*clicker*/) override
    {
        me->DisappearAndDie();
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_ARMING_YELLOW:
                DoCast(me, SPELL_ARMING_VISUAL_YELLOW, true);
                events.RescheduleEvent(EVENT_ARMING_ORANGE, 1000);
                break;
            case EVENT_ARMING_ORANGE:
                DoCast(me, SPELL_ARMING_VISUAL_ORANGE, true);
                events.RescheduleEvent(EVENT_ARMING_RED, 1000);
                break;
            case EVENT_ARMING_RED:
                DoCast(me, SPELL_ARMING_VISUAL_RED, true);
                events.RescheduleEvent(EVENT_ARMED, 2000);
                break;
            case EVENT_ARMED:
                DoCast(me, SPELL_BOMB_ARMED, true);
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                events.RescheduleEvent(EVENT_DETONATE, 4000);
                break;
            case EVENT_DETONATE:
                DoCast(me, SPELL_DETONATE, true);
                me->DespawnOrUnsummon(500);
                break;
            default:
                break;
            }
        }
    }
};

// 61484
struct npc_pavalak_amber_sapper : public ScriptedAI
{
    explicit npc_pavalak_amber_sapper(Creature* creature) : ScriptedAI(creature) {}

    uint32 moveTimer;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        DoCast(me, SPELL_CARRYING_EXPLOSIVES, true);
        Position pos;
        summoner->GetRandomNearPosition(pos, 40.0f);
        pos.m_positionZ = 123.49f;

        me->GetMotionMaster()->MovePoint(POINT_TARGET, pos);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == POINT_TARGET)
        {
            DoCast(me, SPELL_SIEGE_EXPLOSIVE_MISSILE, false);
            me->RemoveAurasDueToSpell(SPELL_CARRYING_EXPLOSIVES);
            moveTimer = 2000;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!moveTimer)
            return;

        if (moveTimer <= diff)
        {
            me->GetMotionMaster()->MoveTargetedHome();
            me->DespawnOrUnsummon(5000);
            moveTimer = 0;
        }
        else
            moveTimer -= diff;
    }
};

// bombardment - 119512
class spell_pavalak_bombardment : public AuraScript
{
    PrepareAuraScript(spell_pavalak_bombardment);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Position pos = sapperPos;
        float dist = frand(20.0f, 50.0f);
        float angle = (float)rand_norm() * static_cast<float>(2 * M_PI);
        pos.m_positionX += dist * std::cos(angle);
        pos.m_positionY += dist * std::sin(angle);

        caster->SummonCreature(NPC_AMBER_SAPPER, pos);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pavalak_bombardment::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 61483
struct npc_pavalak_reinforcements_summoner : public ScriptedAI
{
    explicit npc_pavalak_reinforcements_summoner(Creature* creature) : ScriptedAI(creature)
    {
        summonerGUID.Clear();
        me->SetReactState(REACT_PASSIVE);
        me->SetDisplayId(11686);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
    }

    ObjectGuid summonerGUID;

    void IsSummonedBy(Unit* summoner) override
    {
        summonerGUID = summoner->GetGUID();
    }

    void JustSummoned(Creature* summon) override
    {
        if (auto pavalak = Creature::GetCreature(*me, summonerGUID))
            pavalak->AI()->JustSummoned(summon);
    }
};

// 63720
struct npc_blade_rush : public ScriptedAI
{
    explicit npc_blade_rush(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisplayId(11686);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
    }

    void Reset() override
    {
        me->AddAura(124288, me);
    }
};

void AddSC_boss_general_pavalak()
{
    RegisterCreatureAI(boss_general_pavalak);
    RegisterCreatureAI(npc_siege_explosive);
    RegisterCreatureAI(npc_pavalak_amber_sapper);
    RegisterCreatureAI(npc_pavalak_reinforcements_summoner);
    RegisterCreatureAI(npc_blade_rush);
    RegisterAuraScript(spell_general_pavalak_bulwark);
    RegisterAuraScript(spell_pavalak_bombardment);
}