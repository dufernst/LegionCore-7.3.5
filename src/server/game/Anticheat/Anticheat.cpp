#include "Common.h"
#include "DatabaseEnv.h"
#include "Anticheat.h"
#include "Chat.h"
#include "Player.h"
#include "GameObject.h"
#include "WorldSession.h"
#include "World.h"
#include "Language.h"
#include "Opcodes.h"
#include "MoveSpline.h"
#include "UnitDefines.h"

const char* GetCheatTypeNameFromFlag(CheatType flagId)
{
    switch (flagId)
    {
        case CHEAT_TYPE_WALL_CLIMB:
            return "WallClimb";
        case CHEAT_TYPE_WATER_WALK:
            return "WaterWalk";
        case CHEAT_TYPE_FORBIDDEN:
            return "AccessForbidden";
        case CHEAT_TYPE_BG_NOT_STARTED:
            return "BgNotStarted";
        case CHEAT_TYPE_MULTIJUMP:
            return "MultiJump";
        case CHEAT_TYPE_FALL_UP:
            return "FakeFall";
        case CHEAT_TYPE_UNREACHABLE:
            return "Unreachable";
        case CHEAT_TYPE_TIME_BACK:
            return "ReverseTime";
        case CHEAT_TYPE_OVERSPEED_JUMP:
            return "OverspeedJump";
        case CHEAT_TYPE_JUMP_SPEED_CHANGE:
            return "JumpSpeedChange";
        case CHEAT_TYPE_FLY_HACK_SWIM:
            return "FlyHackSwim";
        case CHEAT_TYPE_ROOT_MOVE:
            return "MovementRooted";
        case CHEAT_TYPE_ROOT_IGNORED:
            return "Unstunnable";
        case CHEAT_TYPE_TELEPORT_HACK:
            return "TeleportHack";
        case CHEAT_TYPE_DESYNC_TIME:
            return "DesyncTime";
        case CHEAT_TYPE_EXPLORE:
            return "Exploration";
        case CHEAT_TYPE_EXPLORE_HIGH_LEVEL:
            return "ExploreHighLevelArea";
        case CHEAT_TYPE_OVERSPEED_Z:
            return "OverspeedZ";
        case CHEAT_TYPE_SKIPPED_HEARTBEATS:
            return "SkippedHeartbeats";
        case CHEAT_TYPE_NUM_DESYNC:
            return "NumDesyncs";
        case CHEAT_TYPE_FAKE_TRANSPORT:
            return "FakeTransport";
        case CHEAT_TYPE_TELE_TO_TRANSPORT:
            return "TeleToTransport";
        case CHEAT_TYPE_SPEED_HACK_ALERTS:
            return "SpeedHackAlerts";
        case CHEAT_TYPE_FLY_HACK:
            return "FlyHack";
        case CHEAT_TYPE_SUPERJUMP:
            return "SuperJump";
        case CHEAT_TYPE_DISABLE_GRAVITY:
            return "DisableGravity";
        default:
            return "UnknownCheat";
    }
}

PlayerCheatsMgr* PlayerCheatsMgr::instance()
{
    static PlayerCheatsMgr instance;
    return &instance;
}

/// PlayerCheatsMgr
void PlayerCheatsMgr::LoadFromDB()
{
    if (!EnableAnticheat())
        return;

    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading table 'cheat_sanctions'");
    _sanctions.clear();

    QueryResult result = WorldDatabase.Query("SELECT cheatType, tickCount, tickAction, totalCount, totalAction, comment FROM cheat_sanctions;");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        CheatSanctions s;
        s.cheatType     = fields[0].GetUInt32();
        s.tickCount     = fields[1].GetUInt32();
        s.tickSanction  = fields[2].GetUInt32();
        s.totalCount    = fields[3].GetUInt32();
        s.totalSanction = fields[4].GetUInt32();
        s.comment       = fields[5].GetString();

        if (s.cheatType >= CHEATS_COUNT)
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "cheat_sanctions has record with invalid cheatType %u > CHEATS_COUNT (%u)", s.cheatType, CHEATS_COUNT);
        else if (s.tickSanction >= CHEAT_MAX_ACTIONS || s.totalSanction >= CHEAT_MAX_ACTIONS)
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "cheat_sanctions has record with invalid action (must be < %u)", CHEAT_MAX_ACTIONS);
        else
            _sanctions.emplace_back(s);
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_GENERAL, ">> %u anticheat checks loaded", _sanctions.size());
}

void PlayerCheatsMgr::LoadConfig()
{
    _enabled            = sWorld->getBoolConfig(CONFIG_ANTICHEAT_ENABLED);
    _antiMultiJump      = sWorld->getBoolConfig(CONFIG_ANTICHEAT_ANTI_MULTI_JUMP_ENABLED);
    _antiSpeedHack      = sWorld->getBoolConfig(CONFIG_ANTICHEAT_ANTI_SPEED_HACK_ENABLED);
    _antiSpeedHackInterp = sWorld->getBoolConfig(CONFIG_ANTICHEAT_USE_INTERPOLATION);
    _antiWallClimb      = sWorld->getBoolConfig(CONFIG_ANTICHEAT_ANTI_WALL_CLIMB_ENABLED);
    _announceCheatMask  = sWorld->getIntConfig(CONFIG_ANTICHEAT_GM_ANNOUNCE_MASK);
    _maxAllowedDesync   = sWorld->getIntConfig(CONFIG_ANTICHEAT_MAX_ALLOWED_DESYNC);
    _notifyCheaters     = sWorld->getBoolConfig(CONFIG_ANTICHEAT_NOTIFY_CHEATERS);
    _logDatas           = sWorld->getBoolConfig(CONFIG_ANTICHEAT_LOG_DATA);
    _logDetails         = sWorld->getBoolConfig(CONFIG_ANTICHEAT_DETAIL_LOG);
}

CheatAction PlayerCheatsMgr::ComputeCheatAction(PlayerCheatData* cheatData, std::stringstream& reason) const
{
    uint32 action = CHEAT_ACTION_NONE;

    for (auto const &s : _sanctions)
    {
        uint32 tickCount = 0;
        uint32 totalCount = 0;

        cheatData->CountCheatOccur(s.cheatType, tickCount, totalCount);

        if (s.tickCount && tickCount >= s.tickCount)
        {
            if (!reason.str().empty())
                reason << "/";
            reason << s.comment << " (Tick: " << tickCount << ")";
            action |= s.tickSanction;
        }

        if (s.totalCount && totalCount >= s.totalCount)
        {
            if (!reason.str().empty())
                reason << "/";
            reason << s.comment << " (Total: " << totalCount << ")";
            action |= s.totalSanction;
        }
    }

    return CheatAction(action);
}

PlayerCheatData* PlayerCheatsMgr::CreateAnticheatFor(Player* player)
{
    auto cd = new PlayerCheatData(player);
    cd->Init();
    return cd;
}

PlayerCheatData::PlayerCheatData(Player* _me) : updateCheckTimer(0), cheatOccuranceTick{}, cheatOccuranceTotal{}, _storeCheatFlags(0), _jumpCount(0),_speedAlertCount(0), _clientDesynchro(0), _jumpInitialSpeed(0.0f),
_jumpInitialVelocity(0.0f), _overspeedDistance(0), _inKnockBack(false), me(_me),_maxOverspeedDistance(0), _maxClientDesynchro(0)
{
    for (auto& clientSpeed : _clientSpeeds)
        clientSpeed = 0.0f;
} 

