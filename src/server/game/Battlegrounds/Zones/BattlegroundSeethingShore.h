
#ifndef __BATTLEGROUNDSS_H
#define __BATTLEGROUNDSS_H

#include "Battleground.h"

struct BattlegoundSeethingShoreScore final : BattlegroundScore
{
    friend class BattlegroundSeethingShore;

protected:
    BattlegoundSeethingShoreScore(ObjectGuid playerGuid, TeamId team);

    void UpdateScore(uint32 type, uint32 value) override;
    void BuildObjectivesBlock(std::vector<int32>& stats) override;

    uint32 Captures = 0;
};

class BattlegroundSeethingShore : public Battleground
{
    std::array<Transport*, MAX_TEAMS> _gunship;
    std::unordered_map<uint32, bool> _azeriteFissureIds;
    uint32 _spawnTimer;
    uint32 _spawnTimerDelay;
    bool _isInformedNearVictory;
    bool _startDelay;
public:
    BattlegroundSeethingShore();
    ~BattlegroundSeethingShore() override;

    void AddPlayer(Player* player) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;
    bool SetupBattleground() override;
    void Reset() override;
    void RelocateDeadPlayers(ObjectGuid guideGuid) override;
    bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
    void ActivateRandomAzeriteFissure();
    void OnGameObjectCreate(GameObject* object) override;
    void OnCreatureRemove(Creature* creature) override;
    void CastActivates(Creature* controller);
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove) override;
    uint32 GetMaxScore() const override;
    void PostUpdateImpl(uint32 diff) override;

    void TeleportToStart(Player* player);
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    void HandlePlayerResurrect(Player* player) override;
    void _CheckPositions(uint32 diff) override;
    void DoAction(uint32 type, ObjectGuid guid) override;

    void EndBattleground(uint32 winner) override;

    uint8 _activeAzerriteFissureCounter{0};
    std::list<ObjectGuid> _azeritesToActivate;

    std::array<WorldSafeLocsEntry*, 2> m_safeLocs;
    std::array<ObjectGuid, 2> m_capitans;
    std::vector<uint32> m_eventTimers{ 60000, 32000, 15000, 10000 };
    std::vector<uint32> m_boxesTimers{ urand(180, 300)*1000, urand(180, 300) * 1000 };
  
    uint8 m_introState = 0;
    uint32 ValidStartPositionTimer = 0;
};

#endif
