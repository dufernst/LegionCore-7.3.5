/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "CollectionMgr.h"
#include "MiscPackets.h"
#include "Player.h"
#include "TransmogrificationPackets.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"

ToyBoxData::ToyBoxData(bool _isFavourite, bool _needSave): isFavourite(_isFavourite), needSave(_needSave)
{
}

HeirloomData::HeirloomData(uint32 _flags, uint32 _bonusId, bool _needSave): flags(_flags), bonusId(_bonusId), needSave(_needSave)
{
}

TransmogData::TransmogData(uint32 _condition, bool _needSave): condition(_condition), needSave(_needSave), needDelete(false)
{
}

CollectionMgr::CollectionMgr(Player* owner): _owner(owner)
{
}

void CollectionMgr::SaveToDB(SQLTransaction& trans)
{
    PreparedStatement* stmt = nullptr;
    uint8 index = 0;

    for (auto& i : _toys)
    {
        if (i.second.needSave)
        {
            index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_TOYS);
            stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
            stmt->setUInt32(index++, i.first);
            stmt->setBool(index++, i.second.isFavourite);
            trans->Append(stmt);
            i.second.needSave = false;
        }
    }

    for (auto& t : _heirlooms)
    {
        if (t.second.needSave)
        {
            index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_HEIRLOOMS);
            stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
            stmt->setUInt32(index++, t.first);
            stmt->setUInt32(index++, t.second.flags);
            trans->Append(stmt);
            t.second.needSave = false;
        }
    }

    for (auto& t : _saveTransmogs)
    {
        if (!t.second)
        {
            index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_TRANSMOG);
            stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
            stmt->setUInt32(index++, t.first);
            trans->Append(stmt);
            _transmogs.erase(t.first);
            continue;
        }
        if (t.second->needSave)
        {
            index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_TRANSMOGS);
            stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
            stmt->setUInt32(index++, _owner->GetGUIDLow());
            stmt->setUInt32(index++, t.first);
            stmt->setUInt32(index++, t.second->condition);
            trans->Append(stmt);
            t.second->needSave = false;
        }
        if (t.second->needDelete)
        {
            index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_TRANSMOG);
            stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
            stmt->setUInt32(index++, t.first);
            trans->Append(stmt);
            _transmogs.erase(t.first);
        }
    }
    _saveTransmogs.clear();

    for (auto itr = _favoriteAppearances.begin(); itr != _favoriteAppearances.end();)
    {
        switch (itr->second)
        {
            case FavoriteAppearanceState::New:
                index = 0;
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_BNET_ITEM_FAVORITE_APPEARANCE);
                stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
                stmt->setUInt32(index++, itr->first);
                trans->Append(stmt);
                itr->second = FavoriteAppearanceState::Unchanged;
                ++itr;
                break;
            case FavoriteAppearanceState::Removed:
                index = 0;
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_BNET_ITEM_FAVORITE_APPEARANCE);
                stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
                stmt->setUInt32(index++, itr->first);
                trans->Append(stmt);
                itr = _favoriteAppearances.erase(itr);
                break;
            case FavoriteAppearanceState::Unchanged:
                ++itr;
                break;
        }
    }

    _favoriteAppearances.clear();

    for (auto const& mount : _saveMounts)
    {
        index = 0;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_MOUNTS);
        stmt->setUInt32(index++, _owner->GetSession()->GetAccountId());
        stmt->setUInt32(index++, mount.first);
        stmt->setUInt16(index++, mount.second);
        trans->Append(stmt);
    }
    _saveMounts.clear();
}

