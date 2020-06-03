#include "dragon_soul.h"
#include "Group.h"
#include "LFGMgr.h"
#include "LFGQueue.h"
#include "Creature.h"

#define MAX_ENCOUNTER 8

const uint32 twilightAssaultStalkerDbGuidsH[7][8] = {
    {344491,344492,344493,344490,344494,0},
    {344532,344533,344534,344531,344535,0},
    {344537,344538,344539,344536,344540,0},
    {344529,344468,344526,344527,344524,344528,344530,0},
    {344423,344419,344420,344421,344418,344422,344424,0},
    {344547,344542,344543,344544,344541,344545,344546,0},
    {344501,344496,344497,344498,344495,344499,344500,0},
};

const uint32 twilightAssaultStalkerDbGuidsV[5][10] = {
    {344473,344472,344471,344467,344525,344470,344469,344466,344474,0},
    {344482,344481,344480,344476,344477,344479,344478,344475,344483,0},
    {344433,344432,344431,344427,344428,344430,344429,344426,344434,0},
    {344464,344463,344462,344458,344459,344461,344460,344457,344465,0},
    {344455,344454,344453,344449,344450,344452,344451,344448,344456,0},
};

class instance_dragon_soul : public InstanceMapScript
{
    public:
        instance_dragon_soul() : InstanceMapScript("instance_dragon_soul", 967) { }

        InstanceScript* GetInstanceScript(InstanceMap* map) const
        {
            return new instance_dragon_soul_InstanceMapScript(map);
        }

        struct instance_dragon_soul_InstanceMapScript : public InstanceScript, public instance_dragon_soul_trash_accessor
        {
            instance_dragon_soul_InstanceMapScript(Map* map) : InstanceScript(map)
            {
                SetBossNumber(MAX_ENCOUNTER);

                uiMorchokGUID.Clear();
                uiKohcromGUID.Clear();
                uiYorsahjGUID.Clear();
                uiZonozzGUID.Clear();
                uiHagaraGUID.Clear();
                uiUltraxionGUID.Clear();
                uiBlackhornGUID.Clear();
                uiAllianceShipGUID.Clear();
                uiAllianceShipFirstGUID.Clear();
                uiHordeShipGUID.Clear();
                uiSwayzeGUID.Clear();
                // teleports
                uiWyrmrestBaseFromSummitGUID.Clear();
                uiWyrmrestBaseFromGunshipGUID.Clear();
                uiWyrmrestBaseFromMaelstormGUID.Clear();
                uiWyrmrestSummitGUID.Clear();
                uiEyeofEternityGUID.Clear();
                uiDeckGUID.Clear();
                uiMaelstormGUID.Clear();
                uiDeathwingGUID.Clear();
                uiAlexstraszaDragonGUID.Clear();
                uiNozdormuDragonGUID.Clear();
                uiYseraDragonGUID.Clear();
                uiKalecgosDragonGUID.Clear();
                uiThrall2GUID.Clear();

                //memset(uiLesserCacheofTheAspects, 0, sizeof(uiLesserCacheofTheAspects));
                //memset(uiBackPlates, 0, sizeof(uiBackPlates));
                //memset(uiGreaterCacheofTheAspects, 0, sizeof(uiGreaterCacheofTheAspects));
                //memset(uiElementiumFragment, 0, sizeof(uiGreaterCacheofTheAspects));

                //memset(twilightAssaultStalkerGuidsH, 0, sizeof(twilightAssaultStalkerGuidsH));
                //memset(twilightAssaultStalkerGuidsV, 0, sizeof(twilightAssaultStalkerGuidsV));
                memset(twilightAssaultLanesUsedH, 0, sizeof(twilightAssaultLanesUsedH));
                memset(twilightAssaultLanesUsedV, 0, sizeof(twilightAssaultLanesUsedV));

                uiNethestraszGUID.Clear();
                uiOpenPortalEvent = 0;
                bHagaraEvent = 0;
                uiThrallEvent.Clear();
                uiDragonSoulEvent = 0;
                uiUltraxionTrash = 0;
                uiDragonsCount = 0;
                uiDeathwingEvent.Clear();
                uiDelayedChestData = 0;

                lfrSectionFound = false;
                isFallOfDeathwing = false;
            }

            void OnPlayerEnter(Player* pPlayer)
            {
                if (!uiTeamInInstance)
                    uiTeamInInstance = pPlayer->GetTeam();
                if (isLfr && !lfrSectionFound)
                {
                    lfrSectionFound = true;
                    Group* group = pPlayer->GetGroup();
                    if (!group)
                        return;
                    if (group->isLFGGroup())
                        isFallOfDeathwing = sLFGMgr->GetDungeon(group->GetGUID()) == 417;

                    if (isFallOfDeathwing)
                    {
                        if (Creature* boss = instance->GetCreature(uiMorchokGUID))
                            boss->DespawnOrUnsummon();
                        if (Creature* boss = instance->GetCreature(uiZonozzGUID))
                            boss->DespawnOrUnsummon();
                        if (Creature* boss = instance->GetCreature(uiYorsahjGUID))
                            boss->DespawnOrUnsummon();
                        if (Creature* boss = instance->GetCreature(uiHagaraGUID))
                            boss->DespawnOrUnsummon();
                        SetBossState(DATA_MORCHOK, DONE);
                        SetBossState(DATA_ZONOZZ, DONE);
                        SetBossState(DATA_YORSAHJ, DONE);
                        SetBossState(DATA_HAGARA, DONE);
                        uiOpenPortalEvent = DONE;
                    }
                }
                if (GetBossState(DATA_MADNESS) == DONE && pPlayer->GetCurrentAreaID() == 5893)
                    pPlayer->AddAura(SPELL_CALM_MAELSTROM_SKYBOX, pPlayer);
            }

