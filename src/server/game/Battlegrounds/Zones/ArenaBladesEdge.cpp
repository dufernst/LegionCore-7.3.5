#include "Battleground.h"
#include "ArenaBladesEdge.h"

enum ArenaObjectTypes
{
    OBJECT_DOOR_1   = 0,
    OBJECT_DOOR_2,
    OBJECT_BUFF_1,
    OBJECT_BUFF_2,

    OBJECT_MAX
};

enum ArenaObjects
{
    OBJECT_TYPE_DOOR_1 = 265571,
    OBJECT_TYPE_DOOR_2 = 265569,
    OBJECT_TYPE_BUFF_1 = 184663,
    OBJECT_TYPE_BUFF_2 = 184664,
};

Position const goPositions[][2] =
{
    {{2776.371f, 6055.702f, -3.733995f, 2.217981f}, {0.0f, 0.0f, 0.8952494f, 0.4455655f}},
    {{2797.014f, 5953.527f, -4.099238f, 5.410522f}, {0.0f, 0.0f, -0.4226179f, 0.9063079f}},
    {{2815.306f, 5972.267f, -4.452f, 2.24f}, {0.0f, 0.0f, 0.40f, 0.91f}},
    {{2761.437f, 6038.106f, -3.49f, 5.57f}, {0.0f, 0.0f, 0.40f, 0.91f}},
};

ArenaBladesEdge::ArenaBladesEdge()
{
    BgObjects.resize(OBJECT_MAX);
}

ArenaBladesEdge::~ArenaBladesEdge() = default;

void ArenaBladesEdge::StartingEventCloseDoors()
{
    for (uint32 i = OBJECT_DOOR_1; i <= OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    Arena::StartingEventCloseDoors();
}

void ArenaBladesEdge::StartingEventOpenDoors()
{
    for (uint32 i = OBJECT_DOOR_1; i <= OBJECT_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = OBJECT_BUFF_1; i <= OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, ARENA_COUNTDOWN_MAX);

    Arena::StartingEventOpenDoors();
}

bool ArenaBladesEdge::SetupBattleground()
{
    if (!AddObject(OBJECT_DOOR_1, OBJECT_TYPE_DOOR_1, goPositions[0][0], goPositions[0][1], RESPAWN_IMMEDIATELY) ||
        !AddObject(OBJECT_DOOR_2, OBJECT_TYPE_DOOR_2, goPositions[1][0], goPositions[1][1], RESPAWN_IMMEDIATELY) ||
        !AddObject(OBJECT_BUFF_1, OBJECT_TYPE_BUFF_1, goPositions[2][0], goPositions[2][1], RESPAWN_IMMEDIATELY) ||
        !AddObject(OBJECT_BUFF_2, OBJECT_TYPE_BUFF_2, goPositions[3][0], goPositions[3][1], RESPAWN_IMMEDIATELY))
        return false;

    return true;
}
