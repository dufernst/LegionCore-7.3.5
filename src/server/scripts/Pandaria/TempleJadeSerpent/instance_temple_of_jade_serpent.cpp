/*==============
==============*/

enum eSpells
{
    SPELL_CORRUPTED_WATERS      = 115167,
    //Spells for Lorewalker Stonestep event.
    SPELL_SHA_CORRUPTION        = 107034, //On spawn: Lot of mobs and decoration
    //HAUNTING SHA SPELLS
    SPELL_SHA_EXPLOSION_2       = 111812,
    //ZAO SUNSEEKER SPELLS
    SPELL_SHA_CORRUPTION_2      = 123947,
    SPELL_SHOOT_SUN             = 112084, //10 52 00 - 11 26 50 - 11 52 35
    SPELL_SHOOT_SUN_2           = 112085,
    SPELL_SHA_CORRUPTION_3      = 120000, //OnSpawn: Jiang
    SPELL_HELLFIRE_ARROW        = 113017,
    //LOREWALKER STONESTEP SPELLS
    SPELL_JADE_BLAST            = 107035,
    SPELL_JADE_BLAST_2          = 107048,
    SPELL_ROOT_SELF             = 106822,
    SPELL_LOREWALKER_ALACRITY   = 122714, //To cast on players
    SPELL_MEDITATION            = 122715,
    //SCROLL SPELLS
    SPELL_SCROLL_FLOOR          = 107350, //On spawn
    SPELL_JADE_ENERGY           = 107384,
    SPELL_JADE_ENERGY_2         = 111452, //On spawn
    SPELL_DRAW_SHA_2            = 111393,
    SPELL_DRAW_SHA_3            = 111431,
    SPELL_SHA_BURNING           = 111588, //OnDied
    SPELL_SHA_EXPLOSION         = 111579, //OnDied
    SPELL_DEATH                 = 98391,  //OnDied
    //SUN SPELLS
    SPELL_DRAW_SHA              = 111395,
    SPELL_SUN                   = 107349,
    SPELL_GROW_LOW              = 104921,
    SPELL_GROW_HIGH             = 111701,
    SPELL_SUNFIRE_EXPLOSION     = 111737,
    SPELL_DUMMY_NUKE            = 105898, //11 00 04 - 11 07 05 - 11 40 22
    SPELL_SUNFIRE_BLAST         = 111853,
    SPELL_SUNFIRE_RAYS          = 107223, //09 26 28 - 09 46 29 - 10 06 46
    SPELL_EXTRACT_SHA           = 111764,
    SPELL_EXTRACT_SHA_2         = 111806,
    SPELL_EXTRACT_SHA_3         = 111807,
    SPELL_EXTRACT_SHA_4         = 111768,
    SPELL_UNKNOWN               = 105581,
    //JIANG SPELLS
    SPELL_JUGGLER_JIANG         = 114745, //OnSpawn: Jiang
    //THE SONGBIRD QUEEN SPELLS
    SPELL_SINGING_SONGBIRD      = 114789, //OnSpawn: Songbird
    //HAUNTING SHA SPELLS
    SPELL_HAUNTING_GAZE         = 114650, //OnSpawn
    //XIANG SPELLS
    SPELL_JUGGLER_XIANG         = 114718,
    //FISH SPELLS
    SPELL_WATER_BUBBLE          = 114549, //OnSpawn
    //ChannelSpell : 42808, 512
    
    SPELL_POSSESSED_BY_SHA      = 110164, //On Spawn
    SPELL_JADE_ESSENCE          = 106797, //AddAura on phase 2
    SPELL_TRANSFORM_VISUAL      = 74620, //When the dragon is dead, cast this and remove the possess aura.
    
    SPELL_FIGMENT_OF_DOUBT_2    = 106935,
    SPELL_COPY_WEAPON           = 41054,
    SPELL_COPY_WEAPON_2         = 41055,
    SPELL_BOUNDS_OF_REALITY_2   = 117665
};

enum eCreatures
{
    CREATURE_WISE_MARI              = 56448,

