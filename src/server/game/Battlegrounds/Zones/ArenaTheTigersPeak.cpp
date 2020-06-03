#include "ArenaTheTigersPeak.h"
#include "Battleground.h"


enum BattlegroundTTPObjectTypes
{
    BG_TTP_OBJECT_DOOR_1 = 0,
    BG_TTP_OBJECT_DOOR_2 = 1,
    BG_TTP_OBJECT_BUFF_1 = 2,
    BG_TTP_OBJECT_BUFF_2 = 3,
    BG_TTP_OBJECT_MAX = 4
};

enum BattlegroundTTPObjects
{
    BG_TTP_OBJECT_TYPE_DOOR_1 = 219395,
    BG_TTP_OBJECT_TYPE_DOOR_2 = 219396,
    BG_TTP_OBJECT_TYPE_BUFF_1 = 184663,
    BG_TTP_OBJECT_TYPE_BUFF_2 = 184664
};

ArenaTheTigersPeak::ArenaTheTigersPeak()
{
    BgObjects.resize(BG_TTP_OBJECT_MAX);
}

ArenaTheTigersPeak::~ArenaTheTigersPeak() = default;

void ArenaTheTigersPeak::StartingEventCloseDoors()
{
    for (uint32 i = BG_TTP_OBJECT_DOOR_1; i <= BG_TTP_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    Arena::StartingEventCloseDoors();
}

void ArenaTheTigersPeak::StartingEventOpenDoors()
{
    for (uint32 i = BG_TTP_OBJECT_DOOR_1; i <= BG_TTP_OBJECT_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = BG_TTP_OBJECT_BUFF_1; i <= BG_TTP_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, 60);

    Arena::StartingEventOpenDoors();
}

bool ArenaTheTigersPeak::SetupBattleground()
{
    if (!AddObject(BG_TTP_OBJECT_DOOR_1, BG_TTP_OBJECT_TYPE_DOOR_1, 501.932f, 633.429f, 380.708f, 0.0262353f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_TTP_OBJECT_DOOR_2, BG_TTP_OBJECT_TYPE_DOOR_2, 632.101f, 633.791f, 380.704f, 3.20989f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_TTP_OBJECT_BUFF_1, BG_TTP_OBJECT_TYPE_BUFF_1, 566.6805f, 602.2274f, 383.6813f, 3.316144f, 0, 0, -1.f, 0, 120) ||
        !AddObject(BG_TTP_OBJECT_BUFF_2, BG_TTP_OBJECT_TYPE_BUFF_2, 566.6563f, 664.566f, 383.6809f, 2.460913f, 0, 0, 0, 1.f, 120))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "ArenaTheTigersPeak: Failed to spawn some object!");
        return false;
    }

    return true;
}
