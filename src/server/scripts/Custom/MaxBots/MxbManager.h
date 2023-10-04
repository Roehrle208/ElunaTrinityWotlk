#ifndef __MXB_MANAGER_H
#define __MXB_MANAGER_H

enum MxbUpdateTaskTypes : uint32
{
    BOT_SPAWN = 0,
    BOT_DESPAWN
};

class MxbUpdateTask 
{
public:
    MxbUpdateTask(uint32 type, uint32 charID, bool isGeneric) : _type(type), _charID(charID), _isGeneric(isGeneric) { };
    MxbUpdateTask(uint32 type, std::string name, uint32 race, uint32 class_) : _type(type), _name(name), _race(race), _class(class_) { };
    uint32 GetType() const { return _type; };
    bool IsGenericBot() const { return _isGeneric; };
    bool IsPlayerBot() const { return !_isGeneric; };
    // generic bots
    std::string GetName() const { return _name; };
    uint32 GetRace() const { return _race; };
    uint32 GetClass() const { return _class; };
    // player bots
    uint32 GetCharacterID() const { return _charID; };
private:
    uint32 _type;
    bool _isGeneric = true;
    // generic bots
    std::string _name;
    uint32 _race;
    uint32 _class;
    // player bots
    uint32 _charID;
};

class MxbManager {

public:
    // core hooks
    void OnWorldUpdate(uint32 diff);
    void AddWorldUpdateTask(MxbUpdateTask* task) { _worldUpdateTaskQueue.push_back(task); };
    typedef std::list<MxbUpdateTask*> WorldUpdateTaskQueue;
    // main functions
    static MxbManager* instance();
    void CreateBot(std::string botName, uint32 botCharacterClass, uint32 botCharacterRace);
    void LoginBot(std::string botName);
    void LogoutBot(std::string botName);
    void DeleteBot(std::string botName);

private:
    MxbManager() = default;
    // core hooks
    WorldUpdateTaskQueue _worldUpdateTaskQueue;
    int _checkDelay = 0;
    // main vars
    uint32 checkBotAccount(std::string botName);
    uint32 checkBotAccount(uint32 characterID);
    uint32 createBotAccount(std::string botName);
    uint32 checkBotCharacter(uint32 botAccountID, std::string botName);
    uint32 createBotCharacter(uint32 botAccountID, std::string botName, uint32 botCharacterClass, uint32 botCharacterRace);
    bool isBotLoggedIn(uint32 botCharacterID);
    Player* getLoggedInBot(uint32 botCharacterID);
    bool loginBot(uint32 botAccountID, uint32 botCharacterID);
    bool logoutBot(uint32 botCharacterID);
    void deleteBot(uint32 botAccountID, uint32 botCharacterID);
    void deleteBotAccount(uint32 botAccountID);
};

#define sMxbMgr MxbManager::instance()

#endif