bool CollectionMgr::LoadFromDB(PreparedQueryResult toys, PreparedQueryResult heirlooms, PreparedQueryResult transmogs, PreparedQueryResult mounts, PreparedQueryResult favoriteAppearances)
{
    if (toys)
    {
        do
        {
            Field* fields = toys->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            bool isFavourite = fields[1].GetBool();

            _toys[itemId] = ToyBoxData(isFavourite, false);
        }
        while (toys->NextRow());

        _owner->AddDelayedEvent(10, [this]() -> void
        {
            if (_owner)
                _owner->UpdateAchievementCriteria(CRITERIA_TYPE_OWN_TOY_COUNT, 1);
        });
    }

    if (heirlooms)
    {
        do
        {
            Field* fields = heirlooms->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            uint32 flags = fields[1].GetUInt32();

            auto heirloom = sDB2Manager.GetHeirloomByItemId(itemId);
            if (!heirloom)
                continue;

            uint32 bonusId = 0;
            if (flags & HEIRLOOM_FLAG_BONUS_LEVEL_110)
                bonusId = heirloom->UpgradeItemBonusListID[2];
            else if (flags & HEIRLOOM_FLAG_BONUS_LEVEL_100)
                bonusId = heirloom->UpgradeItemBonusListID[1];
            else if (flags & HEIRLOOM_FLAG_BONUS_LEVEL_90)
                bonusId = heirloom->UpgradeItemBonusListID[0];

            _heirlooms[itemId] = HeirloomData(flags, bonusId, false);
        }
        while (heirlooms->NextRow());
    }

    if (transmogs)
    {
        do
        {
            Field* fields = transmogs->Fetch();
            uint32 transmogId = fields[0].GetUInt32();
            uint32 condition = fields[1].GetUInt32();

            if (condition)
                _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_CONDITIONAL_TRANSMOG, condition);
            else
            {
                uint16 offset = uint16(transmogId / 32);
                uint32 fieldMod = offset * 32;
                uint32 transmogField = 0;
                if (uint32 trFl = _owner->GetDynamicValue(PLAYER_DYNAMIC_FIELD_TRANSMOG, offset))
                    transmogField = trFl;
                uint8 field = transmogId - fieldMod;
                transmogField |= (1 << field);
                _owner->SetDynamicValue(PLAYER_DYNAMIC_FIELD_TRANSMOG, offset, transmogField);
            }

            _transmogs[transmogId] = TransmogData(condition, false);
        }
        while (transmogs->NextRow());
    }

    if (favoriteAppearances)
    {
        do
        {
            _favoriteAppearances[favoriteAppearances->Fetch()[0].GetUInt32()] = FavoriteAppearanceState::Unchanged;
        } while (favoriteAppearances->NextRow());
    }

    for (auto const& t : _toys)
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_TOYS, t.first);

    for (auto const& item : _heirlooms)
    {
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOMS, item.first);
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOM_FLAGS, item.second.flags);
    }

    if (mounts)
    {
        do
        {
            Field* fields = mounts->Fetch();
            uint32 spellId = fields[0].GetUInt32();
            if (!sDB2Manager.GetMount(spellId))
                continue;

            _mounts[spellId] = fields[1].GetUInt16();
        }
        while (mounts->NextRow());
    }

    for (auto const& m : _mounts)
        AddMount(m.first, MountFlags(m.second), false, true);

    return true;
}

bool CollectionMgr::AddToy(uint32 itemId, bool isFavourite /*= false*/)
{
    if (UpdateAccountToys(itemId, isFavourite))
    {
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_TOYS, itemId);
        _owner->UpdateAchievementCriteria(CRITERIA_TYPE_OWN_TOY, itemId, 1);
        _owner->UpdateAchievementCriteria(CRITERIA_TYPE_OWN_TOY_COUNT, 1);
        return true;
    }

    return false;
}

bool CollectionMgr::UpdateAccountToys(uint32 itemId, bool isFavourite /*= false*/)
{
    return _toys.insert(std::make_pair(itemId, ToyBoxData(isFavourite, true))).second;
}

bool CollectionMgr::HasToy(uint32 toyId)
{
    if (!sDB2Manager.IsToyItem(toyId))
        return false;

    return _toys.find(toyId) != _toys.end();
}

void CollectionMgr::ToySetFavorite(uint32 itemId, bool favorite)
{
    ToyBoxContainer::iterator itr = _toys.find(itemId);
    if (itr == _toys.end())
        return;

    itr->second = ToyBoxData(favorite);
}

bool CollectionMgr::UpdateAccountHeirlooms(uint32 itemId, uint32 flags)
{
    return _heirlooms.insert(std::make_pair(itemId, HeirloomData(flags, 0, true))).second;
}

