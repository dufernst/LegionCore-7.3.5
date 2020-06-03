#ifndef VIGNETTE_H
#define VIGNETTE_H

#include "Common.h"
#include "DB2Stores.h"
#include "Object.h"

namespace Vignette
{
enum class Type : uint8
{
    SourceCreature,           ///< Used for vignette linked to creature, player show the vignette if the creature is in the view-range
    SourceGameObject,         ///< Used for vignette linked to gameobject, player show the vignette if the gameobject is in the view-range
    SourceRare,               ///< Used for vignette linked to rare creature, player show the vignette if the creature is in the view-range and if the player doesn't have the tracking quest
    SourceTreasure,           ///< Used for vignette linked to treasure gameobject, player show the vignette if the treasure is in the view-range and if the player doesn't have the tracking quest
    SourceScript              ///< Used for script, that kind of vignette are never remove automaticaly
};

class Entity
{
    friend class Manager;

public:
    void UpdatePosition(Position newPosition);
    void ResetNeedClientUpdate();
    ObjectGuid GetGuid() const;
    ObjectGuid GeSourceGuid() const;
    Type GetVignetteType() const;
    bool NeedClientUpdate() const;
    Position const& GetPosition() const;
    uint32 GetZoneID() const;
    VignetteEntry const* GetVignetteEntry() const;

private:
    Entity(VignetteEntry const* vignetteEntry, uint32 mapID);
    ~Entity();

    void Create(Type type, Position const& position, uint32 zoneID, ObjectGuid sourceGuid);

    VignetteEntry const* _vignetteEntry;
    ObjectGuid _guid;
    ObjectGuid _sourceGuid;
    Position _position;
    uint32 _map;
    uint32 _zoneID;
    Type _type;

protected:
    bool _needClientUpdate;
};

}
#endif
