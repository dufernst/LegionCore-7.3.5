#ifndef DEF_ZULAMAN_H
#define DEF_ZULAMAN_H

enum Data
{
    DATA_AKILZON            = 0,
    DATA_NALORAKK           = 1,
    DATA_JANALAI            = 2,
    DATA_HALAZZI            = 3,
    DATA_HEX_LORD_MALACRASS = 4,
    DATA_DAAKARA            = 5,
    DATA_MAIN_GATE          = 6,
    DATA_TEMPEST            = 7,
    DATA_VENDOR_1           = 8,
    DATA_VENDOR_2           = 9,
};

enum CreaturesIds
{
    NPC_AKILZON             = 23574,
    NPC_NALORAKK            = 23576,
    NPC_JANALAI             = 23578,
    NPC_HALAZZI             = 23577,
    NPC_HEX_LORD_MALACRASS  = 24239,
    NPC_DAAKARA             = 23863,

    // Hostages
    NPC_BAKKALZU            = 52941, // near Akilzon
    NPC_BAKKALZU_CORPSE     = 52942,
    NPC_HAZLEK              = 52939,
    NPC_HAZLEK_CORPSE       = 52940,
    NPC_NORKANI             = 52943,
    NPC_NORKANI_CORPSE      = 52944,
    NPC_KASHA               = 52945,
    NPC_KASHA_CORPSE        = 52946,


    // Event npcs
    NPC_FOREST_FROG         = 24396,
    NPC_HARALD              = 52915, // Vendor 1
    NPC_EULINDA             = 52914, // Vendor 2
    NPC_ARINOTH             = 52919, // Money Bag
    NPC_LENZO               = 52917, // Money Bag
    NPC_MELISSA             = 52947, // Amani Charm Box
    NPC_MAWAGO              = 52920, // Amani Charm Box
    NPC_MELASONG            = 52916, // Amani Charm Box
    NPC_ROSA                = 52905, // Amani Charm Box
    NPC_RELISSA             = 52912, // Amani Charm Box
    NPC_TYLLAN              = 52909, // Amani Charm Box
    NPC_KALDRICK            = 52918, // Amani Charm Box
    NPC_MICAH               = 52910, // Amani Charm Box

    NPC_AMANISHI_TEMPEST    = 24549,
    NPC_AMANISHI_WARRIOR    = 24225,
    NPC_AMANI_EAGLE         = 24159,
    NPC_AMANISHI_LOOKOUT    = 24175,
};

enum GameObjectsIds
{
    GO_STRANGE_GONG         = 187359,
    GO_MAIN_GATE            = 186728,
    GO_AKILZON_EXIT         = 186858,
    GO_HALAZZI_ENTRANCE     = 186304,
    GO_HALAZZI_EXIT         = 186303,
    GO_MALACRASS_ENTRANCE   = 186305,
    GO_MALACRASS_EXIT       = 186306,
    GO_DAAKARA_EXIT         = 186859,
    
    GO_HAZLEK_CAGE          = 187377,
    GO_HAZLEK_TRUNK         = 186648,

    GO_NORKANI_CAGE         = 187379,
    GO_NORKANI_PACKAGE      = 186667,

    GO_KASHA_CAGE           = 187380,
    GO_KASHA_BAG            = 186672,
    GO_KASHA_VASE           = 186671,
};

enum SharedSpells
{
    SPELL_ZULAMAN_ACHIEVEMENT = 100938,
};

template<class AI>
CreatureAI* GetInstanceAI(Creature* creature)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId("instance_zulaman"))
                return new AI(creature);
    return NULL;
}

#endif

