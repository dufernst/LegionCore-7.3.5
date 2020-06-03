/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITYCORE_AREATRIGGER_H
#define TRINITYCORE_AREATRIGGER_H

#include "GridObject.h"
#include "MapObject.h"
#include "Object.h"

class Unit;
class SpellInfo;
class Spell;
class AreaTriggerAI;

namespace G3D
{
    class Vector2;
    class Vector3;
}
namespace Movement
{
    template<typename length_type>
    class Spline;
}

enum AreaTriggerActionMoment
{
    AT_ACTION_MOMENT_ENTER          = 0x0001, // when unit enters areatrigger
    AT_ACTION_MOMENT_LEAVE          = 0x0002, // when unit exits areatrigger
    AT_ACTION_MOMENT_UPDATE_TARGET  = 0x0004, // on areatrigger update target For sing target spell
    AT_ACTION_MOMENT_DESPAWN        = 0x0008, // when areatrigger despawn use with non AOE spell
    AT_ACTION_MOMENT_SPAWN          = 0x0010, // when areatrigger spawn
    AT_ACTION_MOMENT_REMOVE         = 0x0020, // when areatrigger remove
    //range should be = distance.
    AT_ACTION_MOMENT_ON_THE_WAY     = 0x0040, // when target is betwin source and dest points. For movement only. WARN! Should add AT_ACTION_MOMENT_ENTER flag too
    AT_ACTION_MOMENT_ON_STOP_MOVE   = 0x0080, // when target is betwin source and dest points. For movement only. WARN! Should add AT_ACTION_MOMENT_ENTER flag too
    AT_ACTION_MOMENT_ON_ACTIVATE    = 0x0100, // when areatrigger active
    AT_ACTION_MOMENT_ON_CAST_ACTION = 0x0200, // when areatrigger cast from aura tick
    AT_ACTION_MOMENT_UPDATE         = 0x0400, // on areatrigger update For AOE spell
    AT_ACTION_MOMENT_ON_DESPAWN     = 0x0800, // on areatrigger despawn use with AOE spell
    AT_ACTION_MOMENT_LEAVE_ALL      = 0x1000, // when all units exits areatrigger
};

enum AreaTriggerActionType
{
    AT_ACTION_TYPE_CAST_SPELL               = 0,
    AT_ACTION_TYPE_REMOVE_AURA              = 1,
    AT_ACTION_TYPE_ADD_STACK                = 2,
    AT_ACTION_TYPE_REMOVE_STACK             = 3,
    AT_ACTION_TYPE_CHANGE_SCALE             = 4,
    AT_ACTION_TYPE_SHARE_DAMAGE             = 5,
    AT_ACTION_TYPE_APPLY_MOVEMENT_FORCE     = 6,
    AT_ACTION_TYPE_REMOVE_MOVEMENT_FORCE    = 7,
    AT_ACTION_TYPE_CHANGE_DURATION_ANY_AT   = 8,
    AT_ACTION_TYPE_CHANGE_AMOUNT_FROM_HEALT = 9,
    AT_ACTION_TYPE_RE_PATCH_TO_CASTER       = 10,
    AT_ACTION_TYPE_SET_AURA_CUSTOM_ADD      = 11,
    AT_ACTION_TYPE_SET_AURA_CUSTOM_REMOVE   = 12,
    AT_ACTION_TYPE_REMOVE_AURA_BY_CASTER    = 13,
    AT_ACTION_TYPE_CAST_SPELL_NOT_TRIGGER   = 14,
    AT_ACTION_TYPE_NO_ACTION                = 15,
    AT_ACTION_TYPE_RE_PATCH                 = 16,
    AT_ACTION_TYPE_REMOVE_OWNED_AURA        = 17,
    AT_ACTION_TYPE_OWNER_CAST_SPELL         = 18,
    AT_ACTION_TYPE_RE_PATCH_SCRIPT          = 19,
    AT_ACTION_TYPE_CASTER_GUID_REMOVE_AURA  = 20, //Only remove GUID caster aura
    AT_ACTION_TYPE_MAX                      = 21
};