/// PlayerCheatData
void PlayerCheatData::Init()
{
    memset(cheatOccuranceTick, 0, sizeof(cheatOccuranceTick));
    memset(cheatOccuranceTotal, 0, sizeof(cheatOccuranceTotal));

    _overspeedDistance = 0.f;
    _maxOverspeedDistance = 0.f;
    _clientDesynchro = 0;
    _maxClientDesynchro = 0;

    _jumpInitialSpeed = 0.0f;
    _jumpInitialVelocity = 0.0f;

    _storeCheatFlags = 0;
    _jumpCount = 0;
    updateCheckTimer = CHEATS_UPDATE_INTERVAL;
    _inKnockBack = false;

    _orders =
    {
        { SMSG_MOVE_SET_WALK_SPEED, CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK },
        { SMSG_MOVE_SET_RUN_SPEED, CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK },
        { SMSG_MOVE_SET_RUN_BACK_SPEED, CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK },
        { SMSG_MOVE_SET_SWIM_SPEED, CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK },
        { SMSG_MOVE_SET_SWIM_BACK_SPEED, CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK },
        { SMSG_MOVE_SET_TURN_RATE, CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK },
        { SMSG_MOVE_ROOT, CMSG_MOVE_FORCE_ROOT_ACK },
        { SMSG_MOVE_UNROOT, CMSG_MOVE_FORCE_UNROOT_ACK },
        { SMSG_MOVE_SET_FEATHER_FALL, SMSG_MOVE_SET_NORMAL_FALL, CMSG_MOVE_FEATHER_FALL_ACK },
        { SMSG_MOVE_SET_HOVERING, SMSG_MOVE_UNSET_HOVERING, CMSG_MOVE_HOVER_ACK },
        { SMSG_MOVE_SET_CAN_FLY, SMSG_MOVE_UNSET_CAN_FLY, CMSG_MOVE_SET_CAN_FLY_ACK },
        { SMSG_MOVE_SET_WATER_WALK, SMSG_MOVE_SET_LAND_WALK, CMSG_MOVE_WATER_WALK_ACK },
        { SMSG_MOVE_ENABLE_GRAVITY, SMSG_MOVE_DISABLE_GRAVITY, CMSG_MOVE_GRAVITY_ENABLE_ACK }
    };
}

void PlayerCheatData::KnockBack(float speedxy, float speedz, float cos, float sin)
{
    GetLastMovementInfo().fall.startClientTime = getMSTime() - GetLastMovementInfo().MoveTime + GetLastMovementInfo().ClientMoveTime;
    GetLastMovementInfo().fall.start.m_positionX = me->GetPositionX();
    GetLastMovementInfo().fall.start.m_positionY = me->GetPositionY();
    GetLastMovementInfo().fall.start.m_positionZ = me->GetPositionZ();
    GetLastMovementInfo().fall.Direction.Pos.m_positionX = cos;
    GetLastMovementInfo().fall.Direction.Pos.m_positionY = sin;
    GetLastMovementInfo().fall.HorizontalSpeed = speedxy;
    GetLastMovementInfo().MoveFlags[0] = MOVEMENTFLAG_FALLING | (GetLastMovementInfo().MoveFlags[0] & ~MOVEMENTFLAG_MASK_MOVING_OR_TURN);
    _jumpInitialSpeed = speedz;
    _inKnockBack = true;
}

void PlayerCheatData::StoreCheat(uint32 type, uint32 count)
{
    cheatOccuranceTotal[type] += count;
    cheatOccuranceTick[type] += count;
}

/*void PlayerCheatData::HandleCommand(ChatHandler* handler) const
{
    handler->SendSysMessage("----- ANTICHEAT v3 -----");
    handler->SendSysMessage("_____ Orders/ACK");

    for (auto const &order : _orders)
        handler->PSendSysMessage("\t%s counter=%i", GetOpcodeNameForLogging(order.clientResp).c_str(), order.counter);

    handler->SendSysMessage("_____ Cheats detected");
    for (uint32 i = 0; i < CHEATS_COUNT; ++i)
        if (cheatOccuranceTotal[i])
            handler->PSendSysMessage("%2u x %s (cheat %u - 0x%x)", cheatOccuranceTotal[i], GetCheatTypeNameFromFlag(CheatType(i)), i, 1 << i);

    handler->SendSysMessage("_____ Interpolation");
    handler->PSendSysMessage("MaxSpaceDesync=%f", _maxOverspeedDistance);
    handler->PSendSysMessage("MaxTimeDesync=%u", _maxClientDesynchro);
}*/

void PlayerCheatData::CountCheatOccur(uint32 type, uint32& tickCount, uint32& totalCount) const
{
    ASSERT(type < CHEATS_COUNT);

    tickCount = cheatOccuranceTick[type];
    totalCount = cheatOccuranceTotal[type];
}

CheatAction PlayerCheatData::Update(uint32 diff, std::stringstream& reason)
{
    if (!sAnticheatMgr->EnableAnticheat())
        return CHEAT_ACTION_NONE;

    // Every X seconds, combine detected cheats
    if (updateCheckTimer >= diff)
    {
        updateCheckTimer -= diff;
        return CHEAT_ACTION_NONE;
    }

    return Finalize(reason);
}

CheatAction PlayerCheatData::Finalize(std::stringstream& reason)
{
    auto const now = getMSTime();

    for (auto &order : _orders)
    {
        if (order.counter > 0 && order.lastRcvd < order.lastSent && (now - order.lastSent) > ALLOWED_ACK_LAG)
        {
            if (order.clientResp == CMSG_MOVE_FORCE_ROOT_ACK)
            {
                StoreCheat(CHEAT_TYPE_ROOT_IGNORED, order.counter);
                order.counter = 0;
            }
        }
    }

    if (_maxOverspeedDistance < fabs(_overspeedDistance))
        _maxOverspeedDistance = fabs(_overspeedDistance);

    if (_maxClientDesynchro < static_cast<uint32>(abs(_clientDesynchro)))
        _maxClientDesynchro = abs(_clientDesynchro);

    cheatOccuranceTick[CHEAT_TYPE_TELEPORT_HACK] = uint32(fabs(_overspeedDistance));
    cheatOccuranceTick[CHEAT_TYPE_DESYNC_TIME] = abs(_clientDesynchro);

    if (sAnticheatMgr->EnableDetailsLog() && _clientDesynchro)
        sLog->outAnticheat("Desynchro %ims / %fyards", _clientDesynchro, _overspeedDistance);
    updateCheckTimer = CHEATS_UPDATE_INTERVAL;

    /// Check detected cheats with DB rules
    CheatAction result = sAnticheatMgr->ComputeCheatAction(this, reason);

    /// Log data
    // if (sAnticheatMgr->EnableDataLog() && me->IsInWorld())
    // {
        // CharacterDatabase.PExecute("INSERT INTO cheats_raw "
                                   // "(account, guid, posx, posy, posz, map, desyncMs, desyncDist) VALUES "
                                   // "(%u,      %u,     %f,   %f,   %f, %u,  %i,      %f);",
                                   // me->GetSession()->GetAccountId(), me->GetGUIDLow(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(),
                                   // me->GetMapId(), _clientDesynchro, _overspeedDistance);
    // }

    /// Reset to zero tick counts
    memset(cheatOccuranceTick, 0, sizeof(cheatOccuranceTick));

    _clientDesynchro = 0;
    _overspeedDistance = 0.0f;
    return result;
}

