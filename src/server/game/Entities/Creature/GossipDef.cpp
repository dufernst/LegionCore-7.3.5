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

#include "QuestDef.h"
#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Formulas.h"
#include "QuestPackets.h"
#include "NPCPackets.h"
#include "QuestData.h"
#include "GossipData.h"

GossipMenu::GossipMenu()
{
    _menuId = 0;
    _locale = DEFAULT_LOCALE;
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::AddMenuItem(int32 menuItemId, uint8 icon, std::string const& message, uint32 sender, uint32 action, std::string const& boxMessage, uint32 boxMoney, uint32 boxCurrency, bool coded /*= false*/)
{
    if (!this || _menuItems.size() > GOSSIP_MAX_MENU_ITEMS)
        return;
    //ASSERT(_menuItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    // Find a free new id - script case
    if (menuItemId == -1)
    {
        menuItemId = 0;
        if (!_menuItems.empty())
        {
            for (GossipMenuItemContainer::const_iterator itr = _menuItems.begin(); itr != _menuItems.end(); ++itr)
            {
                if (int32(itr->first) > menuItemId)
                    break;

                menuItemId = itr->first + 1;
            }
        }
    }

    GossipMenuItem menuItem;

    menuItem.OptionNPC = icon;
    menuItem.Message = message;
    menuItem.IsCoded = coded;
    menuItem.Sender = sender;
    menuItem.OptionType = action;
    menuItem.BoxMessage = boxMessage;
    menuItem.BoxMoney = boxMoney;
    menuItem.BoxCurrency = boxCurrency;
    menuItem.menuItemId = menuItemId;

    _menuItems[menuItemId] = menuItem;
}

void GossipMenu::AddMenuItem(uint32 menuId, uint32 menuItemId, uint32 sender, uint32 action)
{
    auto bounds = sGossipDataStore->GetGossipMenuItemsMapBounds(menuId);
    if (bounds.first == bounds.second)
        return;

    auto localeConstant = GetLocale();
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second.OptionIndex != menuItemId)
            continue;

        auto strOptionText = itr->second.OptionText;
        auto strBoxText = itr->second.BoxText;

        if (localeConstant != LOCALE_none)
        {
            if (auto optionBroadcastText = sBroadcastTextStore.LookupEntry(itr->second.OptionBroadcastTextID))
                strOptionText = DB2Manager::GetBroadcastTextValue(optionBroadcastText, GetLocale());
            else if (auto gossipMenuLocale = sGossipDataStore->GetGossipMenuItemsLocale(MAKE_PAIR32(menuId, menuItemId)))
                ObjectMgr::GetLocaleString(gossipMenuLocale->OptionText, GetLocale(), strOptionText);

            if (auto boxBroadcastText = sBroadcastTextStore.LookupEntry(itr->second.BoxBroadcastTextID))
                strBoxText = DB2Manager::GetBroadcastTextValue(boxBroadcastText, GetLocale());
            else if (auto gossipMenuLocale = sGossipDataStore->GetGossipMenuItemsLocale(MAKE_PAIR32(menuId, menuItemId)))
                ObjectMgr::GetLocaleString(gossipMenuLocale->BoxText, GetLocale(), strBoxText);
        }

        AddMenuItem(-1, itr->second.OptionNPC, strOptionText, sender, action, strBoxText, itr->second.BoxMoney, itr->second.BoxCurrency, itr->second.BoxCoded);
    }
}

void GossipMenu::SetMenuId(uint32 menu_id)
{
    _menuId = menu_id;
}

uint32 GossipMenu::GetMenuId() const
{
    return _menuId;
}

void GossipMenu::SetLocale(LocaleConstant locale)
{
    _locale = locale;
}

LocaleConstant GossipMenu::GetLocale() const
{
    return _locale;
}

