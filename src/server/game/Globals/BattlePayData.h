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

#ifndef _BATTLE_PAY_DATA_STORE_H
#define _BATTLE_PAY_DATA_STORE_H

#include "BattlePayPackets.h"
#include "BattlePayMgr.h"

struct BattlePayDisplayInfoLocale
{
    StringVector Name1;
    StringVector Name2;
    StringVector Name3;
    StringVector Name4;
};

class BattlePayDataStoreMgr
{
    BattlePayDataStoreMgr();
    ~BattlePayDataStoreMgr();

public:
    static BattlePayDataStoreMgr* instance();

    void Initialize();
    std::vector<Battlepay::ProductGroup> const& GetProductGroups() const;
    std::vector<Battlepay::ShopEntry> const& GetShopEntries() const;
    uint32 GetProductGroupId(uint32 productId);
    std::map<uint32, Battlepay::Product> const& GetProducts() const;
    bool ProductExist(uint32 productID) const;
    Battlepay::Product const& GetProduct(uint32 productID) const;
    Battlepay::DisplayInfo const* GetDisplayInfo(uint32 id) const;
    std::vector<WorldPackets::BattlePay::ProductDisplayVisualData> const* GetDisplayInfoVisuals(uint32 id) const;
    Battlepay::ProductGroupLocale const* GetProductGroupLocale(uint32 entry) const;
    BattlePayDisplayInfoLocale const* GetDisplayInfoLocale(uint32 entry) const;

private:
    void LoadProductGroups();
    void LoadProduct();
    void LoadShopEntires();
    void LoadDisplayInfos();
    void LoadDisplayInfoVisuals();
    void LoadProductGroupLocales();
    void LoadDisplayInfoLocales();
};

#define sBattlePayDataStore BattlePayDataStoreMgr::instance()

#endif
