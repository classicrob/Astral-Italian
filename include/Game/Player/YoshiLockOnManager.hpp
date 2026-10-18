#pragma once

#include "Game/NameObj/NameObj.hpp"

class LiveActor;

class YoshiLockOnManager : public NameObj {
public:
    void fn_8041A0B0();
    void fn_8041E120(LiveActor*);
    void fn_8041E140();
    u32 _14;
    LiveActor* mContextActor;
    bool mContextActive;
    bool _1D;
    bool _1E;
};
