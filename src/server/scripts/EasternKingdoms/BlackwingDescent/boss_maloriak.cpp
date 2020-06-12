#include"Spell.h"
#include "GameObjectAI.h"
#include "blackwing_descent.h"

enum ScriptTexts
{
    SAY_AGGRO       = 0,
    SAY_DEATH       = 1,
    SAY_KILL        = 2,
    SAY_RED         = 3,
    SAY_GREEN       = 4,
    SAY_BLUE        = 5,
    SAY_ABERRATIONS = 7,
    SAY_PRIME       = 8,
};
enum Spells
{
    //shared
    SPELL_THROW_RED_BOTTLE_A        = 77925,
    SPELL_THROW_RED_BOTTLE_B        = 77928,
    SPELL_DRINK_RED_BOTTLE          = 88699,
    SPELL_FIRE_IMBUED               = 78896,
    SPELL_THROW_BLUE_BOTTLE_A       = 77932,
    SPELL_THROW_BLUE_BOTTLE_B       = 77934,
    SPELL_DRINK_BLUE_BOTTLE         = 88700,
    SPELL_FROST_IMBUED              = 78895,
    SPELL_THROW_GREEN_BOTTLE_A      = 77937,
    SPELL_THROW_GREEN_BOTTLE_B      = 77938,
    SPELL_THROW_BLACK_BOTTLE_A      = 92831,
    SPELL_THROW_BLACK_BOTTLE_B      = 92837,
    SPELL_DRINK_BLACK_BOTTLE        = 92828,
    SPELL_DARK_IMBUED               = 92716,
    SPELL_DEBILITATING_SLIME        = 77615,
    SPELL_DEBILITATING_SLIME_1      = 77948, // knock back
    SPELL_BERSERK                   = 64238,

    //1&2 phases
    SPELL_RELEASE_ABERRATIONS       = 77569,
    SPELL_ARCANE_STORM              = 77896,
    SPELL_ARCANE_STORM_DMG          = 77908,
    SPELL_REMEDY                    = 77912,

    //1 - fire phase
    SPELL_CONSUMING_FLAMES          = 77786,
    SPELL_SCORCHING_BLAST           = 77679,

    //2 - frost phase
    SPELL_FLASH_FREEZE              = 77699,
    SPELL_BITING_CHILL              = 77760,
    SPELL_BITING_CHILL_DMG          = 77763,

    //dark phase
    SPELL_ENGULFING_DARKNESS        = 92754,
    SPELL_ENGULFING_DARKNESS_DMG    = 92787,
    SPELL_DARK_SLUDGE               = 92929,
    SPELL_DARK_SLUDGE_DMG           = 92930,

    //3 - execute phase
    SPELL_RELEASE_ALL_MINIONS       = 77991,
    SPELL_ABSOLUTE_ZERO             = 78223,
    SPELL_ABSOLUTE_ZERO_AURA        = 78201,
    SPELL_ABSOLUTE_ZERO_DMG         = 78208,
    SPELL_SHATTER                   = 77715,
    SPELL_ACID_NOVA                 = 78225,
    SPELL_MAGMA_JETS                = 78194,
    SPELL_MAGMA_JETS_SUMMON         = 78094,
    SPELL_MAGMA_JETS_DMG            = 78095,

    //adds aura
    SPELL_GROWN_CATALYST            = 77987,

    //prime subject
    SPELL_REND                      = 78034,
    SPELL_FIXATE                    = 78617,
    SPELL_DROWNED_STATE             = 77564,
    SPELL_BOSS_HITTIN_YA            = 91334,

    SPELL_CLEAR_ACHIEVEMENT         = 94148,

    SPELL_MASTER_ADVENTURER_AWARD   = 89798,
};

enum Adds
{
    NPC_ABERRATION          = 41440,
    NPC_PRIME_SUBJECT       = 41841,
    NPC_FLASH_FREEZE        = 41576,
    NPC_ABSOLUTE_ZERO       = 41961,
    NPC_MAGMA_JET_1         = 50030,
    NPC_MAGMA_JET_2         = 41901,
    NPC_VILE_SWILL          = 49811,
    NPC_CAULDRON_TRIGGER    = 41505,
};

enum Events
{
    EVENT_FIRE_PHASE            = 1,
    EVENT_FROST_PHASE           = 2,
    EVENT_GREEN_PHASE           = 3,
    EVENT_ARCANE_STORM          = 4,
    EVENT_REMEDY                = 5,
    EVENT_CONSUMING_FLAMES      = 6,
    EVENT_SCORCHING_BLAST       = 7,
    EVENT_FLASH_FREEZE          = 8,
    EVENT_BITING_CHILL          = 9,
    EVENT_ABSOLUTE_ZERO         = 10,
    EVENT_ACID_NOVA             = 11,
    EVENT_MAGMA_JETS            = 12,
    EVENT_MAGMA_JETS_T          = 13,
    EVENT_RELEASE_ABERRATIONS   = 14,
    EVENT_RELEASE_ALL_MINIONS   = 15,
    EVENT_REND                  = 16,
    EVENT_FIXATE                = 17,
    EVENT_BERSERK               = 18,
    EVENT_DARK_PHASE            = 19,
    EVENT_DARK_MAGIC            = 20,
    EVENT_ENGULFING_DARKNESS    = 21,
    EVENT_DARK_SLUDGE           = 22,
    EVENT_CONTINUE              = 23,
    EVENT_CONTINUE_STUNNED      = 24,
    EVENT_JUMP_TO               = 25,
};