void PlayerCheatData::OrderSent(uint32 opcode)
{
    for (auto &order : _orders)
    {
        if (order.serverOpcode1 == opcode || order.serverOpcode2 == opcode)
        {
            order.lastSent = getMSTime();
            ++order.counter;
            break;
        }
    }
}

void PlayerCheatData::CheckForOrderAck(uint32 opcode)
{
    for (auto &order : _orders)
    {
        if (order.clientResp == opcode)
        {
            --order.counter;
            break;
        }
    }
}

/// Movement processing anticheat main routine
bool PlayerCheatData::HandleAnticheatTests(MovementInfo& movementInfo, WorldSession* session, uint32 opcode)
{
    ASSERT(me);
    ASSERT(session);

    /// TODO: Currently anticheat is disabled with Mind Controlled players!
    if (!sAnticheatMgr->EnableAnticheat() || me != session->GetPlayer())
        return true;

    uint32 cheatType = 0x0;
#define APPEND_CHEAT(t) cheatType |= (1 << t)

    // check ACK responses
    CheckForOrderAck(opcode);

    if (_inKnockBack && opcode != CMSG_MOVE_FALL_LAND)
        movementInfo.fall = GetLastMovementInfo().fall;

    if (opcode == CMSG_MOVE_FEATHER_FALL_ACK)
    {
        GetLastMovementInfo().fall.startClientTime = movementInfo.fall.startClientTime = movementInfo.ClientMoveTime;
        //GetLastMovementInfo().jump.start = movementInfo.jump.start = movementInfo.pos;
        _jumpInitialSpeed = std::max(_jumpInitialSpeed, 7.0f);
    }

    if (opcode == CMSG_MOVE_JUMP && movementInfo.fall.HorizontalSpeed > (GetClientSpeed(MOVE_RUN) + 0.0001f))
        APPEND_CHEAT(CHEAT_TYPE_OVERSPEED_JUMP);

    // Not allowed to change jump speed while jumping
    if ((movementInfo.MoveFlags[0] & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR)) && (GetLastMovementInfo().MoveFlags[0] & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR)))
        if (fabs(movementInfo.fall.HorizontalSpeed - GetLastMovementInfo().fall.HorizontalSpeed) > 0.0001f)
            if (fabs(movementInfo.fall.HorizontalSpeed - std::min(GetClientSpeed(MOVE_RUN), 2.5f)) > 0.0001f)
                APPEND_CHEAT(CHEAT_TYPE_JUMP_SPEED_CHANGE);

    if ((movementInfo.MoveFlags[0] & MOVEMENTFLAG_ROOT) && (movementInfo.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING_OR_TURN))
        APPEND_CHEAT(CHEAT_TYPE_ROOT_MOVE);

    if (sAnticheatMgr->EnableAntiMultiJumpHack())
    {
        if (opcode == CMSG_MOVE_HEARTBEAT)
        {
            if (_jumpCount && movementInfo.fall.JumpVelocity < _jumpInitialVelocity)
                APPEND_CHEAT(CHEAT_TYPE_SUPERJUMP);
        }
        if (opcode == CMSG_MOVE_DOUBLE_JUMP)
            _jumpInitialVelocity = movementInfo.fall.JumpVelocity;

        if (opcode == CMSG_MOVE_KNOCK_BACK_ACK)
            _jumpInitialVelocity = movementInfo.fall.JumpVelocity;

        if (opcode == CMSG_MOVE_JUMP)
        {
            _jumpInitialVelocity = movementInfo.fall.JumpVelocity;
            _jumpCount++;
            if (_jumpCount > 2 && !me->HasAura(SPELL_DH_DOUBLE_JUMP))
                APPEND_CHEAT(CHEAT_TYPE_MULTIJUMP);
        }
        else if (opcode == CMSG_MOVE_FALL_LAND || opcode == CMSG_MOVE_START_SWIM)
            _jumpCount = 0;
    }

    if (movementInfo.ClientMoveTime == 0)
        APPEND_CHEAT(CHEAT_TYPE_NULL_CLIENT_TIME);

    // Dont accept movement packets while movement is controlled by server (fear, charge, etc..)
    if (!me->movespline->Finalized())
        return false;

    // Timing checks
    if (GetLastMovementInfo().ClientMoveTime)
    {
        if (GetLastMovementInfo().MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING)
        {
            int32 currentDesync = static_cast<int32>(getMSTimeDiff(GetLastMovementInfo().ClientMoveTime, movementInfo.ClientMoveTime)) - getMSTimeDiff(GetLastMovementInfo().MoveTime, movementInfo.MoveTime);
            _clientDesynchro += currentDesync;
            if (currentDesync > 1000)
                APPEND_CHEAT(CHEAT_TYPE_NUM_DESYNC);
        }

        // Client going back in time ... ?!
        if (movementInfo.ClientMoveTime < GetLastMovementInfo().ClientMoveTime)
            APPEND_CHEAT(CHEAT_TYPE_TIME_BACK);
    }

    // Warsong Battleground - specific checks
    if (me->GetMapId() == 489)
    {
        // Too high - not allowed (but possible with some engineering items malfunction)
        if (!(movementInfo.MoveFlags[0] & (MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_FALLING)) && movementInfo.Pos.m_positionZ > 380.0f)
            APPEND_CHEAT(CHEAT_TYPE_FORBIDDEN);
        if (Battleground* bg = me->GetBattleground())
        {
            if (bg->GetStatus() == STATUS_WAIT_JOIN)
            {
                // Battleground not started. Players should be in their starting areas.
                if (me->GetTeamId() == TEAM_ALLIANCE && movementInfo.Pos.m_positionX < 1490.0f)
                    APPEND_CHEAT(CHEAT_TYPE_BG_NOT_STARTED);
                if (me->GetTeamId() == TEAM_HORDE && movementInfo.Pos.m_positionX > 957.0f)
                    APPEND_CHEAT(CHEAT_TYPE_BG_NOT_STARTED);
            }
        }
    }

    // Movement states checks
    if (!me->IsLaunched() && !me->IsFalling() && (!me->movespline || me->movespline->Finalized()))
    {
        auto const moveFlags = movementInfo.GetMovementFlags();
        //uint32 removeMoveFlags = 0;

        if ((moveFlags & MOVEMENTFLAG_WALKING)/* && (moveFlags & MOVEMENTFLAG_FIXED_Z)*/ && (moveFlags & MOVEMENTFLAG_SWIMMING) && (moveFlags & MOVEMENTFLAG_HOVER)
            && (moveFlags & MOVEMENTFLAG_FALLING_FAR) && (moveFlags & MOVEMENTFLAG_FLYING))
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK_SWIM);
            //removeMoveFlags |= moveFlags;
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had old flyhack moveFlags mask", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
        }

        // no need to check pending orders for this.  players should never levitate.
        if ((moveFlags & MOVEMENTFLAG_DISABLE_GRAVITY) && !me->HasAuraType(SPELL_AURA_DISABLE_GRAVITY))
        {
            for (auto &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_GRAVITY_ENABLE_ACK)
                {
                    if (!order.counter)
                    {
                        AddCheats(1 << CHEAT_TYPE_DISABLE_GRAVITY);
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had not find fly aura and MOVEMENTFLAG_DISABLE_GRAVITY", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
                    }
                }
            }
        }

        // detect new flyhack (these two flags should never happen at the same time)
        if ((moveFlags & MOVEMENTFLAG_SWIMMING) && (moveFlags & MOVEMENTFLAG_FLYING))
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK_SWIM);
            //removeMoveFlags |= MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING;
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had MOVEMENTFLAG_SWIMMING and MOVEMENTFLAG_FLYING", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
        }

        // detect new flyhack (these two flags should never happen at the same time)
        if ((moveFlags & MOVEMENTFLAG_FLYING) && !(me->HasAuraType(SPELL_AURA_FLY) || me->HasAuraType(SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED)
            || me->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || me->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED)
            || me->HasAuraType(SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS)))
        {
            for (auto &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_SET_CAN_FLY_ACK)
                {
                    if (!order.counter)
                    {
                        AddCheats(1 << CHEAT_TYPE_FLY_HACK);
                        //removeMoveFlags |= MOVEMENTFLAG_FLYING;
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had not find fly aura and MOVEMENTFLAG_FLYING", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
                    }
                }
            }
        }

        /*if (moveFlags & MOVEMENTFLAG_FIXED_Z)
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK_SWIM);
            //removeMoveFlags |= MOVEMENTFLAG_FIXED_Z;
            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had MOVEMENTFLAG_FIXED_Z", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
        }*/

        if (opcode == CMSG_MOVE_STOP_SWIM && (moveFlags & MOVEMENTFLAG_SWIMMING))
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK_SWIM);
            //removeMoveFlags |= MOVEMENTFLAG_SWIMMING;
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) received opcode CMSG_MOVE_STOP_SWIM, but had MOVEMENTFLAG_SWIMMING", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
        }

        // if water walking with no aura and no pending removal order, cheater
        if (moveFlags & MOVEMENTFLAG_WATERWALKING && !me->HasAuraType(SPELL_AURA_WATER_WALK) && !me->HasAuraType(SPELL_AURA_GHOST))
        {
            for (auto const &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_WATER_WALK_ACK)
                {
                    if (!order.counter)
                    {
                        AddCheats(1 << CHEAT_TYPE_WATER_WALK);
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had MOVEMENTFLAG_WATERWALKING with no water walk aura and no pending orders", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
                    }
                    break;
                }
            }

            //removeMoveFlags |= MOVEMENTFLAG_WATERWALKING;
        }

        // if safe falling with no aura and no pending removal order, cheater
        if (moveFlags & MOVEMENTFLAG_FEATHER_FALL && !me->HasAuraType(SPELL_AURA_FEATHER_FALL))
        {
            for (auto const &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_FEATHER_FALL_ACK)
                {
                    if (!order.counter)
                    {
                        AddCheats(1 << CHEAT_TYPE_SLOW_FALL);
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had MOVEMENTFLAG_FEATHER_FALL with no slow fall aura and no pending orders", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
                    }
                    break;
                }
            }

            //removeMoveFlags |= MOVEMENTFLAG_FEATHER_FALL;
        }

        // if hover with no aura and no pending removal order, cheater
        if (moveFlags & MOVEMENTFLAG_HOVER && !me->HasAuraType(SPELL_AURA_HOVER))
        {
            for (auto const &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_HOVER_ACK)
                {
                    if (!order.counter)
                    {
                        AddCheats(1 << CHEAT_TYPE_HOVER);
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had MOVEMENTFLAG_HOVER with no hover aura and no pending orders", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
                    }
                    break;
                }
            }

            //removeMoveFlags |= MOVEMENTFLAG_HOVER;
        }
    }

    // Minimal checks on transports
    if (movementInfo.transport.Guid)
    {
        // To transport tele hack detection
        if (GetLastMovementInfo().ClientMoveTime && !GetLastMovementInfo().transport.Guid)
        {
            float dist2d = (movementInfo.Pos.m_positionX - GetLastMovementInfo().Pos.m_positionX) * (movementInfo.Pos.m_positionX - GetLastMovementInfo().Pos.m_positionX);
            dist2d += (movementInfo.Pos.m_positionY - GetLastMovementInfo().Pos.m_positionY) * (movementInfo.Pos.m_positionY - GetLastMovementInfo().Pos.m_positionY);
            if (dist2d > 100 * 100)
                APPEND_CHEAT(CHEAT_TYPE_TELE_TO_TRANSPORT);
        }
    }

    // Distance computation related
    if (!me->isInFlight() && !(movementInfo.transport.Guid) && !me->m_transport && sAnticheatMgr->EnableAntiSpeedHack())
    {
        float allowedDXY = 0.0f;
        float allowedDZ = 0.0f;
        float realDistance2D_sq = 0.0f;
        bool speedhack = false;

        int32 dt = movementInfo.ClientMoveTime - GetLastMovementInfo().ClientMoveTime;
        if (sAnticheatMgr->GetMaxAllowedDesync() && dt > sAnticheatMgr->GetMaxAllowedDesync())
            dt = sAnticheatMgr->GetMaxAllowedDesync();

        // Check vs interpolation
        float speed = 0.0f;
        if (sAnticheatMgr->EnableAntiSpeedHackInterpolation())
        {
            float intX, intY, intZ, intO;

            if (InterpolateMovement(GetLastMovementInfo(), dt, intX, intY, intZ, intO, speed))
            {
                auto const intDX = intX - movementInfo.Pos.m_positionX;
                auto const intDY = intY - movementInfo.Pos.m_positionY;
                auto const intDZ = intZ - movementInfo.Pos.m_positionZ;

                auto interpolDist = pow(intDX, 2) + pow(intDY, 2);
                if ((movementInfo.MoveFlags[0] | GetLastMovementInfo().MoveFlags[0]) & MOVEMENTFLAG_FALLING)
                    interpolDist += pow(intDZ, 2);
                interpolDist = sqrt(interpolDist);

                float allowedDX = pow(intX - GetLastMovementInfo().Pos.m_positionX, 2);
                float allowedDY = pow(intY - GetLastMovementInfo().Pos.m_positionY, 2);
                allowedDXY = sqrt(allowedDX + allowedDY);

                realDistance2D_sq = pow(movementInfo.Pos.m_positionX - GetLastMovementInfo().Pos.m_positionX, 2) + pow(movementInfo.Pos.m_positionY - GetLastMovementInfo().Pos.m_positionY, 2);

                if (realDistance2D_sq > (allowedDY + allowedDX) * 1.1f)
                {
                    APPEND_CHEAT(CHEAT_TYPE_SPEED_HACK_ALERTS);
                    _overspeedDistance += sqrt(realDistance2D_sq) - sqrt(allowedDY + allowedDX);
                    speedhack = true;
                    if (sAnticheatMgr->EnableDetailsLog())
                        sLog->outAnticheat("Anticheat (SpeedHackAlerts): [Opcode:%s] Flags 0x%x [ClientMoveTime=%u] dist %f allowed %f real %f speed %f",
                            GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), movementInfo.MoveFlags[0], movementInfo.ClientMoveTime, _overspeedDistance, allowedDXY, realDistance2D_sq, speed);
                }

                if (sAnticheatMgr->EnableDetailsLog())
                    sLog->outAnticheat("[Opcode:%s] Flags 0x%x [DT=%u:DR=%.2f] dist %f speed %f",
                        GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), movementInfo.MoveFlags[0], movementInfo.ClientMoveTime - GetLastMovementInfo().ClientMoveTime, interpolDist, _overspeedDistance, speed);
                //DEBUG_UNIT(me, DEBUG_CHEAT, "[Opcode:%s] Flags 0x%x [DT=%u:DR=%.2f]", GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), movementInfo.MoveFlags[0], movementInfo.ClientMoveTime - GetLastMovementInfo().ClientMoveTime, interpolDist);
            }
        }
        else if (GetMaxAllowedDist(GetLastMovementInfo(), dt, allowedDXY, allowedDZ, speed))
        {
            // Allow some margin
            allowedDXY += 0.5f;
            allowedDZ += 0.5f;
            realDistance2D_sq = pow(movementInfo.Pos.m_positionX - GetLastMovementInfo().Pos.m_positionX, 2) + pow(movementInfo.Pos.m_positionY - GetLastMovementInfo().Pos.m_positionY, 2);

            float allowedD = allowedDXY * allowedDXY * 1.1f;
            if (realDistance2D_sq > allowedD)
            {
                APPEND_CHEAT(CHEAT_TYPE_SPEED_HACK_ALERTS);
                _overspeedDistance += sqrt(realDistance2D_sq) - allowedDXY;
                speedhack = true;
                if (sAnticheatMgr->EnableDetailsLog())
                    sLog->outAnticheat("Anticheat (SpeedHackAlerts): [Opcode:%s] Flags 0x%x [DT=%u:ClientMoveTime=%u] dist %f allowed %f real %f speed %f",
                        GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), movementInfo.MoveFlags[0], dt, movementInfo.ClientMoveTime, _overspeedDistance, allowedD, realDistance2D_sq, speed);
            }

            if (fabs(movementInfo.Pos.m_positionZ - GetLastMovementInfo().Pos.m_positionZ) > allowedDZ)
                APPEND_CHEAT(CHEAT_TYPE_OVERSPEED_Z);

            //DEBUG_UNIT(me, DEBUG_CHEAT, "[Opcode:%s] Flags 0x%x [ClientMoveTime=%u]", GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), movementInfo.MoveFlags[0], movementInfo.ClientMoveTime);
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("[Opcode:%s] Flags 0x%x [DT=%u:ClientMoveTime=%u] dist %f allowed %f real %f speed %f",
                    GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), movementInfo.MoveFlags[0], dt, movementInfo.ClientMoveTime, _overspeedDistance, allowedD, realDistance2D_sq, speed);
        }

        // Client should send heartbeats every 500ms
        // if (dt > 1000 && GetLastMovementInfo().ClientMoveTime && GetLastMovementInfo().MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING)
            // APPEND_CHEAT(CHEAT_TYPE_SKIPPED_HEARTBEATS);

        uint32 destZoneId = 0;
        uint32 destAreaId = 0;

        // Check over-speedhack and far teleports
        if (speedhack && !CheckFarDistance(movementInfo, realDistance2D_sq, destZoneId, destAreaId))
        {
            // get zone and area info
            MapEntry const* mapEntry = sMapStore.LookupEntry(me->GetMapId());
            const auto *srcZoneEntry = sAreaTableStore.LookupEntry(me->GetZoneId());
            const auto *srcAreaEntry = sAreaTableStore.LookupEntry(me->GetAreaId());
            const auto *destZoneEntry = sAreaTableStore.LookupEntry(destZoneId);
            const auto *destAreaEntry = sAreaTableStore.LookupEntry(destAreaId);

            uint32 locale = sWorld->GetDefaultDbcLocale();

            const char *mapName = mapEntry ? mapEntry->MapName->Str[locale] : "<unknown>";
            const char *srcZoneName = srcZoneEntry ? srcZoneEntry->AreaName->Str[locale] : "<unknown>";
            const char *srcAreaName = srcAreaEntry ? srcAreaEntry->AreaName->Str[locale] : "<unknown>";
            const char *destZoneName = destZoneEntry ? destZoneEntry->AreaName->Str[locale] : "<unknown>";
            const char *destAreaName = destAreaEntry ? destAreaEntry->AreaName->Str[locale] : "<unknown>";

            sLog->outAnticheat("ServerAnticheat (TeleportHack): player %s, %s, %.2f yd\n"
                "    map %u \"%s\"\n"
                "    source: zone %u \"%s\" area %u \"%s\" %.2f, %.2f, %.2f\n"
                "    dest:   zone %u \"%s\" area %u \"%s\" %.2f, %.2f, %.2f",
                me->GetName(), GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), sqrt(realDistance2D_sq),
                me->GetMapId(), mapName,
                me->GetZoneId(), srcZoneName, me->GetAreaId(), srcAreaName,
                me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(),
                destZoneId, destZoneName, destAreaId, destAreaName,
                movementInfo.Pos.m_positionX, movementInfo.Pos.m_positionY, movementInfo.Pos.m_positionZ);

            // ban for GM Island
            if (me->GetSession()->GetSecurity() == SEC_PLAYER && destZoneId == 876 && destAreaId == 876)
            {
                sWorld->BanAccount(BAN_ACCOUNT, me->GetSession()->GetPlayerName(), nullptr, "Infiltration on GM Island", "Warden AntiCheat");
                return false;
            }

            // save prevoius point
            Player::SavePositionInDB(me->GetMapId(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), me->GetZoneId(), me->GetGUID());
            me->GetSession()->KickPlayer();
            return false;
        }
    }

    //GetLastMovementInfo() = movementInfo;
    //GetLastMovementInfo().UpdateTime(getMSTime());

    // This is required for proper movement interpolation
    if (opcode == CMSG_MOVE_JUMP)
        _jumpInitialSpeed = 7.95797334f;
    else if (opcode == CMSG_MOVE_FALL_LAND)
    {
        _jumpInitialSpeed = -9.645f;
        _inKnockBack = false;
    }

    AddCheats(cheatType);

    return true;
