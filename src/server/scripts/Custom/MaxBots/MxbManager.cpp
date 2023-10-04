#include "AccountMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerAI.h"
#include "World.h"
#include "WorldSession.h"
#include "MxbManager.h"
#include "MxbCharacterCreateInfo.h"
#include "CharacterCache.h"

/*
* MANAGER
*/

/* PUBLIC */

// core hooks

void MxbManager::OnWorldUpdate(uint32 diff)
{
    MxbUpdateTask* followUpTask = NULL;
    WorldUpdateTaskQueue followUpQueue;
    
    WorldUpdateTaskQueue::iterator it = _worldUpdateTaskQueue.begin();
    while (it != _worldUpdateTaskQueue.end())
    {
        const auto& task = (*it); 
        followUpTask = NULL;

        uint32 type = task->GetType();
        std::string botName;
        uint32 botClass, botRace, botCharacterID, botAccountID;
        ObjectGuid botGuid;
        Player* botPlayer;
        bool taskBelongsToGenericBot = task->IsGenericBot();

        switch (type) {
        case MxbUpdateTaskTypes::BOT_SPAWN:
            botName = task->GetName();
            botClass = task->GetClass();
            botRace = task->GetRace();
            botCharacterID = task->IsGenericBot() ? 0 : task->GetCharacterID();
            botAccountID = task->IsGenericBot() ? checkBotAccount(botName) : checkBotAccount(botCharacterID);

            if (taskBelongsToGenericBot) {
                // create account
                if (!botAccountID) {
                    botAccountID = createBotAccount(botName);
                }
                // create character
                botCharacterID = checkBotCharacter(botAccountID, botName);
                if (!botCharacterID) {
                    botCharacterID = createBotCharacter(botAccountID, botName, botClass, botRace);
                    followUpTask = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_SPAWN, botName, botRace, botClass);
                    break;
                }
            }
            else if (task->IsPlayerBot()) {
                botCharacterID = task->GetCharacterID();
            }
            // login character
            botGuid = ObjectGuid(HighGuid::Player, botCharacterID);
            if (!(botPlayer = ObjectAccessor::FindConnectedPlayer(botGuid))) {
                loginBot(botAccountID, botCharacterID);
            }
            break;
        case MxbUpdateTaskTypes::BOT_DESPAWN:
            botCharacterID = task->GetCharacterID();
            // logout character
            botGuid = ObjectGuid(HighGuid::Player, botCharacterID);
            if (botPlayer = ObjectAccessor::FindConnectedPlayer(botGuid)) {
                logoutBot(botCharacterID);
                if(taskBelongsToGenericBot)
                    followUpTask = new MxbUpdateTask(MxbUpdateTaskTypes::BOT_DESPAWN, botCharacterID, true);
                break;
            }
            // delete character & account if generic
            if (taskBelongsToGenericBot) {
                // delete character
                deleteBot(botAccountID, botCharacterID);
                // delete character account
                deleteBotAccount(botAccountID);
            }
            break;
        }
        if (followUpTask) {
            followUpQueue.push_back(followUpTask); 
        }

        it++; 
    }
    _worldUpdateTaskQueue.clear();

    if (followUpQueue.size()) {
        sWorld->AddPostUpdateCallback(this, &MxbManager::OnWorldUpdate, true);
        for (const auto& newTask : followUpQueue)
            _worldUpdateTaskQueue.push_back(newTask);
    }
}

// main functions

MxbManager* MxbManager::instance()
{
    static MxbManager instance;
    return &instance;
}

void MxbManager::CreateBot(std::string botName, uint32 botCharacterClass = Classes::CLASS_DRUID, uint32 botCharacterRace = Races::RACE_NIGHTELF)
{
    uint32 botAccountID = 0,
        botCharacterID = 0;
    //Player* bot;

    // check account
    botAccountID = checkBotAccount(botName);
    if (!botAccountID) {
        botAccountID = createBotAccount(botName);
    }

    // check character
    botCharacterID = checkBotCharacter(botAccountID, botName);
    if (botCharacterID) {
        return;
    }

    botCharacterID = createBotCharacter(botAccountID, botName, botCharacterClass, botCharacterRace); // @todo: inject class & race
}

