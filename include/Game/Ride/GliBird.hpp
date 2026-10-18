#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class ActorCameraInfo;
class ActorJointCtrl;
class BreakModel;
class CameraTargetMtx;
class GliBird;
class GliBirdGuidance;
class GliBirdNpc;
class GliBirdFootJointCtrl;
class Pichan;
class TalkMessageCtrl;
class VolumeDrawer;

// 15 pointers to tuning values (built by the unnamed constructor fn_8042EC10 outside this unit).
class GliBirdParam {
public:
    GliBirdParam(const LiveActor* pHost);

    /* 0x00 */ const f32* mOffsetFront;
    /* 0x04 */ const f32* mOffsetUp;
    /* 0x08 */ const f32* mRadius;
    /* 0x0C */ const f32* mWaterFriction;
    /* 0x10 */ const f32* mWaterLiftMin;
    /* 0x14 */ const f32* mWaterLiftRate;
    /* 0x18 */ const f32* mDeadGravity;
    /* 0x1C */ const f32* mDamageDeadBirdUpFront;
    /* 0x20 */ const f32* mDamageDeadBirdSide;
    /* 0x24 */ const f32* mDamageDeadPlayerUpFront;
    /* 0x28 */ const f32* mDamageDeadPlayerSide;
    /* 0x2C */ const f32* mWallDeadBirdUpFront;
    /* 0x30 */ const f32* mWallDeadBirdSide;
    /* 0x34 */ const f32* mWallDeadPlayerUpFront;
    /* 0x38 */ const f32* mWallDeadPlayerSide;
};

class GliBird : public LiveActor {
public:
    GliBird(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void showBird();
    bool isEnableFootJointCtrl() const;
    void exeHide();
    void exeReady();
    void exeStart();
    void exeTakeOff();
    void exeGlide();
    void exeDamage();
    void exeDeadDamage() NO_INLINE;
    void exeDeadWall() NO_INLINE;
    void exeDeadWater();
    void exeDeadLava();
    void exeDeadCommon(const f32* pBirdUpFront, const f32* pBirdSide, const f32* pPlayerUpFront, const f32* pPlayerSide);
    void damage(bool isFire);
    void deadLava();
    void restart();
    bool updateGlide(f32, f32);
    bool updateCollision();
    void reactionCollision(const TVec3f& rNormal, HitSensor* pSensor, bool isFire);
    bool tryDeadWall(const TVec3f& rNormal, HitSensor* pSensor, bool isFire);
    void touchWater(const TVec3f& rPos, f32 depth);

    /* 0x090 */ TVec3f mInitPos;
    /* 0x09C */ TVec3f mFront;
    /* 0x0A8 */ TVec3f mUp;
    /* 0x0B4 */ TVec3f mSide;
    /* 0x0C0 */ TVec3f mDirFront;
    /* 0x0CC */ TVec3f mDirUp;
    /* 0x0D8 */ f32 mSpeed;
    /* 0x0DC */ f32 mFriction;
    /* 0x0E0 */ f32 mSideAccel;
    /* 0x0E4 */ f32 mPitchAngle;
    /* 0x0E8 */ f32 mPitchAccel;
    /* 0x0EC */ f32 mYawAngle;
    /* 0x0F0 */ f32 mYawAccel;
    /* 0x0F4 */ f32 mRollAngle;
    /* 0x0F8 */ f32 mRollAccel;
    /* 0x0FC */ f32 mBlend;
    /* 0x100 */ f32 _100;
    /* 0x104 */ LiveActor* mRider;
    /* 0x108 */ s32 mDamageTimer;
    /* 0x10C */ ActorJointCtrl* mJointCtrl;
    /* 0x110 */ GliBirdFootJointCtrl* mFootJointCtrl;
    /* 0x114 */ ActorCameraInfo* mCameraInfo;
    /* 0x118 */ GliBirdParam* mParam;
    /* 0x11C */ s32 mRumbleCount;
    /* 0x120 */ bool mIsWaterEffect;
    /* 0x124 */ s32 mWaterCount;
    /* 0x128 */ TPos3f mWaterMtx;
    /* 0x158 */ BreakModel* mBirdBreakModel;
    /* 0x15C */ BreakModel* mPlayerBreakModel;
    /* 0x160 */ TPos3f mLavaBigMtx;
    /* 0x190 */ TPos3f mLavaSmallMtx;
    /* 0x1C0 */ TPos3f mLavaPlayerMtx;
    /* 0x1F0 */ LiveActor* mDeadBody;
    /* 0x1F4 */ TPos3f mPlayerDeadMtx;
    /* 0x224 */ CameraTargetMtx* mCameraTarget;
    /* 0x228 */ f32 mLastSpeed;
    /* 0x22C */ bool mIsNearGround;
    /* 0x230 */ TVec3f mGroundHitPos;
    /* 0x23C */ TVec3f mGroundSmoothPos;
    /* 0x248 */ TVec3f mGroundNormal;
    /* 0x254 */ TPos3f mApproachMtx;
    /* 0x284 */ TPos3f mVolumeMtx;
    /* 0x2B4 */ VolumeDrawer* mVolumeDrawer;
    /* 0x2B8 */ TPos3f mSpareMtx;
    /* 0x2E8 */ GliBirdNpc* mNpc;
    /* 0x2EC */ bool mIsTutorial;
    /* 0x2ED */ bool mNoRumble;
};

class GliBirdFootJointCtrl : public LiveActor {
public:
    GliBirdFootJointCtrl(GliBird* pHost);

    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAnim();

    /* 0x90 */ GliBird* mHost;
};

class GliBirdTutorial : public LiveActor {
public:
    GliBirdTutorial();

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();
    virtual void appear();
    virtual void kill();

    void registerPichan(LiveActor* pPichan);
    void exeEnd();
    void exeTalkFirst();
    void exeTalkStraight();
    void exeStraightStart();
    void exeStraightHold();
    void exeStraightSuccess();
    void exeTalkTurnLeft();
    void exeTurnLeftStart();
    void exeTurnLeftHold();
    void exeTurnLeftSuccess();
    void exeTalkTurnRight();
    void exeTurnRightStart();
    void exeTurnRightHold();
    void exeTurnRightSuccess();
    bool isOkStraight() const;
    bool isOkTurnLeft() const;
    bool isOkTurnRight() const;
    void startTimerSound(s32 step, s32 count);

    /* 0x90 */ GliBird* mGliBird;
    /* 0x94 */ LiveActor* mPichan;
    /* 0x98 */ TVec3f mPichanPos;
    /* 0xA4 */ TVec3f mPichanRot;
    /* 0xB0 */ GliBirdGuidance* mGuidance;
    /* 0xB4 */ TalkMessageCtrl* mTalkCtrl;
    /* 0xB8 */ TVec3f mPadAccel;
    /* 0xC4 */ s32 mFrame;
    /* 0xC8 */ TVec3f mTutorialPos;
    /* 0xD4 */ s32 mTimerCount;
};

namespace MR {
    void registerGliBirdForTutorial(GliBird*) NO_INLINE;
    void registerPichanForTutorial(Pichan*);
    void startGliBirdTutorialTalk();
    bool isEndGliBirdTutorialTalk() NO_INLINE;
};  // namespace MR
