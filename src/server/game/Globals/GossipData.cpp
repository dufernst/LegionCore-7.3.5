#include "GossipData.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GossipDef.h"
#include "QuestData.h"

GossipDataStoreMgr::GossipDataStoreMgr() = default;

GossipDataStoreMgr::~GossipDataStoreMgr() = default;

GossipDataStoreMgr* GossipDataStoreMgr::instance()
{
    static GossipDataStoreMgr instance;
    return &instance;
}

void GossipDataStoreMgr::LoadGossipMenuItemsLocales()
{
    auto oldMSTime = getMSTime();

    _gossipMenuItemsLocaleStore.clear();

    //                                        0       1   2       3           4
    auto result = WorldDatabase.Query("SELECT MenuID, ID, Locale, OptionText, BoxText FROM gossip_menu_option_locale");
    if (!result)
        return;

    do
    {
        auto fields = result->Fetch();
        auto locale = GetLocaleByName(fields[2].GetString());
        if (locale == LOCALE_none)
            continue;


        auto& data = _gossipMenuItemsLocaleStore[MAKE_PAIR32(fields[0].GetUInt16(), fields[1].GetUInt16())];
        ObjectMgr::AddLocaleString(fields[3].GetString(), locale, data.OptionText);
        ObjectMgr::AddLocaleString(fields[4].GetString(), locale, data.BoxText);

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u gossip_menu_option locale strings in %u ms", static_cast<uint32>(_gossipMenuItemsLocaleStore.size()), GetMSTimeDiffToNow(oldMSTime));
}

void GossipDataStoreMgr::LoadGossipMenu()
{
    auto oldMSTime = getMSTime();

    _gossipMenusStore.clear();

    auto result = WorldDatabase.Query("SELECT Entry, TextID, FriendshipFactionID FROM gossip_menu");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0  gossip_menu entries. DB table `gossip_menu` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();

        GossipMenus gMenu;
        gMenu.Entry = fields[0].GetUInt32();
        gMenu.TextID = fields[1].GetUInt32();
        gMenu.FriendshipFactionID = fields[2].GetUInt32();

        if (!sObjectMgr->GetNpcText(gMenu.TextID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu Entry %u are using non-existing TextID %u", gMenu.Entry, gMenu.TextID);
            continue;
        }

        if (gMenu.FriendshipFactionID && !sFriendshipReputationStore.LookupEntry(gMenu.FriendshipFactionID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu Entry %u are using non-existing FriendshipFactionID %u", gMenu.Entry, gMenu.FriendshipFactionID);
            gMenu.FriendshipFactionID = 0;
        }

        _gossipMenusStore.insert(std::make_pair(gMenu.Entry, gMenu));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u gossip_menu entries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void GossipDataStoreMgr::LoadGossipMenuItems()
{
    auto oldMSTime = getMSTime();

    _gossipMenuItemsStore.clear();

        //                                    0        1           2          3            4          5              6             7            8         9         10
    auto result = WorldDatabase.Query("SELECT MenuID, OptionIndex, OptionNPC, OptionText, OptionType, OptionNpcflag, ActionMenuID, ActionPoiID, BoxCoded, BoxMoney, BoxText, "
        //11                    12                  13              14
        "OptionBroadcastTextID, BoxBroadcastTextID, OptionNpcflag2, BoxCurrency FROM gossip_menu_option ORDER BY MenuID, OptionIndex");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 gossip_menu_option entries. DB table `gossip_menu_option` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();

        GossipMenuItems gMenuItem;
        gMenuItem.MenuID = fields[0].GetUInt32();
        gMenuItem.OptionIndex = fields[1].GetUInt16();
        gMenuItem.OptionNPC = fields[2].GetUInt32();
        gMenuItem.OptionText = fields[3].GetString();
        gMenuItem.OptionType = fields[4].GetUInt8();
        gMenuItem.OptionNpcflag = fields[5].GetUInt32();
        gMenuItem.OptionNpcflag2 = fields[13].GetUInt32();
        gMenuItem.ActionMenuID = fields[6].GetUInt32();
        gMenuItem.ActionPoiID = fields[7].GetUInt32();
        gMenuItem.BoxCoded = fields[8].GetBool();
        gMenuItem.BoxMoney = fields[9].GetUInt32();
        gMenuItem.BoxText = fields[10].GetString();
        gMenuItem.OptionBroadcastTextID = fields[11].GetUInt32();
        gMenuItem.BoxBroadcastTextID = fields[12].GetUInt32();
        gMenuItem.BoxCurrency = fields[14].GetUInt32();

        if (gMenuItem.OptionNPC >= GOSSIP_ICON_MAX)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu_option for menu %u, id %u has unknown icon id %u. Replacing with GOSSIP_ICON_CHAT", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.OptionNPC);
            gMenuItem.OptionNPC = GOSSIP_ICON_CHAT;
        }

        if (gMenuItem.OptionNPC == GOSSIP_ICON_SHIPMENT && gMenuItem.OptionType != GOSSIP_OPTION_GARRISON_SHIPMENT)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu_option for menu %u, id %u has GOSSIP_ICON_SHIPMENT, but handler %i != GOSSIP_OPTION_GARRISON_SHIPMENT  ", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.OptionNPC);
            gMenuItem.OptionType = GOSSIP_OPTION_GARRISON_SHIPMENT;
        }

        if (gMenuItem.OptionNPC == GOSSIP_ICON_CLASS_HALL_UPGRADE && gMenuItem.OptionType != GOSSIP_OPTION_CLASS_HALL_UPGRADE)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu_option for menu %u, id %u has GOSSIP_ICON_CLASS_HALL_UPGRADE, but handler %i != GOSSIP_OPTION_CLASS_HALL_UPGRADE  ", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.OptionNPC);
            gMenuItem.OptionType = GOSSIP_OPTION_CLASS_HALL_UPGRADE;
        }

        if (gMenuItem.OptionType >= GOSSIP_OPTION_MAX)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu_option for menu %u, id %u has unknown option id %u. Option will not be used", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.OptionType);

        if (gMenuItem.ActionPoiID && !sQuestDataStore->GetPointOfInterest(gMenuItem.ActionPoiID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table gossip_menu_option for menu %u, id %u use non-existing action_poi_id %u, ignoring", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.ActionPoiID);
            gMenuItem.ActionPoiID = 0;
        }

        if (gMenuItem.OptionBroadcastTextID && !sBroadcastTextStore.LookupEntry(gMenuItem.OptionBroadcastTextID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gossip_menu_option` for menu %u, id %u has non-existing or incompatible OptionBroadcastTextId %u, ignoring.", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.OptionBroadcastTextID);
            gMenuItem.OptionBroadcastTextID = 0;
        }

        if (gMenuItem.BoxBroadcastTextID && !sBroadcastTextStore.LookupEntry(gMenuItem.BoxBroadcastTextID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `gossip_menu_option` for menu %u, id %u has non-existing or incompatible BoxBroadcastTextID %u, ignoring.", gMenuItem.MenuID, gMenuItem.OptionIndex, gMenuItem.BoxBroadcastTextID);
            gMenuItem.BoxBroadcastTextID = 0;
        }

        _gossipMenuItemsStore.insert(std::make_pair(gMenuItem.MenuID, gMenuItem));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u gossip_menu_option entries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

GossipMenuItemsLocale const* GossipDataStoreMgr::GetGossipMenuItemsLocale(uint32 entry) const
{
    return Trinity::Containers::MapGetValuePtr(_gossipMenuItemsLocaleStore, entry);
}

GossipMenusMapBounds GossipDataStoreMgr::GetGossipMenusMapBounds(uint32 uiMenuId) const
{
    return _gossipMenusStore.equal_range(uiMenuId);
}

GossipMenusMapBoundsNonConst GossipDataStoreMgr::GetGossipMenusMapBoundsNonConst(uint32 uiMenuId)
{
    return _gossipMenusStore.equal_range(uiMenuId);
}

GossipMenuItemsMapBounds GossipDataStoreMgr::GetGossipMenuItemsMapBounds(uint32 uiMenuId) const
{
    return _gossipMenuItemsStore.equal_range(uiMenuId);
}

GossipMenuItemsMapBoundsNonConst GossipDataStoreMgr::GetGossipMenuItemsMapBoundsNonConst(uint32 uiMenuId)
{
    return _gossipMenuItemsStore.equal_range(uiMenuId);
}
