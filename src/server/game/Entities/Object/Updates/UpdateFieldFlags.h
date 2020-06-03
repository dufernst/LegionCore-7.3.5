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

#ifndef _UPDATEFIELDFLAGS_H
#define _UPDATEFIELDFLAGS_H

#include "UpdateFields.h"

enum UpdatefieldFlags
{
    UF_FLAG_NONE                = 0x000,
    UF_FLAG_PUBLIC              = 0x001, // MIRROR_ALL
    UF_FLAG_PRIVATE             = 0x002, // MIRROR_SELF
    UF_FLAG_OWNER               = 0x004, // MIRROR_OWNER
    UF_FLAG_ITEM_OWNER          = 0x008,
    UF_FLAG_SPECIAL_INFO        = 0x010, // MIRROR_EMPATH
    UF_FLAG_PARTY_MEMBER        = 0x020, // MIRROR_PARTY
    UF_FLAG_UNIT_ALL            = 0x040, // MIRROR_UNIT_ALL
    UF_FLAG_DYNAMIC             = 0x080, // MIRROR_VIEWER_DEPENDENT
    UF_FLAG_UNK0X100            = 0x100,
    UF_FLAG_URGENT              = 0x200, // MIRROR_URGENT
    UF_FLAG_URGENT_SELF_ONLY    = 0x400, // MIRROR_URGENT_SELF_ONLY
};

extern uint32 ItemUpdateFieldFlags[CONTAINER_END];
extern uint32 UnitUpdateFieldFlags[PLAYER_FIELD_END];
extern uint32 GameObjectUpdateFieldFlags[GAMEOBJECT_END];
extern uint32 DynamicObjectUpdateFieldFlags[DYNAMIC_OBJECT_END];
extern uint32 CorpseUpdateFieldFlags[CORPSE_END];
extern uint32 AreaTriggerUpdateFieldFlags[AREA_TRIGGER_END];
extern uint32 SceneObjectUpdateFieldFlags[SCENEOBJECT_END];
extern uint32 ConversationUpdateFieldFlags[CONVERSATION_END];

extern uint32 UnitDynamicFieldFlags[PLAYER_DYNAMIC_END];
extern uint32 ItemDynamicFieldFlags[ITEM_DYNAMIC_END];
extern uint32 ConversationDynamicFieldFlags[CONVERSATION_DYNAMIC_END];
extern uint32 GameObjectDynamicUpdateFieldFlags[GAMEOBJECT_DYNAMIC_END];

#endif // _UPDATEFIELDFLAGS_H