            void OnMovieEnded(Player* player)
            {
                if (!player->isDead() && !player->HasAura(SPELL_PARACHUTE) && (GetBossState(DATA_SPINE) == IN_PROGRESS || GetBossState(DATA_SPINE) == DONE) && (GetBossState(DATA_MADNESS) != DONE))
                    player->CastSpell(player, SPELL_PARACHUTE, true);
            }

            void OnCreatureCreate(Creature* pCreature)
            {
                if (isLfr && !lfrSectionFound)
                {
                    Map::PlayerList const &players = instance->GetPlayers();
                    if (!players.isEmpty())
                        if (Player* player = players.begin()->getSource())
                            OnPlayerEnter(player);
                }

                if (isLfr)
                {
                    uint32 entry = pCreature->GetEntry();
                    if (isFallOfDeathwing)
                    {
                        if (entry == NPC_MORCHOK || entry == NPC_ZONOZZ || entry == NPC_YORSAHJ || entry == NPC_HAGARA)
                        {
                            pCreature->DespawnOrUnsummon();
                            return;
                        }
                    }
                    else
                    {
                        if (entry == NPC_ULTRAXION || entry == NPC_BLACKHORN || entry == NPC_SKY_CAPTAIN_SWAYZE || entry == NPC_KAANU_REEVS)
                        {
                            pCreature->DespawnOrUnsummon();
                            return;
                        }
                    }
                }

                switch (pCreature->GetEntry())
                {
                    case NPC_MORCHOK:
                        uiMorchokGUID = pCreature->GetGUID();
                        break;
                    case NPC_KOHCROM:
                        uiKohcromGUID = pCreature->GetGUID();
                        break;
                    case NPC_YORSAHJ:
                        uiYorsahjGUID = pCreature->GetGUID();
                        break;
                    case NPC_ZONOZZ:
                        uiZonozzGUID = pCreature->GetGUID();
                        break;
                    case NPC_HAGARA:
                        uiHagaraGUID = pCreature->GetGUID();
                        break;
                    case NPC_ULTRAXION:
                        uiUltraxionGUID = pCreature->GetGUID();
                        break;
                    case NPC_BLACKHORN:
                        uiBlackhornGUID = pCreature->GetGUID();
                        break;
                    case NPC_SKY_CAPTAIN_SWAYZE:
                        if (pCreature->GetPositionZ() > 200.0f)
                            uiSwayzeGUID = pCreature->GetGUID();
                        if (GetBossState(DATA_BLACKHORN) == DONE)
                            pCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case NPC_TWILIGHT_ASSAULTER_1:
                    case NPC_TWILIGHT_ASSAULTER_2:
                    case NPC_TWILIGHT_ASSAULTER_3:
                    case NPC_TWILIGHT_ASSAULTER_4:
                        assaultersGUIDs.push_back(pCreature->GetGUID());
                        if (pCreature->isDead())
                            pCreature->Respawn();
                        break;
                    case NPC_DEATHWING_EVENT:
                        uiDeathwingEvent = pCreature->GetGUID();
                        if (GetData(DATA_ULTRAXION_TRASH) == DONE)
                            pCreature->DespawnOrUnsummon();
                        break;
                    case NPC_THRALL_1:
                        uiThrallEvent = pCreature->GetGUID();
                        pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        if (GetBossState(DATA_ULTRAXION) == DONE)
                        {
                            pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            pCreature->SummonCreature(NPC_SKY_CAPTAIN_SWAYZE, customPos[1], TEMPSUMMON_MANUAL_DESPAWN, 0);
                            pCreature->SummonCreature(NPC_KAANU_REEVS, customPos[2], TEMPSUMMON_MANUAL_DESPAWN, 0);
                        }
                        else if (GetBossState(DATA_HAGARA) == DONE)
                            pCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        break;
                    case NPC_TWILIGHT_ASSAULTER_STALKER:
                    {
                        for (uint8 row = 0; row < 7; ++row)
                        {
                            for (uint8 col = 0; col < 8; ++col)
                            {
                                if (!twilightAssaultStalkerDbGuidsH[row][col])
                                    break;
                                if (twilightAssaultStalkerDbGuidsH[row][col] == pCreature->GetDBTableGUIDLow())
                                {
                                    twilightAssaultStalkerGuidsH[row][col] = pCreature->GetGUID();
                                    return;
                                }
                            }
                        }
                        for (uint8 col = 0; col < 5; ++col)
                        {
                            for (uint8 row = 0; row < 10; ++row)
                            {
                                if (!twilightAssaultStalkerDbGuidsV[col][row])
                                    break;
                                if (twilightAssaultStalkerDbGuidsV[col][row] == pCreature->GetDBTableGUIDLow())
                                {
                                    twilightAssaultStalkerGuidsV[col][row] = pCreature->GetGUID();
                                    return;
                                }
                            }
                        }
                        TC_LOG_ERROR(LOG_FILTER_TSCR, "instance_dragon_soul: NPC_TWILIGHT_ASSAULTER_STALKER of unknown DB GUID was spawned: %u", pCreature->GetDBTableGUIDLow());
                        break;
                    }
                    case NPC_EIENDORMI:
                    case NPC_VALEERA:
                        dragonstaxiGUIDs.push_back(pCreature->GetGUID());
                        pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
                        if (GetBossState(DATA_MORCHOK) == DONE)
                            pCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
                        break;
                    case NPC_NETHESTRASZ:
                        uiNethestraszGUID = pCreature->GetGUID();
                        pCreature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
                        if ((GetBossState(DATA_YORSAHJ) == DONE) && (GetBossState(DATA_ZONOZZ) == DONE))
                            pCreature->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
                        break;
                    case NPC_THRALL_2:
                        uiThrall2GUID = pCreature->GetGUID();
                        break;
                    // teleports hardcoding
                    case NPC_TRAVEL_TO_WYRMREST_BASE:
                        if (pCreature->GetPositionZ() < 50)
                        {
                            uiWyrmrestBaseFromMaelstormGUID = pCreature->GetGUID();
                            maelstormteleGUIDs.push_back(pCreature->GetGUID());
                            SetPortalState(pCreature, GetBossState(DATA_SPINE) == DONE);
                        }
                        else if (pCreature->GetPositionZ() < 100)
                        {
                            startportalsGUIDs.push_back(pCreature->GetGUID());
                            SetPortalState(pCreature, GetBossState(DATA_MORCHOK) == DONE);
                        }
                        else if (pCreature->GetPositionZ() < 200)
                        {
                            uiWyrmrestBaseFromGunshipGUID = pCreature->GetGUID();
                            SetPortalState(pCreature, GetBossState(DATA_ULTRAXION) == DONE);
                        }
                        else
                        {
                            uiWyrmrestBaseFromSummitGUID = pCreature->GetGUID();
                            SetPortalState(pCreature, (GetBossState(DATA_YORSAHJ) == DONE && GetBossState(DATA_ZONOZZ) == DONE));
                        }
                        teleportGUIDs.push_back(pCreature->GetGUID());
                        break;
                    case NPC_DEATHWING:
                        uiDeathwingGUID = pCreature->GetGUID();
                        break;
                    case NPC_ALEXSTRASZA_DRAGON:
                        uiAlexstraszaDragonGUID = pCreature->GetGUID();
                        break;
                    case NPC_NOZDORMU_DRAGON:
                        uiNozdormuDragonGUID = pCreature->GetGUID();
                        break;
                    case NPC_YSERA_DRAGON:
                        uiYseraDragonGUID = pCreature->GetGUID();
                        break;
                    case NPC_KALECGOS_DRAGON:
                        uiKalecgosDragonGUID = pCreature->GetGUID();
                        break;
                    case NPC_TRAVEL_TO_WYRMREST_TEMPLE:
                        SetPortalState(pCreature, GetBossState(DATA_MORCHOK) == DONE);
                        startportalsGUIDs.push_back(pCreature->GetGUID());
                        teleportGUIDs.push_back(pCreature->GetGUID());
                        break;
                    case NPC_TRAVEL_TO_WYRMREST_SUMMIT:
                        uiWyrmrestSummitGUID = pCreature->GetGUID();
                        wyrmrestsummitGUIDs.push_back(pCreature->GetGUID());
                        teleportGUIDs.push_back(pCreature->GetGUID());
                        SetPortalState(pCreature, (GetBossState(DATA_YORSAHJ) == DONE && GetBossState(DATA_ZONOZZ) == DONE));
                        break;
                    case NPC_TRAVEL_TO_EYE_OF_ETERNITY:
                        uiEyeofEternityGUID = pCreature->GetGUID();
                        wyrmrestsummitGUIDs.push_back(pCreature->GetGUID());
                        teleportGUIDs.push_back(pCreature->GetGUID());
                        SetPortalState(pCreature, GetData(DATA_OPEN_PORTAL_TO_EYE) == DONE);
                        pCreature->SetVisible(GetData(DATA_OPEN_PORTAL_TO_EYE) == DONE);
                        break;
                    case NPC_TRAVEL_TO_DECK:
                        uiDeckGUID = pCreature->GetGUID();
                        maelstormteleGUIDs.push_back(pCreature->GetGUID());
                        teleportGUIDs.push_back(pCreature->GetGUID());
                        pCreature->SetVisible(GetBossState(DATA_SPINE) == DONE);
                        SetPortalState(pCreature, GetBossState(DATA_SPINE) == DONE);
                        break;
                    case NPC_TRAVEL_TO_MAELSTORM:
                        uiMaelstormGUID = pCreature->GetGUID();
                        maelstormteleGUIDs.push_back(pCreature->GetGUID());
                        teleportGUIDs.push_back(pCreature->GetGUID());
                        pCreature->SetVisible(GetBossState(DATA_SPINE) == DONE);
                        SetPortalState(pCreature, GetBossState(DATA_SPINE) == DONE);
                        break;
                    default:
                        break;
                }
            }

