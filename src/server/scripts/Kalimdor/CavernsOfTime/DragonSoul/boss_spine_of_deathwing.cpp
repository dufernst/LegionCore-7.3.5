#include "dragon_soul.h"
#include "ObjectVisitors.hpp"

enum ScriptedTexts
{
    SAY_DEATHWING_LEFT_ROLL_PRE     = 0,
    SAY_DEATHWING_RIGHT_ROLL_PRE    = 1,
    SAY_DEATHWING_LEFT_ROLL         = 2,
    SAY_DEATHWING_RIGHT_ROLL        = 3,
    SAY_DEATHWING_PLATE             = 4,
    SAY_DEATHWING_TALK              = 5,

    ANN_NUCLEAR_BLAST               = 0,
};

enum Spells
{
    // Deathwing
    SPELL_ROLL_CONTROL              = 105036, // check aura
    SPELL_ROLL_CONTROL_AOE_1        = 104621, // triggered by check aura
    SPELL_ROLL_CONTROL_FORCE        = 105740, // 
    SPELL_ROLL_CONTROL_AOE_2        = 105777, // triggered by force spell
    SPELL_ROLL_CONTROL_AOE_3        = 105773, // ?
    SPELL_SUMMON_SLIME_AOE          = 105537, // 
    SPELL_SUMMON_SLIME              = 104999, // cast on spawner

    // Corruption
    SPELL_FIERY_GRIP                = 105490,
    SPELL_SEARING_PLASMA_AOE        = 109379,
    SPELL_SEARING_PLASMA            = 105479,

    // Hideous Amalgamation
    SPELL_ZERO_REGEN                = 109121,
    SPELL_ABSORB_BLOOD_BAR          = 109329,
    SPELL_DEGRADATION               = 106005,
    SPELL_NUCLEAR_BLAST             = 105845,
    SPELL_NUCLEAR_BLAST_SCRIPT      = 105846,
    SPELL_SUPERHEATED_NUCLEUS       = 105834,
    SPELL_SUPERHEATED_NUCLEUS_DMG   = 106264,
    SPELL_ABSORB_BLOOD              = 105244,
    SPELL_ABSORB_BLOOD_DUMMY        = 105241, // target on 105223
    SPELL_ABSORBED_BLOOD            = 105248,

    // Spawner ?
    SPELL_GRASPING_TENDRILS         = 105510,
    SPELL_GRASPING_TENDRILS_10      = 105563,

    // Corrupted Blood
    SPELL_RESIDUE                   = 105223,
    SPELL_BURST                     = 105219,

    // Burning Tendons
    SPELL_SEAL_ARMOR_BREACH_1       = 105847,
    SPELL_SEAL_ARMOR_BREACH_2       = 105848,
    SPELL_BREACH_ARMOR_1            = 105363,
    SPELL_BREACH_ARMOR_2            = 105385,
    SPELL_PLATE_FLY_OFF_LEFT        = 105366,
    SPELL_PLATE_FLY_OFF_RIGHT       = 105384,
    SPELL_SLOW                      = 110907, // ?

    SPELL_BLOOD_CORRUPTION_DEATH    = 106199,
    SPELL_BLOOD_CORRUPTION_EARTH    = 106200,
    SPELL_BLOOD_OF_DEATHWING        = 106201,
    SPELL_BLOOD_OF_NELTHARION       = 106213,

    SPELL_PLAY_MOVIE                = 104574, // script, achievement spell
};

enum Adds
{
    NPC_HIDEOUS_AMALGAMATION    = 53890,
    NPC_CORRUPTION_1            = 53891,
    NPC_CORRUPTION_2            = 56161, // ?
    NPC_CORRUPTION_3            = 56162, // ?
    NPC_SPAWNER                 = 53888,
    NPC_CORRUPTED_BLOOD         = 53889,
    NPC_BURNING_TENDONS_1       = 56341, // left
    NPC_BURNING_TENDONS_2       = 56575, // right
};

enum Events
{
    EVENT_CHECK_PLAYERS                 = 1,
    EVENT_FIERY_GRIP                    = 2,
    EVENT_SEARING_PLASMA                = 3,
    EVENT_CHECK_ROLL                    = 4,
    EVENT_SUMMON_HIDEOUS_AMALGAMATION   = 5,
    EVENT_SUMMON_CORRUPTED_BLOOD        = 6,
    EVENT_CHECK_SPAWNER                 = 7,
    EVENT_MOVE_TO_SPAWNER               = 8,
    EVENT_TALK                          = 9,
    EVENT_END_ENCOUNTER                 = 10,
    EVENT_BLOOD_CORRUPTION              = 11,
    EVENT_SPAWN_LOOT                    = 12,
    EVENT_CHECK_CASTING                 = 13,

    EVENT_PRE_ROLL_L_1,
    EVENT_PRE_ROLL_L_2,
    EVENT_PRE_ROLL_R_1,
    EVENT_PRE_ROLL_R_2,
    EVENT_ROLL_R,
    EVENT_ROLL_L,
};

enum Actions
{
    ACTION_START_BATTLE = 1,
    ACTION_ROLL_LEFT    = 2,
    ACTION_ROLL_RIGHT   = 3,
    ACTION_ROLL_NONE    = 4,
    ACTION_ABSORB_BLOOD = 5,
};

enum MiscData
{
    DATA_POS        = 1,
    POINT_SPAWNER   = 2,
    DATA_PLATES     = 3,
    DATA_BLAST_POS  = 4,
};

enum RollStage
{
    ROLL_NONE           = 0,
    ROLL_PRE_LEFT_1     = 1,
    ROLL_PRE_RIGHT_1    = 2,
    ROLL_PRE_LEFT_2     = 3,
    ROLL_PRE_RIGHT_2    = 4,
    ROLLING_LEFT        = 5,
    ROLLING_RIGHT       = 6,
};

