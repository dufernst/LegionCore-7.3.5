////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "WildBattlePet.h"
 #include <random>
#include "DatabaseEnv.h"
#include "Creature.h"
#include "PetBattle.h"
#include "Map.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "DB2Stores.h"
#include "Common.h"
#include "BattlePetData.h"

void WildBattlePetZonePools::LoadPoolTemplate(Field* fields)
{
    auto& poolTemplate = m_Templates[fields[7].GetUInt32()];
    poolTemplate.Species = fields[1].GetUInt32();
    poolTemplate.BattlePetEntry = fields[2].GetUInt32();
    poolTemplate.Max = fields[3].GetUInt32();
    poolTemplate.RespawnTime = fields[4].GetUInt32();
    poolTemplate.MinLevel = fields[5].GetUInt32();
    poolTemplate.MaxLevel = fields[6].GetUInt32();
    poolTemplate.CreatureEntry = fields[7].GetUInt32();
}

WildBattlePetMgr* WildBattlePetMgr::instance()
{
    static WildBattlePetMgr instance;
    return &instance;
}

WildBattlePetMgr::WildBattlePetMgr() { }

void WildBattlePetMgr::Load()
{
    m_PoolsByMap.clear();
    m_PoolsByMap.resize(sMapStore.GetNumRows() + 1);

    QueryResult result = WorldDatabase.Query("SELECT Zone, Species, `BattlePetEntry`, `Max`, RespawnTime, MinLevel, MaxLevel, CreatureEntry FROM battlepet_wild_zone_pool");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 species definitions. DB table `WildBattlePetZoneSpecies` is empty");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 zoneID = fields[0].GetUInt32();
        uint32 BattlePetEntry = fields[2].GetUInt32();
        uint32 CreatureEntry = fields[7].GetUInt32();
        int32 mapID = 0;

        if (AreaTableEntry const* areaInfo = sAreaTableStore.LookupEntry(zoneID))
            mapID = areaInfo->ContinentID;

        if (mapID == -1)
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "WildBattlePetMgr::Load() no map id found for zone %u", zoneID);
            continue;
        }

        m_PoolsByMap[mapID][zoneID].LoadPoolTemplate(fields);

        m_battlePetSetEntry.insert(CreatureEntry);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u species definitions.", count);
}

void WildBattlePetMgr::Populate(WildPetPoolTemplate* wTemplate, WildBattlePetPool* pTemplate)
{
    if (!wTemplate)
        return;

    if (wTemplate->Max <= pTemplate->Replaced.size())
        return;

    if (sDB2Manager.HasBattlePetSpeciesFlag(wTemplate->Species, BATTLEPET_SPECIES_FLAG_UNTAMEABLE))
        return;

    std::vector<Creature*> availableForReplacement;
    if (!pTemplate->ToBeReplaced.empty())
        for (auto itr2 = pTemplate->ToBeReplaced.begin(); itr2 != pTemplate->ToBeReplaced.end(); ++itr2)
            if (pTemplate->ReplacedRelation.find((*itr2)->GetGUID()) == pTemplate->ReplacedRelation.end())
                availableForReplacement.push_back(*itr2);

    if (availableForReplacement.empty())
        return;

    std::shuffle(availableForReplacement.begin(), availableForReplacement.end(), std::mt19937(std::random_device()()));

    uint32 replaceCount = wTemplate->Max - pTemplate->Replaced.size();
    for (size_t y = 0; y < availableForReplacement.size() && y < replaceCount; y++)
        ReplaceCreature(availableForReplacement[y], wTemplate, pTemplate);
}

