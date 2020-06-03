/*
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "CreatureTextMgr.h"
#include "Garrison.h"

enum data
{
    Q_BUILD_BARRACKS_H   = 34461,
    Q_BUILD_BARRACKS_A   = 34587,

    GAR_BARRACK_BUILD   = 26,

    NPC_GAZLO           = 78466,
    NPC_BAROS           = 77209,
};

class wod_garrisone_horde_lvl1 : public InstanceMapScript
{
public:
    wod_garrisone_horde_lvl1() : InstanceMapScript("wod_garrisone_horde_lvl1", 1152) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new wod_garrisone_horde_lvl1_InstanceMapScript(map);
    }

    struct wod_garrisone_horde_lvl1_InstanceMapScript : public InstanceScript
    {
        wod_garrisone_horde_lvl1_InstanceMapScript(Map* map) : InstanceScript(map) {}

        void Initialize() override
        {

        }

        void OnPlaceBuilding(Player* player, Garrison* garrison, uint32 garrBuildingId, uint32 garrPlotInstanceId, time_t &time) override
        {
            if (garrBuildingId == GAR_BARRACK_BUILD)
            {
                if (player->GetQuestStatus(Q_BUILD_BARRACKS_H) != QUEST_STATUS_INCOMPLETE)
                    return;

                if (Creature *gazlo = player->FindNearestCreature(NPC_GAZLO, 100.0f))
                {
                    //ToDo: event 60 sec len
                    sCreatureTextMgr->SendChat(gazlo, TEXT_GENERIC_0, player->GetGUID());
                    sCreatureTextMgr->SendChat(gazlo, TEXT_GENERIC_1, player->GetGUID());
                    sCreatureTextMgr->SendChat(gazlo, TEXT_GENERIC_2, player->GetGUID());
                    player->KilledMonsterCredit(gazlo->GetEntry(), ObjectGuid::Empty);

                    time -= 3540;
                    //if (Garrison::Plot* plot = garrison->GetPlot(garrPlotInstanceId))
                    //    plot->BuildingInfo.PacketInfo->TimeBuilt -= 3540;
                }
            }
        };

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case 76411:
                {
                    uint32 const sp[3] = {163066, 163071, 163069};
                    creature->CastSpell(creature, sp[urand(0, 2)]);
                    break;
                }
            }
        }

        void Update(uint32 diff) override
        {

        }
    };
};

class wod_garrisone_alliance_lvl1 : public InstanceMapScript
{
public:
    wod_garrisone_alliance_lvl1() : InstanceMapScript("wod_garrisone_alliance_lvl1", 1158) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new wod_garrisone_alliance_lvl1_InstanceMapScript(map);
    }

    struct wod_garrisone_alliance_lvl1_InstanceMapScript : public InstanceScript
    {
        wod_garrisone_alliance_lvl1_InstanceMapScript(Map* map) : InstanceScript(map) {}

        void Initialize() override
        {

        }

        void OnPlaceBuilding(Player* player, Garrison* garrison, uint32 garrBuildingId, uint32 garrPlotInstanceId, time_t &time) override
        {
            if (garrBuildingId == GAR_BARRACK_BUILD)
            {
                if (player->GetQuestStatus(Q_BUILD_BARRACKS_A) != QUEST_STATUS_INCOMPLETE)
                    return;

                if (Creature *baros = player->FindNearestCreature(NPC_BAROS, 100.0f))
                {
                    //ToDo: event 60 sec len
                    sCreatureTextMgr->SendChat(baros, TEXT_GENERIC_4, player->GetGUID());
                    sCreatureTextMgr->SendChat(baros, TEXT_GENERIC_5, player->GetGUID());
                    sCreatureTextMgr->SendChat(baros, TEXT_GENERIC_6, player->GetGUID());
                    sCreatureTextMgr->SendChat(baros, TEXT_GENERIC_7, player->GetGUID());
                    time -= 3540;
                    //if (Garrison::Plot* plot = garrison->GetPlot(garrPlotInstanceId))
                    //    plot->BuildingInfo.PacketInfo->TimeBuilt -= 3540;
                }
            }
        };

        void Update(uint32 diff) override
        {

        }
    };
};

void AddSC_garrison_instance()
{
    new wod_garrisone_horde_lvl1();
    new wod_garrisone_alliance_lvl1();
}