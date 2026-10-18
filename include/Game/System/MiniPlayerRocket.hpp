#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// Rocket the player rides on the world map (0xEC bytes, ctor 0x804E4530).
class MiniPlayerRocket : public LiveActor {
public:
    MiniPlayerRocket();

    virtual ~MiniPlayerRocket();
    virtual void init(const JMapInfoIter&);

    // PLACEHOLDER(core): provisional names of the unnamed rocket methods called by WorldMapOwner; resolve when the MiniPlayerRocket unit is written
    void resetAction();                                  // fn_804E4870
    bool changeAction(const char* pName);                // fn_804E48B0
    void setMoveDirection(const TVec3f& rDir);           // fn_804E4EA0
    void setMoveDirectionInstant(const TVec3f& rDir);    // fn_804E4F10
    void startBounce();                                  // fn_804E4FB0

    /* 0x90 */ u8 _90[4];
    /* 0x94 */ f32 mMoveSpeed;
    /* 0x98 */ u8 _98[0x54];
};
