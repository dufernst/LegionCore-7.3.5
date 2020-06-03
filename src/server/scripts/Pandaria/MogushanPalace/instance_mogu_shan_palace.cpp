/*===============
================*/

#include "VMapFactory.h"
#include "mogu_shan_palace.h"
#include "Packets/WorldStatePackets.h"

class instance_mogu_shan_palace : public InstanceMapScript
{
public:
    instance_mogu_shan_palace() : InstanceMapScript("instance_mogu_shan_palace", 994) {}

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_mogu_shan_palace_InstanceMapScript(map);
    }

    struct instance_mogu_shan_palace_InstanceMapScript : public InstanceScript
    {
        ObjectGuid xin_guid;
        ObjectGuid kuai_guid;
        ObjectGuid ming_guid;
        ObjectGuid haiyan_guid;
        GuidList scrappers;
        GuidList adepts;
        GuidList grunts;

        ObjectGuid gekkan;
        ObjectGuid glintrok_ironhide;
        ObjectGuid glintrok_skulker;
        ObjectGuid glintrok_oracle;
        ObjectGuid glintrok_hexxer;

        GuidList animated_staffs;
        GuidList animated_axes;
        GuidList swordLauncherGuids;

        ObjectGuid doorBeforeTrialGuid;
        ObjectGuid trialChestGuid;
        ObjectGuid doorAfterTrialGuid;
        ObjectGuid doorBeforeKingGuid;
        ObjectGuid secretdoorGuid;

        ObjectGuid BeaconGuid;

        uint8 JadeCount;
        uint8 GemCount;

        instance_mogu_shan_palace_InstanceMapScript(Map* map) : InstanceScript(map) {}

        void Initialize() override
        {
            xin_guid.Clear();
            kuai_guid.Clear();
            ming_guid.Clear();
            haiyan_guid.Clear();

            doorBeforeTrialGuid.Clear();
            trialChestGuid.Clear();
            doorAfterTrialGuid.Clear();
            doorBeforeKingGuid.Clear();
            secretdoorGuid.Clear();

            gekkan.Clear();
            glintrok_ironhide.Clear();
            glintrok_skulker.Clear();
            glintrok_oracle.Clear();
            glintrok_hexxer.Clear();
            
            JadeCount = 0;
            GemCount = 0;
        }

        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override
        {
            packet.Worldstates.emplace_back(static_cast<WorldStates>(6761), JadeCount > 0 && JadeCount != 5); // Show_Jade
            packet.Worldstates.emplace_back(static_cast<WorldStates>(6748), JadeCount); // Jade_Count
        }

        bool SetBossState(uint32 id, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(id, state))
                return false;

            switch (id)
            {
                case DATA_TRIAL_OF_THE_KING:
                    break;
                case DATA_GEKKAN:
                    HandleGameObject(doorAfterTrialGuid, state == DONE);
                    // Todo : mod temp portal phasemask
                    break;
                case DATA_XIN_THE_WEAPONMASTER:
                    //HandleGameObject(doorBeforeTrialGuid, state != IN_PROGRESS);
                    break;
            }

            return true;
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_DOOR_BEFORE_TRIAL:  doorBeforeTrialGuid = go->GetGUID();    break;
                case GO_TRIAL_CHEST:
                case GO_TRIAL_CHEST2:
                    trialChestGuid = go->GetGUID();
                    break;
                case GO_DOOR_AFTER_TRIAL:   doorAfterTrialGuid = go->GetGUID();     break;
                case GO_DOOR_BEFORE_KING:   doorBeforeKingGuid = go->GetGUID();     break;
                case GO_SECRET_DOOR:        secretdoorGuid = go->GetGUID();         break;
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            OnCreatureCreate_gekkan(creature);
            OnCreatureCreate_trial_of_the_king(creature);
            OnCreatureCreate_xin_the_weaponmaster(creature);
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_GEKKAN_ADDS:
                    if (auto Gekkan = instance->GetCreature(gekkan))
                    {
                        if (auto target = Gekkan->SelectNearestTarget(100.0f))
                        {
                            Gekkan->AI()->AttackStart(target);

                            if (auto ironhide = instance->GetCreature(glintrok_ironhide))
                                ironhide->AI()->AttackStart(target);

                            if (auto skulker = instance->GetCreature(glintrok_skulker))
                                skulker->AI()->AttackStart(target);

                            if (auto oracle = instance->GetCreature(glintrok_oracle))
                                oracle->AI()->AttackStart(target);

                            if (auto hexxer = instance->GetCreature(glintrok_hexxer))
                                hexxer->AI()->AttackStart(target);
                        }
                    }
                    break;
                case TYPE_JADECOUNT:
                    JadeCount = data;
                    DoUpdateWorldState(static_cast<WorldStates>(6761), 1);
                    DoUpdateWorldState(static_cast<WorldStates>(6748), JadeCount);

                    if (JadeCount == 5)
                    {
                        DoCastSpellOnPlayers(SPELL_ACHIEV_JADE_QUILEN);
                        DoUpdateWorldState(static_cast<WorldStates>(6761), 0);
                    }
                    break;
                case TYPE_GEMCOUNT:
                    GemCount = data;
                    break;
            }

            SetData_trial_of_the_king(type, data);
            SetData_xin_the_weaponmaster(type, data);
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == TYPE_JADECOUNT)
                return JadeCount;
            if (type == TYPE_GEMCOUNT)
                return GemCount;

            return 0;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
            case TYPE_GET_ENTOURAGE_0:
                return glintrok_hexxer;
            case TYPE_GET_ENTOURAGE_1:
                return glintrok_ironhide;
            case TYPE_GET_ENTOURAGE_2:
                return glintrok_oracle;
            case TYPE_GET_ENTOURAGE_3:
                return glintrok_skulker;
            }
            return ObjectGuid::Empty;
        }

        bool isWipe()
        {
            instance->ApplyOnEveryPlayer([&](Player* player)
            {
                if (player->isAlive() && !player->isGameMaster())
                    return false;
            });

            return true;
        }

        void SetData_xin_the_weaponmaster(uint32 type, uint32 data)
        {
            switch (type)
            {
                case TYPE_ACTIVATE_ANIMATED_STAFF:
                    if (auto creature = instance->GetCreature(Trinity::Containers::SelectRandomContainerElement(animated_staffs)))
                        creature->GetAI()->DoAction(0);
                    break;
                case TYPE_ACTIVATE_ANIMATED_AXE:
                    for (GuidList::iterator guid = animated_axes.begin(); guid != animated_axes.end(); ++guid)
                    {
                        if (auto creature = instance->GetCreature(*guid))
                        {
                            if (data)
                            {
                                creature->AddAura(SPELL_AXE_TOURBILOL, creature);
                                creature->AddAura(130966, creature);
                                creature->GetMotionMaster()->MoveRandom(50.0f);
                            }
                            else
                            {
                                creature->RemoveAurasDueToSpell(SPELL_AXE_TOURBILOL);
                                creature->RemoveAurasDueToSpell(130966);
                                creature->GetMotionMaster()->MoveTargetedHome();
                            }
                        }
                    }
                    break;
                case TYPE_ACTIVATE_SWORD:
                    Position center;
                    center.Relocate(-4632.39f, -2613.20f, 22.0f);

                    bool randPos = urand(0, 1);

                    for (GuidList::iterator guid = swordLauncherGuids.begin(); guid != swordLauncherGuids.end(); ++guid)
                    {
                        bool mustActivate = false;

                        if (auto launcher = instance->GetCreature(*guid))
                        {
                            if (randPos) // Zone 2 & 3
                            {
                                if (launcher->GetPositionX() > center.GetPositionX() && launcher->GetPositionY() > center.GetPositionY()
                                    || launcher->GetPositionX() < center.GetPositionX() && launcher->GetPositionY() < center.GetPositionY())
                                    mustActivate = true;
                            }
                            else // Zone 1 & 4
                            {
                                if (launcher->GetPositionX() > center.GetPositionX() && launcher->GetPositionY() < center.GetPositionY()
                                    || launcher->GetPositionX() < center.GetPositionX() && launcher->GetPositionY() > center.GetPositionY())
                                    mustActivate = true;
                            }

                            if (data && mustActivate)
                                launcher->AddAura(SPELL_THROW_AURA, launcher);
                            else
                                launcher->RemoveAurasDueToSpell(SPELL_THROW_AURA);
                        }
                    }
                break;
            }
        }
        void OnCreatureCreate_xin_the_weaponmaster(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case 59481:
                    creature->SetReactState(REACT_PASSIVE);
                    creature->SetDisplayId(11686);
                    break;
                case CREATURE_ANIMATED_STAFF:
                    animated_staffs.push_back(creature->GetGUID());
                    break;
                case CREATURE_ANIMATED_AXE:
                    animated_axes.push_back(creature->GetGUID());
                    creature->SetReactState(REACT_PASSIVE);
                    creature->SetVirtualItem(0, 30316);
                    break;
                case CREATURE_LAUNCH_SWORD:
                    swordLauncherGuids.push_back(creature->GetGUID());
                    creature->AddAura(130966, creature);
                    break;
            }
        }

        void OnCreatureCreate_gekkan(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case CREATURE_GEKKAN:
                    gekkan = creature->GetGUID();
                    break;
                case CREATURE_GLINTROK_IRONHIDE:
                    glintrok_ironhide = creature->GetGUID();
                    break;
                case CREATURE_GLINTROK_SKULKER:
                    glintrok_skulker = creature->GetGUID();
                    break;
                case CREATURE_GLINTROK_ORACLE:
                    glintrok_oracle = creature->GetGUID();
                    break;
                case CREATURE_GLINTROK_HEXXER:
                    glintrok_hexxer = creature->GetGUID();
                    break;
            }
        }

        void SetData_trial_of_the_king(uint32 type, uint32 data)
        {
            switch (type)
            {
            case TYPE_OUTRO_05:
                if (auto haiyan = instance->GetCreature(haiyan_guid))
                    haiyan->GetAI()->DoAction(1); //ACTION_OUTRO_02
                break;
            case TYPE_OUTRO_04:
                if (auto kuai = instance->GetCreature(kuai_guid))
                    kuai->GetAI()->DoAction(3); //ACTION_OUTRO_02
                break;
            case TYPE_OUTRO_03:
                if (auto ming = instance->GetCreature(ming_guid))
                    ming->GetAI()->DoAction(3); //ACTION_OUTRO_02
                break;
            case TYPE_OUTRO_02:
                if (auto haiyan = instance->GetCreature(haiyan_guid))
                    haiyan->GetAI()->DoAction(1); //ACTION_OUTRO_01
                break;
            case TYPE_OUTRO_01:
                if (auto ming = instance->GetCreature(ming_guid))
                    ming->GetAI()->DoAction(2); //ACTION_OUTRO_01
                break;
            case TYPE_MING_INTRO:
                if (auto ming = instance->GetCreature(ming_guid))
                    ming->GetAI()->DoAction(1); //ACTION_INTRO
                break;
            case TYPE_WIPE_FIRST_BOSS:
                if (auto xin = instance->GetCreature(xin_guid))
                {
                    xin->SetVisible(true);
                    xin->GetAI()->Reset();

                    switch (data)
                    {
                    case 0:
                        for (GuidList::iterator guid = adepts.begin(); guid != adepts.end(); ++guid)
                        {
                            if (auto creature = instance->GetCreature(*guid))
                            {
                                creature->GetAI()->DoAction(2);
                                creature->RemoveAura(121569);
                            }
                        }
                        break;
                    case 1:
                        for (GuidList::iterator guid = scrappers.begin(); guid != scrappers.end(); ++guid)
                        {
                            if (auto creature = instance->GetCreature(*guid))
                            {
                                creature->GetAI()->DoAction(2);
                                creature->RemoveAura(121569);
                            }
                        }
                        break;
                    case 2:
                        for (GuidList::iterator guid = grunts.begin(); guid != grunts.end(); ++guid)
                        {
                            if (auto creature = instance->GetCreature(*guid))
                            {
                                creature->GetAI()->DoAction(2);
                                creature->RemoveAura(121569);
                            }
                        }
                        break;
                    }
                }
                break;
            case TYPE_MING_ATTACK:
                for (GuidList::iterator guid = adepts.begin(); guid != adepts.end(); ++guid)
                    if (auto creature = instance->GetCreature(*guid))
                        creature->GetAI()->DoAction(1);

                if (auto ming = instance->GetCreature(ming_guid))
                {
                    ming->GetMotionMaster()->MovePoint(0, -4237.658f, -2613.860f, 16.48f);
                    ming->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    ming->SetReactState(REACT_AGGRESSIVE);
                }
                break;
            case TYPE_KUAI_ATTACK:
                for (GuidList::iterator guid = scrappers.begin(); guid != scrappers.end(); ++guid)
                    if (auto creature = instance->GetCreature(*guid))
                        creature->GetAI()->DoAction(1);

                if (auto kuai = instance->GetCreature(kuai_guid))
                {
                    kuai->GetMotionMaster()->MovePoint(0, -4215.359f, -2601.283f, 16.48f);
                    kuai->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    kuai->SetReactState(REACT_AGGRESSIVE);
                }
                break;
            case TYPE_HAIYAN_ATTACK:
                for (GuidList::iterator guid = grunts.begin(); guid != grunts.end(); ++guid)
                    if (auto creature = instance->GetCreature(*guid))
                        creature->GetAI()->DoAction(1); //EVENT_ENCOURAGE

                if (auto haiyan = instance->GetCreature(haiyan_guid))
                {
                    haiyan->GetMotionMaster()->MovePoint(0, -4215.772f, -2627.216f, 16.48f);
                    haiyan->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    haiyan->SetReactState(REACT_AGGRESSIVE);
                }
                break;
            case TYPE_ALL_ATTACK:
                for (GuidList::iterator guid = adepts.begin(); guid != adepts.end(); ++guid)
                {
                    if (auto creature = instance->GetCreature(*guid))
                    {
                        creature->GetAI()->DoAction(3); //ACTION_ATTACK

                        GuidList::iterator itr = grunts.begin();
                        std::advance(itr, urand(0, grunts.size() - 1));

                        if (auto grunt = instance->GetCreature(*itr))
                            creature->Attack(grunt, true);
                    }

                    for (GuidList::iterator guid = grunts.begin(); guid != grunts.end(); ++guid)
                    {
                        if (auto creature = instance->GetCreature(*guid))
                        {
                            creature->GetAI()->DoAction(3); //ACTION_ATTACK

                            GuidList::iterator itr = scrappers.begin();
                            std::advance(itr, urand(0, scrappers.size() - 1));

                            if (auto scrapper = instance->GetCreature(*itr))
                                creature->Attack(scrapper, true);
                        }
                    }
                    for (GuidList::iterator guid = scrappers.begin(); guid != scrappers.end(); ++guid)
                    {
                        if (auto creature = instance->GetCreature(*guid))
                        {
                            creature->GetAI()->DoAction(3); //ACTION_ATTACK

                            GuidList::iterator itr = adepts.begin();
                            std::advance(itr, urand(0, adepts.size() - 1));

                            if (auto adept = instance->GetCreature(*itr))
                                creature->Attack(adept, true);
                        }
                    }

                    if (auto beacon = instance->GetCreature(BeaconGuid))
                        beacon->SetVisible(true);
                }
                break;
            case TYPE_MING_RETIRED:
                for (GuidList::iterator guid = adepts.begin(); guid != adepts.end(); ++guid)
                    if (auto creature = instance->GetCreature(*guid))
                        creature->GetAI()->DoAction(2); //EVENT_RETIRE
                break;
            case TYPE_KUAI_RETIRED:
                for (GuidList::iterator guid = scrappers.begin(); guid != scrappers.end(); ++guid)
                    if (auto creature = instance->GetCreature(*guid))
                        creature->GetAI()->DoAction(2); //EVENT_RETIRE
                break;
            case TYPE_HAIYAN_RETIRED:
                for (GuidList::iterator guid = grunts.begin(); guid != grunts.end(); ++guid)
                    if (auto creature = instance->GetCreature(*guid))
                        creature->GetAI()->DoAction(2); //EVENT_RETIRE
     
                if (auto xin = instance->GetCreature(xin_guid))
                {
                    xin->DespawnOrUnsummon();
                    HandleGameObject(doorAfterTrialGuid, true);
                }

                if (auto chest = instance->GetGameObject(trialChestGuid))
                {
                    chest->SetPhaseMask(1, true);
                    chest->SetRespawnTime(604800);
                    instance->SummonCreature(CREATURE_JADE_QUILEN, otherPos[0]);
                }
                
                if (auto go = instance->GetGameObject(secretdoorGuid))
                    go->SetGoState(GO_STATE_ACTIVE);
                break;
            }
        }

        void OnCreatureCreate_trial_of_the_king(Creature* creature)
        {
            switch (creature->GetEntry())
            {
            case CREATURE_GURTHAN_SCRAPPER:
                scrappers.push_back(creature->GetGUID());
                break;
            case CREATURE_HARTHAK_ADEPT:
            case 61549:
                adepts.push_back(creature->GetGUID());
                break;
            case CREATURE_KARGESH_GRUNT:
                grunts.push_back(creature->GetGUID());
                break;
            case CREATURE_KUAI_THE_BRUTE:
                kuai_guid = creature->GetGUID();
                creature->SetReactState(REACT_PASSIVE);
                break;
            case CREATURE_MING_THE_CUNNING:
                ming_guid = creature->GetGUID();
                creature->SetReactState(REACT_PASSIVE);
                break;
            case CREATURE_HAIYAN_THE_UNSTOPPABLE:
                haiyan_guid = creature->GetGUID();
                creature->SetReactState(REACT_PASSIVE);
                break;
            case CREATURE_XIN_THE_WEAPONMASTER_TRIGGER:
                xin_guid = creature->GetGUID();
                creature->SetReactState(REACT_PASSIVE);
                break;
            case CREATURE_WHIRLING_DERVISH:
                break;
            case 64250:
                BeaconGuid = creature->GetGUID();

                if (auto beacon = instance->GetCreature(BeaconGuid))
                    beacon->SetVisible(false);
                break;
            }
        }
    };
};

class go_mogushan_palace_temp_portal : public GameObjectScript
{
public:
    go_mogushan_palace_temp_portal() : GameObjectScript("go_mogushan_palace_temp_portal") {}

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (go->GetPositionZ() < 0.0f)
            player->NearTeleportTo(go->GetPositionX(), go->GetPositionY(), 22.31f, go->GetOrientation());
        else
            player->NearTeleportTo(go->GetPositionX(), go->GetPositionY(), -39.0f, go->GetOrientation());

        return false;
    }
};

void AddSC_instance_mogu_shan_palace()
{
    new instance_mogu_shan_palace();
    //new go_mogushan_palace_temp_portal();
}
