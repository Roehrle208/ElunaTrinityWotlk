#include "MxbManager.h"
#include "WorldSession.h"

class MxbCharacterCreateInfo : protected CharacterCreateInfo
{
    friend class MxbManager;
};
