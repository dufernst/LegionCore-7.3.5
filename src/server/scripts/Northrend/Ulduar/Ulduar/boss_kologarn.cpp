/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Kologarn
SDAuthor: PrinceCreed
SD%Complete: 100%
SD%Comments: Kologarn's vehicleid is wrong.
EndScriptData */

#include "ulduar.h"
#include "Vehicle.h"

enum Spells
{
    // Kologarn
    SPELL_ARM_DEAD_DAMAGE                       = 63629,
    SPELL_TWO_ARM_SMASH                         = 63356,
    SPELL_ONE_ARM_SMASH                         = 63573,
    SPELL_ARM_SWEEP                             = 63766,
    SPELL_STONE_SHOUT                           = 63716,
    SPELL_PETRIFY_BREATH                        = 62030,
    SPELL_SHOCKWAVE                             = 63783,
    SPELL_SHOCKWAVE_VISUAL                      = 63788,
    SPELL_STONE_GRIP                            = 64290,
    SPELL_STONE_GRIP_STUN                       = 62056,
    SPELL_FOCUSED_EYEBEAM_PERIODIC_VISUAL       = 63369,
    SPELL_EYEBEAM_VISUAL_1                      = 63676,
    SPELL_EYEBEAM_VISUAL_2                      = 63702,
    
    SPELL_EYEBEAM_IMMUNITY                      = 64722,
    SPELL_ARM_RESPAWN                           = 64753
};

enum Events
{
    EVENT_NONE,
    EVENT_SMASH,
    EVENT_GRIP,
    EVENT_SWEEP,
    EVENT_SHOCKWAVE,
    EVENT_EYEBEAM,
    EVENT_STONESHOT,
    EVENT_RIGHT,
    EVENT_LEFT
};

enum Actions
{
    ACTIOM_REMOVE_RIGHT,
    ACTION_REMOVE_LEFT,
    ACTION_FAIL_ACHIV,
    ACTION_RESPAWN_RIGHT,
    ACTION_RESPAWN_LEFT,
    ACTION_GRIP
};

enum Npcs
{
    NPC_EYEBEAM_STALKER                         = 33001,
    NPC_EYEBEAM_STALKER_2                       = 33002, 
    NPC_EYEBEAM_1                               = 33632,
    NPC_EYEBEAM_2                               = 33802,
    NPC_RUBBLE                                  = 33768,
    NPC_LEFT_ARM                                = 32933,
    NPC_RIGHT_ARM                               = 32934,
};

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_SLAY                                    = 1,
    SAY_LEFT_ARM_GONE                           = 2,
    SAY_RIGHT_ARM_GONE                          = 3,
    SAY_SHOCKWAVE                               = 4,
    SAY_GRAB_PLAYER                             = 5,
    SAY_DEATH                                   = 6,
    SAY_BERSERK                                 = 7,
    EMOTE_STONE_GRIP                            = 8
};

#define EMOTE_LEFT                              "The Left Arm has regrown!"
#define EMOTE_RIGHT                             "The Right Arm has regrown!"
#define EMOTE_STONE                             "Kologarn casts Stone Grip!"

// Achievements
#define ACHIEVEMENT_LOOKS_COULD_KILL            RAID_MODE(2955, 2956) // TODO
#define ACHIEVEMENT_RUBBLE_AND_ROLL             RAID_MODE(2959, 2960)
#define ACHIEVEMENT_WITH_OPEN_ARMS              RAID_MODE(2951, 2952)
#define ACHIEV_DISARMED_START_EVENT             21687

//Spells
#define SPELL_FOCUSED_EYEBEAM_HELPER            RAID_MODE(63347, 63977) 
#define SPELL_FOCUSED_EYEBEAM_DAMAGE_HELPER     RAID_MODE(63346, 63976)

ObjectGuid GripTargetGUID[3];

const Position Lefteye      = {1788.8652f, -26.1113f, 470.4144f, 3.401f};
const Position Righteye     = {1788.8874f, -23.0051f, 470.4144f, 3.401f};
const Position RubbleLeft   = {1781.814f, -45.07f, 448.808f, 2.260f};
const Position RubbleRight  = {1781.814f, -3.716f, 448.808f, 4.211f};