enum Points
{
    POINT_FIRE  = 1,
    POINT_FROST = 2,
    POINT_GREEN = 3,
    POINT_DARK  = 4,
};

enum Others
{
    DATA_TRAPPED_PLAYER = 1,
};

const Position maloriakHomePos = 
{-105.826f, -470.104f, 73.45f, 1.67f};

const Position maloriakGreenPos = 
{-106.61f, -437.49f, 73.40f, 4.70f};

const Position aberrationPos[2] = 
{
    {-138.48f, -428.37f, 73.241f, 5.63f},
    {-76.70f, -428.15f, 73.23f, 3.51f}
};

const Position maloriaknefariusspawnPos = {-104.74f, -419.98f, 90.29f, 4.75f};
const Position vileswillPos = {-104.74f, -419.98f, 73.23f, 4.75f};

class boss_maloriak : public CreatureScript
{
public:
    boss_maloriak() : CreatureScript("boss_maloriak") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_maloriakAI(pCreature);
    }

    struct boss_maloriakAI : public BossAI
    {
        boss_maloriakAI(Creature* pCreature) : BossAI(pCreature, DATA_MALORIAK)
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
        }

        bool bExecute;
        bool bDark;

        void InitializeAI()
        {
            if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(BDScriptName))
                me->IsAIEnabled = false;
            else if (!me->isDead())
                Reset();
        }

        void Reset()
        {
            _Reset();

            bExecute = false;
            bDark = false;

                me->SetFloatValue(UNIT_FIELD_BOUNDING_RADIUS, 7);
                me->SetFloatValue(UNIT_FIELD_COMBAT_REACH, 7);

            me->SetReactState(REACT_AGGRESSIVE);
            me->SetControlled(false, UNIT_STATE_STUNNED);

            instance->SetData(DATA_MALORIAK_ABERRATIONS, 18);
        }

        void EnterCombat(Unit* attacker)
        {
            instance->DoResetAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, CRITERIA_CONDITION_NO_SPELL_HIT, SPELL_CLEAR_ACHIEVEMENT);
            instance->DoResetAchievementCriteria(CRITERIA_TYPE_KILL_CREATURE, CRITERIA_CONDITION_NO_SPELL_HIT, SPELL_CLEAR_ACHIEVEMENT);

            if (IsHeroic())
                events.RescheduleEvent(EVENT_DARK_PHASE, 15000);
            else
                events.RescheduleEvent(EVENT_FIRE_PHASE, 15000);
            events.RescheduleEvent(EVENT_ARCANE_STORM, 10000);
            events.RescheduleEvent(EVENT_BERSERK, IsHeroic()? 9*MINUTE*IN_MILLISECONDS : 7*MINUTE*IN_MILLISECONDS);
            Talk(SAY_AGGRO);
            DoZoneInCombat();
            if (IsHeroic())
                if (Creature* pNefarius = me->SummonCreature(NPC_LORD_VICTOR_NEFARIUS_HEROIC, maloriaknefariusspawnPos))
                    pNefarius->AI()->DoAction(ACTION_MALORIAK_INTRO);
            instance->SetBossState(DATA_MALORIAK, IN_PROGRESS);
        }

        void JustReachedHome()
        {
            _JustReachedHome();
        }

        void KilledUnit(Unit* victim)
        {
            Talk(SAY_KILL);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DEATH);
            if (Creature* pNefarius = me->SummonCreature(NPC_LORD_VICTOR_NEFARIUS_HEROIC, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f))
                pNefarius->AI()->DoAction(ACTION_MALORIAK_DEATH);
            if (IsHeroic())
                DoCast(me, SPELL_MASTER_ADVENTURER_AWARD, true);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                case POINT_FIRE:
                    Talk(SAY_RED);
                    DoCast(me, SPELL_DRINK_RED_BOTTLE);
                    break;
                case POINT_FROST:
                    Talk(SAY_BLUE);
                    DoCast(me, SPELL_DRINK_BLUE_BOTTLE);                            
                    break;
                case POINT_GREEN:
                    Talk(SAY_GREEN);
                    events.RescheduleEvent(EVENT_JUMP_TO, 1000);
                    break;
                case POINT_DARK:
                    DoCast(me, SPELL_DRINK_BLACK_BOTTLE);
                    events.RescheduleEvent(EVENT_FIRE_PHASE, 100000);
                    break;
                }
            }
            if (id == EVENT_JUMP)
            {
                EventGreenSkills(true);
                if (IsHeroic())
                    events.RescheduleEvent(EVENT_DARK_PHASE, 41000);
                else
                    events.RescheduleEvent(EVENT_FIRE_PHASE, 41000);
                events.RescheduleEvent(EVENT_CONTINUE, 1000);
            }
        }

        void SpellHit(Unit* who, const SpellInfo* spellInfo)
        {
            switch (spellInfo->Id)
            {
            case SPELL_DRINK_RED_BOTTLE:
                EventFireSkills(true);
                events.RescheduleEvent(EVENT_FROST_PHASE, 40000);
                events.RescheduleEvent(EVENT_CONTINUE, 1000);
                break;
            case SPELL_DRINK_BLUE_BOTTLE:
                EventFrostSkills(true);
                events.RescheduleEvent(EVENT_GREEN_PHASE, 40000);
                events.RescheduleEvent(EVENT_CONTINUE, 1000);
                break;
            case SPELL_DRINK_BLACK_BOTTLE:
                EventDarkSkills(true);
                events.RescheduleEvent(EVENT_FIRE_PHASE, 100000);
                events.RescheduleEvent(EVENT_CONTINUE, 1000);
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HealthBelowPct(25) && !bExecute)
            {
                bExecute = true;
                me->InterruptNonMeleeSpells(false);
                EventFireSkills(false);
                EventFrostSkills(false);
                EventGreenSkills(false);
                EventDarkSkills(false);
                events.CancelEvent(EVENT_FIRE_PHASE);
                events.CancelEvent(EVENT_FROST_PHASE);
                events.CancelEvent(EVENT_GREEN_PHASE);
                events.CancelEvent(EVENT_DARK_PHASE);
                me->RemoveAurasDueToSpell(SPELL_FIRE_IMBUED);
                me->RemoveAurasDueToSpell(SPELL_FROST_IMBUED);
                me->RemoveAurasDueToSpell(SPELL_DARK_IMBUED);
                DoCast(me, SPELL_RELEASE_ALL_MINIONS);
                EventExecuteSkills();
                me->SetReactState(REACT_AGGRESSIVE);
                me->GetMotionMaster()->MoveChase(me->getVictim());
                return;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FIRE_PHASE:
                    me->SetReactState(REACT_PASSIVE);
                    EventDarkSkills(false);
                    if (GameObject* pGo = me->FindNearestGameObject(203306, 300.0f))
                        me->CastSpell(pGo, SPELL_THROW_RED_BOTTLE_A, false);
                    me->GetMotionMaster()->MovePoint(POINT_FIRE, maloriakHomePos);
                    break;
                case EVENT_FROST_PHASE:
                    me->SetReactState(REACT_PASSIVE);
                    EventFireSkills(false);
                    if (GameObject* pGo = me->FindNearestGameObject(203306, 300.0f))
                        me->CastSpell(pGo, SPELL_THROW_BLUE_BOTTLE_A, false);
                    me->GetMotionMaster()->MovePoint(POINT_FROST, maloriakHomePos);
                    break;
                case EVENT_GREEN_PHASE:
                    me->SetReactState(REACT_PASSIVE);
                    EventFrostSkills(false);
                    if (GameObject* pGo = me->FindNearestGameObject(203306, 300.0f))
                        me->CastSpell(pGo, SPELL_THROW_GREEN_BOTTLE_A, false);
                    me->GetMotionMaster()->MovePoint(POINT_GREEN, maloriakHomePos);
                    break;
                case EVENT_JUMP_TO:
                    for (SummonList::const_iterator itr = summons.begin(); itr != summons.end(); ++itr)    
                        if (Creature* summon = Unit::GetCreature(*me, *itr))
                            summon->RemoveAurasDueToSpell(SPELL_GROWN_CATALYST);
                    DoCast(me, SPELL_DEBILITATING_SLIME, true);
                    DoCast(me, SPELL_DEBILITATING_SLIME_1, true);
                    me->GetMotionMaster()->MoveJump(maloriakGreenPos.GetPositionX(), maloriakGreenPos.GetPositionY(), maloriakGreenPos.GetPositionZ(), 15.0f, 15.0f);
                    break;
                case EVENT_DARK_PHASE:
                    me->SetReactState(REACT_PASSIVE);
                    if (Creature* pNefarius = me->FindNearestCreature(NPC_LORD_VICTOR_NEFARIUS_HEROIC, 200.0f))
                        pNefarius->AI()->DoAction(ACTION_MALORIAK_DARK_MAGIC);
                    EventGreenSkills(false);
                    me->GetMotionMaster()->MovePoint(POINT_DARK, maloriakHomePos);
                    break;
                case EVENT_ARCANE_STORM:
                    DoCast(me, SPELL_ARCANE_STORM);
                    break;
                case EVENT_REMEDY:
                    DoCast(me, SPELL_REMEDY);
                    events.RescheduleEvent(EVENT_REMEDY, urand(30000, 40000));
                    break;
                case EVENT_RELEASE_ABERRATIONS:
                    {
                        uint32 count = 0;
                        count = instance->GetData(DATA_MALORIAK_ABERRATIONS);
                        if (count >= 3)
                            DoCast(me, SPELL_RELEASE_ABERRATIONS);
                        break;
                    }
                case EVENT_SCORCHING_BLAST:
                    DoCast(SPELL_SCORCHING_BLAST);
                    break;
                case EVENT_CONSUMING_FLAMES:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        DoCast(target, SPELL_CONSUMING_FLAMES);
                    break;
                case EVENT_BITING_CHILL:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                        DoCast(target, SPELL_BITING_CHILL);
                    break;
                case EVENT_FLASH_FREEZE:
                    {
                        std::list<Unit*> targets;
                        uint32 minTargets = RAID_MODE<uint32>(3, 8, 3, 8);
                        SelectTargetList(targets, minTargets, SELECT_TARGET_RANDOM, -20.0f, true);
                        float minDist = 0.0f;
                        if (targets.size() >= minTargets)
                            minDist = -20.0f;
                        if (Unit* targetflashfreeze = SelectTarget(SELECT_TARGET_RANDOM, 1, minDist, true))
                            DoCast(targetflashfreeze, SPELL_FLASH_FREEZE);    
                        break;
                    }
                case EVENT_ENGULFING_DARKNESS:
                    DoCast(me->getVictim(), SPELL_ENGULFING_DARKNESS);
                    events.RescheduleEvent(EVENT_ENGULFING_DARKNESS, 14000);
                    break;
                case EVENT_ABSOLUTE_ZERO:
                    {
                        Unit* targetabsolutezero = NULL;
                        targetabsolutezero = SelectTarget(SELECT_TARGET_RANDOM, 1, -20.0f, true);
                        if (!targetabsolutezero)
                            targetabsolutezero = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true);
                        if (targetabsolutezero)
                            DoCast(targetabsolutezero, SPELL_ABSOLUTE_ZERO);
                        events.RescheduleEvent(EVENT_ABSOLUTE_ZERO, urand(20000, 30000));
                        break;
                    }
                case EVENT_MAGMA_JETS:
                    me->SetControlled(true, UNIT_STATE_STUNNED);
                    DoCast(me, SPELL_MAGMA_JETS);
                    events.RescheduleEvent(EVENT_MAGMA_JETS, 15000);
                    events.RescheduleEvent(EVENT_CONTINUE_STUNNED, 3000);
                    break;
                case EVENT_ACID_NOVA:
                    DoCast(SPELL_ACID_NOVA);
                    events.RescheduleEvent(EVENT_ACID_NOVA, urand(15000, 20000));
                    break;
                case EVENT_BERSERK:
                    DoCast(me, SPELL_BERSERK);
                    break;
                case EVENT_CONTINUE:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->GetMotionMaster()->MoveChase(me->getVictim());
                    break;
                case EVENT_CONTINUE_STUNNED:
                    me->SetControlled(false, UNIT_STATE_STUNNED);
                    break;
                }
            }
            if (!bDark)
                DoMeleeAttackIfReady();
        }
    private:

        void EventFireSkills(bool on)
        {
            if (on)
            {
                events.RescheduleEvent(EVENT_RELEASE_ABERRATIONS, 10000);
                events.RescheduleEvent(EVENT_RELEASE_ABERRATIONS, 30000);
                events.RescheduleEvent(EVENT_ARCANE_STORM, 13000);
                events.RescheduleEvent(EVENT_ARCANE_STORM, 33000);
                events.RescheduleEvent(EVENT_SCORCHING_BLAST, 17000);
                events.RescheduleEvent(EVENT_SCORCHING_BLAST, 37000);
                events.RescheduleEvent(EVENT_CONSUMING_FLAMES, 7000);
                events.RescheduleEvent(EVENT_CONSUMING_FLAMES, 22000);
                events.RescheduleEvent(EVENT_CONSUMING_FLAMES, 39000);
            }
            else
            {
                events.CancelEvent(EVENT_ARCANE_STORM);
                events.CancelEvent(EVENT_RELEASE_ABERRATIONS);
                events.CancelEvent(EVENT_CONSUMING_FLAMES);
                events.CancelEvent(EVENT_SCORCHING_BLAST);
            }
        }

        void EventFrostSkills(bool on)
        {
            if (on)
            {
                events.RescheduleEvent(EVENT_RELEASE_ABERRATIONS, 10000); 
                events.RescheduleEvent(EVENT_RELEASE_ABERRATIONS, 30000);
                events.RescheduleEvent(EVENT_ARCANE_STORM, 13000);
                events.RescheduleEvent(EVENT_ARCANE_STORM, 33000);
                events.RescheduleEvent(EVENT_BITING_CHILL, 13000);
                events.RescheduleEvent(EVENT_BITING_CHILL, 24000);
                events.RescheduleEvent(EVENT_BITING_CHILL, 35000);
                events.RescheduleEvent(EVENT_FLASH_FREEZE, 19000);
            }
            else
            {
                events.CancelEvent(EVENT_ARCANE_STORM);
                events.CancelEvent(EVENT_RELEASE_ABERRATIONS);
                events.CancelEvent(EVENT_BITING_CHILL);
                events.CancelEvent(EVENT_FLASH_FREEZE);
            }
        }

        void EventGreenSkills(bool on)
        {
            if (on)
            {
                events.RescheduleEvent(EVENT_RELEASE_ABERRATIONS, 7000); 
                events.RescheduleEvent(EVENT_RELEASE_ABERRATIONS, 25000);
                events.RescheduleEvent(EVENT_ARCANE_STORM, 5000);
                events.RescheduleEvent(EVENT_ARCANE_STORM, 19000);
            }
            else
            {
                events.CancelEvent(EVENT_ARCANE_STORM);
                events.CancelEvent(EVENT_RELEASE_ABERRATIONS);
            }
        }

        void EventDarkSkills(bool on)
        {
            if (on)
            {
                bDark = true;
                for (uint8 i = 0; i < 5; i++)
                    me->SummonCreature(NPC_VILE_SWILL, vileswillPos);
                events.RescheduleEvent(EVENT_ENGULFING_DARKNESS, 5000);
            }
            else
            {
                bDark = false;
                events.CancelEvent(EVENT_ENGULFING_DARKNESS);
            }
        }

        void EventExecuteSkills()
        {
            events.RescheduleEvent(EVENT_ABSOLUTE_ZERO, urand(4000, 5000));
            events.RescheduleEvent(EVENT_MAGMA_JETS, 5000);
            events.RescheduleEvent(EVENT_ACID_NOVA, urand(7000, 8000));
        }
    };
};

