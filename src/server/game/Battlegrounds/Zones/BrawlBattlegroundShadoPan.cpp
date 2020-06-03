#include "BrawlBattlegroundShadoPan.h"
#include "WorldStatePackets.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"

BrawlBattlegroundShadoPan::BrawlBattlegroundShadoPan()
{
    BgObjects.resize(BG_SP_MAX);
    BgCreatures.resize(BG_CREATURES_MAX_SP);
}

BrawlBattlegroundShadoPan::~BrawlBattlegroundShadoPan()
{
}

void BrawlBattlegroundShadoPan::Reset()
{
    Battleground::Reset();
}


void BrawlBattlegroundShadoPan::StartingEventCloseDoors()
{
    for (uint32 i = BG_SP_DOOR_1; i <= BG_SP_DOOR_2; ++i)
    {
        DoorClose(i);
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
    }

    for (uint32 i = SP_BARRIER_1; i <= SP_BARRIER_2; ++i)
        if (Creature* barr = GetBGCreature(i))
            barr->SetVisible(false);
}

void BrawlBattlegroundShadoPan::StartingEventOpenDoors()
{
    for (uint32 i = BG_SP_DOOR_1; i <= BG_SP_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = BG_SP_CHEST_1; i <= BG_SP_CHEST_2; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);

    for (uint32 i = SP_BARRIER_1; i <= SP_BARRIER_2; ++i)
        if (Creature* barr = GetBGCreature(i))
            barr->AddDelayedEvent(2000, [barr]() -> void
        {
            barr->SetVisible(true);
        });

    UpdateWorldState(WorldStates::ARENA_SHOW_END_TIMER, 1);
    UpdateWorldState(WorldStates::ARENA_END_TIMER, int32(time(nullptr) + std::chrono::duration_cast<Seconds>(Minutes(15)).count()));
}

bool BrawlBattlegroundShadoPan::SetupBattleground()
{
    if (!AddObject(BG_SP_DOOR_1, BG_SP_ENTRY_DOOR_1, 4235.771f, -433.5714f, 380.6548f, 3.141593f, 0, 0, 1, -0.00000004371139f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SP_DOOR_2, BG_SP_ENTRY_DOOR_2, 4365.829f, -433.5714f, 380.6548f, 0, 0, 0, 1, -0.00000004371139f, RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_SP_CHEST_1, BG_SP_ENTRY_CHEST, 4300.008f, -402.0504f, 383.6811f, 1.554882f, 0, 0, 0, 1, RESPAWN_ONE_DAY) ||
        !AddObject(BG_SP_CHEST_2, BG_SP_ENTRY_CHEST, 4300.008f, -464.5573f, 383.6811f, 4.737973f, 0, 0, 0, 1, RESPAWN_ONE_DAY))
        return false;

    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(5850);
    if (!sg || !AddSpiritGuide(SP_SPIRIT_MAIN_ALLIANCE, sg->Loc, TEAM_ALLIANCE))
        return false;

    sg = sWorldSafeLocsStore.LookupEntry(5851);
    if (!sg || !AddSpiritGuide(SP_SPIRIT_MAIN_HORDE, sg->Loc, TEAM_HORDE))
        return false;

    if (!AddCreature(SP_BOSS_FOR_PURPLE_ENTRY, SP_BOSS_FOR_PURPLE, TEAM_ALLIANCE, 4300.14f, -425.4167f, 380.5921f, 5.5526f) ||
        !AddCreature(SP_BOSS_FOR_GOLD_ENTRY, SP_BOSS_FOR_GOLD, TEAM_HORDE, 4299.793f, -440.9948f, 380.5918f, 1.188661f) ||
        !AddCreature(SP_NPC_CONTROLLER, SP_CONTROLLER, TEAM_NEUTRAL, 4300.288f, -433.1354f, 380.6615f, 0) ||
        !AddCreature(SP_NPC_BARRIER, SP_BARRIER_1, TEAM_NEUTRAL, 4366.558f, -433.6233f, 380.7882f, 0) ||
        !AddCreature(SP_NPC_BARRIER, SP_BARRIER_2, TEAM_NEUTRAL, 4235.521f, -433.625f, 380.7905f, 0))
        return false;

    return true;
}

void BrawlBattlegroundShadoPan::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    UpdateBossesAndController(diff);
    CheckAndUpdatePointStatus(diff);

    if (GetElapsedTime() >= Minutes(15))
        Battleground::BattlegroundTimedWin();

    if (m_waitChestRespawn)
        if (m_chestRespawnTimer <= diff)
        {
            SpawnBGObject(urand(BG_SP_CHEST_1, BG_SP_CHEST_2), RESPAWN_IMMEDIATELY);

            if(Creature* controller = GetBGCreature(SP_CONTROLLER))
                controller->AI()->ZoneTalk(2);
            m_waitChestRespawn = false;
        }
        else
            m_chestRespawnTimer -= diff;
}

