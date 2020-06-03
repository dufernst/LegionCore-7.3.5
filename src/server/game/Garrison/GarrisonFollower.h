
#ifndef GarrisonFollower_h__
#define GarrisonFollower_h__

#include "GarrisonGlobal.h"
#include "Packets/GarrisonPackets.h"

struct Follower
{
    WorldPackets::Garrison::GarrisonFollower PacketInfo;

    uint32 GetItemLevel() const;
    void ModAssistant(SpellInfo const* spellInfo, Player* caster);
    void IncreaseFollowerExperience(SpellInfo const* spellInfo, Player* caster);
    uint8 RollQuality(uint32 baseQuality);
    void GiveLevel(uint32 level);
    void SetQuality(uint32 quality);
    uint32 GiveXP(uint32 xp);

    uint8 TypeID = GarrisonConst::FollowerType::Garrison;
    uint32 Faction = 0;
    std::unordered_map<uint32, uint32> ItemTraits;
    ObjectDBState DbState = DB_STATE_UNCHANGED;
    ObjectDBState db_state_ability = DB_STATE_UNCHANGED;
};

#endif // GarrisonFollower_h__
