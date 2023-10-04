#ifndef _MXB_EVENTS_H
#define _MXB_EVENTS_H

#include "EventProcessor.h"

//Teleport home: near or far, only used for free bots
class TeleportHomeEvent : public BasicEvent
{
    friend class MxbAI;
protected:
    TeleportHomeEvent(MxbAI* ai) : _ai(ai) {}
    ~TeleportHomeEvent() {}

    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
    {
        _ai->TeleportHome();
        return true;
    }

private:
    MxbAI* _ai;
};
//Delayed teleport finish: adds bot back to world on new location
class TeleportFinishEvent : public BasicEvent
{
    friend class MxbAI;
protected:
    TeleportFinishEvent(MxbAI* ai) : _ai(ai) {}
    ~TeleportFinishEvent() {}

    //Execute is always called while creature is out of world so ai is never deleted
    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
    {
        _ai->FinishTeleport();
        return true;
    }

private:
    MxbAI* _ai;
};

#endif
