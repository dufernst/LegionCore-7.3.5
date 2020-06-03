
#include "ScriptMgr.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"

class battlepay_commandscript : public CommandScript
{
public:
    battlepay_commandscript() : CommandScript("battlepay_commandscript") { }

    ChatCommand* GetCommands() const override
    {
        static ChatCommand BattlepayCommandTable[] =
        {
            { "reload",                 SEC_ADMINISTRATOR,  false,  &HandleReloadBattlePay,         "", nullptr },
            { nullptr,                  0,                  false,  nullptr,                        "", nullptr }
        };

        static ChatCommand CommandTable[] =
        {
             { "battlepay",             SEC_ADMINISTRATOR,  true,   nullptr,                        "", BattlepayCommandTable },
             { nullptr,                 0,                  false,  nullptr,                        "", nullptr }
        };

        return CommandTable;
    }

    static bool HandleReloadBattlePay(ChatHandler* /*chatHandler*/, char const* /*args*/)
    {
        //sBattlepayMgr->();
        sBattlePayDataStore->Initialize();
        return true;
    }
};

void AddSC_battlepay_commandscript()
{
    new battlepay_commandscript();
}
