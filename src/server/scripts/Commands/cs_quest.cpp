/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
Name: quest_commandscript
%Complete: 100
Comment: All quest related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "QuestData.h"

class quest_commandscript : public CommandScript
{
public:
    quest_commandscript() : CommandScript("quest_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> questCommandTable =
        {
            { "add",            SEC_ADMINISTRATOR,  false, &HandleQuestAdd,                    ""},
            { "complete",       SEC_ADMINISTRATOR,  false, &HandleQuestComplete,               ""},
            { "remove",         SEC_ADMINISTRATOR,  false, &HandleQuestRemove,                 ""},
            { "reward",         SEC_ADMINISTRATOR,  false, &HandleQuestReward,                 ""},
            { "status",         SEC_PLAYER,         false, &HandleQuestStatus,                 ""}
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "quest",          SEC_ADMINISTRATOR,  false, NULL,                  "", questCommandTable }
        };
        return commandTable;
    }

    static bool HandleQuestAdd(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .addquest #entry'
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level:min_level:max_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sQuestDataStore->GetQuestTemplate(entry);
        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // check item starting quest (it can work incorrectly if added without item in inventory)
        ItemTemplateContainer const* itc = sObjectMgr->GetItemTemplateStore();
        ItemTemplateContainer::const_iterator result = std::find_if(itc->begin(), itc->end(), [entry](ItemTemplateContainer::value_type const& value)
        {
            return value.second.GetStartQuestID() == entry;
        });

        if (result != itc->end())
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, entry, result->second.GetId());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // ok, normal (creature/GO starting) quest
        if (player->CanAddQuest(quest, true))
        {
            player->AddQuest(quest, NULL);

            if (player->CanCompleteQuest(entry))
                player->CompleteQuest(entry);
        }

        return true;
    }

    static bool HandleQuestRemove(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .removequest #entry'
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level:min_level:max_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sQuestDataStore->GetQuestTemplate(entry);

        if (!quest)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // remove all quest entries for 'entry' from quest log
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuest = player->GetQuestSlotQuestId(slot);
            if (logQuest == entry)
            {
                player->SetQuestSlot(slot, 0);

                // we ignore unequippable quest items in this case, its' still be equipped
                player->TakeQuestSourceItem(logQuest, false);
            }
        }

        player->RemoveActiveQuest(entry);
        player->RemoveRewardedQuest(entry);

        handler->SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
        return true;
    }

    static bool HandleQuestComplete(ChatHandler* handler, const char* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .quest complete #entry
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level:min_level:max_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sQuestDataStore->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        for (uint32 i = 0; i < quest->Objectives.size(); ++i)
        {
            QuestObjective const& obj = quest->Objectives[i];

            switch (obj.Type)
            {
                case QUEST_OBJECTIVE_ITEM:
                {
                    uint32 curItemCount = player->GetItemCount(obj.ObjectID, true);
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, obj.ObjectID, obj.Amount - curItemCount);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = player->StoreNewItem(dest, obj.ObjectID, true);
                        player->SendNewItem(item, obj.Amount - curItemCount, true, false);
                    }
                    break;
                }
                case QUEST_OBJECTIVE_MONSTER:
                {
                    if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(obj.ObjectID))
                        for (uint16 z = 0; z < obj.Amount; ++z)
                            player->KilledMonster(creatureInfo, ObjectGuid::Empty);
                    break;
                }
                case QUEST_OBJECTIVE_GAMEOBJECT:
                {
                    for (uint16 z = 0; z < obj.Amount; ++z)
                        player->KillCreditGO(obj.ObjectID, ObjectGuid::Empty);
                    break;
                }
                case QUEST_OBJECTIVE_MIN_REPUTATION:
                {
                    uint32 curRep = player->GetReputationMgr().GetReputation(obj.ObjectID);
                    if (curRep < uint32(obj.Amount))
                        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(obj.ObjectID))
                            player->GetReputationMgr().SetReputation(factionEntry, obj.Amount);
                    break;
                }
                case QUEST_OBJECTIVE_MAX_REPUTATION:
                {
                    uint32 curRep = player->GetReputationMgr().GetReputation(obj.ObjectID);
                    if (curRep > uint32(obj.Amount))
                        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(obj.ObjectID))
                            player->GetReputationMgr().SetReputation(factionEntry, obj.Amount);
                    break;
                }
                case QUEST_OBJECTIVE_MONEY:
                {
                    player->ModifyMoney(obj.Amount);
                    break;
                }
            }
        }

        //if (sWorld->getBoolConfig(CONFIG_QUEST_ENABLE_QUEST_TRACKER)) // check if Quest Tracker is enabled
        //{
        //    // prepare Quest Tracker datas
        //    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_QUEST_TRACK_GM_COMPLETE);
        //    stmt->setUInt32(0, quest->GetQuestId());
        //    stmt->setUInt64(1, player->GetGUID().GetGUIDLow());

        //    // add to Quest Tracker
        //    CharacterDatabase.Execute(stmt);
        //}

        player->CompleteQuest(entry);
        return true;
    }

    static bool HandleQuestReward(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // .quest reward #entry
        // number or [name] Shift-click form |color|Hquest:quest_id:quest_level:min_level:max_level|h[name]|h|r
        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);

        Quest const* quest = sQuestDataStore->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) != QUEST_STATUS_COMPLETE)
        {
            handler->PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->RewardQuest(quest, 0, player);
        return true;
    }

    static bool HandleQuestStatus(ChatHandler* handler, char const* args)
    {
        Player* player = handler->getSelectedPlayer();
        if (!player)
        {
            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* cId = handler->extractKeyFromLink((char*)args, "Hquest");
        if (!cId)
            return false;

        uint32 entry = atol(cId);
        Quest const* quest = sQuestDataStore->GetQuestTemplate(entry);
        if (!quest)
        {
            handler->PSendSysMessage(600028, player->GetName(), entry);
            return false;
        }

        switch (player->GetQuestStatus(entry))
        {
        case QUEST_STATUS_INCOMPLETE:
            handler->PSendSysMessage(600025, entry, player->GetName());
            break;
        case QUEST_STATUS_COMPLETE:
            handler->PSendSysMessage(600026, entry, player->GetName());
            break;
        case QUEST_STATUS_REWARDED:
            handler->PSendSysMessage(600027, entry, player->GetName());
            break;
        case QUEST_STATUS_NONE:
            handler->PSendSysMessage(600028, player->GetName(), entry);
            break;
        }
        return true;
    }
};

void AddSC_quest_commandscript()
{
    new quest_commandscript();
}
