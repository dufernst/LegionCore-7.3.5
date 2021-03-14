
#include "GarrisonPlot.h"
#include "Garrison.h"
#include "ObjectMgr.h"
#include "GarrisonMgr.h"
#include <G3D/Quat.h>

#define RESP_GO_LOOT (12 * HOUR * MINUTE)

bool Building::CanActivate() const
{
    if (PacketInfo && PacketInfo->TimeBuilt + sGarrBuildingStore.AssertEntry(PacketInfo->GarrBuildingID)->BuildSeconds <= time(nullptr))
        return true;

    return false;
}

//! Only for GARR_BTYPE_TRADING_POST
void Plot::getRandTrader(uint32 & entry)
{
    const uint32 __data[2][5] =
    {
        { 87200, 87201, 87203, 87202, 87204 },
        { 86779, 86778, 86776, 86777, 86683 }
    };

    auto isHorde = false;

    switch (entry)
    {
        //Horde
    case 86779:
    case 86778:
    case 86776:
    case 86777:
    case 86683:
        isHorde = true;
        break;
        //Alliance
    case 87200:
    case 87201:
    case 87203:
    case 87202:
    case 87204:
        break;
    default:
        return;
    }

    entry = __data[isHorde][urand(0, 4)];

    for (ObjectGuid const& guid : BuildingInfo.Spawns)
        if (guid.GetEntry() == entry)
            getRandTrader(entry);
}

bool isSpecialSpawnEntry(uint32 entry)
{
    switch (entry)
    {
        //GARR_BTYPE_WORKSHOP lvl1
    case 233900:
    case 234146:
    case 235078:
    case 234095:
        //GARR_BTYPE_WORKSHOP lvl2
    case 233901:
    case 234017:
    case 233899:
    case 235126:
    case 234018:
    case 234019:
        return true;
    }

    return false;
}

void getRandSpecialEntry(uint32 buildingTypeID, uint32 lvl, uint32 &entry)
{
    uint32  const workshop[10] = { 233900, 234146, 235078, 234095, 233901, 234017, 233899, 235126, 234018, 234019 };
    switch (buildingTypeID)
    {
    case GARR_BTYPE_WORKSHOP:
        entry = workshop[urand(0, lvl > 1 ? 10 : 4)];
        break;
    default:
        break;
    }
}

