#pragma once

#include "Game/NameObj/NameObj.hpp"

class LiveActor;
class YoshiLockOnTarget;

class YoshiLockOnManager : public NameObj {
public:
    bool fn_80419F90(YoshiLockOnTarget*);
    bool fn_80419FB0(YoshiLockOnTarget*);
    bool fn_80419FD0(YoshiLockOnTarget*, bool);
    bool fn_80419FF0(YoshiLockOnTarget*);
    bool fn_8041A000(YoshiLockOnTarget*);
    void fn_8041A0B0();
    void fn_8041E120(LiveActor*);
    void fn_8041E140();
    u32 _14;
    LiveActor* mContextActor;
    bool mContextActive;
    bool _1D;
    bool _1E;
};
