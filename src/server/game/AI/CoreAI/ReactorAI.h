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

#ifndef TRINITY_REACTORAI_H
#define TRINITY_REACTORAI_H

#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "CreatureTextMgr.h"

class Unit;
typedef std::map<ObjectGuid, EventMap> PlayerEventMap;

class ReactorAI : public CreatureAI
{
    public:

        explicit ReactorAI(Creature* c) : CreatureAI(c), CreatureTexts(nullptr), CreatureCombatTexts(nullptr) {}

        void MoveInLineOfSight(Unit*);

        void Reset();
        void InitializeAI();
        void UpdateAI(uint32);
        static int Permissible(const Creature*);
        void EnterCombat(Unit* who);
        void JustDied(Unit* killer);

        void AddClientVisibility(ObjectGuid guid) override;
        void RemoveClientVisibility(ObjectGuid guid) override;

        void AttackedBy(Unit* who) override;
    protected:
        EventMap spellCasts;
        EventMap events;
        EventMap textCombatEvents;
        PlayerEventMap textEvents;
        CreatureTextGroup const* CreatureTexts;
        CreatureTextGroup const* CreatureCombatTexts;
};
#endif

