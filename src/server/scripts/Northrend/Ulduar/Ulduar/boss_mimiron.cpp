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
SDName: Mimiron
SDAuthor: PrinceCreed
SD%Complete: 95
SDComments: P3Wx2 Laser Barrage not works in phase 4 due to core limits.
EndScriptData */

#include "ulduar.h"
#include "Vehicle.h"

enum Yells
{
    SAY_AGGRO                                   = -1603240,
    SAY_HARDMODE_ON                             = -1603241,
    SAY_MKII_ACTIVATE                           = -1603242,
    SAY_MKII_SLAY_1                             = -1603243,
    SAY_MKII_SLAY_2                             = -1603244,
    SAY_MKII_DEATH                              = -1603245,
    SAY_VX001_ACTIVATE                          = -1603246,
    SAY_VX001_SLAY_1                            = -1603247,
    SAY_VX001_SLAY_2                            = -1603248,
    SAY_VX001_DEATH                             = -1603249,
    SAY_AERIAL_ACTIVATE                         = -1603250,
    SAY_AERIAL_SLAY_1                           = -1603251,
    SAY_AERIAL_SLAY_2                           = -1603252,
    SAY_AERIAL_DEATH                            = -1603253,
    SAY_V07TRON_ACTIVATE                        = -1603254,
    SAY_V07TRON_SLAY_1                          = -1603255,
    SAY_V07TRON_SLAY_2                          = -1603256,
    SAY_V07TRON_DEATH                           = -1603257,
    SAY_BERSERK                                 = -1603258,
    SAY_YS_HELP                                 = -1603259
};

#define EMOTE_LEVIATHAN                         "Leviathan MK II begins to cast Plasma Blast!"

enum eSpells
{
    // Leviathan MK II
    SPELL_MINES_SPAWN                           = 65347,
    SPELL_PROXIMITY_MINES                       = 63027,
    SPELL_PLASMA_BLAST                          = 62997,
    SPELL_SHOCK_BLAST                           = 63631,
    SPELL_EXPLOSION                             = 66351,
    SPELL_NAPALM_SHELL                          = 63666,
    
    // VX-001
    SPELL_RAPID_BURST                           = 63382,
    SPELL_ROCKET_STRIKE                         = 63036,
    SPELL_ROCKET_STRIKE_AURA                    = 64064,
    SPELL_ROCKET_STRIKE_DMG                     = 63041,
    SPELL_LASER_BARRAGE                         = 63274,
    SPELL_SPINNING_UP                           = 63414,
    SPELL_HEAT_WAVE                             = 63677,
    SPELL_HAND_PULSE                            = 64348,
    
    // Aerial Command Unit
    SPELL_PLASMA_BALL                           = 63689,
    SPELL_MAGNETIC_CORE                         = 64436,
    SPELL_MAGNETIC_CORE_VISUAL                  = 64438,
    SPELL_BOOM_BOT                              = 63801,
    SPELL_MAGNETIC_FIELD                        = 64668,
    SPELL_HOVER                                 = 57764, // Set Hover position
    SPELL_BERSERK                               = 47008,
    
    // Hard Mode
    SPELL_EMERGENCY_MODE                        = 64582,
    SPELL_FLAME_SUPPRESSANT_1                   = 64570,
    SPELL_FLAME_SUPPRESSANT_2                   = 65192,
    SPELL_FLAME                                 = 64561,
    SPELL_FROST_BOMB                            = 64624,
    SPELL_WATER_SPRAY                           = 64619,
    SPELL_SIREN                                 = 64616
};

enum eEvents
{
    // Leviathan MK II
    EVENT_NONE,
    EVENT_PROXIMITY_MINE,
    EVENT_NAPALM_SHELL,
    EVENT_PLASMA_BLAST,
    EVENT_SHOCK_BLAST,
    EVENT_FLAME_SUPPRESSANT,
    
    // VX-001
    EVENT_RAPID_BURST,
    EVENT_PRE_LASER_BARRAGE,
    EVENT_LASER_BARRAGE,
    EVENT_ROCKET_STRIKE,
    EVENT_HEAT_WAVE,
    EVENT_HAND_PULSE,
    EVENT_FROST_BOMB,
    
    // Aerial Command Unit
    EVENT_PLASMA_BALL,
    EVENT_REACTIVATE_AERIAL,
    EVENT_SUMMON_BOTS
};

enum Phases
{
    PHASE_NULL,
    PHASE_INTRO,
    PHASE_COMBAT,
    PHASE_LEVIATHAN_SOLO,
    PHASE_LEVIATHAN_ASSEMBLED,
    PHASE_VX001_ACTIVATION,
    PHASE_VX001_SOLO,
    PHASE_VX001_ASSEMBLED,
    PHASE_AERIAL_ACTIVATION,
    PHASE_AERIAL_SOLO,
    PHASE_AERIAL_ASSEMBLED,
    PHASE_V0L7R0N_ACTIVATION,
};

enum eActions
{
    DO_START_ENCOUNTER                          = 1,
    DO_ACTIVATE_VX001                           = 2,
    DO_START_VX001                              = 3,
    DO_ACTIVATE_AERIAL                          = 4,
    DO_START_AERIAL                             = 5,
    DO_DISABLE_AERIAL                           = 6,
    DO_ACTIVATE_V0L7R0N                         = 7,
    DO_LEVIATHAN_ASSEMBLED                      = 8,
    DO_VX001_ASSEMBLED                          = 9,
    DO_AERIAL_ASSEMBLED                         = 10,
    DO_ACTIVATE_DEATH_TIMER                     = 11,
    DO_ENTER_ENRAGE                             = 12,
    DO_ACTIVATE_HARD_MODE                       = 13,
    ACTION_MINE_FAIL                            = 14,
    ACTION_ROCKET_FAIL                          = 15,
    ACTION_BOOM_FAIL                            = 16,
};

enum Npcs
{
    NPC_VX_001                                  = 33651,
    NPC_AERIAL_UNIT                             = 33670,
    NPC_BURST_TARGET                            = 34211,
    NPC_JUNK_BOT                                = 33855,
    NPC_ASSAULT_BOT                             = 34057,
    NPC_BOOM_BOT                                = 33836,
    NPC_EMERGENCY_BOT                           = 34147,
    NPC_FLAME                                   = 34363,
    NPC_FLAME_SPREAD                            = 34121,
    NPC_FROST_BOMB                              = 34149
};

bool MimironHardMode;

// Achievements
#define ACHIEVEMENT_FIREFIGHTER                 RAID_MODE(3180, 3189)
#define ACHIEVEMENT_NOT_SO_FRIENDLY_FIRE        RAID_MODE(3138, 2995)
#define ACHIEVEMENT_SET_UP_US_THE_BOMB          RAID_MODE(2989, 3237) // TODO

enum MimironChests
{
    CACHE_OF_INNOVATION_10                      = 194789,
    CACHE_OF_INNOVATION_HARDMODE_10             = 194957,
    CACHE_OF_INNOVATION_25                      = 194956,
    CACHE_OF_INNOVATION_HARDMODE_25             = 194958
};