//GARR_BTYPE_WORKSHOP
GameObject* Plot::CreateGameObject(Map* map, GarrisonFactionIndex faction, Garrison* garrison)
{
    if (GarrisonConst::Globals::isClassHallMap(map->GetId()))
        return nullptr;

    auto entry = EmptyGameObjectId;
    GarrBuildingEntry const* buildingEtry = nullptr;
    if (BuildingInfo.PacketInfo)
    {
        auto plotEntry = sGarrPlotStore.AssertEntry(sGarrPlotInstanceStore.AssertEntry(PacketInfo.GarrPlotInstanceID)->GarrPlotID);
        buildingEtry = sGarrBuildingStore.AssertEntry(BuildingInfo.PacketInfo->GarrBuildingID);
        entry = faction == GARRISON_FACTION_INDEX_HORDE ? plotEntry->HordeConstructObjID : plotEntry->AllianceConstructObjID;
        if (BuildingInfo.PacketInfo->Active || !entry)
            entry = faction == GARRISON_FACTION_INDEX_HORDE ? buildingEtry->HordeGameObjectID : buildingEtry->AllianceGameObjectID;
    }

    if (!sObjectMgr->GetGameObjectTemplate(entry))
    {
        TC_LOG_ERROR(LOG_FILTER_PLAYER, "Garrison attempted to spawn gameobject whose template doesn't exist (%u)", entry);
        return nullptr;
    }

    Position const& pos = PacketInfo.PlotPos;
    GameObject* building = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
    if (!building->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), entry, map, 1, pos, G3D::Quat(RotationX, RotationY, RotationZ, RotationW), 255, GO_STATE_READY))
    {
        delete building;
        return nullptr;
    }

    if ((building->GetGoType() == GAMEOBJECT_TYPE_GARRISON_BUILDING || building->GetGoType() == GAMEOBJECT_TYPE_GARRISON_PLOT)/* && building->GetGOInfo()->garrisonBuilding.mapID*/)
    {
        if (auto goList = sGarrisonMgr.GetGoSpawnBuilding(PacketInfo.GarrPlotInstanceID, BuildingInfo.PacketInfo && BuildingInfo.PacketInfo->Active ? BuildingInfo.PacketInfo->GarrBuildingID : 0))
        {
            for (auto const& data : *goList)
            {
                if (GarrisonMgr::getFirstMap(map->GetId()) != data.mapid)
                    continue;

                if (data.building && !BuildingInfo.PacketInfo || (BuildingInfo.PacketInfo && BuildingInfo.PacketInfo->Active == data.building))
                    continue;

                entry = data.id;
                bool const specSpawn = isSpecialSpawnEntry(entry);
                if (specSpawn)
                    getRandSpecialEntry(buildingEtry->BuildingType, buildingEtry->UpgradeLevel, entry);

                GameObject* linkGO = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
                if (!linkGO->Create(sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate(), entry, map, 1, Position(data.posX, data.posY, data.posZ, data.orientation), data.rotation, 255, GO_STATE_READY) || !linkGO->IsPositionValid() || !map->AddToMap(linkGO))
                {
                    delete linkGO;
                    continue;
                }

                if (buildingEtry)
                    linkGO->garrBuildingType = buildingEtry->BuildingType;

                if (specSpawn && buildingEtry)
                {
                    if (auto specTime = garrison->GetSpecialSpawnBuildingTime(buildingEtry->BuildingType))
                    {
                        // fix this? this is probably incorrect as GetSpecialSpawnBuildingTime already returns a diff
                        int32 d = specTime - time(nullptr);
                        if (d > 0)
                            linkGO->SetRespawnTime(d);
                    }
                }

                linkGO->SetRespawnDelayTime(RESP_GO_LOOT);
                BuildingInfo.Spawns.insert(linkGO->GetGUID());
            }
        }

        if (auto npcList = sGarrisonMgr.GetNpcSpawnBuilding(PacketInfo.GarrPlotInstanceID, BuildingInfo.PacketInfo && BuildingInfo.PacketInfo->Active ? BuildingInfo.PacketInfo->GarrBuildingID : 0))
        {
            for (auto const& data : *npcList)
            {
                if (GarrisonMgr::getFirstMap(map->GetId()) != data.mapid)
                    continue;

                if (data.building && !BuildingInfo.PacketInfo || (BuildingInfo.PacketInfo && BuildingInfo.PacketInfo->Active == data.building))
                    continue;

                entry = data.id;

                //rand npc check.
                if (buildingEtry && buildingEtry->BuildingType == GARR_BTYPE_TRADING_POST)
                    getRandTrader(entry);

                auto linkNPC = new Creature();
                if (!linkNPC->Create(sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate(), map, 1, entry, 0, 0, data.posX, data.posY, data.posZ, data.orientation) || !linkNPC->IsPositionValid() || !map->AddToMap(linkNPC))
                {
                    delete linkNPC;
                    continue;
                }

                if (data.building)
                    linkNPC->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, urand(0, 1) ? 173 : 69);

                BuildingInfo.Spawns.insert(linkNPC->GetGUID());
            }
        }
    }

    BuildingInfo.Guid = building->GetGUID();
    return building;
}

void Plot::DeleteGameObject(Map* map)
{
    if (BuildingInfo.Guid.IsEmpty())
        return;

    for (ObjectGuid const& guid : BuildingInfo.Spawns)
    {
        WorldObject* object = nullptr;
        switch (guid.GetHigh())
        {
        case HighGuid::Creature:
            object = map->GetCreature(guid);
            break;
        case HighGuid::GameObject:
            object = map->GetGameObject(guid);
            break;
        default:
            continue;
        }

        if (object)
            object->AddObjectToRemoveList();
    }

    BuildingInfo.Spawns.clear();

    if (GameObject* oldBuilding = map->GetGameObject(BuildingInfo.Guid))
        oldBuilding->Delete();

    BuildingInfo.Guid.Clear();
}

void Plot::ClearBuildingInfo(Player* owner)
{
    WorldPackets::Garrison::GarrisonPlotPlaced plotPlaced;
    plotPlaced.GarrTypeID = GARRISON_TYPE_GARRISON;
    plotPlaced.PlotInfo = &PacketInfo;
    owner->SendDirectMessage(plotPlaced.Write());

    BuildingInfo.PacketInfo = boost::none;
    db_state_building = DB_STATE_REMOVED;
}

void Plot::SetBuildingInfo(WorldPackets::Garrison::GarrisonBuildingInfo const& buildingInfo, Player* owner)
{
    if (!BuildingInfo.PacketInfo)
    {
        WorldPackets::Garrison::GarrisonPlotRemoved plotRemoved;
        plotRemoved.GarrPlotInstanceID = PacketInfo.GarrPlotInstanceID;
        owner->SendDirectMessage(plotRemoved.Write());
    }
    db_state_building = DB_STATE_CHANGED;
    BuildingInfo.PacketInfo = buildingInfo;
}
