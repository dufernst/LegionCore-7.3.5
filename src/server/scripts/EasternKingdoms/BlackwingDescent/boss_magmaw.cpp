#include "Spell.h"
#include "blackwing_descent.h"

enum Spells
{
    SPELL_BERSERK                       = 26667,
    SPELL_LAVA_SPEW                     = 77839,
    SPELL_MAGMA_SPLIT_1                 = 78359,
    SPELL_MAGMA_SPLIT_2                 = 78068,
    SPELL_MANGLE0                       = 78412,
    SPELL_MANGLE                        = 89773,
    SPELL_MOLTEN_TANTRUM                = 78403,
    SPELL_PILLAR_OF_FLAME_0             = 78017,
    SPELL_PILLAR_OF_FLAME_1             = 77970,
    SPELL_SWENTERING_ARMOR              = 78199,
    SPELL_MANGLED_LIFELESS              = 78362,
    SPELL_IGNITION_DUMMY                = 92128,
    SPELL_MASSIVE_CRASH                 = 88287,
    SPELL_MASSIVE_CRASH_DUMMY           = 88253,

    SPELL_PARASITIC_INFECTION           = 78941,
    SPELL_PARASITIC_INFECTION_DMG       = 78937,
    SPELL_PARASITIC_INFECTION_0         = 78097,

    SPELL_EMOTE_LAVA_SPLASH             = 79461,

    SPELL_POINT_OF_VULNERABILITY        = 79011,

    SPELL_LAUNCH_HOOK_1                 = 77917,
    SPELL_LAUNCH_HOOK_2                 = 77941,

    SPELL_FIERY_SLASH                   = 92144,
    SPELL_ARMAGEDDON                    = 92177,
};

enum Adds
{
    NPC_EXPOSED_HEAD                    = 42347,
    NPC_PILLAR_OF_FLAME                 = 41843,
    NPC_LAVA_PARASITE_1                 = 42321,
    NPC_MAGMAW_PINCER_1                 = 41620,
    NPC_MAGMAW_PINCER_2                 = 41789,
    NPC_MAGMAW_SPIKE_STALKER            = 41767,
    NPC_IGNITION_TRIGGER                = 49447,
    NPC_LAVA_PARASITE_2                 = 41806,
    NPC_BLAZING_BONE_CONSTRUCT          = 49416,
    NPC_MASSIVE_CRASH                   = 47330,
    NPC_ROOM_STALKER                    = 47196,
};

enum Events
{
    EVENT_LAVA_SPEW             = 1,
    EVENT_MAGMA_SPLIT           = 2,
    EVENT_MANGLE                = 3,
    EVENT_PILLAR_OF_FLAME       = 4,
    EVENT_MELEE_CHECK           = 5,
    EVENT_BERSERK               = 7,
    EVENT_HEAD_START            = 8,
    EVENT_HEAD_END              = 9,
    EVENT_FIERY_SLASH           = 10,
};

enum Actions
{
    ACTION_HEAD_START       = 1,
    ACTION_HEAD_EVENT       = 2,
    ACTION_HEAD_END         = 3,
    ACTION_HOOKED           = 4,
};

enum OtherData
{
    DATA_PASSENGERS         = 1,
};