uint32 CollectionMgr::GetHeirloomBonus(uint32 itemId)
{
    HeirloomContainer::const_iterator z = _heirlooms.find(itemId);
    if (z != _heirlooms.end())
        return z->second.bonusId;

    return 0;
}

bool CollectionMgr::HasHeirloom(uint32 itemId)
{
    return Trinity::Containers::MapGetValuePtr(_heirlooms, itemId);
}

void CollectionMgr::AddHeirloom(uint32 itemId, uint32 flags)
{
    if (UpdateAccountHeirlooms(itemId, flags))
    {
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOMS, itemId);
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOM_FLAGS, flags);
    }
}

void CollectionMgr::UpgradeHeirloom(uint32 itemId, uint32 castItem)
{
    Player* player = _owner;
    if (!player)
        return;

    HeirloomEntry const* heirloom = sDB2Manager.GetHeirloomByItemId(itemId);
    if (!heirloom)
        return;

    HeirloomContainer::iterator s = _heirlooms.find(itemId);
    if (s == _heirlooms.end())
        return;

    uint32 flags = s->second.flags;
    uint32 bonusId = 0;

    if (heirloom->UpgradeItemID[0] == castItem)
    {
        flags |= HEIRLOOM_FLAG_BONUS_LEVEL_90;
        bonusId = heirloom->UpgradeItemBonusListID[0];
    }
    if (heirloom->UpgradeItemID[1] == castItem)
    {
        flags |= HEIRLOOM_FLAG_BONUS_LEVEL_100;
        bonusId = heirloom->UpgradeItemBonusListID[1];
    }
    if (heirloom->UpgradeItemID[2] == castItem)
    {
        flags |= HEIRLOOM_FLAG_BONUS_LEVEL_110;
        bonusId = heirloom->UpgradeItemBonusListID[2];
    }

    for (Item* item : player->GetItemListByEntry(itemId, true))
        item->AddBonuses(bonusId);

    std::vector<uint32> const& fields = player->GetDynamicValues(PLAYER_DYNAMIC_FIELD_HEIRLOOMS);
    uint16 offset = std::find(fields.begin(), fields.end(), itemId) - fields.begin();

    player->SetDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOM_FLAGS, offset, flags);
    s->second.flags = flags;
    s->second.bonusId = bonusId;
    s->second.needSave = true;
}

void CollectionMgr::CheckHeirloomUpgrades(Item* item)
{
    Player* player = _owner;
    if (!player)
        return;

    HeirloomEntry const* heirloom = sDB2Manager.GetHeirloomByItemId(item->GetEntry());
    if (!heirloom)
        return;

    HeirloomContainer::iterator v = _heirlooms.find(item->GetEntry());
    if (v == _heirlooms.end())
        return;

    uint32 heirloomItemId = heirloom->StaticUpgradedItemID;
    uint32 newItemId = 0;
    while (HeirloomEntry const* heirloomDiff = sDB2Manager.GetHeirloomByItemId(heirloomItemId))
    {
        if (player->GetItemByEntry(heirloomDiff->ItemID))
            newItemId = heirloomDiff->ItemID;

        if (HeirloomEntry const* heirloomSub = sDB2Manager.GetHeirloomByItemId(heirloomDiff->StaticUpgradedItemID))
        {
            heirloomItemId = heirloomSub->ItemID;
            continue;
        }

        break;
    }

    if (newItemId)
    {
        std::vector<uint32> const& fields = player->GetDynamicValues(PLAYER_DYNAMIC_FIELD_HEIRLOOMS);
        uint16 offset = std::find(fields.begin(), fields.end(), v->first) - fields.begin();

        player->SetDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOMS, offset, newItemId);
        player->SetDynamicValue(PLAYER_DYNAMIC_FIELD_HEIRLOOM_FLAGS, offset, 0);

        _heirlooms.erase(v);
        _heirlooms[newItemId] = HeirloomData();

        return;
    }

    std::vector<uint32> const& fields = item->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);
    for (uint32 bonusId : fields)
        if (bonusId != v->second.bonusId)
        {
            item->ClearDynamicValue(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);
            item->_bonusData.Initialize(item->GetTemplate());
        }

    if (std::find(fields.begin(), fields.end(), v->second.bonusId) == fields.end())
        item->AddBonuses(v->second.bonusId);
}