            void OnCreatureRemove(Creature* pCreature)
            {
                switch (pCreature->GetEntry())
                {
                    case NPC_ULTRAXION:
                        uiUltraxionGUID.Clear();
                        break;
                    case NPC_BLACKHORN:
                        uiBlackhornGUID.Clear();
                        break;
                    case NPC_DEATHWING:
                        uiDeathwingGUID.Clear();
                        break;
                    case NPC_ALEXSTRASZA_DRAGON:
                        uiAlexstraszaDragonGUID.Clear();
                        break;
                    case NPC_NOZDORMU_DRAGON:
                        uiNozdormuDragonGUID.Clear();
                        break;
                    case NPC_YSERA_DRAGON:
                        uiYseraDragonGUID.Clear();
                        break;
                    case NPC_KALECGOS_DRAGON:
                        uiKalecgosDragonGUID.Clear();
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* pGo)
            {
                if (isLfr && !lfrSectionFound)
                {
                    Map::PlayerList const &players = instance->GetPlayers();
                    if (!players.isEmpty())
                        if (Player* player = players.begin()->getSource())
                            OnPlayerEnter(player);
                }

                switch (pGo->GetEntry())
                {
                    case GO_LESSER_CACHE_OF_THE_ASPECTS_LFR:
                        if (!isLfr)
                        {
                            pGo->SetPhaseMask(256, true);
                            return;
                        }
                        uiLesserCacheofTheAspects[1] = pGo->GetGUID();
                        break;
                    case GO_LESSER_CACHE_OF_THE_ASPECTS_10N:
                        uiLesserCacheofTheAspects[0] = pGo->GetGUID();
                        break;
                    case GO_LESSER_CACHE_OF_THE_ASPECTS_25N:
                        if (isLfr)
                        {
                            pGo->SetPhaseMask(256, true);
                            return;
                        }
                        uiLesserCacheofTheAspects[1] = pGo->GetGUID();
                        break;
                    case GO_LESSER_CACHE_OF_THE_ASPECTS_10H:
                        uiLesserCacheofTheAspects[2] = pGo->GetGUID();
                        break;
                    case GO_LESSER_CACHE_OF_THE_ASPECTS_25H:
                        uiLesserCacheofTheAspects[3] = pGo->GetGUID();
                        break;
                    case GO_ALLIANCE_SHIP:
                        uiAllianceShipGUID = pGo->GetGUID();
                        if (GetBossState(DATA_ULTRAXION) == DONE)
                        {
                            pGo->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                            pGo->UpdateObjectVisibility();
                        }
                        break;
                    case GO_ALLIANCE_SHIP_FIRST:
                        uiAllianceShipFirstGUID = pGo->GetGUID();
                        if (GetBossState(DATA_ULTRAXION) == DONE)
                        {
                            pGo->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                            pGo->UpdateObjectVisibility();
                        }
                        break;
                    case GO_HORDE_SHIP:
                        uiHordeShipGUID = pGo->GetGUID();
                        if (GetBossState(DATA_ULTRAXION) == DONE)
                        {
                            pGo->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
                            pGo->UpdateObjectVisibility();
                        }
                        break;
                    case GO_DEATHWING_BACK_PLATE_1:
                        uiBackPlates[0] = pGo->GetGUID();
                        break;
                    case GO_DEATHWING_BACK_PLATE_2:
                        uiBackPlates[1] = pGo->GetGUID();
                        break;
                    case GO_DEATHWING_BACK_PLATE_3:
                        uiBackPlates[2] = pGo->GetGUID();
                        break;
                    case GO_GREATER_CACHE_OF_THE_ASPECTS_LFR:
                        if (!isLfr)
                        {
                            pGo->SetPhaseMask(256, true);
                            return;
                        }
                        uiGreaterCacheofTheAspects[1] = pGo->GetGUID();
                        break;
                    case GO_GREATER_CACHE_OF_THE_ASPECTS_10N:
                        uiGreaterCacheofTheAspects[0] = pGo->GetGUID();
                        break;
                    case GO_GREATER_CACHE_OF_THE_ASPECTS_25N:
                        if (isLfr)
                        {
                            pGo->SetPhaseMask(256, true);
                            return;
                        }
                        uiGreaterCacheofTheAspects[1] = pGo->GetGUID();
                        break;
                    case GO_GREATER_CACHE_OF_THE_ASPECTS_10H:
                        uiGreaterCacheofTheAspects[2] = pGo->GetGUID();
                        break;
                    case GO_GREATER_CACHE_OF_THE_ASPECTS_25H:
                        uiGreaterCacheofTheAspects[3] = pGo->GetGUID();
                        break;
                    case GO_ELEMENTIUM_FRAGMENT_LFR:
                        if (!isLfr)
                        {
                            pGo->SetPhaseMask(256, true);
                            return;
                        }
                        uiElementiumFragment[1] = pGo->GetGUID();
                        break;
                    case GO_ELEMENTIUM_FRAGMENT_10N:
                        uiElementiumFragment[0] = pGo->GetGUID();
                        break;
                    case GO_ELEMENTIUM_FRAGMENT_25N:
                        if (isLfr)
                        {
                            pGo->SetPhaseMask(256, true);
                            return;
                        }
                        uiElementiumFragment[1] = pGo->GetGUID();
                        break;
                    case GO_ELEMENTIUM_FRAGMENT_10H:
                        uiElementiumFragment[2] = pGo->GetGUID();
                        break;
                    case GO_ELEMENTIUM_FRAGMENT_25H:
                        uiElementiumFragment[3] = pGo->GetGUID();
                        break;
                    default:
                        break;
                }
            }

            void OnUnitDeath(Unit* unit)
            {
                if (Player* player = unit->ToPlayer())
                {
                    if (IsEncounterInProgress() && (player->GetCurrentAreaID() == 5960 || player->GetCurrentAreaID() == 5893))
                    {
                        player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 94644, 0, 0, player, player);
                        player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, 94655, 0, 0, player, player);
                    }
                }
            }

