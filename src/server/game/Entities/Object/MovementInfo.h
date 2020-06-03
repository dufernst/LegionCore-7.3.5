
#ifndef MovementInfo_h__
#define MovementInfo_h__

#include "ObjectGuid.h"
#include "Position.h"

namespace  WorldPackets
{
    namespace  Movement
    {
        struct MovementForce;
    }
}

struct MovementInfo
{
    struct TransportInfo
    {
        void Reset()
        {
            Guid.Clear();
            Pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
            VehicleSeatIndex = -1;
            MoveTime = 0;
            PrevMoveTime = 0;
            VehicleRecID = 0;
        }

        ObjectGuid Guid;
        Position Pos;
        uint32 MoveTime;
        uint32 PrevMoveTime; //< Optional
        uint32 VehicleRecID; //< Optional
        int8 VehicleSeatIndex;
    } transport;

    struct FallInfo
    {
        void Reset()
        {
            hasFallDirection = false;
            Direction.Pos.Relocate(0.0f, 0.0f);
            HorizontalSpeed = 0.0f;
            JumpVelocity = 0.0f;
            fallTime = 0;
            lastTimeUpdate = 0;
            startClientTime = 0;
        }

        void SetFallTime(uint32 time) { fallTime = time; lastTimeUpdate = 0; }

        TaggedPosition<Position::XY> Direction;
        float HorizontalSpeed;
        float JumpVelocity;
        uint32 fallTime;
        uint32 lastTimeUpdate;
        bool hasFallDirection;
        Position start;
        uint32 startClientTime;
    } fall;

    GuidVector RemoveForcesIDs;
    std::map<ObjectGuid, WorldPackets::Movement::MovementForce> Forces;
    ObjectGuid Guid;
    Position Pos;
    uint32 MoveFlags[2];
    uint32 ClientMoveTime;
    uint32 MoveTime;
    uint32 MoveIndex;
    float splineElevation;
    float pitch;
    bool HeightChangeFailed;
    bool RemoteTimeValid;
    bool hasFallData;
    bool hasTransportData;
    bool hasSpline;

    MovementInfo()
    {
        Guid.Clear();
        Pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        MoveTime = getMSTime();
        memset(MoveFlags, 0, sizeof(MoveFlags));
        transport.Reset();
        fall.Reset();
        ClientMoveTime = MoveIndex = 0;
        splineElevation = 0.0f;
        pitch = 0.0f;
        HeightChangeFailed = false;
        RemoteTimeValid = false;
        hasFallData = false;
        hasTransportData = false;
        hasSpline = false;
    }

    uint32 GetMovementFlags() const { return MoveFlags[0]; }
    void SetMovementFlags(uint32 flag) { MoveFlags[0] = flag; }
    void AddMovementFlag(uint32 flag) { MoveFlags[0] |= flag; }
    void RemoveMovementFlag(uint32 flag) { MoveFlags[0] &= ~flag; }
    bool HasMovementFlag(uint32 flag) const { return (MoveFlags[0] & flag) != 0; }

    uint32 GetExtraMovementFlags() const { return MoveFlags[1]; }
    void SetExtraMovementFlags(uint32 flag) { MoveFlags[1] = flag; }
    void AddExtraMovementFlag(uint32 flag) { MoveFlags[1] |= flag; }
    void RemoveExtraMovementFlag(uint32 flag) { MoveFlags[1] &= ~flag; }
    bool HasExtraMovementFlag(uint32 flag) const { return (MoveFlags[1] & flag) != 0; }

    void ChangeOrientation(float o) { Pos.m_orientation = o; }
    void ChangePosition(float x, float y, float z, float o) { Pos.Relocate(x, y, z, o); }
    void UpdateTime(uint32 _time) { MoveTime = _time; }

    void ResetTransport()
    {
        transport.Reset();
    }

    void ResetFall()
    {
        fall.Reset();
    }
};

#endif // MovementInfo_h__
