/*
    Dungeon : Eye of Azshara 100-110
    Encounter: Serpentrix
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "eye_of_azshara.h"

enum Says
{
    SAY_SUBMERGE           = 0,
    SAY_TOXIC_WOUND        = 1,
    
};

enum Spells
{
    SPELL_TOXIC_WOUND           = 191855,
    SPELL_POISON_SPIT           = 192050,
    SPELL_POISON_SPIT_TARG      = 191839,
    SPELL_POISON_SPIT_S1        = 191841, //Speed: 2.00
    SPELL_POISON_SPIT_S2        = 191843, //Speed: 3.00
    SPELL_POISON_SPIT_S3        = 191845, //Speed: 0.75
    SPELL_SUBMERGE              = 191873,
    SPELL_SUM_HYDRA_SPAWN       = 192010,
    SPELL_RAMPAGE               = 191848,
    SPELL_RAMPAGE_TARG          = 191850,
    SPELL_HYDRA_HEAD            = 202680,

    SPELL_BLAZING_NOVA          = 192003,
    SPELL_ARCANE_BLAST          = 192005,
};

enum eEvents
{
    EVENT_TOXIC_WOUND           = 1,
    EVENT_POISON_SPIT           = 2,
    EVENT_SUBMERGE              = 3,
    EVENT_SUM_HYDRA             = 4,
    EVENT_RAMPAGE               = 5,
};

Position const tpPos[6] =
{
    {-3256.36f, 4370.39f, 0.37f},
    {-3294.20f, 4460.52f, -0.6f},
    {-3304.17f, 4405.53f, 0.08f},
    {-3193.61f, 4435.89f, -0.7f},
    {-3199.40f, 4384.95f, 0.16f},
    {-3246.71f, 4479.65f, 0.26f}
};

uint32 hydrasEntry[2] =
{
    NPC_BLAZING_HYDRA_SPAWN,
    NPC_ARCANE_HYDRA_SPAWN
};

//91808
struct boss_serpentrix : public BossAI
{
    boss_serpentrix(Creature* creature) : BossAI(creature, DATA_SERPENTRIX) 
    {
        SetCombatMovement(false);
    }

    uint8 healthPct = 0;
    uint8 sumGidrCount = 0;
    uint16 checkVictimTimer = 0;

    void Reset() override
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(191855);
        _Reset();
        healthPct = 68;
        checkVictimTimer = 2000;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        _EnterCombat();

        events.RescheduleEvent(EVENT_TOXIC_WOUND, 6000);
        events.RescheduleEvent(EVENT_POISON_SPIT, 11000);
    }

    void EnterEvadeMode() override
    {
        me->NearTeleportTo(me->GetHomePosition());
        BossAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        me->PlayOneShotAnimKit(10626); //Death anim
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || who->GetDistance(me) > 3.0f)
            return;

        BossAI::MoveInLineOfSight(who);
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_POISON_SPIT_TARG:
            case SPELL_RAMPAGE_TARG:
                if (!target->HasAura(191797)) //Violent Winds
                    DoCast(target, SPELL_POISON_SPIT_S1, true); //Medium speed
                else if (me->GetOrientation() >= 0.0f && me->GetOrientation() <= 3.14f)
                    DoCast(target, SPELL_POISON_SPIT_S3, true); //Fast speed
                else 
                    DoCast(target, SPELL_POISON_SPIT_S2, true); //Slow speed
                break;
            case SPELL_SUBMERGE:
                events.RescheduleEvent(EVENT_SUM_HYDRA, 500);
                break;
            case SPELL_SUM_HYDRA_SPAWN:
                me->SummonCreature(hydrasEntry[sumGidrCount], target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
                    ++sumGidrCount;
                break;
            case SPELL_TOXIC_WOUND:
                Talk(SAY_TOXIC_WOUND, target->GetGUID());
            default:
                break;
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (me->HealthBelowPct(healthPct))
        {
            healthPct -= 34;
            me->CastStop();
            events.RescheduleEvent(EVENT_SUBMERGE, 1);
            checkVictimTimer += 10000;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (checkVictimTimer <= diff)
        {
            checkVictimTimer = 3000;

            if (!me->IsWithinMeleeRange(me->getVictim()))
            {
                bool found = false;

                if (auto target = SelectTarget(SELECT_TARGET_NEAREST, 0, 100.0f, true))
                {
                    if (me->IsWithinMeleeRange(target))
                    {
                        found = true;
                        DoModifyThreatPercent(me->getVictim(), -100);
                        me->AddThreat(target, 10000.0f);
                    }
                }
                if (!found && !me->HasAura(SPELL_RAMPAGE))
                    events.RescheduleEvent(EVENT_RAMPAGE, 100);
                checkVictimTimer = 1500;
            }
        }
        else
            checkVictimTimer -= diff;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TOXIC_WOUND:
                    DoCast(SPELL_TOXIC_WOUND);
                    events.RescheduleEvent(EVENT_TOXIC_WOUND, 25000);
                    break;
                case EVENT_POISON_SPIT:
                    DoCast(SPELL_POISON_SPIT);
                    events.RescheduleEvent(EVENT_POISON_SPIT, 9000);
                    break;
                case EVENT_SUBMERGE:
                    Talk(SAY_SUBMERGE);
                    DoCast(SPELL_SUBMERGE);
                    break;
                case EVENT_SUM_HYDRA:
                    sumGidrCount = 0;
                    DoCast(SPELL_SUM_HYDRA_SPAWN);
                    break;
                case EVENT_RAMPAGE:
                    DoCast(SPELL_RAMPAGE);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//97259, 97260
struct npc_serpentrix_hydras : public ScriptedAI
{
    npc_serpentrix_hydras(Creature* creature) : ScriptedAI(creature) 
    {
        SetCombatMovement(false);
    }

    EventMap events;

    void Reset() override {}

    void IsSummonedBy(Unit* summoner) override
    {
        if (!summoner->isInCombat())
            me->DespawnOrUnsummon();

        summoner->CastSpell(me, SPELL_HYDRA_HEAD); //передает урон тому, кто кастил
        DoZoneInCombat(me, 100.0f);
        events.RescheduleEvent(EVENT_1, 2000);
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
                    if (me->GetEntry() == NPC_BLAZING_HYDRA_SPAWN)
                        DoCast(SPELL_BLAZING_NOVA);
                    else if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        DoCast(pTarget, SPELL_ARCANE_BLAST);
                    events.RescheduleEvent(EVENT_1, 2500);
                    break;
            }
        }
    }
};

//91792
struct npc_eye_of_azshara_stormwake_hydra : public ScriptedAI
{
    npc_eye_of_azshara_stormwake_hydra(Creature* creature) : ScriptedAI(creature) {}

    EventMap events;

    void Reset() override {}

    void EnterCombat(Unit* /*who*/) override
    {
        events.RescheduleEvent(EVENT_1, urandms(8, 10));
        events.RescheduleEvent(EVENT_2, urandms(18, 20));
        events.RescheduleEvent(EVENT_3, 2000);
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
                    DoCast(196296);
                    events.RescheduleEvent(EVENT_1, urandms(20,22));
                    break;
                case EVENT_2:
                    DoCast(196290);
                    events.RescheduleEvent(EVENT_2, 30000);
                    break;
                case EVENT_3:
                {
                    events.RescheduleEvent(EVENT_3, 2000);
                    std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
                    for (auto ref : threatList)
                    {
                        if (auto player = Player::GetPlayer(*me, ref->getUnitGuid()))
                        {
                            if (player->GetDistance(me) < 5.0f && me->isInBack(player, M_PI / 2))
                            {
                                DoCast(196287);
                                events.RescheduleEvent(EVENT_3, 24000);
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
    }
};

//91787
struct npc_eye_of_azshara_cove_seagull : public ScriptedAI
{
    npc_eye_of_azshara_cove_seagull(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
    }

    InstanceScript* instance;
    uint32 checkWindTimer = 1000;
    uint32 blindingPeckTimer = 0;

    void Reset() override {}

    void EnterCombat(Unit* /*who*/) override
    {
        blindingPeckTimer = urandms(8, 10);
    }

    void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply) override
    {
        if (spellId == 194899 && apply)
        {
            float x = me->GetPositionX();
            float y = me->GetPositionY();

            me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetMap()->GetHeight(x, y, me->GetPositionZ()));
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (checkWindTimer)
        {
            if (checkWindTimer <= diff)
            {
                checkWindTimer = 1000;

                if (!me->isInCombat() && instance->GetData(DATA_WIND_ACTIVE))
                {
                    checkWindTimer = 0;
                    DoCast(me, 194899, true);
                }
            }
            else
                checkWindTimer -= diff;
        }

        if (!UpdateVictim())
            return;

        if (blindingPeckTimer)
        {
            if (blindingPeckTimer <= diff)
            {
                blindingPeckTimer = 20000;
                DoCastVictim(195561);
            }
            else
                blindingPeckTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

//191873
class spell_serpentrix_submerge_teleport : public SpellScript
{
    PrepareSpellScript(spell_serpentrix_submerge_teleport);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (!GetCaster())
            return;

        uint8 rand = urand(0, 5);

        while (GetCaster()->GetDistance(tpPos[rand]) < 10.0f)
        {
            rand = urand(0, 5);
        }
        GetCaster()->NearTeleportTo(tpPos[rand].GetPosition());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_serpentrix_submerge_teleport::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_TELEPORT_L);
    }
};

//196175
class spell_eye_of_azshara_armorshell : public AuraScript
{
    PrepareAuraScript(spell_eye_of_azshara_armorshell);

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & amount)
    {
        Unit* caster = GetCaster();
        if (!caster)
           return;

        if (amount == 0)
        {
           Remove(AURA_REMOVE_BY_ENEMY_SPELL);
           caster->CastStop();
           caster->CastSpell(caster, 196183);
        }
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_eye_of_azshara_armorshell::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

//202314
class spell_eye_of_azshara_vile_blood : public AuraScript
{
    PrepareAuraScript(spell_eye_of_azshara_vile_blood);

    void OnProc(AuraEffect const* auraEffect, ProcEventInfo& eventInfo)
    {
        auto target = eventInfo.GetActionTarget();
        if (!target || GetCaster()->GetGUID() == target->GetGUID())
            return;
     
        GetCaster()->CastSpell(target, 202315, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_eye_of_azshara_vile_blood::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

//196296, 196290
class spell_eye_of_azshara_roiling_storm : public AuraScript
{
    PrepareAuraScript(spell_eye_of_azshara_roiling_storm);

    void OnTick(AuraEffect const* aurEff)
    {
        auto caster = GetCaster()->ToCreature();
        if (!caster)
            return;

        if (GetId() == 196296)
        {
            if (aurEff->GetTickNumber() == 1)
                caster->CastSpell(caster, 196298, true);
        }
        else if (GetId() == 196290)
        {
            std::list<HostileReference*> threatList = caster->getThreatManager().getThreatList();
            Trinity::Containers::RandomResizeList(threatList, 3);
            for (auto ref : threatList)
            {
                if (auto player = Player::GetPlayer(*caster, ref->getUnitGuid()))
                {
                    Position pos;
                    player->GetRandomNearPosition(pos, 5.0f);
                    caster->CastSpell(pos, 196292, true);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_eye_of_azshara_roiling_storm::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_eye_of_azshara_roiling_storm::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

//196299, 196293
class spell_eye_of_azshara_roiling_storm_script : public SpellScript
{
    PrepareSpellScript(spell_eye_of_azshara_roiling_storm_script);

    void HandleScript(SpellEffIndex /*effectIndex*/)
    {
        auto caster = GetCaster();
        if (!caster || !GetHitDest())
            return;

        Position pos;
        Position tempPos;
        float dist = caster->GetExactDist2d(GetHitDest());
        float angle = caster->GetRelativeAngle(GetHitDest());
        uint8 maxCount = 1;

        if (GetId() == 196299)
        {
            maxCount = 3;
            angle += -M_PI / 6;
        }

        for (uint8 i = 0; i < maxCount; ++i)
        {
            caster->SimplePosXYRelocationByAngle(pos, dist + 10.0f, angle);
            caster->PlayOrphanSpellVisual(GetHitDest()->GetPosition(), {0.f, 0.f, 0.0f}, pos, 39034, 1.75f, ObjectGuid::Empty, true);
            caster->CastSpellDelay(pos, 196294, true, 1750);

            tempPos = pos;
            caster->SimplePosXYRelocationByAngle(pos, dist + 20.0f, angle);
            caster->AddDelayedCombat(1750, [caster, pos, tempPos]() -> void
            {
                if (caster)
                {
                    caster->PlayOrphanSpellVisual(tempPos, {0.f, 0.f, 0.0f}, pos, 39034, 1.5f, ObjectGuid::Empty, true);
                    caster->CastSpellDelay(pos, 196294, true, 1500);
                }
            });

            angle += M_PI / 6;
        }
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_eye_of_azshara_roiling_storm_script::HandleScript, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_boss_serpentrix()
{
    RegisterCreatureAI(boss_serpentrix);
    RegisterCreatureAI(npc_serpentrix_hydras);
    RegisterCreatureAI(npc_eye_of_azshara_stormwake_hydra);
    RegisterCreatureAI(npc_eye_of_azshara_cove_seagull);
    RegisterSpellScript(spell_serpentrix_submerge_teleport);
    RegisterAuraScript(spell_eye_of_azshara_armorshell);
    RegisterAuraScript(spell_eye_of_azshara_vile_blood);
    RegisterAuraScript(spell_eye_of_azshara_roiling_storm);
    RegisterSpellScript(spell_eye_of_azshara_roiling_storm_script);
}