enum AreaTriggerTargetFlags
{
    AT_TARGET_FLAG_FRIENDLY             = 0x0000001,             // casted on targets that are friendly to areatrigger owner
    AT_TARGET_FLAG_VALIDATTACK          = 0x0000002,             // casted on targets that are valid attcak to areatrigger owner
    AT_TARGET_FLAG_OWNER                = 0x0000004,             // casted only on areatrigger owner
    AT_TARGET_FLAG_PLAYER               = 0x0000008,             // casted only on players
    AT_TARGET_FLAG_NOT_PET              = 0x0000010,             // casted on everyone except pets
    AT_TARGET_FLAG_CAST_AT_SRC          = 0x0000020,             // casted on areatrigger position as dest
    AT_TARGET_FLAG_CASTER_IS_TARGET     = 0x0000040,             // casted on areatrigger caster is target
    AT_TARGET_FLAG_NOT_FULL_HP          = 0x0000080,             // casted on targets if not full hp
    AT_TARGET_FLAG_ALWAYS_TRIGGER       = 0x0000100,             // casted always at any action on owner
    AT_TARGET_FLAT_IN_FRONT             = 0x0000200,             // WARNING! If target come from back he not get cast. ToDo it..
    AT_TARGET_FLAG_NOT_FULL_ENERGY      = 0x0000400,             // casted on targets if not full enegy
    AT_TARGET_FLAG_GROUP_OR_RAID        = 0x0000800,             // casted on targets that in group to areatrigger owner
    AT_TARGET_FLAG_HOSTILE              = 0x0001000,             // casted on targets that are hostile to areatrigger owner
    AT_TARGET_FLAG_TARGET_IS_CASTER     = 0x0002000,             // casted on areatrigger target is caster
    AT_TARGET_FLAG_CAST_AURA_TARGET     = 0x0004000,             // casted on aura target
    AT_TARGET_FLAG_NOT_AURA_TARGET      = 0x0008000,             // casted on target is not aura target
    AT_TARGET_FLAG_TARGET_IS_SUMMONER   = 0x0010000,
    AT_TARGET_FLAG_NOT_OWNER            = 0x0020000,
    AT_TARGET_FLAG_NPC_ENTRY            = 0x0040000,
    AT_TARGET_FLAG_TARGET_PASSANGER     = 0x0080000,
    AT_TARGET_FLAG_TARGET_PASSANGER_VEH = 0x0100000,
    AT_TARGET_FLAG_TARGET_IS_CASTER_2   = 0x0200000,
    AT_TARGET_FLAG_SCRIPT               = 0x0400000,
    AT_TARGET_FLAG_CASTER_AURA_TARGET   = 0x0800000,             // casted from aura target
    AT_TARGET_FLAG_NOT_IN_LOS           = 0x1000000,
    AT_TARGET_FLAG_NOT_IN_LOS_Z         = 0x2000000,

    AT_TARGET_MASK_REQUIRE_TARGET = 
        AT_TARGET_FLAG_FRIENDLY | AT_TARGET_FLAG_HOSTILE | AT_TARGET_FLAG_OWNER | AT_TARGET_FLAG_PLAYER |
        AT_TARGET_FLAG_NOT_PET  | AT_TARGET_FLAG_CASTER_IS_TARGET | AT_TARGET_FLAG_NOT_FULL_HP | AT_TARGET_FLAG_ALWAYS_TRIGGER | AT_TARGET_FLAT_IN_FRONT,
};