#undef APPEND_CHEAT
}

bool PlayerCheatData::HandleSpeedChangeAck(MovementInfo& movementInfo, WorldSession* session, uint32 opcode, float newSpeed)
{
    static char const* move_type_name[MAX_MOVE_TYPE] = { "Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate", "FlightRate", "FlightBackRate", "PitchRate" };
    UnitMoveType moveType;
    switch (opcode)
    {
        case CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK:
            moveType = MOVE_WALK;
            break;
        case CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK:
            moveType = MOVE_RUN;
            break;
        case CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK:
            moveType = MOVE_RUN_BACK;
            break;
        case CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK:
            moveType = MOVE_SWIM;
            break;
        case CMSG_MOVE_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:
            moveType = MOVE_SWIM_BACK;
            break;
        case CMSG_MOVE_FORCE_TURN_RATE_CHANGE_ACK:
            moveType = MOVE_TURN_RATE;
            break;
        case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK:
            moveType = MOVE_FLIGHT;
            break;
        case CMSG_MOVE_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:
            moveType = MOVE_FLIGHT_BACK;
            break;
        case CMSG_MOVE_FORCE_PITCH_RATE_CHANGE_ACK:
            moveType = MOVE_PITCH_RATE;
            break;
        default:
            return false;
    }

    // Compare to server side speed.
    if (!me->GetTransport() && fabs(me->GetSpeed(moveType) - newSpeed) > 0.01f)
    {
        if (sAnticheatMgr->EnableDetailsLog())
            sLog->outAnticheat("%sSpeedChange player %s incorrect. %f -> %f (instead of %f)", move_type_name[moveType], me->GetName(), _clientSpeeds[moveType], newSpeed, me->GetSpeed(moveType));
        me->SetSpeedRate(moveType, me->GetSpeedRate(moveType));
    }

    // Compute anticheat generic checks - with old speed.
    HandleAnticheatTests(movementInfo, session, opcode);
    _clientSpeeds[moveType] = newSpeed;
    return true;
}

