/*
*/

struct npc_great_gnomeregan_race_gate : ScriptedAI
{
    npc_great_gnomeregan_race_gate(Creature* creature) : ScriptedAI(creature) {}

    std::set<ObjectGuid> PlayerList;

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || !who->IsPlayer())
            return;

        if (me->GetDistance(who) > 7.0f)
            return;

        if (who->ToPlayer()->GetQuestStatus(47709) == QUEST_STATUS_NONE)
            return;

        if (who->ToPlayer()->GetQuestStatus(47709) == QUEST_STATUS_COMPLETE)
            return;

        if (PlayerList.find(who->GetGUID()) == PlayerList.end())
        {
            PlayerList.insert(who->GetGUID());
            DoCast(246069);
            me->CastSpell(who, 245989, true);
        }
    }
};

struct npc_great_gnomeregan_race_gate_start : ScriptedAI
{
    npc_great_gnomeregan_race_gate_start(Creature* creature) : ScriptedAI(creature) {}

    std::set<ObjectGuid> PlayerList;

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || !who->IsPlayer())
            return;

        if (me->GetDistance(who) > 7.0f)
            return;

        if (who->ToPlayer()->GetQuestStatus(47709) == QUEST_STATUS_NONE)
            return;

        if (who->ToPlayer()->GetQuestStatus(47709) == QUEST_STATUS_COMPLETE)
            return;

        if (PlayerList.find(who->GetGUID()) == PlayerList.end())
        {
            PlayerList.insert(who->GetGUID());
            DoCast(246070);
            me->CastSpell(who, 245999, true);
        }
    }
};

struct npc_great_gnomeregan_race_gate_finish : ScriptedAI
{
    npc_great_gnomeregan_race_gate_finish(Creature* creature) : ScriptedAI(creature) {}

    std::set<ObjectGuid> PlayerList;

    void MoveInLineOfSight(Unit* who) override
    {
        if (!who || !who->IsPlayer())
            return;

        if (me->GetDistance(who) > 7.0f)
            return;

        if (who->ToPlayer()->GetQuestStatus(47709) == QUEST_STATUS_NONE)
            return;

        if (who->ToPlayer()->GetQuestStatus(47709) == QUEST_STATUS_COMPLETE)
            return;

        if (PlayerList.find(who->GetGUID()) == PlayerList.end())
        {
            PlayerList.insert(who->GetGUID());
            DoCast(246070);
            me->CastSpell(who, 246000, true);
        }
    }
};

void AddSC_TheGreatGromereganRun()
{
    RegisterCreatureAI(npc_great_gnomeregan_race_gate);
    RegisterCreatureAI(npc_great_gnomeregan_race_gate_start);
    RegisterCreatureAI(npc_great_gnomeregan_race_gate_finish);
}