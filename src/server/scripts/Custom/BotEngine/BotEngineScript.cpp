#include "Chat.h"
#include "ChatCommand.h"
#include "ObjectAccessor.h"
#include "WorldSession.h"
#include "World.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Trinity::ChatCommands;

class BotEngineCommands : public CommandScript
{
public:
    BotEngineCommands() : CommandScript("BotEngineCommands") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable botCommandTable = 
        {
            { "login", HandleLoginCommand, rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
        };
        static ChatCommandTable commandTable = 
        {
            { "bot", botCommandTable },
        };
        return commandTable;
    }

    static bool HandleLoginCommand(ChatHandler* handler, const char* args)
    {
        char* accountStr = strtok((char*)args, " ");
        char* charStr = strtok(nullptr, " ");

        if (!accountStr || !charStr)
        {
            handler->SendSysMessage(".bot login <accountId> <characterId>");
            return false;
        }

        uint32 accountId = atoi(accountStr);
        uint32 characterId = atoi(charStr);

        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(characterId);

        if (Player* existing = ObjectAccessor::FindConnectedPlayer(guid))
        {
            handler->PSendSysMessage("Bot %s (ID: %u) ist bereits eingeloggt.", existing->GetName(), characterId);
            return false;
        }

        WorldSession* session = sWorld->FindSession(accountId);
        if (!session)
        {
            session = new WorldSession(accountId, "bot", nullptr, SEC_PLAYER, 0, 0, Minutes(0), LOCALE_enUS, 0, false);
            sWorld->AddSession(session);
        }

        session->IsBot(true);
        session->HandleBotLogin(guid);

        handler->PSendSysMessage("Bot mit Account %u und CharId %u wurde eingeloggt.", accountId, characterId);
        return true;
    }
};

void AddBotEngineScripts()
{
    new BotEngineCommands();
}
