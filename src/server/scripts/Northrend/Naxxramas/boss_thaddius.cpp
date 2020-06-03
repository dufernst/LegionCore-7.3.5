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

#include "naxxramas.h"

//Stalagg
enum StalaggYells
{
    SAY_STAL_AGGRO          = -1533023, //not used
    SAY_STAL_SLAY           = -1533024, //not used
    SAY_STAL_DEATH          = -1533025  //not used
};

enum StalagSpells
{
    SPELL_POWERSURGE        = 28134,
    H_SPELL_POWERSURGE      = 54529,
    SPELL_MAGNETIC_PULL     = 28338,
    SPELL_STALAGG_TESLA     = 28097
};

//Feugen
enum FeugenYells
{
    SAY_FEUG_AGGRO          = -1533026, //not used
    SAY_FEUG_SLAY           = -1533027, //not used
    SAY_FEUG_DEATH          = -1533028 //not used
};

enum FeugenSpells
{
    SPELL_STATICFIELD       = 28135,
    H_SPELL_STATICFIELD     = 54528,
    SPELL_FEUGEN_TESLA      = 28109
};

// Thaddius DoAction
enum ThaddiusActions
{
    ACTION_FEUGEN_RESET,
    ACTION_FEUGEN_DIED,
    ACTION_STALAGG_RESET,
    ACTION_STALAGG_DIED,
    ACTION_SHOCKED
};

//generic
#define C_TESLA_COIL            16218           //the coils (emotes "Tesla Coil overloads!")

enum TeslaSpells
{
    SPELL_SHOCK                 = 28099,
    SPELL_STALAGG_CHAIN         = 28096,
    SPELL_STALAGG_TESLA_PASSIVE = 28097,
    SPELL_FEUGEN_TESLA_PASSIVE  = 28109,
    SPELL_FEUGEN_CHAIN          = 28111
};

//Thaddius
enum ThaddiusYells
{
    SAY_GREET               = -1533029, //not used
    SAY_AGGRO_1             = -1533030,
    SAY_AGGRO_2             = -1533031,
    SAY_AGGRO_3             = -1533032,
    SAY_SLAY                = -1533033,
    SAY_ELECT               = -1533034, //not used
    SAY_DEATH               = -1533035,
    SAY_SCREAM1             = -1533036, //not used
    SAY_SCREAM2             = -1533037, //not used
    SAY_SCREAM3             = -1533038, //not used
    SAY_SCREAM4             = -1533039, //not used
    EMOTE_LOSING_LINK       = -1533149, //not used
    EMOTE_TESLA_OVERLOAD    = -1533150, //not used
    EMOTE_POLARITY_SHIFT    = -1533151 //not used
};

enum ThaddiusSpells
{
    SPELL_POLARITY_SHIFT        = 28089,
    SPELL_BALL_LIGHTNING        = 28299,
    SPELL_POSITIVE_CHARGE       = 29659,
    SPELL_NEGATIVE_CHARGE       = 29660,
    SPELL_CHAIN_LIGHTNING       = 28167,
    H_SPELL_CHAIN_LIGHTNING     = 54531,
    SPELL_BERSERK               = 27680,
    SPELL_POLARITY_CHARGE_1     = 28059,
    SPELL_POLARITY_CHARGE_2     = 39088,
    SPELL_POLARITY_CHARGE_3     = 39091,
    SPELL_POLARITY_CHARGE_4     = 28084,
//   SPELL_THADIUS_SPAWN       = 28160,
};

enum Events
{
    EVENT_NONE,
    EVENT_OVERLOAD,
    EVENT_SHIFT,
    EVENT_CHAIN,
    EVENT_BERSERK,
};

