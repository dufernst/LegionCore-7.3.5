#include "ZoneScript.h"
#include "Creature.h"
#include "WorldStatePackets.h"

ZoneScript::ZoneScript(): m_type(ZONE_TYPE_MAP) { }

void ZoneScript::SetType(uint8 type)
{
    m_type = type;
}

uint32 ZoneScript::GetCreatureEntry(uint32, CreatureData const* data)
{
    return data->id;
}

InstanceScript* ZoneScript::ToInstanceScript()
{
    if (m_type == ZONE_TYPE_INSTANCE)
        return reinterpret_cast<InstanceScript*>(this);
    return nullptr;
}
