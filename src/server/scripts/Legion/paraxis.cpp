#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"

enum Misc
{
    NPC_PARAXIS                         = 127190,
    NPC_SHIELDBEARER                    = 127389,

    GO_HOLY_SHIELD                      = 276297,

    SPELL_PARAXIS_INTRO                 = 255102,
    SPELL_PARAXIS_INFO_UNDER_SHIELD     = 252508,
    SPELL_PARAXIS_INFO_WITHOUT_SHIELD   = 252510,

    SPELL_PARAXIS_TICK                  = 248836,
    SPELL_PARAXIS_COMMING               = 247214,

    SPELL_HOLY_SHIELD                   = 252509,
    SPELL_HOLY_SHIELD_AT                = 252973,

    EVENT_PARAXIS                       = 154,

    AREA_WINDIKAR                       = 8916,
    AREA_SHEILD                         = 9140,

    STAGE_NOT_ACTIVE                    = 0,
    STAGE_INTRO                         = 1,
    STAGE_ACTIVE                        = 2,
};

Position const pos[8]
{
    { -2920.86f, 8788.26f, -242.10f, 0.72f }, // big
    { -2351.72f, 8919.84f, -160.90f, 2.42f }, // big
    { -3484.29f, 8817.26f, -201.37f, 0.39f }, // big
    {-3203.27f, 9060.12f, -169.06f, 3.45f},
    {-3083.32f, 9387.12f, -164.76f, 1.10f},
    {-2460.06f, 9499.05f, -191.67f, 4.20f},
    {-2624.23f, 9097.12f, -137.14f, 4.21f},
    {-3152.38f, 8655.84f, -155.78f, 1.97f}, 
};

class OutdoorPVPParaxis : public OutdoorPvP
{
public:
    OutdoorPVPParaxis()
    {
        m_TypeId = OUTDOOR_PVP_PARAXIS;
    }

    ~OutdoorPVPParaxis() = default;

    bool SetupOutdoorPvP() override
    {
        RegisterZone(8899);
        return true;
    }

    void HandlePlayerEnterZone(ObjectGuid guid, uint32 zone) override
    {
        OutdoorPvP::HandlePlayerEnterZone(guid, zone);

        if (m_stage == STAGE_NOT_ACTIVE)
            return;

        Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
        if (!player)
            return;

        switch (m_stage)
        {
        case STAGE_INTRO:
            player->CastSpell(player, SPELL_PARAXIS_INTRO, true);
            break;
        }
    }


    void HandlePlayerLeaveZone(ObjectGuid guid, uint32 zone) override
    {
        OutdoorPvP::HandlePlayerLeaveZone(guid, zone);

        Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
        if (!player)
            return;

        for (uint32 spell : {SPELL_PARAXIS_INTRO, SPELL_PARAXIS_INFO_UNDER_SHIELD, SPELL_PARAXIS_INFO_WITHOUT_SHIELD})
            player->RemoveAurasDueToSpell(spell);
    }

    void HandleGameEventStart(uint32 eventId) override
    {
        if (eventId != EVENT_PARAXIS)
            return;

        if (!m_map)
            return;

        Creature* paraxis = m_map->SummonCreature(NPC_PARAXIS, Position(-3833.93f, 9150.06f, 297.56f, 5.83f));
        if (!paraxis)
            return;

        m_paraxisGuid = paraxis->GetGUID();
        paraxis->setActive(true);
        paraxis->AI()->ZoneTalk(0);
        paraxis->SetVisible(false);
        ApplyOnEveryPlayerInZone([](Player* player)
        {
            player->CastSpell(player, SPELL_PARAXIS_INTRO, true);
        });
        m_stage = STAGE_INTRO;

        m_introCount = 0;
        m_startTimer = 1000;
        m_needTimer = true;
    }

    void SetData(uint32 type, uint32 data) override
    {
        if (type && data)
        {
            m_stage = STAGE_NOT_ACTIVE;
            m_needTimer = false;
            ApplyOnEveryPlayerInZone([](Player* player)
            {
                for (uint32 spell : {SPELL_PARAXIS_INTRO, SPELL_PARAXIS_INFO_UNDER_SHIELD, SPELL_PARAXIS_INFO_WITHOUT_SHIELD})
                    player->RemoveAurasDueToSpell(spell);
            });

            if (Creature* cre = ObjectAccessor::GetObjectInMap(m_paraxisGuid, m_map, (Creature*)nullptr))
                cre->DespawnOrUnsummon();

            for (auto& guid : m_eventNpcs)
                if (Creature* cre = ObjectAccessor::GetObjectInMap(guid, m_map, (Creature*)nullptr))
                    cre->DespawnOrUnsummon();
            
            bool first = true;
            for (auto& guid : m_eventGO)
                if (GameObject* go = ObjectAccessor::GetObjectInMap(guid, m_map, (GameObject*)nullptr))
                {
                    if (first)
                    {
                        std::list<Creature*> casters;
                        GetCreatureListWithEntryInGrid(casters, go, 126048, 70.0f);
                        GetCreatureListWithEntryInGrid(casters, go, 126049, 70.0f);
                        GetCreatureListWithEntryInGrid(casters, go, 126044, 70.0f);

                        for (auto cre : casters)
                            cre->CastStop();

                        first = false;
                    }
                    go->SetRespawnTime(0);                                 // not save respawn time
                    go->Delete();
                }

            m_eventNpcs.clear();
            m_eventGO.clear();
            m_paraxisGuid = ObjectGuid::Empty;
        }
    }

