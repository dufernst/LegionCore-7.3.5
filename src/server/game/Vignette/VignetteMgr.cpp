#include "VignetteMgr.h"
#include "ObjectAccessor.h"
#include "UpdatePackets.h"
#include "AreaTrigger.h"
#include "Corpse.h"
#include "Conversation.h"

namespace Vignette
{

Manager::Manager(Player const* player)
{
    _owner = player;
}

Manager::~Manager()
{
    _owner = nullptr;

    for (auto itr : _vignettes)
        delete itr.second;
}

Entity* Manager::CreateAndAddVignette(VignetteEntry const* vignetteEntry, uint32 const mapID, Type const vignetteType, Position const position, uint32 zoneID, ObjectGuid const sourceGuid /*= ObjectGuid::Empty*/)
{
    for (auto v : _vignettes)
        if (v.second->GetVignetteEntry()->ID == vignetteEntry->ID && v.second->GeSourceGuid() == sourceGuid)
            return nullptr;

    auto vignette = new Entity(vignetteEntry, mapID);
    vignette->Create(vignetteType, position, zoneID, sourceGuid);

    _vignettes.insert(std::make_pair(vignette->GetGuid(), vignette));
    _addedVignette.insert(vignette->GetGuid());

    return vignette;
}

void Manager::DestroyAndRemoveVignetteByEntry(VignetteEntry const* vignetteEntry)
{
    if (!vignetteEntry)
        return;

    for (auto itr = _vignettes.begin(); itr != _vignettes.end();)
    {
        if (itr->second->GetVignetteEntry()->ID == vignetteEntry->ID)
        {
            delete itr->second;
            _removedVignette.insert(itr->first);
            itr = _vignettes.erase(itr);
            continue;
        }

        ++itr;
    }
}

void Manager::DestroyAndRemoveVignettes(std::function<bool(Entity*)> const& lambda)
{
    for (auto itr = _vignettes.begin(); itr != _vignettes.end();)
    {
        if (lambda(itr->second))
        {
            delete itr->second;
            _removedVignette.insert(itr->first);
            itr = _vignettes.erase(itr);
            continue;
        }

        ++itr;
    }
}

void Manager::SendVignetteUpdateToClient()
{
    WorldPackets::Update::VignetteUpdate updatePacket;
    updatePacket.ForceUpdate = false;

    for (auto x : _removedVignette)
        updatePacket.Removed.IDs.emplace_back(x);

    for (auto v : _updatedVignette)
    {
        auto x = _vignettes.find(v);
        if (x == _vignettes.end())
            continue;

        auto vignette = x->second;
        updatePacket.Updated.Data.emplace_back(vignette->GetGuid(), vignette->GetPosition(), vignette->GetVignetteEntry()->ID, vignette->GetZoneID());
        updatePacket.Updated.IdList.IDs.emplace_back(v);
    }

    for (auto v : _addedVignette)
    {
        auto x = _vignettes.find(v);
        if (x == _vignettes.end())
            continue;

        auto vignette = x->second;
        updatePacket.Added.Data.emplace_back(vignette->GetGuid(), vignette->GetPosition(), vignette->GetVignetteEntry()->ID, vignette->GetZoneID());
        updatePacket.Added.IdList.IDs.emplace_back(v);
    }

    _owner->GetSession()->SendPacket(updatePacket.Write());

    _updatedVignette.clear();
    _addedVignette.clear();
    _removedVignette.clear();
}

void Manager::Update()
{
    for (auto itr : _vignettes)
    {
        auto vignette = itr.second;

        if (vignette->GeSourceGuid().IsUnit())
            if (auto sourceCreature = sObjectAccessor->GetCreature(*_owner, vignette->GeSourceGuid()))
                vignette->UpdatePosition(sourceCreature->GetPosition());

        if (vignette->NeedClientUpdate())
        {
            _updatedVignette.insert(vignette->GetGuid());
            vignette->ResetNeedClientUpdate();
        }
    }

    if (!_addedVignette.empty() || !_updatedVignette.empty() || !_removedVignette.empty())
        SendVignetteUpdateToClient();
}

inline VignetteEntry const* GetVignetteEntryFromWorldObject(WorldObject const* target)
{
    uint32 vignetteId = 0;
    if (target->IsCreature())
        vignetteId = target->ToCreature()->GetCreatureTemplate()->VignetteID;

    if (target->IsGameObject())
        vignetteId = target->ToGameObject()->GetGOInfo()->GetVignetteId();

    if (!vignetteId)
        return nullptr;

    return sVignetteStore.LookupEntry(vignetteId);
}

inline uint32 GetTrackingQuestIdFromWorldObject(WorldObject const* target)
{
    uint32 trackingQuest = 0;
    if (target->IsCreature())
        trackingQuest = target->ToCreature()->GetCreatureTemplate()->TrackingQuestID;

    if (target->IsGameObject())
        trackingQuest = target->ToGameObject()->GetGOInfo()->GetTrackingQuestId();

    return trackingQuest;
}

inline Type GetDefaultVignetteTypeFromWorldObject(WorldObject const* target)
{
    switch (target->GetTypeId())
    {
        case TYPEID_UNIT:
            return Type::SourceCreature;
        case TYPEID_GAMEOBJECT:
            return Type::SourceGameObject;
        default:
            return Type::SourceScript;
    }
}

inline Type GetTrackingVignetteTypeFromWorldObject(WorldObject const* target)
{
    switch (target->GetTypeId())
    {
        case TYPEID_UNIT:
            return Type::SourceRare;
        case TYPEID_GAMEOBJECT:
            return Type::SourceTreasure;
        default:
            return Type::SourceScript;
    }
}

template <class T>
void Manager::OnWorldObjectAppear(T const* target)
{
    auto vignetteEntry = GetVignetteEntryFromWorldObject(target);
    if (vignetteEntry == nullptr)
        return;

    auto type = GetDefaultVignetteTypeFromWorldObject(target);

    if (auto trackingQuest = GetTrackingQuestIdFromWorldObject(target))
        if (!_owner->IsQuestBitFlaged(sDB2Manager.GetQuestUniqueBitFlag(trackingQuest)))
            type = GetTrackingVignetteTypeFromWorldObject(target);

    if (CanSeeVignette(target, vignetteEntry->ID))
        CreateAndAddVignette(vignetteEntry, target->GetMapId(), type, target->GetPosition(), target->GetCurrentZoneID(), target->GetGUID());
}

template <class T>
void Manager::OnWorldObjectDisappear(T const* target)
{
    if (!GetVignetteEntryFromWorldObject(target))
        return;

    DestroyAndRemoveVignettes([target](Entity const* vignette) -> bool
    {
        return vignette->GeSourceGuid() == target->GetGUID() && vignette->GetVignetteType() != Type::SourceScript;
    });
}

template <class T>
bool Manager::CanSeeVignette(T const* obj, uint32 vignetteID)
{
    if (!vignetteID/* || (obj->IsUnit() && (!obj->ToUnit()->isAlive() || obj->ToUnit()->isPet()))*/)
        return false;

    VignetteEntry const* vignette = sVignetteStore.LookupEntry(vignetteID);
    if (!vignette || !sConditionMgr->IsPlayerMeetingCondition(const_cast<Player*>(_owner), vignette->PlayerConditionID))
        return false;

    if (!sConditionMgr->IsObjectMeetToConditions(const_cast<Player*>(_owner), sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_VIGNETTE, vignetteID)))
        return false;

