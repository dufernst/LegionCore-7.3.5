/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "ObjectDefines.h"
#include "GridDefines.h"
#include "GridNotifiers.h"
#include "SpellMgr.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "Group.h"
#include "SmartAI.h"
#include "ScriptMgr.h"

SmartAI::~SmartAI()
{
}

SmartAI::SmartAI(Creature* c) : CreatureAI(c), mFollowArrivedTimer(0)
{
    // copy script to local (protection for table reload)

    mWayPoints = nullptr;
    mEscortState = SMART_ESCORT_NONE;
    mCurrentWPID = 0;//first wp id is 1 !!
    mWPReached = false;
    mWPPauseTimer = 0;
    mLastWP = nullptr;

    mCanRepeatPath = false;

    // spawn in run mode
    me->SetWalk(false);
    mRun = false;

    mEvadeDisabled = false;

    me->GetPosition(&mLastOOCPos);

    mCanAutoAttack = true;
    mCanCombatMove = true;

    mForcedPaused = false;
    mLastWPIDReached = 0;

    mEscortQuestID = 0;

    mDespawnTime = 0;
    mDespawnState = 0;

    mEscortInvokerCheckTimer = 1000;
    mFollowGuid = ObjectGuid::Empty;
    mFollowDist = 0;
    mFollowAngle = 0;
    mFollowCredit = 0;
    mFollowArrivedEntry = 0;
    mFollowCreditType = 0;
    mInvincibilityHpLevel = 0;
    _bossId = c->GetBossId();
    instance = c->GetInstanceScript();
    _boundary = instance ? instance->GetBossBoundary(_bossId) : nullptr;
}

void SmartAI::UpdateDespawn(const uint32 diff)
{
    if (mDespawnState <= 1 || mDespawnState > 3)
        return;

    if (mDespawnTime < diff)
    {
        if (mDespawnState == 2)
        {
            me->SetVisible(false);
            mDespawnTime = 5000;
            mDespawnState++;
        }
        else
            me->DespawnOrUnsummon();
    }
    else mDespawnTime -= diff;
}

void SmartAI::Reset()
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))//dont mess up escort movement after combat
        SetRun(mRun);
    if (instance && _bossId)
    {
        instance->SetBossState(_bossId, NOT_STARTED);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_END, me);
    }
    GetScript()->OnReset();
}

WayPoint* SmartAI::GetNextWayPoint()
{
    if (!mWayPoints || mWayPoints->empty())
        return nullptr;

    mCurrentWPID++;
    WPPath::const_iterator itr = mWayPoints->find(mCurrentWPID);
    if (itr != mWayPoints->end())
    {
        mLastWP = (*itr).second;
        if (mLastWP->id != mCurrentWPID)
        {
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "SmartAI::GetNextWayPoint: Got not expected waypoint id %u, expected %u", mLastWP->id, mCurrentWPID);
        }
        return (*itr).second;
    }
    return nullptr;
}

bool SmartAI::HasEscortState(uint32 uiEscortState)
{
    return (mEscortState & uiEscortState);
}

void SmartAI::AddEscortState(uint32 uiEscortState)
{
    mEscortState |= uiEscortState;
}

void SmartAI::RemoveEscortState(uint32 uiEscortState)
{
    mEscortState &= ~uiEscortState;
}

void SmartAI::SetAutoAttack(bool on)
{
    mCanAutoAttack = on;
}

void SmartAI::StartPath(bool run, uint32 path, bool repeat, Unit* /*invoker*/)
{
    if (me->isInCombat())// no wp movement in combat
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "SmartAI::StartPath: Creature entry %u wanted to start waypoint movement while in combat, ignoring.", me->GetEntry());
        return;
    }
    if (HasEscortState(SMART_ESCORT_ESCORTING))
        StopPath();
    if (path)
        if (!LoadPath(path))
            return;
    if (!mWayPoints || mWayPoints->empty())
        return;

    AddEscortState(SMART_ESCORT_ESCORTING);
    mCanRepeatPath = repeat;

    SetRun(run);

    if (WayPoint* wp = GetNextWayPoint())
    {
        me->GetPosition(&mLastOOCPos);
        me->GetMotionMaster()->MovePoint(wp->id, wp->x, wp->y, wp->z);
        GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_START, nullptr, wp->id, GetScript()->GetPathId());
    }
}

