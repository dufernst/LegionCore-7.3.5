/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

#define DEFAULT_MAX_PLAYER_DISTANCE 50

struct Escort_Waypoint
{
    Escort_Waypoint(uint32 _id, float _x, float _y, float _z, uint32 _w);

    uint32 id;
    float x;
    float y;
    float z;
    uint32 WaitTimeMs;
};

enum eEscortState
{
    STATE_ESCORT_NONE       = 0x000,                        //nothing in progress
    STATE_ESCORT_ESCORTING  = 0x001,                        //escort are in progress
    STATE_ESCORT_RETURNING  = 0x002,                        //escort is returning after being in combat
    STATE_ESCORT_PAUSED     = 0x004                         //will not proceed with waypoints before state is removed
};

struct npc_escortAI : public ScriptedAI
{
        explicit npc_escortAI(Creature* creature);
        ~npc_escortAI() {}

        // CreatureAI functions
        void AttackStart(Unit* who);

        void MoveInLineOfSight(Unit* who);

        void JustDied(Unit*);

        void JustRespawned();

        void ReturnToLastPoint();

        void MovePoint(uint32 point, float x, float y, float z);

        void EnterEvadeMode();

        void UpdateAI(uint32 diff);                   //the "internal" update, calls UpdateEscortAI()
        virtual void UpdateEscortAI(uint32 const diff);     //used when it's needed to add code in update (abilities, scripted events, etc)

        void MovementInform(uint32, uint32);

        // EscortAI functions
        void AddWaypoint(uint32 id, float x, float y, float z, uint32 waitTime = 0);    // waitTime is in ms

        //this will set the current position to x/y/z/o, and the current WP to pointId.
        bool SetNextWaypoint(uint32 pointId, float x, float y, float z, float orientation);

        //this will set the current position to WP start position (if setPosition == true),
        //and the current WP to pointId
        bool SetNextWaypoint(uint32 pointId, bool setPosition = true, bool resetWaypointsOnFail = true);

        bool GetWaypointPosition(uint32 pointId, float& x, float& y, float& z);

        virtual void WaypointReached(uint32 pointId) = 0;
        virtual void LastWaypointReached() {}
        virtual void WaypointStart(uint32 /*pointId*/) {}

        void Start(bool isActiveAttacker = true, bool run = false, ObjectGuid playerGUID = ObjectGuid::Empty, Quest const* quest = nullptr, bool instantRespawn = false, bool canLoopPath = false, bool resetWaypoints = true);

        void SetRun(bool on = true);
        void SetEscortPaused(bool on);

        bool HasEscortState(uint32 escortState) { return (m_uiEscortState & escortState) != 0; }
        virtual bool IsEscorted() { return (m_uiEscortState & STATE_ESCORT_ESCORTING); }

        void SetMaxPlayerDistance(float newMax) { MaxPlayerDistance = newMax; }
        float GetMaxPlayerDistance() { return MaxPlayerDistance; }

        void SetDespawnAtEnd(bool despawn) { DespawnAtEnd = despawn; }
        void SetDespawnAtFar(bool despawn) { DespawnAtFar = despawn; }
        void SetWPStarTimer(float SetWPStarTimer) { m_uiWPWaitTimer = SetWPStarTimer; }
        bool GetAttack() { return m_bIsActiveAttacker; }//used in EnterEvadeMode override
        void SetCanAttack(bool attack) { m_bIsActiveAttacker = attack; }
        ObjectGuid GetEventStarterGUID() { return m_uiPlayerGUID; }
        void SetCurentWP(uint32 id);
        uint32 GetCurentWP() { return CurrentWP->id; }
        void SetGeneratePath(bool path) { GeneratePath = path; }

        void SetFollowerGUID(ObjectGuid guid) { m_uifollowerGUID = guid; } // add follower guid
    protected:
    Player* GetPlayerForEscort();

    private:
        bool AssistPlayerInCombat(Unit* who);
        bool IsPlayerOrGroupInRange();
        void FillPointMovementListForCreature();

        void AddEscortState(uint32 escortState) { m_uiEscortState |= escortState; }
        void RemoveEscortState(uint32 escortState) { m_uiEscortState &= ~escortState; }

        ObjectGuid m_uifollowerGUID;
        ObjectGuid m_uiPlayerGUID;
        uint32 m_uiWPWaitTimer;
        uint32 m_uiPlayerCheckTimer;
        uint32 m_uiEscortState;
        float MaxPlayerDistance;

        Quest const* m_pQuestForEscort;                     //generally passed in Start() when regular escort script.

        std::list<Escort_Waypoint> WaypointList;
        std::list<Escort_Waypoint>::iterator CurrentWP;

        bool m_bIsActiveAttacker;                           //obsolete, determined by faction.
        bool m_bIsRunning;                                  //all creatures are walking by default (has flag MOVEMENTFLAG_WALK)
        bool m_bCanInstantRespawn;                          //if creature should respawn instantly after escort over (if not, database respawntime are used)
        bool m_bCanReturnToStart;                           //if creature can walk same path (loop) without despawn. Not for regular escort quests.
        bool DespawnAtEnd;
        bool DespawnAtFar;
        bool ScriptWP;
        bool HasImmuneToNPCFlags;
        bool GeneratePath;
};
#endif