bool CollectionMgr::CanApplyHeirloomXpBonus(uint32 itemId, uint32 level)
{
    std::vector<uint32> shitData{ 785, 786, 787, 788, 789 }; //< cos no special flags or other inque shit

    if (auto const& item = sDB2Manager.GetHeirloomByItemId(itemId))
    {
        if (std::find(shitData.begin(), shitData.end(), item->ID) != shitData.end())
            return level >= 100;

        auto b = _heirlooms.find(itemId);
        if (b == _heirlooms.end())
            return false;

        if (b->second.flags & HEIRLOOM_FLAG_BONUS_LEVEL_110)
            return level <= 110;
        if (b->second.flags & HEIRLOOM_FLAG_BONUS_LEVEL_100)
            return level <= 100;
        if (b->second.flags & HEIRLOOM_FLAG_BONUS_LEVEL_90)
            return level <= 90;

        return level <= 60;
    }

    return true;
}

bool CollectionMgr::HasTransmog(uint32 transmogId)
{
    TransmogContainer::const_iterator z = _transmogs.find(transmogId);
    return z != _transmogs.end();
}

uint32 constexpr PlayerClassByArmorSubclass[MAX_ITEM_SUBCLASS_ARMOR] =
{
    CLASSMASK_ALL_PLAYABLE,                                                                                                 //ITEM_SUBCLASS_ARMOR_MISCELLANEOUS
    (1 << (CLASS_PRIEST - 1)) | (1 << (CLASS_MAGE - 1)) | (1 << (CLASS_WARLOCK - 1)),                                       //ITEM_SUBCLASS_ARMOR_CLOTH
    (1 << (CLASS_ROGUE - 1)) | (1 << (CLASS_MONK - 1)) | (1 << (CLASS_DRUID - 1)) | (1 << (CLASS_DEMON_HUNTER - 1)),        //ITEM_SUBCLASS_ARMOR_LEATHER
    (1 << (CLASS_HUNTER - 1)) | (1 << (CLASS_SHAMAN - 1)),                                                                  //ITEM_SUBCLASS_ARMOR_MAIL
    (1 << (CLASS_WARRIOR - 1)) | (1 << (CLASS_PALADIN - 1)) | (1 << (CLASS_DEATH_KNIGHT - 1)),                              //ITEM_SUBCLASS_ARMOR_PLATE
    CLASSMASK_ALL_PLAYABLE,                                                                                                 //ITEM_SUBCLASS_ARMOR_BUCKLER
    (1 << (CLASS_WARRIOR - 1)) | (1 << (CLASS_PALADIN - 1)) | (1 << (CLASS_SHAMAN - 1)),                                    //ITEM_SUBCLASS_ARMOR_SHIELD
    1 << (CLASS_PALADIN - 1),                                                                                               //ITEM_SUBCLASS_ARMOR_LIBRAM
    1 << (CLASS_DRUID - 1),                                                                                                 //ITEM_SUBCLASS_ARMOR_IDOL
    1 << (CLASS_SHAMAN - 1),                                                                                                //ITEM_SUBCLASS_ARMOR_TOTEM
    1 << (CLASS_DEATH_KNIGHT - 1),                                                                                          //ITEM_SUBCLASS_ARMOR_SIGIL
    (1 << (CLASS_PALADIN - 1)) | (1 << (CLASS_DEATH_KNIGHT - 1)) | (1 << (CLASS_SHAMAN - 1)) | (1 << (CLASS_DRUID - 1)),    //ITEM_SUBCLASS_ARMOR_RELIC
};

enum class AppearanceErrors
{
    OK = 0,
    CANT_EQUIP = 1,
    INVALID_ITEM_TYPE = 2,
    INVALID_SOURCE = 3,
    LEGENDARY = 4,
    SAME_APPEARANCE = 5,
    MISMATCH = 6,
};