enum Achievements
{
    ACHIEVEMENT_SHOCKING_10 = 2178,
    ACHIEVEMENT_SHOCKING_25 = 2179
};
class boss_thaddius : public CreatureScript
{
public:
    boss_thaddius() : CreatureScript("boss_thaddius") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new boss_thaddiusAI (pCreature);
    }

    struct boss_thaddiusAI : public BossAI
    {
        boss_thaddiusAI(Creature *c) : BossAI(c, BOSS_THADDIUS)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            // init is a bit tricky because thaddius shall track the life of both adds, but not if there was a wipe
            // and, in particular, if there was a crash after both adds were killed (should not respawn)

            // Moreover, the adds may not yet be spawn. So just track down the status if mob is spawn
            // and each mob will send its status at reset (meaning that it is alive)
            checkFeugenAlive = false;
            if (Creature *pFeugen = me->GetCreature(*me, instance->GetGuidData(DATA_FEUGEN)))
                checkFeugenAlive = pFeugen->isAlive();

            checkStalaggAlive = false;
            if (Creature *pStalagg = me->GetCreature(*me, instance->GetGuidData(DATA_STALAGG)))
                checkStalaggAlive = pStalagg->isAlive();

            if (!checkFeugenAlive && !checkStalaggAlive)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                me->SetReactState(REACT_AGGRESSIVE);
            }
            else
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                me->SetReactState(REACT_PASSIVE);
            }
        }

        bool checkStalaggAlive;
        bool checkFeugenAlive;
        bool bShocked;
        uint32 uiAddsTimer;

        void Reset() override
        {
            _Reset();
            bShocked = false;
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            if (!(rand()%5))
                DoScriptText(SAY_SLAY, me);
        }

        void JustDied(Unit* /*Killer*/) override
        {
            _JustDied();
            DoScriptText(SAY_DEATH, me);
            if (InstanceScript *instance = me->GetInstanceScript())
            {
                if (!bShocked)
                     instance->DoCompleteAchievement(RAID_MODE(ACHIEVEMENT_SHOCKING_10, ACHIEVEMENT_SHOCKING_25));
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POSITIVE_CHARGE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NEGATIVE_CHARGE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POLARITY_CHARGE_1);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POLARITY_CHARGE_2);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POLARITY_CHARGE_3);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_POLARITY_CHARGE_4);
            }
        }

        void DoAction(const int32 action) override
        {
            switch (action)
            {
                case ACTION_FEUGEN_RESET:
                    checkFeugenAlive = true;
                    break;
                case ACTION_FEUGEN_DIED:
                    checkFeugenAlive = false;
                    break;
                case ACTION_STALAGG_RESET:
                    checkStalaggAlive = true;
                    break;
                case ACTION_STALAGG_DIED:
                    checkStalaggAlive = false;
                    break;
                case ACTION_SHOCKED:
                    bShocked = true;
                    break;
            }

            if (!checkFeugenAlive && !checkStalaggAlive)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                // REACT_AGGRESSIVE only reset when he takes damage.
                DoZoneInCombat();
            }
            else
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_STUNNED);
                me->SetReactState(REACT_PASSIVE);
            }
        }

        void EnterCombat(Unit * /*who*/) override
        {
            _EnterCombat();
            DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);
            events.ScheduleEvent(EVENT_OVERLOAD, 15000);
            events.ScheduleEvent(EVENT_SHIFT, 30000);
            events.ScheduleEvent(EVENT_CHAIN, urand(10000, 20000));
            events.ScheduleEvent(EVENT_BERSERK, 360000);
        }

      void EnterEvadeMode() override
      {
          CreatureAI::EnterEvadeMode();
          Reset();
          if (Creature *pStalagg = me->GetCreature(*me, instance->GetGuidData(DATA_STALAGG)))
          {
              pStalagg->Respawn();
              pStalagg->GetMotionMaster()->MovePoint(0, pStalagg->GetHomePosition());
          }
          if (Creature *pFeugen = me->GetCreature(*me, instance->GetGuidData(DATA_FEUGEN)))
          {
              pFeugen->Respawn();
              pFeugen->GetMotionMaster()->MovePoint(0, pFeugen->GetHomePosition());
          }
      }

        void DamageTaken(Unit * /*pDoneBy*/, uint32 & /*uiDamage*/, DamageEffectType dmgType) override
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (checkFeugenAlive && checkStalaggAlive)
                uiAddsTimer = 0;

            if (checkStalaggAlive != checkFeugenAlive)
            {
                uiAddsTimer += diff;
                if (uiAddsTimer > 5000)
                {
                    if (!checkStalaggAlive)
                    {
                        if (instance)
                            if (Creature *pStalagg = me->GetCreature(*me, instance->GetGuidData(DATA_STALAGG)))
                            {
                                pStalagg->Respawn();
                                pStalagg->GetMotionMaster()->MovePoint(0, pStalagg->GetHomePosition());
                            }
                    }
                    else
                    {
                        if (instance)
                            if (Creature *pFeugen = me->GetCreature(*me, instance->GetGuidData(DATA_FEUGEN)))
                            {
                                pFeugen->Respawn();
                                pFeugen->GetMotionMaster()->MovePoint(0, pFeugen->GetHomePosition());
                            }
                    }
                }
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_OVERLOAD:
                        if (Creature *pTesla = me->FindNearestCreature(C_TESLA_COIL, 50))
                            DoScriptText(EMOTE_TESLA_OVERLOAD, pTesla);
                        if (instance)
                        {
                            if (GameObject* go = GameObject::GetGameObject(*me, instance->GetGuidData(DATA_THADDIUS_TESLA05)))
                                go->UseDoorOrButton();
                            if (GameObject* go = GameObject::GetGameObject(*me, instance->GetGuidData(DATA_THADDIUS_TESLA06)))
                                go->UseDoorOrButton();
                        }
                        break;
                    case EVENT_SHIFT:
                        DoCastAOE(SPELL_POLARITY_SHIFT);
                        events.ScheduleEvent(EVENT_SHIFT, 30000);
                        return;
                    case EVENT_CHAIN:
                        DoCast(me->getVictim(), RAID_MODE(SPELL_CHAIN_LIGHTNING, H_SPELL_CHAIN_LIGHTNING));
                        events.ScheduleEvent(EVENT_CHAIN, urand(10000, 20000));
                        return;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK);
                        return;
                }
            }

            if (events.GetTimer() > 15000 && !me->IsWithinMeleeRange(me->getVictim()))
                DoCast(me->getVictim(), SPELL_BALL_LIGHTNING);
            else
                DoMeleeAttackIfReady();
        }
    };

};

