#include "blackrock_foundry.h"

class instance_blackrock_foundry : public InstanceMapScript
{
public:
    instance_blackrock_foundry() : InstanceMapScript("instance_blackrock_foundry", 1205) {}

    struct instance_blackrock_foundry_InstanceMapScript : public InstanceScript
    {
        instance_blackrock_foundry_InstanceMapScript(Map* map) : InstanceScript(map) {}

        WorldLocation loc_res_pla;

        void Initialize() override
        {
            SetBossNumber(MaxBossData);
        }

        void OnCreatureCreate(Creature* creature) override {}

        void OnGameObjectCreate(GameObject* gameObject) override {}

        void SetData(uint32 type, uint32 data) override {}

        uint32 GetData(uint32 type) const override
        {
            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            return ObjectGuid::Empty;
        }

        WorldLocation* GetClosestGraveYard(float x, float y, float z) override
        {
            uint32 graveyardId = 4778;

            if (WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(graveyardId))
            {
                loc_res_pla.Relocate(gy->Loc.X, gy->Loc.Y, gy->Loc.Z);
                loc_res_pla.SetMapId(gy->MapID);
            }

            return &loc_res_pla;
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_blackrock_foundry_InstanceMapScript(map);
    }
};

void AddSC_instance_blackrock_foundry()
{
    new instance_blackrock_foundry();
}
