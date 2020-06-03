#ifndef __BrawlBattlegroundSouthshoreVsTarrenMill
#define __BrawlBattlegroundSouthshoreVsTarrenMill

#include "Battleground.h"

class BrawlBattlegroundSouthshoreVsTarrenMill : public Battleground
{
    std::unordered_map<ObjectGuid, uint8> m_playerPointsCounter;
    std::unordered_map<ObjectGuid, uint8> _playerRank;
public:
    BrawlBattlegroundSouthshoreVsTarrenMill();
    ~BrawlBattlegroundSouthshoreVsTarrenMill();

    void Reset() override;
    bool SetupBattleground() override;
    void PostUpdateImpl(uint32 diff) override;
    void AddPlayer(Player* player) override;
    void OnPlayerEnter(Player* player) override;
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    void HandleKillPlayer(Player* player, Player* killer) override;
    bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
};

#endif