class npc_maloriak_flash_freeze : public CreatureScript
{
public:
    npc_maloriak_flash_freeze() : CreatureScript("npc_maloriak_flash_freeze") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_maloriak_flash_freezeAI(creature);
    }

    struct npc_maloriak_flash_freezeAI : public Scripted_NoMovementAI
    {
        npc_maloriak_flash_freezeAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            trappedPlayer.Clear();
        }

        ObjectGuid trappedPlayer;
        uint32 existenceCheckTimer;

        void Reset()
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void SetGUID(ObjectGuid const& guid, int32 type)
        {
            if (type == DATA_TRAPPED_PLAYER)
            {
                trappedPlayer = guid;
                existenceCheckTimer = 1000;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, trappedPlayer))
            {
                trappedPlayer.Clear();
                player->RemoveAurasDueToSpell(SPELL_FLASH_FREEZE);
            }
            DoCast(me, SPELL_SHATTER);
            me->DespawnOrUnsummon(800);
        }

        void UpdateAI(uint32 diff)
        {
            if (!trappedPlayer)
                return;

            if (existenceCheckTimer <= diff)
            {
                Player* player = ObjectAccessor::GetPlayer(*me, trappedPlayer);
                if (!player || player->isDead() || !player->HasAura(SPELL_FLASH_FREEZE))
                {
                    JustDied(me);
                    me->DespawnOrUnsummon();
                    return;
                }
            }
            else
                existenceCheckTimer -= diff;
        }            
    };
};