enum KologarnChests
{
    CACHE_OF_LIVING_STONE_10                    = 195046,
    CACHE_OF_LIVING_STONE_25                    = 195047
};

//32930
class boss_kologarn : public CreatureScript
{
public:
    boss_kologarn() : CreatureScript("boss_kologarn") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_kologarnAI (pCreature);
    }

    struct boss_kologarnAI : public BossAI
    {
        boss_kologarnAI(Creature *pCreature) : BossAI(pCreature, BOSS_KOLOGARN), vehicle(pCreature->GetVehicleKit()),
            left(true), right(true)
        {
            me->SetDisableGravity(true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
            SetCombatMovement(false);
            

            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        Vehicle *vehicle;

        bool left;
        bool right;
        bool melee;
        bool Gripped;
        bool OpenArms;
        bool Beamachiv;
        uint32 RubbleCount;


        void Reset() override
        {
            _Reset();
            right = true;
            left = true;
            melee = true;
            OpenArms = true;
            Beamachiv = true;
            me->SetDisableGravity(true);
            me->LowerPlayerDamageReq(me->GetMaxHealth());
            if (instance)
                instance->DoStopTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, ACHIEV_DISARMED_START_EVENT);
        }
        
        void JustDied(Unit* Killer) override
        {                
            Talk(SAY_DEATH);
            _JustDied();

            if (instance)
            {
                 // Rubble and Roll
                if (RubbleCount > 4)
                    instance->DoCompleteAchievement(ACHIEVEMENT_RUBBLE_AND_ROLL);

                // Remove Stone Grip from players
                Map::PlayerList const &players = instance->instance->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    Player* pPlayer = itr->getSource();
 
                    if (!pPlayer)
                        continue;
 
                    if (pPlayer->HasAura(SPELL_STONE_GRIP_STUN))
                    {
                        pPlayer->RemoveAurasDueToSpell(RAID_MODE(64290, 64292));
                        pPlayer->RemoveAurasDueToSpell(SPELL_STONE_GRIP_STUN);
                        pPlayer->GetMotionMaster()->MoveJump(1767.80f, -18.38f, 448.808f, 10, 10);
                    }
                }
            }

            // Hack to disable corpse fall
            me->GetMotionMaster()->MoveTargetedHome();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->setFaction(35);
            // Chest spawn
            //me->SummonGameObject(RAID_MODE(CACHE_OF_LIVING_STONE_10, CACHE_OF_LIVING_STONE_25), 1836.52f, -36.11f, 448.81f, 0.56f, 0, 0, 1, 1, 604800);
            me->SummonGameObject(CACHE_OF_LIVING_STONE_10, 1836.52f, -36.11f, 448.81f, 0.56f, 0, 0, 1, 1, 604800);
        }

        bool Isright()
        {
            return right;
        }

        bool Isleft()
        {
            return left;
        }

        bool IsBeamachiv()
        {
            return Beamachiv;
        }

        bool IsOpenArms()
        {
            return OpenArms;
        }

       
        void KilledUnit(Unit* who) override
        {
            if (!(rand()%5))
                Talk(SAY_SLAY);
        }
        
        void EnterCombat(Unit* who) override
        {
            if (!instance->CheckRequiredBosses(BOSS_KOLOGARN, me->GetEntry(), who->ToPlayer()))
            {
                EnterEvadeMode();
                return;
            }

            Talk(SAY_AGGRO);
            _EnterCombat();
            
            RubbleCount = 0;
            Gripped = false;
            for (int32 n = 0; n < RAID_MODE(1, 3); ++n)
                GripTargetGUID[n].Clear();
            
            for (int32 n = 0; n < 2; ++n)
            {
                if (vehicle->GetPassenger(n))
                    vehicle->GetPassenger(n)->ToCreature()->AI()->DoZoneInCombat();
            }
            events.ScheduleEvent(EVENT_SMASH, 5000);
            events.ScheduleEvent(EVENT_SWEEP, 10000);
            events.ScheduleEvent(EVENT_EYEBEAM, 10000);
            events.ScheduleEvent(EVENT_SHOCKWAVE, 12000);
            events.ScheduleEvent(EVENT_GRIP, 40000);
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            
            if (!left || !right)
            {
                if (OpenArms)
                    OpenArms = false;
            }
                
            if (melee && (!left && !right))
            {
                me->SetReactState(REACT_PASSIVE);
                me->AttackStop();
                melee = false;
                DoCast(me->getVictim(), SPELL_STONE_SHOUT, true);
            }

            if (!melee && (left || right))
            {
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat();
                melee = true;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            
            if (events.GetTimer() > 15000 && !me->IsWithinMeleeRange(me->getVictim()))
                DoCastAOE(SPELL_PETRIFY_BREATH, true);
        
            switch(events.GetEvent())
            {
                case EVENT_NONE: break;
                case EVENT_SMASH:
                    if (left && right)
                    {
                        if (me->IsWithinMeleeRange(me->getVictim()))
                            DoCastVictim(SPELL_TWO_ARM_SMASH, true);
                    }
                    else if (left || right)
                    {
                        if (me->IsWithinMeleeRange(me->getVictim()))
                            DoCastVictim(SPELL_ONE_ARM_SMASH, true);
                    }
                    events.RescheduleEvent(EVENT_SMASH, 15000);
                    break;
                case EVENT_SWEEP:
                    if (left)
                        DoCastAOE(SPELL_ARM_SWEEP, true);
                    events.RescheduleEvent(EVENT_SWEEP, 15000);
                    break;
                case EVENT_GRIP:
                    if (right && instance)
                    {
                        if (Creature* RightArm = me->GetCreature(*me, instance->GetGuidData(DATA_RIGHT_ARM)))
                        {
                            Talk(EMOTE_STONE_GRIP);
                            Talk(SAY_GRAB_PLAYER);
                            // Grip up to 3 players
                            for (int32 n = 0; n < RAID_MODE(1, 3); ++n)
                            {
                                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 1, 40, true))
                                    if (!pTarget->HasAura(SPELL_STONE_GRIP_STUN))
                                        GripTargetGUID[n] = pTarget->GetGUID();
                            }
                            RightArm->ToCreature()->AI()->DoAction(ACTION_GRIP);
                        }
                    }
                    events.RescheduleEvent(EVENT_GRIP, 40000);
                    break;
                case EVENT_SHOCKWAVE:
                    if (left)
                    {
                        Talk(SAY_SHOCKWAVE);
                        DoCast(me, SPELL_SHOCKWAVE, true);
                        DoCast(me, SPELL_SHOCKWAVE_VISUAL, true);
                    }
                    events.RescheduleEvent(EVENT_SHOCKWAVE, urand(15000, 25000));
                    break;
                case EVENT_EYEBEAM:
                    if (Unit *pTarget = SelectTarget(SELECT_TARGET_FARTHEST, 1, 50, true))
                    {
                        if (Creature* EyeStalker = me->SummonCreature(NPC_EYEBEAM_STALKER, Lefteye,TEMPSUMMON_TIMED_DESPAWN,10000))
                        {
                            if (Creature* EyeBeam = me->SummonCreature(NPC_EYEBEAM_1,pTarget->GetPositionX(),pTarget->GetPositionY()+9,pTarget->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,10000))
                            {
                                EyeStalker->CastSpell(EyeBeam, SPELL_EYEBEAM_VISUAL_2, true);
                                EyeBeam->CastSpell(me, SPELL_EYEBEAM_VISUAL_1, true);
                                EyeBeam->AI()->AttackStart(pTarget);
                            }
                        }
                        if (Creature* EyeStalker = me->SummonCreature(NPC_EYEBEAM_STALKER, Righteye,TEMPSUMMON_TIMED_DESPAWN,10000))
                        {
                            if (Creature* EyeBeam = me->SummonCreature(NPC_EYEBEAM_2,pTarget->GetPositionX(),pTarget->GetPositionY()-9,pTarget->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,10000))
                            {
                                EyeStalker->CastSpell(EyeBeam, SPELL_EYEBEAM_VISUAL_2, true);
                                EyeBeam->CastSpell(me, SPELL_EYEBEAM_VISUAL_1, true);
                                EyeBeam->AI()->AttackStart(pTarget);
                            }
                        }
                            
                    }
                    events.RescheduleEvent(EVENT_EYEBEAM, 20000);
                    break;
                case EVENT_LEFT:
                    if (Creature* La = me->GetCreature(*me, instance->GetGuidData(DATA_LEFT_ARM)))
                    {
                        La->ToCreature()->AI()->DoAction(ACTION_REMOVE_LEFT);
                        me->CastSpell(La, SPELL_ARM_RESPAWN, true);
                    }
                    left = true;
                    events.CancelEvent(EVENT_LEFT);
                    break;                
                case EVENT_RIGHT:
                    if (Creature* Ra = me->GetCreature(*me, instance->GetGuidData(DATA_RIGHT_ARM)))
                    {
                        Ra->ToCreature()->AI()->DoAction(ACTIOM_REMOVE_RIGHT);
                        me->CastSpell(Ra, SPELL_ARM_RESPAWN, true);
                    }
                    right = true;
                    events.CancelEvent(EVENT_RIGHT);
                    break;
            }

            DoMeleeAttackIfReady();
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_RESPAWN_LEFT:
                    left = false;
                    Talk(SAY_LEFT_ARM_GONE);
                    me->DealDamage(me, int32(me->GetMaxHealth() * 15 / 100)); // decreases Kologarn's health by 15%
                    ++RubbleCount;
                    if (instance && right)
                        instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, ACHIEV_DISARMED_START_EVENT);
                    events.ScheduleEvent(EVENT_LEFT, 30000);
                    break;
                case ACTION_RESPAWN_RIGHT:
                    right = false;
                    Talk(SAY_RIGHT_ARM_GONE);
                    me->DealDamage(me, int32(me->GetMaxHealth() * 15 / 100));
                    ++RubbleCount;
                    if (instance && left)
                        instance->DoStartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, ACHIEV_DISARMED_START_EVENT);
                    events.ScheduleEvent(EVENT_RIGHT, 30000);
                    break;
                case ACTION_FAIL_ACHIV:
                    if (!Beamachiv)
                        return;
                    Beamachiv = false;
                    break;
            }
        }    
    };

};