bool SmartAI::LoadPath(uint32 entry)
{
    if (HasEscortState(SMART_ESCORT_ESCORTING))
        return false;
    mWayPoints = sSmartWaypointMgr->GetPath(entry);
    if (!mWayPoints)
    {
        GetScript()->SetPathId(0);
        return false;
    }
    //should be activ for correct move betwin grid while relocating plr for example
    me->setActive(true);
    GetScript()->SetPathId(entry);
    return true;
}

void SmartAI::PausePath(uint32 delay, bool forced)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
        return;
    if (HasEscortState(SMART_ESCORT_PAUSED))
    {
        TC_LOG_DEBUG(LOG_FILTER_GENERAL, "SmartAI::StartPath: Creature entry %u wanted to pause waypoint movement while already paused, ignoring.", me->GetEntry());
        return;
    }
    mForcedPaused = forced;
    me->GetPosition(&mLastOOCPos);
    AddEscortState(SMART_ESCORT_PAUSED);
    mWPPauseTimer = delay;
    if (forced)
    {
        SetRun(mRun);
        me->StopMoving();//force stop
        me->GetMotionMaster()->MoveIdle();//force stop
    }
    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_PAUSED, nullptr, mLastWP->id, GetScript()->GetPathId());
}

void SmartAI::StopPath(uint32 DespawnTime, uint32 quest, bool fail)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
        return;

    if (quest)
        mEscortQuestID = quest;
    SetDespawnTime(DespawnTime);
    //mDespawnTime = DespawnTime;

    me->GetPosition(&mLastOOCPos);
    me->StopMoving();//force stop
    me->GetMotionMaster()->MoveIdle();
    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_STOPPED, nullptr, mLastWP->id, GetScript()->GetPathId());
    EndPath(fail);
}

void SmartAI::EndPath(bool fail)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_ENDED, nullptr, mLastWP->id, GetScript()->GetPathId());

    RemoveEscortState(SMART_ESCORT_ESCORTING | SMART_ESCORT_PAUSED | SMART_ESCORT_RETURNING);
    mWayPoints = nullptr;
    mCurrentWPID = 0;
    mWPPauseTimer = 0;
    mLastWP = nullptr;

    if (mCanRepeatPath)
        StartPath(mRun, GetScript()->GetPathId(), mCanRepeatPath);
    else
        GetScript()->SetPathId(0);

    ObjectList* targets = GetScript()->GetTargetList(SMART_ESCORT_TARGETS);
    if (targets && mEscortQuestID)
    {
        if (targets->size() == 1 && GetScript()->IsPlayer((*targets->begin())))
        {
            Player* player = (*targets->begin())->ToPlayer();
            if (!fail && player->IsAtGroupRewardDistance(me) && !player->GetCorpse())
                player->GroupEventHappens(mEscortQuestID, me);

            if (fail && player->GetQuestStatus(mEscortQuestID) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(mEscortQuestID);

            if (Group* group = player->GetGroup())
            {
                for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                {
                    Player* groupGuy = groupRef->getSource();

                    if (!fail && groupGuy->IsAtGroupRewardDistance(me) && !groupGuy->GetCorpse())
                        groupGuy->AreaExploredOrEventHappens(mEscortQuestID);
                    if (fail && groupGuy->GetQuestStatus(mEscortQuestID) == QUEST_STATUS_INCOMPLETE)
                        groupGuy->FailQuest(mEscortQuestID);
                }
            }
        }
        else
        {
            for (ObjectList::iterator iter = targets->begin(); iter != targets->end(); ++iter)
            {
                if (GetScript()->IsPlayer((*iter)))
                {
                    Player* player = (*iter)->ToPlayer();
                    if (!fail && player->IsAtGroupRewardDistance(me) && !player->GetCorpse())
                        player->AreaExploredOrEventHappens(mEscortQuestID);
                    if (fail && player->GetQuestStatus(mEscortQuestID) == QUEST_STATUS_INCOMPLETE)
                        player->FailQuest(mEscortQuestID);
                }
            }
        }
    }
    if (mDespawnState == 1)
        StartDespawn();
}

