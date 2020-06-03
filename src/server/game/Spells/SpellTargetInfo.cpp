
#include "SpellTargetInfo.h"
#include "SpellPackets.h"
#include "TradeData.h"

SpellDestination::SpellDestination()
{
    _position.Relocate(0, 0, 0, 0);
    _transportGUID.Clear();
    _transportOffset.Relocate(0, 0, 0, 0);
}

SpellDestination::SpellDestination(float x, float y, float z, float orientation, uint32 mapId)
{
    _position.Relocate(x, y, z, orientation);
    _transportGUID.Clear();
    _position.m_mapId = mapId;
    _transportOffset.Relocate(0, 0, 0, 0);
}

SpellDestination::SpellDestination(Position const& pos)
{
    _position.Relocate(pos);
    _transportGUID.Clear();
    _transportOffset.Relocate(0, 0, 0, 0);
}

SpellDestination::SpellDestination(WorldObject const& wObj)
{
    _transportGUID = wObj.GetTransGUID();
    _transportOffset.Relocate(wObj.GetTransOffset());
    _position.Relocate(wObj);
    _position.SetOrientation(wObj.GetOrientation());
}

TargetInfo::TargetInfo(ObjectGuid tGUID, uint32 effMask) : TargetInfo()
{
    targetGUID = tGUID;
    effectMask = effMask;
}

TargetInfo::TargetInfo()
{
    targetGUID = ObjectGuid::Empty;
    missCondition = SPELL_MISS_NONE;
    reflectResult = SPELL_MISS_NONE;
    timeDelay = 0LL;
    effectMask = 0;
    targetInfoMask = 0;
    processed = false;
    damage = 0;
    damageBeforeHit = 0;
    scaleAura = false;
}

bool TargetInfo::HasMask(uint32 Mask) const
{
    return targetInfoMask & Mask;
}

uint32 TargetInfo::GetMask() const
{
    return targetInfoMask;
}

void TargetInfo::AddMask(uint32 Mask)
{
    targetInfoMask |= Mask;
}

SpellCastTargets::SpellCastTargets() : m_objectTarget(nullptr), m_caster(nullptr), m_itemTarget(nullptr), m_itemTargetEntry(0), m_targetMask(0), m_pitch(0), m_speed(0) { }

SpellCastTargets::SpellCastTargets(Unit* caster, WorldPackets::Spells::SpellCastRequest const& spellCastRequest) : m_objectTarget(nullptr), m_caster(caster),
m_itemTarget(nullptr), m_itemTargetGUID(spellCastRequest.Target.Item), m_objectTargetGUID(spellCastRequest.Target.Unit), m_itemTargetEntry(0), m_targetMask(spellCastRequest.Target.Flags), m_pitch(0.0f), m_speed(0.0f), m_strTarget(spellCastRequest.Target.Name)
{
    if (spellCastRequest.Target.SrcLocation)
    {
        m_src._transportGUID = spellCastRequest.Target.SrcLocation->Transport;
        Position* pos;
        if (!m_src._transportGUID.IsEmpty())
            pos = &m_src._transportOffset;
        else
            pos = &m_src._position;

        pos->Relocate(spellCastRequest.Target.SrcLocation->Location);
        if (spellCastRequest.Target.Orientation)
            pos->SetOrientation(*spellCastRequest.Target.Orientation);
    }

    if (spellCastRequest.Target.DstLocation)
    {
        m_dst._transportGUID = spellCastRequest.Target.DstLocation->Transport;
        Position* pos;
        if (!m_dst._transportGUID.IsEmpty())
            pos = &m_dst._transportOffset;
        else
            pos = &m_dst._position;

        if (m_caster && m_dst._position.GetMapId() == MAPID_INVALID)
            m_dst._position.SetMapId(m_caster->GetMapId());

        pos->Relocate(spellCastRequest.Target.DstLocation->Location);
        if (spellCastRequest.Target.Orientation)
            pos->SetOrientation(*spellCastRequest.Target.Orientation);
    }

    SetPitch(spellCastRequest.MissileTrajectory.Pitch);
    SetSpeed(spellCastRequest.MissileTrajectory.Speed);

    Update(caster);
}

