/*====================
======================*/

#include "stormstout_brewery.h"

class instance_stormstout_brewery : public InstanceMapScript
{
public:
    instance_stormstout_brewery() : InstanceMapScript("instance_stormstout_brewery", 961) {}

    struct instance_stormstout_brewery_InstanceMapScript : public InstanceScript
    {
        instance_stormstout_brewery_InstanceMapScript(Map* map) : InstanceScript(map) {}

        ObjectGuid ookookGuid;
        ObjectGuid hoptallusGuid;
        ObjectGuid yanzhuGuid;
        ObjectGuid ookexitdoorGuid;
        ObjectGuid doorGuid;
        ObjectGuid door2Guid;
        ObjectGuid door3Guid;
        ObjectGuid door4Guid;
        ObjectGuid lastdoorGuid;
        ObjectGuid carrotdoorGuid;
        ObjectGuid sTriggerGuid;
        uint16 HoplingCount;
        uint16 GoldenHoplingCount;

        void Initialize() override
        {
            SetBossNumber(3);
            ookookGuid.Clear();
            hoptallusGuid.Clear();
            yanzhuGuid.Clear();
            ookexitdoorGuid.Clear();
            doorGuid.Clear();
            door2Guid.Clear();
            door3Guid.Clear();
            door4Guid.Clear();
            lastdoorGuid.Clear();
            carrotdoorGuid.Clear();
            HoplingCount = 0;
            GoldenHoplingCount = 0;
            sTriggerGuid.Clear();
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
            case GO_EXIT_OOK_OOK:
                ookexitdoorGuid = go->GetGUID();
                break;
            case GO_DOOR:
                doorGuid = go->GetGUID();
                break;
            case GO_DOOR2:
                door2Guid = go->GetGUID();
                break;
            case GO_DOOR3:
                door3Guid = go->GetGUID();
                break;
            case GO_DOOR4:
                door4Guid = go->GetGUID();
                break;
            case GO_LAST_DOOR:
                lastdoorGuid = go->GetGUID();
                break;
            case GO_CARROT_DOOR:
                carrotdoorGuid = go->GetGUID();
                break;
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_OOK_OOK:
                    ookookGuid = creature->GetGUID();
                    break;
                case NPC_HOPTALLUS:
                    hoptallusGuid = creature->GetGUID();
                    break;
                case NPC_YAN_ZHU:
                    yanzhuGuid = creature->GetGUID();
                    break;
                case NPC_TRIGGER_SUMMONER:
                    sTriggerGuid = creature->GetGUID();
                    break;
            }
        }
        
        bool SetBossState(uint32 id, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
            case DATA_OOK_OOK:
                {
                    if (state == DONE)
                    {
                        HandleGameObject(ookexitdoorGuid, true);
                        HandleGameObject(doorGuid, true);
                        HandleGameObject(door2Guid, true);
                        if (auto trigger = instance->GetCreature(sTriggerGuid))
                        {
                            trigger->CastSpell(trigger, SPELL_HOPPER_SUM_EXPLOSIVE);
                            trigger->CastSpell(trigger, SPELL_HOPPER_SUM_HAMMER);
                            trigger->CastSpell(trigger, SPELL_HOPLING_AURA_3);
                        }
                    }
                }
                break;
            case DATA_HOPTALLUS:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                        HandleGameObject(door2Guid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(door2Guid, false);
                        if (auto trigger = instance->GetCreature(sTriggerGuid))
                        {
                            trigger->RemoveAurasDueToSpell(SPELL_HOPPER_SUM_EXPLOSIVE);
                            trigger->RemoveAurasDueToSpell(SPELL_HOPPER_SUM_HAMMER);
                            trigger->RemoveAurasDueToSpell(SPELL_HOPLING_AURA_3);
                        }
                        break;
                    case DONE:
                        {
                            HandleGameObject(door2Guid, true);
                            HandleGameObject(door3Guid, true);
                            HandleGameObject(door4Guid, true);
                            if (auto go = instance->GetGameObject(carrotdoorGuid))
                                go->Delete();
                        }
                        break;
                    }
                }
                break;
            case DATA_YAN_ZHU:
                {
                    switch (state)
                    {
                    case NOT_STARTED:
                    case DONE:
                        HandleGameObject(lastdoorGuid, true);
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(lastdoorGuid, false);
                        break;
                    }
                }
                break;
            }
            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == DATA_HOPLING)
            {
                HoplingCount = data;
                if (HoplingCount > 100)
                    HoplingCount = 100;
            }

            if (type == DATA_GOLDEN_HOPLING)
                GoldenHoplingCount = data;
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_HOPLING)
                return HoplingCount;

            if (type == DATA_GOLDEN_HOPLING)
                return GoldenHoplingCount;

            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case NPC_OOK_OOK:
                    return ookookGuid;
                case NPC_HOPTALLUS:
                    return hoptallusGuid;
                case NPC_YAN_ZHU:
                    return yanzhuGuid;
            }

            return ObjectGuid::Empty;
        }

        void Update(uint32 diff) override
        {
            // Challenge
            InstanceScript::Update(diff);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_stormstout_brewery_InstanceMapScript(map);
    }
};

void AddSC_instance_stormstout_brewery()
{
    new instance_stormstout_brewery();
}
