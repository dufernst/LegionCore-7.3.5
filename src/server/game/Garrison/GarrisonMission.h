
#ifndef GarrisonMission_h__
#define GarrisonMission_h__

#include "GarrisonGlobal.h"
#include "Packets/GarrisonPackets.h"

struct Follower;

struct Mission
{
    WorldPackets::Garrison::GarrisonMission PacketInfo;
    std::list<uint64> CurrentFollowerDBIDs;
    ObjectDBState DbState = DB_STATE_NEW;

    void Start(Player* owner, std::vector<uint64> const &followers);
    void BonusRoll(Player* onwer);
    void Complete(Player* owner);
    bool HasBonusRoll();
    float ComputeSuccessChance();
    float CalcChance(float a, float b, float c);
    uint32 GetDuration(Player* owner);
    std::vector<Follower*> GetMissionFollowers(Garrison* garrison);
};

#endif // GarrisonMission_h__