const Position corruptionPos[8] = 
{
    {-13868.6f, -13667.3f, 262.836f, 0.0698132f}, // left 1
    {-13841.1f, -13666.9f, 262.795f, 3.01942f}, // right 1
    {-13868.5f, -13654.1f, 262.946f, 0.0174533f}, // left 2
    {-13842.8f, -13654.1f, 263.965f, 5.84685f}, // right 2
    {-13867.1f, -13638.5f, 264.783f, 2.33874f}, // left 3
    {-13841.2f, -13635.2f, 265.261f, 0.785398f}, // right 3
    {-13870.5f, -13614.4f, 266.88f, 1.95477f}, // left 4
    {-13839.3f, -13614.8f, 266.32f, 6.23082f} // right 4
};

const Position tendonsPos[6] =
{
    {-13862.8f, -13645.f, 265.752f, 1.5708f}, // left 1
    {-13846.8f, -13644.7f, 265.789f, 1.5708f}, // right 1
    {-13862.6f, -13626.3f, 266.59f, 1.5708f}, // left 2
    {-13846.6f, -13626.f, 266.638f, 1.5708f}, // right 2
    {-13862.6f, -13604.9f, 269.227f, 1.5708f}, // left 3
    {-13846.6f, -13604.7f, 269.174f, 1.5708f} // right 3
};

