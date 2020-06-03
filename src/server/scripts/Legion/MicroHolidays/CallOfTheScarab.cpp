/*
*/

#include "OutdoorPvP.h"
#include "Packets/WorldStatePackets.h"
#include "World.h"
#include "PrecompiledHeaders/ScriptPCH.h"

enum Misc
{
    EVENT_ACTIVE                = 1,
    EVENT_HORDE_FLAG            = 76,
    EVENT_ALLIANCE_FLAG         = 77,
    EVENT_CALL_OF_THE_SCARAB    = 78,
    EVENT_WINNER_TEAM           = 306
};

class OutdoorPvPSilithus : public OutdoorPvP
{
public:
    OutdoorPvPSilithus()
    {
        m_TypeId = OUTDOOR_PVP_SILITHUS;
    }

    ~OutdoorPvPSilithus() = default;

    bool SetupOutdoorPvP() override
    {
        for (auto zone : { 5695, 1377 })
            RegisterZone(zone);

        if (!sGameEventMgr->IsActiveEvent(EVENT_CALL_OF_THE_SCARAB))
        {
            sWorld->setWorldState(WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE, 0);
            sWorld->setWorldState(WS_SCORE_CALL_OF_THE_SCARAB_HORDE, 0);
            CharacterDatabase.PExecute("DELETE FROM character_currency WHERE currency in (1325,1324);");
            CharacterDatabase.PExecute("DELETE FROM character_queststatus_rewarded WHERE quest in (45785,45787);");
            CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE quest in (45785,45787);");
        }

        return true;
    }

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override
    {
        packet.Worldstates.emplace_back(static_cast<WorldStates>(12953), AllianceScore);
        packet.Worldstates.emplace_back(static_cast<WorldStates>(12952), HordeScore);
    }

    void HandlePlayerEnterZone(ObjectGuid guid, uint32 zone) override
    {
        OutdoorPvP::HandlePlayerEnterZone(guid, zone);

        if (m_stage != EVENT_ACTIVE)
            return;

        Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
        if (!player)
            return;

        AllianceScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE);
        HordeScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_HORDE);

        player->SendUpdateWorldState(static_cast<WorldStates>(12952), HordeScore);
        player->SendUpdateWorldState(static_cast<WorldStates>(12953), AllianceScore);
    }

    void HandlePlayerLeaveZone(ObjectGuid guid, uint32 zone) override
    {
        OutdoorPvP::HandlePlayerLeaveZone(guid, zone);

        Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
        if (!player)
            return;

        player->SendUpdateWorldState(static_cast<WorldStates>(12952), 0);
        player->SendUpdateWorldState(static_cast<WorldStates>(12953), 0);
    }

    bool Update(uint32 diff) override
    {
        if (update_worldstate <= diff)
        {
            update_worldstate = 2000;

            ApplyOnEveryPlayerInZone([this](Player* player) -> void
            {
                AllianceScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE);
                HordeScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_HORDE);

                if (AllianceScore > CurrectAllianceScore)
                {
                    CurrectAllianceScore = AllianceScore;
                    player->SendUpdateWorldState(static_cast<WorldStates>(12953), AllianceScore);
                }

                if (HordeScore > CurrectHordeScore)
                {
                    CurrectHordeScore = AllianceScore;
                    player->SendUpdateWorldState(static_cast<WorldStates>(12952), HordeScore);
                }
            });

            if (!b_winner)
            {
                if (sGameEventMgr->IsActiveEvent(EVENT_WINNER_TEAM))
                {
                    b_winner = true;
                    m_winner = 100;
                }
            }
        }
        else
            update_worldstate -= diff;

        if (m_winner)
        {
            if (m_winner <= diff)
            {
                m_winner = 0;

                AllianceScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE);
                HordeScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_HORDE);

                if (AllianceScore > HordeScore)
                {
                    WorldDatabase.PExecute("UPDATE game_event SET start_time='0000-00-00 00:00:00' WHERE eventEntry = 76");
                    WorldDatabase.PExecute("UPDATE game_event SET start_time=NOW() WHERE eventEntry = 77");

                    if (sGameEventMgr->IsActiveEvent(EVENT_HORDE_FLAG))
                        sGameEventMgr->StopEvent(EVENT_HORDE_FLAG, true);

                    if (!sGameEventMgr->IsActiveEvent(EVENT_ALLIANCE_FLAG))
                        sGameEventMgr->StartEvent(EVENT_ALLIANCE_FLAG, true);
                }
                else if (HordeScore > AllianceScore)
                {
                    WorldDatabase.PExecute("UPDATE game_event SET start_time='0000-00-00 00:00:00' WHERE eventEntry = 77");
                    WorldDatabase.PExecute("UPDATE game_event SET start_time=NOW() WHERE eventEntry = 76");

                    if (sGameEventMgr->IsActiveEvent(EVENT_ALLIANCE_FLAG))
                        sGameEventMgr->StopEvent(EVENT_ALLIANCE_FLAG, true);

                    if (!sGameEventMgr->IsActiveEvent(EVENT_HORDE_FLAG))
                        sGameEventMgr->StartEvent(EVENT_HORDE_FLAG, true);
                }

                if (sGameEventMgr->IsActiveEvent(EVENT_WINNER_TEAM))
                    sGameEventMgr->StopEvent(EVENT_WINNER_TEAM, true);

                CurrectAllianceScore = 0;
                CurrectHordeScore = 0;
                update_worldstate = 0;
                HordeScore = 0;
                AllianceScore = 0;
                sWorld->setWorldState(WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE, 0);
                sWorld->setWorldState(WS_SCORE_CALL_OF_THE_SCARAB_HORDE, 0);
                CharacterDatabase.PExecute("DELETE FROM character_currency WHERE currency in (1325,1324)");
                ApplyOnEveryPlayerInZone([this](Player* player) -> void
                {
                    player->SendUpdateWorldState(static_cast<WorldStates>(12952), 0);
                    player->SendUpdateWorldState(static_cast<WorldStates>(12953), 0);
                });

                if (sGameEventMgr->IsActiveEvent(EVENT_CALL_OF_THE_SCARAB))
                    sGameEventMgr->StopEvent(EVENT_CALL_OF_THE_SCARAB, true);
            }
            else
                m_winner -= diff;
        }

        return true;
    }

    void HandleGameEventStart(uint32 eventId) override
    {
        if (eventId != EVENT_CALL_OF_THE_SCARAB)
            return;

        if (!m_map)
            return;

        m_stage = EVENT_ACTIVE;
        update_worldstate = 2000;
        AllianceScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_ALLINCE);
        HordeScore = sWorld->getWorldState(WS_SCORE_CALL_OF_THE_SCARAB_HORDE);
        CurrectAllianceScore = AllianceScore;
        CurrectHordeScore = CurrectHordeScore;
    }