void SmartAI::ResumePath()
{
    //mWPReached = false;
    SetRun(mRun);
    if (mLastWP)
        me->GetMotionMaster()->MovePoint(mLastWP->id, mLastWP->x, mLastWP->y, mLastWP->z);
}

void SmartAI::ReturnToLastOOCPos()
{
    SetRun(mRun);
    me->GetMotionMaster()->MovePoint(SMART_ESCORT_LAST_OOC_POINT, mLastOOCPos);
}

void SmartAI::UpdatePath(const uint32 diff)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
        return;
    if (mEscortInvokerCheckTimer < diff)
    {
        if (!IsEscortInvokerInRange())
        {
            StopPath(mDespawnTime, mEscortQuestID, true);
        }
        mEscortInvokerCheckTimer = 1000;
    }
    else mEscortInvokerCheckTimer -= diff;
    // handle pause
    if (HasEscortState(SMART_ESCORT_PAUSED))
    {
        if (mWPPauseTimer < diff)
        {
            if (!me->isInCombat() && !HasEscortState(SMART_ESCORT_RETURNING) && (mWPReached || mLastWPIDReached == SMART_ESCORT_LAST_OOC_POINT || mForcedPaused))
            {
                GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_RESUMED, nullptr, mLastWP->id, GetScript()->GetPathId());
                RemoveEscortState(SMART_ESCORT_PAUSED);
                if (mForcedPaused)// if paused between 2 wps resend movement
                {
                    ResumePath();
                    mWPReached = false;
                    mForcedPaused = false;
                }
                if (mLastWPIDReached == SMART_ESCORT_LAST_OOC_POINT)
                    mWPReached = true;
            }
            mWPPauseTimer = 0;
        }
        else {
            mWPPauseTimer -= diff;
        }
    }
    if (HasEscortState(SMART_ESCORT_RETURNING))
    {
        if (mWPReached)//reached OOC WP
        {
            RemoveEscortState(SMART_ESCORT_RETURNING);
            if (!HasEscortState(SMART_ESCORT_PAUSED))
                ResumePath();
            mWPReached = false;
        }
    }
    if (me->isInCombat() || HasEscortState(SMART_ESCORT_PAUSED | SMART_ESCORT_RETURNING))
        return;
    // handle next wp
    if (mWPReached)//reached WP
    {
        mWPReached = false;
        if (mCurrentWPID == GetWPCount())
        {
            EndPath();
        }
        else if (WayPoint* wp = GetNextWayPoint())
        {
            SetRun(mRun);
            me->GetMotionMaster()->MovePoint(wp->id, wp->x, wp->y, wp->z);
        }
    }
}

void SmartAI::UpdateAI(uint32 diff)
{
    GetScript()->OnUpdate(diff);
    UpdatePath(diff);
    UpdateDespawn(diff);

    //TODO move to void
    if (!mFollowGuid.IsEmpty())
    {
        if (mFollowArrivedTimer < diff)
        {
            if (me->FindNearestCreature(mFollowArrivedEntry, INTERACTION_DISTANCE, true))
            {
                if (Player* player = me->GetPlayer(*me, mFollowGuid))
                {
                    if (!mFollowCreditType)
                        player->RewardPlayerAndGroupAtEvent(mFollowCredit, me);
                    else
                        player->GroupEventHappens(mFollowCredit, me);
                }
                mFollowGuid = ObjectGuid::Empty;
                mFollowDist = 0;
                mFollowAngle = 0;
                mFollowCredit = 0;
                mFollowArrivedTimer = 1000;
                mFollowArrivedEntry = 0;
                mFollowCreditType = 0;
                SetDespawnTime(5000);
                me->StopMoving();
                me->GetMotionMaster()->MoveIdle();
                StartDespawn();
                GetScript()->ProcessEventsFor(SMART_EVENT_FOLLOW_COMPLETED);
                return;
            }
            mFollowArrivedTimer = 1000;
        }
        else mFollowArrivedTimer -= diff;
    }

    if (!UpdateVictim())
        return;

    if (mCanAutoAttack)
        DoMeleeAttackIfReady();
}

