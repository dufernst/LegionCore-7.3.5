#include "ArenaNagrand.h"
#include "Battleground.h"

enum ArenaNagrandArenaObjectTypes
{
    BG_NA_OBJECT_DOOR_1     = 0,
    BG_NA_OBJECT_DOOR_2,

    BG_NA_OBJECT_BUFF_1,
    BG_NA_OBJECT_BUFF_2,

    BG_NA_OBJECT_MAX
};

enum ArenaNagrandArenaObjects
{
    BG_NA_OBJECT_TYPE_DOOR_1 = 260527,
    BG_NA_OBJECT_TYPE_DOOR_2 = 260528,

    BG_NA_OBJECT_TYPE_BUFF_1 = 184663,
    BG_NA_OBJECT_TYPE_BUFF_2 = 184664
};


ArenaNagrandArena::ArenaNagrandArena()
{
    BgObjects.resize(BG_NA_OBJECT_MAX);
}

ArenaNagrandArena::~ArenaNagrandArena() = default;

void ArenaNagrandArena::StartingEventCloseDoors()
{
    for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    Arena::StartingEventCloseDoors();
}

void ArenaNagrandArena::StartingEventOpenDoors()
{
    for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = BG_NA_OBJECT_BUFF_1; i <= BG_NA_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, 60);

    Arena::StartingEventOpenDoors();
}

bool ArenaNagrandArena::SetupBattleground()
{
    if (!AddObject(BG_NA_OBJECT_DOOR_1, BG_NA_OBJECT_TYPE_DOOR_1, -2067.722f, 6699.386f, 11.9068f, 2.063599f, 0, 0, 0.8582239f, 0.5132754f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_NA_OBJECT_DOOR_2, BG_NA_OBJECT_TYPE_DOOR_2, -2019.238f, 6609.098f, 11.9068f, 5.205194f, 0, 0, -0.5132742f, 0.8582247f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_NA_OBJECT_BUFF_1, BG_NA_OBJECT_TYPE_BUFF_1, -2090.78f, 6629.46f, 12.84f, 0.53f, 0, 0, 0.6626201f, -0.7489557f, 120) ||
        !AddObject(BG_NA_OBJECT_BUFF_2, BG_NA_OBJECT_TYPE_BUFF_2, -1995.612f, 6679.230f, 13.068f, 3.522f, 0, 0, 0.03489945f, -0.9993908f, 120))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundNA: Failed to spawn some object!");
        return false;
    }

    return true;
}
