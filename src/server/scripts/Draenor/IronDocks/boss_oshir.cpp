/*
    Dungeon : Iron Docks 93-95
    Encounter: Oshir
*/

#include "iron_docks.h"

enum Spells
{
    //SPELL_RENDING_SLASHES    = 168184, intro?
    SPELL_HAMSTRING         = 162350,
    SPELL_RENDING_SLASHES   = 161239,
    SPELL_R_SLASHES_REMOVE  = 162910,
    SPELL_BREAKOUT_JUMP     = 178124,
    SPELL_BREAKOUT_RYLAK    = 178126,
    SPELL_BREAKOUT_WOLF     = 178128,
    SPELL_TIME_TO_FEED      = 161530,
    SPELL_FEEDING_FRENZY    = 162424,

    //Christmas
    SPELL_CHRISTMAS_CAP     = 176924
};

enum eEvents
{
    EVENT_HAMSTRING         = 1,
    EVENT_RENDING_SLASHES   = 2,
    EVENT_JUMP_TO_CAGE      = 3,
    EVENT_OPEN_CAGE         = 4,
    EVENT_TIME_FEED         = 5
};

enum Emotes
{
    //Intro
    EMOTE_1             = 6794, //23:32
    EMOTE_2             = 680,  //23:35
    EMOTE_3             = 783,  //23:35
    EMOTE_4             = 799,  //23:37
    EMOTE_5             = 6794, //23:43

    EMOTE_DESTOY_CAGE   = 7598
};

struct boss_oshir : public BossAI
{
    explicit boss_oshir(Creature* creature) : BossAI(creature, DATA_OSHIR)
    {
        intro = false;
    }

    uint8 rand;
    uint32 DamageCount;
    uint32 HealthPct;

    bool intro;

    void Reset() override
    {
        events.Reset();
        _Reset();
        summons.DespawnAll();
        rand = urand(0, 1);
        DamageCount = 0;
        HealthPct = 0;
        me->RemoveAurasDueToSpell(SPELL_FEEDING_FRENZY);
        me->SetReactState(REACT_AGGRESSIVE);

        if (!intro)
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);

        if (sGameEventMgr->IsActiveEvent(2))
            DoCast(SPELL_CHRISTMAS_CAP);
        else
        {
            if (me->HasAura(SPELL_CHRISTMAS_CAP))
                me->RemoveAura(SPELL_CHRISTMAS_CAP);
        }
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();
        events.RescheduleEvent(EVENT_HAMSTRING, 4000);
        events.RescheduleEvent(EVENT_JUMP_TO_CAGE, 17000);
        events.RescheduleEvent(EVENT_TIME_FEED, 36000);
    }

    void EnterEvadeMode() override
    {
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        summons.DespawnAll();
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who->IsPlayer())
            return;

        if (me->GetDistance(who) > 100.0f)
            return;

        if (!intro)
        {
            intro = true;

            if (GameObject* cage = me->FindNearestGameObject(239227, 5.0f))
                cage->SetGoState(GO_STATE_ACTIVE);

            me->SetHomePosition(6956.0f, -1101.0f, 4.7f, 4.0f);
            me->NearTeleportTo(6956.0f, -1101.0f, 4.7f, 4.0f); //hack
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            summons.DespawnEntry(NPC_RENDING_SLASHES);
            DoCast(SPELL_R_SLASHES_REMOVE);
            me->SetReactState(REACT_AGGRESSIVE);
        }

        if (type == EFFECT_MOTION_TYPE)
        {
            switch (id)
            {
            case SPELL_BREAKOUT_JUMP:
                me->PlayOneShotAnimKit(EMOTE_DESTOY_CAGE);
                events.RescheduleEvent(EVENT_OPEN_CAGE, 1000);
                break;
            }
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);
        //wtf? В сниффе sum кастит на босса 161298, босс кастит на него джамп 161243, а на видео босс просто идет.
        if (summon->GetEntry() == NPC_RENDING_SLASHES)
            me->GetMotionMaster()->MovePoint(1, summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ());
    }

    void DamageTaken(Unit* who, uint32 &damage, DamageEffectType /*dmgType*/) override
    {
        if (!who->IsPlayer())
            return;

        if (AuraEffect const* aurEff = me->GetAuraEffect(SPELL_FEEDING_FRENZY, EFFECT_0))
        {
            DamageCount += damage;
            HealthPct = me->CountPctFromMaxHealth(aurEff->GetAmount());

            if (DamageCount >= HealthPct)
            {
                DamageCount = 0;
                me->InterruptSpell(CURRENT_CHANNELED_SPELL, false);
                aurEff->GetBase()->Remove();
            }
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_BREAKOUT_JUMP)
            if (target->GetEntry() == NPC_RYLAK_CAGE || target->GetEntry() == NPC_WOLF_CAGE)
                target->ToCreature()->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->GetDistance(me->GetHomePosition()) >= 120.0f)
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
            case EVENT_HAMSTRING:
                DoCast(SPELL_HAMSTRING);
                me->StopAttack();
                events.RescheduleEvent(EVENT_RENDING_SLASHES, 3000);
                events.RescheduleEvent(EVENT_HAMSTRING, 46000);
                break;
            case EVENT_RENDING_SLASHES:
                DoCast(me, SPELL_RENDING_SLASHES, true);
                break;
            case EVENT_JUMP_TO_CAGE:
                me->StopAttack();
                me->GetMotionMaster()->Clear(false);
                if (rand == 0)
                {
                    rand = 1;
                    DoCast(SPELL_BREAKOUT_RYLAK);
                }
                else
                {
                    rand = 0;
                    DoCast(SPELL_BREAKOUT_WOLF);
                }
                events.RescheduleEvent(EVENT_JUMP_TO_CAGE, 46000);
                break;
            case EVENT_TIME_FEED:
                DoCast(SPELL_TIME_TO_FEED);
                events.RescheduleEvent(EVENT_TIME_FEED, 60000);
                break;
            case EVENT_OPEN_CAGE:
                for (uint8 i = 0; i < 19; ++i)
                    if (me->GetDistance(cageSpawn[i]) < 4.5f)
                        instance->SetData(DATA_OSHIR_CAGE, i);

                me->SetReactState(REACT_AGGRESSIVE);
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

void AddSC_boss_oshir()
{
    RegisterCreatureAI(boss_oshir);
}
