#include "BattlegroundMap.h"
#include "ObjectMgr.h"
#include "Map.h"

BattlegroundMap::BattlegroundMap(uint32 id, time_t expiry, uint32 instanceId, Map* parent, Difficulty difficulty) : Map(id, expiry, instanceId, difficulty, parent)
{
    m_bg = nullptr;

    BattlegroundMap::InitVisibilityDistance();

    if (auto _distMap = GetVisibleDistance(TYPE_VISIBLE_MAP, id))
        m_VisibleDistance = _distMap;
}

BattlegroundMap::~BattlegroundMap()
{
    if (!m_bg)
        return;

    m_bg->SetBgMap(nullptr);
    m_bg = nullptr;
}

void BattlegroundMap::InitVisibilityDistance()
{
    m_VisibleDistance = GetEntry()->IsBattleArena() ? sWorld->GetMaxVisibleDistanceInArenas() : sWorld->GetMaxVisibleDistanceInBG();
    m_VisibilityNotifyPeriod = World::GetVisibilityNotifyPeriodInBGArenas();
}

Battleground* BattlegroundMap::GetBG()
{
    return m_bg;
}

void BattlegroundMap::SetBG(Battleground* bg)
{
    m_bg = bg;
}

bool BattlegroundMap::CanEnter(Player* player)
{
    if (player->GetMapRef().getTarget() == this)
    {
        TC_LOG_ERROR(LOG_FILTER_MAPS, "BGMap::CanEnter - player %u is already in map!", player->GetGUIDLow());
        ASSERT(false);
        return false;
    }

    if (player->GetBattlegroundId() != GetInstanceId())
        return false;

    return Map::CanEnter(player);
}

bool BattlegroundMap::AddPlayerToMap(Player* player, bool initPlayer /*= true*/)
{
    if (auto bg = player->GetBattleground())
        bg->OnPlayerEnter(player);

    player->AddDelayedEvent(10, [player]() -> void
    {
        player->OnEnterMap(); // UpdatePhase
    });

    player->m_InstanceValid = true;
    return Map::AddPlayerToMap(player, initPlayer);
}

void BattlegroundMap::RemovePlayerFromMap(Player* player, bool remove)
{
    if (player && player->IsSpectator() && !player->IsSpectateCanceled())
        if (GetBG())
            GetBG()->RemoveSpectator(player);

#ifdef TRINITY_DEBUG
    TC_LOG_INFO(LOG_FILTER_MAPS, "MAP: Removing player '%s' from bg '%u' of map '%s' before relocating to another map", player->GetName(), GetInstanceId(), GetMapName());
#endif
    Map::RemovePlayerFromMap(player, remove);
}

void BattlegroundMap::SetUnload()
{
    m_unloadTimer = MIN_UNLOAD_DELAY;
}

void BattlegroundMap::RemoveAllPlayers()
{
    if (!HavePlayers())
        return;

    for (auto& itr : m_mapRefManager)
    {
        auto player = itr.getSource();
        if (!player || player->IsBeingTeleportedFar())
            continue;

        if (player->IsSpectator())
            player->SetSpectateRemoving(true);
        player->TeleportTo(player->GetBattlegroundEntryPoint());
    }
}