class mob_stalagg : public CreatureScript
{
public:
    mob_stalagg() : CreatureScript("mob_stalagg") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_stalaggAI(pCreature);
    }

    struct mob_stalaggAI : public ScriptedAI
    {
        mob_stalaggAI(Creature *c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 powerSurgeTimer;
        uint32 magneticPullTimer;
        uint32 uiIdleTimer;
        uint32 uiShockTimer;
        ObjectGuid uiTeslaGuid;
        uint32 uiChainTimer;
        bool bChainReset;
        bool bShock;
        bool bSwitch;
        Position homePosition;

        void Reset() override
        {
            if (instance)
            {
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetGuidData(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_STALAGG_RESET);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetGuidData(DATA_THADDIUS_TESLA06)))
                    go->ResetDoorOrButton();
            }
            powerSurgeTimer = urand(20000, 25000);
            magneticPullTimer = 20000;
            uiIdleTimer = 3*IN_MILLISECONDS;
            uiShockTimer = 1*IN_MILLISECONDS;
            uiChainTimer = 10*IN_MILLISECONDS;
            bChainReset = true;
            bShock = false;
            bSwitch = false;
            homePosition = me->GetHomePosition();
        }

        void JustDied(Unit * /*killer*/) override
        {
            if (instance)
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetGuidData(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_STALAGG_DIED);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bChainReset)
            {
                if (uiChainTimer <= uiDiff)
                {
                    if (Creature *pTesla = me->FindNearestCreature(C_TESLA_COIL, 50))
                    {
                        uiTeslaGuid = pTesla->GetGUID();
                        pTesla->CastSpell(me, SPELL_STALAGG_CHAIN, false);
                    }
                    bChainReset = false;
                    uiChainTimer = 3*IN_MILLISECONDS;
                } else uiChainTimer -= uiDiff;
            }

            if (!UpdateVictim())
                return;

            if (magneticPullTimer <= uiDiff)
            {
                if (Creature *pFeugen = me->GetCreature(*me, instance->GetGuidData(DATA_FEUGEN)))
                {
                    Unit* pStalaggVictim = me->getVictim();
                    Unit* pFeugenVictim = pFeugen->getVictim();

                    if (pFeugenVictim && pStalaggVictim)
                    {
                        // magnetic pull is not working. So just jump.

                        // reset aggro to be sure that feugen will not follow the jump
                        float uiTempThreat = pFeugen->getThreatManager().getThreat(pFeugenVictim);
                        pFeugen->getThreatManager().modifyThreatPercent(pFeugenVictim, -100);
                        pFeugenVictim->JumpTo(me, 0.3f);
                        pFeugen->AddThreat(pStalaggVictim, uiTempThreat);
                        pFeugen->SetReactState(REACT_PASSIVE);

                        uiTempThreat = me->getThreatManager().getThreat(pStalaggVictim);
                        me->getThreatManager().modifyThreatPercent(pStalaggVictim, -100);
                        pStalaggVictim->JumpTo(pFeugen, 0.3f);
                        me->AddThreat(pFeugenVictim, uiTempThreat);
                        me->SetReactState(REACT_PASSIVE);
                        uiIdleTimer = 3*IN_MILLISECONDS;
                        bSwitch = true;
                    }
                }

                magneticPullTimer = 20000;
            }
            else magneticPullTimer -= uiDiff;

            if (powerSurgeTimer <= uiDiff)
            {
                DoCast(me, RAID_MODE(SPELL_POWERSURGE, H_SPELL_POWERSURGE));
                powerSurgeTimer = urand(15000, 20000);
            } else powerSurgeTimer -= uiDiff;

            if (bSwitch)
                if (uiIdleTimer <= uiDiff)
                {
                    if (Creature *pFeugen = me->GetCreature(*me, instance->GetGuidData(DATA_FEUGEN)))
                        pFeugen->SetReactState(REACT_AGGRESSIVE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    bSwitch = false;
                } else uiIdleTimer -= uiDiff;

            if (me->GetDistance(homePosition) > 15)
            {
                if (uiShockTimer <= uiDiff)
                {
                    if (Creature *pTesla = Creature::GetCreature(*me, uiTeslaGuid))
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                            pTesla->CastSpell(pTarget, SPELL_SHOCK, false);
                    uiShockTimer = 1*IN_MILLISECONDS;
                    bShock = true;
                }else uiShockTimer -= uiDiff;
            } else if (bShock)
            {
                bShock = false;
                bChainReset = true;
            }

            DoMeleeAttackIfReady();
        }
    };

};

