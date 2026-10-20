#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/Rock.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class AnimScaleParam;
class KeySwitch;
class ModelObj;
class Nerve;

class Wanwan : public LiveActor {
public:
    enum Type { TypeNormal = 0, TypeMini = 1, TypeGold = 2 };

    Wanwan(f32 = 10.0f, const char* = "\x83\x8f\x83\x93\x83\x8f\x83\x93");

    virtual ~Wanwan();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isGoalIn() const NO_INLINE;  // fn_8021B2F0
    static s32 getGoalStayStep() NO_INLINE;  // fn_8021B300
    bool isMini() const { return mType == 1; }
    static s32 getType(const JMapInfoIter&);
    // Preserve the existing Rock::Type ABI; its values use Wanwan's Type domain.
    static s32 getAppearStarPieceNum(Rock::Type) NO_INLINE;

    void initMapToolInfo(const JMapInfoIter&);
    void initModel();
    void initSensor();
    void initEffect();
    bool isBreakByWall() const;
    bool move(f32);
    void calcBaseMtx(TPos3f*) const;
    bool isInClippingRange() const;
    void startSoundWanwanVoice();
    void startRollLevelSound(bool);
    bool tryFreeze(const Nerve*);
    void setBtkForEnvironmentMap(LiveActor*, const char*);
    void rumblePadAndCamera();
    void setNerveBreak(bool);
    void updateRotateX(f32);
    void appearStarPiece();
    void moveOnRail(f32, f32, bool) NO_INLINE;
    bool isForceInvalidBindSection() const;
    bool tryBreakReachedGoal();
    bool tryRequestAppearPowerStar();
    void requestAppearPowerStar() NO_INLINE;
    bool tryOnSwitchDead();

    void exeAppearSign();
    void exeAppear();
    void exeAppearMoveInvalidBind();
    void exeMove();
    void exeMoveInvalidBind();
    void exeMoveLava();
    void exeMoveToGoal();
    void exeGoalIn();
    void exeBreak();
    void exeFreeze();

    /* 0x090 */ NameObj* mItemSource;
    /* 0x094 */ s32 mType;
    /* 0x098 */ ModelObj* mModel;
    /* 0x09C */ f32 mSpeed;
    /* 0x0A0 */ bool mBreakAtGoal;
    /* 0x0A4 */ TVec3f mInitPos;
    /* 0x0B0 */ f32 mRadius;
    /* 0x0B4 */ f32 mRotSpeed;
    /* 0x0B8 */ TPos3f mBaseMtx;
    /* 0x0E8 */ TVec3f mFallVel;
    /* 0x0F4 */ f32 mGravityAcceleration;
    /* 0x0F8 */ s32 mRailPointNo;
    /* 0x0FC */ TVec3f mLastFront;
    /* 0x108 */ TVec3f mFront;
    /* 0x114 */ u32 mLandCount;
    /* 0x118 */ bool mHasBeenAirborne;
    /* 0x119 */ bool mZeroGravity;
    /* 0x11C */ s32 mVoiceTimer;
    /* 0x120 */ const Nerve* mFreezeReturnNerve;
    /* 0x124 */ s32 mInvalidBindMoveStep;
    /* 0x128 */ s32 mAppearStep;
    /* 0x12C */ s32 mMoveStep;
    /* 0x130 */ f32 mRotateOffs;
    /* 0x134 */ s32 mRollSoundTimer;
    /* 0x138 */ TVec3f mGoalPos;
    /* 0x144 */ bool mIsAtGoal;
    /* 0x145 */ bool mRequestedPowerStar;
    /* 0x146 */ bool mHideAtStart;
    /* 0x148 */ AnimScaleParam* mScaleParam;
    /* 0x14C */ AnimScaleController* mScaleCtrl;
    /* 0x150 */ bool mIsInLava;
    /* 0x154 */ TPos3f mLavaMtx;
    /* 0x184 */ KeySwitch* mKeySwitch;
};

class WanwanCreator : public LiveActor {
public:
    WanwanCreator(const char*);

    virtual ~WanwanCreator();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();

    void invalidate();
    void initMapToolInfo(const JMapInfoIter&);
    void create();
    void exeActive();
    void exeDeactive();

    /* 0x90 */ Wanwan* mWanwans;
    /* 0x94 */ s32 mType;
    /* 0x98 */ f32 mSpeed;
    /* 0x9C */ bool mBreakAtGoal;
    /* 0xA0 */ s32 mInterval;
    /* 0xA4 */ s32 mCount;
    /* 0xA8 */ u32 mTimer;
    /* 0xAC */ bool mValid;
};

class BreakModel;
class WalkerStateBindStarPointer;

class WanwanSearch : public LiveActor {
public:
    WanwanSearch(const char*);

    virtual ~WanwanSearch();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isMove() const NO_INLINE;          // fn_8021E9F0
    bool isMoveOrFreeze() const NO_INLINE;  // fn_8021EA00
    bool isMove2() const NO_INLINE;         // fn_8021EA60
    void startVoice() NO_INLINE;            // fn_8021EA70

    void exeWait();
    void exeMove();
    void exeGoal();
    void exeBreak();

    /* 0x090 */ TPos3f mMtx;
    /* 0x0C0 */ WalkerStateBindStarPointer* mBindState;
    /* 0x0C4 */ BreakModel* mBreakModel;
    /* 0x0C8 */ TVec3f mInitPos;
    /* 0x0D4 */ TQuat4f mQuat;
    /* 0x0E4 */ TVec3f mFront;
    /* 0x0F0 */ TVec3f mGoalPos;
    /* 0x0FC */ TVec3f mGoalUp;
    /* 0x108 */ TVec3f mGoalFront;
    /* 0x114 */ f32 mAccel;
    /* 0x118 */ f32 mRadius;
    /* 0x11C */ s32 mAirCount;
    /* 0x120 */ bool mBreakOnDeath;
    /* 0x124 */ s32 mVoiceTimer;
    /* 0x128 */ TVec3f mLastPos;
};