SpellCastTargets::~SpellCastTargets() = default;

ObjectGuid SpellCastTargets::GetUnitTargetGUID() const
{
    switch (m_objectTargetGUID.GetHigh())
    {
    case HighGuid::Player:
    case HighGuid::Vehicle:
    case HighGuid::Creature:
    case HighGuid::Pet:
        return m_objectTargetGUID;
    default:
        return ObjectGuid::Empty;
    }
}

Unit* SpellCastTargets::GetUnitTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToUnit();
    return nullptr;
}

void SpellCastTargets::SetUnitTarget(Unit* target)
{
    if (!target)
        return;

    m_objectTarget = target;
    m_objectTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_UNIT;
}

ObjectGuid SpellCastTargets::GetGOTargetGUID() const
{
    switch (m_objectTargetGUID.GetHigh())
    {
    case HighGuid::Transport:
    case HighGuid::GameObject:
        return m_objectTargetGUID;
    default:
        return ObjectGuid::Empty;
    }
}

GameObject* SpellCastTargets::GetGOTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToGameObject();
    return nullptr;
}

void SpellCastTargets::SetGOTarget(GameObject* target)
{
    if (!target)
        return;

    m_objectTarget = target;
    m_objectTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_GAMEOBJECT;
}

ObjectGuid SpellCastTargets::GetCorpseTargetGUID() const
{
    switch (m_objectTargetGUID.GetHigh())
    {
    case HighGuid::Corpse:
        return m_objectTargetGUID;
    default:
        return ObjectGuid::Empty;
    }
}

Corpse* SpellCastTargets::GetCorpseTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToCorpse();
    return nullptr;
}

WorldObject* SpellCastTargets::GetObjectTarget() const
{
    return m_objectTarget;
}

ObjectGuid SpellCastTargets::GetObjectTargetGUID() const
{
    return m_objectTargetGUID;
}

void SpellCastTargets::RemoveObjectTarget()
{
    m_objectTarget = nullptr;
    m_objectTargetGUID.Clear();
    m_targetMask &= ~(TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK | TARGET_FLAG_GAMEOBJECT_MASK);
}

void SpellCastTargets::SetItemTarget(Item* item)
{
    if (!item)
        return;

    m_itemTarget = item;
    m_itemTargetGUID = item->GetGUID();
    m_itemTargetEntry = item->GetEntry();
    m_targetMask |= TARGET_FLAG_ITEM;
}

void SpellCastTargets::SetTradeItemTarget(Player* caster)
{
    m_itemTargetGUID.SetRawValue({ uint8(TRADE_SLOT_NONTRADED), 0, 0, 0, 0, 0, 0, 0 });
    m_itemTargetEntry = 0;
    m_targetMask |= TARGET_FLAG_TRADE_ITEM;

    Update(caster);
}

void SpellCastTargets::UpdateTradeSlotItem()
{
    if (m_itemTarget && (m_targetMask & TARGET_FLAG_TRADE_ITEM))
    {
        m_itemTargetGUID = m_itemTarget->GetGUID();
        m_itemTargetEntry = m_itemTarget->GetEntry();
    }
}

SpellDestination const* SpellCastTargets::GetSrc() const
{
    return &m_src;
}

Position const* SpellCastTargets::GetSrcPos() const
{
    return &m_src._position;
}