void PlayerCheatData::InitSpeeds(Unit* unit)
{
    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        _clientSpeeds[i] = unit->GetSpeed(UnitMoveType(i));
}


void PlayerCheatData::Unreachable(Unit* attacker)
{
    if (IsInKnockBack())
        return;
    if (attacker->GetCharmerOrOwnerGUID() == me->GetGUID())
        return;
    if (me->GetTransport())
        return;

    float waterLevel = (me->GetBaseMap()->GetWaterLevel(me->GetPositionX(), me->GetPositionY()) + 5.0f);
    if (me->GetPositionZ() < waterLevel)
        return;
    if (attacker->GetPositionZ() < waterLevel)
        return;

    AddCheats(1 << CHEAT_TYPE_UNREACHABLE);
}

MovementInfo& PlayerCheatData::GetLastMovementInfo()
{
    return me->m_movementInfo;
}

void PlayerCheatData::AddCheats(uint32 cheats, uint32 count)
{
    if (!cheats || me->isGameMaster())
        return;

    std::string sName = me->GetName();

    // This is a new cheat detected for this player
    if (!(_storeCheatFlags & cheats))
    {
        _storeCheatFlags |= cheats;
        if (cheats & sAnticheatMgr->AnnounceCheatMask())
        {
            std::string cheatInfos = "|cffffffff|Hplayer:" + sName + "|h[" + sName + "]|h|r : ";
            for (uint32 i = 0; i < CHEATS_COUNT; ++i)
                if (cheats & (1 << i))
                    cheatInfos += "-[" + std::string(GetCheatTypeNameFromFlag(CheatType(i))) + "]";
            sWorld->SendGMText(LANG_GM_ANNOUNCE_COLOR, "[AntiCheatAlert]", cheatInfos.c_str());
        }
    }

    if (sAnticheatMgr->NotifyCheaters())
    {
        for (uint32 i = 0; i < CHEATS_COUNT; ++i)
            if (cheats & (1 << i))
                ChatHandler(me).PSendSysMessage("[AntiCheat] Cheat : %s i %u CheatType %u", GetCheatTypeNameFromFlag(CheatType(i)), i, CheatType(i));
    }

    for (uint32 i = 0; i < CHEATS_COUNT; ++i)
        if (cheats & (1 << i))
            StoreCheat(i, count);
}

