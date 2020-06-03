
#include "BattlegroundSeethingShore.h"
#include "BattlegroundPackets.h"
#include "WorldStatePackets.h"
#include "SpellPackets.h"
#include "SpellScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"
#include "GameObjectPackets.h"
#include "PointMovementGenerator.h"
#include "CreatureTextMgr.h"

namespace GameObjects
{
    enum
    {
        Azerite = 281307, // summoned by AzeriteFissure via spell at caster pos
        Azerite2 = 272471,
        HordeTransport = 279254,
        HordeAirshipPrepCollision = 281226,
        AllianceTransport = 278407,
        AllianceAirshipPrepCollision = 281224,
    };

    Position const HordeTransportPos = { 1399.684f, 2766.317f, 3.485144f };
    Position const AllianceTransportPos = { 1399.684f, 2766.317f, 3.485144f };
}

namespace Creatures
{
    enum
    {
        Controller = 125269,
        AzeriteFissure = 125253,
        VignetteDummy = 129344, // summoned by Spells::AzeriteGeyser
        BuffBox         = 133532,
    };

    Position const ControllerPos = { 1399.684f, 2766.317f, 3.485144f };

    Position const AzeriteFissurePositions[] =
    {
        { 1113.924f, 2886.993f, 38.53647f },
        { 1126.649f, 2781.239f, 31.00365f },
        { 1243.155f, 2721.465f, 11.96428f },
        { 1257.406f, 2882.731f, 27.66519f },
        { 1259.161f, 2571.423f, 8.653152f },
        { 1339.556f, 2785.681f, 2.665191f },
        { 1346.759f, 2920.885f, 33.31765f },
        { 1361.733f, 2643.267f, 4.608177f },
        { 1390.245f, 2570.769f, 7.360876f },
        { 1441.059f, 2700.151f, 9.876933f },
        { 1454.085f, 2598.380f, 15.35741f },
        { 1461.358f, 2823.238f, 31.80409f },
    };
}

namespace Scores
{
    enum
    {
        NearVictory = 1200,
        MaxTeamScore = 1500
    };
}

namespace Misc
{
    static uint32 GetTeamWorldSafeLocks(TeamId teamId)
    {
        switch (teamId)
        {
        case TEAM_ALLIANCE:
            return 6337;
        case TEAM_HORDE:
            return 6380;
        default:
            return 0;
        }
    }
}

namespace Objects
{
    enum
    {
        ObjectsMax,

        AzeriteFissure = 0,
        NpcController = 13,

        CreaturesMax
    };
}

namespace Spells
{
    enum : uint32
    {
        ActivateAzerite = 248688,
        SummonAzeriteCaptureNode = 262749, // 08:53:49.031                 summon GameObjects::Azerite and casted by Creatures::AzeriteFissure
        SummonAzeriteCaptureNode2 = 248674, // 08:53:48.703                summon GameObjects::Azerite2 and casted by Creatures::AzeriteFissure
        AzeriteKnockBack = 262385,
        UnchartedTerritory = 262086, // too far away
        RocketParachute = 262359,
        RocketParachite2 = 250917,
        RocketParachute3 = 250921,
        AchivementProgress = 261953,
        AchivementProgress2 = 261968,
        EarthquakeCameraShake = 248719, // on Azerite spawn - after AzeritePreSpawn despawn
        AzeriteGeyser = 248668,
        AzeriteCapturedAllianceCosmeticFX = 262508,
        AzeriteCapturedHordeCosmeticFX = 262512,
    };
}

namespace WorldStatesBG
{
    enum
    {
        AllianceTotalScore = 13844,
        HordeTotalScrore = 13845,
        AllianceEarnedScore = 13855,
        HordeEarnedScore = 13856,
        MaxScores = 13846
    };
}

BattlegoundSeethingShoreScore::BattlegoundSeethingShoreScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team) { }

void BattlegoundSeethingShoreScore::UpdateScore(uint32 type, uint32 value)
{
    switch (type)
    {
    case SCORE_BASES_ASSAULTED:
        Captures += value;
        break;
    default:
        BattlegroundScore::UpdateScore(type, value);
        break;
    }
}

void BattlegoundSeethingShoreScore::BuildObjectivesBlock(std::vector<int32>& stats)
{
    stats.push_back(Captures);
}

