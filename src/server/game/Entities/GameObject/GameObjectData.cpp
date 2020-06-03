
#include "GameObjectData.h"

bool GameObjectTemplate::IsDespawnAtAction() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_CHEST: 
        return chest.consumable != 0;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.consumable != 0 || goober.cooldown != 0;
    default: 
        return false;
    }
}

bool GameObjectTemplate::IsUsableMounted() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_QUESTGIVER: 
        return questgiver.allowMounted;
    case GAMEOBJECT_TYPE_TEXT: 
        return text.allowMounted;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.allowMounted;
    case GAMEOBJECT_TYPE_SPELLCASTER: 
        return spellCaster.allowMounted;
    case GAMEOBJECT_TYPE_UI_LINK: 
        return UILink.allowMounted;
    default: 
        return false;
    }
}

uint32 GameObjectTemplate::GetLockId() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_DOOR: 
        return door.open;
    case GAMEOBJECT_TYPE_BUTTON: 
        return button.open;
    case GAMEOBJECT_TYPE_QUESTGIVER: 
        return questgiver.open;
    case GAMEOBJECT_TYPE_CHEST: 
        return chest.open;
    case GAMEOBJECT_TYPE_TRAP: 
        return trap.open;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.open;
    case GAMEOBJECT_TYPE_AREADAMAGE: 
        return areaDamage.open;
    case GAMEOBJECT_TYPE_CAMERA: 
        return camera.open;
    case GAMEOBJECT_TYPE_FLAGSTAND: 
        return flagStand.open;
    case GAMEOBJECT_TYPE_FISHINGHOLE: 
        return fishingHole.open;
    case GAMEOBJECT_TYPE_FLAGDROP: 
        return flagDrop.open;
    case GAMEOBJECT_TYPE_NEW_FLAG: 
        return newflag.open;
    case GAMEOBJECT_TYPE_NEW_FLAG_DROP: 
        return newflagdrop.open;
    case GAMEOBJECT_TYPE_CAPTURE_POINT: 
        return capturePoint.open;
    case GAMEOBJECT_TYPE_GATHERING_NODE: 
        return gatheringNode.open;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetXpLevel() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_CHEST: 
        return chest.xpLevel;
    case GAMEOBJECT_TYPE_GATHERING_NODE: 
        return gatheringNode.xpLevel;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetVignetteId() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_CHEST:
        return chest.SpawnVignette;
    case GAMEOBJECT_TYPE_GOOBER:
        return goober.SpawnVignette;
    case GAMEOBJECT_TYPE_GATHERING_NODE:
        return gatheringNode.SpawnVignette;
    case GAMEOBJECT_TYPE_CAPTURE_POINT:
        return capturePoint.SpawnVignette;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetTrackingQuestId() const
{
    uint32 playerConditionID = 0;
    switch (type)
    {
    case GAMEOBJECT_TYPE_CHEST:
        playerConditionID = chest.conditionID1;
        break;
    default:
        break;
    }

    if (auto playerCondition = sPlayerConditionStore[playerConditionID])
        return playerCondition->PrevQuestID[1];
    return 0;
}

bool GameObjectTemplate::GetDespawnPossibility() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_DOOR: 
        return door.noDamageImmune != 0;
    case GAMEOBJECT_TYPE_BUTTON: 
        return button.noDamageImmune != 0;
    case GAMEOBJECT_TYPE_QUESTGIVER: 
        return questgiver.noDamageImmune != 0;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.noDamageImmune != 0;
    case GAMEOBJECT_TYPE_FLAGSTAND: 
        return flagStand.noDamageImmune != 0;
    case GAMEOBJECT_TYPE_FLAGDROP: 
        return flagDrop.noDamageImmune != 0;
    default: 
        return true;
    }
}