class npc_spine_of_deathwing_deathwing : public CreatureScript
{
    public:
        npc_spine_of_deathwing_deathwing() : CreatureScript("npc_spine_of_deathwing_deathwing") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_spine_of_deathwing_deathwingAI>(pCreature);
        }

        struct npc_spine_of_deathwing_deathwingAI : public Scripted_NoMovementAI
        {
            npc_spine_of_deathwing_deathwingAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
                me->setActive(true);
                instance = me->GetInstanceScript();
                rollStage = ROLL_NONE;
                destroyedPlates = 0;
                achieve = 0;
            }

            void DoAction(const int32 action)
            {
                switch (action)
                {
                    case ACTION_START_BATTLE:
                        events.ScheduleEvent(EVENT_CHECK_PLAYERS, 10000);
                        //ResetBattle(false, false);
                        StartBattle();
                        DoCast(me, SPELL_ROLL_CONTROL);
                        break;
                    case ACTION_ROLL_LEFT:
                        if (!rollleft && !rollright)
                        {
                            rollleft = true;
                            events.ScheduleEvent(EVENT_PRE_ROLL_L_1, 2000);
                        }
                        break;
                    case ACTION_ROLL_RIGHT:
                        if (!rollright && !rollleft)
                        {
                            rollright = true;
                            events.ScheduleEvent(EVENT_PRE_ROLL_R_1, 2000);
                        }
                        break;
                    case ACTION_ROLL_NONE:
                        rollStage = ROLL_NONE; 
                        break;
                }
            }

            bool AllowAchieve()
            {
                return (achieve == 4);
            }
            
            void SpellHitTarget(Unit* target, const SpellInfo* spell) override
            {
                if (spell->Id == SPELL_ROLL_CONTROL_AOE_1)
                {
                    if (rolling)
                    {
                        if (target->GetPositionX() >= -13850.3 && rollright)
                            target->CastSpell(target->GetPositionX() + 50, target->GetPositionY() - 100.0f, target->GetPositionZ() + 30.0f, SPELL_ROLL_CONTROL_AOE_2, true);

                        if (target->GetPositionX() <= -13860 && rollleft)
                            target->CastSpell(target->GetPositionX() + 50, target->GetPositionY() - 100.0f, target->GetPositionZ() + 30.0f, SPELL_ROLL_CONTROL_AOE_2, true);
                    }

                }
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == DATA_PLATES)
                {
                    destroyedPlates = (uint8)data;
                    if (instance)
                    {
                        if (destroyedPlates == 1)
                        {
                            Talk(SAY_DEATHWING_PLATE);
                            instance->HandleGameObject(instance->GetGuidData(DATA_BACK_PLATE_1), true);
                            for (uint8 i = 4; i < 6; ++i)
                                if (Creature* pCorruption = me->SummonCreature(NPC_CORRUPTION_1, corruptionPos[i]))
                                {
                                    pCorruption->AI()->SetData(DATA_POS, i);
                                    DoZoneInCombat(pCorruption);
                                }
                        }
                        else if (destroyedPlates == 2)
                        {
                            Talk(SAY_DEATHWING_PLATE);
                            instance->HandleGameObject(instance->GetGuidData(DATA_BACK_PLATE_2), true);
                            for (uint8 i = 6; i < 8; ++i)
                                if (Creature* pCorruption = me->SummonCreature(NPC_CORRUPTION_1, corruptionPos[i]))
                                {
                                    pCorruption->AI()->SetData(DATA_POS, i);
                                    DoZoneInCombat(pCorruption);
                                }
                        }
                        else if (destroyedPlates == 3)
                        {
                            Talk(SAY_DEATHWING_PLATE);
                            instance->HandleGameObject(instance->GetGuidData(DATA_BACK_PLATE_3), true);
                            events.ScheduleEvent(EVENT_END_ENCOUNTER, 1000);
                        }
                    }
                }
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_PLATES)
                    return (uint32)destroyedPlates;
                return 0;
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage, DamageEffectType dmgType)
            {
                if (me->GetHealth() <= damage)
                    damage = 0;
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_END_ENCOUNTER:
                            events.Reset();
                            ResetBattle(true);
                            DoCastAOE(SPELL_PLAY_MOVIE);
                            if (instance)
                            {
                                if (achieve >= 4)
                                    instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 104574);

                                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SEARING_PLASMA);
                                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SEARING_PLASMA_AOE);
                                switch (GetDifficultyID())
                                {
                                    case DIFFICULTY_10_N:
                                        instance->SetData(DATA_SPAWN_GREATER_CHEST, DATA_GREATER_CACHE_10N);
                                        break;
                                    case DIFFICULTY_25_N:
                                        instance->SetData(DATA_SPAWN_GREATER_CHEST, DATA_GREATER_CACHE_25N);
                                        break;
                                    case DIFFICULTY_10_HC:
                                        instance->SetData(DATA_SPAWN_GREATER_CHEST, DATA_GREATER_CACHE_10H);
                                        instance->DoCompleteAchievement(6115);
                                        break;
                                    case DIFFICULTY_25_HC:
                                        instance->SetData(DATA_SPAWN_GREATER_CHEST, DATA_GREATER_CACHE_25H);
                                        instance->DoCompleteAchievement(6115);
                                        break;
                                }
                                instance->DoStartMovie(75);
                                Map::PlayerList const &plrList = instance->instance->GetPlayers();
                                if (!plrList.isEmpty())
                                    for (Map::PlayerList::const_iterator i = plrList.begin(); i != plrList.end(); ++i)
                                        if (Player* pPlayer = i->getSource())
                                        {
                                            pPlayer->NearTeleportTo(customPos[4].GetPositionX(), customPos[4].GetPositionY(), customPos[4].GetPositionZ(), customPos[4].GetOrientation());
                                            if (!pPlayer->isDead())
                                                pPlayer->CastSpell(pPlayer, SPELL_PARACHUTE, true);
                                        }
                            }
                            break;
                        case EVENT_SUMMON_CORRUPTED_BLOOD:
                        {
                            std::list<Creature*> targets;
                            me->GetCreatureListWithEntryInGrid(targets, NPC_SPAWNER, 200.0f);
                            uint32 num = 0;
                            if (!targets.empty())
                            {
                                num = targets.size();
                                Unit* pSpawner = Trinity::Containers::SelectRandomContainerElement(targets);
                                if (Creature* pBlood = me->SummonCreature(NPC_CORRUPTED_BLOOD, *pSpawner))
                                    DoZoneInCombat(pBlood);
                            }
                            events.ScheduleEvent(EVENT_SUMMON_CORRUPTED_BLOOD, (IsHeroic() ? (8000 - num * 500) : 8000));
                            break;
                        }
                        case EVENT_TALK:
                            Talk(SAY_DEATHWING_TALK);
                            events.ScheduleEvent(EVENT_TALK, urand(35000, 45000));
                            break;
                        case EVENT_CHECK_PLAYERS:
                            if (instance)
                            {
                                if ((instance->GetBossState(DATA_SPINE) != IN_PROGRESS) || !CheckPlayers())
                                {
                                    ResetBattle();
                                }
                                else
                                    events.ScheduleEvent(EVENT_CHECK_PLAYERS, 1000);
                            }
                            break;
                        case EVENT_PRE_ROLL_L_1:
                            Talk(SAY_DEATHWING_LEFT_ROLL_PRE);
                            events.ScheduleEvent(EVENT_PRE_ROLL_L_2, 5000);
                            break;
                        case EVENT_PRE_ROLL_L_2:
                            Talk(SAY_DEATHWING_LEFT_ROLL);
                            events.ScheduleEvent(EVENT_ROLL_L, 7000);
                            break;
                        case EVENT_ROLL_L:
                            DoRoll(1);
                            if (achieve == 1)
                                achieve = 0;
                            else
                                achieve++;
                            break;
                        case EVENT_PRE_ROLL_R_1:
                            Talk(SAY_DEATHWING_RIGHT_ROLL_PRE);
                            events.ScheduleEvent(EVENT_PRE_ROLL_R_2, 5000);
                            break;
                        case EVENT_PRE_ROLL_R_2:
                            Talk(SAY_DEATHWING_RIGHT_ROLL);
                            events.ScheduleEvent(EVENT_ROLL_R, 7000);
                            break;
                        case EVENT_ROLL_R:
                            DoRoll(2);
                            if (achieve == 0)
                                achieve = 0;
                            else
                                achieve++;
                            break;
                    }
                } 
            }

        private:
            EventMap events;
            InstanceScript* instance;
            RollStage rollStage;
            uint8 destroyedPlates;
            int8 achieve;
            bool rollright = false;
            bool rollleft = false;
            bool rolling = false;

            void DoRoll(uint8 side /* 1 - left, 2 - right*/)
            {
                rolling = true;

                DespawnCreatures(NPC_HIDEOUS_AMALGAMATION);

                me->AddDelayedEvent(5000, [this] {
                    rollright = false;
                    rollleft = false;
                    rolling = false;
                });
            }

            void ResetBattle(bool done = false, bool despawn = true)
            {
                DespawnCreatures(NPC_HIDEOUS_AMALGAMATION);
                DespawnCreatures(NPC_CORRUPTION_1);
                DespawnCreatures(NPC_CORRUPTION_2);
                DespawnCreatures(NPC_CORRUPTION_3);
                DespawnCreatures(NPC_SPAWNER);
                DespawnCreatures(NPC_CORRUPTED_BLOOD);
                DespawnCreatures(NPC_BURNING_TENDONS_1);
                DespawnCreatures(NPC_BURNING_TENDONS_2);

                if (instance)
                {
                    instance->SetBossState(DATA_SPINE, (done ? DONE : NOT_STARTED));
                    if (!done)
                    {
                        instance->HandleGameObject(instance->GetGuidData(DATA_BACK_PLATE_1),  false);
                        instance->HandleGameObject(instance->GetGuidData(DATA_BACK_PLATE_2),  false);
                        instance->HandleGameObject(instance->GetGuidData(DATA_BACK_PLATE_3),  false);

                        Map::PlayerList const& players = instance->instance->GetPlayers();
                        if (!players.isEmpty())
                            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                                if (Player* players = itr->getSource())
                                    players->TeleportTo(967, customPos[0].GetPositionX(), customPos[0].GetPositionY(), customPos[0].GetPositionZ(), customPos[0].GetOrientation());
                    }
                }
                
                if (despawn)
                    me->DespawnOrUnsummon(2000);
            }

            void StartBattle()
            {
                for (uint8 i = 0; i < 4; ++i)
                    if (Creature* pCorruption = me->SummonCreature(NPC_CORRUPTION_1, corruptionPos[i]))
                    {
                        pCorruption->AI()->SetData(DATA_POS, i);
                        DoZoneInCombat(pCorruption);
                    }

                for (uint8 i = 0; i < 6; ++i)
                    if (Creature* pTendons = me->SummonCreature((((i % 2) == 1) ? NPC_BURNING_TENDONS_2 : NPC_BURNING_TENDONS_1), tendonsPos[i]))
                    {
                        pTendons->AI()->SetData(DATA_POS, i);
                        DoZoneInCombat(pTendons);
                    }

                rollStage = ROLL_NONE;
                events.ScheduleEvent(EVENT_CHECK_ROLL, 3000);
                events.ScheduleEvent(EVENT_TALK, urand(3000, 5000));
                events.ScheduleEvent(EVENT_SUMMON_CORRUPTED_BLOOD, 10000);
                me->setFaction(14);
            }

            void DespawnCreatures(uint32 entry)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, me, entry, 1000.0f);

                if (creatures.empty())
                   return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                     (*iter)->DespawnOrUnsummon();
            }

            bool CheckPlayers()
            {
                Player* player = NULL;
                AnyLivePlayerNoGmCheck check(me, 200.0f);
                Trinity::PlayerSearcher<AnyLivePlayerNoGmCheck> searcher(me, player, check);
                Trinity::VisitNearbyWorldObject(me, 200.0f, searcher);
                return (player ? true : false);
            }

            class AnyLivePlayerNoGmCheck
            {
                public:
                    AnyLivePlayerNoGmCheck(WorldObject const* obj, float range, bool withAura = false) : _obj(obj), _range(range), _withAura(withAura) {}
                    bool operator()(Player* u)
                    {
                        if (!u->isAlive())
                            return false;

                        if (!_obj->IsWithinDistInMap(u, _range))
                            return false;

                        if (u->isGameMaster())
                            return false;

                        if (_withAura)
                        {
                            uint32 spellId1 = SPELL_GRASPING_TENDRILS_10;
                            uint32 spellId2 = SPELL_FIERY_GRIP;

                            if (u->HasAura(spellId1) || u->HasAura(spellId2))
                                return false;
                        }

                        return true;
                    }

                private:
                    WorldObject const* _obj;
                    float _range;
                    bool _withAura;
            };
        };
};