BattlegroundSeethingShore::BattlegroundSeethingShore() : _gunship{}, _spawnTimer{ 0 }, _startDelay{ false }
{
    BgObjects.resize(Objects::ObjectsMax);
    BgCreatures.resize(Objects::CreaturesMax);

    _isInformedNearVictory = false;
    _spawnTimerDelay = 90 * IN_MILLISECONDS;
    _activeAzerriteFissureCounter = 0;

    for (uint8 i = 0; i < 2; ++i)
        m_safeLocs[i] = new WorldSafeLocsEntry();
}

BattlegroundSeethingShore::~BattlegroundSeethingShore()
{
    for (uint8 i = 0; i < 2; ++i)
        delete m_safeLocs[i];
}

uint32 BattlegroundSeethingShore::GetMaxScore() const
{
    return Scores::MaxTeamScore;
}

void BattlegroundSeethingShore::PostUpdateImpl(uint32 diff)
{
    if (m_introState < m_eventTimers.size())
    {
        if (m_eventTimers[m_introState] <= diff)
        {
            for (auto& guid : m_capitans)
                if (Creature* cre = GetBgMap()->GetCreature(guid))
                    cre->AI()->DoAction(m_introState);

            ++m_introState;
        }
        else
            m_eventTimers[m_introState] -= diff;
    }

    _spawnTimer += diff;
    if (_spawnTimer >= _spawnTimerDelay)
    {
        _spawnTimer = 0;
        ActivateRandomAzeriteFissure();
    }

    if (GetStatus() == STATUS_IN_PROGRESS)
        for (uint8 i = 0; i < 2; ++i)
            if (m_boxesTimers[i] <= diff)
            {
                m_boxesTimers[i] = urand(180, 300) * 1000;

                if (auto ship = _gunship[i])
                {
                    Position pos(ship->GetPositionX(), ship->GetPositionY(), ship->GetPositionZ() - 40.0f);
                    Position pos2 = (_gunship[i == 0 ? 1 : 0]->GetPosition());
                    pos.SimplePosXYRelocationByAngle(pos, 65.0f, pos.GetAngle(&pos2), true);
                    GetBgMap()->SummonCreature(Creatures::BuffBox, pos);
                }

                if (Creature* capitan = GetBgMap()->GetCreature(m_capitans[i]))
                    capitan->AI()->DoAction(5);
            }
            else
                m_boxesTimers[i] -= diff;
}

void BattlegroundSeethingShore::TeleportToStart(Player * player)
{
    float x = 0, y = 0, z = 0;
    if (auto gunship = sTransportMgr->GetTransport(GetBgMap(), player->GetBGTeamId() == TEAM_HORDE ? 279254 : 278407))
    {
        gunship->CalculatePassengerPosition(x, y, z);
        player->TeleportTo(1803, x, y, z + (player->GetBGTeamId() == TEAM_ALLIANCE ? 25.0f : 40.0f), 0.f);
    }
}

WorldSafeLocsEntry const * BattlegroundSeethingShore::GetClosestGraveYard(Player * player)
{
    m_safeLocs[player->GetBGTeamId()]->MapID = 1803;
    float x = 0, y = 0, z = 0;
    if (auto gunship = _gunship[player->GetBGTeamId()])
        gunship->CalculatePassengerPosition(x, y, z);
    else
        return nullptr;

    m_safeLocs[player->GetBGTeamId()]->Loc = DBCPosition4D(x, y, z + (player->GetBGTeamId() == TEAM_ALLIANCE ? 25.0f : 40.0f), 0.0f);

    return m_safeLocs[player->GetBGTeamId()];
}

void BattlegroundSeethingShore::HandlePlayerResurrect(Player * player)
{
    player->CastSpell(player, Spells::UnchartedTerritory);
    player->CastSpell(player, Spells::RocketParachute);
    player->CastSpell(player, Spells::RocketParachite2);

    if (Creature* cap = GetBgMap()->GetCreature(m_capitans[player->GetBGTeamId()]))
        cap->AI()->DoAction(9);
}