    auto guid = obj->GetGUID();
    uint32 trackingQuest = 0;

    if (guid.IsUnit())
        if (auto creatureSource = sObjectAccessor->GetCreature(*_owner, guid))
        {
            trackingQuest = creatureSource->GetTrackingQuestID();
            if (trackingQuest && _owner->IsQuestRewarded(trackingQuest))
                return false;
        }

    if (guid.IsGameObject())
        if (auto goSource = sObjectAccessor->FindGameObject(guid))
        {
            trackingQuest = goSource->GetGOInfo()->GetTrackingQuestId();
            if (trackingQuest && _owner->IsQuestRewarded(trackingQuest))
                return false;
        }

    return true;
}

template void Manager::OnWorldObjectDisappear(Corpse const*);
template void Manager::OnWorldObjectDisappear(Creature const*);
template void Manager::OnWorldObjectDisappear(GameObject const*);
template void Manager::OnWorldObjectDisappear(DynamicObject const*);
template void Manager::OnWorldObjectDisappear(AreaTrigger const*);
template void Manager::OnWorldObjectDisappear(Conversation const*);
template void Manager::OnWorldObjectDisappear(WorldObject const*);
template void Manager::OnWorldObjectDisappear(Player const*);

template void Manager::OnWorldObjectAppear(Corpse const*);
template void Manager::OnWorldObjectAppear(Creature const*);
template void Manager::OnWorldObjectAppear(GameObject const*);
template void Manager::OnWorldObjectAppear(DynamicObject const*);
template void Manager::OnWorldObjectAppear(AreaTrigger const*);
template void Manager::OnWorldObjectAppear(Conversation const*);
template void Manager::OnWorldObjectAppear(WorldObject const*);
template void Manager::OnWorldObjectAppear(Player const*);

}
