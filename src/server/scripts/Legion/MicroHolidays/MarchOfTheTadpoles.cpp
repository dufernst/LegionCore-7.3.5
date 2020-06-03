/*
*/

#include <QuestData.h>

struct npc_march_of_the_tadpoles : public ScriptedAI
{
    npc_march_of_the_tadpoles(Creature* creature) : ScriptedAI(creature) {}

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 action) override
    {
        if (action == 0)
        {
            switch (me->GetEntry())
            {
            case 118571:
                player->CastSpell(player, 235315, false);
                break;
            case 118575:
                player->CastSpell(player, 235322, false);
                break;
            case 118576:
                player->CastSpell(player, 235323, false);
                break;
            case 118577:
                player->CastSpell(player, 235324, false);
                break;
            case 118578:
                player->CastSpell(player, 235325, false);
                break;
            case 118579:
                player->CastSpell(player, 235326, false);
                break;
            case 118580:
                player->CastSpell(player, 235327, false);
                break;
            case 118581:
                player->CastSpell(player, 235328, false);
                break;
            case 118582:
                player->CastSpell(player, 235329, false);
                break;
            case 118583:
                player->CastSpell(player, 235330, false);
                break;
            case 118584:
                player->CastSpell(player, 235331, false);
                break;
            case 118585:
                player->CastSpell(player, 235332, false);
                break;
            case 118586:
                player->CastSpell(player, 235333, false);
                break;
            case 118587:
                player->CastSpell(player, 235334, false);
                break;
            case 118588:
                player->CastSpell(player, 235335, false);
                break;
            case 118589:
                player->CastSpell(player, 235336, false);
                break;
            case 118590:
                player->CastSpell(player, 235337, false);
                break;
            case 118591:
                player->CastSpell(player, 235338, false);
                break;
            case 118592:
                player->CastSpell(player, 235339, false);
                break;
            case 118594:
                player->CastSpell(player, 235341, false);
                break;
            case 118595:
                player->CastSpell(player, 235342, false);
                break;
            case 118596:
                player->CastSpell(player, 235343, false);
                break;
            case 118597:
                player->CastSpell(player, 235344, false);
                break;
            case 118598:
                player->CastSpell(player, 235345, false);
                break;
            case 118599:
                player->CastSpell(player, 235346, false);
                break;
            case 118600:
                player->CastSpell(player, 235347, false);
                break;
            case 118601:
                player->CastSpell(player, 235348, false);
                break;
            case 118602:
                player->CastSpell(player, 235349, false);
                break;
            case 118603:
                player->CastSpell(player, 235350, false);
                break;
            case 118604:
                player->CastSpell(player, 235351, false);
                break;
            case 118605:
                player->CastSpell(player, 235352, false);
                break;
            }

            player->KilledMonsterCredit(118593);
            player->CLOSE_GOSSIP_MENU();
        }
    }
};

void AddSC_MarchOfTheTadpoles()
{
    RegisterCreatureAI(npc_march_of_the_tadpoles);
}