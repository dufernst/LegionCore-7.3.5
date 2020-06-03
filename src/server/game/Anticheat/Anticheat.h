#ifndef _HEADER_CHEATS
#define _HEADER_CHEATS

#include <vector>
#include <map>

#include "Common.h"
#include "Unit.h" // For MovementInfo

enum CheatType
{
    CHEAT_TYPE_WALL_CLIMB           = 0,
    CHEAT_TYPE_WATER_WALK           = 1,
    CHEAT_TYPE_FORBIDDEN            = 2,
    CHEAT_TYPE_BG_NOT_STARTED       = 3,
    CHEAT_TYPE_MULTIJUMP            = 4,
    CHEAT_TYPE_FALL_UP              = 5,
    CHEAT_TYPE_UNREACHABLE          = 6,
    CHEAT_TYPE_TIME_BACK            = 7,
    CHEAT_TYPE_OVERSPEED_JUMP       = 8,
    CHEAT_TYPE_JUMP_SPEED_CHANGE    = 9,
    CHEAT_TYPE_FLY_HACK_SWIM        = 10,
    CHEAT_TYPE_NULL_CLIENT_TIME     = 11,
    CHEAT_TYPE_ROOT_MOVE            = 12,
    CHEAT_TYPE_ROOT_IGNORED         = 13,
    CHEAT_TYPE_TELEPORT_HACK        = 14,
    CHEAT_TYPE_DESYNC_TIME          = 15,
    CHEAT_TYPE_MOVE_STOP            = 16,
    CHEAT_TYPE_EXPLORE              = 17,
    CHEAT_TYPE_EXPLORE_HIGH_LEVEL   = 18,
    CHEAT_TYPE_OVERSPEED_Z          = 19,
    CHEAT_TYPE_SKIPPED_HEARTBEATS   = 20,
    CHEAT_TYPE_NUM_DESYNC           = 21,
    CHEAT_TYPE_FAKE_TRANSPORT       = 22,
    CHEAT_TYPE_TELE_TO_TRANSPORT    = 23,
    CHEAT_TYPE_SLOW_FALL            = 24,
    CHEAT_TYPE_HOVER                = 25,
    CHEAT_TYPE_SPEED_HACK_ALERTS    = 26,
    CHEAT_TYPE_NO_FALLTIME          = 27,
    CHEAT_TYPE_FLY_HACK             = 28,
    CHEAT_TYPE_SUPERJUMP            = 29,
    CHEAT_TYPE_DISABLE_GRAVITY      = 30,
    CHEATS_COUNT
};

enum CheatAction
{
    CHEAT_ACTION_NONE           = 0x00,
    CHEAT_ACTION_LOG            = 0x01,
    CHEAT_ACTION_REPORT_GMS     = 0x02,
    CHEAT_ACTION_KICK           = 0x04,
    CHEAT_ACTION_BAN_ACCOUNT    = 0x08,
    CHEAT_ACTION_BAN_IP_ACCOUNT = 0x10,
    CHEAT_ACTION_TELEPORT_BACK  = 0x20,
    CHEAT_ACTION_MUTE_PUB_CHANS = 0x40, // Mutes the account from public channels
    CHEAT_MAX_ACTIONS,
};

#define CHEATS_UPDATE_INTERVAL      4000
// Time between server sends stun, and client is actually stunned
#define ALLOWED_ACK_LAG             2000
const char* GetCheatTypeNameFromFlag(CheatType type);


class ChatHandler;
class Player;
class Unit;
class WorldPacket;
class PlayerCheatData;
class WorldSession;
struct AreaTableEntry;

struct CheatSanctions
{
    uint32 cheatType;
    uint32 tickCount;
    uint32 tickSanction;
    uint32 totalCount;
    uint32 totalSanction;
    std::string comment;
};