bool SmartAI::IsEscortInvokerInRange()
{
    ObjectList* targets = GetScript()->GetTargetList(SMART_ESCORT_TARGETS);
    if (targets)
    {
        if (targets->size() == 1 && GetScript()->IsPlayer((*targets->begin())))
        {
            Player* player = (*targets->begin())->ToPlayer();
            if (me->GetDistance(player) <= SMART_ESCORT_MAX_PLAYER_DIST)
                return true;

            if (Group* group = player->GetGroup())
            {
                for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                {
                    Player* groupGuy = groupRef->getSource();

                    if (me->GetDistance(groupGuy) <= SMART_ESCORT_MAX_PLAYER_DIST)
                        return true;
                }
            }
        }
        else
        {
            for (ObjectList::iterator iter = targets->begin(); iter != targets->end(); ++iter)
            {
                if (GetScript()->IsPlayer((*iter)))
                {
                    if (me->GetDistance((*iter)->ToPlayer()) <= SMART_ESCORT_MAX_PLAYER_DIST)
                        return true;
                }
            }
        }
    }
    return true;//escort targets were not set, ignore range check
}

void SmartAI::MovepointReached(uint32 id)
{
    if (id != SMART_ESCORT_LAST_OOC_POINT && mLastWPIDReached != id)
        GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_REACHED, nullptr, id);

    mLastWPIDReached = id;
    mWPReached = true;
}

void SmartAI::MovementInform(uint32 MovementType, uint32 Data)
{
    if ((MovementType == POINT_MOTION_TYPE && Data == SMART_ESCORT_LAST_OOC_POINT) || (MovementType == FOLLOW_MOTION_TYPE))
        me->ClearUnitState(UNIT_STATE_EVADE);

    GetScript()->ProcessEventsFor(SMART_EVENT_MOVEMENTINFORM, nullptr, MovementType, Data);
    if (MovementType != POINT_MOTION_TYPE || !HasEscortState(SMART_ESCORT_ESCORTING))
        return;
    MovepointReached(Data);
}

void SmartAI::RemoveAuras()
{
    // Only loop throught the applied auras, because here is where all auras on the current unit are stored
    Unit::AuraApplicationMap appliedAuras = me->GetAppliedAuras();
    for (Unit::AuraApplicationMap::iterator iter = appliedAuras.begin(); iter != appliedAuras.end(); ++iter)
    {
        Aura const* aura = iter->second->GetBase();
        if (!aura->GetSpellInfo()->IsPassive() && !aura->GetSpellInfo()->HasAura(SPELL_AURA_CONTROL_VEHICLE) && aura->GetCaster() != me)
            me->RemoveAurasDueToSpell(aura->GetId());
    }
}

void SmartAI::EnterEvadeMode()
{
    if (!me->isAlive() || me->IsInEvadeMode())
        return;

    if (mEvadeDisabled)
    {
        GetScript()->ProcessEventsFor(SMART_EVENT_EVADE);
        return;
    }

    RemoveAuras();

    me->AddUnitState(UNIT_STATE_EVADE);
    me->DeleteThreatList();
    me->ClearSaveThreatTarget();
    me->CombatStop(true);
    me->LoadCreaturesAddon();
    me->SetLootRecipient(nullptr);
    me->ResetPlayerDamageReq();

    //me->m_Events.AddEvent(new SetImuneDelayEvent(*me), me->m_Events.CalculateTime(8000));
    //me->SetReactState(REACT_PASSIVE);
    //me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

    GetScript()->ProcessEventsFor(SMART_EVENT_EVADE);//must be after aura clear so we can cast spells from db

    SetRun(mRun);
    if (HasEscortState(SMART_ESCORT_ESCORTING))
    {
        AddEscortState(SMART_ESCORT_RETURNING);
        ReturnToLastOOCPos();
    }
    else if (!mFollowGuid.IsEmpty())
    {
        if (Unit* target = me->GetUnit(*me, mFollowGuid))
            me->GetMotionMaster()->MoveFollow(target, mFollowDist, mFollowAngle);
    }
    else
        me->GetMotionMaster()->MoveTargetedHome();

    Reset();
}

