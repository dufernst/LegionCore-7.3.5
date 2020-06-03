#pragma once

#include "Common.h"
#include "Map.h"

class Battleground;
enum Difficulty : uint8;

class BattlegroundMap : public Map
{
    Battleground* m_bg;
public:
    BattlegroundMap(uint32 id, time_t, uint32 instanceId, Map* parent, Difficulty difficulty);
    ~BattlegroundMap() override;

    bool AddPlayerToMap(Player*, bool initPlayer = true) override;
    void RemovePlayerFromMap(Player*, bool) override;
    bool CanEnter(Player* player) override;
    void SetUnload();
    void RemoveAllPlayers() override;
    void InitVisibilityDistance() override;
    Battleground* GetBG();
    void SetBG(Battleground* bg);
};
