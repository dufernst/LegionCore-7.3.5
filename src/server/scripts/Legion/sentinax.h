#ifndef SENTINAX_H
#define SENTINAX_H

#include "OutdoorPvP.h"
#include "GameEventMgr.h"


enum SomeValues
{
    DIST_SENTINAX               = 450,
    GO_SENTINAX                 = 268706,
    
    NPC_TARGET_PORTAL           = 950010,
    NPC_OWNER_PORTALS           = 950011,
    NPC_SENTINAX                = 950012, 
    NPC_TARGET_LASER            = 950013,
    GOB_PORTAL                  = 269125,
    
    MAX_ACTIVE_PORTALS          = 8,
    COOLDOWN_FOR_BOSSES         = 181000,
};

enum Spells
{
    SPELL_LASER_DMG             = 240852,
    SPELL_NOT_FREE_PORTAL       = 242872,
    
    SPELL_SUMMON_LASER          = 241400,
    SPELL_GREEN_BEAM            = 40071,
    SPELL_EFFECT_SUMMON_MOB     = 241418,
    SPELL_EFFECT_SUMMON_BOSS    = 241419,
    // SPELL_EFFECT_PORTAL         = ,
};

enum SpellsCall
{
    // common
    TORMENT_PORTAL              = 240311,
    ENGINEERING_PORTAL          = 240308,
    WARBEAST_PORTAL             = 240305,
    CARNAGE_PORTAL              = 240302,
    FIRESTORM_PORTAL            = 240299,
    DOMINATION_PORTAL           = 240123,
    
    // uncommon
    GREATER_TORMENT_PORTAL      = 240312,
    GREATER_ENGINEERING_PORTAL  = 240309,
    GREATER_WARBEAST_PORTAL     = 240306,
    GREATER_CARHAGE_PORTAL      = 240303,
    GREATER_FIRESTORM_PORTAL    = 240300,
    GREATER_DOMINATION_PORTAL   = 240297,
    
    BOSS_TORMENT_PORTAl         = 240313, // Illisthyndria
    BOSS_WARBEAST_PORTAL        = 240307, // anthynia
    BOSS_CARNAGE_PORTAL         = 240304, // XILLIOUS    
    BOSS_DOMITAION_PORTAL       = 240298, // thanotalion
    BOSS_FIRESTORM_PORTAL       = 240301, // skulguloth
    BOSS_ENGINEERING_PORTAL     = 240310, // obliterator
};


std::map<uint32, uint32> portalForEmpowered
{
    {GREATER_TORMENT_PORTAL, TORMENT_PORTAL},
    {GREATER_ENGINEERING_PORTAL, ENGINEERING_PORTAL},
    {GREATER_WARBEAST_PORTAL, WARBEAST_PORTAL},
    {GREATER_CARHAGE_PORTAL, CARNAGE_PORTAL},
    {GREATER_FIRESTORM_PORTAL, FIRESTORM_PORTAL},
    {GREATER_DOMINATION_PORTAL, DOMINATION_PORTAL} 
};


enum Npc
{
    // torment
    NPC_OBSERVER                = 120684,
    NPC_LEGION_PAINMAIDEN       = 120682,
    NPC_INTERROGATOR            = 120683, // 2
    
    //engineering
    NPC_CURSED_PILLAGER         = 120677, // 3
    NPC_PUTRID_ALCHEMIST        = 120676, // 4
    NPC_GANARG_ENGINEER         = 120678, // 5
    
    // warbeast
    NPC_SPELLSTALKER            = 120668, // 6
    NPC_WEBSPEWER               = 120666, // 7
    NPC_VENOMRETCHER            = 120667, // 8
    
    // carnage
    NPC_WRATHGUARD              = 120661, // 9
    NPC_INVADER                 = 120658, // 10
    NPC_VANGUARD                = 120659, // 11
    
    // firestorm
    NPC_FIEND                   = 120629, // 12
    NPC_BLAZING_IMP             = 120627, // 13
    NPC_VIRULENT_IMP            = 120628, // 14
    
    // domination
    NPC_INFECTIOUS_STALKER      = 120576, // 15
    NPC_HUNGERING_STALKER       = 120575, // 16
    NPC_DARLFIEND_DEVOURER      = 120577, // 17
    
    // torment uncommon
    NPC_WARDEN                  = 120685, // 18
    
    // engineering uncommon
    NPC_MOARG                   = 120679, // 19
    
    // warbeast uncommon
    NPC_FELSHRIEKER             = 120669, // 20
    NPC_RIPPER                  = 120674, // 21
    
    // carnage uncommon
    NPC_FIRECALLER              = 120663, // 22
    
    // firestorm uncommon
    NPC_INFERNAL                = 120631, // 23
    NPC_VILE_MOTHER             = 120640, // 24
    