class npc_spine_of_deathwing_corruption : public CreatureScript
{
    public:
        npc_spine_of_deathwing_corruption() : CreatureScript("npc_spine_of_deathwing_corruption") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_spine_of_deathwing_corruptionAI>(pCreature);
        }

        struct npc_spine_of_deathwing_corruptionAI : public Scripted_NoMovementAI
        {
            npc_spine_of_deathwing_corruptionAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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
                damageCounter = 0;
                isGrip = false;
                instance = me->GetInstanceScript();
            }

            void IsSummonedBy(Unit* /*who*/)
            {
                if (Creature* pSpawner = me->FindNearestCreature(NPC_SPAWNER, 5.0f, true))
                    pSpawner->DespawnOrUnsummon();
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (!instance->GetData(DATA_IS_LFR))
                {
                    events.ScheduleEvent(EVENT_SEARING_PLASMA, urand(1000, 8000));
                    events.ScheduleEvent(EVENT_FIERY_GRIP, urand(31000, 33000));
                }
            }

            void DamageTaken(Unit* /*who*/, uint32 &damage, DamageEffectType dmgType)
            {
                if (!isGrip)
                    return;

                if (damageCounter <= damage)
                {
                    damageCounter = 0;
                    isGrip = false;
                    me->InterruptSpell(CURRENT_CHANNELED_SPELL);
                }
                else
                    damageCounter -= damage;
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == DATA_POS)
                    pos = (uint8)data;
            }

            void JustDied(Unit* /*who*/)
            {
                events.Reset();
                if (!me->FindNearestCreature(NPC_CORRUPTION_1, 200.0f, true))
                {
                    uint8 i = (((pos % 2) == 1) ? pos - 1 : pos + 1);
                    if (Creature* pCorruption = me->SummonCreature(NPC_CORRUPTION_1, corruptionPos[i]))
                    {
                        pCorruption->AI()->SetData(DATA_POS, i);
                        DoZoneInCombat(pCorruption);
                    }
                }
                me->SummonCreature(NPC_SPAWNER, *me);
                    
                me->DespawnOrUnsummon(2000);
            }

            void UpdateAI(uint32 diff)
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
                        case EVENT_FIERY_GRIP:
                        {
                            Unit* pTarget = NULL;
                            pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true, -SPELL_FIERY_GRIP);
                            if (!pTarget)
                                pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true);
                            if (pTarget)
                            {
                                isGrip = true;
                                damageCounter = me->CountPctFromMaxHealth(20);
                                me->CastCustomSpell(SPELL_FIERY_GRIP, SPELLVALUE_MAX_TARGETS, RAID_MODE(1, 3), pTarget, false);
                            }
                            events.ScheduleEvent(EVENT_FIERY_GRIP, urand(31000, 33000));
                            break;
                        }
                        case EVENT_SEARING_PLASMA:
                            DoCastAOE(SPELL_SEARING_PLASMA_AOE);
                            events.ScheduleEvent(EVENT_SEARING_PLASMA, urand(15000, 20000));
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap events;
            InstanceScript* instance;
            uint32 damageCounter;
            bool isGrip;
            uint8 pos;
        };
};