Position const magmawmassivecrashPos[2][21] =
{
    {
        {-355.258f, -66.156f,  215.363f, 3.32963f},
        {-349.104f, -57.5792f, 214.837f, 3.35319f},
        {-355.983f, -79.9935f, 213.749f, 3.91082f},
        {-348.35f,  -71.9141f, 213.26f,  3.96973f},
        {-341.228f, -63.2021f, 212.833f, 3.96973f},
        {-335.636f, -53.1296f, 212.332f, 5.80755f},
        {-344.959f, -47.9226f, 212.061f, 6.07851f},
        {-354.565f, -47.2949f, 213.04f,  3.04373f},
        {-350.721f, -88.5883f, 213.92f,  3.97443f},
        {-342.783f, -80.5202f, 213.868f, 3.93124f},
        {-335.879f, -72.941f,  212.87f,  4.06083f},
        {-330.037f, -64.3112f, 212.393f, 4.14329f},
        {-324.021f, -54.4161f, 211.863f, 4.19434f},
        {-317.603f, -44.6127f, 211.952f, 0.96793f},
        {-329.399f, -43.7365f, 211.748f, 6.02982f},
        {-341.139f, -38.8838f, 211.426f, 6.12014f},
        {-350.813f, -26.2997f, 211.345f, 2.15388f},
        {-352.253f, -37.0172f, 211.603f, 2.95734f},
        {-338.237f, -27.1878f, 211.154f, 3.06337f},
        {-327.528f, -32.3648f, 211.394f, 3.06337f},
        {-316.344f, -33.1654f, 211.428f, 3.17725f}
    },
    { 
        {-313.627f, -72.6881f, 213.266f, 1.00637f},
        {-319.881f, -82.0775f, 213.552f, 4.27756f},
        {-306.833f, -83.4444f, 213.633f, 4.36788f},
        {-302.725f, -74.0836f, 213.345f, 4.69382f},
        {-304.055f, -63.3109f, 212.826f, 4.57601f},
        {-300.848f, -54.0836f, 212.39f,  4.18332f},
        {-308.972f, -52.744f,  212.326f, 5.62846f},
        {-315.607f, -58.0178f, 212.578f, 1.02995f},
        {-320.357f, -65.927f,  212.802f, 1.0378f},
        {-326.56f,  -74.2884f, 213.145f, 0.841454f},
        {-333.371f, -83.2f,    213.706f, 0.566564f},
        {-343.042f, -91.8511f, 213.916f, 3.12304f},
        {-334.444f, -92.7097f, 213.903f, 3.06413f},
        {-324.424f, -93.0766f, 213.909f, 2.98166f},
        {-315.574f, -93.3725f, 213.924f, 3.12696f},
        {-304.845f, -93.5295f, 213.919f, 3.10733f},
        {-296.142f, -88.6009f, 214.03f,  4.72919f},
        {-292.752f, -78.8147f, 213.567f, 4.71348f},
        {-295.368f, -68.4218f, 213.065f, 4.71741f},
        {-293.046f, -56.9843f, 212.531f, 4.62709f},
        {-295.83f,  -46.4565f, 212.04f,  1.27344f}
    }
};

Position const magmawnefariusspawnPos = {-340.92f, -67.10f, 225.49f, 4.26f}; 

Position const magmawpincerPos[2] = 
{
    {-317.28f, -51.07f, 212.24f, 0.0f},
    {-310.37f, -55.48f, 212.46f, 0.0f}
};

Position const magmawspikestalkerPos = {-314.72f, -48.44f, 212.10f, 0.0f};

