
#ifndef GossipData_h
#define GossipData_h

struct GossipMenuItems
{
    ConditionList   Conditions;
    uint32          MenuID;
    uint32          OptionIndex;
    uint32          OptionBroadcastTextID;
    uint32          BoxBroadcastTextID;
    uint32          OptionType;
    uint32          OptionNpcflag;
    uint32          OptionNpcflag2;
    uint32          ActionMenuID;
    uint32          ActionPoiID;
    uint32          BoxMoney;
    uint32          BoxCurrency;
    std::string     OptionText;
    std::string     BoxText;
    uint8           OptionNPC;
    bool            BoxCoded;
};

struct GossipMenus
{
    ConditionList   Conditions;
    uint32          Entry;
    uint32          TextID;
    uint32          FriendshipFactionID;
};

typedef std::multimap<uint32, GossipMenus> GossipMenusContainer;
typedef std::pair<GossipMenusContainer::const_iterator, GossipMenusContainer::const_iterator> GossipMenusMapBounds;
typedef std::pair<GossipMenusContainer::iterator, GossipMenusContainer::iterator> GossipMenusMapBoundsNonConst;
typedef std::multimap<uint32, GossipMenuItems> GossipMenuItemsContainer;
typedef std::pair<GossipMenuItemsContainer::const_iterator, GossipMenuItemsContainer::const_iterator> GossipMenuItemsMapBounds;
typedef std::pair<GossipMenuItemsContainer::iterator, GossipMenuItemsContainer::iterator> GossipMenuItemsMapBoundsNonConst;
typedef std::unordered_map<uint32, GossipMenuItemsLocale> GossipMenuItemsLocaleContainer;

class GossipDataStoreMgr
{
    GossipDataStoreMgr();
    ~GossipDataStoreMgr();

public:
    static GossipDataStoreMgr* instance();

    void LoadGossipMenuItemsLocales();
    void LoadGossipMenu();
    void LoadGossipMenuItems();

    GossipMenuItemsLocale const* GetGossipMenuItemsLocale(uint32 entry) const;
    GossipMenusMapBounds GetGossipMenusMapBounds(uint32 uiMenuId) const;
    GossipMenusMapBoundsNonConst GetGossipMenusMapBoundsNonConst(uint32 uiMenuId);
    GossipMenuItemsMapBounds GetGossipMenuItemsMapBounds(uint32 uiMenuId) const;
    GossipMenuItemsMapBoundsNonConst GetGossipMenuItemsMapBoundsNonConst(uint32 uiMenuId);
private:
    GossipMenusContainer _gossipMenusStore;
    GossipMenuItemsContainer _gossipMenuItemsStore;
    GossipMenuItemsLocaleContainer _gossipMenuItemsLocaleStore;
};

#define sGossipDataStore GossipDataStoreMgr::instance()

#endif