class npc_eyebeam_stalker : public CreatureScript
{
public:
    npc_eyebeam_stalker() : CreatureScript("npc_eyebeam_stalker") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_eyebeam_stalkerAI (pCreature);
    }

    struct npc_eyebeam_stalkerAI : public ScriptedAI
    {
        npc_eyebeam_stalkerAI(Creature *c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetDisableGravity(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
        }
    };

};


class npc_focused_eyebeam : public CreatureScript
{
public:
    npc_focused_eyebeam() : CreatureScript("npc_focused_eyebeam") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_focused_eyebeamAI (pCreature);
    }

    struct npc_focused_eyebeamAI : public ScriptedAI
    {
        npc_focused_eyebeamAI(Creature *c) : ScriptedAI(c), Beam(true)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            DoCast(me, SPELL_EYEBEAM_IMMUNITY);
            DoCast(me, SPELL_FOCUSED_EYEBEAM_PERIODIC_VISUAL);
            me->SetSpeed(MOVE_RUN, 0.5f, true);
            me->SetDisplayId(11686);
            DamageTimer = 3000;
            checkTimer = 1500;

            Instance = c->GetInstanceScript();
        }

        InstanceScript* Instance;

        bool Beam;
        uint32 checkTimer;
        uint32 DamageTimer;

        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (target->GetTypeId() == TYPEID_PLAYER && (spell->Id == SPELL_FOCUSED_EYEBEAM_DAMAGE_HELPER && Beam))
            {
                Beam = false;
                if (Creature* Kologarn = me->GetCreature(*me, Instance->GetGuidData(DATA_KOLOGARN)))
                    Kologarn->ToCreature()->AI()->DoAction(ACTION_FAIL_ACHIV);
            }
        }


        void UpdateAI(uint32 diff) override
        {
            if (DamageTimer)
            {
                if (DamageTimer <= diff)
                {
                    DoCast(me, SPELL_FOCUSED_EYEBEAM_HELPER, true);
                    DamageTimer = 0;
                }
                else DamageTimer -= diff;
            }

            if (checkTimer <= diff)
            {
                if (me->getVictim() && me->getVictim()->isAlive())
                    me->GetMotionMaster()->MovePoint(0,me->getVictim()->GetPositionX(),me->getVictim()->GetPositionY(),me->getVictim()->GetPositionZ());
            
                checkTimer = 500;
            }
            else checkTimer -= diff;
        }
    };

};