void WildBattlePetMgr::ReplaceCreature(Creature* creature, WildPetPoolTemplate* wTemplate, WildBattlePetPool* pTemplate)
{
    if (!creature->FindMap())
        return;

    BattlePetSpeciesEntry const* speciesInfo = sBattlePetSpeciesStore.LookupEntry(wTemplate->Species);
    if (!speciesInfo)
        return;

    auto replacementCreature = new Creature();
    replacementCreature->m_isTempWorldObject = true;

    if (!replacementCreature->Create(sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate(), creature->GetMap(), creature->GetPhaseMask(), speciesInfo->CreatureID, 0, 0, creature->m_positionX, creature->m_positionY, creature->m_positionZ, creature->m_orientation))
    {
        delete replacementCreature;
        return;
    }

    replacementCreature->SetHomePosition(*replacementCreature);

    // BattlePet fill data
    if (!replacementCreature->m_battlePetInstance)
        replacementCreature->m_battlePetInstance = std::make_shared<BattlePetInstance>();

    auto battlePetInstance = replacementCreature->m_battlePetInstance;

    battlePetInstance->JournalID.Clear();
    battlePetInstance->Slot = 0;
    battlePetInstance->NameTimeStamp = 0;
    battlePetInstance->Species = speciesInfo->ID;
    battlePetInstance->DisplayModelID = replacementCreature->GetDisplayId();
    battlePetInstance->XP = 0;
    battlePetInstance->Flags = 0;
    battlePetInstance->Health = 20000;

    // Select level
    battlePetInstance->Level = std::max(urand(wTemplate->MinLevel, wTemplate->MaxLevel), static_cast<uint32>(1));

    // Select breed
    uint8 MinQuality = 0;
    if (BattlePetTemplate const* temp = sBattlePetDataStore->GetBattlePetTemplate(speciesInfo->ID))
    {
        battlePetInstance->Breed = sBattlePetDataStore->GetRandomBreedID(temp->BreedIDs);
        MinQuality = temp->MinQuality;
    }
    else
        battlePetInstance->Breed = 3;

    uint8 randQuality = sBattlePetDataStore->GetRandomQuailty();
    battlePetInstance->Quality = MinQuality > randQuality ? MinQuality : randQuality;

    // Select abilities
    uint32 l_AbilityLevels[3];
    memset(l_AbilityLevels, 0, sizeof(l_AbilityLevels));
    memset(battlePetInstance->Abilities, 0, sizeof(battlePetInstance->Abilities));

    for (auto const& speciesXAbilityInfo : sBattlePetSpeciesXAbilityStore)
    {
        if (speciesXAbilityInfo->BattlePetSpeciesID != battlePetInstance->Species || speciesXAbilityInfo->RequiredLevel > battlePetInstance->Level)
            continue;

        if (l_AbilityLevels[speciesXAbilityInfo->SlotEnum])
        {
            int chance = 80;
            if (l_AbilityLevels[speciesXAbilityInfo->SlotEnum] < speciesXAbilityInfo->RequiredLevel)
                chance = 100 - chance;

            if (rand() % 100 > chance)
                continue;
        }

        battlePetInstance->Abilities[speciesXAbilityInfo->SlotEnum] = speciesXAbilityInfo->BattlePetAbilityID;
        l_AbilityLevels[speciesXAbilityInfo->SlotEnum] = speciesXAbilityInfo->RequiredLevel;
    }

    // Set creature flags
    replacementCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_WILD_BATTLE_PET);
    replacementCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    replacementCreature->SetUInt32Value(UNIT_FIELD_WILD_BATTLE_PET_LEVEL, battlePetInstance->Level);
    replacementCreature->SetRespawnRadius(3.5f);
    replacementCreature->SetDefaultMovementType(RANDOM_MOTION_TYPE);
    replacementCreature->replacementFromGUID = creature->GetGUID();

    if (!creature->GetMap()->AddToMap(replacementCreature))
    {
        delete replacementCreature;
        return;
    }

    // Despawn replaced creature
    creature->ForcedDespawn();
    creature->SetRespawnTime(MONTH);
    creature->RemoveCorpse(false);

    pTemplate->ReplacedRelation[creature->GetGUID()] = replacementCreature->GetGUID();
    pTemplate->Replaced.insert(replacementCreature);
}

void WildBattlePetMgr::EnableWildBattle(Creature* creature)
{
    BattlePetTemplate const* temp = sBattlePetDataStore->GetBattlePetTemplateByEntry(creature->GetEntry());
    if (!temp)
        return;

    BattlePetSpeciesEntry const* speciesInfo = sBattlePetSpeciesStore.LookupEntry(temp->Species);
    if (!speciesInfo)
        return;

    // BattlePet fill data
    if (!creature->m_battlePetInstance)
        creature->m_battlePetInstance = std::make_shared<BattlePetInstance>();

    auto battlePetInstance = creature->m_battlePetInstance;

    battlePetInstance->JournalID.Clear();
    battlePetInstance->Slot = 0;
    battlePetInstance->NameTimeStamp = 0;
    battlePetInstance->Species = speciesInfo->ID;
    battlePetInstance->DisplayModelID = creature->GetDisplayId();
    battlePetInstance->XP = 0;
    battlePetInstance->Flags = 0;
    battlePetInstance->Health = 20000;

    uint8 minlevel = temp->minlevel;
    uint8 maxlevel = temp->maxlevel;
    if (AreaTableEntry const* areaInfo = sAreaTableStore.LookupEntry(creature->GetCurrentZoneID()))
    {
        if (minlevel < areaInfo->WildBattlePetLevelMin)
            minlevel = areaInfo->WildBattlePetLevelMin;
        if (maxlevel < areaInfo->WildBattlePetLevelMax)
            maxlevel = areaInfo->WildBattlePetLevelMax;
    }

    // Select level
    battlePetInstance->Level = std::max(urand(minlevel, maxlevel), static_cast<uint32>(1));

    // Select breed
    battlePetInstance->Breed = sBattlePetDataStore->GetRandomBreedID(temp->BreedIDs);

    uint8 randQuality = sBattlePetDataStore->GetRandomQuailty();
    battlePetInstance->Quality = temp->MinQuality > randQuality ? temp->MinQuality : randQuality;

    // Select abilities
    uint32 l_AbilityLevels[3];
    memset(l_AbilityLevels, 0, sizeof(l_AbilityLevels));
    memset(battlePetInstance->Abilities, 0, sizeof(battlePetInstance->Abilities));

    for (auto const& speciesXAbilityInfo : sBattlePetSpeciesXAbilityStore)
    {
        if (speciesXAbilityInfo->BattlePetSpeciesID != battlePetInstance->Species || speciesXAbilityInfo->RequiredLevel > battlePetInstance->Level)
            continue;

        if (l_AbilityLevels[speciesXAbilityInfo->SlotEnum])
        {
            int chance = 80;
            if (l_AbilityLevels[speciesXAbilityInfo->SlotEnum] < speciesXAbilityInfo->RequiredLevel)
                chance = 100 - chance;

            if (rand() % 100 > chance)
                continue;
        }

        battlePetInstance->Abilities[speciesXAbilityInfo->SlotEnum] = speciesXAbilityInfo->BattlePetAbilityID;
        l_AbilityLevels[speciesXAbilityInfo->SlotEnum] = speciesXAbilityInfo->RequiredLevel;
    }

    // Set creature flags
    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    creature->SetUInt32Value(UNIT_FIELD_WILD_BATTLE_PET_LEVEL, battlePetInstance->Level);
}

