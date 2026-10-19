#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Player/YoshiFunction.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class BrosBase;
class YoshiLockOnTarget;
class AnimScaleController;

/// @brief Common base of HammerBrosHammer and BoomerangBrosBoomerang (SMG2).
class BrosProjectileBase : public LiveActor {
public:
    BrosProjectileBase(const char*);

    virtual ~BrosProjectileBase() {}
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    // vtable +0x70 .. +0x94
    virtual void v70(const TVec3f&, const TVec3f&) = 0;
    virtual void attachToBros(BrosBase*);  // fn_8018B9C0
    virtual bool v78() = 0;
    virtual bool v7C() = 0;
    virtual bool v80() = 0;
    virtual bool v84() = 0;
    virtual bool v88() = 0;
    virtual bool v8C() = 0;
    virtual bool v90() = 0;

    void clearState();     // fn_8018B720
    void killWithState1();  // fn_8018B730
    void killWithState2();  // fn_8018B750
    void killWithState3();  // fn_8018B770
    void killWithState4();  // fn_8018B790
    void exeEaten();  // fn_8018BDB0

    /* 0x90 */ BrosBase* mOwner;
    /* 0x94 */ AnimScaleController* mScaleCtrl;
    /* 0x98 */ TVec3f mFront;
    /* 0xA4 */ TVec3f mLocalVel;
    /* 0xB0 */ s32 mState;
    /* 0xB4 */ bool mIsPointBound;
    /* 0xB8 */ YoshiLockOnTarget* mYoshiLockOn;
};