class npc_left_arm : public CreatureScript
{
public:
    npc_left_arm() : CreatureScript("npc_left_arm") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_left_armAI (pCreature);
    }

    struct npc_left_armAI : public ScriptedAI
    {
        npc_left_armAI(Creature *c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
        }

        bool BrokenLeft;

        void Reset() override
        {
            BrokenLeft = false;
        }

        InstanceScript* instance;
        
        void DamageTaken(Unit* /*attacker*/, uint32 &damage, DamageEffectType dmgType) override
        {
            if (damage >= me->GetHealth() && !BrokenLeft)
            {
                damage = 0;
                BrokenLeft = true;
                me->SetFullHealth();
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->AttackStop();
                me->SetVisible(false);

                if (instance)
                    if (Creature* pKologarn = me->GetCreature(*me, instance->GetGuidData(DATA_KOLOGARN)))
                        pKologarn->AI()->DoAction(ACTION_RESPAWN_LEFT);
                
                for (uint8 i = 0; i < 5; ++i)
                    me->SummonCreature(NPC_RUBBLE, RubbleRight, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);
            }
        }
        
        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_REMOVE_LEFT:
                    me->SetFullHealth();
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    me->SetVisible(true);
                    BrokenLeft = false;
                    DoZoneInCombat();
                    break;
            }
        }
        
        void JustSummoned(Creature *summon) override
        {
            summon->AI()->DoZoneInCombat();
        }
    };

};




