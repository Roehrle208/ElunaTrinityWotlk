#ifndef _MXB_COMMON_H
#define _MXB_COMMON_H

const char* BotNames[] = {
    "bota",
    "botb",
    "botc",
    "botd",
    "bote",
    "botf",
    "botg",
    "both",
    "boti",
    "botj"
};

struct BotStruct
{
    BotStruct(uint32 _class, uint32 _race, uint32 _nameIndex) : __class(_class), __race(_race)
    {
        __name = BotNames[_nameIndex];
    }
    uint32 __class;
    uint32 __race;
    std::string __name;
};

typedef std::list<BotStruct*> BotStructCollection;

#endif
