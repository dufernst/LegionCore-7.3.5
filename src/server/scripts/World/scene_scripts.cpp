class sceneTrigger_deathwing_simulator : public SceneTriggerScript
{
public:
    sceneTrigger_deathwing_simulator() : SceneTriggerScript("sceneTrigger_deathwing_simulator") {}

    bool OnTrigger(Player* player, SpellScene const* trigger, std::string type) override
    {
        if (type == "BURN PLAYER")
            player->CastSpell(player, 201184, false);

        return true;
    }
};

class sceneTrigger_circuit_game : public SceneTriggerScript
{
public:
    sceneTrigger_circuit_game() : SceneTriggerScript("sceneTrigger_circuit_game") {}

    bool OnTrigger(Player* player, SpellScene const* trigger, std::string type) override
    {
        if (type == "achievement")
            player->CastSpell(player, 210636, true);

        if (type == "stage")
            if (auto aura = player->GetAuraEffect(200015, EFFECT_1))
                aura->SetAmount(aura->GetAmount() + 1);

        return true;
    }
};

class sceneTrigger_karabor_bombing_run : public SceneTriggerScript
{
public:
    sceneTrigger_karabor_bombing_run() : SceneTriggerScript("sceneTrigger_karabor_bombing_run") {}

    bool OnTrigger(Player* player, SpellScene const* trigger, std::string type) override
    {
        if (type == "Credit")
            player->KilledMonsterCredit(72458);

        if (type == "Teleport")
            player->CastSpell(player, 165484, true);

        return true;
    }
};

void AddSC_scene_scripts()
{
    new sceneTrigger_deathwing_simulator();
    new sceneTrigger_circuit_game();
    //new sceneTrigger_karabor_bombing_run();
}