class npc_spine_of_deathwing_hideous_amalgamation : public CreatureScript
{
    public:
        npc_spine_of_deathwing_hideous_amalgamation() : CreatureScript("npc_spine_of_deathwing_hideous_amalgamation") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_spine_of_deathwing_hideous_amalgamationAI>(pCreature);
        }

        struct npc_spine_of_deathwing_hideous_amalgamationAI : public ScriptedAI
        {
            npc_spine_of_deathwing_hideous_amalgamationAI(Creature* pCreature) : ScriptedAI(pCreature)
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
                
                isExplode = false;
            }

            void Reset()
            {
                events.Reset();
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                if (!isExplode && me->isAlive())
                    if (Unit* target = me->SelectNearestTarget(100))
                        AttackStart(target);
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_ABSORB_BLOOD)
                {
                    DoCast(me, SPELL_ABSORBED_BLOOD, true);
                    /*if (me->GetPower(POWER_ALTERNATE) < 9)
                    {
                        me->SetPower(POWER_ALTERNATE, me->GetPower(POWER_ALTERNATE) + 1);
                        if (me->GetPower(POWER_ALTERNATE) == 9)
                            DoCast(me, SPELL_SUPERHEATED_NUCLEUS, true);
                    }*/
                }
            }

            void JustDied(Unit* /*who*/)
            {
                if (IsHeroic())
                {
                    if (InstanceScript* instance = me->GetInstanceScript())
                    {
                        Map::PlayerList const& plrList = instance->instance->GetPlayers();
                        if (!plrList.isEmpty())
                            for (Map::PlayerList::const_iterator itr = plrList.begin(); itr != plrList.end(); ++itr)
                                if (Player* pPlayer = itr->getSource())
                                    if (pPlayer->GetDistance(me) <= 200.0f)
                                        pPlayer->AddAura(SPELL_DEGRADATION, pPlayer);
                    }
                    //DoCastAOE(SPELL_DEGRADATION, true);
                }
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_BLAST_POS)
                {
                    if (me->GetPositionX() > -13870.0f && me->GetPositionX() <= -13854.787109f &&
                        me->GetPositionY() > -13660.0f && me->GetPositionY() < -13640.0f)
                        return 1;
                    if (me->GetPositionX() >= -13854.787109f && me->GetPositionX() <= -13830.0f &&
                        me->GetPositionY() > -13660.0f && me->GetPositionY() < -13640.0f)
                        return 2;
                    if (me->GetPositionX() > -13870.0f && me->GetPositionX() <= -13854.787109f &&
                        me->GetPositionY() > -13640.0f && me->GetPositionY() < -13620.0f)
                        return 3;
                    if (me->GetPositionX() >= -13854.787109f && me->GetPositionX() <= -13830.0f &&
                        me->GetPositionY() > -13640.0f && me->GetPositionY() < -13620.0f)
                        return 4;
                    if (me->GetPositionX() > -13870.0f && me->GetPositionX() <= -13854.787109f &&
                        me->GetPositionY() > -13620.0f && me->GetPositionY() < -13600.0f)
                        return 5;
                    if (me->GetPositionX() >= -13854.787109f && me->GetPositionX() <= -13830.0f &&
                        me->GetPositionY() > -13620.0f && me->GetPositionY() < -13600.0f)
                        return 6;
                }
                return 0;
            }

            void EnterCombat(Unit* /*who*/)
            {
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_BLOOD_CORRUPTION, urand(6000, 11000));
            }

            void DamageTaken(Unit* /*who*/, uint32& damage, DamageEffectType dmgType)
            {
                if (me->GetHealth() <= damage)
                {
                    if (me->GetAura(SPELL_SUPERHEATED_NUCLEUS))
                    {
                        damage = 0;
                        if (!isExplode)
                        {
                            isExplode = true;
                            Talk(ANN_NUCLEAR_BLAST);
                            DoCast(SPELL_NUCLEAR_BLAST);
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                        }
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_BLOOD_CORRUPTION)
                    {
                        DoCastAOE(SPELL_BLOOD_CORRUPTION_DEATH);
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            bool isExplode;
            EventMap events;
        };
};

class npc_spine_of_deathwing_spawner : public CreatureScript
{
    public:
        npc_spine_of_deathwing_spawner() : CreatureScript("npc_spine_of_deathwing_spawner") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_spine_of_deathwing_spawnerAI>(pCreature);
        }

        struct npc_spine_of_deathwing_spawnerAI : public Scripted_NoMovementAI
        {
            npc_spine_of_deathwing_spawnerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void Reset()
            {
                events.Reset();
                events.ScheduleEvent(EVENT_SUMMON_HIDEOUS_AMALGAMATION, 2000);
                DoCast(me, SPELL_GRASPING_TENDRILS, true);
            }

            void UpdateAI(uint32 diff)
            {
                events.Update(diff);
                switch (events.ExecuteEvent())
                {
                    case EVENT_SUMMON_HIDEOUS_AMALGAMATION:
                        me->SummonCreature(NPC_HIDEOUS_AMALGAMATION, *me);
                        break;
                }
            }

        private:
            EventMap events;
        };
};