void BrawlBattlegroundShadoPan::AddPlayer(Player * player)
{
    Battleground::AddPlayer(player);

    auto playerGuid = player->GetGUID();

    PlayerScores[playerGuid] = new BrawlBattlegroundShadoPanScore(player->GetGUID(), player->GetBGTeamId());

    if (player->GetBGTeamId() == TEAM_ALLIANCE) // gold
    {
        player->CastSpell(player, player->GetTeamId() == TEAM_ALLIANCE ? SPELL_GOLD_ALLIANCE : SPELL_GOLD_HORDE, true);
        if (Creature* cre = GetBGCreature(SP_BOSS_FOR_GOLD))
            cre->CastSpell(cre, player->GetTeamId() == TEAM_ALLIANCE ? SPELL_GOLD_ALLIANCE : SPELL_GOLD_HORDE, true);
    }
    else
    {
        player->CastSpell(player, player->GetTeamId() == TEAM_ALLIANCE ? SPELL_PURPLE_ALLIANCE : SPELL_PURPLE_HORDE, true);
        if (Creature* cre = GetBGCreature(SP_BOSS_FOR_PURPLE))
            cre->CastSpell(cre, player->GetTeamId() == TEAM_ALLIANCE ? SPELL_PURPLE_ALLIANCE : SPELL_PURPLE_HORDE, true);
    }
 
}

void BrawlBattlegroundShadoPan::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/)
{
    if (!player)
        return;

    if (player->GetBGTeamId() == TEAM_ALLIANCE) // gold
        player->RemoveAurasDueToSpell(player->GetTeamId() == TEAM_ALLIANCE ? SPELL_GOLD_ALLIANCE : SPELL_GOLD_HORDE);
    else
        player->RemoveAurasDueToSpell(player->GetTeamId() == TEAM_ALLIANCE ? SPELL_PURPLE_ALLIANCE : SPELL_PURPLE_HORDE);
}


WorldSafeLocsEntry const * BrawlBattlegroundShadoPan::GetClosestGraveYard(Player * player)
{
    return sWorldSafeLocsStore.LookupEntry(player->GetBGTeamId() == TEAM_ALLIANCE ? 5850 : 5851);
}

void BrawlBattlegroundShadoPan::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates & packet)
{
    for (uint32 i = 0; i < 14000; ++i)
        packet.Worldstates.emplace_back(i, 0);

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        packet.Worldstates.emplace_back(WorldStates::ARENA_SHOW_END_TIMER, 1);
        packet.Worldstates.emplace_back(WorldStates::ARENA_END_TIMER, int32(time(nullptr) + std::chrono::duration_cast<Seconds>(Minutes(15) - GetElapsedTime()).count()));
    }
    else
    {
        packet.Worldstates.emplace_back(WorldStates::ARENA_SHOW_END_TIMER, 0);
        packet.Worldstates.emplace_back(WorldStates::BG_SP_PURPLE_BOSS, 100);
        packet.Worldstates.emplace_back(WorldStates::BG_SP_GOLD_BOSS, 100);
    }

    packet.Worldstates.emplace_back(WorldStates::BG_SP_SHOW_BAR, 0);
    packet.Worldstates.emplace_back(WorldStates::BG_SP_BAR_STATUS, m_score);
    packet.Worldstates.emplace_back(WorldStates::BG_SP_GREY_BAR, 1);
}