/// Donnees statiques.
class PlayerCheatsMgr
{
    public:
    static PlayerCheatsMgr* instance();
        void LoadConfig();
        void LoadFromDB();
        CheatAction ComputeCheatAction(PlayerCheatData* cheatData, std::stringstream& reason) const;
        PlayerCheatData* CreateAnticheatFor(Player* player);
        std::vector<CheatSanctions> _sanctions;
        // Config.
        bool EnableAnticheat()         const { return _enabled;       }
        bool EnableAntiMultiJumpHack() const { return _antiMultiJump; }
        bool EnableAntiSpeedHack()     const { return _antiSpeedHack; }
        bool EnableAntiSpeedHackInterpolation() const { return _antiSpeedHackInterp; }
        bool EnableAntiWallClimbing()  const { return _antiWallClimb; }
        bool EnableDataLog()     const { return _logDatas; }
        bool EnableDetailsLog()     const { return _logDetails; }
        uint32 AnnounceCheatMask()     const { return _announceCheatMask; }
        uint32 NotifyCheaters()        const { return _notifyCheaters; }
        int32 GetMaxAllowedDesync()    const { return _maxAllowedDesync; }
    protected:
        // Configuration
        bool _enabled;
        bool _antiMultiJump;
        bool _antiSpeedHack;
        bool _antiSpeedHackInterp;
        bool _antiWallClimb;
        bool _logDatas;
        bool _logDetails;
        uint32 _announceCheatMask;
        int32 _maxAllowedDesync;
        uint32 _notifyCheaters;
};

#define sAnticheatMgr PlayerCheatsMgr::instance()

class Player;
class Unit;
class WorldSession;
class WorldPacket;

class ServerOrderData
{
    public:
        ServerOrderData(uint32 serv, uint32 resp) : serverOpcode1(serv), serverOpcode2(0), clientResp(resp), lastSent(0), lastRcvd(0), counter(0) {}
        ServerOrderData(uint32 serv1, uint32 serv2, uint32 resp) : serverOpcode1(serv1), serverOpcode2(serv2), clientResp(resp), lastSent(0), lastRcvd(0), counter(0) {}

        uint32 serverOpcode1;
        uint32 serverOpcode2;
        uint32 clientResp;

        uint32 lastSent;
        uint32 lastRcvd;
        int32 counter;
};

class PlayerCheatData
{
    public:
    explicit PlayerCheatData(Player* _me);

        void Init();
        bool IsInKnockBack() const { return _inKnockBack; }
        void KnockBack(float speedxy, float speedz, float cos, float sin);

        /// STATS
        uint32 updateCheckTimer;
        uint32 cheatOccuranceTick[CHEATS_COUNT];    // per anticheat tick (not world/map tick)
        uint32 cheatOccuranceTotal[CHEATS_COUNT];

        void AddCheats(uint32 cheats, uint32 count = 1);
        void Unreachable(Unit* attacker);
        void StoreCheat(uint32 type, uint32 count=1);
        //void HandleCommand(ChatHandler* handler) const;
        void CountCheatOccur(uint32 type, uint32& tickCount, uint32& totalCount) const;
        CheatAction Update(uint32 diff, std::stringstream& reason);
        CheatAction Finalize(std::stringstream& reason);
        bool HandleAnticheatTests(MovementInfo& movementInfo, WorldSession* session, uint32 opcode);
        bool HandleCustomAnticheatTests(uint32 opcode, MovementInfo& movementInfo);
        bool HandleSpeedChangeAck(MovementInfo& movementInfo, WorldSession* session, uint32 opcode, float newSpeed);
        void InitSpeeds(Unit* unit);
        float GetClientSpeed(UnitMoveType m) const { return _clientSpeeds[m]; }

        void OrderSent(uint32 opcode);
        void CheckForOrderAck(uint32 opcode);

        bool InterpolateMovement(MovementInfo const& mi, uint32 diffMs, float &x, float &y, float &z, float &o, float &speed);
        bool GetMaxAllowedDist(MovementInfo const& mi, uint32 diffMs, float &dxy, float &dz, float &speed);
        MovementInfo& GetLastMovementInfo();
        void OnExplore(AreaTableEntry const* p);
        virtual void OnTransport(Player* plMover, ObjectGuid transportGuid);

        bool AllowUpdatePosition(MovementInfo const& movementInfo, float& distance);
        bool CheckFarDistance(MovementInfo const& movementInfo, float distance, uint32& destZoneId, uint32& destAreaId);

        uint32 _storeCheatFlags;

        uint32 _jumpCount;
        uint32 _speedAlertCount;
        int32 _clientDesynchro;
        float _jumpInitialSpeed;
        float _jumpInitialVelocity;
        float _overspeedDistance;
        bool _inKnockBack;
        float  _clientSpeeds[MAX_MOVE_TYPE];
        std::vector<ServerOrderData> _orders; // Packets sent by server, triggering *_ACK from client
        Player* me;
        // Logs
        float _maxOverspeedDistance;
        uint32 _maxClientDesynchro;
};

#endif