class npc_spine_of_deathwing_corrupted_blood : public CreatureScript
{
    public:
        npc_spine_of_deathwing_corrupted_blood() : CreatureScript("npc_spine_of_deathwing_corrupted_blood") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_spine_of_deathwing_corrupted_bloodAI>(pCreature);
        }

        struct npc_spine_of_deathwing_corrupted_bloodAI : public ScriptedAI
        {
            npc_spine_of_deathwing_corrupted_bloodAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                
                isDead = false;
            }

            void Reset()
            {
                if (!isDead && me->isAlive())
                    if (Unit* target = me->SelectNearestTarget(100))
                        AttackStart(target);
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type == POINT_MOTION_TYPE)
                    if (id == POINT_SPAWNER)
                    {
                        isDead = false;
                        me->SetSpeed(MOVE_RUN, 1.0f, true);
                        me->SetSpeed(MOVE_SWIM, 1.0f, true);
                        me->SetSpeed(MOVE_WALK, 1.0f, true);
                        me->SetSpeed(MOVE_FLIGHT, 1.0f, true);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveAura(SPELL_RESIDUE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        {
                            me->GetMotionMaster()->MoveChase(pTarget);
                            me->Attack(pTarget, true);
                        }
                    }
            }

            void DamageTaken(Unit* /*who*/, uint32 & damage, DamageEffectType dmgType)
            {
                if (me->GetHealth() <= damage)
                {
                    damage = 0;
                    if (!isDead)
                    {
                        isDead = true;
                        DoResetThreat();
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        DoCastAOE(SPELL_BURST, true);
                        DoCast(me, SPELL_RESIDUE, true);
                        me->SetSpeed(MOVE_RUN, 0.03f, true);
                        me->SetSpeed(MOVE_SWIM, 0.03f, true);
                        me->SetSpeed(MOVE_WALK, 0.03f, true);
                        me->SetSpeed(MOVE_FLIGHT, 0.03f, true);
                        events.ScheduleEvent(EVENT_MOVE_TO_SPAWNER, 5000);
                    }
                }
            }

            void UpdateAI(uint32 diff)
            {
                UpdateVictim();

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_MOVE_TO_SPAWNER)
                    {
                        if (Creature* pSpawner = me->FindNearestCreature(NPC_SPAWNER, 200.0f, true))
                            me->GetMotionMaster()->MovePoint(POINT_SPAWNER, *pSpawner);
                    }
                }

                DoMeleeAttackIfReady();
             }

        private:
            EventMap events;
            bool isDead;
        };
};

class npc_spine_of_deathwing_burning_tendons : public CreatureScript
{
    public:
        npc_spine_of_deathwing_burning_tendons() : CreatureScript("npc_spine_of_deathwing_burning_tendons") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return GetInstanceAI<npc_spine_of_deathwing_burning_tendonsAI>(pCreature);
        }

        struct npc_spine_of_deathwing_burning_tendonsAI : public Scripted_NoMovementAI
        {
            npc_spine_of_deathwing_burning_tendonsAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
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
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                isOpened = false;
            }

            void SpellHit(Unit* /*who*/, const SpellInfo* spellInfo)
            {
                if (spellInfo->Id == SPELL_NUCLEAR_BLAST_SCRIPT && !isOpened)
                {
                    isOpened = true;
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    DoCast(me, ((pos % 2) == 1) ? SPELL_BREACH_ARMOR_2 : SPELL_BREACH_ARMOR_1, true);
                    DoCast(me, ((pos % 2) == 1) ? SPELL_SEAL_ARMOR_BREACH_2 : SPELL_SEAL_ARMOR_BREACH_1);
                    events.ScheduleEvent(EVENT_CHECK_CASTING, 1000);
                }
                else if ((spellInfo->Id == SPELL_SEAL_ARMOR_BREACH_2 || spellInfo->Id == SPELL_SEAL_ARMOR_BREACH_1) && isOpened)
                {
                    isOpened = false;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    events.Reset();
                }
            }

            void DamageTaken(Unit* /*who*/, uint32& damage, DamageEffectType dmgType)
            {
                if (!isOpened)
                    damage = 0;
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == DATA_POS)
                    pos = (uint8)data;
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_POS)
                    return pos;
                return 0;
            }

            void EnterEvadeMode()
            {
                return;
            }

            void JustDied(Unit* /*who*/)
            {
                DoCast(me, ((pos % 2) == 1) ? SPELL_PLATE_FLY_OFF_RIGHT : SPELL_PLATE_FLY_OFF_LEFT, true);
                if (Creature* pDeathwing = me->FindNearestCreature(NPC_SPINE_OF_DEATHWING, 300.0f))
                {
                    pDeathwing->AI()->SetData(DATA_PLATES, pDeathwing->AI()->GetData(DATA_PLATES) + 1);
                }
                me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) 
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (isOpened)
                    {
                        if (!me->HasUnitState(UNIT_STATE_CASTING))
                        {
                            isOpened = false;
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        }
                        events.ScheduleEvent(EVENT_CHECK_CASTING, 1000);
                    }
                }
            }

        private:
            uint8 pos;
            bool isOpened;
            EventMap events;
        };
};

class spell_spine_of_deathwing_roll_control_check : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_roll_control_check() : SpellScriptLoader("spell_spine_of_deathwing_roll_control_check") { }

        class spell_spine_of_deathwing_roll_control_check_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spine_of_deathwing_roll_control_check_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (!GetCaster()->GetInstanceScript())
                    return;

                if (targets.empty())
                    return;

                uint8 min_diff = 0;
                uint8 playersLeft = 0;
                uint8 playersRight = 0;

                switch (GetCaster()->GetInstanceScript()->instance->GetDifficultyID())
                {
                    case DIFFICULTY_10_N:
                    case DIFFICULTY_10_HC:
                        min_diff = 4;
                        break;
                    case DIFFICULTY_25_N:
                    case DIFFICULTY_25_HC:
                        min_diff = 6;
                        break;
                }
                if (targets.size() <= min_diff)
                    min_diff = targets.size() == 1 ? 1 : targets.size() / 2;

                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                {
                    if ((*itr)->GetPositionX() >= -13850.3)
                        playersRight++;
                    else if ((*itr)->GetPositionX() <= -13860)
                        playersLeft++;
                }

                if (Creature* pDeathwing = GetCaster()->ToCreature())
                {
                    if (playersLeft > playersRight && (playersLeft - playersRight) >= min_diff)
                        pDeathwing->AI()->DoAction(ACTION_ROLL_LEFT);
                    else if (playersRight > playersLeft && (playersRight - playersLeft) >= min_diff)
                        pDeathwing->AI()->DoAction(ACTION_ROLL_RIGHT);
                    else
                        pDeathwing->AI()->DoAction(ACTION_ROLL_NONE);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spine_of_deathwing_roll_control_check_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spine_of_deathwing_roll_control_check_SpellScript();
        }
};