void SmartAI::MoveInLineOfSight(Unit* who)
{
    if (!who)
        return;

    GetScript()->OnMoveInLineOfSight(who);

    if (me->HasReactState(REACT_PASSIVE) || AssistPlayerInCombat(who))
        return;

    if (!CanAIAttack(who))
        return;

    if (!me->canStartAttack(who, false))
        return;

    if (me->IsHostileTo(who))
    {
        float fAttackRadius = me->GetAttackDistance(who);
        if (me->IsWithinDistInMap(who, fAttackRadius) && me->IsWithinLOSInMap(who))
        {
            if (!me->getVictim())
            {
                who->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                AttackStart(who);
            }
            else if (!who->IsPlayer())
            {
                who->SetInCombatWith(me);
                me->AddThreat(who, 0.0f);
            }
        }
    }
}

bool SmartAI::CanAIAttack(const Unit* /*who*/) const
{
    if (me->GetReactState() == REACT_PASSIVE)
        return false;
    return true;
}

uint32 SmartAI::GetWPCount()
{
    return mWayPoints ? mWayPoints->size() : 0;
}

bool SmartAI::AssistPlayerInCombat(Unit* who)
{
    if (!who || !who->getVictim())
        return false;

    //experimental (unknown) flag not present
    if (!(me->GetCreatureTemplate()->TypeFlags[0] & CREATURE_TYPEFLAGS_CAN_ASSIST))
        return false;

    //not a player
    if (!who->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    //never attack friendly
    if (me->IsFriendlyTo(who))
        return false;

    //too far away and no free sight?
    if (me->IsWithinDistInMap(who, SMART_MAX_AID_DIST) && me->IsWithinLOSInMap(who))
    {
        //already fighting someone?
        if (!me->getVictim())
        {
            AttackStart(who);
            return true;
        }
        who->SetInCombatWith(me);
        me->AddThreat(who, 0.0f);
        return true;
    }

    return false;
}

void SmartAI::JustRespawned()
{
    mDespawnTime = 0;
    mDespawnState = 0;
    mEscortState = SMART_ESCORT_NONE;
    if (!me->IsVisible())
        me->SetVisible(true);
    if (me->getFaction() != me->GetCreatureTemplate()->faction)
        me->RestoreFaction();
    GetScript()->ProcessEventsFor(SMART_EVENT_RESPAWN);
    Reset();
    mFollowGuid = ObjectGuid::Empty;//do not reset follower on Reset(), we need it after combat evade
    mFollowDist = 0;
    mFollowAngle = 0;
    mFollowCredit = 0;
    mFollowArrivedTimer = 1000;
    mFollowArrivedEntry = 0;
    mFollowCreditType = 0;
}

int SmartAI::Permissible(const Creature* creature)
{
    if (creature->GetAIName() == "SmartAI")
        return PERMIT_BASE_SPECIAL;
    return PERMIT_BASE_NO;
}

void SmartAI::JustReachedHome()
{
    GetScript()->ProcessEventsFor(SMART_EVENT_REACHED_HOME);
}

void SmartAI::OnQuestReward(Player* player, Quest const* quest)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_QUEST_REWARDED, player, quest->GetQuestId());
}

void SmartAI::OnStartQuest(Player* player, Quest const* quest)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_QUEST_ACCEPTED, player, quest->GetQuestId());
}

void SmartAI::EnterCombat(Unit* enemy)
{
    me->InterruptNonMeleeSpells(false); // must be before ProcessEvents
    GetScript()->ProcessEventsFor(SMART_EVENT_AGGRO, enemy);
    me->GetPosition(&mLastOOCPos);
    if (instance && _bossId)
    {
        // bosses do not respawn, check only on enter combat
        if (!instance->CheckRequiredBosses(_bossId, me->GetEntry()))
        {
            EnterEvadeMode();
            return;
        }
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_START, me);
        instance->SetBossState(_bossId, IN_PROGRESS);
    }
}