class npc_absolute_zero : public CreatureScript
{
public:
    npc_absolute_zero() : CreatureScript("npc_absolute_zero") { }

    struct npc_absolute_zeroAI : public ScriptedAI
    {
        npc_absolute_zeroAI(Creature* creature) : ScriptedAI(creature)
        {
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            creature->SetSpeed(MOVE_RUN, 0.5f);
            creature->SetSpeed(MOVE_WALK, 0.5f);
        }

        uint32 uiPauseTimer; //����� �� ����������� ����� ��� ������� ����� ������
        uint32 uiDespawnTimer;
        bool bCanExplode; 

        void Reset()
        {
            uiPauseTimer = 3000;
            uiDespawnTimer = 15000;
            bCanExplode = false;
        }

        void IsSummonedBy(Unit* owner)
        {
            DoCast(SPELL_ABSOLUTE_ZERO_AURA);
        }

        void UpdateAI(uint32 diff)
        {
            if ((uiPauseTimer <= diff) && !bCanExplode)
            {
                bCanExplode = true;
                if (Unit* target = me->SelectNearestTarget())
                    me->GetMotionMaster()->MoveFollow(target, 0.1f, 0.0f);
            }
            else
                uiPauseTimer -= diff;

            if (uiDespawnTimer <= diff)
                me->DespawnOrUnsummon();
            else
                uiDespawnTimer -= diff;

            if (Unit* target = me->SelectNearestTarget())
            {
                if ((me->GetDistance(target) <= 4.0f) && bCanExplode)
                {
                    DoCast(SPELL_ABSOLUTE_ZERO_DMG);
                    me->DespawnOrUnsummon(800);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_absolute_zeroAI(creature);
    }
};

class npc_magma_jet : public CreatureScript
{
public:
    npc_magma_jet() : CreatureScript("npc_magma_jet") { }

    struct npc_magma_jetAI : public Scripted_NoMovementAI
    {
        npc_magma_jetAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        EventMap events;
        Unit* creOwner;
        void IsSummonedBy(Unit* owner)
        {
            if (!owner->ToCreature())
                return;
            creOwner = owner->ToCreature();
            Position pos;
            me->SetOrientation(creOwner->GetOrientation());
            owner->GetNearPosition(pos, owner->GetObjectSize()/2.0f, 0.0f);
            me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            events.RescheduleEvent(EVENT_MAGMA_JETS_T, 200);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_MAGMA_JETS_T)
            {
                Position newPos;
                me->GetNearPosition(newPos, 5.5f, 0.0f);
                me->NearTeleportTo(newPos.GetPositionX(), newPos.GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                if (creOwner->GetDistance(me) >= 50.0f)
                    me->DespawnOrUnsummon();
                else
                {
                    DoCast(SPELL_MAGMA_JETS_SUMMON);
                    events.RescheduleEvent(EVENT_MAGMA_JETS_T, 200);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_magma_jetAI(creature);
    }
};

class npc_magma_jet_summon : public CreatureScript
{
public:
    npc_magma_jet_summon() : CreatureScript("npc_magma_jet_summon") { }

    struct npc_magma_jet_summonAI : public Scripted_NoMovementAI
    {
        npc_magma_jet_summonAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
        uint32 summonTimer;

        void Reset()
        {
            DoCast(SPELL_MAGMA_JETS_DMG);
            summonTimer = 15000;
        }

        void UpdateAI(uint32 diff)
        {
            if (summonTimer <= diff)
                me->DespawnOrUnsummon();
            else
                summonTimer -= diff;                
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_magma_jet_summonAI(creature);
    }
};

class npc_aberration : public CreatureScript
{
public:
    npc_aberration() : CreatureScript("npc_aberration") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_aberrationAI(creature);
    }

    struct npc_aberrationAI : public ScriptedAI
    {
        npc_aberrationAI(Creature* creature) : ScriptedAI(creature) { }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void Reset()
        {
            DoCast(SPELL_GROWN_CATALYST);
        }
    };
};

class npc_prime_subject : public CreatureScript
{
public:
    npc_prime_subject() : CreatureScript("npc_prime_subject") { }

    struct npc_prime_subjectAI : public ScriptedAI
    {
        npc_prime_subjectAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            DoCast(SPELL_GROWN_CATALYST);
        }

        void EnterCombat(Unit* who)
        {
            //events.RescheduleEvent(EVENT_FIXATE, 5000);
            events.RescheduleEvent(EVENT_REND, 12000);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FIXATE:
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                    DoCast(me->getVictim(), SPELL_FIXATE);
                    break;
                case EVENT_REND:
                    DoCast(me->getVictim(), SPELL_REND);
                    events.RescheduleEvent(EVENT_REND, urand(12000, 16000));
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prime_subjectAI(creature);
    }
};

class npc_vile_swill : public CreatureScript
{
public:
    npc_vile_swill() : CreatureScript("npc_vile_swill") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_vile_swillAI(creature);
    }

    struct npc_vile_swillAI : public ScriptedAI
    {
        npc_vile_swillAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
        }

        void EnterCombat(Unit* who)
        {
            events.RescheduleEvent(EVENT_DARK_SLUDGE, urand(5000, 10000));
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_DARK_SLUDGE:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(pTarget, SPELL_DARK_SLUDGE);
                    events.RescheduleEvent(EVENT_DARK_SLUDGE, urand(15000, 18000));
                    break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class spell_maloriak_flash_freeze : public SpellScriptLoader
{
public:
    spell_maloriak_flash_freeze() : SpellScriptLoader("spell_maloriak_flash_freeze") { }

    class spell_maloriak_flash_freeze_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_maloriak_flash_freeze_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            Position pos;
            aurEff->GetBase()->GetOwner()->GetPosition(&pos);
            if (!GetCaster())
                return;
            if (TempSummon* summon = GetCaster()->SummonCreature(NPC_FLASH_FREEZE, pos))
                summon->AI()->SetGUID(aurEff->GetBase()->GetOwner()->GetGUID(), DATA_TRAPPED_PLAYER);   
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_maloriak_flash_freeze_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_maloriak_flash_freeze_AuraScript();
    }
};

class GoCheck
{
public:
    GoCheck() { }
    bool operator()(WorldObject* obj) const
    {
        if (!obj->ToGameObject())
            return true;

        return (obj->ToGameObject()->GetEntry() != 206704);
    }
};

class EntryCheck
{
public:
    EntryCheck(uint32 entry) {i_entry = entry;}
    bool operator()(WorldObject* obj) const
    {
        if (!obj->ToCreature())
            return true;

        return (obj->ToCreature()->GetEntry() != i_entry);
    }
private:
    uint32 i_entry;
};

class spell_maloriak_release_aberrations : public SpellScriptLoader
{
public:
    spell_maloriak_release_aberrations() : SpellScriptLoader("spell_maloriak_release_aberrations") { }


    class spell_maloriak_release_aberrations_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_release_aberrations_SpellScript);

        bool Load()
        {
            _count = 0;
            return true;
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty())
                return;

            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                targets.remove_if(GoCheck());
                uint32 count = instance->GetData(DATA_MALORIAK_ABERRATIONS);
                if (!targets.empty() && count > 0)
                    Trinity::Containers::RandomResizeList(targets, count > 3? 3: count);
                else
                    targets.clear();
            }
        }

        void HandleDummy(SpellEffIndex effIndex)
        {
            if (!GetCaster() || !GetHitGObj() || effIndex != EFFECT_0)
                return;

            GetHitGObj()->SetGoState(GO_STATE_ACTIVE);
            GetCaster()->SummonCreature(NPC_ABERRATION, (GetHitGObj()->GetPositionX() < -100.0f? aberrationPos[0]: aberrationPos[1]), TEMPSUMMON_DEAD_DESPAWN);
            _count++;
        }

        void OnAfterCast()
        {
            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                uint32 count = instance->GetData(DATA_MALORIAK_ABERRATIONS);
                instance->SetData(DATA_MALORIAK_ABERRATIONS, (_count > count? 0: count - _count));
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_release_aberrations_SpellScript::FilterTargets, EFFECT_0,TARGET_GAMEOBJECT_SRC_AREA);
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_release_aberrations_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            AfterCast += SpellCastFn(spell_maloriak_release_aberrations_SpellScript::OnAfterCast);
        }

    private:
        uint32 _count;
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_release_aberrations_SpellScript();
    }
};

class spell_maloriak_release_all_minions : public SpellScriptLoader
{
public:
    spell_maloriak_release_all_minions() : SpellScriptLoader("spell_maloriak_release_all_minions") { }