private:
    uint8 m_stage{};
    uint8 m_winner{};
    uint32 update_worldstate{};
    uint32 CurrectAllianceScore{};
    uint32 CurrectHordeScore{};
    uint32 AllianceScore{};
    uint32 HordeScore{};
    bool b_winner = false;
};

class OutdoorPvP_Silithus : public OutdoorPvPScript
{
public:
    OutdoorPvP_Silithus() : OutdoorPvPScript("outdoorpvp_silithus") {}

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPvPSilithus();
    }
};

enum Creatures
{
    NPC_TEMPLAR_FIRE = 15209,
    NPC_TEMPLAR_WATER = 15211,
    NPC_TEMPLAR_AIR = 15212,
    NPC_TEMPLAR_EARTH = 15307
};

class go_wind_stone : public GameObjectScript
{
public:
    go_wind_stone() : GameObjectScript("go_wind_stone") {}

    void SummonNPC(GameObject* go, Player* player, uint32 npc/*, uint32 spell*/)
    {
        //go->CastSpell(player, spell);
        TempSummon* summons = go->SummonCreature(npc, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), player->GetOrientation() - M_PI, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10 * 60 * 1000);
        summons->CastSpell(summons, 25035, false);
        summons->AI()->Talk(0, player->GetGUID());
        summons->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        summons->SendMeleeAttackStart(player);
        summons->CombatStart(player);
    }

public:
    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (!player->HasAura(234185) || !player->HasAura(234184) || !player->HasAura(234182))
            go->CastSpell(player, 24803);

        return false;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action) override
    {
        uint32 c_aura[3] = { 234185, 234184, 234182 };
        uint32 c_templar[4] = { NPC_TEMPLAR_FIRE, NPC_TEMPLAR_AIR, NPC_TEMPLAR_EARTH, NPC_TEMPLAR_WATER };
        uint32 c_duke[4] = { 117663, 117665, 117662, 117664 };
        uint32 c_royal[4] = { 117666, 117667, 117670, 117672 };

        switch (go->GetEntry())
        {
        case 180529:
        case 180456:
        case 180518:
        case 180544:
        case 180549:
        case 180564:
            player->RemoveAura(c_aura[urand(0, 2)]);
            SummonNPC(go, player, c_templar[urand(0, 3)]/*, 24745*/);
            break;
        case 180534:
        case 180461:
        case 180554:
            player->RemoveAura(c_aura[urand(0, 2)]);
            SummonNPC(go, player, c_duke[urand(0, 3)]/*, 24762*/);
            break;
        case 180466:
        case 180539:
        case 180559:
            player->RemoveAura(c_aura[urand(0, 2)]);
            SummonNPC(go, player, c_royal[urand(0, 3)]/*, 24785*/);
            break;
        default:
            break;
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

//117491,117490,117489
struct npc_sillithis_colossus : public ScriptedAI
{
    npc_sillithis_colossus(Creature* creature) : ScriptedAI(creature) {}

    uint32 cast;

    void Reset() override
    {
        cast = 0;
    }

    void EnterCombat(Unit* /*killer*/) override
    {
        cast = 50000;
    }

    void EnterEvadeMode() override
    {
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (cast)
        {
            if (cast <= diff)
            {
                ZoneTalk(0);
                DoCast(26167);

                cast = 57000;
            }
            else
                cast -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

//29519
class spell_silithyst : public AuraScript
{
    PrepareAuraScript(spell_silithyst);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_INTERRUPT)
            caster->CastSpell(caster, 29533, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_silithyst::OnRemove, EFFECT_0, SPELL_AURA_EFFECT_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_CallOfTheScarab()
{
    //new OutdoorPvP_Silithus();
    new go_wind_stone();
    RegisterCreatureAI(npc_sillithis_colossus);
    RegisterAuraScript(spell_silithyst);
}