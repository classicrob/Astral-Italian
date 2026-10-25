#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SpinDriverCamera;
class SpinDriverShootPath;
class ScenarioTitle;
class SimpleLayout;

class ScenarioStarter : public LiveActor {
public:
    ScenarioStarter(const char*);

    virtual ~ScenarioStarter();
    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initShootPath(const JMapInfoIter&);
    void exeWaitScenarioCameraEnd();
    void exeWaitToStart();
    void exeCinemaFrameBlank();
    void exeRailMove();
    void exeRailMoveCanceled();
    void exeShowWelcomeLayout();
    void isStartBgmOnWelcome();
    void getStartPos(TVec3f*);
    bool isFollowValid(s32) const;
    void calcShootMotionTime();
    void turnBindHead(const TVec3f&, f32);
    void updateBindPosition();
    void updateShootMotion();
    bool trySkipTrigger() const;
    void updateBindActorMtx();

    /* 0x08C */ LiveActor* mBindActor;
    /* 0x090 */ TVec3f mHeadDir;
    /* 0x09C */ TVec3f mPathOffset;
    /* 0x0A8 */ TQuat4f mBindQuat;
    /* 0x0BC */ TPos3f mFollowMtx;
    /* 0x0EC */ SpinDriverShootPath* mShootPath;
    /* 0x0BC */ SpinDriverCamera* mSpinDriverCamera;
    /* 0x0C0 */ TVec3f mStartUp;
    /* 0x0CC */ TVec3f mStartFront;
    /* 0x0D8 */ TVec3f mStartSide;
    /* 0x0E4 */ f32 mRollY;
    /* 0x0E8 */ f32 mRollYTotal;
    /* 0x0EC */ f32 mPitchX;
    /* 0x0F0 */ s32 mFlightTime;
    /* 0x0F4 */ s32 mFlyMotionStartStep;
    /* 0x0F8 */ s32 mFlyMotionEndStep;
    /* 0x0FC */ s32 mFallMotionStartStep;
    /* 0x100 */ f32 mInitalPlayerRotation;
    /* 0x104 */ SimpleLayout* mWelcomeLayout;
    /* 0x108 */ ScenarioTitle* mTitle;
    /* 0x10C */ TVec3f mStartPlayerPos;
    /* 0x118 */ TVec3f mStartPlayerRotate;
    /* 0x158 */ bool mIsCinemaFrameStopped;
    /* 0x159 */ bool mFirstIslandFleetVisit;
};