bool PlayerCheatData::InterpolateMovement(MovementInfo const& mi, uint32 diffMs, float &x, float &y, float &z, float &outOrientation, float &speed)
{
    // TODO: These cases are not handled in mvt interpolationo
    // - Transports
    if (mi.MoveFlags[0] & (MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN) || mi.transport.Guid)
        return false;
    // - Not correctly handled yet (issues regarding feather fall auras)
    if (mi.MoveFlags[0] & MOVEMENTFLAG_FALLING_FAR)
        return false;
    // - Server side movement (should be easy to interpolate actually)
    if (!me->movespline->Finalized())
        return false;
    // Dernier paquet pas a jour (connexion, TP autre map ...)
    if (mi.ClientMoveTime == 0)
        return false;
    x = mi.Pos.m_positionX;
    y = mi.Pos.m_positionY;
    z = mi.Pos.m_positionZ;
    outOrientation = mi.Pos.m_orientation;
    float o = outOrientation;
    // Not allowed to move
    if (mi.MoveFlags[0] & MOVEMENTFLAG_ROOT)
        return true;

    if (mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING_FLY)
        speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_FLIGHT_BACK) : GetClientSpeed(MOVE_FLIGHT);
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_SWIMMING)
        speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_SWIM_BACK) : GetClientSpeed(MOVE_SWIM);
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_WALKING)
        speed = GetClientSpeed(MOVE_WALK);
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING)
        speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_RUN_BACK) : GetClientSpeed(MOVE_RUN);
    else if (mi.MoveFlags[0] & (MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN))
        speed = GetClientSpeed(MOVE_PITCH_RATE);
    if (mi.MoveFlags[0] & MOVEMENTFLAG_BACKWARD)
        o += M_PI_F;
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_STRAFE_LEFT)
    {
        if (mi.MoveFlags[0] & MOVEMENTFLAG_FORWARD)
            o += M_PI_F / 4;
        else
            o += M_PI_F / 2;
    }
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_STRAFE_RIGHT)
    {
        if (mi.MoveFlags[0] & MOVEMENTFLAG_FORWARD)
            o -= M_PI_F / 4;
        else
            o -= M_PI_F / 2;
    }
    if (mi.MoveFlags[0] & MOVEMENTFLAG_FALLING)
    {
        float diffT = getMSTimeDiff(mi.fall.startClientTime, diffMs + mi.ClientMoveTime) / 1000.0f;
        x = mi.fall.start.m_positionX;
        y = mi.fall.start.m_positionY;
        z = mi.fall.start.m_positionZ;
        // Fatal error. Avoid crashing here ...
        if (!x || !y || !z || diffT > 10000.0f)
            return false;
        x += mi.fall.Direction.Pos.m_positionX * mi.fall.HorizontalSpeed * diffT;
        y += mi.fall.Direction.Pos.m_positionY * mi.fall.HorizontalSpeed * diffT;
        z -= Movement::computeFallElevation(diffT, mi.MoveFlags[0] & MOVEMENTFLAG_FEATHER_FALL, -_jumpInitialSpeed);
    }
    else if (mi.MoveFlags[0] & (MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT))
    {
        if (mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING)
        {
            // Every 2 sec
            float T = 0.75f * (GetClientSpeed(MOVE_TURN_RATE)) * (diffMs / 1000.0f);
            float R = 1.295f * speed / M_PI * cos(mi.pitch);
            z += diffMs * speed / 1000.0f * sin(mi.pitch);
            // Find the center of the circle we are moving on
            if (mi.MoveFlags[0] & MOVEMENTFLAG_LEFT)
            {
                x += R * cos(o + M_PI / 2);
                y += R * sin(o + M_PI / 2);
                outOrientation += T;
                T = T - M_PI / 2.0f;
            }
            else
            {
                x += R * cos(o - M_PI / 2);
                y += R * sin(o - M_PI / 2);
                outOrientation -= T;
                T = -T + M_PI / 2.0f;
            }
            x += R * cos(o + T);
            y += R * sin(o + T);
        }
        else
        {
            float diffO = GetClientSpeed(MOVE_TURN_RATE) * diffMs / 1000.0f;
            if (mi.MoveFlags[0] & MOVEMENTFLAG_LEFT)
                outOrientation += diffO;
            else
                outOrientation -= diffO;
            return true;
        }
    }
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING)
    {
        float dist = speed * diffMs / 1000.0f;
        x += dist * cos(o) * cos(mi.pitch);
        y += dist * sin(o) * cos(mi.pitch);
        z += dist * sin(mi.pitch);
    }
    else // If we reach here, we did not move
        return true;

    if (!Trinity::IsValidMapCoord(x, y, z, o))
        return false;

    if (!(mi.MoveFlags[0] & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_SWIMMING)))
        z = me->GetMap()->GetHeight(x, y, z);
    return me->GetMap()->isInLineOfSight(mi.Pos.m_positionX, mi.Pos.m_positionY, mi.Pos.m_positionZ + 0.5f, x, y, z + 0.5f, me->GetPhases());
}

