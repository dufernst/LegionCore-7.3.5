
#include "BrawlBattlegroundSouthshoreVsTarrenMill.h"
#include "BattlegroundPackets.h"


static uint8 const MaxRank = 16;
static uint8 const MaxScores = 150;
static uint8 const ScorePerRand = 10;

uint32 playerBuffs[MaxRank][MAX_TEAMS] =
{
    { 168412, 168429 }, // 0%
    { 168413, 168430 }, // 1%
    { 168414, 168431 }, // 2%
    { 168415, 168432 }, // 3%
    { 168416, 168433 }, // 4%
    { 168417, 168434 }, // 5%
    { 168418, 168435 }, // 6%
    { 168419, 168436 }, // 7%
    { 168420, 168437 }, // 8%
    { 168421, 168438 }, // 9%
    { 168422, 168439 }, // 10%
    { 168423, 168440 }, // 11%
    { 168424, 168441 }, // 12%
    { 168425, 168442 }, // 13%
    { 168426, 168443 }, // 14%
    { 168428, 168444 }, // 15%
};

struct BrawlBattlegroundSouthshoreVsTarrenMillScore final : BattlegroundScore
{
    friend class BrawlBattlegroundSouthshoreVsTarrenMill;

protected:
    BrawlBattlegroundSouthshoreVsTarrenMillScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team) { }

    void UpdateScore(uint32 type, uint32 value) override
    {
        BattlegroundScore::UpdateScore(type, value);
    }

    void BuildObjectivesBlock(std::vector<int32>& /*stats*/) override
    {
    }
};

BrawlBattlegroundSouthshoreVsTarrenMill::BrawlBattlegroundSouthshoreVsTarrenMill()
{
    m_playerPointsCounter.clear();
}

BrawlBattlegroundSouthshoreVsTarrenMill::~BrawlBattlegroundSouthshoreVsTarrenMill()
{

}

void BrawlBattlegroundSouthshoreVsTarrenMill::Reset()
{
    Battleground::Reset();
    m_playerPointsCounter.clear();
    _playerRank.clear();
}

bool BrawlBattlegroundSouthshoreVsTarrenMill::SetupBattleground()
{
    return true;
}

void BrawlBattlegroundSouthshoreVsTarrenMill::PostUpdateImpl(uint32 /*diff*/)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (GetElapsedTime() >= Minutes(30))
        Battleground::BattlegroundTimedWin(2);
}

void BrawlBattlegroundSouthshoreVsTarrenMill::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);

    auto playerGuid = player->GetGUID();

    PlayerScores[playerGuid] = new BrawlBattlegroundSouthshoreVsTarrenMillScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(MaxScores).Write());
    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);

    m_playerPointsCounter[playerGuid] = 0;
    _playerRank[playerGuid] = 0;
}

void BrawlBattlegroundSouthshoreVsTarrenMill::OnPlayerEnter(Player* player)
{
    Battleground::OnPlayerEnter(player);
}

WorldSafeLocsEntry const* BrawlBattlegroundSouthshoreVsTarrenMill::GetClosestGraveYard(Player* player)
{
    return sWorldSafeLocsStore.LookupEntry(player->GetBGTeamId() == TEAM_ALLIANCE ? 4850 : 4851);
}

void BrawlBattlegroundSouthshoreVsTarrenMill::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    Battleground::HandleKillPlayer(player, killer);

    if (player == killer)
        return;

    auto killerGuid = killer->GetGUID();

    ++m_TeamScores[killer->GetBGTeamId()];
    m_playerPointsCounter[killerGuid] += _playerRank[player->GetGUID()] + 1;

    player->RemoveAura(playerBuffs[_playerRank[player->GetGUID()]][player->GetTeamId()]);

    if (m_playerPointsCounter[killerGuid] >= ScorePerRand)
    {
        _playerRank[killerGuid] = std::min(uint8(1+_playerRank[killerGuid]), MaxRank);
        m_playerPointsCounter[killerGuid] = 0;

        auto killerTeamId = killer->GetTeamId();
        killer->RemoveAura(playerBuffs[_playerRank[killerGuid] - 1][killerTeamId]);
        killer->CastSpell(killer, playerBuffs[_playerRank[killerGuid]][killerTeamId]);
    }
}

bool BrawlBattlegroundSouthshoreVsTarrenMill::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    return true;
}

void BrawlBattlegroundSouthshoreVsTarrenMill::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    
}
