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

#ifndef CollectionMgr_h__
#define CollectionMgr_h__

#include "WorldSession.h"

enum HeirloomPlayerFlags
{
    HEIRLOOM_FLAG_NONE              = 0x00,
    HEIRLOOM_FLAG_BONUS_LEVEL_90    = 0x01,
    HEIRLOOM_FLAG_BONUS_LEVEL_100   = 0x02,
    HEIRLOOM_FLAG_BONUS_LEVEL_110   = 0x04
};

enum HeirloomItemFlags
{
    HEIRLOOM_ITEM_FLAG_NONE                 = 0x00,
    HEIRLOOM_ITEM_FLAG_SHOW_ONLY_IF_KNOWN   = 0x01,
    HEIRLOOM_ITEM_FLAG_PVP                  = 0x02
};

enum MountFlags : uint16
{
    MOUNT_FLAG_NONE                     = 0x00,
    MOUNT_FLAG_GIFT                     = 0x01,
    MOUNT_FLAG_FAVORITE                 = 0x02,

    //< DBC flags
    MOUNT_FLAG_SELF_MOUNT               = 0x02, // Player becomes the mount himself
    MOUNT_FLAG_FACTION_SPECIFIC         = 0x04,
    MOUNT_FLAG_PREFERRED_SWIMMING       = 0x10,
    MOUNT_FLAG_PREFERRED_WATER_WALKING  = 0x20,
    MOUNT_FLAG_HIDE_IF_UNKNOWN          = 0x40
};

enum class AppearanceErrors;

enum class FavoriteAppearanceState
{
    New,
    Removed,
    Unchanged
};

struct ToyBoxData
{
    explicit ToyBoxData(bool _isFavourite = false, bool _needSave = false);

    bool isFavourite;
    bool needSave;
};

typedef std::map<uint32, ToyBoxData> ToyBoxContainer;

struct HeirloomData
{
    explicit HeirloomData(uint32 _flags = 0, uint32 _bonusId = 0, bool _needSave = false);

    uint32 flags;
    uint32 bonusId;
    bool needSave;
};

typedef std::map<uint32, HeirloomData> HeirloomContainer;

struct TransmogData
{
    explicit TransmogData(uint32 _condition = 0, bool _needSave = false);

    uint32 condition;
    bool needSave;
    bool needDelete;
};

typedef std::map<uint32, TransmogData> TransmogContainer;
typedef std::map<uint32, TransmogData*> TransmogContainerSave;

typedef std::map<uint32, uint16> MountContainer;

class CollectionMgr
{
public:
    explicit CollectionMgr(Player* owner);

    // General
    void SaveToDB(SQLTransaction& trans);
    bool LoadFromDB(PreparedQueryResult toys, PreparedQueryResult heirlooms, PreparedQueryResult transmogs, PreparedQueryResult mounts, PreparedQueryResult favoriteAppearances);

    void Clear();
    uint32 GetSize();

    // Account-wide toys
    void ToySetFavorite(uint32 itemId, bool favorite);
    bool AddToy(uint32 itemId, bool isFavourite = false);
    bool UpdateAccountToys(uint32 itemId, bool isFavourite = false);
    bool HasToy(uint32 toyId);
    ToyBoxContainer& GetAccountToys() { return _toys; }
    uint16 GetAccountToysCount() const { return _toys.size(); }

    // Account-wide heirlooms
    void AddHeirloom(uint32 itemId, uint32 flags);
    void UpgradeHeirloom(uint32 itemId, uint32 castItem);
    void CheckHeirloomUpgrades(Item* item);
    bool UpdateAccountHeirlooms(uint32 itemId, uint32 flags);
    bool CanApplyHeirloomXpBonus(uint32 itemId, uint32 level);
    uint32 GetHeirloomBonus(uint32 itemId);
    bool HasHeirloom(uint32 itemId);
    HeirloomContainer& GetAccountHeirlooms() { return _heirlooms; }
    uint16 GetAccountHeirloomsCount() const { return _heirlooms.size(); }

    void AddItemAppearance(ItemModifiedAppearanceEntry const* itemModifiedAppearance);
    void AddTransmog(uint32 transmogId, uint32 condition);
    bool HasTransmog(uint32 transmogId);
    AppearanceErrors CanAddAppearance(ItemModifiedAppearanceEntry const* itemModifiedAppearance) const;
    void RemoveTransmogCondition(uint32 transmogId, bool add = true);
    void AddTransmogSet(uint32 transmogSetId);
    bool HasItemAppearance(uint32 transmogId) const;
    TransmogContainer const& GetTransmogs() const { return _transmogs; }

    void SetAppearanceIsFavorite(uint32 itemModifiedAppearanceId, bool apply);
    void SendFavoriteAppearances() const;

    bool IsSetCompleted(uint32 transmogSetId) const;

    // Account-wide mounts
    void LoadAccountMounts(PreparedQueryResult result);
    void SetMountFlag(uint32 spellID, MountFlags flags);
    void SendSingleMountUpdate(std::pair<uint32, uint16> mount);

    bool UpdateAccountMounts(uint32 spellID, MountFlags flags);
    bool AddMount(uint32 spellID, MountFlags flags = MOUNT_FLAG_NONE, bool factionMount = false, bool learned = false);
    static void LoadMountDefinitions();
    MountContainer const& GetAccountMounts() const { return _mounts; }
    bool HasMount(uint32 spellID);
    uint16 GetAccountMountsCount() const { return _mounts.size(); }

private:
    Player* _owner;

    ToyBoxContainer _toys;
    HeirloomContainer _heirlooms;
    TransmogContainer _transmogs;
    TransmogContainerSave _saveTransmogs;
    std::unordered_map<uint32, FavoriteAppearanceState> _favoriteAppearances;
    MountContainer _mounts;
    MountContainer _saveMounts;
};

#endif // CollectionMgr_h__