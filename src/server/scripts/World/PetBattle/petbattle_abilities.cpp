////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "PetBattle.h"
#include "PetBattleAbilityEffect.h"
#include "PlayerDefines.h"
#include "DB2Stores.h"

class PlayerScript_Battle_Pet_Training_Spell: public PlayerScript
{
public:
    PlayerScript_Battle_Pet_Training_Spell() : PlayerScript("PlayerScript_Battle_Pet_Training_Spell") { }

    void OnSpellLearned(Player* player, uint32 spellID) override
    {
        switch (spellID)
        {
            case 119467:
            case 122026:
            case 125439:
            case 125610:
            {
                if (!player->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_PET_BATTLES_UNLOCKED))
                {
                    player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_PET_BATTLES_UNLOCKED);
                    player->GetSession()->SendBattlePetJournal();
                    player->GetSession()->SendBattlePetTrapLevel();
                    player->GetSession()->SendBattlePetLicenseChanged();
                }
                break;
            }
            default:
                break;
        }
    }
};

void AddSC_petbattle_abilities()
{
    new PlayerScript_Battle_Pet_Training_Spell();
}