    class spell_maloriak_release_all_minions_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_release_all_minions_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        { 
            if (targets.empty())
                return;

            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            {
                std::list<WorldObject*> tempGos;
                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    if ((*itr)->GetEntry() == 206705)
                        tempGos.push_back((*itr));

                targets.remove_if(GoCheck());

                uint32 count = instance->GetData(DATA_MALORIAK_ABERRATIONS);
                if (count > 0)
                    Trinity::Containers::RandomResizeList(targets, count);
                else
                    targets.clear();

                if (!tempGos.empty())
                    for (std::list<WorldObject*>::const_iterator itr = tempGos.begin(); itr != tempGos.end(); ++itr)
                        targets.push_back((*itr));
            }
        }

        void HandleDummy(SpellEffIndex effIndex)
        {

            if (!GetCaster() || !GetHitGObj() || effIndex != EFFECT_0)
                return;

            if (GetHitGObj()->GetEntry() == 206704)
                GetCaster()->SummonCreature(NPC_ABERRATION, (GetHitGObj()->GetPositionX() < -100.0f? aberrationPos[0]: aberrationPos[1]), TEMPSUMMON_CORPSE_DESPAWN);
            else if (GetHitGObj()->GetEntry() == 206705)
                GetCaster()->SummonCreature(NPC_PRIME_SUBJECT, (GetHitGObj()->GetPositionX() < -100.0f? aberrationPos[0]: aberrationPos[1]), TEMPSUMMON_CORPSE_DESPAWN); 
        }

