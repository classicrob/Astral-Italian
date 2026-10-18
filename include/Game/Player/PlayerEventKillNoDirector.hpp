#pragma once

#include "Game/Player/PlayerEvent.hpp"

class EventKillNoDirector : public EventSequence {
public:
    EventKillNoDirector();
    void showMissLayout(u16, u16);
    void init(u16, u16);
    void finish(u16, u16);
};
