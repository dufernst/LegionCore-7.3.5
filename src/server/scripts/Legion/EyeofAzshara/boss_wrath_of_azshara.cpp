/*
    Dungeon : Eye of Azshara 100-110
    Encounter: Wrath of Azshara
    Normal: 100%, Heroic: 100%, Mythic: 100%
*/

#include "eye_of_azshara.h"

enum Says
{
    SAY_START           = 1,
    SAY_AGGRO           = 2,
    SAY_DELUGE          = 3,
    SAY_ARCANE_EMOTE    = 4,
    SAY_ARCANE          = 5,
    SAY_CRY_EMOTE       = 6,
    SAY_CRY             = 7,
    SAY_STORMS          = 8,
    SAY_DEATH           = 9,
};

enum Spells
{
    //Pre-event spells
    SPELL_SURGING_WATERS_AT         = 192632,
    SPELL_DAMAGE_SELF_20PCT         = 193500,
    SPELL_TEMPEST_ATTUNEMENT        = 193491,
    SPELL_STORM_CONDUIT             = 193196,

    SPELL_MYSTIC_TORNADO            = 192680,
    SPELL_MASSIVE_DELUGE            = 192620,
    SPELL_MASSIVE_DELUGE_DMG        = 192619,
    SPELL_ARCANE_BOMB               = 192705,
    SPELL_ARCANE_BOMB_VEH_SEAT      = 192706,
    SPELL_ARCANE_BOMB_VISUAL        = 192711,
    SPELL_ARCANE_BOMB_DMG           = 192708,
    SPELL_CRY_OF_WRATH              = 192985,
    SPELL_RAGING_STORMS             = 192696,
    SPELL_HEAVING_SANDS             = 197164,

    //Cry of Wrath Events
    SPELL_TIDAL_WAVE_PERIODIC_1     = 192940, //Stage_1 - 20s tick
    SPELL_TIDAL_WAVE_PERIODIC_2     = 192941, //Stage_2 - 6s tick
    SPELL_TIDAL_WAVE_SELECT_POINT_1 = 192793,
    SPELL_TIDAL_WAVE_SELECT_POINT_2 = 192797,
    SPELL_TIDAL_WAVE_AT             = 192753,
    SPELL_TIDAL_WAVE_DMG            = 192801,
    SPELL_LIGHTNING_STRIKES_1       = 192990, //7s tick
    SPELL_LIGHTNING_STRIKES_2       = 192989, //2s tick

    //Heroic
    SPELL_CRUSHING_DEPTHS           = 197365,
    SPELL_MAGIC_RESONANCE           = 196665,
    SPELL_FROST_RESONANCE           = 196666,

    //Summons
    SPELL_MYSTIC_TORNADO_AT         = 192673,
};

enum eEvents
{
    EVENT_MYSTIC_TORNADO        = 1,
    EVENT_MASSIVE_DELUGE        = 2,
    EVENT_ARCANE_BOMB           = 3,
    EVENT_CRY_OF_WRATH          = 4,
    EVENT_RAGING_STORMS         = 5,
    //Heroic
    EVENT_CRUSHING_DEPTHS       = 6,

    ACHIEVEMENT,
};

Position const nagPos[4] =
{
    {-3511.70f, 4479.83f, 1.61f, 5.00f},
    {-3474.94f, 4281.46f, 1.89f, 1.65f},
    {-3382.71f, 4354.07f, 1.39f, 2.83f},
    {-3415.23f, 4442.80f, 1.09f, 3.79f},
};

//96028
class boss_wrath_of_azshara : public CreatureScript
{
public:
    boss_wrath_of_azshara() : CreatureScript("boss_wrath_of_azshara") {}

    struct boss_wrath_of_azsharaAI : public BossAI
    {
        boss_wrath_of_azsharaAI(Creature* creature) : BossAI(creature, DATA_WRATH_OF_AZSHARA) 
        {
            SetCombatMovement(false);
            nagaDiedCount = 0;
            SummonNagas();
            me->SetHealth(me->CountPctFromMaxHealth(20));
            me->setRegeneratingHealth(false);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            me->SetVisible(false);
        }

        bool lowHP = false;
        bool nothited = true;
        uint8 nagaDiedCount = 0;
        uint16 checkVictimTimer = 0;