class npc_right_arm : public CreatureScript
{
public:
    npc_right_arm() : CreatureScript("npc_right_arm") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_right_armAI (pCreature);
    }

    struct npc_right_armAI : public ScriptedAI
    {
        npc_right_armAI(Creature *c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, 64708, true);
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* instance;

        bool Gripped;
        bool BrokenRight;
        uint32 ArmDamage;
        uint32 SqueezeTimer;

        void Reset() override
        {
            ArmDamage = 0;
            Gripped = false;
            BrokenRight = false;
            SqueezeTimer = 0;
            
        }

        void SetArmDamage()
        {
            if (Is25ManRaid())
                ArmDamage = 480000;
            else
                ArmDamage = 100000;
        }
        
        void JustSummoned(Creature *summon) override
        {
            summon->AI()->DoZoneInCombat();
        }

        void KilledUnit(Unit* Victim) override
        {
            if (Victim)
            {
                Victim->ExitVehicle();
                Victim->GetMotionMaster()->MoveJump(1767.80f, -18.38f, 448.808f, 10, 10);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (Gripped)
            {
                if (SqueezeTimer <= diff)
                {
                    for (uint8 n = 0; n < RAID_MODE(1, 3); ++n)
                    {
                        if (me->GetVehicleKit()->GetPassenger(n) && me->GetVehicleKit()->GetPassenger(n)->isAlive())
                            me->Kill(me->GetVehicleKit()->GetPassenger(n), true);
                    }
                    ArmDamage = 0;
                    Gripped = false;
                }  
                else SqueezeTimer -= diff;
            }
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTIOM_REMOVE_RIGHT:
                    me->SetFullHealth();
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    me->SetVisible(true);
                    BrokenRight = false;
                    DoZoneInCombat();
                    break;
                case ACTION_GRIP:
                    for (uint8 n = 0; n < RAID_MODE(1, 3); ++n)
                    {
                        if (Unit* GripTarget = Unit::GetUnit(*me, GripTargetGUID[n]))
                        {
                            if (GripTarget && GripTarget->isAlive() && !GripTarget->GetVehicle())
                            {
                                GripTarget->EnterVehicle(me, n);
                                me->AddAura(SPELL_STONE_GRIP_STUN, GripTarget);
                                GripTargetGUID[n].Clear();
                            }
                        }
                    }
                    SqueezeTimer = 16000;
                    SetArmDamage();
                    Gripped = true;
                    break;
            }
        }
    
        void DamageTaken(Unit* pKiller, uint32 &damage, DamageEffectType dmgType) override
        {
            if (damage >= me->GetHealth() && Gripped && !BrokenRight)
            {
                damage = 0;
                BrokenRight = true;
                me->SetFullHealth();
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetVisible(false);
                
                for (uint8 n = 0; n < RAID_MODE(1, 3); ++n)
                {
                    Unit* pGripTarget = me->GetVehicleKit()->GetPassenger(n);
                    if (pGripTarget && pGripTarget->isAlive())
                    {
                        pGripTarget->RemoveAurasDueToSpell(SPELL_STONE_GRIP);
                        pGripTarget->RemoveAurasDueToSpell(SPELL_STONE_GRIP_STUN);
                        pGripTarget->ExitVehicle();
                        pGripTarget->GetMotionMaster()->MoveJump(1767.80f, -18.38f, 448.808f, 10, 10);
                    }
                }Gripped = false;
                
                for (uint8 i = 0; i < 5; ++i)
                        me->SummonCreature(NPC_RUBBLE, RubbleRight, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);

                if (instance)
                    if (Creature* pKologarn = me->GetCreature(*me, instance->GetGuidData(DATA_KOLOGARN)))
                        pKologarn->AI()->DoAction(ACTION_RESPAWN_RIGHT);
            }
            else if (damage >= me->GetHealth() && !Gripped && !BrokenRight)
            {
                damage = 0;
                BrokenRight = true;
                me->SetFullHealth();
                me->RemoveAllAuras();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->AttackStop();
                me->SetVisible(false);

                for (uint8 i = 0; i < 5; ++i)
                    me->SummonCreature(NPC_RUBBLE, RubbleRight, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);
                
                if (instance)
                    if (Creature* pKologarn = me->GetCreature(*me, instance->GetGuidData(DATA_KOLOGARN)))
                        pKologarn->AI()->DoAction(ACTION_RESPAWN_RIGHT);
            }
            else if (ArmDamage && damage >= ArmDamage && Gripped && !BrokenRight)
            {
                ArmDamage = 0;
                for (uint8 n = 0; n < RAID_MODE(1, 3); ++n)
                {
                    Unit* pGripTarget = me->GetVehicleKit()->GetPassenger(n);
                    if (pGripTarget && pGripTarget->isAlive())
                    {
                        pGripTarget->RemoveAurasDueToSpell(SPELL_STONE_GRIP_STUN);
                        pGripTarget->ExitVehicle();
                        pGripTarget->GetMotionMaster()->MoveJump(1775.0479f, -3.7957f, 448.8068f, 10, 10);
                    }
                    Gripped = false;
                }
                
            }
        }
    };
};

