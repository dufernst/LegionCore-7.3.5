
#include "ArenaAshamanesFall.h"

enum BattlegroundVshObjectTypes
{
    BG_VSH_OBJECT_DOOR_1,
    BG_VSH_OBJECT_DOOR_2,
    BG_VSH_OBJECT_BUFF_3,
    BG_VSH_OBJECT_BUFF_4,

    BG_VSH_OBJECT_MAX
};

enum BattlegroundVshObjects
{
    BG_VSH_OBJECT_TYPE_DOOR_1 = 250430,
    BG_VSH_OBJECT_TYPE_DOOR_2 = 250431,
    BG_VSH_OBJECT_TYPE_BUFF_1 = 184663,
    BG_VSH_OBJECT_TYPE_BUFF_2 = 184664
};

const float BgVshDoorPositions[2][8] =
{
    { 3548.342f, 5584.779f, 323.6123f, 1.544616f, 0.0f, 0.0f, 0.6977901f, 0.7163023f },
    { 3539.870f, 5488.701f, 323.5819f, 1.553341f, 0.0f, 0.0f, 0.7009087f, 0.7132511f }
};

ArenaAshamanesFall::ArenaAshamanesFall()
{
    BgObjects.resize(BG_VSH_OBJECT_MAX);
}

ArenaAshamanesFall::~ArenaAshamanesFall() = default;

void ArenaAshamanesFall::StartingEventCloseDoors()
{
    for (uint32 i = BG_VSH_OBJECT_DOOR_1; i <= BG_VSH_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    Arena::StartingEventCloseDoors();
}

void ArenaAshamanesFall::StartingEventOpenDoors()
{
    for (uint32 i = BG_VSH_OBJECT_DOOR_1; i <= BG_VSH_OBJECT_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = BG_VSH_OBJECT_BUFF_3; i <= BG_VSH_OBJECT_BUFF_4; ++i)
        SpawnBGObject(i, MINUTE);

    Arena::StartingEventOpenDoors();
}

bool ArenaAshamanesFall::SetupBattleground()
{
    if (!AddObject(BG_VSH_OBJECT_DOOR_1, BG_VSH_OBJECT_TYPE_DOOR_1, BgVshDoorPositions[0][0], BgVshDoorPositions[0][1], BgVshDoorPositions[0][2], BgVshDoorPositions[0][3], BgVshDoorPositions[0][4], BgVshDoorPositions[0][5], BgVshDoorPositions[0][6], BgVshDoorPositions[0][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_VSH_OBJECT_DOOR_2, BG_VSH_OBJECT_TYPE_DOOR_2, BgVshDoorPositions[1][0], BgVshDoorPositions[1][1], BgVshDoorPositions[1][2], BgVshDoorPositions[1][3], BgVshDoorPositions[1][4], BgVshDoorPositions[1][5], BgVshDoorPositions[1][6], BgVshDoorPositions[1][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_VSH_OBJECT_BUFF_3, BG_VSH_OBJECT_TYPE_BUFF_1, 3579.075f, 5575.938f, 326.8913f, 2.460913f, 0.0f, 0.0f, 0.9426413f, 0.3338076f, BATTLEGROUND_COUNTDOWN_MAX) ||
        !AddObject(BG_VSH_OBJECT_BUFF_4, BG_VSH_OBJECT_TYPE_BUFF_2, 3579.075f, 5575.938f, 326.8913f, 2.460913f, 0.0f, 0.0f, 0.9426413f, 0.3338076f, BATTLEGROUND_COUNTDOWN_MAX))
        return false;

    return true;
}