class spell_spine_of_deathwing_searing_plasma_aoe : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_searing_plasma_aoe() : SpellScriptLoader("spell_spine_of_deathwing_searing_plasma_aoe") { }

        class spell_spine_of_deathwing_searing_plasma_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spine_of_deathwing_searing_plasma_aoe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;
                
                Trinity::Containers::RandomResizeList(targets, 1);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_SEARING_PLASMA, true);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spine_of_deathwing_searing_plasma_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_spine_of_deathwing_searing_plasma_aoe_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spine_of_deathwing_searing_plasma_aoe_SpellScript();
        }
};

class spell_spine_of_deathwing_nuclear_blast : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_nuclear_blast() : SpellScriptLoader("spell_spine_of_deathwing_nuclear_blast") { }

        class spell_spine_of_deathwing_nuclear_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spine_of_deathwing_nuclear_blast_SpellScript);

            void HandleAfterCast()
            {
                if (!GetCaster())
                    return;

                GetCaster()->CastSpell(GetCaster(), SPELL_NUCLEAR_BLAST_SCRIPT, true);
                GetCaster()->Kill(GetCaster());
                //if (Creature* pAmalgamation = GetCaster()->ToCreature())
                //    pAmalgamation->AI()->JustDied(pAmalgamation);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_spine_of_deathwing_nuclear_blast_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spine_of_deathwing_nuclear_blast_SpellScript();
        }
};

class spell_spine_of_deathwing_absorb_blood : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_absorb_blood() : SpellScriptLoader("spell_spine_of_deathwing_absorb_blood") { }

        class spell_spine_of_deathwing_absorb_blood_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spine_of_deathwing_absorb_blood_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (Creature* pAmalgamation = GetCaster()->ToCreature())
                {
                    Aura* aur = NULL;
                    aur = pAmalgamation->GetAura(SPELL_ABSORBED_BLOOD);
                    if (!aur || aur->GetStackAmount() < 9)
                        if (Creature* pBlood = GetHitUnit()->ToCreature())
                        {
                            pBlood->RemoveAura(SPELL_RESIDUE);
                            pBlood->DespawnOrUnsummon(1000);
                            pAmalgamation->AI()->DoAction(ACTION_ABSORB_BLOOD);
                        }                 
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_spine_of_deathwing_absorb_blood_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spine_of_deathwing_absorb_blood_SpellScript();
        }
};

class spell_spine_of_deathwing_nuclear_blast_script : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_nuclear_blast_script() : SpellScriptLoader("spell_spine_of_deathwing_nuclear_blast_script") { }

        class spell_spine_of_deathwing_nuclear_blast_script_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spine_of_deathwing_nuclear_blast_script_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;

                if (Player* pPlayer = GetCaster()->ToPlayer())
                {
                    if (targets.size() > 1)
                    {
                        targets.sort(DistancePred(GetCaster()));
                        targets.resize(1);
                    }
                    return;
                }

                targets.clear();

                if (Creature* pDeathwing = GetCaster()->FindNearestCreature(NPC_SPINE_OF_DEATHWING, 200.0f, true))
                {
                    uint8 cur_plate = pDeathwing->AI()->GetData(DATA_PLATES);
                    uint8 pos = GetCaster()->ToCreature()->AI()->GetData(DATA_BLAST_POS);
                    if ((cur_plate == 0 && pos != 1 && pos != 2) ||
                        (cur_plate == 1 && pos != 3 && pos != 4) ||
                        (cur_plate == 2 && pos != 5 && pos != 6) ||
                        (cur_plate >= 3))
                        return;

                    Creature* pTendons = GetCaster()->FindNearestCreature((((pos % 2) == 0) ? NPC_BURNING_TENDONS_2 : NPC_BURNING_TENDONS_1), 20.0f, true);
                    if (!pTendons)
                        return;

                    uint8 tendons_pos = pTendons->AI()->GetData(DATA_POS);

                    if ((pos - 1) != tendons_pos)
                        return;

                    targets.push_back(pTendons);
                }
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spine_of_deathwing_nuclear_blast_script_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
            }

        private:
            class DistancePred
            {
                public:
                    DistancePred(Unit* searcher) :  _searcher(searcher) { }
                    bool operator() (WorldObject* a, WorldObject* b) const
                    {
                        float rA = _searcher->GetDistance(a);
                        float rB = _searcher->GetDistance(b);
                        return rA < rB;
                    }
                private:
                    Unit const* _searcher;
            };
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spine_of_deathwing_nuclear_blast_script_SpellScript();
        }
};

