#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class BreakModel;
class ActorCameraInfo;
class AnimScaleController;
class CocoNutBall;
class FireBall;
class FixedPosition;

class OtaRockTank : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    f32 fn_801DFBB0();
    f32 fn_801DFC60();
    f32 fn_801DFD10();
    bool fn_801E1010(TPos3f* pMtx);
    bool fn_801E1020(TPos3f* pMtx);
    bool fn_801E1030(TPos3f* pMtx);
    OtaRockTank(const char* pName);

    virtual ~OtaRockTank();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

public:
    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ ActorCameraInfo* mActorCameraInfo;
    /* 0x98 */ ActorCameraInfo* mAnimCameraInfo;
    /* 0x9C */ ActorCameraInfo* mAnimCameraInfo2;
    /* 0xA0 */ TVec3f mPos;
    /* 0xAC */ bool mHasPowerStar;
    /* 0xB0 */ s32 mUnkB0;
    /* 0xB4 */ LiveActor* mChief;
    /* 0xB8 */ JointController* mJointCtrl0;
    /* 0xBC */ JointController* mJointCtrl1;
    /* 0xC0 */ JointController* mJointCtrl2;
    /* 0xC4 */ f32 mUnkC4;
    /* 0xC8 */ s32 mUnkC8;
    /* 0xCC */ s32 mUnkCC;
    /* 0xD0 */ s32 mUnkD0;
    /* 0xD4 */ CocoNutBall* mCocoNutBalls;
    /* 0xD8 */ FireBall* mFireBalls;
    /* 0xDC */ void* mUnkArrayDC;
    /* 0xE0 */ FixedPosition** mFixedPositions;
    /* 0xE4 */ TVec3f* mUnkArrayE4;
    /* 0xE8 */ TVec3f mRailClipPos;
    /* 0xF4 */ TVec3f mRailDirection;
    /* 0x100 */ TVec3f mPrevRailDirection;
    /* 0x10C */ f32 mUnk10C;
    /* 0x110 */ u8 mUnk110;
    /* 0x114 */ s32 mCurrentRailPointNo;
    /* 0x118 */ s32 mUnk118;
    /* 0x11C */ f32 mUnk11C;
    /* 0x120 */ u8 mUnk120;
    /* 0x121 */ u8 mUnk121;
    /* 0x122 */ u8 mUnk122;
    /* 0x124 */ LiveActor* mUnk124;
    /* 0x128 */ s32 mUnk128;
    /* 0x12C */ s32 mUnk12C;
    /* 0x130 */ f32 mUnk130;
};
