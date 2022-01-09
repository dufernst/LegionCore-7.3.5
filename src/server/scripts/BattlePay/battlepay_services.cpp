#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "DB2Stores.h"

#define ITEM_HEARTHSTONE 6948

template<uint32 t_Level> class BattlePay_Level : BattlePayProductScript
{
public:
    explicit BattlePay_Level(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        player->GiveLevel(t_Level);
        player->CastSpell(player, 34092, true);
        player->CastSpell(player, 54198, true);
        player->ModifyMoney(5000000);

        std::vector<Item*> toBeMailedCurrentEquipment;
        for (int es = EquipmentSlots::EQUIPMENT_SLOT_START; es < EquipmentSlots::EQUIPMENT_SLOT_END; es++)
        {
            if (Item* currentEquiped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, es))
            {
                ItemPosCountVec off_dest;
                if (player->CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, currentEquiped, false) == EQUIP_ERR_OK)
                {
                    player->RemoveItem(INVENTORY_SLOT_BAG_0, es, true);
                    player->StoreItem(off_dest, currentEquiped, true);
                }
                else
                    toBeMailedCurrentEquipment.push_back(currentEquiped);
            }
        }

        if (!toBeMailedCurrentEquipment.empty())
        {
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            MailDraft draft("Inventory Full: Old Equipment.",
                "To equip your new level boost gear, your old gear had to be unequiped. You did not have enough free bag space, the items that could not be added to your bag you can find in this mail.");

            for (Item* currentEquiped : toBeMailedCurrentEquipment)
            {
                player->MoveItemFromInventory(currentEquiped, true);
                currentEquiped->DeleteFromInventoryDB(trans);                   // deletes item from character's inventory
                currentEquiped->SaveToDB(trans);                                // recursive and not have transaction guard into self, item not in inventory and can be save standalone
                draft.AddItem(currentEquiped);
            }
            
            draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
            CharacterDatabase.CommitTransaction(trans);
        }

        std::vector<uint32> toBeMailedNewItems;
        for (const uint32 item : sDB2Manager.GetLowestIdItemLoadOutItemsBy(player->getClass(), 3))
            if (!player->StoreNewItemInBestSlots(item, 1))
                if (item != ITEM_HEARTHSTONE || !player->HasItemCount(ITEM_HEARTHSTONE, 1, true))
                    toBeMailedNewItems.push_back(item);

        if (!toBeMailedNewItems.empty())
        {
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            MailDraft draft("Inventory Full: Level Boost Items.",
                "You did not have enough free bag space to add all your complementary level boost items to your bags, those that did not fit you can find in this mail.");

            for (const uint32 item : toBeMailedNewItems)
            {
                if (Item* pItem = Item::CreateItem(item, 1, player))
                {
                    pItem->SaveToDB(trans);
                    draft.AddItem(pItem);
                }
            }

            draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
            CharacterDatabase.CommitTransaction(trans);
        }

        player->SaveToDB();
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        if (t_Level <= player->getLevel())
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::TooHighLevel, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

class playerScriptTokensAvailable : public PlayerScript
{
public:
    playerScriptTokensAvailable() : PlayerScript("playerScriptTokensAvailable") { }

    void OnLogin(Player* player) override
    {
        if (player)
        {
            if (player->GetSession()->GetBattlePayBalance() >= 100)
            {
                player->SendMessageToPlayer("Returning players received one free level boost per account, you have not used this level boost yet.");
                player->SendMessageToPlayer("Login the character you want to have the level boost on, and claim it via the ingame store (Ingame Menu -> Shop).");
            }
        }
    }
};

template <uint32 t_AccountServiceFlag> class BattlePay_AccountService : BattlePayProductScript
{
public:
    explicit BattlePay_AccountService(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* /*session*/, Battlepay::Product const& /*product*/) override
    {
        //session->SetServiceFlags(t_AccountServiceFlag);
    }

    bool CanBuy(WorldSession* /*session*/, Battlepay::Product const& /*product*/, std::string& reason) override
    {

        return true;
    }
};

void AddSC_BattlePay_Services()
{
    new BattlePay_Level<90>("battlepay_service_level90");
    new playerScriptTokensAvailable();
    //new BattlePay_AccountService<ServiceFlags::PremadePve>("battlepay_service_premade");
}
