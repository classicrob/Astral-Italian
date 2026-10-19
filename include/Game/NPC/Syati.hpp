#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class LiveActorGroup;
class PlayerPoseSetterInWater;
class RaceManagerLayout;
class TalkMessageCtrl;
struct JointControllerInfo;

template < class T >
class JointControlDelegator;

class Syati : public LiveActor {
public:
    Syati(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeFadeoutBeforeTalk();
    void exeWaitBlank();
    void exeFadeinBeforeTalk();
    void exeTalkStartMission();
    void exeReadyToStart();
    void exeCountDown();
    void exeSwim();
    void exeEmitRing();
    void exeWaitStarAppeared();
    void exeReachToEnd();
    void exeWaitAllRingDisappear();
    void exeTalkRetryMission();
    void exeForceKill();
    void exeHideOnShore();
    void exeWaitOnShore();
    void exeWaitTalkNormal();
    void exeTalkNormal();
    void exeStart();
    void initRings(const JMapInfoIter& rIter);
    void initPose();
    void initTalking(const JMapInfoIter& rIter);
    void updateSwimCommon();
    void updatePoseByRail();
    void updateNumRingPassed();
    void updateBlink();
    bool isReadyToEmitRing() const;
    void syncNumRingLeftToActiveRings();
    void resetScore();
    void killAllRings();
    void emitRing();
    void setupBalloonFollowMtx(const TVec3f& rVec);
    bool calcHeadJoint(TPos3f* pPos, const JointControllerInfo& rInfo);

    /* 0x90 */ TQuat4f mOrientationQuat;
    /* 0xA0 */ TVec3f mSmoothedDirection;
    /* 0xAC */ f32 mClippingRange;
    /* 0xB0 */ TVec3f mClippingVec;
    /* 0xBC */ TalkMessageCtrl* mTalkMessageCtrl;
    /* 0xC0 */ ActorCameraInfo* mActorCameraInfo;
    /* 0xC4 */ s32 mHideOnShoreMode;
    /* 0xC8 */ PlayerPoseSetterInWater* mPlayerPoseSetterInWater;
    /* 0xCC */ s32 mBlinkTimer;
    /* 0xD0 */ TVec3f mMarioMoveLocalOffsetVec;
    /* 0xDC */ TPos3f mBalloonFollowMtx;
    /* 0x10C */ JointControlDelegator< Syati >* mJointControlDelegator;
    /* 0x110 */ TMtx34f mUnusedMtx;
    /* 0x140 */ RaceManagerLayout* mRaceManagerLayout;
    /* 0x144 */ LiveActorGroup* mPrizeRingGroup;
    /* 0x148 */ s32 mNumRings;
    /* 0x14C */ s32 mStarSpawnType;
    /* 0x150 */ s32 mSwimMode;
    /* 0x154 */ s32 mPrizeRingLife;
    /* 0x158 */ s32 mPrizeRingCount;
    /* 0x15C */ s32 mCurrentRailPointNo;
};

class PlayerPoseSetterInWater {
public:
    inline PlayerPoseSetterInWater(const TVec3f& rVec, Syati* pSyati);
    void update();

    /* 0x00 */ TVec3f mPosition;
    /* 0x0C */ TQuat4f mOrientationQuat;
    /* 0x1C */ s32 mPitchFrame;
    /* 0x20 */ Vec* mLocalOffset;
};