const Position SummonPos[9] =
{
{2703.93f, 2569.32f, 364.397f, 0},
{2715.33f, 2569.23f, 364.397f, 0},
{2726.85f, 2569.28f, 364.397f, 0},
{2765.24f, 2534.38f, 364.397f, 0},
{2759.54f, 2544.30f, 364.397f, 0},
{2753.82f, 2554.22f, 364.397f, 0},
{2764.95f, 2604.11f, 364.397f, 0},
{2759.19f, 2594.26f, 364.397f, 0},
{2753.56f, 2584.30f, 364.397f, 0}
};

/*---------------------------------------------*
 *                   Mimiron                   *
 *---------------------------------------------*/

//33350
class boss_mimiron : public CreatureScript
{
public:
    boss_mimiron() : CreatureScript("boss_mimiron") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_mimironAI(pCreature);
    }

    struct boss_mimironAI : public BossAI
    {
        boss_mimironAI(Creature *pCreature) : BossAI(pCreature, BOSS_MIMIRON)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
            me->SetReactState(REACT_PASSIVE);
        }
        
        uint32 uiPhase_timer;
        uint32 CheckIsWipe;
        uint32 uiStep;
        uint32 EnrageTimer;
        uint32 FlameTimer;
        uint32 uiBotTimer;
        bool checkBotAlive;
        bool Enraged;
        bool achivrocket;
        bool achivmine;
        bool achivboom;

        Phases phase;
        
        void Reset() override
        {
            _Reset();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_USE_STANDING);
            me->SetVisible(true);
            me->ExitVehicle();
            me->GetMotionMaster()->MoveTargetedHome();
            if (instance)
            {
                instance->SetData(DATA_MIMIRON_ELEVATOR, GO_STATE_ACTIVE);
                
                for (uint8 data = DATA_LEVIATHAN_MK_II; data <= DATA_AERIAL_UNIT; ++data)
                {
                    if (Creature *pCreature = me->GetCreature(*me, instance->GetGuidData(data)))
                    {
                        if (pCreature->isAlive())
                        {
                            pCreature->ExitVehicle();
                            pCreature->AI()->EnterEvadeMode();
                        }
                    }
                }
            }
            phase = PHASE_NULL;
            CheckIsWipe = 0;
            uiStep = 0;
            uiPhase_timer = -1;
            uiBotTimer = 0;
            MimironHardMode = false;
            checkBotAlive = true;
            Enraged = false;
            achivrocket = true;
            achivmine = true;
            achivboom = true;
            DespawnCreatures(34362, 100);
        }
        
        void JustDied(Unit *victim) override
        {
            DoScriptText(SAY_V07TRON_DEATH, me);
            _JustDied();
            
            me->setFaction(35);
            
            if (instance)
            {
                //if (MimironHardMode)
                //    me->SummonGameObject(RAID_MODE(CACHE_OF_INNOVATION_HARDMODE_10, CACHE_OF_INNOVATION_HARDMODE_25), 2732.77f, 2569.42f, 364.31f, 6.25f, 0, 0, 0.7f, 0.7f, 604800);
                //else
                //    me->SummonGameObject(RAID_MODE(CACHE_OF_INNOVATION_10, CACHE_OF_INNOVATION_25), 2732.77f, 2569.42f, 364.31f, 6.25f, 0, 0, 0.7f, 0.7f, 604800);
                me->SummonGameObject(CACHE_OF_INNOVATION_10, 2732.77f, 2569.42f, 364.31f, 6.25f, 0, 0, 0.7f, 0.7f, 604800);
            }
        }

        bool Getachivrocket()
        {
            return achivrocket;
        }

        bool Getachivmine()
        {
            return achivmine;
        }

        bool Getachivboom()
        {
            return achivboom;
        }

        bool GetHardMode()
        {
            return MimironHardMode;
        }

        void EnterCombat(Unit *who) override
        {
            _EnterCombat();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            phase = PHASE_INTRO;
            CheckIsWipe = 1500;
            FlameTimer = 30000;
            if (MimironHardMode)
                EnrageTimer = 8*60*1000; // Enrage in 8 min
            else
                EnrageTimer = 15*60*1000; // Enrage in 15 min
            JumpToNextStep(100);
        }

        void JumpToNextStep(uint32 uiTimer)
        {
            uiPhase_timer = uiTimer;
            ++uiStep;
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (CheckIsWipe)
            {
                if (CheckIsWipe <= diff)
                {
                    if (IsWipe())
                        EnterEvadeMode();
                    CheckIsWipe = 1500;
                }
                else
                    CheckIsWipe -= diff;
            }
                
            if (EnrageTimer <= diff && !Enraged)
            {
                DoScriptText(SAY_BERSERK, me);
                for (uint8 data = DATA_LEVIATHAN_MK_II; data <= DATA_AERIAL_UNIT; ++data)
                {
                    if (Creature *pCreature = me->GetCreature(*me, instance->GetGuidData(data)))
                        pCreature->AI()->DoAction(DO_ENTER_ENRAGE);
                }
                Enraged = true;
            }
            else EnrageTimer -= diff;
            
            if (MimironHardMode)
            {
                if (FlameTimer <= diff)
                {
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (Creature* Flame = me->SummonCreature(NPC_FLAME, pTarget->GetPositionX() + irand(-6,6), pTarget->GetPositionY() + irand(-6,6), pTarget->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN))
                                Flame->AI()->SetGUID(pTarget->GetGUID(), 0); 
                        }
                    }
                    FlameTimer = 30000;
                }
                else 
                    FlameTimer -= diff;
            }
                
            // All sections need to die within 10 seconds, else they respawn
            if (checkBotAlive)
                uiBotTimer = 0;
            else
            {
                uiBotTimer += diff;
                if (uiBotTimer > 15000)
                {
                    if (Creature *pLeviathan = me->GetCreature(*me, instance->GetGuidData(DATA_LEVIATHAN_MK_II)))
                        pLeviathan->AI()->DoAction(DO_LEVIATHAN_ASSEMBLED);
                    if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                        pVX_001->AI()->DoAction(DO_VX001_ASSEMBLED);
                    if (Creature *pAerialUnit = me->GetCreature(*me, instance->GetGuidData(DATA_AERIAL_UNIT)))
                        pAerialUnit->AI()->DoAction(DO_AERIAL_ASSEMBLED);

                    checkBotAlive = true;
                }
                else
                {
                    if (Creature *pLeviathan = me->GetCreature(*me, instance->GetGuidData(DATA_LEVIATHAN_MK_II)))
                        if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                            if (Creature *pAerialUnit = me->GetCreature(*me, instance->GetGuidData(DATA_AERIAL_UNIT)))
                                if (pLeviathan->getStandState() == UNIT_STAND_STATE_DEAD)
                                    if (pVX_001->getStandState() == UNIT_STAND_STATE_DEAD)
                                        if (pAerialUnit->getStandState() == UNIT_STAND_STATE_DEAD)
                                        {
                                            if (!checkBotAlive) //Safe Death
                                            {
                                                checkBotAlive = true;
                                                me->Kill(pLeviathan, true);
                                                pVX_001->DisappearAndDie();
                                                pAerialUnit->DisappearAndDie();
                                                me->Kill(me, true);
                                                //Kill credit
                                                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_KILL_CREATURE, 33432, 1, 0, me);
                                            }
                                        }
                }
            }

            if (phase == PHASE_INTRO)
            {
                if (uiPhase_timer <= diff)
                {
                    switch (uiStep)
                    {
                        case 1:
                            if (MimironHardMode)
                                DoScriptText(SAY_HARDMODE_ON, me);
                            else
                                DoScriptText(SAY_AGGRO, me);
                            JumpToNextStep(10000);
                            break;
                        case 2:
                            if (instance)
                            {
                                if (Creature *pLeviathan = me->GetCreature(*me, instance->GetGuidData(DATA_LEVIATHAN_MK_II)))
                                    me->EnterVehicle(pLeviathan, 4);
                            }
                            JumpToNextStep(2000);
                            break;
                        case 3:
                            me->ChangeSeat(2);
                            JumpToNextStep(2000);
                            break;
                        case 4:
                            me->ChangeSeat(5);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STAND);
                            JumpToNextStep(2500);
                            break;
                        case 5:
                            DoScriptText(SAY_MKII_ACTIVATE, me);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_TALK);
                            JumpToNextStep(6000);
                            break;
                        case 6:
                            me->ChangeSeat(6);
                            JumpToNextStep(2000);
                            break;
                        case 7:
                            if (instance)
                            {
                                if (Creature *pLeviathan = me->GetCreature(*me, instance->GetGuidData(DATA_LEVIATHAN_MK_II)))
                                {
                                    me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STAND);
                                    pLeviathan->AI()->DoAction(DO_START_ENCOUNTER);
                                    phase = PHASE_COMBAT;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                    uiPhase_timer -= diff;
            }
            
            if (phase == PHASE_VX001_ACTIVATION)
            {
                if (uiPhase_timer <= diff)
                {
                    switch (uiStep)
                    {
                        case 1:
                            DoScriptText(SAY_MKII_DEATH, me);
                            JumpToNextStep(10000);
                            break;
                        case 2:
                            me->ChangeSeat(1);
                            JumpToNextStep(2000);
                            break;
                        case 3:
                            if (instance)
                                instance->SetData(DATA_MIMIRON_ELEVATOR, GO_STATE_READY);
                            JumpToNextStep(5000);
                            break;
                        case 4:
                            if (instance)
                            {
                                if (Creature *pVX_001 = me->SummonCreature(NPC_VX_001, 2744.65f, 2569.46f, 364.397f, 3.14159f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000))
                                {
                                    instance->SetData(DATA_MIMIRON_ELEVATOR, GO_STATE_ACTIVE_ALTERNATIVE);
                                }
                            }
                            JumpToNextStep(8000);
                            break;
                        case 5:
                            if (instance)
                            {
                                if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                                    me->EnterVehicle(pVX_001, 0);
                            }
                            JumpToNextStep(3500);
                            break;
                        case 6:
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_TALK);
                            DoScriptText(SAY_VX001_ACTIVATE, me);
                            JumpToNextStep(10000);
                            break;
                        case 7:
                            me->ChangeSeat(1);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_SIT);
                            JumpToNextStep(2000);
                            break;
                        case 8:
                            if (instance)
                            {
                                if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                                    pVX_001->HandleEmoteCommand(EMOTE_ONESHOT_EMERGE);
                            }
                            JumpToNextStep(3500);
                            break;
                        case 9:
                            if (instance)
                                if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                                {
                                    pVX_001->AddAura(SPELL_HOVER, pVX_001); // Hover
                                    pVX_001->AI()->DoAction(DO_START_VX001);
                                    phase = PHASE_COMBAT;
                                }
                            break;
                        default:
                            break;
                    }
                }
                else
                    uiPhase_timer -= diff;
            }
            
            if (phase == PHASE_AERIAL_ACTIVATION)
            {
                if (uiPhase_timer <= diff)
                {
                    switch (uiStep)
                    {
                        case 1:
                            me->ChangeSeat(4);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STAND);
                            JumpToNextStep(2500);
                            break;
                        case 2:
                            DoScriptText(SAY_VX001_DEATH, me);
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_TALK);
                            JumpToNextStep(5000);
                            break;
                        case 3:
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STAND);
                            if (instance)
                                me->SummonCreature(NPC_AERIAL_UNIT, 2744.65f, 2569.46f, 380, 3.14159f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                            JumpToNextStep(5000);
                            break;
                        case 4:
                            me->ExitVehicle();
                            me->GetMotionMaster()->MoveJump(2745.06f, 2569.36f, 379.90f, 10, 15);
                            JumpToNextStep(2000);
                            break;
                        case 5:
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_TALK);
                            DoScriptText(SAY_AERIAL_ACTIVATE, me);
                            JumpToNextStep(8000);
                            break;
                        case 6:
                            me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_STAND);
                            me->SetVisible(false);
                            if (instance)
                            {
                                if (Creature *pAerialUnit = me->GetCreature(*me, instance->GetGuidData(DATA_AERIAL_UNIT)))
                                {
                                    pAerialUnit->AI()->DoAction(DO_START_AERIAL);
                                    phase = PHASE_COMBAT;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                    uiPhase_timer -= diff;
            }
            
            if (phase == PHASE_V0L7R0N_ACTIVATION)
            {
                if (uiPhase_timer <= diff)
                {
                    switch (uiStep)
                    {
                        case 1:
                            if (instance)
                            {
                                DoScriptText(SAY_AERIAL_DEATH, me);
                                if (Creature *pLeviathan = me->GetCreature(*me, instance->GetGuidData(DATA_LEVIATHAN_MK_II)))
                                    if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                                    {
                                        pLeviathan->GetMotionMaster()->MoveTargetedHome();
                                        pVX_001->SetStandState(UNIT_STAND_STATE_STAND);
                                        pVX_001->EnterVehicle(pLeviathan, 7);
                                        me->EnterVehicle(pVX_001, 1);
                                    }
                            }
                            JumpToNextStep(8000);
                            break;
                        case 2:
                            if (instance)
                                if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                                    if (Creature *pAerialUnit = me->GetCreature(*me, instance->GetGuidData(DATA_AERIAL_UNIT)))
                                    {
                                        DoScriptText(SAY_V07TRON_ACTIVATE, me);
                                        pAerialUnit->SetCanFly(false);
                                        pAerialUnit->SetDisableGravity(false);
                                        pAerialUnit->EnterVehicle(pVX_001, 3);
                                    }
                            JumpToNextStep(10000);
                            break;
                        case 3:
                            if (instance)
                            {
                                if (Creature *pLeviathan = me->GetCreature(*me, instance->GetGuidData(DATA_LEVIATHAN_MK_II)))
                                    pLeviathan->AI()->DoAction(DO_LEVIATHAN_ASSEMBLED);
                                if (Creature *pVX_001 = me->GetCreature(*me, instance->GetGuidData(DATA_VX_001)))
                                    pVX_001->AI()->DoAction(DO_VX001_ASSEMBLED);
                                if (Creature *pAerialUnit = me->GetCreature(*me, instance->GetGuidData(DATA_AERIAL_UNIT)))
                                    pAerialUnit->AI()->DoAction(DO_AERIAL_ASSEMBLED);
                                phase = PHASE_COMBAT;
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                    uiPhase_timer -= diff;
            }
        }
        
        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case DO_ACTIVATE_VX001:
                    phase = PHASE_VX001_ACTIVATION;
                    uiStep = 0;
                    uiPhase_timer = -1;
                    JumpToNextStep(100);
                    break;
                case DO_ACTIVATE_AERIAL:
                    phase = PHASE_AERIAL_ACTIVATION;
                    uiStep = 0;
                    uiPhase_timer = -1;
                    JumpToNextStep(5000);
                    break;
                case DO_ACTIVATE_V0L7R0N:
                    me->SetVisible(true);
                    phase = PHASE_V0L7R0N_ACTIVATION;
                    uiStep = 0;
                    uiPhase_timer = -1;
                    JumpToNextStep(1000);
                    break;
                case DO_ACTIVATE_DEATH_TIMER:
                    checkBotAlive = false;
                    break;
                case DO_ACTIVATE_HARD_MODE:
                    MimironHardMode = true;
                    DoZoneInCombat();
                    break;
                case ACTION_MINE_FAIL:
                    achivmine = false;
                    break;
                case ACTION_ROCKET_FAIL:
                    achivrocket = false;
                    break;
                case ACTION_BOOM_FAIL:
                    achivboom = false;
                    break;
            }
        }

        bool IsWipe()
        {
            Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
            
            if (PlayerList.isEmpty())
                return true;
            
            for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
            {
                Player* player = Itr->getSource();

                if (!player)
                    continue;

                if (player->isAlive() && !player->isGameMaster())
                    return false;
            }
            return true;
        }
        
        void DespawnCreatures(uint32 entry, float distance, bool discs = false)
        {
            std::list<Creature*> m_pCreatures;
            GetCreatureListWithEntryInGrid(m_pCreatures, me, entry, distance);
     
            if (m_pCreatures.empty())
                return;
     
            for(std::list<Creature*>::iterator iter = m_pCreatures.begin(); iter != m_pCreatures.end(); ++iter)
                (*iter)->DespawnOrUnsummon();
        }
        
    };

};


