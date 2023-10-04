#include "Chat.h"
#include "ChatCommand.h"
#include "CharacterCache.h"
#include "DatabaseEnv.h"
#include "Group.h"
#include "Log.h"
#include "Loot.h"
#include "LFGMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ObjectAccessor.h" 
#include "MxbCommon.h" 
#include "MxbManager.h"
#include "World.h"
#include "WorldSession.h"

using namespace Trinity::ChatCommands;

class mxb_commands : public CommandScript
{
public:
    mxb_commands() : CommandScript("mxb_commands") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable mxbCommandTable =
        {
            { "load",               HandleLoadCmd              , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "loadset",            HandleLoadSetCmd           , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "unload",             HandleUnloadCmd            , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "unloadset",          HandleUnloadSetCmd         , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "create",             HandleCreateCmd            , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "login",              HandleLoginCmd             , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "telep",              HandleTeleCmd              , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "logout",             HandleLogoutCmd            , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
            { "delete",             HandleDeleteCmd            , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },   
            { "setglyphs",          HandleSetGlyphsCmd         , rbac::RBAC_ROLE_PLAYER       , Console::Yes },
            { "lfgSetRoles",        HandleLfgSetRolesCmd       , rbac::RBAC_ROLE_PLAYER       , Console::Yes },
            { "lfgAcceptProposal",  HandleLfgAcceptProposalCmd , rbac::RBAC_ROLE_PLAYER       , Console::Yes },
            { "lfgSetMasterLooter", HandleLfgSetMasterLooter   , rbac::RBAC_ROLE_ADMINISTRATOR, Console::Yes },
        };
        static ChatCommandTable commandTable =
        {
            { "mxb", mxbCommandTable },
        };
        return commandTable;
    }

    static bool HandleLoadCmd(ChatHandler* handler, const char* args)
    {
        // e.g. create 10 Bots for Naxx 10 run
        if (!*args) {
            loadNaxx10();
            sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate);
            return true;
        }

        BotStruct* botStruct;
        MxbUpdateTask* task;
        char* type_str = strtok((char*)args, " ");
        uint32 type = atoi(type_str ? type_str : args);

