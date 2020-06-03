/*
    Dungeon : Shadowmoon Burial Grounds 100
    Encounter: Sadana Bloodfury
*/

#include "shadowmoon_burial_grounds.h"

enum Says
{
    SAY_ENTER_ROOM      = 0,
    SAY_AGGRO           = 1,
    SAY_DEATH           = 2,
    SAY_KILL_PLAYER     = 3,
    SAY_DARK_COMMUNION  = 4,
    SAY_DARK_ECLIPSE    = 5
};

enum Spells
{
    //Sadana
    SPELL_SHADOW_RITUAL_VISUAL    = 152158,
    SPELL_DARK_ECLIPSE_CHECK      = 164710,
    SPELL_DARK_ECLIPSE_HIT_RUNE   = 164705,
    SPELL_DEATHSPIKE              = 153079,
    SPELL_DAGGERFALL_SUMMON       = 153200,
    SPELL_DAGGERFALL_TARGETING    = 153240,
    SPELL_WHISPERS_DARK_STAR      = 153094,
    SPELL_DARK_COMMUNION          = 153153,
    SPELL_DARK_ECLIPSE_VISUAL     = 164685,
    SPELL_DARK_ECLIPSE_CHANNEL    = 164974,
    SPELL_DARK_ECLIPSE_AT         = 164704,
    SPELL_DARK_ECLIPSE_DMG        = 164686,
    SPELL_TELEPORT_TO_HOME        = 155689,
    //Daggerfall
    SPELL_DAGGERFALL_VISUAL       = 153216,
    SPELL_DAGGERFALL_DAMAGE_FALL  = 153225,
    SPELL_DAGGERFALL_DAMAGE_TICK  = 153236,
    //Spirit
    SPELL_INVISIBLE_MAN_TRANSFORM = 152821,
    SPELL_DARK_COMMUNION_HEAL     = 153164,
    SPELL_PURPLE_SHADOWY_DROWN    = 153136,
    SPELL_PURPLE_SHADOWY          = 152311,
    SPELL_FEIGN_DEATH             = 114371,
    //Runes
    SPELL_SHADOW_RUNE_1_AT        = 152684,
    SPELL_SHADOW_RUNE_2_AT        = 152691,
    SPELL_SHADOW_RUNE_3_AT        = 152695
};

enum eEvents
{
    EVENT_DEATHSPIKE        = 1,
    EVENT_DAGGERFALL_SUMM   = 2,
    EVENT_DARK_STAR         = 3,
    EVENT_DARK_COMMUNION    = 4,
    EVENT_DARK_ECLIPSE_1    = 5,
    EVENT_DARK_ECLIPSE_2    = 6,
    EVENT_DARK_ECLIPSE_3    = 7,

    //Dagger
    EVENT_MOVE_FALL_1       = 1,
    EVENT_MOVE_FALL_2       = 2
};

Position const spiritPos[4] =
{
    {1795.85f,  2.427f, 282.14f, 4.67f},
    {1772.43f, -44.29f, 281.39f, 0.74f},
    {1812.56f, -49.17f, 280.96f, 1.99f},
    {1795.8f,  -26.65f, 261.39f, 3.14f}
};

struct boss_sadana_bloodfury : public BossAI
{
    explicit boss_sadana_bloodfury(Creature* creature) : BossAI(creature, DATA_SADANA)
    {
        DoCast(SPELL_SHADOW_RITUAL_VISUAL);
        intro = true;
    }

    bool intro;

    void Reset() override
    {
        events.Reset();
        _Reset();
        me->SetReactState(REACT_AGGRESSIVE);

        for (uint8 i = 0; i < 3; ++i)
            me->SummonCreature(NPC_DEFILED_SPIRIT, spiritPos[i]);

        me->SummonCreature(NPC_DARK_ECLIPSE, spiritPos[3]);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO);
        _EnterCombat();

        me->RemoveAurasDueToSpell(SPELL_SHADOW_RITUAL_VISUAL);
        DoCast(SPELL_DARK_ECLIPSE_CHECK);