class boss_magmaw : public CreatureScript
{
public:
    boss_magmaw() : CreatureScript("boss_magmaw") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_magmawAI(pCreature);
    }

    struct boss_magmawAI : public Scripted_NoMovementAI
    {
        boss_magmawAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature), summons(me)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        SummonList summons;
        bool bGrounded;
        bool bShadowBreath;

        void Reset()
        {
            bGrounded = false;
            bShadowBreath = false;
            summons.DespawnAll();
            events.Reset();

            me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 10);
            me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 10);

            if (!instance)
                return;
            instance->SetBossState(DATA_MAGMAW, NOT_STARTED);
        }

        void EnterCombat(Unit* attacker)
        {
            events.RescheduleEvent(EVENT_BERSERK, 6 * MINUTE * IN_MILLISECONDS);
            events.RescheduleEvent(EVENT_MELEE_CHECK, 6000);
            events.RescheduleEvent(EVENT_LAVA_SPEW, urand(3000, 7000));
            events.RescheduleEvent(EVENT_PILLAR_OF_FLAME, urand(10000, 15000));
            events.RescheduleEvent(EVENT_MAGMA_SPLIT, urand(15000, 20000));
            //events.RescheduleEvent(EVENT_MANGLE, 65000);
            //events.RescheduleEvent(EVENT_HEAD_START, 5000);

            if (IsHeroic())
                if (Creature* pNefarius = me->SummonCreature(NPC_LORD_VICTOR_NEFARIUS_HEROIC, magmawnefariusspawnPos))
                {
                    pNefarius->AI()->DoAction(ACTION_MAGMAW_INTRO);
                    pNefarius->AI()->DoAction(ACTION_BLAZING_INFERNO);
                }
                DoZoneInCombat();
                instance->SetBossState(DATA_MAGMAW, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (!instance)
                return;
            summons.DespawnAll();
            instance->SetBossState(DATA_MAGMAW, DONE);
            instance->SaveToDB();
            if (Creature* pNefarius = me->SummonCreature(NPC_LORD_VICTOR_NEFARIUS_HEROIC, 
                me->GetPositionX(),
                me->GetPositionY(),
                me->GetPositionZ(),
                0.0f))
                pNefarius->AI()->DoAction(ACTION_MAGMAW_DEATH);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            if (me->isInCombat())
                summon->SetInCombatWithZone();
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (attacker->GetGUID() == me->GetGUID())
                damage = 0;
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void UpdateAI(uint32 diff)
        {
            if (!instance || !UpdateVictim())
                return;

            if (IsHeroic() && me->HealthBelowPct(30) && !bShadowBreath)
            {
                bShadowBreath = true;
                if (Creature* pNefarius = me->FindNearestCreature(NPC_LORD_VICTOR_NEFARIUS_HEROIC, 200.0f))
                    pNefarius->AI()->DoAction(ACTION_SHADOW_BREATH);
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    /*case EVENT_HEAD_START:
                    SetGrounded(true, 0);
                    if (Creature* pMagmawhead = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW_HEAD)))
                    pMagmawhead->AI()->DoAction(ACTION_HEAD_START);
                    events.RescheduleEvent(EVENT_HEAD_END, 20000);
                    break;
                    case EVENT_HEAD_END:
                    if (Creature* pMagmawhead = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW_HEAD)))
                    pMagmawhead->AI()->DoAction(ACTION_HEAD_END);
                    SetGrounded(false, 0);
                    events.RescheduleEvent(EVENT_MELEE_CHECK, 6000);
                    events.RescheduleEvent(EVENT_LAVA_SPEW, urand(3000, 7000));
                    events.RescheduleEvent(EVENT_PILLAR_OF_FLAME, urand(10000, 15000));
                    events.RescheduleEvent(EVENT_MAGMA_SPLIT, urand(15000, 20000));
                    break;*/
                case EVENT_BERSERK:
                    DoCast(me, SPELL_BERSERK);
                    break;
                case EVENT_MELEE_CHECK:
                    if (!me->IsWithinMeleeRange(me->getVictim()))
                        DoCast(me, SPELL_MAGMA_SPLIT_2);
                    events.RescheduleEvent(EVENT_MELEE_CHECK, 1000);
                    break;
                case EVENT_MAGMA_SPLIT:
                    me->CastCustomSpell(SPELL_MAGMA_SPLIT_1, SPELLVALUE_MAX_TARGETS, RAID_MODE(3, 8, 3, 8), 0, false); 
                    events.RescheduleEvent(EVENT_MAGMA_SPLIT, urand(15000, 20000));
                    break;
                case EVENT_LAVA_SPEW:
                    DoCast (me, SPELL_LAVA_SPEW);
                    events.RescheduleEvent(EVENT_LAVA_SPEW, 22000);
                    break;
                case EVENT_PILLAR_OF_FLAME:
                    Unit* target;
                    target = SelectTarget(SELECT_TARGET_RANDOM, 1, -20.0f, true);
                    if (!target)
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
                    if (!target)
                        break;
                    me->SummonCreature(NPC_PILLAR_OF_FLAME,
                        target->GetPositionX(),
                        target->GetPositionY(),
                        target->GetPositionZ(),
                        0.0f, TEMPSUMMON_TIMED_DESPAWN, 8000);
                    events.RescheduleEvent(EVENT_PILLAR_OF_FLAME, urand(35000, 45000));
                    break;
                case EVENT_MANGLE:
                    DoCast(me->getVictim(), SPELL_MANGLE);
                    //events.CancelEvent(EVENT_MELEE_CHECK);
                    //events.CancelEvent(EVENT_LAVA_SPEW);
                    //events.CancelEvent(EVENT_PILLAR_OF_FLAME);
                    //events.CancelEvent(EVENT_MAGMA_SPLIT);
                    events.RescheduleEvent(EVENT_MANGLE, 95000);
                    //events.RescheduleEvent(EVENT_HEAD_START, 12000);
                    break;
                }
            }
            if (me->getVictim())
                if (!me->getVictim()->HasAura(SPELL_MANGLE) && !bGrounded)
                    DoMeleeAttackIfReady();
        }
    private:

        void SetGrounded(bool apply, uint32 mode)
        {
            if (apply)
            {
                bGrounded = true;
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_PASSIVE);
                me->SetControlled(true, UNIT_STATE_STUNNED);
                me->AddAura(mode = 0? 45799: 45798, me);
                //45800
                //45798
                //91390
                //45799
            }
            else
            {
                bGrounded = false;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetControlled(false, UNIT_STATE_STUNNED);
                me->RemoveAurasDueToSpell(mode = 0? 45799: 45798);
            }
        }

        void _ChangeSpell(const uint8 mode)
        {
            if (mode == 1)
            {
                if (SpellInfo* spell = GET_SPELL(SPELL_MASSIVE_CRASH_DUMMY))
                {
                    spell->Effects[0]->Effect = 0;
                    spell->Misc.Duration.Duration = 30 * IN_MILLISECONDS;
                }
            }
            else
            {
                if (SpellInfo* spell = GET_SPELL(SPELL_MASSIVE_CRASH_DUMMY))
                {
                    spell->Effects[0]->Effect = 3;
                    spell->Misc.Duration.Duration = 6 * IN_MILLISECONDS;
                }
            }
        }
    };
};

