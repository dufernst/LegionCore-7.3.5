/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Silithus
SD%Complete: 100
SDComment: Quest support: 7785, 8304.
SDCategory: Silithus
EndScriptData */

/* ContentData
npc_highlord_demitrian
npcs_rutgar_and_frankal
go_wind_stone
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Group.h"

/*###
## npc_highlord_demitrian
###*/

#define GOSSIP_DEMITRIAN1 "What do you know of it?"
#define GOSSIP_DEMITRIAN2 "I am listening, Demitrian."
#define GOSSIP_DEMITRIAN3 "Continue, please."
#define GOSSIP_DEMITRIAN4 "A battle?"
#define GOSSIP_DEMITRIAN5 "<Nod>"
#define GOSSIP_DEMITRIAN6 "Caught unaware? How?"
#define GOSSIP_DEMITRIAN7 "So what did Ragnaros do next?"

class npc_highlord_demitrian : public CreatureScript
{
public:
    npc_highlord_demitrian() : CreatureScript("npc_highlord_demitrian") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(6842, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(6843, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(6844, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(6867, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(6868, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(6869, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->SEND_GOSSIP_MENU(6870, creature->GetGUID());

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 19016, 1);
            if (msg == EQUIP_ERR_OK)
                player->StoreNewItem(dest, 19016, true);
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(7785) == QUEST_STATUS_NONE &&
            (player->HasItemCount(18563, 1, false) || player->HasItemCount(18564, 1, false)))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DEMITRIAN1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->SEND_GOSSIP_MENU(6812, creature->GetGUID());
            return true;
    }

};

/*###
## npcs_rutgar_and_frankal
###*/

//gossip item text best guess
#define GOSSIP_ITEM1 "I seek information about Natalia"

#define GOSSIP_ITEM2 "That sounds dangerous!"
#define GOSSIP_ITEM3 "What did you do?"
#define GOSSIP_ITEM4 "Who?"
#define GOSSIP_ITEM5 "Women do that. What did she demand?"
#define GOSSIP_ITEM6 "What do you mean?"
#define GOSSIP_ITEM7 "What happened next?"

#define GOSSIP_ITEM11 "Yes, please continue"
#define GOSSIP_ITEM12 "What language?"
#define GOSSIP_ITEM13 "The Priestess attacked you?!"
#define GOSSIP_ITEM14 "I should ask the monkey about this"
#define GOSSIP_ITEM15 "Then what..."

enum RutgarAndFrankal //trigger creatures to kill
{
    TRIGGER_FRANKAL     = 15221,
    TRIGGER_RUTGAR      = 15222
};

class npcs_rutgar_and_frankal : public CreatureScript
{
public:
    npcs_rutgar_and_frankal() : CreatureScript("npcs_rutgar_and_frankal") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(7755, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(7756, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->SEND_GOSSIP_MENU(7757, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(7758, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(7759, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                player->SEND_GOSSIP_MENU(7760, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                player->SEND_GOSSIP_MENU(7761, creature->GetGUID());
                                                                //'kill' our trigger to update quest status
                player->KilledMonsterCredit(TRIGGER_RUTGAR, ObjectGuid::Empty);
                break;

            case GOSSIP_ACTION_INFO_DEF + 9:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM11, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                player->SEND_GOSSIP_MENU(7762, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 10:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM12, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                player->SEND_GOSSIP_MENU(7763, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 11:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM13, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                player->SEND_GOSSIP_MENU(7764, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 12:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM14, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
                player->SEND_GOSSIP_MENU(7765, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 13:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM15, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
                player->SEND_GOSSIP_MENU(7766, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 14:
                player->SEND_GOSSIP_MENU(7767, creature->GetGUID());
                                                                //'kill' our trigger to update quest status
                player->KilledMonsterCredit(TRIGGER_FRANKAL, ObjectGuid::Empty);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(8304) == QUEST_STATUS_INCOMPLETE &&
            creature->GetEntry() == 15170 &&
            !player->GetReqKillOrCastCurrentCount(8304, TRIGGER_RUTGAR))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        if (player->GetQuestStatus(8304) == QUEST_STATUS_INCOMPLETE &&
            creature->GetEntry() == 15171 &&
            player->GetReqKillOrCastCurrentCount(8304, TRIGGER_RUTGAR))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+9);

        player->SEND_GOSSIP_MENU(7754, creature->GetGUID());

        return true;
    }

};

void AddSC_silithus()
{
    new npc_highlord_demitrian();
    new npcs_rutgar_and_frankal();
}