void SpellCastTargets::SetSrc(float x, float y, float z)
{
    m_src = SpellDestination(x, y, z);
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::SetSrc(Position const& pos)
{
    m_src = SpellDestination(pos);
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::SetSrc(WorldObject const& wObj)
{
    m_src = SpellDestination(wObj);
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::ModSrc(Position const& pos)
{
    ASSERT(m_targetMask & TARGET_FLAG_SOURCE_LOCATION);

    if (m_src._transportGUID)
    {
        Position offset;
        m_src._position.GetPositionOffsetTo(pos, offset);
        m_src._transportOffset.RelocateOffset(offset);
    }
    m_src._position.Relocate(pos);
}

void SpellCastTargets::RemoveSrc()
{
    m_targetMask &= ~(TARGET_FLAG_SOURCE_LOCATION);
}

SpellDestination const* SpellCastTargets::GetDst() const
{
    return &m_dst;
}

WorldLocation const* SpellCastTargets::GetDstPos() const
{
    return &m_dst._position;
}

void SpellCastTargets::SetDst(float x, float y, float z, float orientation, uint32 mapId)
{
    m_dst = SpellDestination(x, y, z, orientation, mapId);
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::SetDst(Position const& pos)
{
    m_dst = SpellDestination(pos);
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    if (m_caster && m_dst._position.GetMapId() == MAPID_INVALID)
        m_dst._position.SetMapId(m_caster->GetMapId());
}

void SpellCastTargets::SetDst(WorldObject const& wObj)
{
    m_dst = SpellDestination(wObj);
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    if (m_caster)
        m_dst._position.SetMapId(m_caster->GetMapId());
}

void SpellCastTargets::SetDst(SpellCastTargets const& spellTargets)
{
    m_dst = spellTargets.m_dst;
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
    if (m_caster && m_dst._position.GetMapId() == MAPID_INVALID)
        m_dst._position.SetMapId(m_caster->GetMapId());
}

void SpellCastTargets::ModDst(Position const& pos)
{
    ASSERT(m_targetMask & TARGET_FLAG_DEST_LOCATION);

    if (m_dst._transportGUID)
    {
        Position offset;
        m_dst._position.GetPositionOffsetTo(pos, offset);
        m_dst._transportOffset.RelocateOffset(offset);
    }
    m_dst._position.Relocate(pos);
    if (m_caster && m_dst._position.GetMapId() == MAPID_INVALID)
        m_dst._position.SetMapId(m_caster->GetMapId());
}

void SpellCastTargets::RemoveDst()
{
    m_targetMask &= ~(TARGET_FLAG_DEST_LOCATION);
}

void SpellCastTargets::Update(Unit* caster)
{
    m_objectTarget = m_objectTargetGUID ? ((m_objectTargetGUID == caster->GetGUID()) ? caster : ObjectAccessor::GetWorldObject(*caster, m_objectTargetGUID)) : nullptr;

    m_itemTarget = nullptr;
    if (caster->IsPlayer())
    {
        auto player = caster->ToPlayer();
        if (m_targetMask & TARGET_FLAG_ITEM)
            m_itemTarget = player->GetItemByGuid(m_itemTargetGUID);
        else if (m_targetMask & TARGET_FLAG_TRADE_ITEM)
        {
            ObjectGuid nonTradedGuid;
            nonTradedGuid.SetRawValue(uint64(0), uint64(TRADE_SLOT_NONTRADED));
            if (m_itemTargetGUID == nonTradedGuid)
                if (auto pTrade = player->GetTradeData())
                    m_itemTarget = pTrade->GetTraderData()->GetItem(TRADE_SLOT_NONTRADED);
        }

        if (m_itemTarget)
            m_itemTargetEntry = m_itemTarget->GetEntry();
    }

    if (HasSrc() && m_src._transportGUID)
    {
        if (auto transport = ObjectAccessor::GetWorldObject(*caster, m_src._transportGUID))
        {
            m_src._position.Relocate(transport);
            m_src._position.RelocateOffset(m_src._transportOffset);
        }
    }

    if (HasDst() && m_dst._transportGUID)
    {
        if (auto transport = ObjectAccessor::GetWorldObject(*caster, m_dst._transportGUID))
        {
            m_dst._position.Relocate(transport);
            m_dst._position.RelocateOffset(m_dst._transportOffset);
        }
    }
}

void SpellCastTargets::Write(WorldPackets::Spells::SpellTargetData& data)
{
    data.Flags = m_targetMask;

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_UNIT_MINIPET))
        data.Unit = m_objectTargetGUID;

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM) && m_itemTarget)
        data.Item = m_itemTarget->GetGUID();
    else
        data.Flags &= ~(TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM);

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
        data.SrcLocation = boost::in_place();
        data.SrcLocation->Transport = m_src._transportGUID;
        if (m_src._transportGUID.IsEmpty())
            data.SrcLocation->Location = static_cast<Position>(m_src._position);
        else
            data.SrcLocation->Location = m_src._transportOffset;
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        data.DstLocation = boost::in_place();
        data.DstLocation->Transport = m_dst._transportGUID;
        if (m_dst._transportGUID.IsEmpty())
            data.DstLocation->Location = static_cast<Position>(m_dst._position);
        else
            data.DstLocation->Location = m_dst._transportOffset;

        data.Orientation = m_dst._position.GetOrientation();
        data.MapID = m_dst._position.GetMapId();
    }

    if (m_targetMask & TARGET_FLAG_STRING)
        data.Name = m_strTarget;
}