class spell_spine_of_deathwing_blood_corruption : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_blood_corruption() : SpellScriptLoader("spell_spine_of_deathwing_blood_corruption") { }

        class spell_spine_of_deathwing_blood_corruption_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_spine_of_deathwing_blood_corruption_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                uint32 cur_duration = GetDuration();
                if (Unit* owner = GetUnitOwner())
                {
                    Unit* target = NULL;
                    std::list<Player*> players;
                    PlayersCheck check(owner);
                    Trinity::PlayerListSearcher<PlayersCheck> searcher(owner, players, check);
                    Trinity::VisitNearbyObject(owner, 200.0f, searcher);
                    if (!players.empty())
                        target = Trinity::Containers::SelectRandomContainerElement(players);
                    else
                        target = owner;

                    if (target)
                    {   
                        if (Aura* aur = owner->AddAura(GetId() == SPELL_BLOOD_CORRUPTION_EARTH ? SPELL_BLOOD_CORRUPTION_EARTH : (roll_chance_i(GetCharges() * 20) ? SPELL_BLOOD_CORRUPTION_EARTH : SPELL_BLOOD_CORRUPTION_DEATH), target))
                        {
                            aur->SetDuration(cur_duration);
                            aur->SetCharges(GetCharges() + 1);
                            aur->SetAuraAttribute(AURA_ATTR_IS_USING_CHARGES, false);
                        }
                    }
                }
            }

            void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* owner = GetUnitOwner())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_EXPIRE || removeMode == AURA_REMOVE_BY_DEATH)
                        owner->CastSpell(owner, (GetId() == SPELL_BLOOD_CORRUPTION_EARTH) ? SPELL_BLOOD_OF_NELTHARION : SPELL_BLOOD_OF_DEATHWING, true);
                }
            }

            void Register()
            {
                AfterDispel += AuraDispelFn(spell_spine_of_deathwing_blood_corruption_AuraScript::HandleDispel);
                AfterEffectRemove += AuraEffectRemoveFn(spell_spine_of_deathwing_blood_corruption_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }

        private:
            class PlayersCheck
            {
                public:
                    PlayersCheck(WorldObject const* obj) : _obj(obj) {}
                    bool operator()(Player* u)
                    {
                        if (!u->isAlive())
                            return false;

                        if (u->isGameMaster())
                            return false;

                        if (!_obj->IsWithinDistInMap(u, 200.0f))
                            return false;

                        if (u->HasAura(SPELL_BLOOD_CORRUPTION_DEATH) ||
                            u->HasAura(SPELL_BLOOD_CORRUPTION_EARTH))
                            return false;

                        if (_obj->GetGUID() == u->GetGUID())
                            return false;

                        return true;
                    }

                private:
                    WorldObject const* _obj;
            };
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_spine_of_deathwing_blood_corruption_AuraScript();
        }
};

class spell_spine_of_deathwing_roll_control : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_roll_control() : SpellScriptLoader("spell_spine_of_deathwing_roll_control") { }

        class spell_spine_of_deathwing_roll_control_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_spine_of_deathwing_roll_control_AuraScript);

            void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                if (Unit* owner = GetUnitOwner())
                    owner->Kill(owner);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_spine_of_deathwing_roll_control_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_spine_of_deathwing_roll_control_AuraScript();
        }
};

class spell_spine_of_deathwing_blood_corruption_death : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_blood_corruption_death() : SpellScriptLoader("spell_spine_of_deathwing_blood_corruption_death") { }

        class spell_spine_of_deathwing_blood_corruption_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spine_of_deathwing_blood_corruption_death_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (!GetCaster())
                    return;

                if (targets.empty())
                    return;

                Trinity::Containers::RandomResizeList(targets, 1);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_spine_of_deathwing_blood_corruption_death_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spine_of_deathwing_blood_corruption_death_SpellScript();
        }
};

typedef npc_spine_of_deathwing_deathwing::npc_spine_of_deathwing_deathwingAI SpineAI;

class achievement_maybe_he_ll_get_dizzy : public AchievementCriteriaScript
{
    public:
        achievement_maybe_he_ll_get_dizzy() : AchievementCriteriaScript("achievement_maybe_he_ll_get_dizzy") { }

        bool OnCheck(Player* source, Unit* target)
        {
            if (!target)
                return false;

            if (SpineAI* spineAI = CAST_AI(SpineAI, target->GetAI()))
                return spineAI->AllowAchieve();

            return false;
        }
};

class spell_spine_of_deathwing_absorbed_blood_stack : public SpellScriptLoader
{
    public:
        spell_spine_of_deathwing_absorbed_blood_stack() : SpellScriptLoader("spell_spine_of_deathwing_absorbed_blood_stack") { }

        class spell_spine_of_deathwing_absorbed_blood_stack_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_spine_of_deathwing_absorbed_blood_stack_AuraScript);

            void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster)
                        if (GetStackAmount() == 9)
                            caster->CastSpell(caster, SPELL_SUPERHEATED_NUCLEUS, true);
                }
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_spine_of_deathwing_absorbed_blood_stack_AuraScript::OnStackChange, EFFECT_2, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_spine_of_deathwing_absorbed_blood_stack_AuraScript();
        }
};

void AddSC_spine_of_deathwing()
{
    new npc_spine_of_deathwing_deathwing();
    new npc_spine_of_deathwing_corruption();
    new npc_spine_of_deathwing_hideous_amalgamation();
    new npc_spine_of_deathwing_spawner();
    new npc_spine_of_deathwing_corrupted_blood();
    new npc_spine_of_deathwing_burning_tendons();
    new spell_spine_of_deathwing_roll_control_check();
    new spell_spine_of_deathwing_searing_plasma_aoe();
    new spell_spine_of_deathwing_nuclear_blast();
    new spell_spine_of_deathwing_absorb_blood();
    new spell_spine_of_deathwing_nuclear_blast_script();
    new spell_spine_of_deathwing_blood_corruption();
    new spell_spine_of_deathwing_roll_control();
    new spell_spine_of_deathwing_blood_corruption_death();
    new spell_spine_of_deathwing_absorbed_blood_stack();
    //new achievement_maybe_he_ll_get_dizzy();
}