AppearanceErrors CollectionMgr::CanAddAppearance(ItemModifiedAppearanceEntry const* itemModifiedAppearance) const
{
    if (!itemModifiedAppearance)
        return AppearanceErrors::MISMATCH;

    if (itemModifiedAppearance->TransmogSourceTypeEnum == 6 || itemModifiedAppearance->TransmogSourceTypeEnum == 9)
        return AppearanceErrors::INVALID_SOURCE;

    //if (!sItemSearchNameStore.LookupEntry(itemModifiedAppearance->ItemID))
        //return AppearanceErrors::SAME_APPEARANCE;

    auto const& itemTemplate = sObjectMgr->GetItemTemplate(itemModifiedAppearance->ItemID);
    if (!itemTemplate)
        return AppearanceErrors::INVALID_ITEM_TYPE;

    if (_owner->CanUseItem(itemTemplate) != EQUIP_ERR_OK)
        return AppearanceErrors::INVALID_ITEM_TYPE;

    if (itemTemplate->GetQuality() == ITEM_QUALITY_LEGENDARY)
        return AppearanceErrors::LEGENDARY;

    if (itemTemplate->GetFlags2() & ITEM_FLAG2_NO_SOURCE_FOR_ITEM_VISUAL || itemTemplate->GetQuality() == ITEM_QUALITY_ARTIFACT)
        return AppearanceErrors::INVALID_SOURCE;

    switch (itemTemplate->GetClass())
    {
        case ITEM_CLASS_WEAPON:
            if (!(_owner->GetWeaponProficiency() & (1 << itemTemplate->GetSubClass())))
                return AppearanceErrors::CANT_EQUIP;
            if (itemTemplate->GetSubClass() == ITEM_SUBCLASS_WEAPON_EXOTIC || itemTemplate->GetSubClass() == ITEM_SUBCLASS_WEAPON_EXOTIC2 || itemTemplate->GetSubClass() == ITEM_SUBCLASS_WEAPON_MISCELLANEOUS ||
                itemTemplate->GetSubClass() == ITEM_SUBCLASS_WEAPON_THROWN || itemTemplate->GetSubClass() == ITEM_SUBCLASS_WEAPON_SPEAR || itemTemplate->GetSubClass() == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
                return AppearanceErrors::CANT_EQUIP;
            break;
        case ITEM_CLASS_ARMOR:
            switch (itemTemplate->GetInventoryType())
            {
                case INVTYPE_BODY:
                case INVTYPE_SHIELD:
                case INVTYPE_CLOAK:
                case INVTYPE_TABARD:
                case INVTYPE_HOLDABLE:
                    break;
                case INVTYPE_HEAD:
                case INVTYPE_SHOULDERS:
                case INVTYPE_CHEST:
                case INVTYPE_WAIST:
                case INVTYPE_LEGS:
                case INVTYPE_FEET:
                case INVTYPE_WRISTS:
                case INVTYPE_HANDS:
                case INVTYPE_ROBE:
                    if (itemTemplate->GetSubClass() == ITEM_SUBCLASS_ARMOR_MISCELLANEOUS)
                        return AppearanceErrors::CANT_EQUIP;
                    break;
                default:
                    return AppearanceErrors::CANT_EQUIP;
            }
            if (itemTemplate->GetInventoryType() != INVTYPE_CLOAK)
                if (!(PlayerClassByArmorSubclass[itemTemplate->GetSubClass()] & _owner->getClassMask()))
                    return AppearanceErrors::CANT_EQUIP;
            break;
        default:
            return AppearanceErrors::CANT_EQUIP;
    }

    if (itemTemplate->GetQuality() < ITEM_QUALITY_UNCOMMON)
        if (!(itemTemplate->GetFlags2() & ITEM_FLAG2_IGNORE_QUALITY_FOR_ITEM_VISUAL_SOURCE) || !(itemTemplate->GetFlags3() & ITEM_FLAG3_ACTS_AS_TRANSMOG_HIDDEN_VISUAL_OPTION))
            return AppearanceErrors::INVALID_ITEM_TYPE;

    return AppearanceErrors::OK;
}

void CollectionMgr::AddItemAppearance(ItemModifiedAppearanceEntry const* itemModifiedAppearance)
{
    WorldPackets::Misc::DisplayGameError display;
    switch (CanAddAppearance(itemModifiedAppearance))
    {
        case AppearanceErrors::OK:
            AddTransmog(itemModifiedAppearance->ID, 0);
            return;
        case AppearanceErrors::CANT_EQUIP:
            display.Error = UIErrors::ERR_TRANSMOGRIFY_CANT_EQUIP;
            break;
        case AppearanceErrors::INVALID_ITEM_TYPE:
            display.Error = UIErrors::ERR_TRANSMOGRIFY_INVALID_ITEM_TYPE;
            break;
        case AppearanceErrors::INVALID_SOURCE:
            display.Error = UIErrors::ERR_TRANSMOGRIFY_INVALID_SOURCE;
            break;
        case AppearanceErrors::LEGENDARY:
            display.Error = UIErrors::ERR_TRANSMOGRIFY_LEGENDARY;
            break;
        case AppearanceErrors::SAME_APPEARANCE:
            display.Error = UIErrors::ERR_TRANSMOGRIFY_SAME_APPEARANCE;
            break;
        case AppearanceErrors::MISMATCH:
            display.Error = UIErrors::ERR_TRANSMOGRIFY_MISMATCH;
            break;
        default:
            break;
    }
    
    _owner->SendDirectMessage(display.Write());
}

void CollectionMgr::AddTransmog(uint32 transmogId, uint32 condition)
{
    _transmogs.insert(std::make_pair(transmogId, TransmogData(condition, true)));
    _saveTransmogs[transmogId] = &_transmogs[transmogId];

    if (condition)
        _owner->AddDynamicValue(PLAYER_DYNAMIC_FIELD_CONDITIONAL_TRANSMOG, condition);
    else
    {
        auto offset = uint16(transmogId / 32);
        uint32 fieldMod = offset * 32;
        uint32 transmogField = 0;
        if (uint32 trFl = _owner->GetDynamicValue(PLAYER_DYNAMIC_FIELD_TRANSMOG, offset))
            transmogField = trFl;
        uint8 field = transmogId - fieldMod;
        transmogField |= (1 << field);
        _owner->SetDynamicValue(PLAYER_DYNAMIC_FIELD_TRANSMOG, offset, transmogField);
    }

    auto itemModifiedAppearance = sItemModifiedAppearanceStore.LookupEntry(transmogId);
    if (!itemModifiedAppearance)
        return;

    if (auto item = sItemStore.LookupEntry(itemModifiedAppearance->ItemID))
    {
        auto transmogSlot = ItemTransmogrificationSlots[item->InventoryType];
        if (transmogSlot >= 0)
            _owner->UpdateAchievementCriteria(CRITERIA_TYPE_APPEARANCE_UNLOCKED_BY_SLOT, transmogSlot, 1);
    }

    if (auto sets = sDB2Manager.GetTransmogSetsForItemModifiedAppearance(itemModifiedAppearance->ID))
        for (auto set : *sets)
            if (IsSetCompleted(set->ID))
                _owner->UpdateAchievementCriteria(CRITERIA_TYPE_TRANSMOG_SET_UNLOCKED, set->TransmogSetGroupID);
}

void CollectionMgr::AddTransmogSet(uint32 transmogSetId)
{
    auto items = sDB2Manager.GetTransmogSetItems(transmogSetId);
    if (!items)
        return;

    for (auto item : *items)
        if (auto itemModifiedAppearance = sItemModifiedAppearanceStore.LookupEntry(item->ItemModifiedAppearanceID))
            AddItemAppearance(itemModifiedAppearance);
}

bool CollectionMgr::HasItemAppearance(uint32 transmogId) const
{
    auto z = _transmogs.find(transmogId);
    if (z != _transmogs.end())
        return z->second.condition == transmogId;

    return false;
}

void CollectionMgr::RemoveTransmogCondition(uint32 transmogId, bool add)
{
    auto z = _transmogs.find(transmogId);
    if (z == _transmogs.end())
        return;

    _saveTransmogs[transmogId] = &z->second;
    _owner->RemoveDynamicValue(PLAYER_DYNAMIC_FIELD_CONDITIONAL_TRANSMOG, transmogId);

    if (add)
    {
        z->second.condition = 0;
        z->second.needSave = true;

        uint16 offset = uint16(transmogId / 32);
        uint32 fieldMod = offset * 32;
        uint32 transmogField = 0;
        if (uint32 trFl = _owner->GetDynamicValue(PLAYER_DYNAMIC_FIELD_TRANSMOG, offset))
            transmogField = trFl;
        uint8 field = transmogId - fieldMod;
        transmogField |= (1 << field);
        _owner->SetDynamicValue(PLAYER_DYNAMIC_FIELD_TRANSMOG, offset, transmogField);
    }
    else
        z->second.needDelete = true;
}

void CollectionMgr::SetAppearanceIsFavorite(uint32 itemModifiedAppearanceId, bool apply)
{
    auto itr = _favoriteAppearances.find(itemModifiedAppearanceId);
    if (apply)
    {
        if (itr == _favoriteAppearances.end())
            _favoriteAppearances[itemModifiedAppearanceId] = FavoriteAppearanceState::New;
        else if (itr->second == FavoriteAppearanceState::Removed)
            itr->second = FavoriteAppearanceState::Unchanged;
        else
            return;
    }
    else if (itr != _favoriteAppearances.end())
    {
        if (itr->second == FavoriteAppearanceState::New)
            _favoriteAppearances.erase(itemModifiedAppearanceId);
        else
            itr->second = FavoriteAppearanceState::Removed;
    }
    else
        return;

    WorldPackets::Transmogrification::TransmogCollectionUpdate transmogCollectionUpdate;
    transmogCollectionUpdate.IsSetFavorite = apply;
    transmogCollectionUpdate.FavoriteAppearances.push_back(itemModifiedAppearanceId);
    _owner->SendDirectMessage(transmogCollectionUpdate.Write());
}

void CollectionMgr::SendFavoriteAppearances() const
{
    WorldPackets::Transmogrification::TransmogCollectionUpdate transmogCollectionUpdate;
    transmogCollectionUpdate.IsFullUpdate = true;
    transmogCollectionUpdate.FavoriteAppearances.reserve(_favoriteAppearances.size());
    for (const auto& itr : _favoriteAppearances)
        if (itr.second != FavoriteAppearanceState::Removed)
            transmogCollectionUpdate.FavoriteAppearances.push_back(itr.first);
    _owner->SendDirectMessage(transmogCollectionUpdate.Write());
}

bool CollectionMgr::IsSetCompleted(uint32 transmogSetId) const
{
    auto transmogSetItems = sDB2Manager.GetTransmogSetItems(transmogSetId);
    if (!transmogSetItems)
        return false;

    std::array<int8, EQUIPMENT_SLOT_END> knownPieces;
    knownPieces.fill(-1);
    for (auto transmogSetItem : *transmogSetItems)
    {
        auto itemModifiedAppearance = sItemModifiedAppearanceStore.LookupEntry(transmogSetItem->ItemModifiedAppearanceID);
        if (!itemModifiedAppearance)
            continue;

        auto item = sItemStore.LookupEntry(itemModifiedAppearance->ItemID);
        if (!item)
            continue;

        auto transmogSlot = ItemTransmogrificationSlots[item->InventoryType];
        if (transmogSlot < 0 || knownPieces[transmogSlot] == 1)
            continue;

        knownPieces[transmogSlot] = HasItemAppearance(transmogSetItem->ItemModifiedAppearanceID) ? 1 : 0;
    }

    return std::find(knownPieces.begin(), knownPieces.end(), 0) == knownPieces.end();
}

void CollectionMgr::LoadAccountMounts(PreparedQueryResult result)
{
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 spellID = fields[0].GetUInt32();
        if (!sDB2Manager.GetMount(spellID))
            continue;

        _mounts[spellID] = fields[1].GetUInt16();
    }
    while (result->NextRow());
}

