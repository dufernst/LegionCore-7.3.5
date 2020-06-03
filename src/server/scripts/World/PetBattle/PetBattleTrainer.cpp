////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "PetBattle.h"
#include "BattlePetData.h"
#include "QuestData.h"
#include "PetBattleSystem.h"

enum
{
    PetBattleTrainerFightActionID = GOSSIP_ACTION_INFO_DEF + 0xABCD
};

class npc_PetBattleTrainer : public CreatureScript
{
public:
    npc_PetBattleTrainer() : CreatureScript("npc_PetBattleTrainer") { }

    struct npc_PetBattleTrainerAI : CreatureAI
    {
        bool isTrainer;
        std::set<uint32> questIDs{};

        npc_PetBattleTrainerAI(Creature* me) : CreatureAI(me)
        {
            isTrainer = false;

            if (sBattlePetDataStore->GetPetBattleTrainerTeam(me->GetEntry()).empty())
                return;

            for (auto const& v : sQuestDataStore->GetQuestObjectivesByType(QUEST_OBJECTIVE_PET_TRAINER_DEFEAT))
                if (v.ObjectID == me->GetEntry())
                {
                    isTrainer = true;
                    questIDs.insert(v.QuestID);
                }
        }

        void UpdateAI(uint32 /*diff*/) override { }

        void sGossipHello(Player* player) override
        {
            if (me->isQuestGiver())
                player->PrepareQuestMenu(me->GetGUID());

            if (isTrainer)
            {
                bool check = false;
                for (auto questID : questIDs)
                    if (player->GetQuestStatus(questID) == QUEST_STATUS_INCOMPLETE)
                    {
                        check = true;
                        break;
                    }
                if (check)
                    if (BroadcastTextEntry const* bct = sBroadcastTextStore.LookupEntry(62660))
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, DB2Manager::GetBroadcastTextValue(bct, player->GetSession()->GetSessionDbLocaleIndex()), GOSSIP_SENDER_MAIN, PetBattleTrainerFightActionID);
            }

