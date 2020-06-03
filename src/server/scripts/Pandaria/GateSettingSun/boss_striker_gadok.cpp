/*==============
==============*/

#include "gate_setting_sun.h"

struct StrafPointStruct
{
    uint8 pointIdBeginOutside;
    Position beginOutside;

    uint8 pointIdBegin;
    Position begin;
    
    uint8 pointIdEnd;
    Position end;
    
    uint8 pointIdOutside;
    Position outside;
};

enum eMovements
{
    POINT_NORTH_START       = 1,
    POINT_SOUTH_START       = 2,
    POINT_WEST_START        = 3,
    POINT_EAST_START        = 4,

    POINT_NORTH_END         = 5,
    POINT_SOUTH_END         = 6,
    POINT_WEST_END          = 7,
    POINT_EAST_END          = 8,

    POINT_NORTH_OUTSIDE     = 9,
    POINT_SOUTH_OUTSIDE     = 10,
    POINT_WEST_OUTSIDE      = 11,
    POINT_EAST_OUTSIDE      = 12,

    POINT_KRIKTHIK_CIRCLE   = 13,

    MOV_NORTH_SOUTH         = 0,
    MOV_SOUTH_NORTH         = 1,
    MOV_WEST_EAST           = 2,
    MOV_EAST_WEST           = 3
};

StrafPointStruct StrafPoints[4] =
{
    { POINT_NORTH_OUTSIDE, {1258.0f, 2304.644f, 438.0f, 0.0f}, POINT_NORTH_START, {1238.007f, 2304.644f, 435.0f, 0.0f}, POINT_NORTH_END, {1153.398f, 2304.578f, 435.0f, 0.0f}, POINT_SOUTH_OUTSIDE, {1133.4f, 2304.578f, 438.0f, 0.0f} }, // North -> South
    { POINT_SOUTH_OUTSIDE, {1133.4f, 2304.578f, 438.0f, 0.0f}, POINT_SOUTH_START, {1153.398f, 2304.578f, 435.0f, 0.0f}, POINT_SOUTH_END, {1238.007f, 2304.644f, 435.0f, 0.0f}, POINT_NORTH_OUTSIDE, {1258.0f, 2304.644f, 438.0f, 0.0f} }, // South -> North
    { POINT_WEST_OUTSIDE,  {1195.3f, 2366.941f, 438.0f, 0.0f}, POINT_WEST_START,  {1195.299f, 2348.941f, 435.0f, 0.0f}, POINT_WEST_END,  {1195.392f, 2263.441f, 435.0f, 0.0f}, POINT_EAST_OUTSIDE,  {1195.4f, 2243.441f, 438.0f, 0.0f} }, // West  -> East
    { POINT_EAST_OUTSIDE,  {1195.4f, 2243.441f, 438.0f, 0.0f}, POINT_EAST_START,  {1195.392f, 2263.441f, 435.0f, 0.0f}, POINT_EAST_END,  {1195.299f, 2348.941f, 435.0f, 0.0f}, POINT_WEST_OUTSIDE,  {1195.3f, 2366.941f, 438.0f, 0.0f} }  // East  -> West
};

Position CenterPos = {1195.0f, 2304.0f, 438.0f};

enum eSpells
{
    SPELL_PREY_TIME         = 106933,
    SPELL_IMPALING_STRIKE   = 107047,

    SPELL_STRAFING_RUN      = 107342,
    SPELL_STRAFIND_RUN_DMG  = 116298,

    SPELL_RIDE_VEHICLE      = 46598,

    // Disruptor
    SPELL_BOMB              = 115110
};

enum eEvents
{
    EVENT_PREY_TIME         = 1,
    EVENT_IMPALING_STRIKE   = 2,

    EVENT_DISRUPTOR_BOMBARD = 3,
};

enum ePhases
{
    PHASE_MAIN          = 1,
    PHASE_NORTH_SOUTH   = 2,
    PHASE_WEST_EAST     = 3
};

enum eStrafing
{
    STRAF_NONE    = 0,
    STRAF_70      = 1,
    STRAF_30      = 2
};

#define MAX_DISRUPTOR   5
#define MAX_STRIKER     10
#define RADIUS_CIRCLE   100.0f

struct boss_striker_gadok : public BossAI
{
    explicit boss_striker_gadok(Creature* creature) : BossAI(creature, DATA_GADOK) {}

    bool isStrafing;
    uint32 strafingTimer;
    uint8 strafingEventCount;
    uint8 strafingEventProgress;
    uint8 move;

    void Reset() override
    {
        _Reset();
        isStrafing = false;
        strafingTimer = 0;
        strafingEventCount = 0;
        strafingEventProgress = 0;
        move = 0;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        events.RescheduleEvent(EVENT_PREY_TIME, 10000);
        events.RescheduleEvent(EVENT_IMPALING_STRIKE, 19000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        switch (events.ExecuteEvent())
        {
        case EVENT_PREY_TIME:
            if (auto target = SelectTarget(SELECT_TARGET_NEAREST, 1, 5.0f, true))
                DoCast(target, SPELL_PREY_TIME, false);

            events.RescheduleEvent(EVENT_PREY_TIME, 10000);
            break;
        case EVENT_IMPALING_STRIKE:
            if (auto victim = me->getVictim())
                DoCast(victim, SPELL_IMPALING_STRIKE, false);

            events.RescheduleEvent(EVENT_IMPALING_STRIKE, 19000);
            break;
        }
        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
    }
};

struct npc_krikthik : public ScriptedAI
{
    npc_krikthik(Creature* creature) : ScriptedAI(creature) {}

