#include "Chat.h"
#include "ChatCommand.h"
#include "ObjectAccessor.h"
#include "WorldSession.h"
#include "World.h"
#include "Player.h"
#include "ScriptMgr.h" 
#include "WowheadTalentEngine.h" 

using namespace Trinity::ChatCommands;

class GearPlannerCommands : public CommandScript
{
public:
    GearPlannerCommands() : CommandScript("GearPlannerCommands") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable gearPlannerCommandTable = 
        {
            { "pdump", HandleGearPlannerDumpPlayerCommand, rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
        };
        static ChatCommandTable commandTable = 
        {
            { "gearplanner", gearPlannerCommandTable },
        };
        return commandTable;
    }

    static bool HandleGearPlannerDumpPlayerCommand(ChatHandler* handler, const char* args)
    {
        char* charStr = strtok((char*)args, " ");
        if (!charStr)
        {
            handler->SendSysMessage(".gearplanner pdump <characterId> <--force-reload>");
            return false;
        }

        uint32 characterId = atoi(charStr);
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(characterId);
        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player)
        {
            handler->PSendSysMessage("Player with ID %u is not online.", characterId);
            return true;
        }
 
        try 
        {
            const std::string& json = WowheadTalentEngine::Instance().ExportJson(player);
            handler->PSendSysMessage("json: %s", json.c_str());
        }
        catch (const std::exception& e)
        {
            handler->PSendSysMessage("Error occurred while exporting JSON: %s", e.what());
        }

        return true;
    }
};

void AddGearPlannerScripts()
{
    new GearPlannerCommands();

    // Das Initialisieren des Singletons passiert beim ersten Aufruf von Instance()
    // Oder man ruft es hier explizit auf:
    WowheadTalentEngine::Instance(); 
}