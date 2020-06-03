#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "dark_portal.h"
#include "LFGMgr.h"
#include "Group.h"

enum eEnums
{
    SAY_ENTER           = 0,
    SAY_AGGRO           = 1,
    SAY_BANISH          = 2,
    SAY_SLAY            = 3,
    SAY_DEATH           = 4,
    EMOTE_FRENZY        = 5,

    SPELL_CLEAVE        = 40504,
    SPELL_TIME_STOP     = 31422,
    SPELL_ENRAGE        = 37605,
    SPELL_SAND_BREATH   = 31473,
    H_SPELL_SAND_BREATH = 39049
};

class boss_aeonus : public CreatureScript
{
public:
    boss_aeonus() : CreatureScript("boss_aeonus") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_aeonusAI (creature);
    }

    struct boss_aeonusAI : public ScriptedAI
    {
        boss_aeonusAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 SandBreath_Timer;
        uint32 TimeStop_Timer;
        uint32 Frenzy_Timer;

        void Reset()
        {
            SandBreath_Timer = 15000+rand()%15000;
            TimeStop_Timer = 10000+rand()%5000;
            Frenzy_Timer = 30000+rand()%15000;
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_AGGRO);
        }

        void MoveInLineOfSight(Unit* who)
        {
            //Despawn Time Keeper
            if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == C_TIME_KEEPER)
            {
                if (me->IsWithinDistInMap(who, 20.0f))
                {
                    Talk(SAY_BANISH);
                    me->DealDamage(who, who->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }
            }

            ScriptedAI::MoveInLineOfSight(who);
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_DEATH);

            if (instance)
            {
                instance->SetData(TYPE_RIFT, DONE);
                instance->SetData(TYPE_MEDIVH, DONE);//FIXME: later should be removed

                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                if (!players.isEmpty())
                {
                    Player* pPlayer = players.begin()->getSource();
                    if (pPlayer && pPlayer->GetGroup())
                        if (sLFGMgr->GetQueueId(744))
                            sLFGMgr->FinishDungeon(pPlayer->GetGroup()->GetGUID(), 744);
                }
            }
        }

        void KilledUnit(Unit* /*victim*/)
        {
            Talk(SAY_SLAY);
        }

        void UpdateAI(uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            //Sand Breath
            if (SandBreath_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_SAND_BREATH);
                SandBreath_Timer = 15000+rand()%10000;
            } else SandBreath_Timer -= diff;

            //Time Stop
            if (TimeStop_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_TIME_STOP);
                TimeStop_Timer = 20000+rand()%15000;
            } else TimeStop_Timer -= diff;

            //Frenzy
            if (Frenzy_Timer <= diff)
            {
                Talk(EMOTE_FRENZY);
                DoCast(me, SPELL_ENRAGE);
                Frenzy_Timer = 20000+rand()%15000;
            } else Frenzy_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };

};

void AddSC_boss_aeonus()
{
    new boss_aeonus();
}
