#include "BracketMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"

BracketMgr::BracketMgr() = default;

BracketMgr::~BracketMgr()
{
    for (auto & itr : _container)
        for (auto & itr2 : itr.second)
            delete itr2.second;
}

BracketMgr* BracketMgr::instance()
{
    static BracketMgr instance;
    return &instance;
}

void BracketMgr::LoadCharacterBrackets()
{
    uint32 oldMSTime = getMSTime();

    //                                                      0           1       2       3           4       5       6       7               8        9           10
    QueryResult result = CharacterDatabase.Query("SELECT `bracket`, `rating`, `best`, `bestWeek`, `mmr`, `games`, `wins`, `weekGames`, `weekWins`, `guid`, `bestWeekLast` FROM `character_brackets_info`");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 bracket info. DB table `character_brackets_info` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        Bracket* bracket = TryGetOrCreateBracket(ObjectGuid::Create<HighGuid::Player>(fields[9].GetUInt64()), fields[0].GetUInt8());

        uint16 rating = fields[1].GetUInt16();
        uint16 rating_best = fields[2].GetUInt16();
        uint16 rating_best_week = fields[3].GetUInt16();
        uint16 mmv = fields[4].GetUInt16();
        uint32 games = fields[5].GetUInt32();
        uint32 wins = fields[6].GetUInt32();
        uint32 week_games = fields[7].GetUInt16();
        uint32 week_wins = fields[8].GetUInt16();
        uint16 bestWeekLast = fields[10].GetUInt16();

        bracket->InitStats(rating, mmv, games, wins, week_games, week_wins, rating_best_week, rating_best, bestWeekLast);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u brackets data in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

Bracket* BracketMgr::TryGetOrCreateBracket(ObjectGuid guid, uint8 bType)
{
    auto itr = _container.find(guid);
    if (itr == _container.end())
    {
        auto bracket = new Bracket(guid, bType);
        _container[guid][bType] = bracket;
        return bracket;
    }

    auto itr2 = _container[guid].find(bType);
    if (itr2 == _container[guid].end())
    {
        auto bracket = new Bracket(guid, bType);
        _container[guid][bType] = bracket;
        return bracket;
    }

    return _container[guid][bType];
}

void BracketMgr::DeleteBracketInfo(ObjectGuid guid)
{
    auto itr = _container.find(guid);
    if (itr == _container.end())
        return;

    for (auto & itr2 : _container[guid])
        itr2.second->SetState(BRACKET_REMOVED);
}

void BracketMgr::ResetWeekly()
{
    for (auto & itr : _container)
        for (auto & itr2 : itr.second)
            itr2.second->ResetWeekly();
}