void MxbManager::LoginBot(std::string botName)
{
    uint32 botAccountID = 0,
        botCharacterID = 0;
    //Player* bot;

    // check account
    botAccountID = checkBotAccount(botName);
    if (!botAccountID) {
        botAccountID = createBotAccount(botName);
    }

    // check character
    botCharacterID = checkBotCharacter(botAccountID, botName);
    if (!botCharacterID) {
        return;
    }

    if (!getLoggedInBot(botCharacterID)) {
        loginBot(botAccountID, botCharacterID);
    }
}
void MxbManager::LogoutBot(std::string botName)
{
    uint32 botAccountID = 0,
        botCharacterID = 0;
    //Player* bot;

    // check account
    botAccountID = checkBotAccount(botName);
    if (!botAccountID) {
        botAccountID = createBotAccount(botName);
    }

    // check character
    botCharacterID = checkBotCharacter(botAccountID, botName);
    if (!botCharacterID) {
        return;
    }

    logoutBot(botCharacterID);
}
void MxbManager::DeleteBot(std::string botName)
{
    uint32 botAccountID = 0,
        botCharacterID = 0;
    //Player* bot;

    // check account
    botAccountID = checkBotAccount(botName);
    if (!botAccountID) {
        botAccountID = createBotAccount(botName);
    }

    // check character
    botCharacterID = checkBotCharacter(botAccountID, botName);
    if (!botCharacterID) {
        return;
    }

    deleteBot(botAccountID, botCharacterID);
    deleteBotAccount(botAccountID);
}

/* PRIVATE */

uint32 MxbManager::checkBotAccount(std::string botName)
{
    QueryResult result = LoginDatabase.PQuery("SELECT id FROM account WHERE username = UPPER('{}')", "mxb" + botName);
    return result ? result->Fetch()[0].GetUInt32() : false;
}

uint32 MxbManager::checkBotAccount(uint32 characterID)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT account FROM characters WHERE guid={}", characterID);
    return result ? result->Fetch()[0].GetUInt32() : false;
}

uint32 MxbManager::createBotAccount(std::string botName)
{
    AccountOpResult aor = sAccountMgr->CreateAccount("mxb" + botName, "mxb");
    return checkBotAccount(botName);
}

uint32 MxbManager::checkBotCharacter(uint32 botAccountID, std::string botName)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account={} AND name='{}'", botAccountID, botName);
    return result ? result->Fetch()[0].GetUInt32() : false;
}

uint32 MxbManager::createBotCharacter(uint32 botAccountID, std::string botName, uint32 botCharacterClass, uint32 botCharacterRace)
{
    uint8 gender = urand(0, 1),
        skin = 0,
        face = urand(0, 5),
        hairStyle = urand(0, 5),
        hairColor = urand(0, 5),
        facialHair = urand(0, 5),
        outfitID = 0;

    MxbCharacterCreateInfo* bcci = new MxbCharacterCreateInfo();
    bcci->Name = botName;
    bcci->Race = botCharacterRace;
    bcci->Class = botCharacterClass;
    bcci->Gender = gender;
    bcci->Skin = skin;
    bcci->Face = face;
    bcci->HairStyle = hairStyle;
    bcci->HairColor = hairColor;
    bcci->FacialHair = facialHair;
    bcci->OutfitId = 0;
    WorldSession* session = new WorldSession(botAccountID, "mxb", NULL, SEC_PLAYER, 2, 0, LOCALE_enUS, 0, false);
    Player* bot = new Player(session);
    if (!bot->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), bcci)) {
        bot->CleanupsBeforeDelete();
        delete session;
        delete bot;
        sLog->OutMessage("mxb", LogLevel::LOG_LEVEL_ERROR, "Character create failed, %s %d %d", botName, botCharacterRace, botCharacterClass);
        sLog->OutMessage("mxb", LogLevel::LOG_LEVEL_INFO, "Try again");
        throw 2;
    }

    bot->GetMotionMaster()->Initialize();
    bot->setCinematic(2);
    bot->SetAtLoginFlag(AT_LOGIN_NONE);
    bot->SaveToDB(true);

    session->MxbIsBot(true);
    sWorld->AddSession(session);
    sLog->OutMessage("mxb", LogLevel::LOG_LEVEL_INFO, "Create character %d - %s for account %d", bot->GetGUID().GetCounter(), botName, botAccountID);

    return bot->GetGUID().GetRawValue();
}