/*---------------------------------------------*
 *              Leviathan MK II                *
 *---------------------------------------------*/

class boss_leviathan_mk : public CreatureScript
{
public:
    boss_leviathan_mk() : CreatureScript("boss_leviathan_mk") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_leviathan_mkAI(pCreature);
    }

    struct boss_leviathan_mkAI : public BossAI
    {
        boss_leviathan_mkAI(Creature *pCreature) : BossAI(pCreature, BOSS_MIMIRON), vehicle(me->GetVehicleKit()), phase(PHASE_NULL)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
        }
        
        Vehicle* vehicle;
        Phases phase;
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->SetReactState(REACT_PASSIVE);
            me->RemoveAllAuras();
            phase = PHASE_NULL;
            events.SetPhase(PHASE_NULL);
        }
        
        void JustReachedHome() override
        {
            if (Creature *turret = CAST_CRE(me->GetVehicleKit()->GetPassenger(3)))
            {
                turret->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                turret->SetReactState(REACT_PASSIVE);
                turret->AI()->EnterEvadeMode();
            }
        }

        void KilledUnit(Unit *who) override
        {
            if (!(rand()%5))
                if (instance)
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                    {
                        if (phase == PHASE_LEVIATHAN_SOLO)
                            DoScriptText(RAND(SAY_MKII_SLAY_1, SAY_MKII_SLAY_2), pMimiron);
                        else
                            DoScriptText(RAND(SAY_V07TRON_SLAY_1, SAY_V07TRON_SLAY_2), pMimiron);
                    }
        }

        void DamageTaken(Unit *who, uint32 &damage, DamageEffectType dmgType) override
        {
            if (phase == PHASE_LEVIATHAN_SOLO)
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->RemoveAllAuras();
                    me->SetHealth(me->GetMaxHealth());
                    events.SetPhase(PHASE_NULL);
                    phase = PHASE_NULL;
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                        pMimiron->AI()->DoAction(DO_ACTIVATE_VX001);
                    if (Creature *turret = CAST_CRE(me->GetVehicleKit()->GetPassenger(3)))
                        turret->Kill(turret, false);
                    me->SetSpeed(MOVE_RUN, 1.5f, true);
                    me->GetMotionMaster()->MovePoint(0, 2790.11f, 2595.83f, 364.32f);
                }
                
            if (phase == PHASE_LEVIATHAN_ASSEMBLED)
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetHealth(me->GetMaxHealth());
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    events.SetPhase(PHASE_NULL);
                    phase = PHASE_NULL;
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                        pMimiron->AI()->DoAction(DO_ACTIVATE_DEATH_TIMER);
                }
        }

        void EnterCombat(Unit *who) override
        {
            if (MimironHardMode)
            {
                DoCast(me, SPELL_EMERGENCY_MODE);
                events.ScheduleEvent(EVENT_FLAME_SUPPRESSANT, 60000, 0, PHASE_LEVIATHAN_SOLO);
            }
            
            if (Creature *turret = CAST_CRE(me->GetVehicleKit()->GetPassenger(3)))
            {
                turret->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                turret->SetReactState(REACT_AGGRESSIVE);
                turret->AI()->DoZoneInCombat();
            }
            
            events.ScheduleEvent(EVENT_PROXIMITY_MINE, 1000);
            events.ScheduleEvent(EVENT_PLASMA_BLAST, 10000, 0, PHASE_LEVIATHAN_SOLO);
        }
        
        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case DO_START_ENCOUNTER:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    phase = PHASE_LEVIATHAN_SOLO;
                    events.SetPhase(PHASE_LEVIATHAN_SOLO);
                    DoZoneInCombat();
                    break;
                case DO_LEVIATHAN_ASSEMBLED:
                    if (MimironHardMode)
                        DoCast(me, SPELL_EMERGENCY_MODE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetHealth(int32(me->GetMaxHealth() / 2));
                    me->SetSpeed(MOVE_RUN, 1.0f, true);
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    phase = PHASE_LEVIATHAN_ASSEMBLED;
                    events.SetPhase(PHASE_LEVIATHAN_ASSEMBLED);
                    events.RescheduleEvent(EVENT_PROXIMITY_MINE, 1000);
                    events.RescheduleEvent(EVENT_SHOCK_BLAST, 30000);
                    break;
                case DO_ENTER_ENRAGE:
                    DoCast(me, SPELL_BERSERK, true);
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if(!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (phase == PHASE_LEVIATHAN_SOLO || phase == PHASE_LEVIATHAN_ASSEMBLED)
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_PROXIMITY_MINE:
                            for (int i = 0; i < 10; i++)
                                DoCast(SPELL_MINES_SPAWN);
                            DoCast(SPELL_PROXIMITY_MINES);
                            events.RescheduleEvent(EVENT_PROXIMITY_MINE, 35000);
                            break;
                        case EVENT_PLASMA_BLAST:
                            me->MonsterTextEmote(EMOTE_LEVIATHAN, ObjectGuid::Empty, true);
                            DoCast(SPELL_PLASMA_BLAST);
                            events.RescheduleEvent(EVENT_PLASMA_BLAST, urand(30000, 35000), 0, PHASE_LEVIATHAN_SOLO);
                            events.RescheduleEvent(EVENT_SHOCK_BLAST, urand(6000, 10000));
                            break;
                        case EVENT_SHOCK_BLAST:
                            DoCastAOE(SPELL_SHOCK_BLAST);
                            events.RescheduleEvent(EVENT_SHOCK_BLAST, 35000);
                            break;
                        case EVENT_FLAME_SUPPRESSANT:
                            DoCastAOE(SPELL_FLAME_SUPPRESSANT_1);
                            for (int8 n = 0; n < 2; n++)
                            {
                                uint32 npc;
                                if (n == 0)
                                    npc = NPC_FLAME;
                                else 
                                    npc = NPC_FLAME_SPREAD;
                                std::list<Creature*> m_pCreatures;
                                GetCreatureListWithEntryInGrid(m_pCreatures, me, npc, 100);
                                if (!m_pCreatures.empty())
                                    for(std::list<Creature*>::iterator iter = m_pCreatures.begin(); iter != m_pCreatures.end(); ++iter)
                                        (*iter)->DespawnOrUnsummon(3000);
                            }
                            events.CancelEvent(EVENT_FLAME_SUPPRESSANT);
                            break;
                    }
                }
            }
                        
            DoMeleeAttackIfReady();
        }
    };
};

