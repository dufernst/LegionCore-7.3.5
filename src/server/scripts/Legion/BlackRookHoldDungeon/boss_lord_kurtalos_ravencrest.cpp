/*
    Dungeon : Black Rook Hold Dungeon 100-110
    Encounter: Lord Kurtalos Ravencrest
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "black_rook_hold_dungeon.h"

enum Says
{
    //Kurtalos
    SAY_AGGRO                   = 0,

    //Latosius
    SAY_DARK_BLAST              = 0,
    SAY_WARN_DREADLORDS_GUILE   = 1,
    SAY_DREADLORDS_GUILE        = 2,
    SAY_CLOUD_OF_HYPNOSIS       = 3,
};

enum Spells
{
//Kurtalos
    //Phase 1
    SPELL_UNERRING_SHEAR        = 198635,
    SPELL_WHIRLING_BLADE        = 198641,
    SPELL_WHIRLING_BLADE_AT     = 198782,
    SPELL_SUICIDE               = 117624,

    //Phase 2
    SPELL_KURTALOS_GHOST_VISUAL = 199243,
    SPELL_LEGACY_RAVENCREST     = 199368,

//Latosius
    //Phase 1
    SPELL_TELEPORT_1            = 198835,
    SPELL_TELEPORT_2            = 199058,
    SPELL_SHADOW_BOLT           = 198833,
    SPELL_DARK_BLAST            = 198820,

    //Phase 2
    SPELL_CONVERSATION          = 199239, // Dreadlord Conversation
    SPELL_SAP_SOUL              = 198961,
    SPELL_TRANSFORM             = 199064,
    SPELL_SHADOW_BOLT_VOLLEY    = 202019,
    SPELL_CLOUD_OF_HYPNOSIS     = 199143,
    SPELL_DREADLORDS_GUILE      = 199193,
    SPELL_BREAK_PLR_TARGETTING  = 140562,
    SPELL_DARK_OBLITERATION     = 199567,
    SPELL_DARK_OBLITERATION_AT  = 241672,

    //Heroic
    SPELL_STINGING_SWARM        = 201733,
    SPELL_ITCHY_STUN            = 199168,
};

enum eEvents
{
//Kurtalos
    //Phase 1
    EVENT_UNERRING_SHEAR        = 1,
    EVENT_WHIRLING_BLADE        = 2,

//Latosius
    //Phase 1
    EVENT_LATOSIUS_TP           = 1,
    EVENT_SHADOW_BOLT           = 2,
    EVENT_DARK_BLAST            = 3,

    //Phase 2
    EVENT_LATOSIUS_TP_2         = 4,
    EVENT_SAP_SOUL              = 5,
    EVENT_TRANSFORM             = 6,
    EVENT_SHADOW_BOLT_VOLLEY    = 7,
    EVENT_CLOUD_OF_HYPNOSIS     = 8,
    EVENT_DREADLORDS_GUILE      = 9,
    EVENT_SUM_IMAGE             = 10,
    EVENT_IMAGE_END             = 11,
    EVENT_STINGING_SWARM        = 12, //Heroic
};

enum ePhase
{
    PHASE_1     = 0,
    PHASE_2     = 1,
};

Position const tpPos[12] =
{
    {3169.09f, 7432.38f, 271.60f, 5.42f},
    {3165.53f, 7421.75f, 271.60f, 5.85f},
    {3158.29f, 7414.00f, 271.60f, 0.0f },
    {3164.16f, 7395.63f, 271.60f, 0.62f},
    {3173.76f, 7387.44f, 271.60f, 1.08f},
    {3184.47f, 7383.66f, 271.60f, 1.49f},
    {3200.61f, 7390.41f, 271.60f, 2.14f},
    {3205.70f, 7400.89f, 271.60f, 2.61f},
    {3210.50f, 7409.22f, 271.60f, 3.02f},
    {3204.84f, 7427.28f, 271.60f, 3.84f},
    {3195.91f, 7430.39f, 271.60f, 4.24f},
    {3186.87f, 7436.26f, 271.60f, 4.70f}
};

//98970
struct boss_latosius : public BossAI
{
    boss_latosius(Creature* creature) : BossAI(creature, DATA_KURTALOS) {}

    bool secondPhase = false;
    uint8 imageSumCount = 0;

    void Reset() override
    {
        if (instance->GetData(DATA_KURTALOS_STATE) != PHASE_1)
            return;

        _Reset();
        secondPhase = false;
        SetCombatMovement(false);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1);
        SetEquipmentSlots(false, 65483);
        me->SummonCreature(NPC_KURTALOS, 3191.72f, 7423.69f, 270.462f, 0.57f);
    }

    void EnterCombat(Unit* /*who*/) override
    {
        if (instance->GetData(DATA_KURTALOS_STATE) != PHASE_1)
            return;

        _EnterCombat();
        if (auto kurtalos = me->FindNearestCreature(NPC_KURTALOS, 30.0f))
            kurtalos->AI()->DoZoneInCombat(kurtalos, 100.0f);

        events.RescheduleEvent(EVENT_LATOSIUS_TP, 0);
        events.RescheduleEvent(EVENT_DARK_BLAST, 10000);
    }

    void EnterEvadeMode() override
    {
        if (instance->GetData(DATA_KURTALOS_STATE) != PHASE_1)
            instance->SetData(DATA_KURTALOS_STATE, PHASE_1);

        BossAI::EnterEvadeMode();
        me->UpdateEntry(NPC_LATOSIUS);
        me->SetDisplayId(me->GetNativeDisplayId());
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
        me->RemoveAurasDueToSpell(SPELL_TRANSFORM);
        me->NearTeleportTo(me->GetHomePosition());
        me->ClearUnitState(UNIT_STATE_EVADE);
        me->RemoveAllConversationObjects();
        RemoveAuras();
    }

    bool CheckAura()
    {
        auto const& players = me->GetMap()->GetPlayers();
        if (!players.isEmpty())
            for (auto const& itr : players)
                if (Player* player = itr.getSource())
                {
                    if (!player->HasAura(213404))
                        return false;
                }
        
        return true;
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        RemoveAuras();
        
        // for quest need "kill"
        auto const& players = me->GetMap()->GetPlayers();
        if (!players.isEmpty())
        {
            for (auto const& itr : players)
            {
                if (Player* player = itr.getSource())
                    player->KilledMonsterCredit(99611);
            }
            instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER, 1835);
        }
        if (CheckAura())
        {
            instance->DoAddAuraOnPlayers(213407);
            instance->DoRemoveAurasDueToSpellOnPlayers(213404);
        }
    }

    void RemoveAuras()
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNERRING_SHEAR);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LEGACY_RAVENCREST);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_STINGING_SWARM);
    }

    void DoAction(int32 const action) override
    {
        if (action == ACTION_1 && instance->GetData(DATA_KURTALOS_STATE) != PHASE_2)
        {
            instance->SetData(DATA_KURTALOS_STATE, PHASE_2);
            events.Reset();
            events.RescheduleEvent(EVENT_LATOSIUS_TP_2, 1000);
        }
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_DREADLORDS_GUILE:
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
                me->SetDisplayId(11686);
                DoCast(me, SPELL_BREAK_PLR_TARGETTING, true);
                break;
            default:
                break;
        }
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_SAP_SOUL)
            events.RescheduleEvent(EVENT_TRANSFORM, 8000);

        if (spell->Id == SPELL_STINGING_SWARM)
        {
            if (auto sum = target->SummonCreature(NPC_STINGING_SWARM, target->GetPosition()))
                sum->EnterVehicle(target);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (secondPhase && CheckHomeDistToEvade(diff, 41.0f))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_LATOSIUS_TP:
                    DoCast(me, SPELL_TELEPORT_1, true);
                    events.RescheduleEvent(EVENT_SHADOW_BOLT, 500);
                    break;
                case EVENT_SHADOW_BOLT:
                    DoCast(SPELL_SHADOW_BOLT);
                    events.RescheduleEvent(EVENT_SHADOW_BOLT, 3000);
                    break;
                case EVENT_DARK_BLAST:
                    Talk(SAY_DARK_BLAST);
                    me->CastSpellDelay(me, SPELL_DARK_OBLITERATION_AT, true, 100);
                    me->CastSpellDelay(me, SPELL_DARK_BLAST, false, 500);
                    events.CancelEvent(EVENT_SHADOW_BOLT);
                    events.RescheduleEvent(EVENT_LATOSIUS_TP, 5000);
                    events.RescheduleEvent(EVENT_DARK_BLAST, 18000);
                    break;
                case EVENT_LATOSIUS_TP_2:
                    DoCast(me, SPELL_TELEPORT_2, true);
                    DoCast(me, SPELL_CONVERSATION, true);
                    events.RescheduleEvent(EVENT_SAP_SOUL, 1000);
                    break;
                case EVENT_SAP_SOUL:
                    DoCast(me, SPELL_SAP_SOUL, true);
                    break;
                case EVENT_TRANSFORM:
                    secondPhase = true;
                    me->UpdateEntry(NPC_DANTALIONAX);
                    me->SetHealth(me->GetMaxHealth());
                    DoZoneInCombat(me, 100.0f);
                    me->StopAttack();
                    me->SetReactState(REACT_AGGRESSIVE, 3000);
                    DoCast(me, SPELL_TRANSFORM, true);
                    SetEquipmentSlots(false, 0, 0, 0);
                    SetCombatMovement(true);
                    events.RescheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, 14500);
                    events.RescheduleEvent(EVENT_CLOUD_OF_HYPNOSIS, 26000);
                    events.RescheduleEvent(EVENT_DREADLORDS_GUILE, 35000);
                    if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
                        events.RescheduleEvent(EVENT_STINGING_SWARM, 19000);
                    break;
                case EVENT_SHADOW_BOLT_VOLLEY:
                    DoCast(SPELL_SHADOW_BOLT_VOLLEY);
                    events.RescheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, 8500);
                    break;
                case EVENT_CLOUD_OF_HYPNOSIS:
                    Talk(SAY_CLOUD_OF_HYPNOSIS);
                    if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(pTarget, SPELL_CLOUD_OF_HYPNOSIS);
                    events.RescheduleEvent(EVENT_CLOUD_OF_HYPNOSIS, 32000);
                    break;
                case EVENT_DREADLORDS_GUILE:
                    me->StopAttack();
                    imageSumCount = 0;
                    Talk(SAY_WARN_DREADLORDS_GUILE);
                    Talk(SAY_DREADLORDS_GUILE);
                    DoCast(SPELL_DREADLORDS_GUILE);
                    for (auto id : {EVENT_SHADOW_BOLT_VOLLEY, EVENT_CLOUD_OF_HYPNOSIS, EVENT_STINGING_SWARM})
                        events.RecalcEventTimer(id, 26000, true);
                    events.RescheduleEvent(EVENT_DREADLORDS_GUILE, 82000);
                    events.RescheduleEvent(EVENT_SUM_IMAGE, 4000);
                    break;
                case EVENT_SUM_IMAGE:
                    me->SummonCreature(NPC_IMAGE_OF_LATOSIUS, tpPos[imageSumCount], TEMPSUMMON_TIMED_DESPAWN, 4000);
                    if (++imageSumCount > 10)
                    {
                        events.RescheduleEvent(EVENT_IMAGE_END, 3000);
                        break;
                    }
                    events.RescheduleEvent(EVENT_SUM_IMAGE, 1600);
                    break;
                case EVENT_IMAGE_END:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetDisplayId(me->GetNativeDisplayId());
                    me->RemoveAurasDueToSpell(SPELL_DREADLORDS_GUILE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                case EVENT_STINGING_SWARM:
                    if (auto target = SelectTarget(SELECT_TARGET_RANDOM, 0, 90.0f, true, -SPELL_STINGING_SWARM))
                        DoCast(target, SPELL_STINGING_SWARM);
                    events.RescheduleEvent(EVENT_STINGING_SWARM, 17000);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//98965
struct npc_kurtalos_ravencrest : public ScriptedAI
{
    npc_kurtalos_ravencrest(Creature* creature) : ScriptedAI(creature) 
    {
        instance = me->GetInstanceScript();
    }

    InstanceScript* instance;
    EventMap events;
    bool secondPhase = false;

    void Reset() override
    {
        secondPhase = false;
    }

    void EnterCombat(Unit* /*who*/) override
    {
        Talk(SAY_AGGRO); //Fiends, you shall never have our world!

        if (me->GetAnyOwner())
            if (auto latosius = me->GetAnyOwner()->ToCreature())
                latosius->AI()->DoZoneInCombat(latosius, 100.0f);

        events.RescheduleEvent(EVENT_UNERRING_SHEAR, 6000);
        events.RescheduleEvent(EVENT_WHIRLING_BLADE, 10000);
    }

    void EnterEvadeMode() override
    {
        if (instance->GetData(DATA_KURTALOS_STATE) != PHASE_1)
            instance->SetData(DATA_KURTALOS_STATE, PHASE_1);

        if (me->GetAnyOwner())
            if (auto latosius = me->GetAnyOwner()->ToCreature())
                latosius->AI()->EnterEvadeMode();

        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (auto owner = me->GetAnyOwner())
            owner->SummonCreature(NPC_SOUL_KURTALOS, me->GetPosition());
    }

    void SpellHitTarget(Unit* target, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_WHIRLING_BLADE)
            if (auto blade = me->SummonCreature(NPC_KURTALOS_BLADE_TRIGGER, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 30000))
            {
                blade->AI()->SetGUID(target->GetGUID());
                blade->GetMotionMaster()->MovePoint(1, target->GetPosition());
            }
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_SAP_SOUL)
            me->CastSpellDelay(me, SPELL_SUICIDE, true, 8000);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (damage >= me->GetHealth() || me->HealthBelowPct(20))
        {
            if (!secondPhase)
            {
                secondPhase = true;
                me->StopAttack();
                events.Reset();
                if (auto latosius = me->GetAnyOwner())
                    latosius->GetAI()->DoAction(ACTION_1);
            }
            if (attacker != me)
                damage = 0;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (CheckHomeDistToEvade(diff, 41.0f))
            return;

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_UNERRING_SHEAR:
                    DoCastVictim(SPELL_UNERRING_SHEAR);
                    events.RescheduleEvent(EVENT_UNERRING_SHEAR, 13000);
                    break;
                case EVENT_WHIRLING_BLADE:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        DoCast(pTarget, SPELL_WHIRLING_BLADE);
                    events.RescheduleEvent(EVENT_WHIRLING_BLADE, 10000);
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

//101054 - Soul, 100861 - Blade Trigger, 101028 - Image of Latosius
struct npc_kurtalos_trigger : public ScriptedAI
{
    npc_kurtalos_trigger(Creature* creature) : ScriptedAI(creature) 
    {
        instance = me->GetInstanceScript();
        me->setFaction(14);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
    }

    InstanceScript* instance;
    EventMap events;
    Position pos[2];

    void Reset() override {}

    void IsSummonedBy(Unit* summoner) override
    {
        if (instance->GetBossState(DATA_KURTALOS) != IN_PROGRESS)
        {
            me->DespawnOrUnsummon();
            return;
        }

        switch(me->GetEntry())
        {
            case NPC_KURTALOS_BLADE_TRIGGER:
                pos[0] = me->GetPosition();
                DoCast(me, SPELL_WHIRLING_BLADE_AT, true);
                break;
            case NPC_SOUL_KURTALOS:
                me->setFaction(35);
                DoCast(me, SPELL_KURTALOS_GHOST_VISUAL, true);
                events.RescheduleEvent(EVENT_3, 24000);
                break;
            case NPC_IMAGE_OF_LATOSIUS:
                me->CastSpellDelay(me, SPELL_DARK_OBLITERATION_AT, true, 100);
                me->CastSpellDelay(me, SPELL_DARK_OBLITERATION, false, 500);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == 1)
            events.RescheduleEvent(EVENT_1, 200);

        if (id == 2)
            events.RescheduleEvent(EVENT_2, 200);
    }

    void SetGUID(ObjectGuid const& guid, int32 /*id*/ = 0) override
    {
        if (auto player = Player::GetPlayer(*me, guid))
            pos[1] = player->GetPosition();
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_1:
                    if (instance->GetBossState(DATA_KURTALOS) != IN_PROGRESS)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    me->GetMotionMaster()->MovePoint(2, pos[0], false);
                    break;
                case EVENT_2:
                    if (instance->GetBossState(DATA_KURTALOS) != IN_PROGRESS)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    me->GetMotionMaster()->MovePoint(1, pos[1], false);
                    break;
                case EVENT_3:
                    if (instance->GetBossState(DATA_KURTALOS) == IN_PROGRESS)
                        me->CastSpell(me, SPELL_LEGACY_RAVENCREST, true);
                    me->DespawnOrUnsummon(2000);
                    break;
            }
        }
    }
};

