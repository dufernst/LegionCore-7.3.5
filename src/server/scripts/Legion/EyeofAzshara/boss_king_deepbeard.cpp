/*
    Dungeon : Eye of Azshara 100-110
    Encounter: King Deepbeard
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "eye_of_azshara.h"

enum Says
{
    SAY_AGGRO                       = 0,
    SAY_WARN_CALL_THE_SEAS          = 1,
    SAY_CALL_THE_SEAS               = 2,
    SAY_FRENZY                      = 3,
    SAY_GASEOUS_BUBBLES             = 4,
    SAY_QUAKE                       = 5,
    SAY_DEATH                       = 6,
};

enum Spells
{
    SPELL_GROUND_SLAM               = 193093,
    SPELL_GROUND_SLAM_MISSILE       = 193056,
    SPELL_GASEOUS_BUBBLES           = 193018,
    SPELL_GASEOUS_BUBBLES_EXPLOSE   = 193047,
    SPELL_QUAKE                     = 193152,
    SPELL_QUAKE_VISAL               = 193175,
    SPELL_QUAKE_DMG                 = 193159,
    SPELL_CALL_THE_SEAS             = 193051,
    SPELL_FRENZY                    = 197550,

    //Mythic
    SPELL_AFTERSHOCK                = 193167,

    //Christmas
    SPELL_CHRISTMAS_CAP             = 220862
};

enum eEvents
{
    EVENT_GROUND_SLAM           = 1,
    EVENT_GASEOUS_BUBBLES       = 2,
    EVENT_QUAKE                 = 3,
    EVENT_CALL_THE_SEAS         = 4
};

//91797
class boss_king_deepbeard : public CreatureScript
{
public:
    boss_king_deepbeard() : CreatureScript("boss_king_deepbeard") {}

    struct boss_king_deepbeardAI : public BossAI
    {
        boss_king_deepbeardAI(Creature* creature) : BossAI(creature, DATA_DEEPBEARD) {}

        bool lowHp = false;
        uint16 changeEnergyTimer = 0;

        void Reset() override
        {
            _Reset();
            me->SetPower(POWER_ENERGY, 30);
            changeEnergyTimer = 1000;
            lowHp = false;

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
            Talk(SAY_AGGRO);
            _EnterCombat();

            events.RescheduleEvent(EVENT_GROUND_SLAM, 7000);
            events.RescheduleEvent(EVENT_GASEOUS_BUBBLES, 11000);
            events.RescheduleEvent(EVENT_CALL_THE_SEAS, 20000);
        }

        void EnterEvadeMode()
        {
            DisableRotation(false, false);
            BossAI::EnterEvadeMode();
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPct(31) && !lowHp)
            {
                lowHp = true;
                DoCast(me, SPELL_FRENZY, true);
                Talk(SAY_FRENZY);
            }
        }

        void SpellFinishCast(const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_GROUND_SLAM)
                DisableRotation(false);
        }

        void DisableRotation(bool disable, bool delay = true)
        {
            if (disable)
            {
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_CANNOT_TURN);
            }
            else
            {
                if (delay)
                {
                    me->AddDelayedEvent(1000, [this] () -> void
                    {
                        if (me)
                        {
                            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_CANNOT_TURN);
                        }
                    });
                }
                else
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                    me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_CANNOT_TURN);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (changeEnergyTimer)
            {
                if (changeEnergyTimer <= diff)
                {
                    changeEnergyTimer = 1000;
                    if (me->GetPower(POWER_ENERGY) < 100)
                    {
                        me->SetPower(POWER_ENERGY, me->GetPower(POWER_ENERGY) + 5);
                    }
                    else
                    {
                        changeEnergyTimer = 0;
                        events.RescheduleEvent(EVENT_QUAKE, 0);
                    }
                }
                else changeEnergyTimer -= diff;
            }

            events.Update(diff);

            if (CheckHomeDistToEvade(diff, 40.0f, -3454.89f, 4158.99f, 29.18f))
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_GROUND_SLAM:
                        DisableRotation(true);
                        DoCastVictim(SPELL_GROUND_SLAM);
                        DoCast(me, SPELL_GROUND_SLAM_MISSILE, true);
                        events.RescheduleEvent(EVENT_GROUND_SLAM, 18000);
                        break;
                    case EVENT_GASEOUS_BUBBLES:
                        Talk(SAY_GASEOUS_BUBBLES);
                        DoCast(SPELL_GASEOUS_BUBBLES);
                        events.RescheduleEvent(EVENT_GASEOUS_BUBBLES, 33000);
                        break;
                    case EVENT_QUAKE:
                        Talk(SAY_QUAKE);
                        DoCast(SPELL_QUAKE);
                        changeEnergyTimer = 2000;
                        break;
                    case EVENT_CALL_THE_SEAS:
                        Talk(SAY_WARN_CALL_THE_SEAS);
                        Talk(SAY_CALL_THE_SEAS);
                        DoCast(SPELL_CALL_THE_SEAS);
                        events.RescheduleEvent(EVENT_CALL_THE_SEAS, 30000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_king_deepbeardAI (creature);
    }
};

//97916
class npc_deepbeard_quake : public CreatureScript
{
public:
    npc_deepbeard_quake() : CreatureScript("npc_deepbeard_quake") {}

    struct npc_deepbeard_quakeAI : public ScriptedAI
    {
        npc_deepbeard_quakeAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
            instance = me->GetInstanceScript();
        }

        EventMap events;
        InstanceScript* instance;
        uint32 checkTimer = 0;

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            events.RescheduleEvent(EVENT_1, 200);
            checkTimer = 1000;
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_QUAKE)
                AfterShock();
        }

        void AfterShock()
        {
            //Mythic
            if (GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE)
                me->CastSpell(me, SPELL_AFTERSHOCK, true);
        }

        void UpdateAI(uint32 diff) override
        {
            if (checkTimer)
            {
                if (checkTimer <= diff)
                {
                    if (instance->GetBossState(DATA_DEEPBEARD) != IN_PROGRESS)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    checkTimer = 1000;
                }
                else
                    checkTimer -= diff;
            }

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        DoCast(me, SPELL_QUAKE_VISAL, true);
                        me->CastSpell(me, SPELL_QUAKE_DMG);
                        AfterShock();
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_deepbeard_quakeAI(creature);
    }
};

//97844
class npc_deepbeard_call_the_seas : public CreatureScript
{
public:
    npc_deepbeard_call_the_seas() : CreatureScript("npc_deepbeard_call_the_seas") {}

    struct npc_deepbeard_call_the_seasAI : public ScriptedAI
    {
        npc_deepbeard_call_the_seasAI(Creature* creature) : ScriptedAI(creature) 
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        bool moveActive;
        uint16 randomMoveTimer;
        uint16 checkBossStateTimer;

        void Reset() override
        {
            moveActive = true;
            randomMoveTimer = 1000;
            checkBossStateTimer = 1000;
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            moveActive = true;
        }

        void UpdateAI(uint32 diff) override
        {
            if (moveActive)
                if (randomMoveTimer <= diff)
                {
                    moveActive = false;
                    randomMoveTimer = 500;
                    float angle = (float)rand_norm() * static_cast<float>(2 * M_PI);
                    float distance = 20 * (float)rand_norm();
                    float x = me->GetHomePosition().GetPositionX() + distance * std::cos(angle);
                    float y = me->GetHomePosition().GetPositionY() + distance * std::sin(angle);
                    float z = me->GetHomePosition().GetPositionZ();
                    Trinity::NormalizeMapCoord(x);
                    Trinity::NormalizeMapCoord(y);
    
                    me->GetMotionMaster()->MovePoint(1, x, y, z);
                }
                else randomMoveTimer -= diff;

            if (checkBossStateTimer <= diff)
            {
                if (instance->GetBossState(DATA_DEEPBEARD) != IN_PROGRESS)
                    if (me->HasAura(193054))
                        me->RemoveAurasDueToSpell(193054); //CALL_THE_SEAS
                checkBossStateTimer = 1000;
            }
            else
                checkBossStateTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_deepbeard_call_the_seasAI(creature);
    }
};

//193018
class spell_deepbeard_gaseous_explosion : public AuraScript
{
    PrepareAuraScript(spell_deepbeard_gaseous_explosion);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
    {
        if (auto instance = GetCaster()->GetInstanceScript())
            if (auto deepbeard = instance->instance->GetCreature(instance->GetGuidData(NPC_KING_DEEPBEARD)))
            {
                if (AuraEffect const* challengeAurEff = deepbeard->GetAuraEffect(206150, EFFECT_1))
                    AddPct(amount, challengeAurEff->GetAmount());
            }
    }

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (!GetCaster() || !GetTarget())
            return;

        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            if (GetTarget()->GetMap()->GetDifficultyID() != DIFFICULTY_LFR && GetTarget()->GetMap()->GetDifficultyID() != DIFFICULTY_NORMAL)
            {
                float damage = 0.0f;

                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_GASEOUS_BUBBLES_EXPLOSE))
                    damage = spellInfo->GetEffect(EFFECT_0, GetCaster()->GetSpawnMode())->CalcValue();

                damage += aurEff->GetAmount();
                GetTarget()->CastCustomSpell(GetTarget(), SPELL_GASEOUS_BUBBLES_EXPLOSE, &damage, 0, 0, true);
            }
            else
                GetTarget()->CastSpell(GetTarget(), SPELL_GASEOUS_BUBBLES_EXPLOSE, true);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_deepbeard_gaseous_explosion::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectRemove += AuraEffectRemoveFn(spell_deepbeard_gaseous_explosion::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_boss_king_deepbeard()
{
    new boss_king_deepbeard();
    new npc_deepbeard_quake();
    new npc_deepbeard_call_the_seas();
    RegisterAuraScript(spell_deepbeard_gaseous_explosion);
}