void BattlegroundSeethingShore::_CheckPositions(uint32 diff)
{
    if (GetStatus() != STATUS_WAIT_JOIN)
        return;

    ValidStartPositionTimer += diff;
    if (ValidStartPositionTimer < 1000)
        return;

    ValidStartPositionTimer = 0;

    for (auto const& itr : GetPlayers())
        if (auto player = ObjectAccessor::FindPlayer(itr.first))
            if (player->GetPositionZ() < 317.0f)
                TeleportToStart(player);
}

void BattlegroundSeethingShore::DoAction(uint32 type, ObjectGuid guid)
{
    m_capitans[type] = guid;
}

void BattlegroundSeethingShore::EndBattleground(uint32 winner)
{
    Battleground::EndBattleground(winner);

    if (Creature* cre = GetBgMap()->GetCreature(m_capitans[0]))
        cre->AI()->DoAction(winner == ALLIANCE ? 7 : 8);
    if (Creature* cre = GetBgMap()->GetCreature(m_capitans[1]))
        cre->AI()->DoAction(winner == HORDE ? 7 : 8);
}

void BattlegroundSeethingShore::StartingEventCloseDoors() { }

void BattlegroundSeethingShore::StartingEventOpenDoors()
{
    for (auto gunship : _gunship)
    {
        for (uint32 goID : {GameObjects::HordeAirshipPrepCollision, GameObjects::AllianceAirshipPrepCollision })
            if (auto obj = gunship->FindNearestGameObject(goID, 100.f))
            {
                obj->SetLootState(GO_ACTIVATED);
                obj->SetGoState(GO_STATE_ACTIVE);
            }
    }


    GetBgMap()->ApplyOnEveryPlayer([](Player* player) -> void
    {
        player->CastSpell(player, Spells::RocketParachite2); // timer only 2 minutes, but prepare 2 minutes too
    });
}

void BattlegroundSeethingShore::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);

    PlayerScores[player->GetGUID()] = new BattlegoundSeethingShoreScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(Scores::MaxTeamScore).Write());

    SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);

    player->CastSpell(player, Spells::UnchartedTerritory);
    player->CastSpell(player, Spells::RocketParachute);
    if (GetStatus() == STATUS_IN_PROGRESS)
        player->CastSpell(player, Spells::RocketParachite2);
}

void BattlegroundSeethingShore::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/)
{
    if (!player)
        return;

    player->RemoveAura(Spells::UnchartedTerritory);
    player->RemoveAura(Spells::RocketParachute);
    player->RemoveAura(Spells::RocketParachite2);
}

void BattlegroundSeethingShore::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(13843, 1);
    packet.Worldstates.emplace_back(WorldStatesBG::AllianceTotalScore, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStatesBG::HordeTotalScrore, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStatesBG::AllianceEarnedScore, 1); //@ FUCK BLIZZARD
    packet.Worldstates.emplace_back(WorldStatesBG::HordeEarnedScore, 0); //@ FUCK BLIZZARD
    packet.Worldstates.emplace_back(WorldStatesBG::MaxScores, Scores::MaxTeamScore);
}

void BattlegroundSeethingShore::EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS || !source->IsWithinDistInMap(object, 10))
        return;

    auto point = object->FindNearestCreature(Creatures::AzeriteFissure, 10.f);
    if (!point)
        return;

    auto teamID = source->GetBGTeamId();

    for (auto i = 0; i < std::extent<decltype(Creatures::AzeriteFissurePositions)>::value; ++i)
    {
        auto id = Objects::AzeriteFissure + i;
        if (auto creature = GetBGCreature(id))
            if (creature->IsWithinDistInMap(object, 10))
                _azeriteFissureIds[id] = false;
    }

    UpdateCapturePoint(NODE_STATUS_CAPTURE, teamID, object, source, false, true);
    --_activeAzerriteFissureCounter;

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    object->SetAnimKitId(2560, true);

    RewardHonorToTeam(30, MS::Battlegrounds::GetTeamByTeamId(teamID));
    UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 100);

    auto& teamScores = m_TeamScores[teamID];
    teamScores += 100;

    SendBattleGroundPoints(teamID != TEAM_ALLIANCE, teamScores);
    UpdateWorldState(teamID == TEAM_ALLIANCE ? WorldStatesBG::AllianceTotalScore : WorldStatesBG::HordeTotalScrore, teamScores);
    UpdateWorldState(WorldStatesBG::HordeEarnedScore/*teamID == TEAM_ALLIANCE ? WorldStatesBG::AllianceEarnedScore : WorldStatesBG::HordeEarnedScore*/, 100); //@ FUCK BLIZZARD

    object->CastSpell(source, teamID == TEAM_ALLIANCE ? Spells::AzeriteCapturedAllianceCosmeticFX : Spells::AzeriteCapturedHordeCosmeticFX);
    object->CastSpell(source, Spells::AchivementProgress); // @TODO
    object->CastSpell(source, Spells::AchivementProgress2); // @TODO
    object->Delete();

    if (!_isInformedNearVictory && teamScores > Scores::NearVictory)
    {
        SendBroadcastText(teamID == TEAM_ALLIANCE ? 147342 : 147341, CHAT_MSG_BG_SYSTEM_NEUTRAL);
        PlaySoundToAll(BG_SOUND_NEAR_VICTORY);
        _isInformedNearVictory = true;
    }

    if (Creature* capitain = GetBgMap()->GetCreature(m_capitans[teamID]))
        capitain->AI()->DoAction(10);

    if (teamScores >= Scores::MaxTeamScore)
        EndBattleground(MS::Battlegrounds::GetTeamByTeamId(teamID));
}