        void OnAfterCast()
        {
            if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                instance->SetData(DATA_MALORIAK_ABERRATIONS, 0);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_release_all_minions_SpellScript::FilterTargets, EFFECT_0,TARGET_GAMEOBJECT_SRC_AREA);
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_release_all_minions_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            AfterCast += SpellCastFn(spell_maloriak_release_all_minions_SpellScript::OnAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_release_all_minions_SpellScript();
    }
};

class spell_maloriak_debilitating_slime : public SpellScriptLoader
{
public:
    spell_maloriak_debilitating_slime() : SpellScriptLoader("spell_maloriak_debilitating_slime") { }


    class spell_maloriak_debilitating_slime_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_debilitating_slime_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(EntryCheck(NPC_ABERRATION));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_debilitating_slime_SpellScript::FilterTargets, EFFECT_2,TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_debilitating_slime_SpellScript();
    }
};

class spell_maloriak_shatter : public SpellScriptLoader
{
public:
    spell_maloriak_shatter() : SpellScriptLoader("spell_maloriak_shatter") { }


    class spell_maloriak_shatter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_shatter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(EntryCheck(NPC_FLASH_FREEZE));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_maloriak_shatter_SpellScript::FilterTargets, EFFECT_1,TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_shatter_SpellScript();
    }
};

