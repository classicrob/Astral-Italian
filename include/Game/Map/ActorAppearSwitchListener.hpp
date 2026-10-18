#pragma once

#include "Game/Map/SwitchEventListener.hpp"

class LiveActor;

class ActorAppearSwitchListener : public SwitchEventListener {
public:
    ActorAppearSwitchListener(LiveActor*, bool, bool);

    virtual void listenSwitchOnEvent();
    virtual void listenSwitchOffEvent();

    /* 0x4 */ LiveActor* mActor;
    /* 0x8 */ bool mUsesOn;
    /* 0x9 */ bool mUsesOff;
};