    bool Update(uint32 diff) override
    {
        if (!m_needTimer)
            return true;

        if (m_startTimer <= diff)
        {
            switch (m_stage)
            {
            case STAGE_INTRO:
            {
                uint16 maxTimer = 180;
                if (m_introCount == maxTimer - 30)
                {
                    for (uint8 i = 0; i < 8; ++i)
                    {
                        if (i < 3)
                        {
                            if (GameObject* go = m_map->SummonGameObject(GO_HOLY_SHIELD, pos[i].GetPositionX(), pos[i].GetPositionY(), pos[i].GetPositionZ(), 0.0f, 0, 0, 0, 0, RESPAWN_ONE_DAY))
                            {
                                m_eventGO.push_back(go->GetGUID());
                                go->setActive(true);

                                if (i == 0)
                                {
                                    std::list<Creature*> casters;
                                    GetCreatureListWithEntryInGrid(casters, go, 126048, 70.0f);
                                    GetCreatureListWithEntryInGrid(casters, go, 126049, 70.0f);
                                    GetCreatureListWithEntryInGrid(casters, go, 126044, 70.0f);

                                    for (auto cre : casters)
                                        if (cre->isAlive())
                                            cre->CastSpell(cre, 252832);
                                }
                            }
                        }
                        else if (Creature* cre = m_map->SummonCreature(NPC_SHIELDBEARER, pos[i]))
                        {
                            cre->SetReactState(REACT_PASSIVE);
                            cre->CastSpell(cre, SPELL_HOLY_SHIELD_AT);
                            cre->setActive(true);
                            m_eventNpcs.push_back(cre->GetGUID());
                        }
                    }
                }

                if (++m_introCount <= maxTimer)
                {
                    ApplyOnEveryPlayerInZone([](Player* player)
                    {
                        if (!player->HasAura(SPELL_PARAXIS_INTRO))
                            player->CastSpell(player, SPELL_PARAXIS_INTRO, true);
                    });
                }
                else
                {
                    m_introCount = 0;
                    m_stage = STAGE_ACTIVE;
                    if (Creature* paraxis = ObjectAccessor::GetObjectInMap(m_paraxisGuid, m_map, (Creature*)nullptr))
                    {
                        paraxis->SetVisible(true);
                        paraxis->AI()->ZoneTalk(1);
                        paraxis->GetMotionMaster()->MovePath(12961247, false);
                        paraxis->CastSpell(paraxis, SPELL_PARAXIS_COMMING);
                    }
                }
                break;
            }
            case STAGE_ACTIVE:
                if (++m_introCount <= 300)
                {
                    ApplyOnEveryPlayerInZone([](Player* player)
                    {
                        if (player->GetCurrentAreaID() == AREA_WINDIKAR || player->HasAura(SPELL_HOLY_SHIELD))
                        {
                            if (!player->HasAura(SPELL_PARAXIS_INFO_UNDER_SHIELD))
                                player->CastSpell(player, SPELL_PARAXIS_INFO_UNDER_SHIELD, true);
                        }
                        else if (!player->HasAura(SPELL_PARAXIS_INFO_WITHOUT_SHIELD))
                            player->CastSpell(player, SPELL_PARAXIS_INFO_WITHOUT_SHIELD, true);
                    });
                }
                else
                    SetData(1, 1);
                            
                break;
            }

            m_startTimer = 1000;
        }
        else
            m_startTimer -= diff;
        return true;
    }

public:
    ObjectGuid m_paraxisGuid{};
    uint32 m_startTimer{};
    bool m_needTimer = false;
    uint8 m_stage = STAGE_NOT_ACTIVE;
    uint16 m_introCount = 0;
    std::vector<ObjectGuid> m_eventNpcs{};
    std::vector<ObjectGuid> m_eventGO{};
};

class OutdoorPvP_Paraxis : public OutdoorPvPScript
{
public:
    OutdoorPvP_Paraxis() : OutdoorPvPScript("outdoorpvp_paraxis") {}

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPVPParaxis();
    }
};


// 127190
struct npc_paraxis : ScriptedAI
{
    npc_paraxis(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        SetCanSeeEvenInPassiveMode(true);
        me->AddUnitState(UNIT_STATE_MOVE_IN_CASTING);
    }


    void MovementInform(uint32 type, uint32 point) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (point == 0)
        {
            me->RemoveAurasDueToSpell(SPELL_PARAXIS_COMMING);
            me->CastSpell(me, SPELL_PARAXIS_TICK);
        }

        if (point != 4)
            return;

        OutdoorPvP* pvp = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetCurrentZoneID());
        if (!pvp)
            return;

        pvp->SetData(1, 1);

        me->DespawnOrUnsummon();
    }
};

// 246313
class spell_paraxis_artillery : public SpellScript
{
    PrepareSpellScript(spell_paraxis_artillery);

    void FilterTargetsDamage(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        OutdoorPvP* pvp = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(caster->GetCurrentZoneID());
        if (!pvp)
            return;

        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
            for (auto& guid : pvp->GetPlayers(i))
                if (Player* player = ObjectAccessor::GetObjectInMap(guid, caster->GetMap(), (Player*)nullptr))
                    targets.push_back(player);

        targets.remove_if([](WorldObject* obj)
        {
            if (!obj->IsPlayer())
                return true;

            Player* player = obj->ToPlayer();


            return player->GetCurrentAreaID() == AREA_WINDIKAR || player->GetCurrentZoneID() != 8899 || !player->isAlive() || player->HasAura(SPELL_HOLY_SHIELD);
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_paraxis_artillery::FilterTargetsDamage, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

void Addsc_paraxis()
{
    new OutdoorPvP_Paraxis();
    RegisterCreatureAI(npc_paraxis);
    RegisterSpellScript(spell_paraxis_artillery);
}