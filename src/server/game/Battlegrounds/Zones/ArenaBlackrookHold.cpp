
#include "Battleground.h"
#include "ArenaBlackrookHold.h"

enum BattlegroundBrhObjectTypes
{
    BG_BRH_OBJECT_DOOR_1 = 0,
    BG_BRH_OBJECT_DOOR_2,
    BG_BRH_OBJECT_DOOR_3,
    BG_BRH_OBJECT_DOOR_4,
    BG_BRH_OBJECT_DOOR_5,
    BG_BRH_OBJECT_DOOR_6,
    BG_BRH_OBJECT_DOOR_7,
    BG_BRH_OBJECT_DOOR_8,
    BG_BRH_OBJECT_DOOR_9,
    BG_BRH_OBJECT_BUFF_1,
    BG_BRH_OBJECT_BUFF_2,
    BG_BRH_OBJECT_ANVIL,

    BG_BRH_OBJECT_MAX
};

enum BattlegroundBrhObjects
{
    BG_BRH_OBJECT_TYPE_DOOR_1 = 245763,
    BG_BRH_OBJECT_TYPE_DOOR_2 = 245766,
    BG_BRH_OBJECT_TYPE_DOOR_3 = 245767,
    BG_BRH_OBJECT_TYPE_DOOR_4 = 245768,
    BG_BRH_OBJECT_TYPE_DOOR_5 = 245770,
    BG_BRH_OBJECT_TYPE_DOOR_6 = 245781,
    BG_BRH_OBJECT_TYPE_DOOR_7 = 245762,
    BG_BRH_OBJECT_TYPE_DOOR_8 = 245765,
    BG_BRH_OBJECT_TYPE_DOOR_9 = 245769,
    BG_BRH_OBJECT_TYPE_BUFF_1 = 184663,
    BG_BRH_OBJECT_TYPE_BUFF_2 = 184664,
    BG_BRH_OBJECT_TYPE_ANVIL = 249825, //Wtf?
};

const float BgBrhGoPositions[12][8] =
{
    { 1464.32f, 1254.87f, 33.24f, 0.84f, 0.0f, 0.0f,  0.40f,  0.91f },
    { 1384.06f, 1258.20f, 33.21f, 3.46f, 0.0f, 0.0f, -0.98f,  0.16f },
    { 1384.61f, 1232.35f, 33.27f, 2.86f, 0.0f, 0.0f,  0.99f,  0.13f },
    { 1411.41f, 1206.09f, 33.22f, 5.03f, 0.0f, 0.0f, -0.58f,  0.81f },
    { 1437.22f, 1206.74f, 33.23f, 4.43f, 0.0f, 0.0f, -0.79f,  0.60f },
    { 1450.72f, 1276.73f, 33.24f, 0.24f, 0.0f, 0.0f,  0.12f,  0.99f },
    { 1455.90f, 1264.99f, 33.27f, 0.54f, 0.0f, 0.0f,  0.26f,  0.96f },
    { 1385.84f, 1245.18f, 33.27f, 3.16f, 0.0f, 0.0f, -0.99f,  0.01f },
    { 1424.42f, 1208.04f, 33.27f, 4.73f, 0.0f, 0.0f, -0.69f,  0.71f },
    { 1185.11f, 1404.49f, 65.18f, 1.57f, 0.0f, 0.0f,  0.71f,  0.70f },
    { 1423.24f, 1287.08f, 32.84f, 4.75f, 0.0f, 0.0f,  0.66f, -0.74f },
    { 1461.35f, 1225.18f,  33.0f, 2.59f, 0.0f, 0.0f,  0.03f, -0.99f }
};


ArenaBlackrookHold::ArenaBlackrookHold()
{
    BgObjects.resize(BG_BRH_OBJECT_MAX);
}

ArenaBlackrookHold::~ArenaBlackrookHold() = default;

