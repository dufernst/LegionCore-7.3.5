#ifndef VIGNETTE_MGR_HPP
#define VIGNETTE_MGR_HPP

#include "Vignette.h"

class WorldObject;
class GameObject;
class Creature;
class Player;

namespace Vignette
{
class Manager
{
public:
    explicit Manager(Player const* player);
    ~Manager();

    Entity* CreateAndAddVignette(VignetteEntry const* vignetteEntry, uint32 mapID, Type vignetteType, Position position, uint32 zoneID, ObjectGuid sourceGuid = ObjectGuid::Empty);
    void DestroyAndRemoveVignetteByEntry(VignetteEntry const* vignetteEntry);
    void DestroyAndRemoveVignettes(std::function<bool(Entity*)> const& lambda);

    void Update();

    template<class T>
    void OnWorldObjectAppear(T const* target);

    template<class T>
    void OnWorldObjectDisappear(T const* target);

    template<class T>
    bool CanSeeVignette(T const* obj, uint32 vignetteID);
private:
    void SendVignetteUpdateToClient();

    Player const* _owner;

    std::map<ObjectGuid, Entity*> _vignettes;
    std::set<ObjectGuid> _removedVignette;
    std::set<ObjectGuid> _addedVignette;
    std::set<ObjectGuid> _updatedVignette;
};

}

#endif ///< VIGNETTE_MGR_HPP
