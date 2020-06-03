
#include "ArenaTolvir.h"
#include "Battleground.h"

enum BattlegroundTVObjectTypes
{
    BG_TV_OBJECT_DOOR_1 = 0,
    BG_TV_OBJECT_DOOR_2 = 1,
    BG_TV_OBJECT_BUFF_1 = 2,
    BG_TV_OBJECT_BUFF_2 = 3,

    BG_TV_OBJECT_MAX
};

enum BattlegroundTVObjects
{
    BG_TV_OBJECT_TYPE_DOOR_1 = 213196,
    BG_TV_OBJECT_TYPE_DOOR_2 = 213197,
    BG_TV_OBJECT_TYPE_BUFF_1 = 184663,
    BG_TV_OBJECT_TYPE_BUFF_2 = 184664
};


ArenaTolvir::ArenaTolvir()
{
    BgObjects.resize(BG_TV_OBJECT_MAX);
}

ArenaTolvir::~ArenaTolvir() = default;

void ArenaTolvir::StartingEventCloseDoors()
{
    for (uint32 i = BG_TV_OBJECT_DOOR_1; i <= BG_TV_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    Arena::StartingEventCloseDoors();
}

void ArenaTolvir::StartingEventOpenDoors()
{
    for (uint32 i = BG_TV_OBJECT_DOOR_1; i <= BG_TV_OBJECT_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = BG_TV_OBJECT_BUFF_1; i <= BG_TV_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, MINUTE);

    Arena::StartingEventOpenDoors();
}

bool ArenaTolvir::SetupBattleground()
{
    if (!AddObject(BG_TV_OBJECT_DOOR_1, BG_TV_OBJECT_TYPE_DOOR_1, -10654.3f, 428.3047f, 23.54276f, 3.141593f, 0, 0, -1.f, 0, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_TV_OBJECT_DOOR_2, BG_TV_OBJECT_TYPE_DOOR_2, -10774.61f, 431.2383f, 23.54276f, 0, 0, 0, 0, 1.f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_TV_OBJECT_BUFF_1, BG_TV_OBJECT_TYPE_BUFF_1, -10717.63f, 383.8223f, 24.412825f, 1.555f, 0.0f, 0.0f, 0.70154f, BATTLEGROUND_COUNTDOWN_MAX) ||
        !AddObject(BG_TV_OBJECT_BUFF_2, BG_TV_OBJECT_TYPE_BUFF_2, -10716.6f, 475.364f, 24.4131f, 0.0f, 0.0f, 0.70068f, -0.713476f, BATTLEGROUND_COUNTDOWN_MAX))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "ArenaTolvir: Failed to spawn some object!");
        return false;
    }

    return true;
}
