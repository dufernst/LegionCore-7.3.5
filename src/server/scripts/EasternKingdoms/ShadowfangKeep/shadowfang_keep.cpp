#include "LFGMgr.h"
#include "Group.h"
#include "shadowfang_keep.h"

#define GOSSIP_SENDER_SHADOWFANG_PORT 33

Position shadowfang_keep_locs[] = 
{
    { -224.482f, 2199.782f, 79.761f, 0.376793f },
    { -254.083f, 2283.067f, 74.9995f, 2.481768f },
    { -249.633f, 2261.914f, 100.890f, 4.975412f },
    { -166.63f, 2180.783f, 129.255f, 5.929675f },
};

class npc_apothecary_hummel : public CreatureScript
{
    public:
        npc_apothecary_hummel() : CreatureScript("npc_apothecary_hummel") { }
        
        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new npc_apothecary_hummelAI (pCreature);
        }

        bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
        {
            pPlayer->PlayerTalkClass->ClearMenus();
            pPlayer->CLOSE_GOSSIP_MENU();

            if (uiAction == 1)
            {
                if (Creature* pFrye = pCreature->FindNearestCreature(NPC_APOTHECARY_FRYE, 100.0f))
                    pFrye->setFaction(14);
                if (Creature* pBaxter = pCreature->FindNearestCreature(NPC_APOTHECARY_BAXTER, 100.0f))
                    pBaxter->setFaction(14);
                pCreature->setFaction(14);
            }
            return false;
        }

        struct npc_apothecary_hummelAI : public ScriptedAI
        {
            npc_apothecary_hummelAI(Creature* pCreature) : ScriptedAI(pCreature)
            {
            }

            void JustDied(Unit* /*killer*/)
            {
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                {
                    Player* pPlayer = players.begin()->getSource();
                    if (pPlayer && pPlayer->GetGroup())
                        sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 288);
                }
            }
        };
};
        
class npc_haunted_stable_hand_portal : public CreatureScript
{
    public:
        npc_haunted_stable_hand_portal() : CreatureScript("npc_haunted_stable_hand_portal") { }

        bool OnGossipHello(Player* player, Creature* pCreature)
        {
            bool ru = player->GetSession()->GetSessionDbLocaleIndex() == LOCALE_ruRU;

            if (InstanceScript* instance = pCreature->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_VALDEN) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to Lower Observatory." : "Teleport to Lower Observatory.", GOSSIP_SENDER_SHADOWFANG_PORT, 3);
                else if (instance->GetBossState(DATA_SPRINGVALE) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to The Courtyard." : "Teleport to The Courtyard.", GOSSIP_SENDER_SHADOWFANG_PORT, 2);
                else if (instance->GetBossState(DATA_SILVERLAINE) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to Dining Hall." : "Teleport to Dining Hall.", GOSSIP_SENDER_SHADOWFANG_PORT, 1);
                else if (instance->GetBossState(DATA_ASHBURY) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ru ? "Teleport to The Courtyard." : "Teleport to The Courtyard.", GOSSIP_SENDER_SHADOWFANG_PORT, 0);
            }

            player->SEND_GOSSIP_MENU(player->GetGossipTextId(pCreature), pCreature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* /*pCreature*/, uint32 sender, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();

            if (action >= 4)
                return false;

            Position loc = shadowfang_keep_locs[action];
            if (!player->isInCombat())
                player->NearTeleportTo(loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetOrientation(), false);
            return true;
        }
};

void AddSC_shadowfang_keep()
{
    new npc_apothecary_hummel();
    new npc_haunted_stable_hand_portal();
}