enum AreaTriggerMoveType
{
    AT_MOVE_TYPE_DEFAULT             = 0,
    AT_MOVE_TYPE_LIMIT_TO_TARGET     = 1,
    AT_MOVE_TYPE_SPIRAL              = 2,
    AT_MOVE_TYPE_BOOMERANG           = 3,
    AT_MOVE_TYPE_CHAGE_ROTATION      = 4,
    AT_MOVE_TYPE_RE_PATH             = 5,
    AT_MOVE_TYPE_RANDOM              = 6,
    AT_MOVE_TYPE_ANGLE_TO_CASTER     = 7,
    AT_MOVE_TYPE_RE_PATH_LOS         = 8,
    AT_MOVE_TYPE_PART_PATH           = 9,
    AT_MOVE_TYPE_MOVE_FORWARD        = 10,
    AT_MOVE_TYPE_RE_PATH_TO_CASTER   = 11,
    AT_MOVE_TYPE_RE_PATH_TO_TARGET   = 12,
    AT_MOVE_TYPE_MAX,
};

struct AreaTriggerPolygon
{
    std::vector<TaggedPosition<Position::XY>> Vertices;
    std::vector<TaggedPosition<Position::XY>> VerticesTarget;
    float Height = 0.0f;
    float HeightTarget = 0.0f;
};

struct AreaTriggerCircle
{
    Optional<ObjectGuid> PathTarget;
    Optional<TaggedPosition<Position::XYZ>> Center;
    uint32 TimeToTarget = 0;
    int32 ElapsedTimeForMovement = 0;
    uint32 StartDelay = 0;
    float Radius = 0.0f;
    float BlendFromRadius = 0.0f;
    float InitialAngle = 0.0f;
    float ZOffset = 0.0f;
    bool CounterClockwise = false;
    bool CanLoop = false;
};

struct AreaTriggerTemplateCircle
{
    float Radius = 0.0f;
    float Speed = 0.0f;
    bool HasTarget = false;
    bool HasCenterPoint = false;
    bool CounterClockwise = false;
    bool CanLoop = false;
    bool RandRevers = false;
    bool IsDinamicRadius = false;
};

struct AreaTriggerTemplateSequence
{
    bool oncreated = false;
    bool entered = false;
    uint32 animationid = 0;
    uint32 timer1 = 0;
    bool entered1 = false;
    uint32 animationid1 = 0;
    uint32 timer2 = 0;
    bool entered2 = false;
    uint32 animationid2 = 0;
    bool cycle = false;
};

struct AreaTriggerSpline
{
    int32 TimeToTarget = 0;
    int32 ElapsedTimeForMovement = 0;
    std::vector<G3D::Vector3> VerticesPoints;
};

struct AreaTriggerScaleData
{
    AreaTriggerScaleData();

    union
    {
        int32 integerValue;
        float floatValue;
    } OverrideScale[7];

    union
    {
        int32 integerValue;
        float floatValue;
    } ExtraScale[7];

    uint32 timeToTargetScale;
};

struct AreaTriggerAction
{
    uint32 id;
    AreaTriggerActionMoment moment;
    AreaTriggerActionType actionType;
    AreaTriggerTargetFlags targetFlags;
    uint32 spellId;
    uint32 chargeRecoveryTime;
    float scaleStep;
    float scaleMin;
    float scaleMax;
    bool scaleVisualUpdate;
    int32 hasAura;
    int32 hasAura2;
    int32 hasAura3;
    int32 auraCaster;
    int32 hasspell;
    int32 hitMaxCount;
    int32 amount;
    int8 maxCharges;
    bool onDespawn;
    float minDistance;
};

typedef std::list<AreaTriggerAction> AreaTriggerActionList;

struct AreaTriggerInfo
{
    AreaTriggerInfo();