class boss_leviathan_mk_turret : public CreatureScript
{
public:
    boss_leviathan_mk_turret() : CreatureScript("boss_leviathan_mk_turret") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_leviathan_mk_turretAI (pCreature);
    }

    struct boss_leviathan_mk_turretAI : public ScriptedAI
    {
        boss_leviathan_mk_turretAI(Creature *c) : ScriptedAI(c) 
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            uiNapalmShell = urand(4000, 8000);
        }

        uint32 uiNapalmShell;
        
        void UpdateAI(uint32 diff) override
        {
            if(!UpdateVictim())
                return;

            if (uiNapalmShell <= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_FARTHEST, 0))
                    if (!me->IsWithinDist(pTarget, 12))
                        DoCast(pTarget, SPELL_NAPALM_SHELL);
                uiNapalmShell = urand(8000, 12000);
            }
            else uiNapalmShell -= diff;
        }
    };

};

class npc_proximity_mine : public CreatureScript
{
public:
    npc_proximity_mine() : CreatureScript("npc_proximity_mine") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_proximity_mineAI (pCreature);
    }

    struct npc_proximity_mineAI : public Scripted_NoMovementAI
    {
        npc_proximity_mineAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
           me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
           uiBoomTimer = 35000;
           Boom = false;
           instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 uiBoomTimer;
        bool Boom;

        void MoveInLineOfSight(Unit *who) override
        {
            if (!Boom && me->IsWithinDistInMap(who, 0.5f) && who->GetTypeId() == TYPEID_PLAYER)
            {
                DoCastAOE(SPELL_EXPLOSION);
                Boom = true;
                if (Creature* Mimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                {
                    Mimiron->AI()->DoAction(ACTION_MINE_FAIL);
                    me->DespawnOrUnsummon(1000);
                }
            }
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (uiBoomTimer <= diff)
            {
                if (!Boom)
                {
                    me->DespawnOrUnsummon(1000);
                    Boom = true;
                }
            }
            else uiBoomTimer -= diff;
        }
    };
};