bool PlayerCheatData::GetMaxAllowedDist(MovementInfo const& mi, uint32 diffMs, float &dxy, float &dz, float &speed)
{
    dxy = dz = 0.001f; // Epsilon
    speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_RUN_BACK) : GetClientSpeed(MOVE_RUN);
    if (mi.transport.Guid)
        return false;
    if (!me->movespline->Finalized())
        return false;
    // Dernier paquet pas a jour (connexion, TP autre map ...)
    if (!mi.ClientMoveTime)
        return false;

    // No mvt allowed
    if ((mi.MoveFlags[0] & MOVEMENTFLAG_ROOT) || !(mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING))
        return true;

    if (mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING_FLY)
        speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_FLIGHT_BACK) : GetClientSpeed(MOVE_FLIGHT);
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_SWIMMING)
        speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_SWIM_BACK) : GetClientSpeed(MOVE_SWIM);
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_WALKING)
        speed = GetClientSpeed(MOVE_WALK);
    else if (mi.MoveFlags[0] & MOVEMENTFLAG_MASK_MOVING)
        speed = mi.MoveFlags[0] & (MOVEMENTFLAG_BACKWARD) ? GetClientSpeed(MOVE_RUN_BACK) : GetClientSpeed(MOVE_RUN);
    else if (mi.MoveFlags[0] & (MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN))
        speed = GetClientSpeed(MOVE_PITCH_RATE);

    if (mi.MoveFlags[0] & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR))
    {
        dxy = mi.fall.HorizontalSpeed / 1000 * diffMs;
        static const float terminalVelocity = 60.148003f;
        static const float terminalSavefallVelocity = 7.f;
        dz = (mi.MoveFlags[0] & MOVEMENTFLAG_FEATHER_FALL) ? terminalSavefallVelocity : terminalVelocity;
        dz = dz / 1000 * diffMs;
        return true;
    }
    // TODO: Maximum dyx/dz (max climb angle) if not swimming.
    dxy = speed / 1000 * diffMs;
    dz = speed / 1000 * diffMs;
    return true;
}

void PlayerCheatData::OnExplore(AreaTableEntry const* p)
{
    // AddCheats(1 << CHEAT_TYPE_EXPLORE);
    if (static_cast<int32>(me->getLevel() + 10) < p->ExplorationLevel)
        AddCheats(1 << CHEAT_TYPE_EXPLORE_HIGH_LEVEL);
}

void PlayerCheatData::OnTransport(Player* plMover, ObjectGuid transportGuid)
{
    // The anticheat is disabled on transports, so we need to be sure that the player is indeed on a transport.
    GameObject* transportGobj = plMover->GetMap()->GetGameObject(transportGuid);
    float maxDist2d = 70.0f; // Transports usually dont go far away.
    if (plMover->GetMapId() == 369) // Deeprun tram
        maxDist2d = 3000.0f;
    if (!transportGobj || !transportGobj->IsTransport() || !transportGobj->IsWithinDist(plMover, maxDist2d, false))
        AddCheats(1 << CHEAT_TYPE_FAKE_TRANSPORT);
}

// TEMPORARY!!!

bool PlayerCheatData::HandleCustomAnticheatTests(uint32 opcode, MovementInfo& movementInfo)
{
    // TODO These checks are unreliable and should be implemented in other way

    if (!me->m_mover->IsPlayer())
        return true;

    Player* mover = me->m_mover->ToPlayer();

    /* teleport hack check */
    if (!mover->m_transport && !mover->m_movementInfo.transport.Guid && !mover->isInFlight())
    {
        float distance;

        if (!AllowUpdatePosition(movementInfo, distance))
        {
            uint32 destZoneId = 0;
            uint32 destAreaId = 0;

            mover->GetBaseMap()->GetZoneAndAreaId(destZoneId, destAreaId, movementInfo.Pos.m_positionX, movementInfo.Pos.m_positionY, movementInfo.Pos.m_positionZ);

            // get zone and area info
            MapEntry const* mapEntry = sMapStore.LookupEntry(mover->GetMapId());
            const auto *srcZoneEntry = sAreaTableStore.LookupEntry(mover->GetZoneId());
            const auto *srcAreaEntry = sAreaTableStore.LookupEntry(mover->GetAreaId());
            const auto *destZoneEntry = sAreaTableStore.LookupEntry(destZoneId);
            const auto *destAreaEntry = sAreaTableStore.LookupEntry(destAreaId);

            uint32 locale = sWorld->GetDefaultDbcLocale();

            const char *mapName = mapEntry ? mapEntry->MapName->Str[locale] : "<unknown>";
            const char *srcZoneName = srcZoneEntry ? srcZoneEntry->AreaName->Str[locale] : "<unknown>";
            const char *srcAreaName = srcAreaEntry ? srcAreaEntry->AreaName->Str[locale] : "<unknown>";
            const char *destZoneName = destZoneEntry ? destZoneEntry->AreaName->Str[locale] : "<unknown>";
            const char *destAreaName = destAreaEntry ? destAreaEntry->AreaName->Str[locale] : "<unknown>";

            sLog->outAnticheat("OldServerAnticheat (TeleportHack): player %s, %s, %.2f yd\n"
                "    map %u \"%s\"\n"
                "    source: zone %u \"%s\" area %u \"%s\" %.2f, %.2f, %.2f\n"
                "    dest:   zone %u \"%s\" area %u \"%s\" %.2f, %.2f, %.2f",
                mover->GetName(), GetOpcodeNameForLogging(static_cast<OpcodeClient>(opcode)).c_str(), distance,
                mover->GetMapId(), mapName,
                mover->GetZoneId(), srcZoneName, mover->GetAreaId(), srcAreaName,
                mover->GetPositionX(), mover->GetPositionY(), mover->GetPositionZ(),
                destZoneId, destZoneName, destAreaId, destAreaName,
                movementInfo.Pos.m_positionX, movementInfo.Pos.m_positionY, movementInfo.Pos.m_positionZ);

            // ban for GM Island
            if (me->GetSession()->GetSecurity() == SEC_PLAYER && destZoneId == 876 && destAreaId == 876)
            {
                sWorld->BanAccount(BAN_ACCOUNT, me->GetSession()->GetPlayerName(), nullptr, "Infiltration on GM Island", "Warden AntiCheat");
                return false;
            }

            // save prevoius point
            Player::SavePositionInDB(mover->GetMapId(), mover->m_movementInfo.Pos.m_positionX, mover->m_movementInfo.Pos.m_positionY, mover->m_movementInfo.Pos.m_positionZ, mover->m_movementInfo.Pos.m_orientation, mover->GetZoneId(), mover->GetGUID());
            me->GetSession()->KickPlayer();
            return false;
        }
    }

    // in launched/falling/spline movement - maybe false positives
    if (!mover->IsLaunched() && !mover->IsFalling() && (!mover->movespline || mover->movespline->Finalized()))
    {
        auto const moveFlags = movementInfo.GetMovementFlags();
        int32 removeMoveFlags = 0;

        if ((moveFlags & MOVEMENTFLAG_WALKING)/* && (moveFlags & MOVEMENTFLAG_FIXED_Z)*/ && (moveFlags & MOVEMENTFLAG_SWIMMING) && (moveFlags & MOVEMENTFLAG_HOVER)
            && (moveFlags & MOVEMENTFLAG_FALLING_FAR) && (moveFlags & MOVEMENTFLAG_FLYING))
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK_SWIM);
            removeMoveFlags |= moveFlags;
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("OldServerAnticheat (MovementFlags hack): player %s had old flyhack moveFlags mask", me->GetSession()->GetPlayerName().c_str());
        }

        // no need to check pending orders for this.  players should never levitate.
        if ((moveFlags & MOVEMENTFLAG_DISABLE_GRAVITY) && !me->HasAuraType(SPELL_AURA_DISABLE_GRAVITY))
        {
            for (auto &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_GRAVITY_ENABLE_ACK)
                {
                    if (!order.counter)
                    {
                        AddCheats(1 << CHEAT_TYPE_DISABLE_GRAVITY);
                        removeMoveFlags |= MOVEMENTFLAG_DISABLE_GRAVITY;
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had not find fly aura and MOVEMENTFLAG_DISABLE_GRAVITY", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
                    }
                }
            }
        }

        // detect new flyhack (these two flags should never happen at the same time)
        if ((moveFlags & MOVEMENTFLAG_SWIMMING) && (moveFlags & MOVEMENTFLAG_FLYING))
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK_SWIM);
            removeMoveFlags |= MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING;
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("OldServerAnticheat (MovementFlags hack): player %s had MOVEMENTFLAG_SWIMMING and MOVEMENTFLAG_FLYING", me->GetSession()->GetPlayerName().c_str());
        }

        // detect new flyhack (these two flags should never happen at the same time)
        if ((moveFlags & MOVEMENTFLAG_FLYING) && !(mover->HasAuraType(SPELL_AURA_FLY) || mover->HasAuraType(SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED)
            || mover->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || mover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED)
            || mover->HasAuraType(SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS)))
        {
            AddCheats(1 << CHEAT_TYPE_FLY_HACK);
            removeMoveFlags |= MOVEMENTFLAG_FLYING;
            if (sAnticheatMgr->EnableDetailsLog())
                sLog->outAnticheat("Anticheat (MovementFlags hack): player %s (security: %u) had !MOVEMENTFLAG_CAN_FLY and MOVEMENTFLAG_FLYING", me->GetSession()->GetPlayerName().c_str(), me->GetSession()->GetSecurity());
        }

        /*if (moveFlags & MOVEMENTFLAG_FIXED_Z)
        {
            removeMoveFlags |= MOVEMENTFLAG_FIXED_Z;
            sLog->outAnticheat("OldServerAnticheat (MovementFlags hack): player %s had MOVEMENTFLAG_FIXED_Z", me->GetSession()->GetPlayerName().c_str());
        }*/

        // if water walking with no aura and no pending removal order, cheater
        if (moveFlags & MOVEMENTFLAG_WATERWALKING && !me->HasAuraType(SPELL_AURA_WATER_WALK) && !me->HasAuraType(SPELL_AURA_GHOST))
        {
            for (auto const &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_WATER_WALK_ACK)
                {
                    if (!order.counter)
                    {
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("OldServerAnticheat (MovementFlags hack): player %s had MOVEMENTFLAG_WATERWALKING with no water walk aura and no pending orders", me->GetSession()->GetPlayerName().c_str());
                        AddCheats(1 << CHEAT_TYPE_WATER_WALK);
                    }
                    break;
                }
            }

            removeMoveFlags |= MOVEMENTFLAG_WATERWALKING;
        }

        // if safe falling with no aura and no pending removal order, cheater
        if (moveFlags & MOVEMENTFLAG_FEATHER_FALL && !me->HasAuraType(SPELL_AURA_FEATHER_FALL))
        {
            for (auto const &order : _orders)
            {
                if (order.clientResp == CMSG_MOVE_FEATHER_FALL_ACK)
                {
                    if (!order.counter)
                    {
                        if (sAnticheatMgr->EnableDetailsLog())
                            sLog->outAnticheat("OldServerAnticheat (MovementFlags hack): player %s had MOVEMENTFLAG_FEATHER_FALL with no slow fall aura and no pending orders", me->GetSession()->GetPlayerName().c_str());
                        AddCheats(1 << CHEAT_TYPE_SLOW_FALL);
                    }
                    break;
                }
            }

            removeMoveFlags |= MOVEMENTFLAG_FEATHER_FALL;
        }

        if (removeMoveFlags)
            movementInfo.RemoveMovementFlag(MovementFlags(removeMoveFlags));
    }

    return true;
}