            ObjectGuid GetGuidData(uint32 type) const
            {
                switch (type)
                {
                    case DATA_MORCHOK: return uiMorchokGUID;
                    case DATA_KOHCROM: return uiKohcromGUID;
                    case DATA_YORSAHJ: return uiYorsahjGUID;
                    case DATA_ZONOZZ: return uiZonozzGUID;
                    case DATA_HAGARA: return uiHagaraGUID;
                    case DATA_ULTRAXION: return uiUltraxionGUID;
                    case DATA_BLACKHORN: return uiBlackhornGUID;
                    case DATA_LESSER_CACHE_10N: return uiLesserCacheofTheAspects[0];
                    case DATA_LESSER_CACHE_25N: return uiLesserCacheofTheAspects[1];
                    case DATA_LESSER_CACHE_10H: return uiLesserCacheofTheAspects[2];
                    case DATA_LESSER_CACHE_25H: return uiLesserCacheofTheAspects[3];
                    case DATA_SWAYZE: return uiSwayzeGUID;
                    case DATA_DEATHWING: return uiDeathwingGUID;
                    case DATA_ALEXSTRASZA_DRAGON: return uiAlexstraszaDragonGUID;
                    case DATA_NOZDORMU_DRAGON: return uiNozdormuDragonGUID;
                    case DATA_YSERA_DRAGON: return uiYseraDragonGUID;
                    case DATA_KALECGOS_DRAGON: return uiKalecgosDragonGUID;
                    case DATA_ALLIANCE_SHIP: return uiAllianceShipGUID;
                    case DATA_ALLIANCE_SHIP_FIRST: return uiAllianceShipFirstGUID;
                    case DATA_HORDE_SHIP: return uiHordeShipGUID;
                    case DATA_BACK_PLATE_1: return uiBackPlates[0];
                    case DATA_BACK_PLATE_2: return uiBackPlates[1];
                    case DATA_BACK_PLATE_3: return uiBackPlates[2];
                    case DATA_GREATER_CACHE_10N: return uiGreaterCacheofTheAspects[0];
                    case DATA_GREATER_CACHE_25N: return uiGreaterCacheofTheAspects[1];
                    case DATA_GREATER_CACHE_10H: return uiGreaterCacheofTheAspects[2];
                    case DATA_GREATER_CACHE_25H: return uiGreaterCacheofTheAspects[3];
                    case DATA_ELEM_FRAGMENT_10N: return uiElementiumFragment[0];
                    case DATA_ELEM_FRAGMENT_25N: return uiElementiumFragment[1]; 
                    case DATA_ELEM_FRAGMENT_10H: return uiElementiumFragment[2];
                    case DATA_ELEM_FRAGMENT_25H: return uiElementiumFragment[3];
                    case DATA_DRAGON_SOUL_EVENT: return uiDeathwingEvent;
                    case DATA_THRALL_MADNESS: return uiThrall2GUID;
                }
                return ObjectGuid::Empty;
            }