/*---------------------------------------------*
 *                    VX-001                   *
 *---------------------------------------------*/

class boss_vx_001 : public CreatureScript
{
public:
    boss_vx_001() : CreatureScript("boss_vx_001") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_vx_001AI(pCreature);
    }

    struct boss_vx_001AI : public BossAI
    {
        boss_vx_001AI(Creature *pCreature) : BossAI(pCreature, BOSS_MIMIRON), vehicle(me->GetVehicleKit()), phase(PHASE_NULL)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
            instance = pCreature->GetInstanceScript();
        }
        
        InstanceScript * instance;

        Vehicle* vehicle;
        Phases phase;
        EventMap events;
        
        void Reset() override
        {
            events.Reset();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->RemoveAllAuras();
            phase = PHASE_NULL;
            events.SetPhase(PHASE_NULL);
        }

        void KilledUnit(Unit *who) override
        {
            if (!(rand()%5))
                if (instance)
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                    {
                        if (phase == PHASE_VX001_SOLO)
                            DoScriptText(RAND(SAY_VX001_SLAY_1, SAY_VX001_SLAY_2), pMimiron);
                        else
                            DoScriptText(RAND(SAY_V07TRON_SLAY_1, SAY_V07TRON_SLAY_2), pMimiron);
                    }
        }

        void EnterCombat(Unit *who) override
        {
            if (MimironHardMode)
            {
                DoCast(me, SPELL_EMERGENCY_MODE);
                events.ScheduleEvent(EVENT_FROST_BOMB, 15000);
            }
                
            //events.ScheduleEvent(EVENT_RAPID_BURST, 500, 0, PHASE_VX001_SOLO);
            //events.RescheduleEvent(EVENT_PRE_LASER_BARRAGE, urand(30000, 40000), 0, PHASE_VX001_SOLO);
            events.ScheduleEvent(EVENT_ROCKET_STRIKE, 20000);
            events.ScheduleEvent(EVENT_HEAT_WAVE, urand(8000, 10000), 0, PHASE_VX001_SOLO);
            events.ScheduleEvent(EVENT_HAND_PULSE, 15000, 0, PHASE_VX001_ASSEMBLED);
        }
        
        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case DO_START_VX001:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                    phase = PHASE_VX001_SOLO;
                    events.SetPhase(PHASE_VX001_SOLO);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat();
                    //me->GetMotionMaster()->MoveRotate(50000, rand()%2 ? ROTATE_DIRECTION_LEFT : ROTATE_DIRECTION_RIGHT);
                    break;
                case DO_VX001_ASSEMBLED:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetHealth(int32(me->GetMaxHealth() / 2));
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    phase = PHASE_VX001_ASSEMBLED;
                    events.SetPhase(PHASE_VX001_ASSEMBLED);
                    DoZoneInCombat();
                    events.RescheduleEvent(EVENT_PRE_LASER_BARRAGE, urand(30000, 40000), 0, PHASE_VX001_SOLO);
                    events.RescheduleEvent(EVENT_ROCKET_STRIKE, 20000);
                    events.RescheduleEvent(EVENT_HAND_PULSE, 15000, 0, PHASE_VX001_ASSEMBLED);
                    if (MimironHardMode)
                    {
                        DoCast(me, SPELL_EMERGENCY_MODE);
                        events.RescheduleEvent(EVENT_FROST_BOMB, 15000);
                    }
                    break;
                case DO_ENTER_ENRAGE:
                    DoCast(me, SPELL_BERSERK, true);
                    break;
            }
        }

        void DamageTaken(Unit *who, uint32 &damage, DamageEffectType dmgType) override
        {
            if (phase == PHASE_VX001_SOLO)
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->AttackStop();
                    me->GetMotionMaster()->Initialize();
                    me->RemoveAllAuras();
                    me->SetHealth(me->GetMaxHealth());
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    phase = PHASE_NULL;
                    events.SetPhase(PHASE_NULL);
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                        pMimiron->AI()->DoAction(DO_ACTIVATE_AERIAL);
                }
                
            if (phase == PHASE_VX001_ASSEMBLED)
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->AttackStop();
                    me->SetHealth(me->GetMaxHealth());
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    events.SetPhase(PHASE_NULL);
                    phase = PHASE_NULL;
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                        pMimiron->AI()->DoAction(DO_ACTIVATE_DEATH_TIMER);
                }
        }
        
        void UpdateAI(uint32 diff) override
        {
            if(!UpdateVictim())
                return;
            
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (phase == PHASE_VX001_SOLO || phase == PHASE_VX001_ASSEMBLED)
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_RAPID_BURST: //Not Work Spell
                            me->MonsterTextEmote("Warning Rapid Burst", ObjectGuid::Empty, true);
                            if (Unit * pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            {
                                pTarget->ToCreature();
                                me->SetFacingToObject(pTarget);
                                DoCast(pTarget, SPELL_RAPID_BURST, true);
                            }
                            events.RescheduleEvent(EVENT_RAPID_BURST, 6000, 0, PHASE_VX001_SOLO);
                            break;
                        case EVENT_PRE_LASER_BARRAGE:
                            me->MonsterTextEmote("Warning Laser Barrage", ObjectGuid::Empty, true);
                            me->GetMotionMaster()->MoveRotate(15000, rand()%2 ? ROTATE_DIRECTION_LEFT : ROTATE_DIRECTION_RIGHT);
                            DoCastAOE(SPELL_SPINNING_UP, true);
                            me->ClearUnitState(UNIT_STATE_CASTING);
                            events.DelayEvents(14000);
                            events.RescheduleEvent(EVENT_PRE_LASER_BARRAGE, urand(30000, 40000), 0, PHASE_VX001_SOLO);
                            //events.RescheduleEvent(EVENT_LASER_BARRAGE, 3000);
                            break;
                        case EVENT_LASER_BARRAGE: //In the Future need for realize barrage in phase assemble (Sanmay)
                            events.CancelEvent(EVENT_LASER_BARRAGE);
                            break;
                        case EVENT_ROCKET_STRIKE:
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_FARTHEST, 1, 100, true))
                            {
                                Position Pos;
                                pTarget->GetPosition(&Pos);
                                if (Creature * Strike = me->SummonCreature(34047, Pos, TEMPSUMMON_TIMED_DESPAWN, 10000))
                                    me->CastSpell(Strike, SPELL_ROCKET_STRIKE, true);
                            }
                            events.RescheduleEvent(EVENT_ROCKET_STRIKE, urand(20000, 25000));
                            break;
                        case EVENT_HEAT_WAVE:
                            DoCastAOE(SPELL_HEAT_WAVE);
                            events.RescheduleEvent(EVENT_HEAT_WAVE, 10000, 0, PHASE_VX001_SOLO);
                            break;
                        case EVENT_HAND_PULSE:
                            if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                DoCast(pTarget, SPELL_HAND_PULSE);
                            events.RescheduleEvent(EVENT_HAND_PULSE, urand(10000, 12000));
                            break;
                        case EVENT_FROST_BOMB:
                            me->SummonCreature(NPC_FROST_BOMB, SummonPos[rand()%9], TEMPSUMMON_TIMED_DESPAWN, 11000);
                            events.RescheduleEvent(EVENT_FROST_BOMB, 45000);
                            break;
                    }
                }
            }
        }
    };
};