        switch (type) {
        case 900:
            botStruct = new BotStruct(CLASS_WARRIOR, RACE_HUMAN, 0);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_MAGE, RACE_GNOME, 1);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            break;
        case 1000:
            // 10er Raid (Tanks: Warrior, Paladin, Healer: Druid, Priest, Shaman, DPS: /*Warrior*/, Paladin, Hunter, Warlock, Mage
            botStruct = new BotStruct(CLASS_WARRIOR, RACE_NIGHTELF, 0);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_PALADIN, RACE_HUMAN, 1);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_DRUID, RACE_NIGHTELF, 2);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_PRIEST, RACE_GNOME, 3);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_SHAMAN, RACE_DRAENEI, 4);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_PALADIN, RACE_HUMAN, 5);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_HUNTER, RACE_DWARF, 6);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_WARLOCK, RACE_GNOME, 7);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            botStruct = new BotStruct(CLASS_MAGE, RACE_GNOME, 8);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            break;
        default:
            return false;
        }

        sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate);
        return true;
    }

    /*
     * command: .mxb loadset botsequence [...botsequence ...]
     * botsequences:
     *   - generic bot syntax: "gb:{class_id}:{race_id}"
     *   - player  bot syntax: "pb:{character_id}"
     */
    static bool HandleLoadSetCmd(ChatHandler* handler, const char* args)
    {
        if (!*args) {
            return false;
        }

        char* sets_str = strtok((char*)args, " ");
        std::vector<char*> botSequences;
        do {
            botSequences.push_back(!sets_str ? (char*)args : sets_str);
        } while(sets_str = strtok(NULL, " "));

        BotStruct* botStruct;
        MxbUpdateTask* task;
        uint8 gbNameIndex = 0;
        uint8 botClass = 0;
        uint8 botRace = 0;
        std::string botName;
        uint8 pbCharacterId = 0;
        char* token;
        std::string token_str;

        std::vector<MxbUpdateTask*> tasks;
        for (auto &seq : botSequences) {
            token = strtok(seq, ":");
            if (!token)
                return false;

            token_str = token;
            if (token_str == "gb") {
                token = strtok(NULL, ":");
                if (!token)
                    return false;
                botClass = atoi(token);
                token = strtok(NULL, ":");
                botRace = atoi(token);
                if (!botRace)
                    return false;

                botStruct = new BotStruct(botClass, botRace, gbNameIndex++);
                task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            }
            else if (token_str == "pb") {
                token = strtok(NULL, ":");
                if (!token)
                    return false;
                pbCharacterId = atoi(token);
                if (!pbCharacterId)
                    return false;
                task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, pbCharacterId, false);
            }
            tasks.push_back(task);
        }

        for (auto t : tasks) {
            sMxbMgr->AddWorldUpdateTask(t);
        }
        sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate);
        return true;
    }

    static bool HandleUnloadCmd(ChatHandler* handler, const char* args) // funktioniert nur im World-Update-Loop
    {
        // e.g. logout and delete 10 Bots for Naxx 10 run
        if (!*args) {
            unloadNaxx10();
            sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate);
            return true;
        }

        BotStruct* botStruct;
        MxbUpdateTask* task;
        char* type_str = strtok((char*)args, " ");
        uint32 type = atoi(type_str ? type_str : args);

        switch (type) {
        case 999:
            botStruct = new BotStruct(CLASS_WARRIOR, RACE_HUMAN, 0);
            task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_DESPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
            break;
        default:
            return false;
        }

        sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate);
        return true;
    }

    /*
     * command: .mxb unloadset botsequence [...botsequence ...]
     * botsequences:
     *   - generic bot syntax: "gb:{character_id}"
     *   - player  bot syntax: "pb:{character_id}"
     */
    static bool HandleUnloadSetCmd(ChatHandler* handler, const char* args)
    {
        if (!*args) {
            return false;
        }

        char* sets_str = strtok((char*)args, " ");
        std::vector<char*> botSequences;
        do {
            botSequences.push_back(!sets_str ? (char*)args : sets_str);
        } while (sets_str = strtok(NULL, " "));

        MxbUpdateTask* task;
        uint8 pbCharacterId = 0;
        char* token;
        std::string token_str;

        std::vector<MxbUpdateTask*> tasks;
        for (auto& seq : botSequences) {
            token = strtok(seq, ":");
            if (!token)
                return false;

            token_str = token;
            if (token_str == "gb") {
                token = strtok(NULL, ":");
                if (!token)
                    return false;
                pbCharacterId = atoi(token);
                if (!pbCharacterId)
                    return false;
                task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_DESPAWN, pbCharacterId, true);
            }
            else if (token_str == "pb") {
                token = strtok(NULL, ":");
                if (!token)
                    return false;
                pbCharacterId = atoi(token);
                if (!pbCharacterId)
                    return false;
                task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_DESPAWN, pbCharacterId, false);
            }
            tasks.push_back(task);
        }

        for (auto t : tasks) {
            sMxbMgr->AddWorldUpdateTask(t);
        }
        sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate);
        return true;
    }

    static bool HandleCreateCmd(ChatHandler* handler, const char* args)
    {
        sMxbMgr->CreateBot("botone", Classes::CLASS_WARRIOR, Races::RACE_NIGHTELF);
        return true;
    }

    static bool HandleLoginCmd(ChatHandler* handler, const char* args)
    {
        sMxbMgr->LoginBot("botone");

        return true;
    }

    static bool HandleTeleCmd(ChatHandler* handler, const char* args)
    {
        Player* bot;
        ObjectGuid botGuid;
        uint32 botCharacterId = 172;
        for (botCharacterId = 76; botCharacterId <= 85; botCharacterId++) {
            botGuid = ObjectGuid(HighGuid::Player, botCharacterId);
            if (!(bot = ObjectAccessor::FindConnectedPlayer(botGuid))) {
                continue;
            }

            Player* owner = handler->GetSession()->GetPlayer();
            bot->TeleportTo(owner->GetMap()->GetId(), owner->GetPositionX() + rand() % 10, owner->GetPositionY() + rand() % 10, owner->GetPositionZ(), TELE_REVIVE_AT_TELEPORT);
        }

        return true;
    }

    static bool HandleLogoutCmd(ChatHandler* handler, const char* args)
    {
        sMxbMgr->LogoutBot("botone");
        return true;
    }

    static bool HandleDeleteCmd(ChatHandler* handler, const char* args)
    {
        sMxbMgr->DeleteBot("botone"); 
        return true;
    }

    /*
     * command: .mxb setglyphs botGuid majorGlyph1Id majorGlyph2Id majorGlyph3Id minorGlyph1Id minorGlyph2Id minorGlyph3Id
     * @todo: ersetzen durch lua-call (@see SetUInt32Value(PLAYER_FIELD_GLYPHS_1 + slot, glyph))
     */
    static bool HandleSetGlyphsCmd(ChatHandler* handler, const char* args)
    {
        // load bot
        char* bot_guid_str = !*args ? nullptr : strtok((char*)args, " ");
        uint32 botCharacterId = atoi(bot_guid_str);
        ObjectGuid botGuid = ObjectGuid(HighGuid::Player, botCharacterId);
        Player* bot = ObjectAccessor::FindConnectedPlayer(botGuid);

        if (!bot || !*args)
            return false;

        // collect glyph-ids
        char* glyph_ids_str;
        std::vector<int> glyphIds;
        while (glyph_ids_str = strtok(NULL, " ")) {
            glyphIds.push_back(atoi(!glyph_ids_str ? args : glyph_ids_str));
        }

        if (glyphIds.size() > 6)
            return false;
         
        uint8 slot = 0;
        for (auto glyphId : glyphIds) { 
            bot->SetGlyph(slot++, glyphId);
        }
        bot->SaveToDB();

        return true;
    }

    /*
     * command: .mxb lfgSetRoles guid1:roles [guid2:roles [guid3:roles [guid4:roles]]]
     */
    static bool HandleLfgSetRolesCmd(ChatHandler* handler, const char* args)
    {
        if (!*args) {
            return false;
        }

        ObjectGuid gguid = handler->GetSession()->GetPlayer()->GetGroup()->GetGUID();

        // proposal id
        char* tokens_str = strtok((char*)args, " ");

        // bot guid(s)
        std::vector<char*> botSequences;
        do {
            botSequences.push_back(!tokens_str ? (char*)args : tokens_str);
        } while (tokens_str = strtok(NULL, " "));

        // accept invitation for all bots 
        uint32 botLowGuid;
        ObjectGuid botGuid;
        char* token;
        std::string token_str;
        uint8 roles;

        for (auto& seq : botSequences) {

            token = strtok(seq, ":");
            if (!token)
                return false;

            botLowGuid = atoi(token);
            botGuid = ObjectGuid(HighGuid::Player, botLowGuid);
            token = strtok(NULL, ":");
            roles = atoi(token);
            sLFGMgr->UpdateRoleCheck(gguid, botGuid, roles);
        }

        return true;
    }

    /*
     * command: .mxb lfgSetMasterLooter guid1
     */
    static bool HandleLfgSetMasterLooter(ChatHandler* handler, const char* args)
    {
        if (!*args) {
            return false;
        }

        Group* group = handler->GetSession()->GetPlayer()->GetGroup();
        if (!group->IsLeader(handler->GetSession()->GetPlayer()->GetGUID()))
            return false;

        // proposal id
        char* tokens_str = strtok((char*)args, " ");
        uint32 lootMasterGuid = atoi(tokens_str);
        ObjectGuid lootMaster = ObjectGuid(HighGuid::Player, lootMasterGuid);

        group->SetLootMethod(MASTER_LOOT);
        group->SetMasterLooterGuid(lootMaster);
        group->SendUpdate();

        return true;
    }

    /*
     * command: .mxb lfgAcceptProposal proposalId guid1 [guid2 [guid3 [guid4]]]
     */
    static bool HandleLfgAcceptProposalCmd(ChatHandler* handler, const char* args)
    {
        if (!*args) {
            return false;
        }

        // proposal id
        char* tokens_str = strtok((char*)args, " ");
        uint32 proposalId = atoi(tokens_str);

        // bot guid(s)
        tokens_str = strtok(NULL, " ");
        std::vector<char*> botGuids;
        do {
            botGuids.push_back(!tokens_str ? (char*)args : tokens_str);
        } while (tokens_str = strtok(NULL, " "));

        // accept invitation for all bots 
        uint32 botLowGuid;
        ObjectGuid botGuid;
        for (auto& botGuidStr : botGuids) {
            botLowGuid = atoi(botGuidStr);
            botGuid = ObjectGuid(HighGuid::Player, botLowGuid);
            sLFGMgr->UpdateProposal(proposalId, botGuid, true);
        }

        return true;
    }