            void SetData(uint32 type, uint32 data)
            {   
                switch (type)
                {
                    case DATA_OPEN_PORTAL_TO_EYE:
                        uiOpenPortalEvent = data;
                        if (uiOpenPortalEvent == DONE)
                            if (Creature* EyeofEternityTele = instance->GetCreature(uiEyeofEternityGUID))
                            {
                                ActivatePortal(EyeofEternityTele);
                                SaveToDB();
                            }
                        break;
                    case DATA_HAGARA_EVENT:
                        bHagaraEvent = data;
                        switch (data)
                        {
                            case IN_PROGRESS:
                                if (Creature* WyrmrestSummitTele = instance->GetCreature(uiWyrmrestSummitGUID))
                                    DeactivatePortal(WyrmrestSummitTele);
                                break;
                            case FAIL:
                                UpdatePortals();
                                break;
                            case DONE:
                                UpdatePortals();
                                SaveToDB();
                                break;
                        }
                        break;
                    case DATA_DRAGON_SOUL_EVENT:
                        uiDragonSoulEvent = data;
                        if (uiDragonSoulEvent == DONE)
                            SaveToDB();
                        break;
                    case DATA_ULTRAXION_TRASH:
                        uiUltraxionTrash = data;
                        switch (data)
                        {
                        case IN_PROGRESS:
                            for (GuidVector::const_iterator itr = assaultersGUIDs.begin(); itr != assaultersGUIDs.end(); ++itr)
                                if (Creature* assaulter = instance->GetCreature(*itr))
                                {
                                    assaulter->SetPhaseMask(1, true);
                                    assaulter->CastSpell(assaulter, SPELL_TWILIGHT_SPAWN);
                                }
                            break;
                        case SPECIAL:
                            for (GuidVector::const_iterator itr = assaultersGUIDs.begin(); itr != assaultersGUIDs.end(); ++itr)
                                if (Creature* assaulter = instance->GetCreature(*itr))
                                    assaulter->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                            if (!teleportGUIDs.empty())
                                for (GuidVector::const_iterator itr = teleportGUIDs.begin(); itr != teleportGUIDs.end(); ++itr)
                                    if (Creature* pTeleports = instance->GetCreature((*itr)))
                                        DeactivatePortal(pTeleports);
                            break;
                        case FAIL:
                            for (GuidVector::const_iterator itr = assaultersGUIDs.begin(); itr != assaultersGUIDs.end(); ++itr)
                                if (Creature* assaulter = instance->GetCreature(*itr))
                                {
                                    assaulter->SetCanFly(true);
                                    assaulter->SetDisableGravity(true);
                                    assaulter->GetMotionMaster()->Clear(true);
                                    assaulter->Respawn(true);
                                }
                            if (Creature* deathwing = instance->GetCreature(uiDeathwingEvent))
                                deathwing->AI()->DoAction(ACTION_DEATHWING_RESET);
                            UpdatePortals();
                            break;
                        case DONE:
                            SaveToDB();
                            UpdatePortals();
                            break;
                        }
                        break;
                    case DATA_DRAGONS_COUNT:
                        if (++uiDragonsCount == 15)
                        {
                            if (Creature* thrall = instance->GetCreature(uiThrallEvent))
                                thrall->AI()->DoAction(ACTION_STOP_ASSAULTERS_SPAWN);
                            if (Creature* Deathwing = instance->GetCreature(GetGuidData(DATA_DRAGON_SOUL_EVENT)))
                                Deathwing->AI()->DoAction(ACTION_DEATHWING_INTRO);
                            for (GuidVector::const_iterator itr = assaultersGUIDs.begin(); itr != assaultersGUIDs.end(); ++itr)
                                if (Creature* assaulters = instance->GetCreature(*itr))
                                {
                                    assaulters->AI()->DoAction(ACTION_STOP_ASSAULT);
                                    assaulters->SetPhaseMask(256, true);
                                    assaulters->CastSpell(assaulters, SPELL_TWILIGHT_ESCAPE);
                                }
                            SetData(DATA_ULTRAXION_TRASH, DONE);
                        }
                        break;
                    case DATA_NEXT_ASSAULTER:
                    {
                        GuidList guids(assaultersGUIDs.begin(), assaultersGUIDs.end());
                        AssaulterCheck check(this);
                        Trinity::Containers::RandomResizeList(guids, check, 1);
                        if (!guids.empty())
                            instance->GetCreature(*guids.begin())->AI()->DoAction(ACTION_START_ASSAULT);
                        break;
                    }
                    case DATA_SPAWN_GREATER_CHEST:
                        if (uiDelayedChestData)
                        {
                            if (instance->GetGameObject(GetGuidData(uiDelayedChestData)))
                            {
                                DoRespawnGameObject(GetGuidData(uiDelayedChestData), DAY);
                                uiDelayedChestData = 0;
                            }
                        }
                        else if (instance->GetGameObject(GetGuidData(data)))
                        {
                            DoRespawnGameObject(GetGuidData(data), DAY);
                            uiDelayedChestData = 0;
                        }
                        else
                            uiDelayedChestData = data;
                        break;
                    default:
                        break;
                }
            }
            
            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_OPEN_PORTAL_TO_EYE:
                        return uiOpenPortalEvent;
                    case DATA_HAGARA_EVENT:
                        return bHagaraEvent;
                    case DATA_DRAGON_SOUL_EVENT:
                        return uiDragonSoulEvent;
                    case DATA_ULTRAXION_TRASH:
                        return uiUltraxionTrash;
                    case DATA_NEXT_ASSAULTER:
                        return 20000 - uiDragonsCount * 800;
                    case DATA_IS_LFR:
                        return IsLFR();
                    case DATA_IS_FALL_OF_DEATHWING:
                        return IsFallOfDeathwing();
                    default:
                        break;
                }

                return 0;
            }

            bool SetBossState(uint32 type, EncounterState state)
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;

                if (type == DATA_MORCHOK && state == DONE)
                    if (!dragonstaxiGUIDs.empty())
                        for (GuidVector::const_iterator itr = dragonstaxiGUIDs.begin(); itr != dragonstaxiGUIDs.end(); ++itr)
                            if (Creature* dragonstaxi = instance->GetCreature((*itr)))
                                dragonstaxi->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);

                if ((GetBossState(DATA_YORSAHJ) == DONE) && (GetBossState(DATA_ZONOZZ) == DONE))
                    if (Creature* Nethestrasz = instance->GetCreature(uiNethestraszGUID))
                        Nethestrasz->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);

                if (type == DATA_HAGARA && state == DONE)
                    if (Creature* Thrall = instance->GetCreature(uiThrallEvent))
                        Thrall->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                if (type == DATA_SPINE)
                    if (!maelstormteleGUIDs.empty())
                        for (GuidVector::const_iterator itr = maelstormteleGUIDs.begin(); itr != maelstormteleGUIDs.end(); ++itr)
                            if (Creature* Teleports = instance->GetCreature((*itr)))
                                Teleports->SetVisible(state == DONE ? true : false);

                if (type == DATA_BLACKHORN && state == DONE)
                    if (Creature* Swayze = instance->GetCreature(uiSwayzeGUID))
                        Swayze->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                // visual state of teleporters
                if (state == IN_PROGRESS)
                {
                    if (!teleportGUIDs.empty())
                        for (GuidVector::const_iterator itr = teleportGUIDs.begin(); itr != teleportGUIDs.end(); ++itr)
                            if (Creature* Teleports = instance->GetCreature((*itr)))
                                DeactivatePortal(Teleports);
                }
                else
                    UpdatePortals();

                return true;
            }
            
            void UpdatePortals()
            {
                if (GetBossState(DATA_MORCHOK) == DONE)
                    if (!startportalsGUIDs.empty())
                        for (GuidVector::const_iterator itr = startportalsGUIDs.begin(); itr != startportalsGUIDs.end(); ++itr)
                            if (Creature* Teleports = instance->GetCreature((*itr)))
                                ActivatePortal(Teleports);
                if ((GetBossState(DATA_YORSAHJ) == DONE) && (GetBossState(DATA_ZONOZZ) == DONE))
                    if (Creature* WyrmrestBaseFromSummitTele = instance->GetCreature(uiWyrmrestBaseFromSummitGUID))
                        ActivatePortal(WyrmrestBaseFromSummitTele);
                if ((GetBossState(DATA_HAGARA) == DONE) || (GetData(DATA_OPEN_PORTAL_TO_EYE) == DONE))
                    if (!wyrmrestsummitGUIDs.empty())
                        for (GuidVector::const_iterator itr = wyrmrestsummitGUIDs.begin(); itr != wyrmrestsummitGUIDs.end(); ++itr)
                            if (Creature* Teleports = instance->GetCreature((*itr)))
                                ActivatePortal(Teleports);
                if (GetBossState(DATA_ULTRAXION) == DONE)
                    if (Creature* WyrmrestBaseFromGunship = instance->GetCreature(uiWyrmrestBaseFromGunshipGUID))
                        ActivatePortal(WyrmrestBaseFromGunship);
                if (GetBossState(DATA_SPINE) == DONE)
                    if (!maelstormteleGUIDs.empty())
                        for (GuidVector::const_iterator itr = maelstormteleGUIDs.begin(); itr != maelstormteleGUIDs.end(); ++itr)
                            if (Creature* Teleports = instance->GetCreature((*itr)))
                                ActivatePortal(Teleports);
            }

            std::string GetSaveData()
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;

                std::ostringstream saveStream;
                saveStream << "D S " << GetBossSaveData() << uiOpenPortalEvent  << " " << bHagaraEvent << " " << uiDragonSoulEvent << " " << uiUltraxionTrash << " ";

                str_data = saveStream.str();

                OUT_SAVE_INST_DATA_COMPLETE;
                return str_data;
            }

            void Load(const char* in)
            {
                if (!in)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(in);

                char dataHead1, dataHead2;

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'D' && dataHead2 == 'S')
                {
                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                            tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }
                    
                    uint32 tmpEvent;
                    loadStream >> tmpEvent;
                    if (tmpEvent != DONE) 
                        tmpEvent = NOT_STARTED;
                    uiOpenPortalEvent = tmpEvent;

                    loadStream >> tmpEvent;
                    if (tmpEvent != DONE) 
                        tmpEvent = NOT_STARTED;
                    bHagaraEvent = tmpEvent;

                    loadStream >> tmpEvent;
                    if (tmpEvent != DONE) 
                        tmpEvent = NOT_STARTED;
                    uiDragonSoulEvent = tmpEvent;

                    loadStream >> tmpEvent;
                    if (tmpEvent != DONE) 
                        tmpEvent = NOT_STARTED;
                    uiUltraxionTrash = tmpEvent;
                } else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

            bool IsLFR() const
            {
                return isLfr;
            }
            bool IsFallOfDeathwing() const
            {
                return isLfr && isFallOfDeathwing;
            }
            
            bool CheckAchievementCriteriaMeet(uint32 criteria_id, Player const* /*source*/, Unit const* /*target*/, uint32 /*miscvalue1*/)
            {
                switch (criteria_id)
                {
                    case CRITERIA_STATS_MORCHOK_KILLS_LFR:
                    case CRITERIA_STATS_ZONOZZ_KILLS_LFR:
                    case CRITERIA_STATS_YORSAHJ_KILLS_LFR:
                    case CRITERIA_STATS_HAGARA_KILLS_LFR:
                    case CRITERIA_STATS_ULTRAXION_KILLS_LFR:
                    case CRITERIA_STATS_WARMASTER_KILLS_LFR:
                    case CRITERIA_STATS_SPINE_KILLS_LFR:
                    case CRITERIA_STATS_DEATHWING_KILLS_LFR:
                        return IsLFR();
                    case CRITARIA_DONT_STAND_SO_CLOSE_TO_ME:
                    case CRITERIA_PING_PONG_CHAMPION:
                    case CRITERIA_TASTE_THE_RAINBOW_BY:
                    case CRITERIA_TASTE_THE_RAINBOW_RG:
                    case CRITERIA_TASTE_THE_RAINBOW_BB:
                    case CRITERIA_TASTE_THE_RAINBOW_PY:
                    case CRITERIA_HOLDING_HANDS:
                    case CRITERIA_MINUTES_TO_MIDNIGHT:
                    case CRITERIA_DECK_DEFENDER:
                    case CRITERIA_MAYBE_HELL_GET_DIZZY:
                    case CRITERIA_ALEXSTRASZA_FIRST:
                    case CRITERIA_KALECGOS_FIRST:
                    case CRITERIA_NOZDORMU_FIRST:
                    case CRITERIA_YSERA_FIRST:
                    case CRITERIA_DESTROYERS_END:
                        return !IsLFR();
                    default:
                        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Achievement system call InstanceScript::CheckAchievementCriteriaMeet but instance script for map %u not have implementation for achievement criteria %u", instance->GetId(), criteria_id);
                        break;
                }

                return false;
            }

            Creature* GetNextTwilightAssaulterStalker(Creature const* current) override
            {
                ObjectGuid currentGuid = current->GetGUID();
                for (uint8 row = 0; row < 7; ++row)
                {
                    for (uint8 col = 0; col < 8; ++col)
                    {
                        if (!twilightAssaultStalkerGuidsH[row][col])
                            break;
                        if (twilightAssaultStalkerGuidsH[row][col] == currentGuid)
                        {
                            //ASSERT(twilightAssaultLanesUsedH[row]);
                            if (!twilightAssaultLanesUsedH[row])
                                return NULL;
                            bool inverseDirection = twilightAssaultLanesUsedH[row] == 2;
                            if (inverseDirection && col == 0)
                                return NULL;
                            ObjectGuid nextGuid = twilightAssaultStalkerGuidsH[row][inverseDirection ? col-1 : col+1];
                            return nextGuid ? instance->GetCreature(nextGuid) : NULL;
                        }
                    }
                }
                for (uint8 col = 0; col < 5; ++col)
                {
                    for (uint8 row = 0; row < 10; ++row)
                    {
                        if (!twilightAssaultStalkerGuidsV[col][row])
                            break;
                        if (twilightAssaultStalkerGuidsV[col][row] == currentGuid)
                        {
                            //ASSERT(twilightAssaultLanesUsedV[col]);
                            if (!twilightAssaultLanesUsedV[col])
                                return NULL;
                            bool inverseDirection = twilightAssaultLanesUsedV[col] == 2;
                            if (inverseDirection && row == 0)
                                return NULL;
                            ObjectGuid nextGuid = twilightAssaultStalkerGuidsV[col][inverseDirection ? row-1 : row+1];
                            return nextGuid ? instance->GetCreature(nextGuid) : NULL;
                        }
                    }
                }
                return NULL;
            }
            Position const* GetRandomTwilightAssaulterAssaultPosition(bool horizonal, bool fromEnd, uint8& lane, ObjectGuid& targetGUID) override
            {
                if (horizonal)
                {
                    // Check if all lanes are used
                    bool allUsed = true;
                    for (uint8 i = 0; i < 7; ++i)
                        if (!twilightAssaultLanesUsedH[i])
                        {
                            allUsed = false;
                            break;
                        }
                    if (allUsed)
                    {
                        lane = 0;
                        return NULL;
                    }

                    // Find random unused lane
                    uint8 row;
                    do { row = urand(0, 6); }
                    while (twilightAssaultLanesUsedH[row]);

                    lane = row;

                    uint8 col = 0;
                    if (fromEnd)
                        while (twilightAssaultStalkerGuidsH[row][col + 1]) { ++col; } // Find the last one

                    Position* assaultPos = new Position();
                    if (Creature* stalker = instance->GetCreature(targetGUID = twilightAssaultStalkerGuidsH[row][col]))
                        stalker->GetPosition(assaultPos);

                    if (fromEnd)
                    {
                        Position offset = { 0.0f, -10.0f, 10.0f };
                        assaultPos->RelocateOffset(offset);
                        assaultPos->SetOrientation(M_PI/2);
                    }
                    else
                    {
                        Position offset = { 0.0f, 10.0f, 10.0f };
                        assaultPos->RelocateOffset(offset);
                        assaultPos->SetOrientation(M_PI*2 - M_PI/2);
                    }

                    twilightAssaultLanesUsedH[row] = fromEnd ? 2 : 1;
                    return assaultPos;
                }
                else
                {
                    // Check if all lanes are used
                    bool allUsed = true;
                    for (uint8 i = 0; i < 5; ++i)
                        if (!twilightAssaultLanesUsedV[i])
                        {
                            allUsed = false;
                            break;
                        }
                    if (allUsed)
                    {
                        lane = 0;
                        return NULL;
                    }

                    // Find random unused lane
                    uint8 col;
                    do { col = urand(0, 4); }
                    while (twilightAssaultLanesUsedV[col]);

                    lane = col;

                    uint8 row = 0;
                    if (fromEnd)
                        while (twilightAssaultStalkerGuidsV[col][row + 1]) { ++row; } // Find the last one

                    Position* assaultPos = new Position();
                    if (Creature* stalker = instance->GetCreature(targetGUID = twilightAssaultStalkerGuidsV[col][row]))
                        stalker->GetPosition(assaultPos);

                    if (fromEnd)
                    {
                        Position offset = { -10.0f, 0.0f, 10.0f };
                        assaultPos->RelocateOffset(offset);
                        assaultPos->SetOrientation(0);
                    }
                    else
                    {
                        Position offset = { 10.0f, 0.0f, 10.0f };
                        assaultPos->RelocateOffset(offset);
                        assaultPos->SetOrientation(M_PI);
                    }

                    twilightAssaultLanesUsedV[col] = fromEnd ? 2 : 1;
                    return assaultPos;
                }
            }
            void FreeTwilightAssaulterAssaultLane(bool horizontal, uint8 lane) override
            {
                if (horizontal)
                    twilightAssaultLanesUsedH[lane] = 0;
                else
                    twilightAssaultLanesUsedV[lane] = 0;
            }
            virtual void CleanTwilightAssaulterAssaultLane(bool horizontal, uint8 lane) override
            {
                if (horizontal)
                {
                    for (uint8 col = 0; col < 8; ++col)
                    {
                        if (Creature* stalker = instance->GetCreature(twilightAssaultStalkerGuidsH[lane][col]))
                        {
                            stalker->RemoveAllAuras();
                        }
                    }
                }
            }

            void ActivatePortal(Creature* portal)
            {
                portal->RemoveAura(SPELL_TELEPORT_VISUAL_DISABLED);
                portal->CastSpell(portal, SPELL_TELEPORT_VISUAL_DISABLED, true);
                portal->CastSpell(portal, SPELL_TELEPORT_VISUAL_ACTIVE, true);
                portal->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
            }
            void DeactivatePortal(Creature* portal)
            {
                portal->RemoveAura(SPELL_TELEPORT_VISUAL_ACTIVE);
                portal->CastSpell(portal, SPELL_TELEPORT_VISUAL_DISABLED, true);
                portal->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_SPELLCLICK);
            }
            void SetPortalState(Creature* portal, bool active)
            {
                if (active)
                    ActivatePortal(portal);
                else
                    DeactivatePortal(portal);
            }

            private:
                uint32 uiTeamInInstance;

                ObjectGuid uiMorchokGUID;
                ObjectGuid uiKohcromGUID;
                ObjectGuid uiYorsahjGUID;
                ObjectGuid uiZonozzGUID;
                ObjectGuid uiHagaraGUID;
                ObjectGuid uiUltraxionGUID;
                ObjectGuid uiBlackhornGUID;
                ObjectGuid uiAllianceShipGUID;
                ObjectGuid uiAllianceShipFirstGUID;
                ObjectGuid uiHordeShipGUID;
                ObjectGuid uiSwayzeGUID;
                // teleports
                ObjectGuid uiWyrmrestBaseFromSummitGUID;
                ObjectGuid uiWyrmrestBaseFromGunshipGUID;
                ObjectGuid uiWyrmrestBaseFromMaelstormGUID;
                ObjectGuid uiWyrmrestSummitGUID;
                ObjectGuid uiEyeofEternityGUID;
                ObjectGuid uiDeckGUID;
                ObjectGuid uiMaelstormGUID;
                ObjectGuid uiDeathwingGUID;
                ObjectGuid uiAlexstraszaDragonGUID;
                ObjectGuid uiNozdormuDragonGUID;
                ObjectGuid uiYseraDragonGUID;
                ObjectGuid uiKalecgosDragonGUID;
                ObjectGuid uiThrall2GUID;

                ObjectGuid uiLesserCacheofTheAspects[4];
                GuidVector teleportGUIDs;
                ObjectGuid uiBackPlates[3];
                ObjectGuid uiGreaterCacheofTheAspects[4];
                ObjectGuid uiElementiumFragment[4];

                GuidVector dragonstaxiGUIDs;
                GuidVector assaultersGUIDs;
                GuidVector startportalsGUIDs;
                GuidVector wyrmrestsummitGUIDs;
                GuidVector maelstormteleGUIDs;

                ObjectGuid twilightAssaultStalkerGuidsH[7][8];
                ObjectGuid twilightAssaultStalkerGuidsV[5][10];
                uint8 twilightAssaultLanesUsedH[7];
                uint8 twilightAssaultLanesUsedV[5];

                ObjectGuid uiNethestraszGUID;
                uint32 uiOpenPortalEvent;
                ObjectGuid uiDeathwingEvent;
                ObjectGuid uiThrallEvent;
                uint32 bHagaraEvent;
                uint32 uiDragonSoulEvent;
                uint32 uiUltraxionTrash;
                uint8 uiDragonsCount;
                uint32 uiDelayedChestData;
               
                bool isLfr;
                bool lfrSectionFound;
                bool isFallOfDeathwing;

                class AssaulterCheck
                {
                    public:
                        AssaulterCheck(instance_dragon_soul_InstanceMapScript* instance) : m_instance(instance) { }

                        bool operator()(ObjectGuid guid)
                        {
                            Creature* assaulter = m_instance->instance->GetCreature(guid);
                            return assaulter && assaulter->isAlive() && !assaulter->AI()->GetData(1);
                        }

                    private:
                        instance_dragon_soul_InstanceMapScript* m_instance;
                };
        };
};

void AddSC_instance_dragon_soul()
{
    new instance_dragon_soul();
}