bool CollectionMgr::UpdateAccountMounts(uint32 spellID, MountFlags flags)
{
    _saveMounts.insert(std::make_pair(spellID, uint16(flags)));
    return _mounts.insert(std::make_pair(spellID, uint16(flags))).second;
}

std::map<uint32, uint32> _mountDefinitions;

bool CollectionMgr::AddMount(uint32 spellID, MountFlags flags /*= MOUNT_FLAG_NONE*/, bool factionMount /*= false*/, bool loading /*= false*/)
{
    MountEntry const* mount = sDB2Manager.GetMount(spellID);
    if (!mount)
        return false;

    std::map<uint32, uint32>::const_iterator itr = _mountDefinitions.find(spellID);
    if (itr != _mountDefinitions.end() && !factionMount)
        AddMount(itr->second, MOUNT_FLAG_NONE, true, loading);

    UpdateAccountMounts(spellID, flags);

    if (!loading)
    {
        if (!factionMount)
            SendSingleMountUpdate(std::make_pair(spellID, uint16(flags)));

        if (!_owner->HasSpell(spellID))
            _owner->learnSpell(spellID, false);
    }

    _owner->AddDelayedEvent(10, [this]() -> void
    {
        if (_owner)
            _owner->UpdateAchievementCriteria(CRITERIA_TYPE_LEARN_SKILLLINE_SPELLS, SKILL_MOUNTS);
    });

    return true;
}