class npc_rocket_strike : public CreatureScript
{
public:
    npc_rocket_strike() : CreatureScript("npc_rocket_strike") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_rocket_strikeAI (pCreature);
    }

    struct npc_rocket_strikeAI : public Scripted_NoMovementAI
    {
        npc_rocket_strikeAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->DespawnOrUnsummon(10000);
            me->SetDisplayId(11686);
            DoCast(me, SPELL_ROCKET_STRIKE_AURA);

            instance = pCreature->GetInstanceScript();
        }
        
        InstanceScript* instance;

        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_ROCKET_STRIKE_DMG && target->GetTypeId() == TYPEID_PLAYER)
                if (Creature* Mimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                    Mimiron->AI()->DoAction(ACTION_ROCKET_FAIL);
            
        }
    };
};


/*---------------------------------------------*
 *             Aerial Command Unit             *
 *---------------------------------------------*/

class boss_aerial_unit : public CreatureScript
{
public:
    boss_aerial_unit() : CreatureScript("boss_aerial_unit") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_aerial_unitAI(pCreature);
    }

    struct boss_aerial_unitAI : public BossAI
    {
        boss_aerial_unitAI(Creature *pCreature) : BossAI(pCreature, BOSS_MIMIRON), vehicle(me->GetVehicleKit()), phase(PHASE_NULL)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_ROCKET_STRIKE_DMG, true);
        }
        
        Vehicle* vehicle;
        Phases phase;
        EventMap events;
        uint8 spawnedAdds;
        
        void Reset() override
        {
            events.Reset();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->RemoveAllAuras();
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            phase = PHASE_NULL;
            events.SetPhase(PHASE_NULL);
            summons.DespawnAll();
            spawnedAdds = 0;
        }

        void KilledUnit(Unit *who) override
        {
            if (!(rand()%5))
                if (instance)
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                    {
                        if (phase == PHASE_AERIAL_SOLO)
                            DoScriptText(RAND(SAY_AERIAL_SLAY_1, SAY_AERIAL_SLAY_2), pMimiron);
                        else
                            DoScriptText(RAND(SAY_V07TRON_SLAY_1, SAY_V07TRON_SLAY_2), pMimiron);
                    }
        }

        void EnterCombat(Unit *who) override
        {
            if (MimironHardMode)
                DoCast(me, SPELL_EMERGENCY_MODE);

            events.ScheduleEvent(EVENT_PLASMA_BALL, 1000);
            events.ScheduleEvent(EVENT_SUMMON_BOTS, 10000, 0, PHASE_AERIAL_SOLO);
        }
        
        void DoAction(const int32 action) override
        {
            switch(action)
            {
                case DO_START_AERIAL:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    phase = PHASE_AERIAL_SOLO;
                    events.SetPhase(PHASE_AERIAL_SOLO);
                    DoZoneInCombat();
                    break;
                case DO_DISABLE_AERIAL:
                    me->CastStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->Clear(true);
                    DoCast(me, SPELL_MAGNETIC_CORE);
                    DoCast(me, SPELL_MAGNETIC_CORE_VISUAL);
                    if (Creature *pMagneticCore = me->GetCreature(*me, instance->GetGuidData(DATA_MAGNETIC_CORE)))
                        if (pMagneticCore->isAlive())
                            me->NearTeleportTo(pMagneticCore->GetPositionX(), pMagneticCore->GetPositionY(), 368.965f, 0, false);
                    events.RescheduleEvent(EVENT_PLASMA_BALL, 22000, 0, PHASE_AERIAL_SOLO);
                    events.RescheduleEvent(EVENT_SUMMON_BOTS, 24000, 0, PHASE_AERIAL_SOLO);
                    events.ScheduleEvent(EVENT_REACTIVATE_AERIAL, 20000, 0, PHASE_AERIAL_SOLO);
                    break;
                case DO_AERIAL_ASSEMBLED:
                    if (MimironHardMode)
                        DoCast(me, SPELL_EMERGENCY_MODE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetHealth(int32(me->GetMaxHealth() / 2));
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    phase = PHASE_AERIAL_ASSEMBLED;
                    events.SetPhase(PHASE_AERIAL_ASSEMBLED);
                    events.RescheduleEvent(EVENT_PLASMA_BALL, 2000);
                    break;
                case DO_ENTER_ENRAGE:
                    DoCast(me, SPELL_BERSERK, true);
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if(!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (phase == PHASE_AERIAL_SOLO || phase == PHASE_AERIAL_ASSEMBLED)
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_PLASMA_BALL:
                            if (phase == PHASE_AERIAL_SOLO && me->getVictim())
                            {
                                float x = me->getVictim()->GetPositionX();
                                float y = me->getVictim()->GetPositionY();
                                float z = me->getVictim()->GetPositionZ();
                                if (me->IsWithinDist3d(x, y, z, 30))
                                {
                                    me->GetMotionMaster()->Initialize();
                                    DoCastVictim(SPELL_PLASMA_BALL);
                                }
                                else me->GetMotionMaster()->MovePoint(0, x, y, 380.04f);
                            }
                            else if (phase == PHASE_AERIAL_ASSEMBLED && me->getVictim())
                            {
                                if (me->getVictim()->IsWithinDist3d(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 35))
                                    DoCastVictim(SPELL_PLASMA_BALL);
                                else if (Unit *pTarget = SelectTarget(SELECT_TARGET_NEAREST, 0))
                                    DoCast(pTarget, SPELL_PLASMA_BALL);
                            }
                            events.RescheduleEvent(EVENT_PLASMA_BALL, 2000);
                            break;
                        case EVENT_REACTIVATE_AERIAL:
                            me->RemoveAurasDueToSpell(SPELL_MAGNETIC_CORE_VISUAL);
                            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), 380.04f, 3.14159f, false);
                            me->SetReactState(REACT_AGGRESSIVE);
                            events.CancelEvent(EVENT_REACTIVATE_AERIAL);
                            break;
                        case EVENT_SUMMON_BOTS:
                            if (phase == PHASE_AERIAL_SOLO)
                            {
                                spawnAdd();
                                events.RescheduleEvent(EVENT_SUMMON_BOTS, 10000, 0, PHASE_AERIAL_SOLO);
                            }
                            break;
                    }
                }
            }
        }
        
        void spawnAdd()
        {
            switch(spawnedAdds)
            {
                case 0:
                    for(uint8 n = 0; n < 2; n++)
                        me->SummonCreature(NPC_JUNK_BOT, SummonPos[rand()%9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                    break;
                case 1:
                    me->SummonCreature(NPC_ASSAULT_BOT, SummonPos[rand()%9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                    if (MimironHardMode)
                        for (uint8 i = 0; i < 2; ++i)
                            me->SummonCreature(NPC_EMERGENCY_BOT, SummonPos[rand()%9], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                    break;
                case 2:
                    me->SummonCreature(NPC_BOOM_BOT, 2744.65f, 2569.46f, 364.397f, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                    break;
            }

            spawnedAdds++;
            if(spawnedAdds > 2)
            {
                spawnedAdds = 0;
            }
        }
        
        void JustSummoned(Creature *summon) override
        {
            summons.Summon(summon);
            
            if (summon->GetEntry() == NPC_EMERGENCY_BOT)
                return;
                
            summon->AI()->DoZoneInCombat();
            if (MimironHardMode)
                DoCast(me, SPELL_EMERGENCY_MODE, true);
        }
        
        void DamageTaken(Unit *who, uint32 &damage, DamageEffectType dmgType) override
        {
            if (phase == PHASE_AERIAL_SOLO)
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->GetMotionMaster()->Clear(true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->RemoveAllAuras();
                    me->SetHealth(me->GetMaxHealth());
                    events.CancelEvent(EVENT_SUMMON_BOTS);
                    phase = PHASE_NULL;
                    events.SetPhase(PHASE_NULL);
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                        pMimiron->AI()->DoAction(DO_ACTIVATE_V0L7R0N);
                }
                
            if (phase == PHASE_AERIAL_ASSEMBLED)
                if (damage >= me->GetHealth())
                {
                    damage = 0;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetHealth(me->GetMaxHealth());
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    events.SetPhase(PHASE_NULL);
                    phase = PHASE_NULL;
                    if (Creature *pMimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                        pMimiron->AI()->DoAction(DO_ACTIVATE_DEATH_TIMER);
                }
        }
    };
};

class npc_magnetic_core : public CreatureScript
{
public:
    npc_magnetic_core() : CreatureScript("npc_magnetic_core") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_magnetic_coreAI (pCreature);
    }

    struct npc_magnetic_coreAI : public Scripted_NoMovementAI
    {
        npc_magnetic_coreAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
        {
            DoCast(SPELL_MAGNETIC_CORE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->DespawnOrUnsummon(21000);
            if (Creature *pAerialUnit = me->FindNearestCreature(NPC_AERIAL_UNIT, 20, true))
                if (!pAerialUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE) && pAerialUnit->IsFlying())
                    pAerialUnit->AI()->DoAction(DO_DISABLE_AERIAL);
        }
    };
};

class npc_assault_bot : public CreatureScript
{
public:
    npc_assault_bot() : CreatureScript("npc_assault_bot") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_assault_botAI (pCreature);
    }

    struct npc_assault_botAI : public ScriptedAI
    {
        npc_assault_botAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;
        uint32 uiFieldTimer;

        void Reset() override
        {
            uiFieldTimer = urand(4000, 6000);
        }
        

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiFieldTimer <= uiDiff)
            {
                DoCastVictim(SPELL_MAGNETIC_FIELD);
                uiFieldTimer = urand(15000, 20000);
            }
            else uiFieldTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
        
        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            // Achievement Not-So-Friendly Fire
            if (spell->Id == SPELL_ROCKET_STRIKE_DMG && instance)
            {
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 65040, 0, 0, me);
                me->Kill(me, true);
            }
        }
    };

};