class achievement_disarmed : public AchievementCriteriaScript
{
    public:
        achievement_disarmed() : AchievementCriteriaScript("achievement_disarmed")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* Kl = target->ToCreature())
                if (boss_kologarn::boss_kologarnAI * KlAI = CAST_AI(boss_kologarn::boss_kologarnAI, Kl->AI()))
                    if (!KlAI->Isright() && !KlAI->Isleft())
                        return true;

            return false;
        }

};

class achievement_looks_could_kill : public AchievementCriteriaScript
{
    public:
        achievement_looks_could_kill() : AchievementCriteriaScript("achievement_looks_could_kill")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* Kl = target->ToCreature())
                if (boss_kologarn::boss_kologarnAI * KlAI = CAST_AI(boss_kologarn::boss_kologarnAI, Kl->AI()))
                    if (KlAI->IsBeamachiv())
                        return true;

            return false;
        }

};

class achievement_with_open_arms : public AchievementCriteriaScript
{
    public:
        achievement_with_open_arms() : AchievementCriteriaScript("achievement_with_open_arms")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* Kl = target->ToCreature())
                if (boss_kologarn::boss_kologarnAI * KlAI = CAST_AI(boss_kologarn::boss_kologarnAI, Kl->AI()))
                    if (KlAI->IsOpenArms())
                        return true;

            return false;
        }

};

void AddSC_boss_kologarn()
{
    new boss_kologarn();
    new npc_eyebeam_stalker();
    new npc_focused_eyebeam();
    new npc_left_arm();
    new npc_right_arm();
    new achievement_disarmed();
    new achievement_looks_could_kill();
    new achievement_with_open_arms();
}