bool BattlegroundSeethingShore::SetupBattleground()
{
    if (!AddCreature(Creatures::Controller, Objects::NpcController, TEAM_NEUTRAL, Creatures::ControllerPos))
        return false;

    for (auto i = 0; i < std::extent<decltype(Creatures::AzeriteFissurePositions)>::value; ++i)
    {
        auto id = Objects::AzeriteFissure + i;
        _azeriteFissureIds.insert(std::make_pair(id, false));
        if (!AddCreature(Creatures::AzeriteFissure, id, TEAM_NEUTRAL, Creatures::AzeriteFissurePositions[i]))
            return false;
    }

    _gunship[TEAM_HORDE] = sTransportMgr->GetTransport(GetBgMap(), GameObjects::HordeTransport);
    _gunship[TEAM_ALLIANCE] = sTransportMgr->GetTransport(GetBgMap(), GameObjects::AllianceTransport);

    for (auto gunship : _gunship)
        if (!gunship)
            return false;

    return true;
}

void BattlegroundSeethingShore::Reset()
{
    Battleground::Reset();

    _isInformedNearVictory = false;

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        m_TeamScores[i] = 0;

    _spawnTimerDelay = 30 * IN_MILLISECONDS;
    _spawnTimer = 0;
    _startDelay = false;
    _activeAzerriteFissureCounter = 0;
}

void BattlegroundSeethingShore::RelocateDeadPlayers(ObjectGuid guideGuid)
{
    auto& ghostList = m_ReviveQueue[guideGuid];

    for (auto const& guid : ghostList)
    {
        if (auto player = ObjectAccessor::FindPlayer(GetBgMap(), guid))
        {
            TeleportToStart(player);
        }
    }

    ghostList.clear();
}

bool BattlegroundSeethingShore::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
    case SCORE_BASES_ASSAULTED:
        player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, 0, 1);
        break;
    default:
        break;
    }
    return true;
}

void BattlegroundSeethingShore::OnGameObjectCreate(GameObject* object)
{
    if (object->GetEntry() != GameObjects::Azerite2 || object->GetEntry() != GameObjects::Azerite)
        return;

    UpdateCapturePoint(NODE_STATUS_NEUTRAL, TEAM_NONE, object, nullptr, true);

    object->AddDelayedEvent(3000, [object]()
    {
        object->SendCustomAnim(1, false);
    });
}

void BattlegroundSeethingShore::OnCreatureRemove(Creature* creature)
{
    if (creature->GetEntry() != Creatures::VignetteDummy)
        return;

    auto azeriteFissure = creature->FindNearestCreature(Creatures::AzeriteFissure, 10.f);
    if (!azeriteFissure)
        return;

    creature->PlaySpellVisual({}, 74145, 0.f, azeriteFissure->GetGUID());

    auto randNode = { Spells::SummonAzeriteCaptureNode, Spells::SummonAzeriteCaptureNode2 };
    azeriteFissure->CastSpell(azeriteFissure, Trinity::Containers::SelectRandomContainerElement(randNode), true);
    azeriteFissure->CastSpell(azeriteFissure, Spells::AzeriteKnockBack, true);
}

