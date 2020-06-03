/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __BATTLEGROUNDMGR_H
#define __BATTLEGROUNDMGR_H

#include "Battleground.h"
#include "BattlegroundQueue.h"
#include "Common.h"
#include "FunctionProcessor.h"

namespace WorldPackets
{
    namespace Battleground
    {
        struct BattlefieldStatusHeader;

        class BattlefieldStatusNone;
        class BattlefieldStatusNeedConfirmation;
        class BattlefieldStatusActive;
        class BattlefieldStatusQueued;
        class BattlefieldStatusFailed;
    }
}

struct CreateBattlegroundData
{
    Position TeamStartLoc[MAX_TEAMS]{};
    uint32 MinPlayersPerTeam;
    uint32 MaxPlayersPerTeam;
    uint32 LevelMin;
    uint32 LevelMax;
    uint32 MapID;
    uint32 scriptId;
    char const* BattlegroundName;
    uint16 bgTypeId;
    bool IsArena;
    bool IsRbg;
    bool IsBrawl;

    uint8 MaxGroupSize;
};

struct QueueSchedulerItem
{
    QueueSchedulerItem(uint32 MMRating, uint8 joinType, uint8 bgQueueTypeId, uint16 bgTypeId, uint8 bracketid, Roles role = ROLES_DEFAULT, uint8 bracket_MinLevel = 0);
    QueueSchedulerItem() = default;

    uint32 MatchMakingRating = 0;
    uint16 BgTypeID = 0;
    Roles _role = ROLES_DEFAULT;
    uint8 JoinType = 0;
    uint8 BgQueueTypeID = 0;
    uint8 BracketMinLevel = 0;
    uint8 BracketID = 0;
};

enum PvpRewardTypes : uint8
{
    PvpReward_Skirmish     = 0, // Skirmish Win
    PvpReward_BG           = 1, // Random BG Win
    PvpReward_Arena_2v2    = 2, // 2v2
    PvpReward_Arena_3v3    = 3, // 3v3
    PvpReward_RBG          = 4, // Rated BG Win
    PvpReward_Arena_1v1    = 5, // 1v1
    PvpReward_DeathMatch   = 6, // deathmatch
    PvpReward_Brawl        = 7, // Brawl
    PvpReward_BrawlArena   = 8, // BrawlArena
};

struct PvpReward
{
    std::vector<uint32> ItemsA;
    std::vector<uint32> ItemsH;
    std::vector<uint32> ItemsAElit;
    std::vector<uint32> ItemsHElit;
    std::vector<uint32> Relics;
    std::vector<uint32> RelicsElit;
    uint32 ChestA = 0;
    uint32 ChestH = 0;
    float ChestChance = 0.0f;
    uint32 QuestAFirst = 0;
    uint32 QuestAWin = 0;
    uint32 QuestAlose = 0;
    uint32 QuestHFirst = 0;
    uint32 QuestHWin = 0;
    uint32 QuestHLose = 0;
    float ItemsChance = 0.0f;
    uint32 ItemCAFirst = 0;
    uint32 ItemCAWin = 0;
    uint32 ItemCALose = 0;
    uint8 BracketLevel = 0;
    uint16 BaseLevel = 0;
    uint16 ElitLevel = 0;
    uint16 BonusBaseLevel = 0;
    float RateLegendary = 0.0f;
    float ChanceBonusLevel = 0.0f;
};

struct BrawlData
{
    BrawlData(uint16 bgTypeId, uint32 holidayId, uint32 journalId, std::string name, std::string description, std::string description2, uint8 type);
    BrawlData() = default;

    uint16 BgTypeId = 0;
    uint32 HolidayNameId = 0;
    uint32 JournalId = 0;
    std::string Name; // like comment
    std::string Description;
    std::string Description2;
    uint8 Type = 0;

    std::vector<uint32> HolidayIDs{};
};

class BattlegroundMgr
{
    BattlegroundMgr();
    ~BattlegroundMgr();

public:
    static BattlegroundMgr* instance();

    void Update(uint32 diff);