void ArenaBlackrookHold::StartingEventCloseDoors()
{
    for (uint32 i = BG_BRH_OBJECT_DOOR_1; i < BG_BRH_OBJECT_MAX; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    Arena::StartingEventCloseDoors();
}

void ArenaBlackrookHold::StartingEventOpenDoors()
{
    for (uint32 i = BG_BRH_OBJECT_DOOR_1; i < BG_BRH_OBJECT_DOOR_7; ++i)
        DoorOpen(i);

    for (uint32 i = BG_BRH_OBJECT_BUFF_1; i <= BG_BRH_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, ARENA_COUNTDOWN_MAX);

    Arena::StartingEventOpenDoors();
}

bool ArenaBlackrookHold::SetupBattleground()
{
    if (!AddObject(BG_BRH_OBJECT_DOOR_1, BG_BRH_OBJECT_TYPE_DOOR_1, BgBrhGoPositions[0][0], BgBrhGoPositions[0][1], BgBrhGoPositions[0][2], BgBrhGoPositions[0][3], BgBrhGoPositions[0][4], BgBrhGoPositions[0][5], BgBrhGoPositions[0][6], BgBrhGoPositions[0][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_2, BG_BRH_OBJECT_TYPE_DOOR_2, BgBrhGoPositions[1][0], BgBrhGoPositions[1][1], BgBrhGoPositions[1][2], BgBrhGoPositions[1][3], BgBrhGoPositions[1][4], BgBrhGoPositions[1][5], BgBrhGoPositions[1][6], BgBrhGoPositions[1][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_3, BG_BRH_OBJECT_TYPE_DOOR_3, BgBrhGoPositions[2][0], BgBrhGoPositions[2][1], BgBrhGoPositions[2][2], BgBrhGoPositions[2][3], BgBrhGoPositions[2][4], BgBrhGoPositions[2][5], BgBrhGoPositions[2][6], BgBrhGoPositions[2][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_4, BG_BRH_OBJECT_TYPE_DOOR_4, BgBrhGoPositions[3][0], BgBrhGoPositions[3][1], BgBrhGoPositions[3][2], BgBrhGoPositions[3][3], BgBrhGoPositions[3][4], BgBrhGoPositions[3][5], BgBrhGoPositions[3][6], BgBrhGoPositions[3][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_5, BG_BRH_OBJECT_TYPE_DOOR_5, BgBrhGoPositions[4][0], BgBrhGoPositions[4][1], BgBrhGoPositions[4][2], BgBrhGoPositions[4][3], BgBrhGoPositions[4][4], BgBrhGoPositions[4][5], BgBrhGoPositions[4][6], BgBrhGoPositions[4][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_6, BG_BRH_OBJECT_TYPE_DOOR_6, BgBrhGoPositions[5][0], BgBrhGoPositions[5][1], BgBrhGoPositions[5][2], BgBrhGoPositions[5][3], BgBrhGoPositions[5][4], BgBrhGoPositions[5][5], BgBrhGoPositions[5][6], BgBrhGoPositions[5][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_7, BG_BRH_OBJECT_TYPE_DOOR_7, BgBrhGoPositions[6][0], BgBrhGoPositions[6][1], BgBrhGoPositions[6][2], BgBrhGoPositions[6][3], BgBrhGoPositions[6][4], BgBrhGoPositions[6][5], BgBrhGoPositions[6][6], BgBrhGoPositions[6][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_8, BG_BRH_OBJECT_TYPE_DOOR_8, BgBrhGoPositions[7][0], BgBrhGoPositions[7][1], BgBrhGoPositions[7][2], BgBrhGoPositions[7][3], BgBrhGoPositions[7][4], BgBrhGoPositions[7][5], BgBrhGoPositions[7][6], BgBrhGoPositions[7][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_DOOR_9, BG_BRH_OBJECT_TYPE_DOOR_9, BgBrhGoPositions[8][0], BgBrhGoPositions[8][1], BgBrhGoPositions[8][2], BgBrhGoPositions[8][3], BgBrhGoPositions[8][4], BgBrhGoPositions[8][5], BgBrhGoPositions[8][6], BgBrhGoPositions[8][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_ANVIL, BG_BRH_OBJECT_TYPE_ANVIL, BgBrhGoPositions[9][0], BgBrhGoPositions[9][1], BgBrhGoPositions[9][2], BgBrhGoPositions[9][3], BgBrhGoPositions[9][4], BgBrhGoPositions[9][5], BgBrhGoPositions[9][6], BgBrhGoPositions[9][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_BRH_OBJECT_BUFF_1, BG_BRH_OBJECT_TYPE_BUFF_1, BgBrhGoPositions[10][0], BgBrhGoPositions[10][1], BgBrhGoPositions[10][2], BgBrhGoPositions[10][3], BgBrhGoPositions[10][4], BgBrhGoPositions[10][5], BgBrhGoPositions[10][6], BgBrhGoPositions[10][7], BUFF_RESPAWN_TIME) ||
        !AddObject(BG_BRH_OBJECT_BUFF_2, BG_BRH_OBJECT_TYPE_BUFF_2, BgBrhGoPositions[11][0], BgBrhGoPositions[11][1], BgBrhGoPositions[11][2], BgBrhGoPositions[11][3], BgBrhGoPositions[11][4], BgBrhGoPositions[11][5], BgBrhGoPositions[11][6], BgBrhGoPositions[11][7], BUFF_RESPAWN_TIME))
        return false;

    return true;
}