    // domination uncommon
    NPC_DRAINING_EYE            = 120581, // 25
    
    
    // boss torment 
    NPC_IILLISTHYNDRIA          = 120686,    
    // boss warbeast
    NPC_ANTHYNA                 = 120675,    
    // boss carnage
    NPC_XILLIOUS                = 120665,    
    // boss domination
    NPC_THANOTALION             = 120583,     
    // boss firestorm
    NPC_SKULGULOTH              = 120641,    
    // boss engineering
    NPC_OBLITERATOR             = 120681,
    
};

uint32 Npcs []
{
        // torment
    NPC_OBSERVER,
    NPC_LEGION_PAINMAIDEN ,
    NPC_INTERROGATOR ,
    
    //engineering
    NPC_CURSED_PILLAGER,
    NPC_PUTRID_ALCHEMIST,
    NPC_GANARG_ENGINEER,
    
    // warbeast
    NPC_SPELLSTALKER,
    NPC_WEBSPEWER,
    NPC_VENOMRETCHER,
    
    // carnage
    NPC_WRATHGUARD,
    NPC_INVADER,
    NPC_VANGUARD,
    
    // firestorm
    NPC_FIEND,
    NPC_BLAZING_IMP,
    NPC_VIRULENT_IMP,
    
    // domination
    NPC_INFECTIOUS_STALKER,
    NPC_HUNGERING_STALKER,
    NPC_DARLFIEND_DEVOURER,
    
    // torment uncommon
    NPC_WARDEN ,
    
    // engineering uncommon
    NPC_MOARG,
    
    // warbeast uncommon
    NPC_FELSHRIEKER,
    NPC_RIPPER ,
    
    // carnage uncommon
    NPC_FIRECALLER ,
    
    // firestorm uncommon
    NPC_INFERNAL  ,
    NPC_VILE_MOTHER   ,
    
    // domination uncommon
    NPC_DRAINING_EYE
};


class OutdoorPVPSentinax : public OutdoorPvP
{
public:

    OutdoorPVPSentinax()
    {
        m_TypeId = OUTDOOR_PVP_SENTINAX;
    }

    bool SetupOutdoorPvP() override 
    {
        RegisterZone(7543);
        return true;
    }

    void Initialize(uint32 zone) override
    {
        for (auto event : { 117, 118, 119, 120, 121, 122, 123, 124 })
            if (sGameEventMgr->IsActiveEvent(event))
            {
                last_active_event = event;
                break;
            }
    }
    
    uint32 GetData(uint32 /*DataId*/) const override
    {
        return active_portals < MAX_ACTIVE_PORTALS;
    }
    
    void SetData(uint32 DataId, uint32 value) override
    {
        if (DataId) // increment
        {
            active_portals += value;
            if (DataId == 2) // if rare boss, then start event
            {
                sGameEventMgr->StartEvent(153, true);

                timer_for_reload = COOLDOWN_FOR_BOSSES;
                has_timer = true;
                
                if (!m_playersInArea.empty()) // for reload auras
                {
                    for (GuidSet::iterator itr = m_playersInArea.begin(); itr != m_playersInArea.end(); ++itr)
                        if (Player* player = ObjectAccessor::GetObjectInMap((*itr), m_map, (Player*)nullptr))
                            player->AddDelayedEvent(100, [player] () -> void { if (player) player->UpdateArea(player->GetCurrentAreaID()); });
                }
            }
        }
        else if (value) // decrement
        {
            if (value >= active_portals)
                active_portals = 0;
            else
                active_portals -= value;
        }
        else // reset
            active_portals = 0;

    }

    bool Update(uint32 diff) override
    {
        if (has_timer)
        {
            if (timer_for_reload <= diff)
            {
                if (!m_playersInArea.empty())
                {
                    for (GuidSet::iterator itr = m_playersInArea.begin(); itr != m_playersInArea.end(); ++itr)
                        if (Player* player = ObjectAccessor::GetObjectInMap((*itr), m_map, (Player*)nullptr))
                            player->AddDelayedEvent(100, [player] () -> void { if (player) player->UpdateArea(player->GetCurrentAreaID()); });
                }
                has_timer = false;
            }
            else
                timer_for_reload -= diff;
            
        }
        
        if (timer_for_check <= diff)
        {
            if (!sGameEventMgr->IsActiveEvent(last_active_event)) // new position, we need reload
            {
                for (auto event : {117, 118, 119, 120, 121, 122, 123, 124})
                    if (sGameEventMgr->IsActiveEvent(event))
                    {
                        last_active_event = event;
                        break;
                    }
                
                active_portals = 0; 
            }
            timer_for_check = 5000;
        }
        else
            timer_for_check -= diff;
        return true;
    }
    
private:
    uint32 active_portals = 0;
    uint32 timer_for_reload;
    bool has_timer = false;
    uint32 last_active_event = 0;
    uint32 timer_for_check = 5000;
};


#endif