class npc_magmaw_head : public CreatureScript
{
public:
    npc_magmaw_head() : CreatureScript("npc_magmaw_head") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_magmaw_headAI(pCreature);
    }

    struct npc_magmaw_headAI : public ScriptedAI
    {
        npc_magmaw_headAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
            me->SetReactState(REACT_PASSIVE);
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset()
        {
            if (!instance)
                return;
            if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->ClearUnitState(UNIT_STATE_ONVEHICLE);
        }

        void DoAction(const int32 action)
        {
            if (!instance)
                return;
            switch (action)
            {
            case ACTION_HEAD_START:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->ClearUnitState(UNIT_STATE_ONVEHICLE);
                if (Creature* pMagmaw = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW)))
                {
                    me->SetMaxHealth(pMagmaw->GetMaxHealth());
                    me->SetHealth(pMagmaw->GetHealth());
                }
                me->setFaction(16);
                me->AddAura(SPELL_POINT_OF_VULNERABILITY, me);
                break;
            case ACTION_HEAD_END:
                me->CombatStop();
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (!instance)
                return;

            if (Creature* pMagmaw = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW)))
            {
                if (me->GetHealth() > damage && pMagmaw->GetHealth() > damage)
                    pMagmaw->SetHealth(pMagmaw->GetHealth() - damage);
                else
                    damage = 0;
            }
        }

        void UpdateAI(uint32 diff)
        {            
        }
    };
};

class npc_lava_parasite : public CreatureScript
{
public:
    npc_lava_parasite() : CreatureScript("npc_lava_parasite") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_lava_parasiteAI(pCreature);
    }

    struct npc_lava_parasiteAI : public ScriptedAI
    {
        npc_lava_parasiteAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetSpeed(MOVE_WALK, 0.3f);
            me->SetSpeed(MOVE_RUN, 0.3f);
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset()
        {
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (instance && instance->GetBossState(DATA_MAGMAW) != IN_PROGRESS)
                me->DespawnOrUnsummon();

            if (me->getVictim())
            {
                if ((me->GetDistance(me->getVictim()) <= 4.0f))
                {
                    if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
                    {
                        me->getVictim()->CastSpell(me->getVictim(), SPELL_PARASITIC_INFECTION, true);
                        me->DespawnOrUnsummon();
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

class npc_pillar_of_flame : public CreatureScript
{
public:
    npc_pillar_of_flame() : CreatureScript("npc_pillar_of_flame") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_pillar_of_flameAI(pCreature);
    }

    struct npc_pillar_of_flameAI : public ScriptedAI
    {
        npc_pillar_of_flameAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 uiFlameTimer;
        bool bFlame;

        void Reset()
        {
            uiFlameTimer = 4000;
            bFlame = false;
            DoCast(me, SPELL_PILLAR_OF_FLAME_0);
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon();
        }

        void JustSummoned(Creature* summon)
        {
            if (!instance)
                return;

            if (Creature* pMagmaw = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_MAGMAW)))
                if (pMagmaw->isInCombat())
                    summon->SetInCombatWithZone();
        }

        void UpdateAI(uint32 diff)
        {
            if (uiFlameTimer <= diff && !bFlame)
            {
                bFlame = true;
                me->RemoveAurasDueToSpell(SPELL_PILLAR_OF_FLAME_0);
                DoCast(me, SPELL_PILLAR_OF_FLAME_1);
            }
            else
                uiFlameTimer -= diff;
        }
    };
};

class spell_magmaw_mangle : public SpellScriptLoader
{
public:
    spell_magmaw_mangle() : SpellScriptLoader("spell_magmaw_mangle") { }

    class spell_magmaw_mangle_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_magmaw_mangle_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (!GetTarget() && GetCaster())
                return;

            GetTarget()->SetControlled(true, UNIT_STATE_STUNNED);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (!GetTarget() && GetCaster())
                return;

            GetTarget()->SetControlled(false, UNIT_STATE_STUNNED);
            GetTarget()->CastSpell(GetTarget() , SPELL_SWENTERING_ARMOR, true);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_magmaw_mangle_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_magmaw_mangle_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_magmaw_mangle_AuraScript();
    }
};

class spell_parasitic_infection : public SpellScriptLoader
{
public:
    spell_parasitic_infection() : SpellScriptLoader("spell_parasitic_infection") { }

    class spell_parasitic_infection_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_parasitic_infection_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (!GetTarget())
                return;

            GetTarget()->CastSpell(GetTarget(), SPELL_PARASITIC_INFECTION_DMG, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_parasitic_infection_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_parasitic_infection_AuraScript();
    }
};

void AddSC_boss_magmaw()
{
    new boss_magmaw();
    new npc_magmaw_head();
    new npc_pillar_of_flame();
    new npc_lava_parasite();
    new spell_magmaw_mangle();
    new spell_parasitic_infection();
}