uint32 SpellCastTargets::GetTargetMask() const
{
    return m_targetMask;
}

void SpellCastTargets::SetTargetMask(uint32 newMask)
{
    m_targetMask = newMask;
}

void SpellCastTargets::SetTargetFlag(SpellCastTargetFlags flag)
{
    m_targetMask |= flag;
}

ObjectGuid SpellCastTargets::GetItemTargetGUID() const
{
    return m_itemTargetGUID;
}

Item* SpellCastTargets::GetItemTarget() const
{
    return m_itemTarget;
}

uint32 SpellCastTargets::GetItemTargetEntry() const
{
    return m_itemTargetEntry;
}

bool SpellCastTargets::HasSrc() const
{
    return (GetTargetMask() & TARGET_FLAG_SOURCE_LOCATION) != 0;
}

bool SpellCastTargets::HasDst() const
{
    return (GetTargetMask() & TARGET_FLAG_DEST_LOCATION) != 0;
}

bool SpellCastTargets::HasTraj() const
{
    return m_speed != 0;
}

float SpellCastTargets::GetPitch() const
{
    return m_pitch;
}

void SpellCastTargets::SetPitch(float elevation)
{
    m_pitch = elevation;
}

float SpellCastTargets::GetSpeed() const
{
    return m_speed;
}

void SpellCastTargets::SetSpeed(float speed)
{
    m_speed = speed;
}

float SpellCastTargets::GetDist2d() const
{
    return m_src._position.GetExactDist2d(&m_dst._position);
}

float SpellCastTargets::GetSpeedXY() const
{
    return m_speed * std::cos(m_pitch);
}

float SpellCastTargets::GetSpeedZ() const
{
    return m_speed * std::sin(m_pitch);
}

ArchaeologyWeights const& SpellCastTargets::GetWeights()
{
    return m_weights;
}

void SpellCastTargets::SetCaster(Unit* caster)
{
    m_caster = caster;
}

void SpellCastTargets::OutDebug() const
{
    if (!m_targetMask)
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "No targets");

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "target mask: %u", m_targetMask);
    if (m_targetMask & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK | TARGET_FLAG_GAMEOBJECT_MASK))
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Object target: %s", m_objectTargetGUID.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_ITEM)
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Item target: %s", m_itemTargetGUID.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_TRADE_ITEM)
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Trade item target: %s", m_itemTargetGUID.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Source location: transport guid: %s trans offset: %s position: %s", m_src._transportGUID.ToString().c_str(), m_src._transportOffset.ToString().c_str(), m_src._position.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Destination location: transport guid: %s trans offset: %s position: %s", m_dst._transportGUID.ToString().c_str(), m_dst._transportOffset.ToString().c_str(), m_dst._position.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_STRING)
        TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "String: %s", m_strTarget.c_str());

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "speed: %f", m_speed);
    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "elevation: %f", m_pitch);
}