class spell_maloriak_remedy : public SpellScriptLoader
{
public:
    spell_maloriak_remedy() : SpellScriptLoader("spell_maloriak_remedy") { }

    class spell_maloriak_remedy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_maloriak_remedy_AuraScript);

        void HandleTick(AuraEffect const* aurEff)
        {
            int32 baseAmount = aurEff->GetBaseAmount();
            if (baseAmount > 0)
                const_cast<AuraEffect*>(aurEff)->SetAmount(baseAmount * aurEff->GetTickNumber());
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_maloriak_remedy_AuraScript::HandleTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_maloriak_remedy_AuraScript();
    }
};

class spell_maloriak_throw_bottle : public SpellScriptLoader
{
public:
    spell_maloriak_throw_bottle() : SpellScriptLoader("spell_maloriak_throw_bottle") { }


    class spell_maloriak_throw_bottle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_throw_bottle_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster() || !GetHitGObj())
                return;

            GetCaster()->CastSpell(GetHitGObj(), GetSpellInfo()->Effects[EFFECT_0]->BasePoints, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_throw_bottle_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_throw_bottle_SpellScript();
    }
};

class spell_maloriak_drink_bottle : public SpellScriptLoader
{
public:
    spell_maloriak_drink_bottle() : SpellScriptLoader("spell_maloriak_drink_bottle") { }


