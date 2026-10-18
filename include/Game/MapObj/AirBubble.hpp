#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AirBubble : public LiveActor {
public:
    /// @brief Creates a new `AirBubble`.
    /// @param pName A pointer to the null-terminated name of the object.
    AirBubble(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void appearMove(const TVec3f&, s32);
    void exeWait();
    void exeMove();
    void exeVanish();
    void exeGather();
    void exeBreak();
    void exeKillWait();
    bool canSpinGet() const;
    bool startVanish();
    bool tryStartGather();
    bool tryStartGatherFromVanish();

    /* 0x90 */ TVec3f mAnchorPos;
    /* 0x9C */ TVec3f _9C;
    /* 0xA8 */ f32 _A8;
    /* 0xAC */ s32 mLife;
};