    G3D::AABox box;
    AreaTriggerPolygon Polygon;
    AreaTriggerActionList actions;
    AreaTriggerTemplateCircle circleTemplate;
    AreaTriggerTemplateSequence sequenceTemplate;
    float Radius;
    float RadiusTarget;
    float RandomRadiusOfSpawn;
    float LocationZOffset;
    float LocationZOffsetTarget;
    float windX;
    float windY;
    float windZ;
    float windSpeed;
    float RollPitchYaw1X;
    float RollPitchYaw1Y;
    float RollPitchYaw1Z;
    float TargetRollPitchYawX;
    float TargetRollPitchYawY;
    float TargetRollPitchYawZ;
    float Distance;
    float Speed;
    float RePatchSpeed;
    float AngleToCaster;
    float AnglePointA;
    float AnglePointB;
    float Param;
    uint32 spellId;
    uint32 DecalPropertiesId;
    uint32 activationDelay;
    uint32 updateDelay;
    uint32 customEntry;
    uint32 moveType;
    uint32 waitTime;
    uint32 hitType;
    uint32 MoveCurveID;
    uint32 ElapsedTime;
    uint32 MorphCurveID;
    uint32 FacingCurveID;
    uint32 ScaleCurveID;
    uint32 HasFollowsTerrain;
    uint32 HasAttached;
    uint32 HasAbsoluteOrientation;
    uint32 HasDynamicShape;
    uint32 HasFaceMovementDir;
    uint32 hasAreaTriggerBox;
    uint32 windType;
    uint32 polygon;
    int32 VisualID;
    uint8 maxCount;
    uint8 maxActiveTargets;
    bool isMoving;
    bool RePatch;
    bool isCircle = false;
    bool isSequence = false;
    bool OnDestinationReachedDespawn;
    bool WithObjectSize;
    bool AliveOnly;
    bool AllowBoxCheck;
};

class AreaTrigger : public WorldObject, public GridObject<AreaTrigger>, public MapObject
{
        struct ActionInfo
        {
            ActionInfo();
            ActionInfo(AreaTriggerAction const* _action);

            uint32 hitCount;
            uint8 charges;
            uint32 recoveryTime;
            int32 amount;
            bool onDespawn;
            AreaTriggerAction const* action;
        };
        typedef std::map<uint8, ActionInfo> ActionInfoMap;

    public:

        AreaTrigger();
        ~AreaTrigger();

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void AI_Initialize();
        void AI_Destroy();

        AreaTriggerAI* AI();

        bool CreateAreaTrigger(ObjectGuid::LowType guidlow, uint32 triggerEntry, Unit* caster, SpellInfo const* info, Position const& pos, Position const& posMove, Spell* spell = nullptr, ObjectGuid targetGuid = ObjectGuid::Empty, uint32 customEntry = 0, ObjectGuid castID = ObjectGuid::Empty, Map* map = nullptr);
        void Update(uint32 p_time) override;
        void UpdateAffectedList(uint32 p_time, AreaTriggerActionMoment actionM);
        void ActionOnUpdate(uint32 p_time);
        void ActionOnDespawn();
        void Remove(bool duration = true);
        void Despawn();
        uint32 GetSpellId() const;
        void SetSpellId(uint32 spell);
        void SetRadius(float radius);
        ObjectGuid GetCasterGUID() const;
        Unit* GetCaster() const;
        void SetTargetGuid(ObjectGuid targetGuid);
        ObjectGuid GetTargetGuid() const;
        Unit* GetTarget() const;
        int32 GetDuration() const;
        void SetDuration(int32 newDuration);
        int32 GeTimeToTarget() const;
        void SetTimeToTarget(int32 timeToTarget);
        void Delay(int32 delaytime);
        float GetVisualScale(bool max = false) const;
        float GetRadius() const;
        void CalculateRadius(Spell* spell = nullptr);
        uint32 GetCustomEntry() const;
        uint32 GetRealEntry() const;
        bool IsUnitAffected(ObjectGuid guid) const;
        void AffectUnit(Unit* unit, AreaTriggerActionMoment actionM);
        void AffectUnitLeave(AreaTriggerActionMoment actionM);
        void AffectOwner(AreaTriggerActionMoment actionM);
        void DoAction(Unit* unit, ActionInfo& action);
        void DoActionLeave(ActionInfo& action);
        bool CheckActionConditions(AreaTriggerAction const& action, Unit* unit);
        void UpdateActionCharges(uint32 p_time);
        bool GetAreaTriggerCylinder() const;
        bool HasTargetRollPitchYaw() const;
        bool HasPolygon() const;
        bool HasCircleData() const;
        AreaTriggerCircle const* GetCircleData() const;
        bool CheckValidateTargets(Unit* unit, AreaTriggerActionMoment actionM);

