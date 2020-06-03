/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Creature.h"
#include "CreatureAISelector.h"
#include "PassiveAI.h"

#include "MovementGenerator.h"
#include "Pet.h"
#include "TemporarySummon.h"
#include "CreatureAIFactory.h"
#include "ScriptMgr.h"

namespace FactorySelector
{
    CreatureAI* selectAI(Creature* creature)
    {
        CreatureAICreator const* aiFactory = nullptr;
        if (creature->isPet())
            aiFactory = sCreatureAIRegistry->GetRegistryItem("PetAI");

        if (!aiFactory)
            if (auto scriptedAI = sScriptMgr->GetCreatureAI(creature))
                return scriptedAI;

        auto ainame = creature->GetAIName();
        if (!aiFactory && !ainame.empty())
            aiFactory = sCreatureAIRegistry->GetRegistryItem(ainame);

        if (!aiFactory)
        {
            if (creature->isMinion())
                aiFactory = sCreatureAIRegistry->GetRegistryItem("BattlePetAI");

            if (creature->IsVehicle() && creature->CanVehicleAI())
                aiFactory = sCreatureAIRegistry->GetRegistryItem("VehicleAI");
            else if (creature->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN) && static_cast<Guardian*>(creature)->GetOwner()->IsPlayer())
                aiFactory = sCreatureAIRegistry->GetRegistryItem("PetAI");
            else if (creature->HasFlag64(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK))
                aiFactory = sCreatureAIRegistry->GetRegistryItem("NullCreatureAI");
            else if (creature->isGuard())
                aiFactory = sCreatureAIRegistry->GetRegistryItem("GuardAI");
            else if (creature->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
                aiFactory = sCreatureAIRegistry->GetRegistryItem("PetAI");
            else if (creature->isTotem())
                aiFactory = sCreatureAIRegistry->GetRegistryItem("TotemAI");
            else if (creature->isAnySummons())
                aiFactory = sCreatureAIRegistry->GetRegistryItem("AnyPetAI");
            else if (creature->isTrigger())
            {
                if (creature->m_templateSpells[0])
                    aiFactory = sCreatureAIRegistry->GetRegistryItem("TriggerAI");
                else
                    aiFactory = sCreatureAIRegistry->GetRegistryItem("NullCreatureAI");
            }
            else if (creature->GetCreatureType() == CREATURE_TYPE_CRITTER && !creature->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
                aiFactory = sCreatureAIRegistry->GetRegistryItem("CritterAI");
        }

        if (!aiFactory)
        {
            auto best_val = -1;
            auto const& l = sCreatureAIRegistry->GetRegisteredItems();
            for (const auto& iter : l)
            {
                CreatureAICreator const* factory = iter.second;
                auto p = dynamic_cast<const SelectableAI*>(factory);
                ASSERT(p);
                auto val = p->Permit(creature);
                if (val > best_val)
                {
                    best_val = val;
                    aiFactory = p;
                }
            }
        }

        // select NullCreatureAI if not another cases
        ainame = aiFactory == nullptr ? "NullCreatureAI" : aiFactory->key();

        TC_LOG_DEBUG(LOG_FILTER_TSCR, "Creature %u used AI is %s.", creature->GetGUIDLow(), ainame.c_str());
        creature->SetNPCAIName(ainame);
        return aiFactory == nullptr ? new NullCreatureAI(creature) : aiFactory->Create(creature);
    }

    MovementGenerator* selectMovementGenerator(Creature* creature)
    {
        auto& mv_registry(*MovementGeneratorRegistry::instance());
        ASSERT(creature->GetCreatureTemplate());
        auto mv_factory = mv_registry.GetRegistryItem(creature->GetDefaultMovementType());

        /* if (mv_factory == NULL)
        {
            int best_val = -1;
            StringVector l;
            mv_registry.GetRegisteredItems(l);
            for (StringVector::iterator iter = l.begin(); iter != l.end(); ++iter)
            {
            const MovementGeneratorCreator *factory = mv_registry.GetRegistryItem((*iter).c_str());
            const SelectableMovement *p = dynamic_cast<const SelectableMovement *>(factory);
            ASSERT(p != NULL);
            int val = p->Permit(creature);
            if (val > best_val)
            {
                best_val = val;
                mv_factory = p;
            }
            }
        }*/

        return mv_factory == nullptr ? nullptr : mv_factory->Create(creature);
    }

    GameObjectAI* SelectGameObjectAI(GameObject* go)
    {
        if (auto scriptedAI = sScriptMgr->GetGameObjectAI(go))
            return scriptedAI;

        auto aiFactory = sGameObjectAIRegistry->GetRegistryItem(go->GetAIName());
        auto ainame = aiFactory == nullptr || go->GetScriptId() ? "NullGameObjectAI" : aiFactory->key();

        TC_LOG_DEBUG(LOG_FILTER_TSCR, "GameObject %u used AI is %s.", go->GetGUIDLow(), ainame.c_str());

        return aiFactory == nullptr ? new NullGameObjectAI(go) : aiFactory->Create(go);
    }
}