bool MxbManager::isBotLoggedIn(uint32 botCharacterID)
{
    ObjectGuid guid = ObjectGuid(HighGuid::Player, botCharacterID);
    return ObjectAccessor::FindConnectedPlayer(guid) ? true : false;
}

Player* MxbManager::getLoggedInBot(uint32 botCharacterID)
{
    ObjectGuid guid = ObjectGuid(HighGuid::Player, botCharacterID); 
    return ObjectAccessor::FindConnectedPlayer(guid);  
}

bool MxbManager::loginBot(uint32 botAccountID, uint32 botCharacterID)
{
    ObjectGuid botGuid = ObjectGuid(HighGuid::Player, botCharacterID);
    Player* bot;
    if (bot = ObjectAccessor::FindConnectedPlayer(botGuid)) {
        sLog->OutMessage("mxb", LogLevel::LOG_LEVEL_INFO, "Bot %d %s is already in world", botCharacterID, bot->GetName());
        return false;
    }

    WorldSession* session = sWorld->FindSession(botAccountID);
    if (!session) {
        session = new WorldSession(botAccountID, "mxb", NULL, SEC_PLAYER, 2, 0, LOCALE_enUS, 0, false);
        sWorld->AddSession(session);
    }

    session->MxbIsBot(true);
    session->HandlePlayerLogin_Simple(botGuid);
    sLog->OutMessage("mxb", LogLevel::LOG_LEVEL_INFO, "Log in character %d %d", botAccountID, botCharacterID);

    return true;
}

bool MxbManager::logoutBot(uint32 botCharacterID)
{
    ObjectGuid guid = ObjectGuid(HighGuid::Player, botCharacterID);
    Player* bot = ObjectAccessor::FindConnectedPlayer(guid);
    if (bot)
    {
        if (!bot->IsAlive())
        {
            bot->ResurrectPlayer(1.0f);
            bot->SpawnCorpseBones();
        }
        bot->ClearInCombat();
        sLog->OutMessage("lfm", LogLevel::LOG_LEVEL_INFO, "Log out robot %s", bot->GetName());
        std::ostringstream msgStream;
        msgStream << bot->GetName() << " logged out";
        sWorld->SendServerMessage(ServerMessageType::SERVER_MSG_STRING, msgStream.str().c_str());
        if (WorldSession* session = bot->GetSession())
        {
            session->LogoutPlayer(true);
        }
    }

    return true;
}

void MxbManager::deleteBot(uint32 botAccountID, uint32 botCharacterID)
{
    ObjectGuid botGuid = ObjectGuid(HighGuid::Player, botCharacterID);
    if (botGuid) {
        Player::DeleteFromDB(botGuid, botAccountID, true, true);
        sLog->OutMessage("mxb", LogLevel::LOG_LEVEL_INFO, "Deleted character %d %d", botAccountID, botCharacterID);
    }

}

void MxbManager::deleteBotAccount(uint32 botAccountID)
{
    AccountOpResult aor = sAccountMgr->DeleteAccount(botAccountID);
}