void GossipMenu::AddGossipMenuItemData(uint32 menuItemId, uint32 gossipActionMenuId, uint32 gossipActionPoi)
{
    GossipMenuItemData& itemData = _menuItemData[menuItemId];

    itemData.GossipActionMenuId = gossipActionMenuId;
    itemData.GossipActionPoi = gossipActionPoi;
}

uint32 GossipMenu::GetMenuItemSender(uint32 menuItemId) const
{
    auto itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return 0;

    return itr->second.Sender;
}

uint32 GossipMenu::GetMenuItemAction(uint32 menuItemId) const
{
    auto itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return 0;

    return itr->second.OptionType;
}

bool GossipMenu::IsMenuItemCoded(uint32 menuItemId) const
{
    auto itr = _menuItems.find(menuItemId);
    if (itr == _menuItems.end())
        return false;

    return itr->second.IsCoded;
}

void GossipMenu::ClearMenu()
{
    _menuItems.clear();
    _menuItemData.clear();
}

GossipMenuItemContainer const& GossipMenu::GetMenuItems() const
{
    return _menuItems;
}

PlayerMenu::PlayerMenu(WorldSession* session) : _session(session)
{
    if (_session)
        _gossipMenu.SetLocale(_session->GetSessionDbLocaleIndex());
}

PlayerMenu::~PlayerMenu()
{
    ClearMenus();
}

GossipMenu& PlayerMenu::GetGossipMenu()
{
    return _gossipMenu;
}

QuestMenu& PlayerMenu::GetQuestMenu()
{
    return _questMenu;
}

bool PlayerMenu::Empty() const
{
    return _gossipMenu.Empty() && _questMenu.Empty();
}

void PlayerMenu::ClearMenus()
{
    _gossipMenu.ClearMenu();
    _questMenu.ClearMenu();
}

uint32 PlayerMenu::GetGossipOptionSender(uint32 selection) const
{
    return _gossipMenu.GetMenuItemSender(selection);
}

uint32 PlayerMenu::GetGossipOptionAction(uint32 selection) const
{
    return _gossipMenu.GetMenuItemAction(selection);
}

bool PlayerMenu::IsGossipOptionCoded(uint32 selection) const
{
    return _gossipMenu.IsMenuItemCoded(selection);
}

void PlayerMenu::SendGossipMenu(uint32 titleTextId, ObjectGuid objectGUID, uint32 friendshipFactionID /*= 0*/) const
{
    if (!this)
        return;

    WorldPackets::NPC::GossipMessage packet;
    packet.GossipGUID = objectGUID;
    packet.TextID = titleTextId;
    packet.GossipID = _gossipMenu.GetMenuId();
    packet.FriendshipFactionID = friendshipFactionID;

    for (auto const& itr : _gossipMenu.GetMenuItems())
    {
        auto const& item = itr.second;

        WorldPackets::NPC::ClientGossipOptions opt;
        opt.ClientOption = item.menuItemId;
        opt.OptionNPC = item.OptionNPC;
        opt.OptionFlags = item.IsCoded;
        opt.OptionCost = item.BoxMoney;
        opt.Text = item.Message;
        opt.Confirm = item.BoxMessage;
        packet.GossipOptions.emplace_back(opt);
    }

    packet.GossipText.resize(_questMenu.GetMenuItemCount());
    uint32 count = 0;
    for (uint8 i = 0; i < _questMenu.GetMenuItemCount(); ++i)
    {
        auto const& item = _questMenu.GetItem(i);
        if (auto quest = sQuestDataStore->GetQuestTemplate(item.QuestId))
        {
            auto& text = packet.GossipText[count];
            text.QuestID = item.QuestId;
            text.QuestType = item.QuestIcon;
            text.QuestLevel = quest->Level;
            text.QuestMaxScalingLevel = quest->MaxScalingLevel;
            text.QuestFlags[0] = quest->GetFlags();
            text.QuestFlags[1] = quest->FlagsEx;
            text.Repeatable = quest->IsRepeatable();
            auto title = quest->LogTitle;
            if (auto localeData = sQuestDataStore->GetQuestLocale(item.QuestId))
                ObjectMgr::GetLocaleString(localeData->LogTitle, _session->GetSessionDbLocaleIndex(), title);
            text.QuestTitle = title;
            ++count;
        }
    }

    // Shrink to the real size
    packet.GossipText.resize(count);

    if (Player* player = _session->GetPlayer())
        player->SendDirectMessage(packet.Write());
}