class npc_emergency_bot : public CreatureScript
{
public:
    npc_emergency_bot() : CreatureScript("npc_emergency_bot") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_emergency_botAI (pCreature);
    }

    struct npc_emergency_botAI : public ScriptedAI
    {
        npc_emergency_botAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->SetReactState(REACT_PASSIVE);
            me->GetMotionMaster()->MoveRandom(15);
            uiSprayTimer = 5000;
        }

        uint32 uiSprayTimer;

        void UpdateAI(uint32 uiDiff) override
        {
            if (uiSprayTimer <= uiDiff)
            {
                DoCast(SPELL_WATER_SPRAY);
                for (int8 n = 0; n < 2; n++)
                {
                    uint32 npc;
                    if (n == 0)
                        npc = NPC_FLAME;
                    else 
                        npc = NPC_FLAME_SPREAD;
                    std::list<Creature*> m_pCreatures;
                    GetCreatureListWithEntryInGrid(m_pCreatures, me, npc, 12);
                    if (!m_pCreatures.empty())
                        for(std::list<Creature*>::iterator iter = m_pCreatures.begin(); iter != m_pCreatures.end(); ++iter)
                            (*iter)->DespawnOrUnsummon();
                }
                uiSprayTimer = 5000;
            }
            else uiSprayTimer -= uiDiff;
        }
    };

};


/*---------------------------------------------*
 *           DO NOT PUSH THIS BUTTON!          *
 *---------------------------------------------*/

class go_not_push_button : public GameObjectScript
{
public:
    go_not_push_button() : GameObjectScript("go_not_push_button") { }

    bool OnGossipHello(Player* pPlayer, GameObject* pGo) override
    {
        InstanceScript* instance = pGo->GetInstanceScript();

        if (!instance)
            return false;

        if (instance->GetBossState(BOSS_MIMIRON) == NOT_STARTED && pPlayer)
            if (Creature *pMimiron = pPlayer->GetCreature(*pPlayer, instance->GetGuidData(DATA_MIMIRON)))
                pMimiron->AI()->DoAction(DO_ACTIVATE_HARD_MODE);

        return true;
    }
};