bool PlayerCheatData::AllowUpdatePosition(MovementInfo const& movementInfo, float& distance)
{
    // check valid source coordinates
    if (me->GetPositionX() == 0.0f || me->GetPositionY() == 0.0f || me->GetPositionZ() == 0.0f)
        return true;

    // check valid destination coordinates
    if (movementInfo.Pos.m_positionX == 0.0f || movementInfo.Pos.m_positionY == 0.0f || movementInfo.Pos.m_positionZ == 0.0f)
        return true;

    // if knockbacked
    if (me->IsLaunched())
        return true;

    // ignore valid teleport state
    if (me->IsBeingTeleported())
        return true;

    // some exclude zones - lifts and other, but..
    uint32 destZoneId = 0;
    uint32 destAreaId = 0;

    me->GetBaseMap()->GetZoneAndAreaId(destZoneId, destAreaId, movementInfo.Pos.m_positionX, movementInfo.Pos.m_positionY, movementInfo.Pos.m_positionZ);

    // checks far teleports
    if (destZoneId == me->GetZoneId() && destAreaId == me->GetAreaId())
    {
        // Thousand Needles - Great Lift
        if (me->GetZoneId() == 2257 || (me->GetZoneId() == 400 && me->GetAreaId() == 485))
            return true;

        // Undercity Lift
        if (me->GetZoneId() == 1497 && me->GetAreaId() == 1497)
            return true;
    }

    float deltaX = me->GetPositionX() - movementInfo.Pos.m_positionX;
    float deltaY = me->GetPositionY() - movementInfo.Pos.m_positionY;
    float deltaZ = me->GetPositionZ() - movementInfo.Pos.m_positionZ;
    distance = sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);

    return distance < 40.0f;
}

bool PlayerCheatData::CheckFarDistance(MovementInfo const& movementInfo, float distance, uint32& destZoneId, uint32& destAreaId)
{
    // check valid source coordinates
    if (me->GetPositionX() == 0.0f || me->GetPositionY() == 0.0f || me->GetPositionZ() == 0.0f)
        return true;

    // check valid destination coordinates
    if (movementInfo.Pos.m_positionX == 0.0f || movementInfo.Pos.m_positionY == 0.0f || movementInfo.Pos.m_positionZ == 0.0f)
        return true;

    // if knockbacked
    if (me->IsLaunched())
        return true;

    // ignore valid teleport state
    if (me->IsBeingTeleported())
        return true;

    uint32 destZone;
    uint32 destArea;
    // some exclude zones - lifts and other, but..
    me->GetBaseMap()->GetZoneAndAreaId(destZone, destArea, movementInfo.Pos.m_positionX, movementInfo.Pos.m_positionY, movementInfo.Pos.m_positionZ);
    destZoneId = destZone;
    destAreaId = destArea;

    // checks far teleports
    if (destZoneId == me->GetZoneId() && destAreaId == me->GetAreaId())
    {
        // Thousand Needles - Great Lift
        if (me->GetZoneId() == 2257 || (me->GetZoneId() == 400 && me->GetAreaId() == 485))
            return true;

        // Undercity Lift
        if (me->GetZoneId() == 1497 && me->GetAreaId() == 1497)
            return true;
    }

    // some test extrapolation
    return distance <= 20.0 * 20.0f;
}
