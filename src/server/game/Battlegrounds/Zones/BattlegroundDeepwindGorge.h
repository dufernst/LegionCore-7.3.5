/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __BATTLEGROUNDDG_H
#define __BATTLEGROUNDDG_H

#define BG_DG_MAX_TEAM_SCORE 1500

enum BG_DG_ObjectTypes
{
    BG_DG_DOOR_1 = 0,
    BG_DG_DOOR_2,
    BG_DG_DOOR_3,
    BG_DG_DOOR_4,

    BG_DG_CART_ALLIANCE,
    BG_DG_CART_HORDE,

    BG_DG_SPIRIT_MAIN_ALLIANCE,
    BG_DG_SPIRIT_MAIN_HORDE,

    BG_DG_OBJECT_MAX
};

enum BG_DG_UnitTypes
{
    BG_DG_UNIT_FLAG_BOT,
    BG_DG_UNIT_FLAG_MID,
    BG_DG_UNIT_FLAG_TOP,

    BG_DG_SPIRIT_ALLIANCE_BOT,
    BG_DG_SPIRIT_HORDE_BOT,
    BG_DG_SPIRIT_ALLIANCE_TOP,
    BG_DG_SPIRIT_HORDE_TOP,

    BG_DG_UNIT_MAX
};

#define MAX_POINTS 3

enum BG_DG_SPELLS
{
    BG_DG_AURA_NEUTRAL          = 98554,
    BG_DG_AURA_HORDE_CONTEST    = 98545,
    BG_DG_AURA_ALLIANCE_CONTEST = 98543,
    BG_DG_AURA_HORDE_CAPTURED   = 98527,
    BG_DG_AURA_ALLIANCE_CATURED = 98519,

    BG_DG_AURA_CART_HORDE       = 141555,
    BG_DG_AURA_CART_ALLIANCE    = 141551,
    BG_DG_AURA_CARTS_CHAINS     = 141553,

    BG_DG_CAPTURE_SPELL         = 97388,

    BG_DG_SPELL_SPAWN_ALLIANCE_CART = 141554,
    BG_DG_SPELL_SPAWN_HORDE_CART    = 141550,

    BG_DG_AURA_PLAYER_FLAG_HORDE    = 141210,
    BG_DG_AURA_PLAYER_FLAG_ALLIANCE = 140876
};

enum BG_DG_UnitEntry
{
    BG_DG_ENTRY_FLAG                = 53194
};

enum BG_DG_LOCS
{
    BG_DG_LOC_SPIRIT_ALLIANCE_BOT = 4488,
    BG_DG_LOC_SPIRIT_HORDE_BOT    = 4545,
    BG_DG_LOC_SPIRIT_ALLIANCE_TOP = 4546,
    BG_DG_LOC_SPIRIT_HORDE_TOP    = 4489
};

enum BG_DG_ObjectEntry
{
    BG_DG_ENTRY_DOOR_1          = 220159,
    BG_DG_ENTRY_DOOR_2          = 220160,
    BG_DG_ENTRY_DOOR_3          = 220161,
    BG_DG_ENTRY_DOOR_4          = 220366,

    BG_DG_ENTRY_CART_ALLIANCE   = 220164,
    BG_DG_ENTRY_CART_HORDE      = 220166
};

enum PointStates
{
    POINT_STATE_NEUTRAL             = 0,
    POINT_STATE_CONTESTED_ALLIANCE  = 1,
    POINT_STATE_CONTESTED_HORDE     = 2,
    POINT_STATE_CAPTURED            = 3,
    POINT_STATE_CAPTURED_ALLIANCE   = 3,
    POINT_STATE_CAPTURED_HORDE      = 4
};

enum BG_DG_Objectives
{
    DG_OBJECTIVE_CAPTURE_CART   = 457,
    DG_OBJECTIVE_RETURN_CART    = 458,
    DG_OBJECTIVE_CAPTURE_FLAG   = 459,
    DG_OBJECTIVE_DEFENDED_FLAG  = 460,
};

static uint32 const BgDGCartRetured[MAX_TEAMS]       = {73832, 73834};
static uint32 const BgDGCartReturedToBase[MAX_TEAMS] = {73836, 73837};
static uint32 const BgDGCartReady[MAX_TEAMS]         = {73540, 73541};
static uint32 const BgDGCartChecked[MAX_TEAMS]       = {73548, 73549};
static uint32 const BgDGCartCaptured[MAX_TEAMS]      = {74023, 74024};

WorldStates const DgNodeStates[3][4] =
{
    {WorldStates::DG_SHOW_GOBLIN_MINE_ICON,   WorldStates::DG_GOBLIN_MINE_ALLIANCE_ASSAULT,   WorldStates::DG_GOBLIN_MINE_HORDE_ASSAULT,   WorldStates::DG_GOBLIN_MINE_CAPTURED_BY_TEAM},
    {WorldStates::DG_SHOW_MIDDLE_MINE_ICON,   WorldStates::DG_MIDDLE_MINE_ALLIANCE_ASSAULT,   WorldStates::DG_MIDDLE_MINE_HORDE_ASSAULT,   WorldStates::DG_MIDDLE_MINE_CAPTURED_BY_TEAM},
    {WorldStates::DG_SHOW_PANDAREN_MINE_ICON, WorldStates::DG_PANDAREN_MINE_ALLIANCE_ASSAULT, WorldStates::DG_PANDAREN_MINE_HORDE_ASSAULT, WorldStates::DG_PANDAREN_MINE_CAPTURED_BY_TEAM}
};

