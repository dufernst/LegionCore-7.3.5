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

#ifndef TRINITY_CREATUREAIFACTORY_H
#define TRINITY_CREATUREAIFACTORY_H

#include "FactoryHolder.h"
#include "GameObjectAI.h"

struct SelectableAI : FactoryHolder<CreatureAI>, Permissible<Creature>
{
    SelectableAI(std::string id) : FactoryHolder<CreatureAI>(id) { }
};

template<class REAL_AI>
struct CreatureAIFactory : SelectableAI
{
    CreatureAIFactory(std::string name) : SelectableAI(name) { }

    CreatureAI* Create(void*) const override;

    int Permit(const Creature* c) const override { return REAL_AI::Permissible(c); }
};

template<class REAL_AI>
CreatureAI* CreatureAIFactory<REAL_AI>::Create(void* data) const
{
    Creature* creature = reinterpret_cast<Creature*>(data);
    return (new REAL_AI(creature));
}

typedef FactoryHolder<CreatureAI> CreatureAICreator;
typedef FactoryHolder<CreatureAI>::FactoryHolderRegistry CreatureAIRegistry;

#define sCreatureAIRegistry CreatureAIRegistry::instance()

struct SelectableGameObjectAI : FactoryHolder<GameObjectAI>, Permissible<GameObject>
{
    SelectableGameObjectAI(std::string id) : FactoryHolder<GameObjectAI>(id) { }
};

template<class REAL_GO_AI>
struct GameObjectAIFactory : SelectableGameObjectAI
{
    GameObjectAIFactory(std::string name) : SelectableGameObjectAI(name) { }

    GameObjectAI* Create(void*) const override;

    int Permit(const GameObject* g) const override { return REAL_GO_AI::Permissible(g); }
};

template<class REAL_GO_AI>
GameObjectAI* GameObjectAIFactory<REAL_GO_AI>::Create(void* data) const
{
    GameObject* go = reinterpret_cast<GameObject*>(data);
    return (new REAL_GO_AI(go));
}

typedef FactoryHolder<GameObjectAI> GameObjectAICreator;
typedef FactoryHolder<GameObjectAI>::FactoryHolderRegistry GameObjectAIRegistry;

#define sGameObjectAIRegistry GameObjectAIRegistry::instance()

#endif