class mob_feugen : public CreatureScript
{
public:
    mob_feugen() : CreatureScript("mob_feugen") { }

    CreatureAI* GetAI(Creature* pCreature) const override
    {
        return new mob_feugenAI(pCreature);
    }

    struct mob_feugenAI : public ScriptedAI
    {
        mob_feugenAI(Creature *c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 staticFieldTimer;
        uint32 uiShockTimer;
        ObjectGuid uiTeslaGuid;
        uint32 uiChainTimer;
        bool bChainReset;
        bool bShock;
        Position homePosition;

        void Reset() override
        {
            if (instance)
            {
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetGuidData(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_FEUGEN_RESET);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetGuidData(DATA_THADDIUS_TESLA05)))
                    go->ResetDoorOrButton();
            }
            staticFieldTimer = 5000;
            uiShockTimer = 1*IN_MILLISECONDS;
            uiChainTimer = 10*IN_MILLISECONDS;
            bChainReset = true;
            bShock = false;
            homePosition = me->GetHomePosition();
        }

        void JustDied(Unit * /*killer*/) override
        {
            if (instance)
                if (Creature *pThaddius = me->GetCreature(*me, instance->GetGuidData(DATA_THADDIUS)))
                    if (pThaddius->AI())
                        pThaddius->AI()->DoAction(ACTION_FEUGEN_DIED);
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bChainReset)
            {
                if (uiChainTimer <= uiDiff)
                {
                    if (Creature *pTesla = me->FindNearestCreature(C_TESLA_COIL, 50))
                    {
                        uiTeslaGuid = pTesla->GetGUID();
                        pTesla->CastSpell(me, SPELL_FEUGEN_CHAIN, false);
                    }
                    bChainReset = false;
                    uiChainTimer = 3*IN_MILLISECONDS;
                } else uiChainTimer -= uiDiff;
            }

            if (!UpdateVictim())
                return;

            if (staticFieldTimer <= uiDiff)
            {
                DoCast(me, RAID_MODE(SPELL_STATICFIELD, H_SPELL_STATICFIELD));
                staticFieldTimer = 5000;
            } else staticFieldTimer -= uiDiff;

            if (me->GetDistance(homePosition) > 15)
            {
                if (uiShockTimer <= uiDiff)
                {
                    if (Creature *pTesla = Creature::GetCreature(*me, uiTeslaGuid))
                        if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 200, true))
                            pTesla->CastSpell(pTarget, SPELL_SHOCK, false);
                    uiShockTimer = 1*IN_MILLISECONDS;
                    bShock = true;
                }else uiShockTimer -= uiDiff;
            } else if (bShock)
            {
                bShock = false;
                bChainReset = true;
            }
            DoMeleeAttackIfReady();
        }
    };

};

void AddSC_boss_thaddius()
{
    new boss_thaddius();
    new mob_stalagg();
    new mob_feugen();
}