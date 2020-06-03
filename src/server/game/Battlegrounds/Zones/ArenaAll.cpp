
#include "Battleground.h"
#include "Player.h"
#include "ArenaAll.h"

ArenaAll::ArenaAll() = default;

ArenaAll::~ArenaAll() = default;

void ArenaAll::StartingEventCloseDoors()
{ }

void ArenaAll::StartingEventOpenDoors()
{ }

void ArenaAll::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
}

void ArenaAll::RemovePlayer(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*team*/)
{ }

void ArenaAll::HandleKillPlayer(Player* player, Player* killer)
{
    Battleground::HandleKillPlayer(player, killer);
}

bool ArenaAll::SetupBattleground()
{
    return true;
}
