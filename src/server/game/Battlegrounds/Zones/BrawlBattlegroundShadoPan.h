#ifndef __BrawlBattlegroundShadoPan
#define __BrawlBattlegroundShadoPan

#include "Battleground.h"


enum BG_SP_Spells
{
    SPELL_GOLD_ALLIANCE         = 245636,
    SPELL_GOLD_HORDE            = 251672,
    SPELL_PURPLE_ALLIANCE       = 245635,
    SPELL_PURPLE_HORDE          = 251579,

    SPELL_GOLD_TEAM_CONTROL     = 236648,
    SPELL_NEUTRAL_TEAM_CONTROL  = 236637,
    SPELL_PURPLE_TEAM_CONTROL   = 236649,

    SPELL_SWITCH_OFF_BOSS       = 236666,
    SPELL_GIFT_OF_THE_EMPEROR   = 236722,
};

enum BG_SP_Objects
{
    BG_SP_ENTRY_DOOR_1          = 219395,
    BG_SP_ENTRY_DOOR_2          = 219395,
    BG_SP_ENTRY_CHEST           = 267961,
};

enum BG_SP_ObjectTypes
{
    BG_SP_DOOR_1                = 0,
    BG_SP_DOOR_2,
    BG_SP_CHEST_1,
    BG_SP_CHEST_2,
    BG_SP_MAX
};

enum BG_SP_CreatureTypes
{
    SP_SPIRIT_MAIN_ALLIANCE = 0,
    SP_SPIRIT_MAIN_HORDE = 1,
    SP_BOSS_FOR_GOLD,
    SP_BOSS_FOR_PURPLE,
    SP_CONTROLLER,
    SP_BARRIER_1,
    SP_BARRIER_2,

    BG_CREATURES_MAX_SP,
};

enum BG_SP_Creature
{
    SP_BOSS_FOR_GOLD_ENTRY      = 119194,
    SP_BOSS_FOR_PURPLE_ENTRY    = 122183,

    SP_NPC_BARRIER          = 119195,
    SP_NPC_CONTROLLER       = 119183,
};

class BrawlBattlegroundShadoPan : public Battleground
{
public:
    BrawlBattlegroundShadoPan();
    ~BrawlBattlegroundShadoPan();
    void Reset() override;

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
    void PostUpdateImpl(uint32 diff) override;

    void AddPlayer(Player* player) override;
    void RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/) override;

    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;

    void EventPlayerUsedGO(Player* /*player*/, GameObject* /*go*/) override;

private:
    void CheckAndUpdatePointStatus(uint32 diff);
    void UpdateBossesAndController(uint32 diff);

    uint32 m_pointUpdateTimer{};
    uint32 m_bossessUpdateTimer{};
    int8 m_score = 50;
    uint32 m_chestRespawnTimer = 45000;
    bool m_waitChestRespawn = true;
};


struct BrawlBattlegroundShadoPanScore final : BattlegroundScore
{
    friend class BrawlBattlegroundShadoPan;

protected:
    BrawlBattlegroundShadoPanScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team) { }

    void UpdateScore(uint32 type, uint32 value) override
    {
        BattlegroundScore::UpdateScore(type, value);
    }

    void BuildObjectivesBlock(std::vector<int32>& /*stats*/) override
    {
    }
};

#endif