            player->TalkedToCreature(me->GetEntry(), me->GetGUID());
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(me), me->GetGUID());
        }
    };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (action == PetBattleTrainerFightActionID)
        {
            player->CLOSE_GOSSIP_MENU();

            static float distance = 10.0f;

            float l_X = creature->m_positionX + (std::cos(creature->m_orientation) * distance);
            float l_Y = creature->m_positionY + (std::sin(creature->m_orientation) * distance);
            float l_Z = player->GetMap()->GetHeight(l_X, l_Y, MAX_HEIGHT);

            Position playerPosition = Position(l_X, l_Y, l_Z);
            playerPosition.m_orientation = atan2(creature->m_positionY - l_Y, creature->m_positionX - l_X);
            playerPosition.m_orientation = (playerPosition.m_orientation >= 0.0f) ? playerPosition.m_orientation : 2 * M_PI + playerPosition.m_orientation;

            Position trainerPosition = Position(creature->m_positionX, creature->m_positionY, creature->m_positionZ, creature->m_orientation);

            Position battleCenterPosition = Position((playerPosition.m_positionX + trainerPosition.m_positionX) / 2, (playerPosition.m_positionY + trainerPosition.m_positionY) / 2, 0.0f, trainerPosition.m_orientation + M_PI);
            battleCenterPosition.m_positionZ = player->GetMap()->GetHeight(battleCenterPosition.m_positionX, battleCenterPosition.m_positionY, MAX_HEIGHT);

            PetBattleRequest* battleRequest = sPetBattleSystem->CreateRequest(player->GetGUID());
            battleRequest->OpponentGuid = creature->GetGUID();
            battleRequest->PetBattleCenterPosition = battleCenterPosition;
            battleRequest->TeamPosition[PETBATTLE_TEAM_1] = playerPosition;
            battleRequest->TeamPosition[PETBATTLE_TEAM_2] = trainerPosition;
            battleRequest->RequestType = PETBATTLE_TYPE_PVE;

            eBattlePetRequests canEnterResult = sPetBattleSystem->CanPlayerEnterInPetBattle(player, battleRequest);
            if (canEnterResult != PETBATTLE_REQUEST_OK)
            {
                player->GetSession()->SendPetBattleRequestFailed(canEnterResult);
                sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);
                return true;
            }

            std::shared_ptr<BattlePetInstance> playerPets[MAX_PETBATTLE_SLOTS];
            std::shared_ptr<BattlePetInstance> wildBattlePets[MAX_PETBATTLE_SLOTS];

            for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
            {
                playerPets[i] = nullptr;
                wildBattlePets[i] = nullptr;
            }

            uint32 wildsPetCount = 0;
            for (BattlePetNpcTeamMember& v : sBattlePetDataStore->GetPetBattleTrainerTeam(creature->GetEntry()))
            {
                if (wildsPetCount >= MAX_PETBATTLE_SLOTS)
                    break;

                auto battlePetInstance = std::make_shared<BattlePetInstance>();

                uint32 l_DisplayID = 0;

                if (sBattlePetSpeciesStore.LookupEntry(v.Specie) && sObjectMgr->GetCreatureTemplate(sBattlePetSpeciesStore.LookupEntry(v.Specie)->CreatureID))
                {
                    l_DisplayID = sObjectMgr->GetCreatureTemplate(sBattlePetSpeciesStore.LookupEntry(v.Specie)->CreatureID)->Modelid[0];

                    if (!l_DisplayID)
                    {
                        l_DisplayID = sObjectMgr->GetCreatureTemplate(sBattlePetSpeciesStore.LookupEntry(v.Specie)->CreatureID)->Modelid[1];

                        if (!l_DisplayID)
                        {
                            l_DisplayID = sObjectMgr->GetCreatureTemplate(sBattlePetSpeciesStore.LookupEntry(v.Specie)->CreatureID)->Modelid[2];
                            if (!l_DisplayID)
                                l_DisplayID = sObjectMgr->GetCreatureTemplate(sBattlePetSpeciesStore.LookupEntry(v.Specie)->CreatureID)->Modelid[3];
                        }
                    }
                }

                battlePetInstance->JournalID.Clear();
                battlePetInstance->Slot = 0;
                battlePetInstance->NameTimeStamp = 0;
                battlePetInstance->Species = v.Specie;
                battlePetInstance->DisplayModelID = l_DisplayID;
                battlePetInstance->XP = 0;
                battlePetInstance->Flags = 0;
                battlePetInstance->Health = 20000;

                uint8 randQuality = sBattlePetDataStore->GetRandomQuailty();
                battlePetInstance->Quality = v.minquality > randQuality ? v.minquality : randQuality;
                battlePetInstance->Breed = sBattlePetDataStore->GetRandomBreedID(v.BreedIDs);
                battlePetInstance->Level = std::max(urand(v.minlevel, v.maxlevel), static_cast<uint32>(1));

                for (size_t i = 0; i < MAX_PETBATTLE_ABILITIES; ++i)
                    battlePetInstance->Abilities[i] = v.Ability[i];

                wildBattlePets[wildsPetCount] = battlePetInstance;
                wildBattlePets[wildsPetCount]->OriginalCreature.Clear();
                wildsPetCount++;
            }

            size_t playerPetCount = 0;
            std::shared_ptr<BattlePet>* petSlots = player->GetBattlePetCombatTeam();
            for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
            {
                if (!petSlots[i])
                    continue;

                if (playerPetCount >= MAX_PETBATTLE_SLOTS || playerPetCount >= player->GetUnlockedPetBattleSlot())
                    break;

                playerPets[playerPetCount] = std::make_shared<BattlePetInstance>();
                playerPets[playerPetCount]->CloneFrom(petSlots[i]);
                playerPets[playerPetCount]->Slot = playerPetCount;
                playerPets[playerPetCount]->OriginalBattlePet = petSlots[i];

                ++playerPetCount;
            }

            player->GetSession()->SendPetBattleFinalizeLocation(battleRequest);

            PetBattle* petBattle = sPetBattleSystem->CreateBattle();

            petBattle->Teams[PETBATTLE_TEAM_1]->OwnerGuid = player->GetGUID();
            petBattle->Teams[PETBATTLE_TEAM_1]->PlayerGuid = player->GetGUID();
            petBattle->Teams[PETBATTLE_TEAM_2]->OwnerGuid = creature->GetGUID();

            for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
            {
                if (playerPets[i])
                    petBattle->AddPet(PETBATTLE_TEAM_1, playerPets[i]);

                if (wildBattlePets[i])
                    petBattle->AddPet(PETBATTLE_TEAM_2, wildBattlePets[i]);
            }

            petBattle->BattleType = battleRequest->RequestType;
            petBattle->PveBattleType = PVE_PETBATTLE_TRAINER;

            player->_petBattleId = petBattle->ID;

            sPetBattleSystem->RemoveRequest(battleRequest->RequesterGuid);

            for (size_t i = 0; i < MAX_PETBATTLE_SLOTS; ++i)
            {
                if (playerPets[i])
                    playerPets[i] = nullptr;

                if (wildBattlePets[i])
                    wildBattlePets[i] = nullptr;
            }

            player->GetMotionMaster()->MovePointWithRot(PETBATTLE_ENTER_MOVE_SPLINE_ID, playerPosition.m_positionX, playerPosition.m_positionY, playerPosition.m_positionZ, playerPosition.m_orientation);
        }
        else
            player->CLOSE_GOSSIP_MENU();

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_PetBattleTrainerAI(creature);
    }
};

void AddSC_npc_PetBattleTrainer()
{
    new npc_PetBattleTrainer;
}
