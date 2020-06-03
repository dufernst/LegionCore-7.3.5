#ifndef __BATTLEGROUNDDM_H
#define __BATTLEGROUNDDM_H

#include "Battleground.h"
#include "BattlegroundScore.h"

//! 1. Дроп за Дм
//! 1место СА 143680(400ед шанс 100%), с шансом 30% 860 дроп база ilvl, 138469 (С шансом 100% Орда) 138474 (С шансом 100% Альянс)
//! 2место СА 138880(200ед шанс 100%), 138469 (С шансом 50% Орда) 138474 (С шансом 50% Альянс)
//! 3место СА 141858(100ед шанс 100%) 138469 (С шансом 25% Орда) 138474 (С шансом 25% Альянс)
//! other  138469 (С шансом 5% Орда) 138474 (С шансом 5% Альянс)
//! 2. Точки реса от колв игроков Дм
//! От, количества игроков, подключаются дополнительные точки ресса. Кд на спеллы после смерти сбрасывается.
//! 3. Система мега киллов

//! Каждй раз при убийстве нескольких героев подряд, без собственной смерти, увеличивается размер получаемо хонора за его убийство. 
//! Стандартная награда 1-5 хонора.
//! При убийстве игрока со стриком >5 (Mega Kill), награда 5-10 хонора. 
//! При убийстве игрока со стриком >10 (Beyond Godlike), награда 10-30 хонора. 
//! Анонсы в чат
//! 3 кила – “Killing Spree” 
//! 4 – “Dominating” 
//! 5 – “Mega Kill” 
//! 6 – “Unstoppable” 
//! 7 – “Wicked Sick” 
//! 8 – “Monster Kill” 
//! 9 – “Godlike” 
//! 10 – “Beyond Godlike” 

//! Если же вражеские герои умирают почти одновременно от вашей руки вы услышите 
//! “Double Kill!” за двух героев
//! “Triple Kill!” за троих
//! “Ultra Kill!” за четверых и
//! “RAMPAGE!”, за пятерых.

//! 4.Спелл http://ru.wowhead.com/spell=46392 накладывается на игрока после 5ых убийств подряд (10%)
//! С шагом в 2 убийства увеличивается % на 10



enum SomeValues
{
    KILLS_FOR_HIGH_COST         = 5,
    BASE_TIME_FOR_STRIKE        = 10000,
    SPELL_FOCUSED_ASSAULT       = 46392, // for increasing damage taken
    
    GOB_BUFF_EYE                = 184663,    
};


enum Texts
{
    TEXT_LOSE_STRIKE_BY         = 20200,
    TEXT_KILLED_BY              = 20201,
    TEXT_DOUBLE_KILL            = 20202,
    TEXT_TRIPLE_KILL            = 20203,
    TEXT_ULTRA_KILL             = 20204,
    TEXT_RAMPAGE                = 20205,
    
    TEXT_KILLING_SPREE          = 20206,
    TEXT_DOMINATING             = 20207,
    TEXT_MEGA_KILL              = 20208,
    TEXT_UNSTOPPABLE            = 20209,
    TEXT_WICKED_SICK            = 20210,
    TEXT_MONSTER_KILL           = 20211,
    TEXT_GOD_LIKE               = 20212,
    TEXT_BEYOUND_GOD_LIKE       = 20213,
};

uint32 const DM_Buffs[3]
{
    BG_OBJECTID_SPEEDBUFF_ENTRY,
    BG_OBJECTID_REGENBUFF_ENTRY,
    BG_OBJECTID_BERSERKERBUFF_ENTRY,
    // GOB_BUFF_EYE
 };
 
Position const dm_buf_pos[6]
{
    {731.42f, 301.24f, 1.55f, 1.92f},
    {500.06f, 200.30f, 40.51f, 3.17f},
    {761.47f, 604.40f, 32.43f, 0.92f},
    {1297.98f, 562.80f, 40.51f, 6.12f},
    {1062.31f, 468.56f, 0.91f, 3.38f},
    
    {1036.84f, 160.94f, 83.32f, 5.01f}
};

struct BattlegroundDMScore final : BattlegroundScore
{
    friend class BattlegroundDeathMatch;

protected:
    BattlegroundDMScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team) { }

    void UpdateScore(uint32 type, uint32 value) override
    {
        BattlegroundScore::UpdateScore(type, value);
    }

    void BuildObjectivesBlock(std::vector<int32>& stats) override {}
};


#define MIDDLE_HP   4500000
#define STEP_RATING 20
#define KILLS_PER_STEP 15

class BattlegroundDeathMatch : public Battleground
{
public:
    BattlegroundDeathMatch() : DMTeam(1), timer_for_end(1200000), small_delayed_timer(60000) {BgObjects.resize(6);}
    ~BattlegroundDeathMatch() { }
    
    void Reset() override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;

    void AddPlayer(Player* player) override;
    void OnPlayerEnter(Player* player) override;
    void RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket) override;
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    void HandleKillPlayer(Player*, Player*) override;
    void HandleStartTimer(TimerType type) override {}; // not needed
    void EndBattleground(uint32 winner) override;
    
    void PostUpdateImpl(uint32 diff) override;

    static int32 CalculateRating(BattlegroundScore* bs) { return CalculateRating(bs->GetScore(SCORE_KILLING_BLOWS), bs->GetScore(SCORE_DEATHS), bs->GetScore(SCORE_DAMAGE_DONE)); }
    static int32 CalculateRating(uint32 kills, uint32 dies, uint64 dmg);
    
private:
    
    
    void SendSysMessageToAll(uint32 textid, Player* first = nullptr, Player* second = nullptr);
    void SendDirectMessageToAll(uint32 textid, Player* first);
    
    uint32 DMTeam;
    std::map<ObjectGuid, uint32> strike;
    std::map<ObjectGuid, std::pair<uint32, uint32>> temp_strike;  // guid, kills, last time
    
    uint32 timer_for_end;
    uint32 small_delayed_timer;
};


#endif