void BattlegroundSeethingShore::CastActivates(Creature* controller)
{
    auto id = 0u;
    while (!id)
    {
        auto fissure = Trinity::Containers::SelectRandomContainerElement(_azeriteFissureIds);
        if (fissure.second == false)
            id = fissure.first;
    }

    auto azeriteFissure = GetBGCreature(id);
    if (!azeriteFissure)
        return;

    if (_activeAzerriteFissureCounter < 3)
    {
        _activeAzerriteFissureCounter++;
        _spawnTimerDelay = 10 * IN_MILLISECONDS;
    }
    else
        return;

    _azeritesToActivate.emplace_back(azeriteFissure->GetGUID());

    _azeriteFissureIds[id] = true;
    controller->CastSpell(controller, Spells::ActivateAzerite, true);
    SendBroadcastText(136038, CHAT_MSG_BG_SYSTEM_NEUTRAL); // seems not blizzlike

    for (auto& guid : m_capitans)
        if (Creature* cre = GetBgMap()->GetCreature(guid))
            cre->AI()->DoAction(11);
}

void BattlegroundSeethingShore::ActivateRandomAzeriteFissure()
{
    auto controller = GetBGCreature(Objects::NpcController);

    if (!controller || _activeAzerriteFissureCounter >= 3)
        return;

    if (!_startDelay)
    {
        _startDelay = true;
        for (auto i = 0; i < 3; i++)
        {
            controller->AddDelayedEvent(uint64(500 * (i + 1)), [this, controller]()
            {
                CastActivates(controller);
            });
        }
    }

    CastActivates(controller);
}