void PlayerMenu::SendCloseGossip() const
{
    if (Player* player = _session->GetPlayer())
        player->SendDirectMessage(WorldPackets::NPC::GossipComplete().Write());
}

void PlayerMenu::SendPointOfInterest(uint32 poiId) const
{
    PointOfInterest const* poi = sQuestDataStore->GetPointOfInterest(poiId);
    if (!poi)
        return;

    WorldPackets::NPC::GossipPOI packet;
    packet.Flags = poi->flags;
    packet.Pos = Position(poi->x, poi->y);
    packet.Icon = poi->icon;
    packet.Importance = poi->data;
    packet.Name = poi->icon_name;
    if (PointOfInterestLocale const* localeData = sQuestDataStore->GetPointOfInterestLocale(poiId))
        ObjectMgr::GetLocaleString(localeData->IconName, _session->GetSessionDbLocaleIndex(), packet.Name);
    if (Player* player = _session->GetPlayer())
        player->SendDirectMessage(packet.Write());
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

QuestMenu::QuestMenu()
{
    _questMenuItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::AddMenuItem(uint32 QuestId, uint8 Icon)
{
    if (!sQuestDataStore->GetQuestTemplate(QuestId))
        return;

    ASSERT(_questMenuItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    QuestMenuItem questMenuItem;

    questMenuItem.QuestId = QuestId;
    questMenuItem.QuestIcon = Icon;

    _questMenuItems.push_back(questMenuItem);
}

bool QuestMenu::HasItem(uint32 questId) const
{
    for (auto _questMenuItem : _questMenuItems)
        if (_questMenuItem.QuestId == questId)
            return true;

    return false;
}

QuestMenuItem const& QuestMenu::GetItem(uint16 index) const
{
    return _questMenuItems[index];
}

void QuestMenu::ClearMenu()
{
    _questMenuItems.clear();
}

void PlayerMenu::SendQuestGiverQuestList(uint32 BroadcastTextID, ObjectGuid npcGUID)
{
    Player* player = _session->GetPlayer();
    if (!player)
        return;

    auto boxBroadcastText = sBroadcastTextStore.LookupEntry(BroadcastTextID);
    if (boxBroadcastText)
        if (!sConditionMgr->IsPlayerMeetingCondition(player, boxBroadcastText->ConditionID))
            return;

    std::string Title;
    if (boxBroadcastText)
        Title = DB2Manager::GetBroadcastTextValue(boxBroadcastText, _session->GetSessionDbLocaleIndex());

    WorldPackets::Quest::QuestGiverQuestList questList;
    questList.QuestGiverGUID = npcGUID;

    questList.GreetEmoteDelay = boxBroadcastText ? boxBroadcastText->EmoteDelay[0] : 0;
    questList.GreetEmoteType = boxBroadcastText ? boxBroadcastText->EmoteID[0] : 0;
    questList.Greeting = Title;

    for (uint32 i = 0; i < _questMenu.GetMenuItemCount(); ++i)
    {
        QuestMenuItem const& questMenuItem = _questMenu.GetItem(i);

        if (Quest const* quest = sQuestDataStore->GetQuestTemplate(questMenuItem.QuestId))
        {
            std::string title = quest->LogTitle;
            if (QuestTemplateLocale const* ql = sQuestDataStore->GetQuestLocale(questMenuItem.QuestId))
                ObjectMgr::GetLocaleString(ql->LogTitle, _session->GetSessionDbLocaleIndex(), title);

            questList.GossipTexts.emplace_back(questMenuItem.QuestId, questMenuItem.QuestIcon, quest->Level, quest->MaxScalingLevel, quest->Flags, quest->FlagsEx, quest->IsRepeatable(), title);
        }
    }

    player->SendDirectMessage(questList.Write());
}

void PlayerMenu::SendQuestGiverStatus(uint32 questStatus, ObjectGuid npcGUID) const
{
    WorldPackets::Quest::QuestGiverStatus packet;
    packet.QuestGiver.Guid = npcGUID;
    packet.QuestGiver.Status = questStatus;
    if (Player* player = _session->GetPlayer())
        player->SendDirectMessage(packet.Write());
}

void PlayerMenu::SendQuestGiverQuestDetails(Quest const* quest, ObjectGuid npcGUID, bool activateAccept, bool isAreaTrigger /*=false*/) const
{
    Player* player = _session->GetPlayer();
    if (!player)
        return;

    std::string questLogTitle = quest->LogTitle;
    std::string questDescription = quest->QuestDescription;
    std::string questLogDescription = quest->LogDescription;
    std::string questEndText = quest->AreaDescription;
    std::string portraitGiverText = quest->PortraitGiverText;
    std::string portraitGiverName = quest->PortraitGiverName;
    std::string portraitTurnInText = quest->PortraitTurnInText;
    std::string portraitTurnInName = quest->PortraitTurnInName;

    if (QuestTemplateLocale const* localeData = sQuestDataStore->GetQuestLocale(quest->GetQuestId()))
    {
        LocaleConstant localeConstant = _session->GetSessionDbLocaleIndex();
        ObjectMgr::GetLocaleString(localeData->LogTitle, localeConstant, questLogTitle);
        ObjectMgr::GetLocaleString(localeData->QuestDescription, localeConstant, questDescription);
        ObjectMgr::GetLocaleString(localeData->LogDescription, localeConstant, questLogDescription);
        ObjectMgr::GetLocaleString(localeData->AreaDescription, localeConstant, questEndText);
        ObjectMgr::GetLocaleString(localeData->PortraitGiverText, localeConstant, portraitGiverText);
        ObjectMgr::GetLocaleString(localeData->PortraitGiverName, localeConstant, portraitGiverName);
        ObjectMgr::GetLocaleString(localeData->PortraitTurnInText, localeConstant, portraitTurnInText);
        ObjectMgr::GetLocaleString(localeData->PortraitTurnInName, localeConstant, portraitTurnInName);
    }

    WorldPackets::Quest::QuestGiverQuestDetails packet;
    packet.QuestGiverGUID = npcGUID;
    packet.InformUnit = player->GetDivider();
    packet.QuestID = quest->GetQuestId();
    packet.QuestTitle = questLogTitle;
    packet.LogDescription = questLogDescription;
    packet.DescriptionText = questDescription;
    packet.PortraitGiverText = portraitGiverText;
    packet.PortraitGiverName = portraitGiverName;
    packet.PortraitTurnInText = portraitTurnInText;
    packet.PortraitTurnInName = portraitTurnInName;
    packet.PortraitGiver = quest->QuestGiverPortrait;
    packet.PortraitTurnIn = quest->QuestTurnInPortrait;
    packet.AutoLaunched = activateAccept;
    packet.QuestFlags[0] = quest->GetFlags() & (sWorld->getBoolConfig(CONFIG_QUEST_IGNORE_AUTO_ACCEPT) ? ~QUEST_FLAGS_AUTO_ACCEPT : ~0);
    packet.QuestFlags[1] = quest->FlagsEx;
    packet.SuggestedPartyMembers = quest->SuggestedPlayers;

    if (quest->SourceSpellID)
        packet.LearnSpells.push_back(quest->SourceSpellID);

    packet.QuestStartItemID = 0;

    quest->BuildQuestRewards(packet.Rewards, player);

    packet.DescEmotes.resize(QUEST_EMOTE_COUNT);
    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        packet.DescEmotes[i].Type = quest->DetailsEmote[i];
        packet.DescEmotes[i].Delay = quest->DetailsEmoteDelay[i];
    }

    QuestObjectives const& objs = quest->GetObjectives();
    packet.Objectives.resize(objs.size());
    for (auto i = 0; i < objs.size(); ++i)
    {
        packet.Objectives[i].ID = objs[i].ID;
        packet.Objectives[i].ObjectID = objs[i].ObjectID;
        packet.Objectives[i].Amount = objs[i].Amount;
        packet.Objectives[i].Type = objs[i].Type;
    }

    player->SendDirectMessage(packet.Write());
}

void PlayerMenu::SendQuestQueryResponse(uint32 questId) const
{
    Quest const* quest = sQuestDataStore->GetQuestTemplate(questId);
    if (!quest)
        return;

    std::string questLogTitle = quest->LogTitle;
    std::string questLogDescription = quest->LogDescription;
    std::string questDescription = quest->QuestDescription;
    std::string areaDescription = quest->AreaDescription;
    std::string questCompletionLog = quest->QuestCompletionLog;
    std::string portraitGiverText = quest->PortraitGiverText;
    std::string portraitGiverName = quest->PortraitGiverName;
    std::string portraitTurnInText = quest->PortraitTurnInText;
    std::string portraitTurnInName = quest->PortraitTurnInName;

    if (QuestTemplateLocale const* questTemplateLocale = sQuestDataStore->GetQuestLocale(quest->GetQuestId()))
    {
        LocaleConstant localeConstant = _session->GetSessionDbLocaleIndex();
        ObjectMgr::GetLocaleString(questTemplateLocale->LogTitle, localeConstant, questLogTitle);
        ObjectMgr::GetLocaleString(questTemplateLocale->LogDescription, localeConstant, questLogDescription);
        ObjectMgr::GetLocaleString(questTemplateLocale->QuestDescription, localeConstant, questDescription);
        ObjectMgr::GetLocaleString(questTemplateLocale->AreaDescription, localeConstant, areaDescription);
        ObjectMgr::GetLocaleString(questTemplateLocale->QuestCompletionLog, localeConstant, questCompletionLog);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitGiverText, localeConstant, portraitGiverText);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitGiverName, localeConstant, portraitGiverName);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitTurnInText, localeConstant, portraitTurnInText);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitTurnInName, localeConstant, portraitTurnInName);
    }

    WorldPackets::Quest::QueryQuestInfoResponse packet;

    packet.Allow = true;
    packet.QuestID = quest->GetQuestId();

    packet.Info.QuestID = quest->GetQuestId();
    packet.Info.QuestType = quest->Type;
    packet.Info.QuestLevel = quest->Level;
    packet.Info.QuestMaxScalingLevel = quest->MaxScalingLevel;
    packet.Info.QuestPackageID = quest->PackageID;
    packet.Info.QuestMinLevel = quest->MinLevel;
    packet.Info.QuestSortID = quest->QuestSortID;
    packet.Info.QuestInfoID = quest->QuestInfoID;
    packet.Info.SuggestedGroupNum = quest->SuggestedPlayers;
    packet.Info.RewardNextQuest = quest->NextQuestIdChain;
    packet.Info.RewardXPDifficulty = quest->RewardXPDifficulty;
    packet.Info.RewardXPMultiplier = quest->RewardXPMultiplier;
    packet.Info.RewardMoneyMultiplier = quest->RewardMoneyMultiplier;

    if (!quest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
        packet.Info.RewardMoney = quest->RewardMoney < 0 ? quest->RewardMoney : _session->GetPlayer()->GetQuestMoneyReward(quest);

    packet.Info.RewardMoneyDifficulty = quest->RewardMoneyDifficulty;
    packet.Info.RewardMoneyMultiplier = quest->RewardMoneyMultiplier;
    packet.Info.RewardBonusMoney = quest->GetRewMoneyMaxLevel();
    for (uint8 i = 0; i < QUEST_REWARD_DISPLAY_SPELL_COUNT; ++i)
        packet.Info.RewardDisplaySpell[i] = quest->RewardDisplaySpell[i];

    packet.Info.RewardSpell = quest->RewardSpell;

    packet.Info.RewardHonor = quest->RewardHonor;
    packet.Info.RewardHonorMultiplier = quest->RewardHonorMultiplier;
    packet.Info.RewardArtifactXPMultiplier = quest->RewardArtifactXPMultiplier;
    packet.Info.RewardArtifactXP = quest->RewardArtifactXP;
    packet.Info.RewardArtifactCategoryID = quest->RewardArtifactCategoryID;

    packet.Info.StartItem = quest->SourceItemId;
    packet.Info.Flags = quest->GetFlags();
    packet.Info.FlagsEx = quest->FlagsEx;
    packet.Info.RewardTitle = quest->RewardTitleId;
    packet.Info.RewardArenaPoints = quest->RewardArenaPoints;
    packet.Info.RewardSkillLineID = quest->RewardSkillId;
    packet.Info.RewardNumSkillUps = quest->RewardSkillPoints;
    packet.Info.RewardFactionFlags = quest->RewardFactionFlags;
    packet.Info.PortraitGiver = quest->QuestGiverPortrait;
    packet.Info.PortraitTurnIn = quest->QuestTurnInPortrait;

    for (uint8 i = 0; i < QUEST_ITEM_COUNT; ++i)
    {
        packet.Info.ItemDrop[i] = quest->ItemDrop[i];
        packet.Info.ItemDropQuantity[i] = quest->ItemDropQuantity[i];
    }

    if (!quest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (uint8 i = 0; i < QUEST_ITEM_COUNT; ++i)
        {
            packet.Info.RewardItems[i] = quest->RewardItemId[i];
            packet.Info.RewardAmount[i] = quest->RewardItemCount[i];
        }
        for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        {
            packet.Info.UnfilteredChoiceItems[i].ItemID = quest->RewardChoiceItemId[i];
            packet.Info.UnfilteredChoiceItems[i].Quantity = quest->RewardChoiceItemCount[i];
        }
    }

    for (uint8 i = 0; i < QUEST_REWARD_REPUTATIONS_COUNT; ++i)
    {
        packet.Info.RewardFactionID[i] = quest->RewardFactionId[i];
        packet.Info.RewardFactionValue[i] = quest->RewardFactionValue[i];
        packet.Info.RewardFactionOverride[i] = quest->RewardFactionOverride[i];
        packet.Info.RewardFactionCapIn[i] = quest->RewardFactionCapIn[i];
    }

    packet.Info.POIContinent = quest->POIContinent;
    packet.Info.POIx = quest->POIx;
    packet.Info.POIy = quest->POIy;
    packet.Info.POIPriority = quest->POIPriority;
    packet.Info.LogTitle = questLogTitle;
    packet.Info.LogDescription = questLogDescription;
    packet.Info.QuestDescription = questDescription;
    packet.Info.AreaDescription = areaDescription;
    packet.Info.QuestCompletionLog = questCompletionLog;
    packet.Info.AllowableRaces = quest->AllowableRaces;
    packet.Info.QuestRewardID = quest->QuestRewardID;
    packet.Info.Expansion = quest->Expansion;

    for (QuestObjective const& questObjective : quest->GetObjectives())
    {
        packet.Info.Objectives.push_back(questObjective);
        if (QuestObjectivesLocale const* questObjectivesLocale = sQuestDataStore->GetQuestObjectivesLocale(questObjective.ID))
            ObjectMgr::GetLocaleString(questObjectivesLocale->Description, _session->GetSessionDbLocaleIndex(), packet.Info.Objectives.back().Description);
    }

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        packet.Info.RewardCurrencyID[i] = quest->RewardCurrencyId[i];
        packet.Info.RewardCurrencyQty[i] = quest->RewardCurrencyCount[i];
    }

    packet.Info.PortraitGiverText = portraitGiverText;
    packet.Info.PortraitGiverName = portraitGiverName;
    packet.Info.PortraitTurnInText = portraitTurnInText;
    packet.Info.PortraitTurnInName = portraitTurnInName;

    packet.Info.AcceptedSoundKitID = quest->SoundAccept;
    packet.Info.CompleteSoundKitID = quest->SoundTurnIn;
    packet.Info.AreaGroupID = quest->AreaGroupID;
    packet.Info.TimeAllowed = quest->LimitTime;

    if (Player* player = _session->GetPlayer())
        player->SendDirectMessage(packet.Write());
}