    uint32 nextMovementTimer;
    float actualAngle;
    float myPositionZ;
    bool direction;

    void Reset() override
    {
        nextMovementTimer = 0;
        actualAngle = me->GetAngle(CenterPos.GetPositionX(), CenterPos.GetPositionY());
        direction = urand(0, 1);

        if (direction)
            myPositionZ = 435.0f;
        else
            myPositionZ = 440.0f;

        // Enable Movements
        MovementInform(POINT_MOTION_TYPE, POINT_KRIKTHIK_CIRCLE);
        me->setActive(true);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == POINT_KRIKTHIK_CIRCLE)
            nextMovementTimer = 50;
    }

    void SelectNextWaypoint(float& x, float& y)
    {
        if (direction)
            actualAngle -= M_PI / 8;
        else
            actualAngle += M_PI / 8;

        x = CenterPos.GetPositionX() + (me->GetObjectSize() + RADIUS_CIRCLE) * std::cos(actualAngle);
        y = CenterPos.GetPositionY() + (me->GetObjectSize() + RADIUS_CIRCLE) * std::sin(actualAngle);
    }

    void UpdateAI(uint32 diff) override
    {
        if (nextMovementTimer)
        {
            if (nextMovementTimer <= diff)
            {
                nextMovementTimer = 0;

                float x = 0.0f;
                float y = 0.0f;
                SelectNextWaypoint(x, y);

                me->GetMotionMaster()->MovePoint(POINT_KRIKTHIK_CIRCLE, x, y, myPositionZ);
            }
            else
                nextMovementTimer -= diff;
        }
    }
};

struct npc_krikthik_striker : public ScriptedAI
{
    npc_krikthik_striker(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    bool isAttackerStriker;

    void Reset() override
    {
        isAttackerStriker = false;
    }

    void DoAction(int32 const action) override
    {
        isAttackerStriker = true;

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        Map::PlayerList const &PlayerList = instance->instance->GetPlayers();
        Map::PlayerList::const_iterator it = PlayerList.begin();
        // Randomize it, everything is done in the "for"
        for (uint8 i = 0; i < urand(0, PlayerList.getSize() - 1); ++i, ++it);

        if (auto player = it->getSource())
            me->AI()->AttackStart(player);
    }

    void UpdateAI(uint32 diff) override
    {
        if (isAttackerStriker)
            return;

        DoMeleeAttackIfReady();
    }
};

struct npc_krikthik_disruptor : public ScriptedAI
{
    explicit npc_krikthik_disruptor(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;

    void Reset() override
    {
        events.RescheduleEvent(EVENT_DISRUPTOR_BOMBARD, urand(5000, 20000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!instance)
            return;

        if (instance->GetBossState(DATA_GADOK) != IN_PROGRESS)
            return;

        events.Update(diff);

        switch (events.ExecuteEvent())
        {
        case EVENT_DISRUPTOR_BOMBARD:

            if (!instance)
                break;

            Map::PlayerList const &PlayerList = instance->instance->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            Map::PlayerList::const_iterator it = PlayerList.begin();
            for (uint8 i = 0; i < urand(0, PlayerList.getSize() - 1); ++i, ++it);

            if (it == PlayerList.end())
                return;

            if (auto player = it->getSource())
                DoCast(player, SPELL_BOMB, true); //Triggered to avoid pillars line of sight

            events.RescheduleEvent(EVENT_DISRUPTOR_BOMBARD, urand(5000, 20000));
            break;
        }
    }
};

struct npc_flak_cannon : public ScriptedAI
{
    explicit npc_flak_cannon(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }

    InstanceScript* instance;

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (!instance)
            return;

        if (instance->GetBossState(DATA_GADOK) != DONE)
            return;

        if (spell->Id == 116554) // Fire Flak Cannon
        {
            for (uint8 i = 0; i < 5; ++i)
            {
                if (auto bombarder = instance->instance->GetCreature(instance->GetGuidData(DATA_RANDOM_BOMBARDER)))
                {
                    DoCast(bombarder, 116553, true);
                    bombarder->GetMotionMaster()->MoveFall();
                    bombarder->DespawnOrUnsummon(2000);
                }
            }
        }
    }
};

class spell_prey_time : public AuraScript
{
    PrepareAuraScript(spell_prey_time);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        Unit* target = GetTarget();
        if (!target)
            return;

        target->CastSpell(caster, SPELL_RIDE_VEHICLE, true);
    }

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;
        Unit* target = GetTarget();
        if (!target)
            return;

        caster->RemoveAura(SPELL_RIDE_VEHICLE);
        target->RemoveAura(SPELL_RIDE_VEHICLE);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_prey_time::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_prey_time::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_boss_striker_gadok()
{
    RegisterCreatureAI(boss_striker_gadok);
    RegisterCreatureAI(npc_krikthik_striker);
    RegisterCreatureAI(npc_krikthik_disruptor);
    RegisterCreatureAI(npc_flak_cannon);
    RegisterAuraScript(spell_prey_time);
}