void SmartAI::JustDied(Unit* killer)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DEATH, killer);
    if (HasEscortState(SMART_ESCORT_ESCORTING))
        EndPath(true);
    if (instance && _bossId)
    {
        instance->SetBossState(_bossId, DONE);
        instance->SaveToDB();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_INSTANCE_END, me);
    }
}

void SmartAI::KilledUnit(Unit* victim)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_KILL, victim);
}

void SmartAI::JustSummoned(Creature* creature)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SUMMONED_UNIT, creature);
}

void SmartAI::AttackStart(Unit* who)
{
    if (who && me->Attack(who, true))
    {
        SetRun(mRun);
        if (me->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_ACTIVE) == POINT_MOTION_TYPE)
            me->GetMotionMaster()->MovementExpired();

        if (mCanCombatMove)
            me->GetMotionMaster()->MoveChase(who);

        me->GetPosition(&mLastOOCPos);
    }
}

void SmartAI::SpellHit(Unit* unit, const SpellInfo* spellInfo)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SPELLHIT, unit, 0, 0, false, spellInfo);
}

void SmartAI::SpellHitTarget(Unit* target, const SpellInfo* spellInfo)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SPELLHIT_TARGET, target, 0, 0, false, spellInfo);
}

void SmartAI::DamageTaken(Unit* doneBy, uint32& damage, DamageEffectType dmgType)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DAMAGED, doneBy, damage);
    if ((damage >= me->GetHealth(doneBy) - mInvincibilityHpLevel) && (mInvincibilityHpLevel > 0))
    {
        damage = 0;
        me->SetHealth(mInvincibilityHpLevel);
    }
}

void SmartAI::HealReceived(Unit* doneBy, uint32& addhealth)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_RECEIVE_HEAL, doneBy, addhealth);
}

void SmartAI::ReceiveEmote(Player* player, uint32 textEmote)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_RECEIVE_EMOTE, player, textEmote);
}

void SmartAI::IsSummonedBy(Unit* summoner)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_JUST_SUMMONED, summoner);
}

void SmartAI::DamageDealt(Unit* doneTo, uint32& damage, DamageEffectType /*damagetype*/)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DAMAGED_TARGET, doneTo, damage);
}

void SmartAI::SummonedCreatureDespawn(Creature* unit)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SUMMON_DESPAWNED, unit);
}

void SmartAI::UpdateAIWhileCharmed(const uint32 /*diff*/)
{
}

void SmartAI::CorpseRemoved(uint32& respawnDelay)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_CORPSE_REMOVED, nullptr, respawnDelay);
}

void SmartAI::PassengerBoarded(Unit* who, int8 seatId, bool apply)
{
    GetScript()->ProcessEventsFor(apply ? SMART_EVENT_PASSENGER_BOARDED : SMART_EVENT_PASSENGER_REMOVED, who, static_cast<uint32>(seatId), 0, apply);
}

void SmartAI::InitializeAI()
{
    GetScript()->OnInitialize(me);
    if (!me->isDead())
        Reset();
    GetScript()->ProcessEventsFor(SMART_EVENT_RESPAWN);
}

void SmartAI::OnCharmed(bool apply)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_CHARMED, nullptr, 0, 0, apply);
}

void SmartAI::DoAction(const int32 param)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ACTION_DONE, nullptr, param);
}

uint32 SmartAI::GetData(uint32 /*id*/) const
{
    return 0;
}

void SmartAI::SetData(uint32 id, uint32 value)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DATA_SET, nullptr, id, value);
}

void SmartAI::SetGUID(ObjectGuid const& /*guid*/, int32 /*id*/)
{
}

ObjectGuid SmartAI::GetGUID(int32 /*id*/)
{
    return ObjectGuid::Empty;
}