        events.RescheduleEvent(EVENT_DEATHSPIKE, 10000);
        events.RescheduleEvent(EVENT_DAGGERFALL_SUMM, 10000);
        events.RescheduleEvent(EVENT_DARK_STAR, 14000);
        events.RescheduleEvent(EVENT_DARK_COMMUNION, 24000);
        events.RescheduleEvent(EVENT_DARK_ECLIPSE_1, 44000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_DEATH);
        _JustDied();
    }

    void KilledUnit(Unit* victim) override
    {
        if (!victim->IsPlayer())
            return;

        uint8 rand = urand(0, 1);

        if (rand)
            Talk(SAY_KILL_PLAYER);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (Player* player = who->ToPlayer())
        {
            if (me->IsWithinDistInMap(who, 110.0f) && intro)
            {
                intro = false;
                Talk(SAY_ENTER_ROOM);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->GetDistance(me->GetHomePosition()) >= 25.0f)
        {
            EnterEvadeMode();
            return;
        }

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_DEATHSPIKE:
                DoCast(me, SPELL_DEATHSPIKE, true);
                break;
            case EVENT_DAGGERFALL_SUMM:
                DoCast(SPELL_DAGGERFALL_TARGETING);
                events.RescheduleEvent(EVENT_DAGGERFALL_SUMM, 28000);
                break;
            case EVENT_DARK_STAR:
                DoCast(SPELL_WHISPERS_DARK_STAR);
                events.RescheduleEvent(EVENT_DARK_STAR, 28000);
                break;
            case EVENT_DARK_COMMUNION:
                me->StopAttack();
                DoCast(SPELL_DARK_COMMUNION);
                Talk(SAY_DARK_COMMUNION);
                events.RescheduleEvent(EVENT_DARK_COMMUNION, 60000);
                break;
            case EVENT_DARK_ECLIPSE_1:
                DoCast(SPELL_DARK_ECLIPSE_VISUAL);
                events.RescheduleEvent(EVENT_DARK_ECLIPSE_1, 44000);
                events.RescheduleEvent(EVENT_DARK_ECLIPSE_2, 4000);
                break;
            case EVENT_DARK_ECLIPSE_2:
                me->StopAttack();
                DoCast(me, SPELL_TELEPORT_TO_HOME, true);
                DoCast(me, SPELL_DARK_ECLIPSE_AT, true);
                DoCast(SPELL_DARK_ECLIPSE_CHANNEL);
                Talk(SAY_DARK_ECLIPSE);
                events.RescheduleEvent(EVENT_DARK_ECLIPSE_3, 6000);
                break;
            case EVENT_DARK_ECLIPSE_3:
                DoCast(SPELL_DARK_ECLIPSE_DMG);
                me->SetReactState(REACT_AGGRESSIVE);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

struct npc_sadana_shadow_rune : public ScriptedAI
{
    explicit npc_sadana_shadow_rune(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        if (summoner->GetEntry() == NPC_WORD_TRIGGER)
            DoCast(SPELL_SHADOW_RUNE_1_AT);

        if (summoner->GetEntry() == NPC_SADANA_BLOODFURY)
            DoCast(SPELL_SHADOW_RUNE_2_AT);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* /*spell*/) override
    {
        if (target->IsPlayer())
        {
            me->RemoveAurasDueToSpell(SPELL_SHADOW_RUNE_1_AT);
            me->RemoveAurasDueToSpell(SPELL_SHADOW_RUNE_2_AT);
        }
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        if (target->GetEntry() == NPC_SADANA_BLOODFURY)
            if (spell->Id == SPELL_DARK_ECLIPSE_HIT_RUNE)
                if (!me->HasAura(SPELL_SHADOW_RUNE_2_AT) && target->HasAura(SPELL_DARK_ECLIPSE_CHANNEL))
                    DoCast(SPELL_SHADOW_RUNE_2_AT);
    }

    void UpdateAI(uint32 diff) override {}
};

struct npc_sadana_daggerfall : public ScriptedAI
{
    explicit npc_sadana_daggerfall(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        DoCast(me, SPELL_DAGGERFALL_VISUAL, true);

        events.RescheduleEvent(EVENT_MOVE_FALL_1, 2000);
        events.RescheduleEvent(EVENT_MOVE_FALL_2, 5000);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        DoCast(me, SPELL_DAGGERFALL_DAMAGE_TICK, true);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_MOVE_FALL_1:
                DoCast(me, SPELL_DAGGERFALL_DAMAGE_FALL, true);
                break;
            case EVENT_MOVE_FALL_2:
                me->GetMotionMaster()->MoveFall();
                break;
            }
        }
    }
};

//75966
struct npc_sadana_defiled_spirit : public ScriptedAI
{
    explicit npc_sadana_defiled_spirit(Creature* creature) : ScriptedAI(creature)
    {
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISTRACT, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
    }

