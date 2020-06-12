/*
    Kloaka
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "GameEventMgr.h"

// 97359
class npc_kloaka_capitan : public CreatureScript
{
public:
    npc_kloaka_capitan() : CreatureScript("npc_kloaka_capitan") { }
    
    
    bool OnGossipHello(Player* player, Creature* creature) 
    {
     
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());     
        
        player->ADD_GOSSIP_ITEM_DB(18778, 0, GOSSIP_SENDER_MAIN, 1);
        if(sGameEventMgr->IsActiveEvent(81))
            player->ADD_GOSSIP_ITEM_DB(18778, 1, GOSSIP_SENDER_MAIN, 2);
        else if(sGameEventMgr->IsActiveEvent(80))
        {
            player->ADD_GOSSIP_ITEM_DB(18778, 2, GOSSIP_SENDER_MAIN, 3);
            player->ADD_GOSSIP_ITEM_DB(18778, 3, GOSSIP_SENDER_MAIN, 4);
        }
        
        player->SEND_GOSSIP_MENU(27321, creature->GetGUID());
        return true;
    }
    
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        
        switch(action)
        {
            case 1:
                player->GetSession()->SendListInventory(creature->GetGUID());
                player->CLOSE_GOSSIP_MENU(); 
                break;
            case 2:
                {
                    if (!player->HasCurrency(1149, 50))
                    {
                        player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, creature, 1149);
                        player->CLOSE_GOSSIP_MENU(); 
                        return false;
                    }
                    player->ModifyCurrency(1149, -50, true, true);
                    sGameEventMgr->StopEvent(81);
                    sGameEventMgr->StartEvent(80);
                    creature->AI()->Talk(0);
                    Map::PlayerList const& players = creature->GetMap()->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player* player = itr->getSource())
                        {
                            player->RemoveAura(223203);
                            player->AddAura(223202, player);
                            player->AddDelayedEvent(1000, [player]() -> void
                            {
                                player->UpdateArea(player->GetAreaId()); // hack for update auras and PvP State
                            });
                        }
                }
                break;
            case 3:
                {
                    if (!player->HasCurrency(1149, 50))
                    {
                        player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, creature, 1149);
                        player->CLOSE_GOSSIP_MENU(); 
                        return false;
                    }
                    player->ModifyCurrency(1149, -50, true, true);
                    
                    sGameEventMgr->StopEvent(80);
                    sGameEventMgr->StartEvent(81);
                    creature->AI()->Talk(1);
                    Map::PlayerList const& players = creature->GetMap()->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                        if (Player* player = itr->getSource())
                        {
                            player->AddAura(223203, player);
                            player->RemoveAura(223202);
                            player->AddDelayedEvent(1000, [player]() -> void
                            {
                                player->UpdateArea(player->GetAreaId()); // hack for update auras and PvP State
                            });
                        }
                }
                break;
            case 4:
           //     player->CastSpell(player, 203892);
                break;
        }
        return true;
    }


};

// 220260 220265 220266 220253
class spell_kloaka_call_some_adds : public SpellScriptLoader
{
    public:
        spell_kloaka_call_some_adds() : SpellScriptLoader("spell_kloaka_call_some_adds") { }

        class spell_kloaka_call_some_adds_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_kloaka_call_some_adds_SpellScript);

            void HandleAfterCast()
            {
                uint32 eventid = 0;
                switch(GetSpellInfo()->Id)
                {
                    case 220260:
                        eventid = 82;
                        break;
                    case 220265:
                        eventid = 83;
                        break;
                    case 220266:
                        eventid = 84;
                        break;
                    case 220253:
                        eventid = 85;
                        break;
                }
                
                if (eventid)
                    sGameEventMgr->StartEvent(eventid, true);
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_kloaka_call_some_adds_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_kloaka_call_some_adds_SpellScript();
        }
};

void AddSC_Kloaka()
{
    new npc_kloaka_capitan();
    new spell_kloaka_call_some_adds();
};