void SmartAI::SetRun(bool run)
{
    if (run)
        me->SetWalk(false);
    else
        me->SetWalk(true);

    mRun = run;
}

void SmartAI::SetFly(bool fly)
{
    me->SetCanFly(fly);
    me->SetDisableGravity(fly);

    if (fly)
        me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    else
        me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
}

void SmartAI::SetSwim(bool swim)
{
    if (swim)
        me->AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    else
        me->RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
}

void SmartAI::SetEvadeDisabled(bool disable)
{
    mEvadeDisabled = disable;
}

void SmartAI::SetInvincibilityHpLevel(uint32 level)
{
    mInvincibilityHpLevel = level;
}

void SmartAI::sGossipHello(Player* player)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_HELLO, player);
}

void SmartAI::sGossipSelect(Player* player, uint32 sender, uint32 action)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_SELECT, player, sender, action);
}

void SmartAI::sGossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/)
{
}

void SmartAI::sQuestAccept(Player* player, Quest const* quest)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ACCEPTED_QUEST, player, quest->GetQuestId());
}

void SmartAI::sQuestReward(Player* player, Quest const* quest, uint32 opt)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_REWARD_QUEST, player, quest->GetQuestId(), opt);
}

bool SmartAI::sOnDummyEffect(Unit* caster, uint32 spellId, SpellEffIndex effIndex)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DUMMY_EFFECT, caster, spellId, static_cast<uint32>(effIndex));
    return true;
}

void SmartAI::sOnActivateTaxiPathTo(Player* player, uint32 patchid)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ON_TAXIPATHTO, player, patchid);
}

void SmartAI::SetCombatMove(bool on)
{
    if (mCanCombatMove == on)
        return;
    mCanCombatMove = on;
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        if (on && me->getVictim())
        {
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
            {
                SetRun(mRun);
                me->GetMotionMaster()->MoveChase(me->getVictim());
                me->CastStop();
            }
        }
        else
        {
            me->StopMoving();
            me->GetMotionMaster()->MoveIdle();
        }
    }
}

bool SmartAI::CanCombatMove()
{
    return mCanCombatMove;
}

void SmartAI::SetFollow(Unit* target, float dist, float angle, uint32 credit, uint32 end, uint32 creditType)
{
    if (!target)
        return;
    SetRun(mRun);
    mFollowGuid = target->GetGUID();
    mFollowDist = dist >= 0.0f ? dist : PET_FOLLOW_DIST;
    mFollowAngle = angle >= 0.0f ? angle : me->GetFollowAngle();
    mFollowArrivedTimer = 1000;
    mFollowCredit = credit;
    mFollowArrivedEntry = end;
    me->GetMotionMaster()->MoveFollow(target, mFollowDist, mFollowAngle);
    mFollowCreditType = creditType;
}

void SmartAI::SetScript9(SmartScriptHolder& e, uint32 entry, Unit* invoker)
{
    if (!this) // https://pastebin.com/14JwKqQM
        return;

    if (invoker)
        GetScript()->mLastInvoker = invoker->GetGUID();
    GetScript()->SetScript9(e, entry);
}

SmartScript* SmartAI::GetScript()
{
    return &mScript;
}

void SmartAI::SetDespawnTime(uint32 t)
{
    mDespawnTime = t;
    mDespawnState = t ? 1 : 0;
}

void SmartAI::StartDespawn()
{
    mDespawnState = 2;
}

void SmartAI::OnSpellClick(Unit* clicker)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ON_SPELLCLICK, clicker);
}

void SmartAI::OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ON_APPLY_OR_REMOVE_AURA, nullptr, spellId, mode, apply);
}

int SmartGameObjectAI::Permissible(const GameObject* g)
{
    if (g->GetAIName() == "SmartGameObjectAI")
        return PERMIT_BASE_SPECIAL;
    return PERMIT_BASE_NO;
}

SmartGameObjectAI::SmartGameObjectAI(GameObject* g) : GameObjectAI(g), go(g)
{
}

SmartGameObjectAI::~SmartGameObjectAI()
{
}

void SmartGameObjectAI::UpdateAI(uint32 diff)
{
    GetScript()->OnUpdate(diff);
}