    class spell_maloriak_drink_bottle_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_maloriak_drink_bottle_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (!GetCaster())
                return;

            uint32 spell_id = 0;

            switch (GetSpellInfo()->Id)
            {
            case SPELL_DRINK_RED_BOTTLE:
                spell_id = SPELL_FIRE_IMBUED;
                break;
            case SPELL_DRINK_BLUE_BOTTLE:
                spell_id = SPELL_FROST_IMBUED;
                break;
            case SPELL_DRINK_BLACK_BOTTLE:
                spell_id = SPELL_DARK_IMBUED;
                break;
            }
            if (spell_id)
                GetCaster()->CastSpell(GetCaster(), spell_id, true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_maloriak_drink_bottle_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_maloriak_drink_bottle_SpellScript();
    }
};

class spell_lord_victor_nefarius_master_adventurer_award : public SpellScriptLoader
{
public:
    spell_lord_victor_nefarius_master_adventurer_award() : SpellScriptLoader("spell_lord_victor_nefarius_master_adventurer_award") { }

    class spell_lord_victor_nefarius_master_adventurer_award_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_lord_victor_nefarius_master_adventurer_award_AuraScript);

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->GetTypeId() == TYPEID_PLAYER)
                if (CharTitlesEntry const* title = sCharTitlesStore.LookupEntry(188))
                    GetTarget()->ToPlayer()->SetTitle(title);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->GetTypeId() == TYPEID_PLAYER)
                if (CharTitlesEntry const* title = sCharTitlesStore.LookupEntry(188))
                    GetTarget()->ToPlayer()->SetTitle(title, true);

        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_lord_victor_nefarius_master_adventurer_award_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectApplyFn(spell_lord_victor_nefarius_master_adventurer_award_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_lord_victor_nefarius_master_adventurer_award_AuraScript();
    }
};

/*class spell_maloriak_consuming_flames : public SpellScriptLoader
{
public:
spell_maloriak_consuming_flames() : SpellScriptLoader("spell_maloriak_consuming_flames") { }

class spell_maloriak_consuming_flames_AuraScript : public AuraScript
{
PrepareAuraScript(spell_maloriak_consuming_flames_AuraScript);

bool Load()
{
m_custom_data = 0;
return true;
}

void HandleTick(AuraEffect const* aurEff)
{
m_custom_data += aurEff->GetAmount();
}

void Register()
{
OnEffectPeriodic += AuraEffectPeriodicFn(spell_maloriak_consuming_flames_AuraScript::HandleTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
}
};

AuraScript *GetAuraScript() const
{
return new spell_maloriak_consuming_flames_AuraScript();
}
};*/

void AddSC_boss_maloriak()
{
    new boss_maloriak();
    new npc_maloriak_flash_freeze();
    new npc_absolute_zero();
    new npc_magma_jet();
    new npc_magma_jet_summon();
    new npc_aberration();
    new npc_prime_subject();
    new npc_vile_swill();
    new spell_maloriak_flash_freeze();
    new spell_maloriak_release_aberrations();
    new spell_maloriak_release_all_minions();
    new spell_maloriak_debilitating_slime();
    new spell_maloriak_shatter();
    new spell_maloriak_remedy();
    new spell_maloriak_throw_bottle();
    new spell_maloriak_drink_bottle();
    new spell_lord_victor_nefarius_master_adventurer_award();
    //new spell_maloriak_consuming_flames();
}