void PlayerMenu::SendQuestGiverOfferReward(Quest const* quest, ObjectGuid npcGUID, bool enableNext) const
{
    Player* player = _session->GetPlayer();

    std::string questTitle = quest->LogTitle;
    std::string questOfferRewardText = quest->OfferRewardText;
    std::string portraitGiverText = quest->PortraitGiverText;
    std::string portraitGiverName = quest->PortraitGiverName;
    std::string portraitTurnInText = quest->PortraitTurnInText;
    std::string portraitTurnInName = quest->PortraitTurnInName;

    LocaleConstant localeConstant = _session->GetSessionDbLocaleIndex();
    if (QuestTemplateLocale const* questTemplateLocale = sQuestDataStore->GetQuestLocale(quest->GetQuestId()))
    {
        ObjectMgr::GetLocaleString(questTemplateLocale->LogTitle, localeConstant, questTitle);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitGiverText, localeConstant, portraitGiverText);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitGiverName, localeConstant, portraitGiverName);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitTurnInText, localeConstant, portraitTurnInText);
        ObjectMgr::GetLocaleString(questTemplateLocale->PortraitTurnInName, localeConstant, portraitTurnInName);
    }

    if (QuestOfferRewardLocale const* questTemplateLocale = sQuestDataStore->GetQuestOfferRewardLocale(quest->GetQuestId()))
        ObjectMgr::GetLocaleString(questTemplateLocale->OfferRewardText, localeConstant, questOfferRewardText);

    WorldPackets::Quest::QuestGiverOfferRewardMessage packet;
    WorldPackets::Quest::QuestGiverOfferReward& offer = packet.QuestData;

    quest->BuildQuestRewards(offer.Rewards, player);
    offer.QuestGiverGUID = npcGUID;

    // Is there a better way? what about game objects?
    if (Creature const* creature = sObjectAccessor->GetCreature(*player, npcGUID))
        offer.QuestGiverCreatureID = creature->GetCreatureTemplate()->Entry;

    offer.QuestID = quest->GetQuestId();
    offer.AutoLaunched = enableNext;
    offer.SuggestedPartyMembers = quest->SuggestedPlayers;

    for (uint32 i = 0; i < QUEST_EMOTE_COUNT && quest->OfferRewardEmote[i]; ++i)
        offer.Emotes.emplace_back(quest->OfferRewardEmote[i], quest->OfferRewardEmoteDelay[i]);

    offer.QuestFlags[0] = quest->GetFlags();
    offer.QuestFlags[1] = quest->FlagsEx;

    packet.QuestTitle = questTitle;
    packet.RewardText = questOfferRewardText;
    packet.PortraitTurnIn = quest->QuestTurnInPortrait;
    packet.PortraitGiver = quest->QuestGiverPortrait;
    packet.PortraitGiverText = portraitGiverText;
    packet.PortraitGiverName = portraitGiverName;
    packet.PortraitTurnInText = portraitTurnInText;
    packet.PortraitTurnInName = portraitTurnInName;
    packet.QuestPackageID = quest->PackageID;
    player->SendDirectMessage(packet.Write());
}