void WildBattlePetMgr::Depopulate(WildBattlePetPool* pTemplate)
{
    std::map<ObjectGuid, ObjectGuid> creatures = pTemplate->ReplacedRelation;
    for (auto & creature : creatures)
    {
        Unit* unit = sObjectAccessor->FindUnit(creature.first);
        if (unit && unit->ToCreature())
            UnreplaceCreature(unit->ToCreature());
    }
}

void WildBattlePetMgr::UnreplaceCreature(Creature* creature)
{
    if (!creature || !creature->GetMap())
        return;

    WildBattlePetPool* pTemplate = creature->GetMap()->GetWildBattlePetPool(creature);
    if (!pTemplate || pTemplate->ReplacedRelation.find(creature->GetGUID()) == pTemplate->ReplacedRelation.end())
        return;

    Unit* unit = sObjectAccessor->FindUnit(pTemplate->ReplacedRelation[creature->GetGUID()]);
    if (!unit || !unit->ToCreature())
        return;

    Creature* replacementCreature = unit->ToCreature();

    replacementCreature->m_battlePetInstance.reset();

    pTemplate->Replaced.erase(replacementCreature);

    replacementCreature->RemoveFromWorld();
    replacementCreature->AddObjectToRemoveList();

    creature->SetRespawnTime(creature->GetCreatureData() ? creature->GetCreatureData()->spawntimesecs : 15);
    creature->Respawn();

    pTemplate->ReplacedRelation.erase(creature->GetGUID());
}

bool WildBattlePetMgr::IsWildPet(Creature* creature)
{
    if (!creature)
        return false;

    return creature->m_battlePetInstance != nullptr;
}

std::shared_ptr<BattlePetInstance> WildBattlePetMgr::GetWildBattlePet(Creature* creature)
{
    if (!IsWildPet(creature) || !creature)
        return nullptr;

    return BattlePetInstance::CloneForBattle(creature->m_battlePetInstance);
}

void WildBattlePetMgr::EnterInBattle(Creature* creature)
{
    if (!IsWildPet(creature))
        return;

    creature->ForcedDespawn();
    creature->SetRespawnTime(MONTH);
    creature->RemoveCorpse(false);
}

void WildBattlePetMgr::LeaveBattle(Creature* creature, bool /*p_Defeated*/)
{
    if (!IsWildPet(creature))
        return;

    Unit* unit = sObjectAccessor->FindUnit(creature->replacementFromGUID);
    if (!unit || !unit->ToCreature())
        return;

    UnreplaceCreature(unit->ToCreature());
}

WildPetPoolTemplate* WildBattlePetMgr::GetWildPetTemplate(uint32 mapId, uint32 zoneId, uint32 entry)
{
    if (m_PoolsByMap.empty() || m_PoolsByMap[mapId].empty())
        return nullptr;

    auto iter = m_PoolsByMap[mapId].find(zoneId);
    if (iter == m_PoolsByMap[mapId].end())
        return nullptr;

    WildBattlePetZonePools* pool = &iter->second;
    auto itr = pool->m_Templates.find(entry);
    if (itr == pool->m_Templates.end())
        return nullptr;

    return &itr->second;
}

bool WildBattlePetMgr::IsBattlePet(uint32 entry)
{
    return m_battlePetSetEntry.find(entry) != m_battlePetSetEntry.end();
}