    void SendBattlegroundList(Player* player, ObjectGuid const& guid, uint16 bgTypeId);
    void BuildBattlegroundStatusHeader(WorldPackets::Battleground::BattlefieldStatusHeader* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 joinType);
    void BuildBattlegroundStatusNone(WorldPackets::Battleground::BattlefieldStatusNone* battlefieldStatus, Player* player, uint32 ticketId, uint32 joinTime);
    void BuildBattlegroundStatusNeedConfirmation(WorldPackets::Battleground::BattlefieldStatusNeedConfirmation* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 timeout, uint32 joinType, uint8 bracketId = 0);
    void BuildBattlegroundStatusActive(WorldPackets::Battleground::BattlefieldStatusActive* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 joinType);
    void BuildBattlegroundStatusQueued(WorldPackets::Battleground::BattlefieldStatusQueued* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 avgWaitTime, uint32 joinType, bool asGroup);
    void BuildBattlegroundStatusFailed(WorldPackets::Battleground::BattlefieldStatusFailed* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint8 result, ObjectGuid const* errorGuid = nullptr);
    void SendAreaSpiritHealerQuery(Player* player, Battleground* bg, ObjectGuid const& guid);

    Battleground* GetBattlegroundThroughClientInstance(uint32 instanceId, uint16 bgTypeId);
    Battleground* GetBattleground(uint32 InstanceID, uint16 bgTypeId);

    Battleground* GetBattlegroundTemplate(uint16 bgTypeId);
    Battleground* CreateNewBattleground(uint16 bgTypeId, PVPDifficultyEntry const* bracketEntry, uint8 joinType, bool isRated, uint16 generatedType = MS::Battlegrounds::BattlegroundTypeId::None, bool useTournamentRules = false, bool isWarGame = false);

    void CreateBattleground(CreateBattlegroundData& data);

    void AddBattleground(uint32 InstanceID, uint16 bgTypeId, Battleground* BG);
    void RemoveBattleground(uint32 instanceID, uint16 bgTypeId);
    uint32 CreateClientVisibleInstanceId(uint16 bgTypeId, uint8 bracketID);

    void CreateInitialBattlegrounds();
    void DeleteAllBattlegrounds();

    void SendToBattleground(Player* player, uint32 InstanceID, uint16 bgTypeId);

    BattlegroundQueue& GetBattlegroundQueue(uint8 bgQueueTypeId);

    std::list<Battleground*> BGFreeSlotQueue;

    void ScheduleQueueUpdate(QueueSchedulerItem* data);
    uint32 GetMaxRatingDifference() const;
    uint32 GetRatingDiscardTimer()  const;
    uint32 GetPrematureFinishTime() const;

    void ToggleTesting();

    void LoadBattleMastersEntry();
    uint16 GetBattleMasterBG(uint32 entry) const;

    void LoadPvpRewards();

    bool isTesting() const;

    std::map<uint16, uint8>* GetSelectionWeight(uint8 iternalPvpType);

    PvpReward* GetPvpReward(PvpRewardTypes type);
    PvpRewardTypes GetPvpRewardType(Battleground* bg);

    CreateBattlegroundData const* GetBattlegroundData(uint32 type);

    bool HaveSpectatorData() const;
    void EraseSpectatorData(uint32 instanceID);
    void AddSpectatorData(uint32 instanceID, ObjectGuid const& guid);
    std::map<uint32, std::list <ObjectGuid>>& GetSpectatorData();

    void InitWargame(Player* player, ObjectGuid opposingPartyMember, uint64 queueID, bool accept);

    uint8 GetBrawlIternalGroup() const;
    bool IsBrawlActive(uint32& expirationTime) const;
    void InitializeBrawlData();
    uint16 GetAtiveBrawlBgTypeId();
    BrawlData GetBrawlData();

    void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function);

private:
    FunctionProcessor m_Functions;
    BattlegroundQueue _battlegroundQueues[MS::Battlegrounds::BattlegroundQueueTypeId::Max];
    std::map<uint32, std::list <ObjectGuid>> _spectatorData;
    std::map<uint32, Battleground*> _battlegrounds[MS::Battlegrounds::BattlegroundTypeId::Max];
    std::map<uint16, uint8> _selectionWeights[MS::Battlegrounds::IternalPvpTypes::Max];
    std::map<uint32, CreateBattlegroundData> _battlegroundData;
    std::unordered_map<uint8 /*Type*/, PvpReward> _pvpRewardsContainer;
    std::unordered_map<uint32, uint16> _battleMastersMap;
    std::vector<QueueSchedulerItem*> _queueUpdateScheduler;
    std::set<uint32> _clientBattlegroundIDs[MS::Battlegrounds::BattlegroundTypeId::Max][MS::Battlegrounds::MaxBrackets];
    std::unordered_map<uint8, BrawlData> _brawlTemplatesContainer;
    uint32 _nextRatedArenaUpdate;
    bool _testing;
};

#define sBattlegroundMgr BattlegroundMgr::instance()

#endif