        AreaTriggerInfo GetAreaTriggerInfo() const;
        void CastAction();
        bool UpdatePosition(ObjectGuid targetGuid);
        void CalculateSplinePosition(Position const& pos, Position const& posMove, Unit* caster);
        void ReCalculateSplinePosition(bool setReach = false);
        void CalculateCyclicPosition(Position const& pos, Position const& posMove, Unit* caster);

        void BindToCaster();
        void UnbindFromCaster();

        SpellInfo const* GetSpellInfo();
        SpellValue const* GetSpellValue();

        //scaling
        void UpdateScale(uint32 p_time);
        void UpdateOverrideScale();
        void SetSphereScale(float mod, uint32 time, bool absolute_change = false, float scaleMin = 0.1f, float scaleMax = 100.0f, bool updateOverride = true);

        // Sequence
        void UpdateSequence(uint32 p_time);

        //movement
        void UpdateSplinePosition(uint32 diff);
        void UpdateRotation(uint32 diff);
        bool isMoving() const;
        void SetMoving(bool active) { _canMove = active; }
        bool HasSpline() const;
        bool IsInArea(Unit* unit);
        bool IsInHeight(Unit* unit);
        bool IsInBox(Unit* unit);
        bool IsInPolygon(Unit* target);
        float CalculateRadiusPolygon();
        AreaTriggerSpline GetSplineInfo() const;
        GuidList* GetAffectedPlayers();
        GuidList* GetAffectedPlayersForAllTime();
        void SendReShape(Position const* pos);
        void MoveTo(Position const* pos);
        void MoveTo(Unit* target);
        void MoveTo(float x, float y, float z);
        void SetPolygonVertices(uint32 index, bool editX, float x, bool editY = false, float y = 0.f);

        void GetCollisionPosition(Position &_dest, float dist, float angle);

        void InitSplines();

        void DebugVisualizePosition(); // Debug purpose only
        void DebugVisualizePolygon(); // Debug purpose only
        float GetProgress() const;

        float _range;
        Item* m_CastItem;
        Aura* m_aura;
        std::vector<bool> ActivePointPolygon;
        std::vector<Creature*> DebugPolygon;

    private:
        bool _HasActionsWithCharges(AreaTriggerActionMoment action = AT_ACTION_MOMENT_ENTER);
        void FillCustomData(Unit* caster);

    protected:
        Unit* _caster;
        ObjectGuid _targetGuid;
        int32 _duration;
        uint32 _activationDelay;
        uint32 _updateDelay;
        uint32 _scaleDelay;
        uint32 _sequenceDelay;
        uint32 _sequenceStep;
        uint32 _liveTime;
        GuidList affectedPlayers;
        GuidList affectedPlayersForAllTime;
        float _radius;
        AreaTriggerInfo atInfo;
        ActionInfoMap _actionInfo;
        uint32 _realEntry;
        bool _reachedDestination;
        int32 _lastSplineIndex;
        uint32 _movementTime;
        uint32 _nextMoveTime;
        uint32 _waitTime;
        bool _on_unload;
        bool _on_despawn;
        std::atomic<bool> _on_remove;
        uint32 _hitCount;
        bool _areaTriggerCylinder;
        float m_moveAngleLos;
        bool _canMove;
        uint32 _currentWP;

        // Movement info
        Movement::MoveSpline* movespline;
        AreaTriggerSpline _spline;
        AreaTriggerCircle* _CircleData;
        AreaTriggerScaleData _scaleData;

        SpellInfo const* m_spellInfo;
        Spell* m_spell;

        std::unique_ptr<AreaTriggerAI> _ai;
        bool m_withoutCaster;
};
#endif