void CollectionMgr::SetMountFlag(uint32 spellID, MountFlags flags)
{
    auto itr = _mounts.find(spellID);
    if (itr != _mounts.end())
    {
        itr->second = uint16(flags);
        SendSingleMountUpdate(*itr);
        _saveMounts.insert(std::make_pair(spellID, uint16(flags)));
    }
}

bool CollectionMgr::HasMount(uint32 spellID)
{
    return _mounts.find(spellID) != _mounts.end();
}

void CollectionMgr::SendSingleMountUpdate(std::pair<uint32, uint16> mount)
{
    MountContainer tempMounts;
    tempMounts.insert(mount);

    WorldPackets::Misc::AccountMountUpdate mountUpdate;
    mountUpdate.Mounts = &tempMounts;
    _owner->SendDirectMessage(mountUpdate.Write());
}

void CollectionMgr::LoadMountDefinitions()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = WorldDatabase.Query("SELECT spellID, otherSpellId FROM mount_definitions");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 mount definitions. DB table `mount_definitions` is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();

        uint32 spellID = fields[0].GetUInt32();
        uint32 otherSpellId = fields[1].GetUInt32();

        if (!sDB2Manager.GetMount(spellID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Mount spell %u defined in `mount_definitions` does not exists in Mount.db2, skipped", spellID);
            continue;
        }

        if (otherSpellId && !sDB2Manager.GetMount(otherSpellId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "OtherSpell %u defined in `mount_definitions` for spellID %u does not exists in Mount.db2, skipped", otherSpellId, spellID);
            continue;
        }

        _mountDefinitions[spellID] = otherSpellId;
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u mount definitions in %u ms", uint32(_mountDefinitions.size()), GetMSTimeDiffToNow(oldMSTime));
}

void CollectionMgr::Clear()
{
    _toys.clear();
    _heirlooms.clear();
    _transmogs.clear();
    _saveTransmogs.clear();
    _favoriteAppearances.clear();
    _mounts.clear();
    _saveMounts.clear();
}

uint32 CollectionMgr::GetSize()
{
    uint32 size = sizeof(CollectionMgr);

    size += _toys.size() * sizeof(ToyBoxContainer);
    size += _heirlooms.size() * sizeof(HeirloomContainer);
    size += _transmogs.size() * sizeof(TransmogContainer);
    size += _saveTransmogs.size() * sizeof(TransmogContainerSave);
    size += _favoriteAppearances.size() * sizeof(FavoriteAppearanceState);
    size += _mounts.size() * sizeof(MountContainer);
    size += _saveMounts.size() * sizeof(_saveMounts);
    return size;
}