uint32 GossipMenu::GetMenuItemCount() const
{
    return _menuItems.size();
}

bool GossipMenu::Empty() const
{
    return _menuItems.empty();
}

GossipMenuItem const* GossipMenu::GetItem(uint32 id) const
{
    return Trinity::Containers::MapGetValuePtr(_menuItems, id);
}

GossipMenuItemData const* GossipMenu::GetItemData(uint32 indexId) const
{
    return Trinity::Containers::MapGetValuePtr(_menuItemData, indexId);
}

void PlayerMenu::SendQuestGiverRequestItems(Quest const* quest, ObjectGuid npcGUID, bool canComplete, bool closeOnCancel) const
{
    // We can always call to RequestItems, but this packet only goes out if there are actually
    // items.  Otherwise, we'll skip straight to the OfferReward

    std::string questTitle = quest->LogTitle;
    std::string requestItemsText = quest->RequestItemsText;

    LocaleConstant localeConstant = _session->GetSessionDbLocaleIndex();
    if (QuestTemplateLocale const* questTemplateLocale = sQuestDataStore->GetQuestLocale(quest->GetQuestId()))
        ObjectMgr::GetLocaleString(questTemplateLocale->LogTitle, localeConstant, questTitle);

    if (QuestRequestItemsLocale const* questRequestItemsLocale = sQuestDataStore->GetQuestRequestItemsLocale(quest->GetQuestId()))
        ObjectMgr::GetLocaleString(questRequestItemsLocale->RequestItemsText, localeConstant, requestItemsText);

    if (!quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_DELIVER) && canComplete)
    {
        SendQuestGiverOfferReward(quest, npcGUID, true);
        return;
    }

    WorldPackets::Quest::QuestGiverRequestItems packet;
    packet.QuestGiverGUID = npcGUID;

    // Is there a better way? what about game objects?
    if (Creature const* creature = sObjectAccessor->GetCreature(*_session->GetPlayer(), npcGUID))
        packet.QuestGiverCreatureID = creature->GetCreatureTemplate()->Entry;

    packet.QuestID = quest->GetQuestId();

    if (canComplete)
    {
        packet.CompEmoteDelay = quest->EmoteOnCompleteDelay;
        packet.CompEmoteType = quest->EmoteOnComplete;
    }
    else
    {
        packet.CompEmoteDelay = quest->EmoteOnIncompleteDelay;
        packet.CompEmoteType = quest->EmoteOnIncomplete;
    }

    packet.QuestFlags[0] = quest->GetFlags();
    packet.QuestFlags[1] = quest->FlagsEx;
    packet.SuggestPartyMembers = quest->SuggestedPlayers;
    packet.StatusFlags = 0xDF; // Unk, send common value

    for (QuestObjective const& obj : quest->GetObjectives())
    {
        switch (obj.Type)
        {
            case QUEST_OBJECTIVE_ITEM:
                packet.Collect.emplace_back(obj.ObjectID, obj.Amount, obj.Flags);
                break;
            case QUEST_OBJECTIVE_CURRENCY:
                packet.Currency.emplace_back(obj.ObjectID, obj.Amount);
                break;
            case QUEST_OBJECTIVE_MONEY:
                packet.MoneyToGet += obj.Amount;
                break;
            default:
                break;
        }
    }

    packet.AutoLaunched = closeOnCancel;
    packet.QuestTitle = questTitle;
    packet.CompletionText = requestItemsText;

    if (Player* player = _session->GetPlayer())
        player->SendDirectMessage(packet.Write());
}

uint8 QuestMenu::GetMenuItemCount() const
{
    return _questMenuItems.size();
}

bool QuestMenu::Empty() const
{
    return _questMenuItems.empty();
}