void SmartGameObjectAI::InitializeAI()
{
    GetScript()->OnInitialize(go);
    GetScript()->ProcessEventsFor(SMART_EVENT_RESPAWN);
    //Reset();
}

void SmartGameObjectAI::Reset()
{
    GetScript()->OnReset();
}

SmartScript* SmartGameObjectAI::GetScript()
{
    return &mScript;
}

// Called when a player opens a gossip dialog with the gameobject.
bool SmartGameObjectAI::GossipHello(Player* player)
{
    TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "SmartGameObjectAI::GossipHello");
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_HELLO, player, 0, 0, false, nullptr, go);
    return false;
}

// Called when a player selects a gossip item in the gameobject's gossip menu.
bool SmartGameObjectAI::GossipSelect(Player* player, uint32 sender, uint32 action)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_SELECT, player, sender, action, false, nullptr, go);
    return false;
}

// Called when a player selects a gossip with a code in the gameobject's gossip menu.
bool SmartGameObjectAI::GossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/)
{
    return false;
}

// Called when a player accepts a quest from the gameobject.
bool SmartGameObjectAI::QuestAccept(Player* player, Quest const* quest)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ACCEPTED_QUEST, player, quest->GetQuestId(), 0, false, nullptr, go);
    return false;
}

// Called when a player selects a quest reward.
bool SmartGameObjectAI::QuestReward(Player* player, Quest const* quest, uint32 opt)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_REWARD_QUEST, player, quest->GetQuestId(), opt, false, nullptr, go);
    return false;
}

// Called when the dialog status between a player and the gameobject is requested.
uint32 SmartGameObjectAI::GetDialogStatus(Player* /*player*/) { return 100; }

// Called when the gameobject is destroyed (destructible buildings only).
void SmartGameObjectAI::Destroyed(Player* player, uint32 eventId)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DEATH, player, eventId, 0, false, nullptr, go);
}

void SmartGameObjectAI::SetData(uint32 id, uint32 value)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DATA_SET, nullptr, id, value);
}

void SmartGameObjectAI::SetScript9(SmartScriptHolder& e, uint32 entry, Unit* invoker)
{
    if (invoker)
        GetScript()->mLastInvoker = invoker->GetGUID();
    GetScript()->SetScript9(e, entry);
}

void SmartGameObjectAI::OnStateChanged(uint32 state, Unit* unit)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GO_STATE_CHANGED, unit, state);
}

void SmartGameObjectAI::EventInform(uint32 eventId)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GO_EVENT_INFORM, nullptr, eventId);
}

class SmartTrigger : public AreaTriggerScript
{
public:

    SmartTrigger() : AreaTriggerScript("SmartTrigger") {}

    bool OnTrigger(Player* player, AreaTriggerEntry const* trigger, bool apply) override
    {
        if (!player->isAlive())
            return false;

        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "AreaTrigger %u is using SmartTrigger script", trigger->ID);
        SmartScript script;
        script.OnInitialize(nullptr, trigger);
        script.ProcessEventsFor(SMART_EVENT_AREATRIGGER_ONTRIGGER, player, trigger->ID);
        return true;
    }
};

class SmartEventObject : public EventObjectScript
{
public:
    SmartEventObject() : EventObjectScript("SmartEventObject") {}

    bool OnTrigger(Player* player, EventObject* event, bool apply) override
    {
        if (!player->isAlive())
            return false;

        TC_LOG_DEBUG(LOG_FILTER_DATABASE_AI, "EventObject %u is using SmartEventObject script", event->GetEntry());
        SmartScript script;
        script.OnInitialize(event, nullptr);
        if (apply)
            script.ProcessEventsFor(SMART_EVENT_EVENTOBJECT_ONTRIGGER, player, event->GetEntry());
        else
            script.ProcessEventsFor(SMART_EVENT_EVENTOBJECT_OFFTRIGGER, player, event->GetEntry());
        return true;
    }
};

void AddSC_SmartSCripts()
{
    new SmartTrigger();
    new SmartEventObject();
}
