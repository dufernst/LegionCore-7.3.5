#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "blackrock_caverns.h"
#include "Spell.h"

#define GOSSIP_SENDER_PORT 645

class bc_teleport : public GameObjectScript
{
    public:
        bc_teleport() : GameObjectScript("bc_teleport") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport Inicio", GOSSIP_SENDER_PORT, 1);
            if (InstanceScript* instance = go->GetInstanceScript())
            {
                if (instance->GetData(DATA_ROMOGG) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport Boss 1", GOSSIP_SENDER_PORT, 2);
                if (instance->GetData(DATA_CORLA) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport Boss 2", GOSSIP_SENDER_PORT, 3);
                if (instance->GetData(DATA_KARSH) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport Boss 3 y 4", GOSSIP_SENDER_PORT, 4);

             }

            player->SEND_GOSSIP_MENU(go->GetGOInfo()->GetGossipMenuId(), go->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, GameObject* /*go*/, uint32 sender, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();

            if (player->isInCombat())
            {
                return true;
            }

            if (sender == GOSSIP_SENDER_PORT)
            { 
                if (action==1)
                   player->TeleportTo(645,233.02f,1128.43f,205.56f,3.40f);
                if (action==2)
                   player->TeleportTo(645,308.03f,951.24f,191.16f,6.21f);
                if (action==3)
                   player->TeleportTo(645,530.54f,863.57f,134.98f,3.15f);
                if (action==4)
                   player->TeleportTo(645,210.67f,713.62f,105.20f,4.50f);

            }
            return true;
        }
};

void AddSC_bc_teleport()
{
    new bc_teleport();
}