        void Reset() override
        {
            _Reset();
            lowHP = false;
            nothited = true;
            checkVictimTimer = 2000;

            if (nagaDiedCount)
                DoCast(me, SPELL_SURGING_WATERS_AT, true);

            DoCast(me, SPELL_HEAVING_SANDS, true);

            me->SetHealth(me->CountPctFromMaxHealth(20));
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGIC_RESONANCE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FROST_RESONANCE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            Talk(SAY_AGGRO);
            _EnterCombat();

            DoCast(me, SPELL_TIDAL_WAVE_PERIODIC_1, true);
            DoCast(me, SPELL_LIGHTNING_STRIKES_1, true);

            events.RescheduleEvent(EVENT_MYSTIC_TORNADO, 9000);
            events.RescheduleEvent(EVENT_MASSIVE_DELUGE, 12000);
            events.RescheduleEvent(EVENT_ARCANE_BOMB, 23000);

            if (GetDifficultyID() != DIFFICULTY_NORMAL)
                events.RescheduleEvent(EVENT_CRUSHING_DEPTHS, 20000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(SAY_DEATH);
            _JustDied();
            instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_DUNGEON_ENCOUNTER, 1814);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGIC_RESONANCE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FROST_RESONANCE);
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case ACHIEVEMENT:
                    return nothited ? 1 : 0;
            }
            return 0;
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPct(11) && !lowHP)
            {
                lowHP = true;
                events.RescheduleEvent(EVENT_CRY_OF_WRATH, 0);
            }
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_ARCANE_BOMB:
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        Talk(SAY_ARCANE_EMOTE, target->ToPlayer()->GetGUID());
                    Talk(SAY_ARCANE);
                    break;
                }
                case SPELL_ARCANE_BOMB_VEH_SEAT:
                {
                    if (auto summon = me->SummonCreature(NPC_ARCANE_BOMB, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 20000))
                    {
                        summon->AI()->SetGUID(target->GetGUID());
                        //Offlike - crash
                        //ObjectGuid targetGuid = target->GetGUID();
                        //summon->AddDelayedEvent(200, [summon, targetGuid] () -> void
                        //{
                        //    if (summon)
                        //        if (auto target = ObjectAccessor::GetUnit(*summon, targetGuid))
                        //            summon->CastSpell(target, 46598, true);
                        //});
                    }
                    break;
                }
                case SPELL_MASSIVE_DELUGE_DMG:
                case SPELL_RAGING_STORMS:
                case SPELL_TIDAL_WAVE_DMG:
                    if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
                        DoCast(target, SPELL_FROST_RESONANCE, true);
                    break;
                case SPELL_MAGIC_RESONANCE:
                case SPELL_FROST_RESONANCE:
                    nothited = false;
                    break;
            }
        }

        void SpellFinishCast(const SpellInfo* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_CRY_OF_WRATH:
                    me->RemoveAurasDueToSpell(SPELL_TIDAL_WAVE_PERIODIC_1);
                    me->RemoveAurasDueToSpell(SPELL_LIGHTNING_STRIKES_1);
                    me->CastSpell(me, SPELL_TIDAL_WAVE_PERIODIC_2, true);
                    me->CastSpell(me, SPELL_LIGHTNING_STRIKES_2, true);
                    instance->SetData(ACTION_1, true);
                    break;
                case SPELL_TIDAL_WAVE_SELECT_POINT_1:
                case SPELL_TIDAL_WAVE_SELECT_POINT_2:
                {
                    Position pos;
                    me->GetPosition(&pos);
                    pos.m_positionZ += 7.0f;
                    uint8 maxCount = spell->Id == SPELL_TIDAL_WAVE_SELECT_POINT_1 ? 1 : 2;
                    uint32 delay = 0;
                    for (uint8 i = 0; i < maxCount; ++i)
                    {
                        if (auto wave = me->SummonCreature(NPC_TIDAL_WAVE, pos, TEMPSUMMON_TIMED_DESPAWN, 3000))
                            wave->CastSpellDelay(wave, SPELL_TIDAL_WAVE_AT, true, delay, nullptr, nullptr, me->GetGUID());

                        delay += 1000;
                    }
                    break;
                }
            }
        }

        void SummonNagas()
        {
            me->SummonCreature(NPC_MYSTIC_SSAVEH, nagPos[0]);
            me->SummonCreature(NPC_RITUALIST_LESHA, nagPos[1]);
            me->SummonCreature(NPC_CHANNELER_VARISZ, nagPos[2]);
            me->SummonCreature(NPC_BINDER_ASHIOI, nagPos[3]);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            switch (summon->GetEntry())
            {
                case NPC_MYSTIC_SSAVEH:
                case NPC_RITUALIST_LESHA:
                case NPC_CHANNELER_VARISZ:
                case NPC_BINDER_ASHIOI:
                    ++nagaDiedCount;
                    //DoCast(me, SPELL_DAMAGE_SELF_20PCT, true);
                    break;
            }
            if (nagaDiedCount == 4)
            {
                me->RemoveAurasDueToSpell(SPELL_STORM_CONDUIT);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
                me->SetReactState(REACT_DEFENSIVE);
                me->SetHealth(me->CountPctFromMaxHealth(20));
                me->CastSpell(me, SPELL_SURGING_WATERS_AT, true);
                Talk(SAY_START);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (checkVictimTimer <= diff)
            {
                if (me->getVictim())
                    if (!me->IsWithinMeleeRange(me->getVictim()))
                        events.RescheduleEvent(EVENT_RAGING_STORMS, 500);

                checkVictimTimer = 2000;
            }
            else
                checkVictimTimer -= diff;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MYSTIC_TORNADO:
                        DoCast(me, SPELL_MYSTIC_TORNADO, true);
                        events.RescheduleEvent(EVENT_MYSTIC_TORNADO, 25000);
                        break;
                    case EVENT_MASSIVE_DELUGE:
                        DoCast(SPELL_MASSIVE_DELUGE);
                        Talk(SAY_DELUGE);
                        events.RescheduleEvent(EVENT_MASSIVE_DELUGE, 48000);
                        break;
                    case EVENT_ARCANE_BOMB:
                        DoCast(SPELL_ARCANE_BOMB);
                        events.RescheduleEvent(EVENT_ARCANE_BOMB, 30000);
                        break;
                    case EVENT_CRY_OF_WRATH:
                        DoCast(SPELL_CRY_OF_WRATH);
                        Talk(SAY_CRY_EMOTE);
                        Talk(SAY_CRY);
                        break;
                    case EVENT_RAGING_STORMS:
                        DoCast(SPELL_RAGING_STORMS);
                        Talk(SAY_STORMS);
                        break;
                    case EVENT_CRUSHING_DEPTHS:
                        //Talk();
                        if (auto pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            DoCast(pTarget, SPELL_CRUSHING_DEPTHS);
                        events.RescheduleEvent(EVENT_CRUSHING_DEPTHS, 40000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_wrath_of_azsharaAI (creature);
    }
};

//98173, 100248, 100249, 100250
class npc_wrath_of_azshara_nagas : public CreatureScript
{
public:
    npc_wrath_of_azshara_nagas() : CreatureScript("npc_wrath_of_azshara_nagas") {}

    struct npc_wrath_of_azshara_nagasAI : public ScriptedAI
    {
        npc_wrath_of_azshara_nagasAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool activateConduit = false;

        void Reset() override
        {
            events.Reset();
        }

        void IsSummonedBy(Unit* summoner) override
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            DoCast(me, SPELL_TEMPEST_ATTUNEMENT, true);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            me->CastStop();
            events.RescheduleEvent(EVENT_1, 6000);
            events.RescheduleEvent(EVENT_2, 9000);

            if (me->GetEntry() != 98173)
                events.RescheduleEvent(EVENT_3, 18000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(0);
        }

        void DoAction(int32 const action) override
        {
            if (activateConduit)
                return;

            activateConduit = true;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_ATTACKABLE_1);
            me->SetReactState(REACT_AGGRESSIVE);
            me->InterruptNonMeleeSpells(false, SPELL_TEMPEST_ATTUNEMENT);
            DoCast(me, SPELL_STORM_CONDUIT, true);
        }

        void JustReachedHome() override
        {
            DoCast(me, SPELL_STORM_CONDUIT, true);
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_ARCANE_BOMB_VEH_SEAT)
            {
                if (auto summon = me->SummonCreature(NPC_ARCANE_BOMB, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 20000))
                {
                    summon->AI()->SetGUID(target->GetGUID());
                    //Offlike - crash
                    //ObjectGuid targetGuid = target->GetGUID();
                    //summon->AddDelayedEvent(200, [summon, targetGuid] () -> void
                    //{
                    //    if (summon)
                    //        if (auto target = ObjectAccessor::GetUnit(*summon, targetGuid))
                    //            summon->CastSpell(target, 46598, true);
                    //});
                }
            }
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
                        DoCast(196516);
                        events.RescheduleEvent(EVENT_1, 6000);
                        break;                   
                    case EVENT_2:
                        if (me->GetEntry() == NPC_BINDER_ASHIOI)
                            DoCast(196515);
                        if (me->GetEntry() == NPC_MYSTIC_SSAVEH)
                            DoCast(196870);                        
                        if (me->GetEntry() == NPC_CHANNELER_VARISZ)
                            DoCast(197105);                        
                        if (me->GetEntry() == NPC_RITUALIST_LESHA)
                            DoCast(196027);
                        events.RescheduleEvent(EVENT_2, 18000);
                        break;                    
                    case EVENT_3:
                        DoCast(SPELL_ARCANE_BOMB);
                        events.RescheduleEvent(EVENT_3, 18000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wrath_of_azshara_nagasAI(creature);
    }
};

//97673
class npc_wrath_of_azshara_mystic_tornado : public CreatureScript
{
public:
    npc_wrath_of_azshara_mystic_tornado() : CreatureScript("npc_wrath_of_azshara_mystic_tornado") {}

    struct npc_wrath_of_azshara_mystic_tornadoAI : public ScriptedAI
    {
        npc_wrath_of_azshara_mystic_tornadoAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        EventMap events;
        uint16 randMoveTimer;

        void Reset() override {}

        void IsSummonedBy(Unit* summoner) override
        {
            if (!summoner->isInCombat())
            {
                me->DespawnOrUnsummon();
                return;
            }

            DoCast(me, SPELL_MYSTIC_TORNADO_AT, true);
            randMoveTimer = 10000;
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (randMoveTimer <= diff)
            {
                randMoveTimer = 10000;
                float angle = (float)rand_norm() * static_cast<float>(2 * M_PI);
                float distance = 15 * (float)rand_norm();
                float x = me->GetHomePosition().GetPositionX() + distance * std::cos(angle);
                float y = me->GetHomePosition().GetPositionY() + distance * std::sin(angle);
                float z = me->GetHomePosition().GetPositionZ();
                Trinity::NormalizeMapCoord(x);
                Trinity::NormalizeMapCoord(y);

                me->GetMotionMaster()->MovePoint(1, x, y, z);
            }
            else randMoveTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wrath_of_azshara_mystic_tornadoAI(creature);
    }
};

//97691
class npc_wrath_of_azshara_arcane_bomb : public CreatureScript
{
public:
    npc_wrath_of_azshara_arcane_bomb() : CreatureScript("npc_wrath_of_azshara_arcane_bomb") {}

    struct npc_wrath_of_azshara_arcane_bombAI : public ScriptedAI
    {
        npc_wrath_of_azshara_arcane_bombAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetSpeed(MOVE_FLIGHT, 5.0f);
        }

        ObjectGuid targetGuid;
        uint32 moveTimer = 0;

        void IsSummonedBy(Unit* summoner) override
        {
            me->CastSpell(me, SPELL_ARCANE_BOMB_VISUAL, true);
            me->CastSpell(me, SPELL_ARCANE_BOMB_DMG);
            moveTimer = 500;
        }

        void Reset() override {}

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_ARCANE_BOMB_DMG)
            {
                if (GetDifficultyID() != DIFFICULTY_LFR && GetDifficultyID() != DIFFICULTY_NORMAL)
                {
                    if (auto owner = me->GetAnyOwner())
                        owner->CastSpell(target, SPELL_MAGIC_RESONANCE, true);
                }
            }
        }

        void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
        {
            targetGuid = guid;
        }

        void UpdateAI(uint32 diff) override
        {
            if (moveTimer)
            {
                if (moveTimer <= diff)
                {
                    moveTimer = 500;

                    if (auto target = Unit::GetUnit(*me, targetGuid))
                    {
                        if (target->HasAura(SPELL_ARCANE_BOMB_VEH_SEAT))
                            me->GetMotionMaster()->MovePoint(1, target->GetPosition(), false);
                        else
                        {
                            moveTimer = 0;
                            me->StopMoving();
                        }
                    }
                }
                else
                    moveTimer -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wrath_of_azshara_arcane_bombAI(creature);
    }
};

//197365
class spell_wrath_of_azshara_crushing_depths : public SpellScript
{
    PrepareSpellScript(spell_wrath_of_azshara_crushing_depths);

    uint8 targetCount = 0;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targetCount = targets.size();
    }

    void HandleDamage()
    {
        SetHitDamage(GetHitDamage() / targetCount);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_wrath_of_azshara_crushing_depths::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnHit += SpellHitFn(spell_wrath_of_azshara_crushing_depths::HandleDamage);
    }
};

//29405
class achievement_ready_for_raiding_v : public AchievementCriteriaScript
{
public:
    achievement_ready_for_raiding_v() : AchievementCriteriaScript("achievement_ready_for_raiding_v") { }

    bool OnCheck(Player* /*player*/, Unit* target) override
    {
        if (!target)
            return false;

        if (Creature* boss = target->ToCreature())
            if (boss->IsAIEnabled && (boss->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_DUNGEON || boss->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_KEYSTONE))
                if (boss->AI()->GetData(ACHIEVEMENT))
                    return true;

        return false;
    }
};

void AddSC_boss_wrath_of_azshara()
{
    new boss_wrath_of_azshara();
    new npc_wrath_of_azshara_nagas();
    new npc_wrath_of_azshara_mystic_tornado();
    new npc_wrath_of_azshara_arcane_bomb();
    RegisterSpellScript(spell_wrath_of_azshara_crushing_depths);
    new achievement_ready_for_raiding_v();
}