uint32 GameObjectTemplate::GetCharges() const
{
    switch (type)
    {
        //case GAMEOBJECT_TYPE_TRAP:        return trap.charges;
    case GAMEOBJECT_TYPE_GUARDPOST: 
        return guardPost.charges;
    case GAMEOBJECT_TYPE_SPELLCASTER: 
        return spellCaster.charges;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetLinkedGameObjectEntry() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_BUTTON: 
        return button.linkedTrap;
    case GAMEOBJECT_TYPE_CHEST: 
        return chest.linkedTrap;
    case GAMEOBJECT_TYPE_SPELL_FOCUS: 
        return spellFocus.linkedTrap;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.linkedTrap;
    case GAMEOBJECT_TYPE_GATHERING_NODE: 
        return gatheringNode.linkedTrap;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetAutoCloseTime() const
{
    uint32 autoCloseTime = 0;
    switch (type)
    {
    case GAMEOBJECT_TYPE_DOOR: 
        autoCloseTime = door.autoClose;
        break;
    case GAMEOBJECT_TYPE_BUTTON: 
        autoCloseTime = button.autoClose;
        break;
    case GAMEOBJECT_TYPE_TRAP: 
        autoCloseTime = trap.autoClose;
        break;
    case GAMEOBJECT_TYPE_GOOBER: 
        autoCloseTime = goober.autoClose;
        break;
    case GAMEOBJECT_TYPE_TRANSPORT: 
        autoCloseTime = transport.autoClose;
        break;
    case GAMEOBJECT_TYPE_AREADAMAGE: 
        autoCloseTime = areaDamage.autoClose;
        break;
    case GAMEOBJECT_TYPE_TRAPDOOR: 
        autoCloseTime = trapdoor.autoClose;
        break;
    default: 
        break;
    }
    return autoCloseTime / IN_MILLISECONDS; // prior to 3.0.3, conversion was / 0x10000;
}

uint32 GameObjectTemplate::GetLootId() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_CHEST: 
        return chest.chestLoot;
    case GAMEOBJECT_TYPE_FISHINGHOLE: 
        return fishingHole.chestLoot;
    case GAMEOBJECT_TYPE_GATHERING_NODE: 
        return gatheringNode.chestLoot;
    case GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD: 
        return challengeModeReward.chestLoot;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetGossipMenuId() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_QUESTGIVER: 
        return questgiver.gossipID;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.gossipID;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetEventScriptId() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.eventID;
    case GAMEOBJECT_TYPE_CHEST: 
        return chest.triggeredEvent;
    case GAMEOBJECT_TYPE_CAMERA: 
        return camera.eventID;
    case GAMEOBJECT_TYPE_GATHERING_NODE: 
        return gatheringNode.triggeredEvent;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetCooldown() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_TRAP: 
        return trap.cooldown;
    case GAMEOBJECT_TYPE_GOOBER: 
        return goober.cooldown;
    default: 
        return 0;
    }
}

uint32 GameObjectTemplate::GetSpell() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_TRAP:
        return trap.spell;
    case GAMEOBJECT_TYPE_GOOBER:
        return goober.spell;
    case GAMEOBJECT_TYPE_DUEL_ARBITER:
    case GAMEOBJECT_TYPE_FISHINGNODE:
    case GAMEOBJECT_TYPE_RITUAL:
        return ritual.spell;
    case GAMEOBJECT_TYPE_SPELLCASTER:
        return spellCaster.spell;
    case GAMEOBJECT_TYPE_NEW_FLAG:
        return newflag.pickupSpell;
    default:
        return 0;
    }
}

bool GameObjectTemplate::HasQuestItem() const
{
    for (auto questItem : QuestItems)
        if (questItem != 0)
            return true;
    return false;
}

bool GameObjectTemplate::IsOploteChest() const
{
    return type == GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD;
}

uint32 GameObjectTemplate::GetSpawnMap() const
{
    switch (type)
    {
    case GAMEOBJECT_TYPE_TRANSPORT: 
        return transport.SpawnMap;
    case GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT: 
        return moTransport.SpawnMap;
    case GAMEOBJECT_TYPE_GARRISON_BUILDING: 
        return garrisonBuilding.SpawnMap;
    case GAMEOBJECT_TYPE_GARRISON_PLOT: 
        return garrisonPlot.SpawnMap;
    case GAMEOBJECT_TYPE_PHASEABLE_MO: 
        return phaseableMO.SpawnMap;
    default: 
        break;
    }
    return 0;
}