    CREATURE_SCROLL                 = 57080,
    CREATURE_ZAO_SUNSEEKER          = 58826,
    CREATURE_LOREWALKTER_STONESTEP  = 56843,
    CREATURE_SUN                    = 56915,
    CREATURE_SUN_TRIGGER            = 56861,
    CREATURE_HAUNTING_SHA_2         = 58856,

    CREATURE_STRIFE                 = 59726,
    CREATURE_PERIL                  = 59051,

    CREATURE_MINION_OF_DOUBTS       = 57109,
    CREATURE_LIU_FLAMEHEART         = 56732,
    CREATURE_YU_LON                 = 56762,
    CREATURE_JADE_FIRE              = 56893,

    CREATURE_FIGMENT_OF_DOUBT       = 56792,
    CREATURE_SHA_OF_DOUBT           = 56439
};

enum eGameObjects
{
    GAMEOBJECT_DOOR_WISE_MARI           = 213550,
    GAMEOBJECT_DOOR_LOREWALKER_STONSTEP = 213549,
    GAMEOBJECT_DOOR_LIU_FLAMEHEART      = 213548,
    GAMEOBJECT_DOOR_LIU_FLAMEHEART_2    = 213544
};

enum eTypes
{
    TYPE_LOREWALKTER_STONESTEP = 0,
    TYPE_NUMBER_SUN_DEFEATED = 1,
    TYPE_SET_SUNS_SELECTABLE = 2,
    TYPE_SET_SCROLL_SELECTABLE = 4,
    TYPE_GET_EVENT_LOREWALKER_STONESTEP = 5,
    TYPE_LIU_FLAMEHEART_STATUS = 6,
    TYPE_IS_WIPE = 7,
    TYPE_CLASS_FIGMENT = 8,
    TYPE_CLASS_FIGMENT_DIE = 9
};

enum eStatus
{
    STATUS_LOREWALKER_STONESTEP_NONE        = 1,
    STATUS_LOREWALKER_STONESTEP_INTRO       = 2,
    STATUS_LOREWALKER_STONESTEP_SPAWN_SUNS  = 3,
    STATUS_LOREWALKER_STONESTEP_SPAWN_SUNS_2= 4,
    STATUS_LOREWALKER_STONESTEP_ZAO_COMBAT  = 5,
    STATUS_LOREWALKER_STONESTEP_FINISH      = 6,
};