class npc_mimiron_flame_trigger : public CreatureScript
{
public:
    npc_mimiron_flame_trigger() : CreatureScript("npc_mimiron_flame_trigger") { }

    struct npc_mimiron_flame_triggerAI : public ScriptedAI
    {
        npc_mimiron_flame_triggerAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            uiFlameTimer = 0;
            tGuid.Clear();
        }

        ObjectGuid tGuid;
        uint32 uiFlameTimer;
        
        void UpdateAI(uint32 diff) override
        {
            if (uiFlameTimer)
            {
                if (uiFlameTimer <= diff)
                {
                    me->StopMoving();
                    me->GetMotionMaster()->Clear(false);
                    if (Unit* target = me->GetUnit(*me, tGuid))
                    {
                        me->GetMotionMaster()->MoveCharge(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 1.2f);
                        me->SummonCreature(NPC_FLAME_SPREAD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                        uiFlameTimer = 5000;
                    }
                }
                else 
                    uiFlameTimer -= diff;
            }
        }

        void SetGUID(ObjectGuid const& guid, int32 type/* = 0 */) override
        {
            tGuid = guid;
            if (Unit* target = me->GetUnit(*me, tGuid))
            {
                me->GetMotionMaster()->MoveCharge(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 1.2f);
                me->SummonCreature(NPC_FLAME_SPREAD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                uiFlameTimer = 5000;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_mimiron_flame_triggerAI (pCreature);
    }
};

class npc_mimiron_flame_spread : public CreatureScript
{
public:
    npc_mimiron_flame_spread() : CreatureScript("npc_mimiron_flame_spread") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_mimiron_flame_spreadAI (pCreature);
    }

    struct npc_mimiron_flame_spreadAI : public Scripted_NoMovementAI
    {
        npc_mimiron_flame_spreadAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            DoCast(me, SPELL_FLAME, true);
        }

        InstanceScript* instance;
        
        void UpdateAI(uint32 uiDiff) override
        {
            if (instance && instance->GetBossState(BOSS_MIMIRON) != IN_PROGRESS)
                me->DespawnOrUnsummon();
        }
    };

};

class npc_frost_bomb : public CreatureScript
{
public:
    npc_frost_bomb() : CreatureScript("npc_frost_bomb") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_frost_bombAI (pCreature);
    }

    struct npc_frost_bombAI : public Scripted_NoMovementAI
    {
        npc_frost_bombAI(Creature *pCreature) : Scripted_NoMovementAI(pCreature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            DoCast(me, SPELL_FROST_BOMB);
            uiFrostTimer = 10000;
        }

        uint32 uiFrostTimer;
        
        void UpdateAI(uint32 diff) override
        {
            if (uiFrostTimer <= diff)
            {
                DoCastAOE(SPELL_FLAME_SUPPRESSANT_2);
                for (int8 n = 0; n < 2; n++)
                {
                    uint32 npc;
                    if (n == 0)
                        npc = NPC_FLAME;
                    else 
                        npc = NPC_FLAME_SPREAD;
                    std::list<Creature*> m_pCreatures;
                    GetCreatureListWithEntryInGrid(m_pCreatures, me, npc, 25);
                    if (!m_pCreatures.empty())
                        for(std::list<Creature*>::iterator iter = m_pCreatures.begin(); iter != m_pCreatures.end(); ++iter)
                            (*iter)->DespawnOrUnsummon(1000);
                }
                uiFrostTimer = 10000;
            }
            else uiFrostTimer -= diff;
        }
    };

};

class npc_boom_bot: public CreatureScript
{
public:
    npc_boom_bot() : CreatureScript("npc_boom_bot") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new npc_boom_botAI (pCreature);
    }

    struct npc_boom_botAI : public ScriptedAI
    {
        npc_boom_botAI(Creature *pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
        }

        InstanceScript* instance;

        bool Boom;

        void Reset() override
        {
            Boom = true;
        }

        void UpdateAI(uint32 diff) override
        {
            if (me->getVictim() && me->GetDistance(me->getVictim()) <= 3.0f)
                if (Boom)
                {
                    Boom = false;
                    DoCast(me->getVictim(), 63801, true);
                    me->Kill(me, true);
                }
        }

        void SpellHitTarget(Unit*target, SpellInfo const* spell) override
        {
            if (spell->Id == 63801 && target->GetTypeId() == TYPEID_PLAYER && instance)
                if (Creature* Mimiron = me->GetCreature(*me, instance->GetGuidData(DATA_MIMIRON)))
                    Mimiron->AI()->DoAction(ACTION_BOOM_FAIL);
        }
    };

}; 

class achievement_mine : public AchievementCriteriaScript
{
    public:
        achievement_mine() : AchievementCriteriaScript("achievement_mine")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
            
            if (Creature * Mim = target->ToCreature())
                if (boss_mimiron::boss_mimironAI* MimAI = CAST_AI(boss_mimiron::boss_mimironAI, Mim->AI()))
                    if (MimAI->Getachivmine())
                        return true;

            return false;
        }
};

class achievement_rocket : public AchievementCriteriaScript
{
    public:
        achievement_rocket() : AchievementCriteriaScript("achievement_rocket")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
            
            if (Creature * Mim = target->ToCreature())
                if (boss_mimiron::boss_mimironAI* MimAI = CAST_AI(boss_mimiron::boss_mimironAI, Mim->AI()))
                    if (MimAI->Getachivrocket())
                        return true;

            return false;
        }
};

class achievement_boom : public AchievementCriteriaScript
{
    public:
        achievement_boom() : AchievementCriteriaScript("achievement_boom")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
            
            if (Creature * Mim = target->ToCreature())
                if (boss_mimiron::boss_mimironAI* MimAI = CAST_AI(boss_mimiron::boss_mimironAI, Mim->AI()))
                    if (MimAI->Getachivboom())
                        return true;

            return false;
        }
};

class achievement_firefighter : public AchievementCriteriaScript
{
    public:
        achievement_firefighter() : AchievementCriteriaScript("achievement_firefighter")
        {
        }

        bool OnCheck(Player* player, Unit* target) override
        {
            if (!target)
                return false;
            
            if (Creature * Mim = target->ToCreature())
                if (boss_mimiron::boss_mimironAI* MimAI = CAST_AI(boss_mimiron::boss_mimironAI, Mim->AI()))
                    if (MimAI->GetHardMode())
                        return true;

            return false;
        }
};

void AddSC_boss_mimiron()
{
    new boss_mimiron();
    new boss_leviathan_mk();
    new boss_leviathan_mk_turret();
    new npc_proximity_mine();
    new boss_vx_001();
    new npc_rocket_strike();
    new boss_aerial_unit();
    new npc_magnetic_core();
    new npc_assault_bot();
    new npc_emergency_bot();
    new go_not_push_button();
    new npc_mimiron_flame_trigger();
    new npc_mimiron_flame_spread();
    new npc_frost_bomb();
    new npc_boom_bot();
    new achievement_boom();
    new achievement_rocket();
    new achievement_mine();
    new achievement_firefighter();
}
