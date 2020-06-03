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

#ifndef PET_H
#define PET_H

#include "Unit.h"
#include "TemporarySummon.h"

enum ActionFeedback
{
    FEEDBACK_NONE            = 0,
    FEEDBACK_PET_DEAD        = 1,
    FEEDBACK_NOTHING_TO_ATT  = 2,
    FEEDBACK_CANT_ATT_TARGET = 3
};

enum PetTalk
{
    PET_TALK_SPECIAL_SPELL  = 0,
    PET_TALK_ATTACK         = 1
};

enum PetNameInvalidReason
{
    // custom, not send
    PET_NAME_SUCCESS                                        = 0,

    PET_NAME_INVALID                                        = 1,
    PET_NAME_NO_NAME                                        = 2,
    PET_NAME_TOO_SHORT                                      = 3,
    PET_NAME_TOO_LONG                                       = 4,
    PET_NAME_MIXED_LANGUAGES                                = 6,
    PET_NAME_PROFANE                                        = 7,
    PET_NAME_RESERVED                                       = 8,
    PET_NAME_THREE_CONSECUTIVE                              = 11,
    PET_NAME_INVALID_SPACE                                  = 12,
    PET_NAME_CONSECUTIVE_SPACES                             = 13,
    PET_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS          = 14,
    PET_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END   = 15,
    PET_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME              = 16
};

#define ACTIVE_SPELLS_MAX           4

#define PET_FOLLOW_DIST  1.0f
#define PET_FOLLOW_ANGLE (M_PI/2)

class Player;

class Pet : public Guardian
{
    public:
        explicit Pet(Player* owner, PetType type = MAX_PET_TYPE);
        virtual ~Pet();

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool isControlled() const;
        bool isTemporarySummoned() const;

        bool IsPermanentPetFor(Player* owner);              // pet have tab in character windows and set UNIT_FIELD_PET_NUMBER

        bool Create (ObjectGuid::LowType const& guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 pet_number);
        bool CreateBaseAtCreature(Creature* creature);
        bool CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner);
        bool CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask);
        bool LoadPetFromDB(Player* owner, uint32 petentry = 0, uint32 petnumber = 0);
        bool isBeingLoaded() const override { return m_loading;}
        void SavePetToDB(bool isDelete = false);
        void Remove();
        static void DeleteFromDB(uint32 guidlow);

        void setDeathState(DeathState s) override;                   // overwrite virtual Creature::setDeathState and Unit::setDeathState
        void Update(uint32 diff) override;                           // overwrite virtual Creature::Update and Unit::Update

        void SetSlot(PetSlot slot) { m_slot = slot; }
        PetSlot GetSlot() { return m_slot; }

        void GivePetLevel(uint8 level);
        void SynchronizeLevelWithOwner();
        bool HaveInDiet(ItemTemplate const* item) const;
        uint32 GetCurrentFoodBenefitLevel(uint32 itemlevel);
        void SetDuration(int32 dur) { m_duration = dur; }
        int32 GetDuration() { return m_duration; }

        bool HasSpell(uint32 spell) override;

        bool IsPetAura(Aura const* aura);

        void _LoadSpellCooldowns();
        void _SaveSpellCooldowns(SQLTransaction& trans);
        void _LoadAuras(uint32 timediff);
        void _SaveAuras(SQLTransaction& trans);
        void _LoadSpells();
        void _SaveSpells(SQLTransaction& trans);

        void CleanupActionBar();

        void InitPetCreateSpells();

        DeclinedName const* GetDeclinedNames() const { return m_declinedname; }

        bool    m_removed;                                  // prevent overwrite pet state in DB at next Pet::Update if pet already removed(saved)

        Unit* GetOwner() { return m_owner; }

        uint32 GetSpecializationId() const { return m_specialization; }
        void SetSpecialization(uint32 id) { m_specialization = id; }
        void LearnSpecializationSpell();
        void UnlearnSpecializationSpell();
        void CheckSpecialization();
        void ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs) override;
        
        uint32 GetGroupUpdateFlag() const { return m_groupUpdateMask; }
        void SetGroupUpdateFlag(uint32 flag);
        void ResetGroupUpdateFlag();

    protected:
        int32   m_duration;                                 // time until unsummon (used mostly for summoned guardians and not used for controlled pets)
        uint32  m_specialization;
        PetSlot m_slot;
        uint32  m_groupUpdateMask;

        DeclinedName *m_declinedname;

    private:
        void SaveToDB(uint32, uint64, uint32) override
        // override of Creature::SaveToDB     - must not be called
        {
            ASSERT(false);
        }
        void DeleteFromDB() override
        // override of Creature::DeleteFromDB - must not be called
        {
            ASSERT(false);
        }
};
#endif