//101008
struct npc_kurtalos_stinging_swarm : public ScriptedAI
{
    npc_kurtalos_stinging_swarm(Creature* creature) : ScriptedAI(creature)
    {
        instance = me->GetInstanceScript();
        me->SetReactState(REACT_PASSIVE);
    }

    InstanceScript* instance;
    uint16 checkTimer = 0;

    void Reset() override
    {
        checkTimer = 2000;
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (auto summoner = me->GetAnyOwner())
            summoner->RemoveAurasDueToSpell(SPELL_STINGING_SWARM);
    }

    void UpdateAI(uint32 diff) override
    {
        if (checkTimer)
        {
            if (checkTimer <= diff)
            {
                if (instance->GetBossState(DATA_KURTALOS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }
                checkTimer = 2000;
            }
            else
                checkTimer -= diff;
        }
    }
};

//198835
class spell_latosius_random_teleport : public SpellScript
{
    PrepareSpellScript(spell_latosius_random_teleport);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (!GetCaster())
            return;

        uint8 rand = urand(0, 11);
        GetCaster()->NearTeleportTo(tpPos[rand].GetPositionX(), tpPos[rand].GetPositionY(), tpPos[rand].GetPositionZ(), tpPos[rand].GetOrientation());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_latosius_random_teleport::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_TELEPORT_L);
    }
};

//201733
class spell_kurtalos_stinging_swarm : public AuraScript
{
    PrepareAuraScript(spell_kurtalos_stinging_swarm);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (urand(0, 100) < 30)
            GetUnitOwner()->CastSpell(GetUnitOwner(), SPELL_ITCHY_STUN, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_kurtalos_stinging_swarm::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

void AddSC_boss_lord_kurtalos_ravencrest()
{
    RegisterCreatureAI(boss_latosius);
    RegisterCreatureAI(npc_kurtalos_ravencrest);
    RegisterCreatureAI(npc_kurtalos_trigger);
    RegisterCreatureAI(npc_kurtalos_stinging_swarm);
    RegisterSpellScript(spell_latosius_random_teleport);
    RegisterAuraScript(spell_kurtalos_stinging_swarm);
}