// Spells::ActivateAzerite 248688
class spell_bg_seething_shore_active_azerite : public SpellScript
{
    PrepareSpellScript(spell_bg_seething_shore_active_azerite);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (auto target = GetHitUnit())
            target->CastSpell(target, Spells::AzeriteGeyser, true);
    }

    void FilterTargetsDamage(std::list<WorldObject*>& targets)
    {
        auto caster = GetCaster();
        if (!caster || !caster->IsCreature())
        {
            targets.clear();
            return;
        }

        auto bg = caster->ToCreature()->GetBattleground();
        if (!bg)
        {
            targets.clear();
            return;
        }

        auto& data = dynamic_cast<BattlegroundSeethingShore*>(bg)->_azeritesToActivate;
        for (auto itr : data)
        {
            auto azeriteToActivate = caster->GetMap()->GetCreature(itr);
            targets.remove_if([azeriteToActivate](WorldObject* obj) -> bool
            {
                if (!obj || !obj->IsCreature())
                    return true;

                auto creature = obj->ToCreature();
                if (creature->GetEntry() != Creatures::AzeriteFissure)
                    return true;

                if (creature->GetDistance2d(azeriteToActivate) > 15.f)
                    return true;

                return false;
            });
        }

        data.clear();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_bg_seething_shore_active_azerite::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_bg_seething_shore_active_azerite::FilterTargetsDamage, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// Spells::RocketParachute2 250917
class spell_bg_seething_shore_rocket_parachute2 : public AuraScript
{
    PrepareAuraScript(spell_bg_seething_shore_rocket_parachute2);

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        auto player = caster->ToPlayer();
        if (/*!player->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_FAR) || */player->HasAura(Spells::RocketParachute3) || player->GetPositionZ() <=120.0f || player->GetPositionZ() >= 272.40f)
            return;

        player->CastSpell(player, Spells::RocketParachute3, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_bg_seething_shore_rocket_parachute2::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Spells::RocketParachute3 250921
class spell_bg_seething_shore_rocket_parachute3 : public AuraScript
{
    PrepareAuraScript(spell_bg_seething_shore_rocket_parachute3);

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        auto caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return;

        auto player = caster->ToPlayer();
        if (player->GetDistanceToZOnfall() > 30.f || player->IsInWater())
            return;

        player->RemoveAura(Spells::RocketParachute3);
        player->CastSpell(player, 252766, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_bg_seething_shore_rocket_parachute3::OnPeriodic, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 130532 131773
struct npc_bgss_capitains : ScriptedAI
{
    npc_bgss_capitains(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        SetCanSeeEvenInPassiveMode(true);
    }

    bool firstAzeritDone = false;
    uint32 textCooldown = 0;
    uint64 lastTimeWas = 0;
    uint64 lastTimeWasRes = 0;

    void InitializeAI() override
    {
        if (Battleground* bg = me->GetBattleground())
            bg->DoAction(me->GetEntry() == 131773, me->GetGUID());
    }

    void DoAction(int32 const action) override
    {
        ObjectGuid target{};
        if (Group* group = me->GetBattleground()->GetBgRaid(me->GetEntry() == 131773))
            target = group->GetLeaderGUID();

        switch (action)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 5:
        case 7:
        case 8:
            sCreatureTextMgr->SendChat(me, action, target, CHAT_MSG_ADDON, LANG_ADDON,TEXT_RANGE_MAP, 0, me->GetEntry() == 131773 ? HORDE : ALLIANCE);
            break;
        case 10: // first getting azerit
            if (firstAzeritDone)
                return;

            firstAzeritDone = true;

            sCreatureTextMgr->SendChat(me, 4, target, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_MAP, 0, me->GetEntry() == 131773 ? HORDE : ALLIANCE);
            lastTimeWas = time(NULL);
            textCooldown = urand(20, 40);
            break;
        case 11: // sometimes tell about new azerit
            if (!firstAzeritDone)
                return;

            if (time(NULL) - lastTimeWas < textCooldown)
                return;

            sCreatureTextMgr->SendChat(me, urand(0, 1) == 1 ? 6 : 4, target, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_MAP, 0, me->GetEntry() == 131773 ? HORDE : ALLIANCE);
            lastTimeWas = time(NULL);
            textCooldown = urand(30, 80);
            break;
        case 9: // resurrect
            if (time(NULL) - lastTimeWasRes < 10)
                return;

            lastTimeWasRes = time(NULL);
            sCreatureTextMgr->SendChat(me, 9, target, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_NORMAL, 0, me->GetEntry() == 131773 ? HORDE : ALLIANCE);
            break;
        }
    }
};

// 133532
struct npc_bgss_buff_box : ScriptedAI
{
    npc_bgss_buff_box(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        SetCanSeeEvenInPassiveMode(true);
    }

    void InitializeAI() override
    {
        me->AddDelayedEvent(2000, [this]() -> void
        {
            me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetWaterOrGroundLevel(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()) + 15.0f);
        });
    }

    void MovementInform(uint32 type, uint32 point) override
    {
        if (point == 1)
        {
            std::array<uint32, 3> m_buffs{ 206564 , 206565 , 206566 };

            if (auto go = me->SummonGameObject(m_buffs[urand(0, 2)], me->GetPositionX(), me->GetPositionY(), me->GetWaterOrGroundLevel(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()), 0, 0, 0, 0, 0, RESPAWN_ONE_DAY))
                go->SetLootState(GO_READY);
            me->DespawnOrUnsummon();
        }
    }
};

// 131149
struct npc_bgss_visual_npcs : ScriptedAI
{
    npc_bgss_visual_npcs(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        SetCanSeeEvenInPassiveMode(true);
    }

    void InitializeAI() override
    {
        if (!me->GetAnyOwner())
        {
            for (uint8 i = 0; i < 2; ++i)
                if (Creature* cre = me->SummonCreature(me->GetEntry()))
                    cre->GetMotionMaster()->MoveFollow(me, 7.0f, PET_FOLLOW_ANGLE*(i == 0 ? 1 : -1));

            me->GetMotionMaster()->MoveRandom(40.0f);
        }
    }

    void MovementInform(uint32 type, uint32 point) override
    {
        if (type != RANDOM_MOTION_TYPE)
            return;

        if (me->GetAnyOwner())
            return;

        me->GetMotionMaster()->MoveRandom(40.0f);
      
    }
};


void AddSC_battleground_seething_shore()
{
    RegisterSpellScript(spell_bg_seething_shore_active_azerite);
    RegisterAuraScript(spell_bg_seething_shore_rocket_parachute2);
    RegisterAuraScript(spell_bg_seething_shore_rocket_parachute3);
    RegisterCreatureAI(npc_bgss_capitains);
    RegisterCreatureAI(npc_bgss_buff_box);
    RegisterCreatureAI(npc_bgss_visual_npcs);
}