class instance_temple_of_jade_serpent : public InstanceMapScript
{
public:
    instance_temple_of_jade_serpent() : InstanceMapScript("instance_temple_of_jade_serpent", 960) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_temple_of_jade_serpent_InstanceMapScript(map);
    }

    struct instance_temple_of_jade_serpent_InstanceMapScript : public InstanceScript
    {
        Position roomCenter;
        uint32 waterDamageTimer;
        ObjectGuid doorWiseMari;
        ObjectGuid wiseMariGUID;

        uint8 eventChoosen;
        ObjectGuid lorewalkter_stonestep;
        ObjectGuid zao_sunseeker;
        ObjectGuid scroll;
        ObjectGuid door_lorewalker;
        ObjectGuid guidPeril;
        ObjectGuid guidStrife;
        uint32 eventStatus_lorewalkter_stonestep;
        uint32 eventStatus_numberSunDefeated;
        uint32 wipeTimer;
        GuidList creatures_corrupted;
        GuidList sunfires;
        GuidList suns;
        GuidList sha_summoned;

        uint32 countMinionDeads;
        ObjectGuid liuGuid;
        ObjectGuid doorLiu;
        ObjectGuid doorLiu_2;
        GuidList mobs_liu;

        ObjectGuid sha_of_doubt_guid;
        uint8 countDps;
        uint8 countHeal;
        uint8 countTank;
        uint8 countFigments;

        instance_temple_of_jade_serpent_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            // Wise Mari script
            doorWiseMari.Clear();
            roomCenter.m_positionX = 1046.941f;
            roomCenter.m_positionY = -2560.606f;
            roomCenter.m_positionZ = 174.9552f;
            roomCenter.m_orientation = 4.33f;
            waterDamageTimer = 250;
            wiseMariGUID.Clear();

            //LoreWalkter Stonestep script.
            lorewalkter_stonestep.Clear();
            zao_sunseeker.Clear();
            scroll.Clear();
            guidPeril.Clear();
            guidStrife.Clear();
            eventStatus_lorewalkter_stonestep = STATUS_LOREWALKER_STONESTEP_NONE;
            eventStatus_numberSunDefeated = 0;
            eventChoosen = 0;
            wipeTimer = 3000;

            //Liu Flameheart script.
            countMinionDeads = 0;
            liuGuid.Clear();
            doorLiu.Clear();
            doorLiu_2.Clear();

            //Sha of doubt script.
            sha_of_doubt_guid.Clear();
            countDps = 0;
            countTank = 0;
            countHeal = 0;
            countFigments = 0;
        }

        void Initialize() override {}

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GAMEOBJECT_DOOR_WISE_MARI:
                    doorWiseMari = go->GetGUID();
                    break;
                case GAMEOBJECT_DOOR_LIU_FLAMEHEART_2:
                    doorLiu_2 = go->GetGUID();
                    break;
                case GAMEOBJECT_DOOR_LOREWALKER_STONSTEP:
                    door_lorewalker = go->GetGUID();
                    break;
                case GAMEOBJECT_DOOR_LIU_FLAMEHEART:
                    doorLiu = go->GetGUID();
                    break;
            }
        }

        
        void OnCreatureCreate(Creature* creature) override
        {
            OnCreatureCreate_lorewalker_stonestep(creature);
            OnCreatureCreate_liu_flameheart(creature);
            OnCreatureCreate_sha_of_doubt(creature);

            if (creature->GetEntry() == 56448)
                wiseMariGUID = creature->GetGUID();
        }

        void OnUnitDeath(Unit* unit) override
        {
            OnUnitDeath_lorewalker_stonestep(unit);
            OnUnitDeath_liu_flameheat(unit);
            OnUnitDeath_wise_mari(unit);
        }
        
        void Update(uint32 diff) override
        {
            // Challenge
            InstanceScript::Update(diff);

            //LOREWALKER STONESTEP: If Wipe, we must clean the event.
            if (wipeTimer <= diff && eventStatus_lorewalkter_stonestep >= STATUS_LOREWALKER_STONESTEP_INTRO && eventStatus_lorewalkter_stonestep < STATUS_LOREWALKER_STONESTEP_FINISH && IsWipe())
            {
                Wipe_lorewalker_stonestep();
                wipeTimer = 3000;
            }
            else
                wipeTimer -= diff;

            //WISE MARI
            if (waterDamageTimer <= diff)
            {
                instance->ApplyOnEveryPlayer([&](Player* player)
                {
                    if (auto wiseMari = Unit::GetUnit(*player, wiseMariGUID))
                    {
                        if (wiseMari->isAlive() || wiseMari->isInCombat())
                        {
                            Position pos;
                            player->GetPosition(&pos);

                            if ((player->GetDistance(roomCenter) < 20.00f && roomCenter.HasInArc(M_PI, &pos)) || (!roomCenter.HasInArc(M_PI, &pos) && player->GetDistance(roomCenter) < 14.00f))
                                if (player->GetPositionZ() > 174.05f && player->GetPositionZ() < 174.23f)
                                    player->CastSpell(player, SPELL_CORRUPTED_WATERS, true);

                            if (player->GetDistance(roomCenter) < 30.00f && player->GetPositionZ() > 170.19f && player->GetPositionZ() < 170.215f)
                                player->CastSpell(player, SPELL_CORRUPTED_WATERS, true);
                        }
                    }
                });

                waterDamageTimer = 250;
            }
            else
                waterDamageTimer -= diff;
        }

        void SetData(uint32 type, uint32 data) override
        {
            SetData_lorewalker_stonestep(type, data);
            SetData_sha_of_doubt(type, data);
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case TYPE_CLASS_FIGMENT:
                if (countDps < 2)
                    return 0;
                else if (countHeal == 0)
                    return 1;
                else if (countTank == 0)
                    return 2;
                return 3;
                break;
            case TYPE_IS_WIPE:
                return IsWipe();
            case TYPE_GET_EVENT_LOREWALKER_STONESTEP:
                return eventChoosen;
            case TYPE_LOREWALKTER_STONESTEP:
                return eventStatus_lorewalkter_stonestep;
            case TYPE_NUMBER_SUN_DEFEATED:
                return eventStatus_numberSunDefeated;
            default:
                return STATUS_NONE;
            case TYPE_LIU_FLAMEHEART_STATUS:
                Creature* creature = instance->GetCreature(liuGuid);
                if (!creature)
                    return 2;

                if (creature->GetHealthPct() < 70.f)
                    return 1;
                else
                    return 0;
                break;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
            case CREATURE_ZAO_SUNSEEKER:
                return zao_sunseeker;
            case CREATURE_SHA_OF_DOUBT:
                return sha_of_doubt_guid;
            }
            return ObjectGuid::Empty;
        }

        void OnUnitDeath_wise_mari(Unit* unit)
        {
            if (unit->GetEntry() == CREATURE_WISE_MARI)
                if (auto go = instance->GetGameObject(doorWiseMari))
                    go->SetGoState(GO_STATE_ACTIVE);
        }

        void SetData_sha_of_doubt(uint32 type, uint32 data)
        {
            switch (type)
            {
            case TYPE_CLASS_FIGMENT_DIE:
                if (data == 0)
                    ++countDps;
                else if (data == 1)
                    ++countHeal;
                else
                    ++countTank;

                if (countDps + countHeal + countTank == countFigments)
                    if (auto sha_doubt = instance->GetCreature(sha_of_doubt_guid))
                        sha_doubt->RemoveAura(SPELL_BOUNDS_OF_REALITY_2);
                break;
            case TYPE_CLASS_FIGMENT:
                countFigments = 0;
                countDps = 0;
                countHeal = 0;
                countTank = 0;
                break;
            }
        }

        void OnCreatureCreate_sha_of_doubt(Creature* creature)
        {
            switch (creature->GetEntry())
            {
            case CREATURE_SHA_OF_DOUBT:
                sha_of_doubt_guid = creature->GetGUID();
                break;
            case CREATURE_FIGMENT_OF_DOUBT:
                if (creature->ToTempSummon())
                {
                    ++countFigments;

                    if (auto summoner = creature->ToTempSummon()->GetSummoner())
                    {
                        summoner->AddAura(SPELL_FIGMENT_OF_DOUBT_2, creature);
                        creature->SetDisplayId(summoner->GetDisplayId());
                        creature->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_MIRROR_IMAGE);
                        summoner->CastSpell(creature, SPELL_COPY_WEAPON, false);
                        summoner->CastSpell(creature, SPELL_COPY_WEAPON_2, false);
                        creature->setFaction(14);

                        uint32 prevItem = creature->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS);

                        if (auto player = summoner->ToPlayer())
                        {
                            if (Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
                                creature->SetVirtualItem(0, mainItem->GetEntry());
                            else
                                creature->SetVirtualItem(0, summoner->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS));

                            prevItem = creature->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS) + 1;

                            if (Item* offItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                                creature->SetVirtualItem(1, offItem->GetEntry());
                            else
                                creature->SetVirtualItem(1, summoner->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS + 1));

                            prevItem = creature->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS) + 2;

                            if (Item* rangedItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
                                creature->SetVirtualItem(2, rangedItem->GetEntry());
                            else
                                creature->SetVirtualItem(2, summoner->GetUInt32Value(UNIT_FIELD_VIRTUAL_ITEMS + 2));
                        }
                    }
                }
                break;
            }
        }

        void OnCreatureCreate_liu_flameheart(Creature* creature)
        {
            switch (creature->GetEntry())
            {
            case CREATURE_JADE_FIRE:
                creature->setFaction(14);
                creature->SetDisplayId(11686);
                creature->SetReactState(REACT_PASSIVE);
                creature->CastSpell(creature, 107108, true);
                creature->ForcedDespawn(5000);
                break;
            case CREATURE_LIU_FLAMEHEART:
                liuGuid = creature->GetGUID();
                break;
            }
        }

        void OnUnitDeath_liu_flameheat(Unit* unit)
        {
            if (unit->ToCreature() && unit->ToCreature()->GetEntry() == CREATURE_MINION_OF_DOUBTS)
            {
                if (unit->GetAreaId() == 6119)
                {
                    ++countMinionDeads;

                    if (countMinionDeads == 3)
                        unit->SummonCreature(CREATURE_LIU_FLAMEHEART, 929.787f, -2561.016f, 180.070f + 5);
                }
            }

            if (unit->ToCreature() && unit->ToCreature()->GetEntry() == CREATURE_YU_LON)
            {
                if (auto creature = instance->GetCreature(liuGuid))
                {
                    creature->RemoveAura(SPELL_JADE_ESSENCE);
                    creature->CastSpell(creature, SPELL_TRANSFORM_VISUAL, false);
                    creature->RemoveAura(SPELL_POSSESSED_BY_SHA);
                    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                    if (auto liu = instance->GetCreature(liuGuid))
                        liu->Kill(liu, true);

                    if (auto Gutoogo = instance->GetGameObject(doorLiu))
                        Gutoogo->SetGoState(GO_STATE_ACTIVE);

                    if (auto go2 = instance->GetGameObject(doorLiu_2))
                        go2->SetGoState(GO_STATE_ACTIVE);
                }
            }
        }

        void SetData_lorewalker_stonestep(uint32 type, uint32 data)
        {
            switch (type)
            {
            case TYPE_SET_SCROLL_SELECTABLE:
                if (auto c = instance->GetCreature(scroll))
                    c->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                break;
            case TYPE_SET_SUNS_SELECTABLE:
                if (eventChoosen != 1)
                    return;

                for (GuidList::const_iterator guid = sha_summoned.begin(); guid != sha_summoned.end(); ++guid)
                {
                    if (auto creature = instance->GetCreature(*guid))
                    {
                        creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        creature->GetAI()->DoAction(TYPE_SET_SUNS_SELECTABLE);
                    }
                }
                break;
            case TYPE_LOREWALKTER_STONESTEP:
                eventStatus_lorewalkter_stonestep = data;
                break;
            case TYPE_NUMBER_SUN_DEFEATED:
                eventStatus_numberSunDefeated += data;

                if (eventStatus_numberSunDefeated == 5)
                {
                    eventStatus_lorewalkter_stonestep = STATUS_LOREWALKER_STONESTEP_ZAO_COMBAT;

                    if (auto zao = instance->GetCreature(zao_sunseeker))
                    {
                        for (GuidList::const_iterator guid = sha_summoned.begin(); guid != sha_summoned.end(); ++guid)
                        {
                            if (auto creature = instance->GetCreature(*guid))
                            {
                                creature->Respawn(true);
                                creature->GetAI()->DoAction(0);
                                creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                creature->SetReactState(REACT_PASSIVE);
                            }
                        }

                        for (GuidList::const_iterator guid = sunfires.begin(); guid != sunfires.end(); ++guid)
                            if (auto creature = instance->GetCreature(*guid))
                                creature->RemoveAura(67422);
                    }
                }
                break;
            }
        }
        void OnCreatureCreate_lorewalker_stonestep(Creature* creature)
        {
            switch (creature->GetEntry())
            {
            case CREATURE_STRIFE:
                guidStrife = creature->GetGUID();
                break;
            case CREATURE_PERIL:
                guidPeril = creature->GetGUID();
                break;
            case CREATURE_HAUNTING_SHA_2:
                sha_summoned.push_back(creature->GetGUID());
                break;
            case CREATURE_SUN:
                creature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                suns.push_back(creature->GetGUID());
                break;
            case CREATURE_ZAO_SUNSEEKER:
                zao_sunseeker = creature->GetGUID();
                break;
            case CREATURE_LOREWALKTER_STONESTEP:
                lorewalkter_stonestep = creature->GetGUID();
                break;
            case CREATURE_SCROLL:
                scroll = creature->GetGUID();
                creature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                creature->CastSpell(creature, SPELL_SCROLL_FLOOR, false);
                creature->CastSpell(creature, SPELL_JADE_ENERGY_2, false);
                creature->CastSpell(creature, SPELL_GROW_LOW, false);
                break;
                //Some creature that need an aura.
            case 59149:
            case 56882:
            case 56871:
            case 56872:
            case 56873:
            case 56874:
            case 59545:
            case 59552:
            case 59544:
                creature->CastSpell(creature, SPELL_SHA_CORRUPTION, false);
                creatures_corrupted.push_back(creature->GetGUID());
                break;
            case 59555: //Haunting Sha
                creature->CastSpell(creature, SPELL_SHA_CORRUPTION, false);
                creature->CastSpell(creature, SPELL_HAUNTING_GAZE, false);
                creatures_corrupted.push_back(creature->GetGUID());
                break;
            case 59553: //Gru
                creature->CastSpell(creature, SPELL_SHA_CORRUPTION, false);
                creature->CastSpell(creature, SPELL_SINGING_SONGBIRD, false);
                creatures_corrupted.push_back(creature->GetGUID());
                break;
            case 59546: //Poisson
                creature->CastSpell(creature, SPELL_SHA_CORRUPTION, false);
                creature->CastSpell(creature, SPELL_WATER_BUBBLE, false);
                creatures_corrupted.push_back(creature->GetGUID());
                break;
            case 59547: //Jiang
                creature->CastSpell(creature, SPELL_SHA_CORRUPTION_3, false);
                creature->CastSpell(creature, SPELL_JUGGLER_JIANG, false);
                creatures_corrupted.push_back(creature->GetGUID());
                break;
            case 65317: //Xiang
                creature->CastSpell(creature, SPELL_SHA_CORRUPTION_3, false);
                creature->CastSpell(creature, SPELL_JUGGLER_XIANG, false);
                creatures_corrupted.push_back(creature->GetGUID());
                break;
            case 58815: //tornado
                creature->SetDisplayId(11686);
                creature->SetReactState(REACT_PASSIVE);
                sunfires.push_back(creature->GetGUID());
                break;
            default:
                if (creature->GetAreaId() == 6118)
                    mobs_liu.push_back(creature->GetGUID());
                break;
            }
        }

        void OnUnitDeath_lorewalker_stonestep(Unit* unit)
        {
            if (unit->ToCreature() && unit->ToCreature()->GetEntry() == CREATURE_STRIFE)
            {
                if (auto peril = instance->GetCreature(guidPeril))
                {
                    if (peril->isDead())
                    {
                        if (auto go = instance->GetGameObject(door_lorewalker))
                            go->SetGoState(GO_STATE_ACTIVE);

                        eventStatus_lorewalkter_stonestep = STATUS_LOREWALKER_STONESTEP_FINISH;
                    }
                }
            }
            
            if (unit->ToCreature() && unit->ToCreature()->GetEntry() == CREATURE_PERIL)
            {
                if (auto strife = instance->GetCreature(guidStrife))
                {
                    if (strife->isDead())
                    {
                        if (auto go = instance->GetGameObject(door_lorewalker))
                            go->SetGoState(GO_STATE_ACTIVE);

                        eventStatus_lorewalkter_stonestep = STATUS_LOREWALKER_STONESTEP_FINISH;
                    }
                }
            }
            
            if (unit->ToCreature() && unit->ToCreature()->GetEntry() == CREATURE_ZAO_SUNSEEKER)
            {
                if (auto go = instance->GetGameObject(door_lorewalker))
                    go->SetGoState(GO_STATE_ACTIVE);

                eventStatus_lorewalkter_stonestep = STATUS_LOREWALKER_STONESTEP_FINISH;
            }
            
            if (unit->ToCreature() && unit->ToCreature()->GetEntry() == CREATURE_SUN)
                unit->CastSpell(unit, SPELL_EXTRACT_SHA_4, false);
            
            if (unit->GetGUID() == scroll)
            {
                unit->RemoveAura(SPELL_JADE_ENERGY_2);
                unit->RemoveAura(SPELL_SCROLL_FLOOR);
                unit->RemoveAura(SPELL_GROW_LOW);
                unit->CastSpell(unit, SPELL_SHA_BURNING, false);
                unit->CastSpell(unit, SPELL_SHA_EXPLOSION, false);
                unit->CastSpell(unit, SPELL_DEATH, false);

                eventChoosen = RAND(1, 2);

                if (eventChoosen == 1)
                {
                    if (auto lorewalker = instance->GetCreature(lorewalkter_stonestep))
                        lorewalker->GetAI()->DoAction(ACTION_2);
                    
                    eventStatus_lorewalkter_stonestep = 2;

                    for (GuidList::const_iterator guid = creatures_corrupted.begin(); guid != creatures_corrupted.end(); ++guid)
                    {
                        if (*guid == lorewalkter_stonestep)
                            continue;

                        if (auto c = instance->GetCreature(*guid))
                        {
                            unit->AddAura(SPELL_DRAW_SHA_2, c);
                            unit->AddAura(SPELL_DRAW_SHA_2, c);
                            c->CastSpell(unit, SPELL_DRAW_SHA_3, false);
                            c->SetUInt32Value(UNIT_FIELD_CHANNEL_SPELL, 42808);
                            c->ForcedDespawn(2000);
                        }
                    }

                    for (GuidList::const_iterator guid = sunfires.begin(); guid != sunfires.end(); ++guid)
                        if (auto c = instance->GetCreature(*guid))
                            c->CastSpell(c, 67422, false);
                }
                else if (eventChoosen == 2)
                    if (auto lorewalker = instance->GetCreature(lorewalkter_stonestep))
                        lorewalker->GetAI()->DoAction(ACTION_1);
            }
        }
        
        void Wipe_lorewalker_stonestep()
        {
            if (auto creature = instance->GetCreature(lorewalkter_stonestep))
            {
                creature->Respawn();
                creature->GetAI()->Reset();
            }

            if (auto creature = instance->GetCreature(guidPeril))
                creature->ForcedDespawn();

            if (auto creature = instance->GetCreature(guidStrife))
                creature->ForcedDespawn();

            if (auto creature = instance->GetCreature(zao_sunseeker))
                creature->ForcedDespawn();

            if (auto creature = instance->GetCreature(scroll))
            {
                creature->Respawn();
                creature->GetAI()->Reset();
                creature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                creature->CastSpell(creature, SPELL_SCROLL_FLOOR, false);
                creature->CastSpell(creature, SPELL_JADE_ENERGY_2, false);
                creature->CastSpell(creature, SPELL_GROW_LOW, false);
            }

            eventStatus_lorewalkter_stonestep = STATUS_LOREWALKER_STONESTEP_NONE;
            eventStatus_numberSunDefeated = 0;

            for (GuidList::const_iterator guid = creatures_corrupted.begin(); guid != creatures_corrupted.end(); ++guid)
            {
                if (auto creature = instance->GetCreature(*guid))
                {
                    creature->Respawn();
                    creature->GetAI()->Reset();
                }
            }

            for (GuidList::const_iterator guid = suns.begin(); guid != suns.end(); ++guid)
                if (auto creature = instance->GetCreature(*guid))
                    creature->ForcedDespawn();

            suns.clear();

            for (GuidList::const_iterator guid = sha_summoned.begin(); guid != sha_summoned.end(); ++guid)
                if (auto creature = instance->GetCreature(*guid))
                    creature->ForcedDespawn();

            sha_summoned.clear();

            for (GuidList::const_iterator guid = sunfires.begin(); guid != sunfires.end(); ++guid)
                if (auto c = instance->GetCreature(*guid))
                    c->RemoveAura(67422);
        }
    };
};

void AddSC_instance_temple_of_jade_serpent()
{
    new instance_temple_of_jade_serpent();
}
