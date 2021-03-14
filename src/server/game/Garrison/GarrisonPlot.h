
#ifndef GarrisonPlot_h_
#define GarrisonPlot_h_

#include "GarrisonGlobal.h"
#include "Packets/GarrisonPackets.h"

struct Building
{
    bool CanActivate() const;

    ObjectGuid Guid;
    ObjectGuid FinalizerGuid;
    GuidUnorderedSet Spawns;
    Optional<WorldPackets::Garrison::GarrisonBuildingInfo> PacketInfo;
};

struct Plot
{
    void getRandTrader(uint32& entry);
    GameObject* CreateGameObject(Map* map, GarrisonFactionIndex faction, Garrison* g);
    void DeleteGameObject(Map* map);
    void ClearBuildingInfo(Player* owner);
    void SetBuildingInfo(WorldPackets::Garrison::GarrisonBuildingInfo const& buildingInfo, Player* owner);

    WorldPackets::Garrison::GarrisonPlotInfo PacketInfo;
    float RotationX;                                                // 6
    float RotationY;                                                // 7
    float RotationZ;                                                // 8
    float RotationW;                                                // 9
    uint32 EmptyGameObjectId = 0;
    uint32 GarrSiteLevelPlotInstId = 0;
    Building BuildingInfo;
    ObjectDBState db_state_building = DB_STATE_NEW;
};

#endif // GarrisonPlot_h_
