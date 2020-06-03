#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "deadmines.h"
#include "Spell.h"

#define GOSSIP_SENDER_DEADMINES_PORT 36

Position deadmines_locs[] = {
    { -64.1528f, -385.99f, 53.192f, 1.85005f },
    { -305.321f, -491.292f, 49.232f, 0.488691f },
    { -201.096f, -606.05f, 19.3022f, 2.74016f },
    { -129.915f, -788.898f, 17.3409f, 0.366518f },
};

enum Adds
{
    // quest
    NPC_EDWIN_CANCLEEF_1    = 42697, 
    NPC_ALLIANCE_ROGUE      = 42700,
    NPC_VANESSA_VANCLEEF_1  = 42371, // little
};

class go_defias_cannon : public GameObjectScript
{
    public:
        go_defias_cannon() : GameObjectScript("go_defias_cannon") { }

        bool OnGossipHello(Player* pPlayer, GameObject* pGo)
        {
            InstanceScript* instance = pGo->GetInstanceScript();
            if (!instance)
                return false;
            //if (instance->GetData(DATA_CANNON_EVENT) != CANNON_NOT_USED)
                //return false ;

            instance->SetData(DATA_CANNON_EVENT, CANNON_BLAST_INITIATED);
            return false;
        }
};

class deadmines_teleport : public GameObjectScript
{
    public:
        deadmines_teleport() : GameObjectScript("deadmines_teleport") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            bool ru = player->GetSession()->GetSessionDbLocaleIndex() == LOCALE_ruRU;

            if (InstanceScript* instance = go->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_GLUBTOK) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to entrance." : "Teleport to entrance.", GOSSIP_SENDER_DEADMINES_PORT, 0);

                if (instance->GetBossState(DATA_ADMIRAL) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to Ironclad Cove." : "Teleport to Ironclad Cove.", GOSSIP_SENDER_DEADMINES_PORT, 3);
                else if (instance->GetBossState(DATA_FOEREAPER) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to The Foundry." : "Teleport to The Foundry.", GOSSIP_SENDER_DEADMINES_PORT, 2);
                else if (instance->GetBossState(DATA_HELIX) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to The Mast Room." : "Teleport to The Mast Room.", GOSSIP_SENDER_DEADMINES_PORT, 1);
            }

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(go), go->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, GameObject* /*go*/, uint32 sender, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();

            if (action >= 4)
                return false;

            Position loc = deadmines_locs[action];
            if (!player->isInCombat())
                player->NearTeleportTo(loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation(), false);
            return true;
        }
};

void AddSC_deadmines()
{
    new go_defias_cannon();
    new deadmines_teleport();
}