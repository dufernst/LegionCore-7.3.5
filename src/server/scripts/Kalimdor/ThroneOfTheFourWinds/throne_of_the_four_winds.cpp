#include "throne_of_the_four_winds.h"

enum SlipstreamEnums
{
    // Land Positions
    DIR_WEST_TO_SOUTH,
    DIR_SOUTH_TO_WEST,
    DIR_NORTH_TO_WEST,
    DIR_WEST_TO_NORTH,
    DIR_EAST_TO_NORTH,
    DIR_NORTH_TO_EAST,
    DIR_SOUTH_TO_EAST,
    DIR_EAST_TO_SOUTH,
    DIR_ERROR,

    // Spells
    SPELL_SLIPSTREAM_BUFF                   = 87740,
    SPELL_SLIPSTREAM_PLAYER_VISUAL          = 85063,
    SPELL_SLEET_STORM_ULTIMATE              = 84644,
};

Position const SlipstreamPositions[8] =
{
    {-245.141129f,  861.474060f,    197.386398f,    0},
    {-92.116440f,   1010.796448f,   197.475754f,    0},
    {-5.322055f,    1010.573608f,   197.520096f,    0},
    {144.480469f,   857.187927f,    197.594208f,    0},
    {144.221481f,   770.720154f,    197.629150f,    0},
    {-9.268366f,    620.736328f,    197.567032f,    0},
    {-96.089645f,   621.198730f,    197.499115f,    0},
    {-245.653870f,  774.446472f,    197.507156f,    0},
};

class npc_slipstream_raid : public CreatureScript
{
public:
    npc_slipstream_raid() : CreatureScript("npc_slipstream_raid") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_slipstream_raidAI (creature);
    }

    struct npc_slipstream_raidAI : public ScriptedAI
    {
        npc_slipstream_raidAI(Creature* creature) : ScriptedAI(creature), isActive(true), linkedSlipstreamObject(NULL), linkedBoss(NULL), isUltimate(NULL)
        {
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);

            SlipstreamPosition = 8;

            for (uint8 i = 0; i <= 7; i++)
                if (me->GetDistance2d(SlipstreamPositions[i].GetPositionX(), SlipstreamPositions[i].GetPositionY()) < 10)
                {
                    SlipstreamPosition = i;
                    break;
                }

                if (SlipstreamPosition >= DIR_ERROR)
                    return;

                SlipstreamPosition += (SlipstreamPosition == DIR_WEST_TO_SOUTH || SlipstreamPosition == DIR_NORTH_TO_WEST ||
                    SlipstreamPosition == DIR_EAST_TO_NORTH || SlipstreamPosition == DIR_SOUTH_TO_EAST ) ? 1 : -1;

                // Assign linked Boss and Slipstream to disabled slipstreams if the bosses casts Ultimate
        }

        uint8 SlipstreamPosition;
        bool isUltimate;
        bool isActive;

        GameObject* linkedSlipstreamObject;
        Unit* linkedBoss;

        void MoveInLineOfSight(Unit* who)
        {
            if (SlipstreamPosition >= DIR_ERROR || who->GetTypeId() != TYPEID_PLAYER || !isActive)
                return;

            if (who->GetExactDist(me) <= 5.0f)
            {
                me->AddAura(SPELL_SLIPSTREAM_BUFF, who);
                me->AddAura(SPELL_SLIPSTREAM_PLAYER_VISUAL, who);

                // if we use the motion master only to relocate the player
                // it will cause bugs
                if (who->GetOrientation() != SlipstreamPositions[SlipstreamPosition].GetOrientation())
                    who->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TURNING);

                me->GetMap()->PlayerRelocation(who->ToPlayer(), SlipstreamPositions[SlipstreamPosition].GetPositionX(), SlipstreamPositions[SlipstreamPosition].GetPositionY(), SlipstreamPositions[SlipstreamPosition].GetPositionZ(), SlipstreamPositions[SlipstreamPosition].GetOrientation());

                who->GetMotionMaster()->MoveJump(SlipstreamPositions[SlipstreamPosition].GetPositionX(), SlipstreamPositions[SlipstreamPosition].GetPositionY(), 198.458481f, 1, 6);
            }
        }

        void UpdateAI(uint32 diff)
        {
            // The Slipstreams are Deactivated before each Ultimate ability

            if (linkedSlipstreamObject && linkedBoss)
            {

            }
            
            else if (isUltimate)
                isUltimate = false;    

            if (!isUltimate != isActive)
            {
                if (isActive)
                {
                    // Activate Slipstream
                    if (linkedSlipstreamObject)
                        linkedSlipstreamObject->SetPhaseMask(PHASEMASK_NORMAL, true);
                }

                else
                {
                    // Deactivate Slipstream
                    if (linkedSlipstreamObject)
                        linkedSlipstreamObject->SetPhaseMask(2, true);
                }

                isActive = !isActive;
            }
        }
    };
};

//6281
class at_throne_four_winds_abyss : public AreaTriggerScript
{
public:
    at_throne_four_winds_abyss() : AreaTriggerScript("at_throne_four_winds_abyss") {}

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool enter)
    {
        pPlayer->NearTeleportTo(-310.46f, 816.64f, 199.5f, 0.10f);

        return true;
    }
};

void AddSC_throne_of_the_four_winds()
{
    new npc_slipstream_raid();
    new at_throne_four_winds_abyss();
}