private:
    static BotStructCollection createBotStructCollection(uint32 tanks, uint32 otanks, uint32 healers, uint32 mages, uint32 rangeDps, uint32 rdmDps)
    {
        BotStructCollection collection;
        uint32 nameIndex = 0, i, randomNumber, shuffleClass, shuffleRace;
        
        for (i = 0; i < tanks; i++) {
            collection.push_back(new BotStruct(CLASS_WARRIOR, RACE_HUMAN, nameIndex++));
        }
        for (i = 0; i < otanks; i++) {
            collection.push_back(new BotStruct(CLASS_PALADIN, RACE_HUMAN, nameIndex++));
        }
        for (i = 0; i < healers; i++) {
            randomNumber = (rand() % 4) + 1;
            switch (randomNumber) {
            case 1:
                shuffleClass = CLASS_SHAMAN;
                shuffleRace = RACE_DRAENEI;
                break;
            case 2:
                shuffleClass = CLASS_DRUID;
                shuffleRace = RACE_NIGHTELF;
                break;
            case 3:
                shuffleClass = CLASS_PRIEST;
                shuffleRace = RACE_DRAENEI;
                break;
            case 4:
                shuffleClass = CLASS_PALADIN;
                shuffleRace = RACE_DWARF;
                break;
            }
            collection.push_back(new BotStruct(shuffleClass, shuffleRace, nameIndex++));
        }
        for (i = 0; i < mages; i++) {
            collection.push_back(new BotStruct(CLASS_MAGE, RACE_GNOME, nameIndex++));
        }
        for (i = 0; i < rangeDps; i++) {
            randomNumber = (rand() % 2) + 1;
            switch (randomNumber) {
            case 1:
                shuffleClass = CLASS_HUNTER;
                shuffleRace = RACE_DRAENEI;
                break;
            case 2:
                shuffleClass = CLASS_WARLOCK;
                shuffleRace = RACE_GNOME;
                break;
            }
            collection.push_back(new BotStruct(shuffleClass, shuffleRace, nameIndex++));
        }
        for (i = 0; i < rdmDps; i++) {
            randomNumber = (rand() % 10) + 1;
            switch (randomNumber) {
            case 1:
                shuffleClass = CLASS_ROGUE;
                shuffleRace = RACE_HUMAN;
                break;
            case 2:
                shuffleClass = CLASS_DEATH_KNIGHT;
                shuffleRace = RACE_HUMAN;
                break;
            case 3:
                shuffleClass = CLASS_WARRIOR;
                shuffleRace = RACE_HUMAN;
                break;
            case 4:
                shuffleClass = CLASS_SHAMAN;
                shuffleRace = RACE_DRAENEI;
                break;
            case 5:
                shuffleClass = CLASS_DRUID;
                shuffleRace = RACE_NIGHTELF;
                break;
            case 6:
                shuffleClass = CLASS_MAGE;
                shuffleRace = RACE_HUMAN;
                break;
            case 7:
                shuffleClass = CLASS_HUNTER;
                shuffleRace = RACE_DRAENEI;
                break;
            case 8:
                shuffleClass = CLASS_PRIEST;
                shuffleRace = RACE_DRAENEI;
                break;
            case 9:
                shuffleClass = CLASS_WARLOCK;
                shuffleRace = RACE_GNOME;
                break;
            case 10:
                shuffleClass = CLASS_PALADIN;
                shuffleRace = RACE_DWARF;
                break;
            }
            collection.push_back(new BotStruct(shuffleClass, shuffleRace, nameIndex++));
        }
        return collection;
    }

    static void loadNaxx10(void) {
        BotStructCollection::iterator it;
        BotStructCollection botStructCollection = createBotStructCollection(1, 1, 3, 1, 1, 3);
        BotStruct* botStruct;
        for (it = botStructCollection.begin(); it != botStructCollection.end(); ++it) {
            botStruct = (*it);
            MxbUpdateTask* task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
        }
    }

    static void unloadNaxx10(void) {
        BotStructCollection::iterator it;
        BotStructCollection botStructCollection = createBotStructCollection(1, 1, 3, 1, 1, 3);
        BotStruct* botStruct;
        for (it = botStructCollection.begin(); it != botStructCollection.end(); ++it) {
            botStruct = (*it);
            MxbUpdateTask* task = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_DESPAWN, botStruct->__name, botStruct->__race, botStruct->__class);
            sMxbMgr->AddWorldUpdateTask(task);
        }
    }
};

void AddSC_MaxBots_Commands()
{
    new mxb_commands();
    //sWorld->AddPostUpdateCallback(sMxbMgr, &MxbManager::OnWorldUpdate); // Server stürzt ab, weil DLL durch Hot-Swap neu verlinkt wird und die Callback dann nicht mehr existiert
}
