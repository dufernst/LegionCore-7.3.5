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

#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "icecrown_citadel.h"
#include "Spell.h"

#define GOSSIP_SENDER_ICC_PORT 631

class icecrown_citadel_teleport : public GameObjectScript
{
    public:
        icecrown_citadel_teleport() : GameObjectScript("icecrown_citadel_teleport") { }

        bool OnGossipHello(Player* player, GameObject* go) override
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to Light's Hammer.", GOSSIP_SENDER_ICC_PORT, LIGHT_S_HAMMER_TELEPORT);
            if (InstanceScript* instance = go->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_LORD_MARROWGAR) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to the Oratory of the Damned.", GOSSIP_SENDER_ICC_PORT, ORATORY_OF_THE_DAMNED_TELEPORT);
                if (instance->GetBossState(DATA_LADY_DEATHWHISPER) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to the Rampart of Skulls.", GOSSIP_SENDER_ICC_PORT, RAMPART_OF_SKULLS_TELEPORT);
                if (instance->GetBossState(DATA_LADY_DEATHWHISPER) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to the Deathbringer's Rise.", GOSSIP_SENDER_ICC_PORT, DEATHBRINGER_S_RISE_TELEPORT);
                if (instance->GetBossState(DATA_DEATHBRINGER_SAURFANG) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to the Upper Spire.", GOSSIP_SENDER_ICC_PORT, UPPER_SPIRE_TELEPORT);
                if (instance->GetBossState(DATA_VALITHRIA_DREAMWALKER) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to the Sindragosa's Lair", GOSSIP_SENDER_ICC_PORT, SINDRAGOSA_S_LAIR_TELEPORT);
                if (instance->GetBossState(DATA_PROFESSOR_PUTRICIDE) == DONE && instance->GetBossState(DATA_BLOOD_QUEEN_LANA_THEL) == DONE && instance->GetBossState(DATA_SINDRAGOSA) == DONE)
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport to The Frozen Throne", GOSSIP_SENDER_ICC_PORT, FROZEN_THRONE_TELEPORT);
            }

            player->SEND_GOSSIP_MENU(go->GetGOInfo()->GetGossipMenuId(), go->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();
            player->CLOSE_GOSSIP_MENU();
            if (InstanceScript* instance = go->GetInstanceScript())
                if(instance->IsEncounterInProgress())
                    return false;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(action);
            if (!spellInfo)
                return false;

            if (player->isInCombat())
            {
                TriggerCastData triggerData;
                triggerData.triggerFlags = TRIGGERED_FULL_MASK;

                Spell* spell = new Spell(player, spellInfo, triggerData);
                spell->SendCastResult(player, spellInfo, SPELL_FAILED_AFFECTING_COMBAT);
                spell->finish(false);
                delete spell;
                return true;
            }

            if (sender == GOSSIP_SENDER_ICC_PORT)
            {
                player->CastSpell(player, 12438, true);
                //player->CastSpell(player, spell, true);
            }
                
            switch(action)
            {
                case LIGHT_S_HAMMER_TELEPORT:
                    player->TeleportTo(631, -17.192f, 2211.440f, 30.1158f, 3.121f);
                    break;
                case ORATORY_OF_THE_DAMNED_TELEPORT:
                    player->TeleportTo(631, -503.620f, 2211.470f, 62.8235f, 3.139f);
                    break;
                case RAMPART_OF_SKULLS_TELEPORT:
                    player->TeleportTo(631, -615.145f, 2211.470f, 199.972f, 6.268f);
                    break;
                case DEATHBRINGER_S_RISE_TELEPORT:
                    player->TeleportTo(631, -549.131f, 2211.290f, 539.291f, 6.275f);
                    break;
                case UPPER_SPIRE_TELEPORT:
                    player->TeleportTo(631, 4199.407f, 2769.478f, 351.064f, 6.258f);
                    break;
                case FROZEN_THRONE_TELEPORT:
                    player->TeleportTo(631, 4356.580f, 2565.750f, 220.40f, 4.886f);
                    break;
                case SINDRAGOSA_S_LAIR_TELEPORT:
                    player->TeleportTo(631, 4356.380f, 2565.555f, 220.462f, 4.704f);
                    break;
                case PLAGUEWORKS_TELEPORT:
                    player->TeleportTo(631, 4357.188f, 2864.480f, 349.330f, 4.738f);
                    break;
                case CRIMSON_HALL_TELEPORT:
                    player->TeleportTo(631, 4452.969f, 2769.371f, 349.349f, 3.195f);
                    break;
                case FROSTWING_HALLS_TELEPORT:
                    player->TeleportTo(631, 4357.062f, 2674.460f, 349.344f, 1.603f);
                    break;
            }

            return true;
        }
};

class at_frozen_throne_teleport : public AreaTriggerScript
{
    public:
        at_frozen_throne_teleport() : AreaTriggerScript("at_frozen_throne_teleport") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/, bool /*enter*/) override
        {
            if (player->isInCombat())
            {
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(FROZEN_THRONE_TELEPORT))
                {
                    TriggerCastData triggerData;
                    triggerData.triggerFlags = TRIGGERED_FULL_MASK;

                    Spell* spell = new Spell(player, spellInfo, triggerData);
                    spell->SendCastResult(player, spellInfo, SPELL_FAILED_AFFECTING_COMBAT);
                    spell->finish(false);
                    delete spell;
                }
                return true;
            }
            
            /*if (InstanceScript* instance = player->GetInstanceScript())
                if (instance->GetBossState(DATA_PROFESSOR_PUTRICIDE) == DONE &&
                    instance->GetBossState(DATA_BLOOD_QUEEN_LANA_THEL) == DONE &&
                    instance->GetBossState(DATA_SINDRAGOSA) == DONE &&
                    instance->GetBossState(DATA_THE_LICH_KING) != IN_PROGRESS)
                    {*/
                        player->CastSpell(player, 12438, true);
                        player->CastSpell(player, FROZEN_THRONE_TELEPORT, true);
                    //}

            return true;
        }
};

void AddSC_icecrown_citadel_teleport()
{
    new icecrown_citadel_teleport();
    new at_frozen_throne_teleport();
}