struct BattlegroundDGScore final : public BattlegroundScore
{
    friend class BattlegroundDeepwindGorge;

    protected:
        BattlegroundDGScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), cartsCaptured(0), cartsDefended(0), pointsCaptured(0), pointsDefended(0) { }

        void UpdateScore(uint32 type, uint32 value) override
        {
            switch (type)
            {
                case SCORE_CARTS_CAPTURED:
                    cartsCaptured += value;
                    break;
                case SCORE_CARTS_DEFENDED:
                    cartsDefended += value;
                    break;
                case SCORE_POINTS_CAPTURED:
                    pointsCaptured += value;
                    break;
                case SCORE_POINTS_DEFENDED:
                    pointsCaptured += value;
                    break;
                default:
                    BattlegroundScore::UpdateScore(type, value);
                    break;
            }
        }

        void BuildObjectivesBlock(std::vector<int32>& stats) override
        {
            stats.push_back(cartsCaptured);
            stats.push_back(cartsDefended);
            stats.push_back(pointsCaptured);
            stats.push_back(pointsDefended);
        }

        uint32 cartsCaptured;
        uint32 cartsDefended;
        uint32 pointsCaptured;
        uint32 pointsDefended;
};

class BattlegroundDeepwindGorge : public Battleground
{
public:
    friend class Point;
    friend class TopPoint;
    friend class BotPoint;

    BattlegroundDeepwindGorge();
    ~BattlegroundDeepwindGorge();

    void AddPlayer(Player* player) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;

    bool UpdatePlayerScore(Player* player, uint32 type, uint32 addvalue, bool doAddHonor) override;

    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;

    void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;
    void _CheckPositions(uint32 diff) override;
    bool SetupBattleground() override;
    void Reset() override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void HandleKillPlayer(Player* player, Player* killer) override;

    void HandlePointCapturing(Player* player, Creature* creature);

    void EventPlayerUsedGO(Player* player, GameObject* go) override;
    void EventPlayerDroppedFlag(Player* Source) override;

    uint32 ModGold(TeamId teamId, int32 val);

    ObjectGuid GetFlagPickerGUID(int32 team) const;
    void GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const override;
    uint8 _GetCapturedNodesForTeam(TeamId teamID);

    uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
    uint32 GetMaxScore() const override { return BG_DG_MAX_TEAM_SCORE; }
    bool IsScoreIncremental() const override { return true; }

private:
        class Point
        {
        public:
            Point(BattlegroundDeepwindGorge* bg);
            ~Point();

            BattlegroundDeepwindGorge* GetBg() { return m_bg; }

            virtual void UpdateState(PointStates state);

            PointStates GetState() { return m_state; }

            void PointClicked(Player* player);
            void Update(Milliseconds diff);

            uint32 TakeGoldCredit() { return m_goldCredit; }

            void SetCreaturePoint(Creature* creature) { m_point = creature; }
            Creature* GetCreaturePoint() { return m_point; }

        protected:
            typedef std::pair<WorldStates, uint32> WorldState;
            WorldState m_currentWorldState;

            PointStates m_state;
            uint32 m_prevAura;

            BattlegroundDeepwindGorge* m_bg;
            Creature* m_point;

            Milliseconds m_timer;

            uint32 m_goldCredit;
        };

        class TopPoint : public Point
        {
        public:
            TopPoint(BattlegroundDeepwindGorge* bg) : Point(bg) {}

            void UpdateState(PointStates state) override;
        };

        class BotPoint : public Point
        {
        public:
            BotPoint(BattlegroundDeepwindGorge* bg): Point(bg) {}

            void UpdateState(PointStates state) override;
        };

        class MiddlePoint : public Point
        {
        public:
            MiddlePoint(BattlegroundDeepwindGorge* bg): Point(bg) {}

            void UpdateState(PointStates state) override;
        };

        class Cart
        {
        public:
            Cart(BattlegroundDeepwindGorge* bg);
            ~Cart() = default;

            void ToggleCaptured(Player* player);
            void CartDropped();
            void CartDelivered();
            void UnbindCartFromPlayer();

            void SetGameObject(GameObject* obj, uint32 id) { m_goCart = obj; m_goBgId = id; }
            GameObject* GetGameObject() { return m_goCart; }

            void SetTeamId(TeamId team) { m_team = team; }
            TeamId GetTeamId() { return m_team; }

            ObjectGuid TakePlayerWhoDroppedFlag() { ObjectGuid v = m_playerDroppedCart; m_playerDroppedCart.Clear(); return v; }

            Player* ControlledBy();
            ObjectGuid  ControlledByPlayerWithGuid() { return m_controlledBy; }

            BattlegroundDeepwindGorge* GetBg() { return m_bg; }
        private:
            BattlegroundDeepwindGorge* m_bg;
            ObjectGuid m_controlledBy;

            GameObject* m_goCart;
            uint32 m_goBgId;

            TeamId m_team;

            ObjectGuid m_playerDroppedCart;
            uint32 m_stolenGold;
        };

private:
    void PostUpdateImpl(Milliseconds diff) override;

        Point* _points[MAX_POINTS];
        Cart*  _carts[MAX_TEAMS];

        int32 _flagsUpdTimer;
        Milliseconds _goldUpdate;
};

#endif