void BrawlBattlegroundShadoPan::EventPlayerUsedGO(Player * player, GameObject * go)
{
    if (go->GetEntry() != BG_SP_ENTRY_CHEST)
        return;

    for (auto itr : GetPlayers())
        if (Player* tempPlayer = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
            if (tempPlayer->GetBGTeamId() == player->GetBGTeamId())
                tempPlayer->CastSpell(tempPlayer, SPELL_GIFT_OF_THE_EMPEROR, true);

    m_chestRespawnTimer = urand(43000, 50000);
    m_waitChestRespawn = true;
    
    if (Creature* controller = GetBGCreature(SP_CONTROLLER))
        controller->AI()->ZoneTalk(player->GetBGTeamId());

    SpawnBGObject(GetObjectType(go->GetGUID()), RESPAWN_ONE_DAY);
}

void BrawlBattlegroundShadoPan::CheckAndUpdatePointStatus(uint32 diff)
{
    m_pointUpdateTimer += diff;
    if (m_pointUpdateTimer < 1000)
        return;

    Creature* controller = GetBGCreature(SP_CONTROLLER);
    if (!controller)
        return;
    int8 changeAmount = 0;
    for (auto itr : GetPlayers())
        if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
            if (player->isAlive() && player->GetDistance2d(controller) <= 10.0f)
            {
                player->SendUpdateWorldState(WorldStates::BG_SP_SHOW_BAR, true, false);
                player->SendUpdateWorldState(WorldStates::BG_SP_BAR_STATUS, m_score, false);

                changeAmount += (player->GetBGTeamId() == TEAM_ALLIANCE ? 1 : -1);
            }
            else
                player->SendUpdateWorldState(WorldStates::BG_SP_SHOW_BAR, false, false);

    m_pointUpdateTimer = 0;

    if (changeAmount == 0) // without changes
        return;

    m_score += 4 * (changeAmount < 0 ? -1 : 1);
    if (m_score < 0)
        m_score = 0;
    else if (m_score > 100)
        m_score = 100;
}

void BrawlBattlegroundShadoPan::UpdateBossesAndController(uint32 diff)
{
    m_bossessUpdateTimer += diff;
    if (m_bossessUpdateTimer < 1000)
        return;

    m_bossessUpdateTimer = 0;

    for (uint8 i = SP_BOSS_FOR_GOLD; i <= SP_BOSS_FOR_PURPLE; ++i)
        if (Creature* boss = GetBGCreature(i))
        {
            UpdateWorldState(BG_SP_PURPLE_BOSS + i % 2, boss->GetHealthPct(), false);
            m_TeamScores[i % 2] = 100 - boss->GetHealthPct();
        }

    if (m_score == 50)
    {
        for (uint8 i = SP_BOSS_FOR_GOLD; i <= SP_BOSS_FOR_PURPLE; ++i)
            if (Creature* boss = GetBGCreature(i))
                if (!boss->HasAura(SPELL_SWITCH_OFF_BOSS))
                    boss->CastSpell(boss, SPELL_SWITCH_OFF_BOSS, true);

        if (Creature* controller = GetBGCreature(SP_CONTROLLER))
            controller->CastSpell(controller, SPELL_NEUTRAL_TEAM_CONTROL, true);

        return;
    }

    uint8 bosssForFreeze = SP_BOSS_FOR_GOLD;
    uint8 bossForUnFreeze = SP_BOSS_FOR_PURPLE;
    if (m_score > 50) // gold
        std::swap(bossForUnFreeze, bosssForFreeze);

    if (Creature* controller = GetBGCreature(SP_CONTROLLER))
        controller->CastSpell(controller, m_score > 50 ? SPELL_GOLD_TEAM_CONTROL : SPELL_PURPLE_TEAM_CONTROL, true);

    if (Creature* boss = GetBGCreature(bosssForFreeze))
        if (!boss->HasAura(SPELL_SWITCH_OFF_BOSS))
            boss->CastSpell(boss, SPELL_SWITCH_OFF_BOSS, true);

    if (Creature* boss = GetBGCreature(bossForUnFreeze))
        if (boss->HasAura(SPELL_SWITCH_OFF_BOSS))
            boss->RemoveAurasDueToSpell(SPELL_SWITCH_OFF_BOSS);
}

enum eEvents
{
    EVENT_BRUTAL_SLASH = 1,
    EVENT_STORM,
    EVENT_HEAL,
};

enum eSpells
{
    SPELL_BRUTAL_SLASH  = 236973,
    SPELL_THUNDERSTORM  = 252571,
    SPELL_FIRESTORM     = 236984,
    SPELL_HEAL          = 236982,
    SPELL_INCREASE_DMG  = 236983,
};

// 119194 122183
struct npc_bg_shado_pan_boss : ScriptedAI
{
    npc_bg_shado_pan_boss(Creature* creature) : ScriptedAI(creature) 
    {
        me->setRegeneratingHealth(false);
    }

    void Reset() override  {} // do nothing
    void EnterEvadeMode() override  {} // do nothing

    void EnterCombat(Unit* ) override
    {
        events.ScheduleEvent(EVENT_BRUTAL_SLASH, 3000);
        events.ScheduleEvent(EVENT_STORM, 8000);
        events.ScheduleEvent(EVENT_HEAL, 20000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
    {
        if (me->GetHealthPct() <= nextPctForBuff)
        {
            me->CastSpell(me, SPELL_INCREASE_DMG, true);
            nextPctForBuff -= 5;
        }
    }

    void JustDied(Unit* who) override
    {
        if (auto bg = me->GetBattleground())
            bg->EndBattleground(me->GetEntry() == SP_BOSS_FOR_PURPLE_ENTRY ? HORDE : ALLIANCE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() || me->HasAura(SPELL_SWITCH_OFF_BOSS))
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_BRUTAL_SLASH:
                DoCastVictim(SPELL_BRUTAL_SLASH);
                events.ScheduleEvent(EVENT_BRUTAL_SLASH, urand(11, 14)*1000);
                break;
            case EVENT_STORM:
                DoCast(me->GetEntry() == SP_BOSS_FOR_PURPLE_ENTRY ? SPELL_FIRESTORM : SPELL_THUNDERSTORM);
                events.ScheduleEvent(EVENT_STORM, urand(11, 14) * 1000);
                break;
            case EVENT_HEAL:
                if (me->GetHealthPct() <= 50)
                    me->CastSpell(me, SPELL_HEAL);
                events.ScheduleEvent(EVENT_HEAL, 20000);
                break;
            }
        }
    }

private:
    EventMap events{};
    int8 nextPctForBuff = 95;
};


void AddSC_battleground_shado_pan()
{
    RegisterCreatureAI(npc_bg_shado_pan_boss);
}