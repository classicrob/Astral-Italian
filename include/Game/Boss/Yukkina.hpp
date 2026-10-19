#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class NameObj;
class Triangle;

class Yukkina : public LiveActor {
public:
    /// @brief Creates a new `Yukkina`.
    /// @param pName A pointer to the null-terminated name of the object.
    Yukkina(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isInvalidTriangle(const Triangle* pTriangle);
    bool isBindedAndHigh() __attribute__((noinline));
    void updateSound();
    void setTurnParam(s32);
    void startWaitAction();

    void exeHidden();
    void exeAppear();
    void exeAppearEnd();
    void exeWait();
    void exeBreak();
    void exeBreakWait();
    void exeJump();
    void exeJumpWait();
    void exeWaitTurn();
    void exeSwitchDead();
    void exeDemoStart();
    void exeDemoEnd();

private:
    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ TPos3f mBaseMtx;
    /* 0xC4 */ TQuat4f mPoseQuat;
    /* 0xD4 */ TQuat4f mBreakBaseQuat;
    /* 0xE4 */ TVec3f mFront;
    /* 0xF0 */ TQuat4f mInitialQuat;
    /* 0x100 */ TVec3f mInitialPos;
    /* 0x10C */ TVec3f mCameraTargetPos;
    /* 0x118 */ s32 mCameraRegisterId;
    /* 0x11C */ s32 mCounter;
    /* 0x120 */ s32 mStep;
    /* 0x124 */ f32 _124;
    /* 0x128 */ f32 _128;
    /* 0x12C */ f32 _12C;
    /* 0x130 */ f32 mLowAngle;
    /* 0x134 */ f32 mHighAngle;
    /* 0x138 */ f32 mRollSpeed;
    /* 0x13C */ s32 mIdx;
    /* 0x140 */ u8 mHasPlayerLink;
};