    EventMap events;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        DoCast(SPELL_INVISIBLE_MAN_TRANSFORM);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 1)
            {
                me->RemoveAurasDueToSpell(SPELL_PURPLE_SHADOWY_DROWN);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(SPELL_PURPLE_SHADOWY);
                me->SetAnimKitId(4843); //GripMoveAnim

                if (auto owner = me->GetAnyOwner())
                    me->GetMotionMaster()->MoveFollow(owner, 0.0f, owner->GetAngle(me));
            }
        }

        if (type == FOLLOW_MOTION_TYPE)
        {
            if (auto owner = me->GetAnyOwner())
            {
                if (me->GetDistance(owner) < 3.0f)
                {
                    me->SetAnimKitId(0);
                    me->CastSpell(owner, SPELL_DARK_COMMUNION_HEAL);
                    me->RemoveAurasDueToSpell(SPELL_DARK_COMMUNION);
                    owner->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    EnterEvadeMode();
                }
            }
        }
    }

    void DamageTaken(Unit* /*who*/, uint32& damage, DamageEffectType /*dmgType*/) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;

            if (auto owner = me->GetAnyOwner())
                owner->ToCreature()->SetReactState(REACT_AGGRESSIVE);

            me->SetAnimKitId(0);
            me->RemoveAurasDueToSpell(SPELL_DARK_COMMUNION);
            DoCast(me, SPELL_FEIGN_DEATH, true);
            events.RescheduleEvent(EVENT_1, 5000);
        }
    }

    void SpellHit(Unit* target, SpellInfo const* spell) override
    {
        if (target->GetEntry() == NPC_SADANA_BLOODFURY)
        {
            if (spell->Id == SPELL_DARK_COMMUNION)
            {
                me->RemoveAurasDueToSpell(SPELL_INVISIBLE_MAN_TRANSFORM);
                me->CastSpell(me, SPELL_PURPLE_SHADOWY_DROWN, true);
                me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() - 20.0f);
            }
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
                EnterEvadeMode();
                me->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
                break;
            }
        }
    }
};

//153240
class spell_sadana_daggerfall_filter : public SpellScript
{
    PrepareSpellScript(spell_sadana_daggerfall_filter);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        uint8 count;
        switch (GetCaster()->GetMap()->GetDifficultyID())
        {
        case DIFFICULTY_NORMAL:
            count = 1;
            break;
        case DIFFICULTY_HEROIC:
            count = 2;
            break;
        case DIFFICULTY_MYTHIC_DUNGEON:
            count = 3;
            break;
        default:
            count = 3;
            break;
        }

        Trinity::Containers::RandomResizeList(targets, count);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sadana_daggerfall_filter::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

//153200, 153225
class spell_sadana_daggerfall : public SpellScript
{
    PrepareSpellScript(spell_sadana_daggerfall);

    void ModDestHeight(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(EFFECT_0);

        switch (GetSpellInfo()->Id)
        {
        case SPELL_DAGGERFALL_SUMMON:
        {
            static Position const offset = { 0.0f, 0.0f, 12.0f, 0.0f };
            GetHitDest()->RelocateOffset(offset);
            break;
        }
        case SPELL_DAGGERFALL_DAMAGE_FALL:
        {
            static Position const offset = { 0.0f, 0.0f, -12.0f, 0.0f };
            WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());
            dest->RelocateOffset(offset);
            break;
        }
        }
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_sadana_daggerfall::ModDestHeight, EFFECT_0, SPELL_EFFECT_SUMMON);
        OnEffectLaunch += SpellEffectFn(spell_sadana_daggerfall::ModDestHeight, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
    }
};

//153153
class spell_sadana_dark_communion : public SpellScript
{
    PrepareSpellScript(spell_sadana_dark_communion);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster()));
        WorldObject* target = targets.back();
        targets.clear();
        targets.push_back(target);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sadana_dark_communion::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

void AddSC_boss_sadana_bloodfury()
{
    RegisterCreatureAI(boss_sadana_bloodfury);
    RegisterCreatureAI(npc_sadana_shadow_rune);
    RegisterCreatureAI(npc_sadana_daggerfall);
    RegisterCreatureAI(npc_sadana_defiled_spirit);
    RegisterSpellScript(spell_sadana_daggerfall_filter);
    RegisterSpellScript(spell_sadana_daggerfall);
    RegisterSpellScript(spell_sadana_dark_communion);
}
