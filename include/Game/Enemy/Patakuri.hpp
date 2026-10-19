#pragma once

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/Kuribo.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class YoshiLockOnTarget;

/// Wing parts of a Patakuri (SMG2-only, TU00). Named by address where the original name is unknown.
class PatakuriWing : public PartsModel {
public:
    PatakuriWing(LiveActor*);

    virtual ~PatakuriWing();

    // 0x801E4A20: start the damage action and kill the wing when it ends.
    void startWingDamage();
};

/// SMG2-only Yoshi-edible walker that carries a Kuribo body (TU00).
class Patakuri : public LiveActor {
public:
    Patakuri(const char*);

    virtual ~Patakuri();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool tryTrampleDefeat();
    bool tryStartHipDropDown();
    static TVec3f calcBodyDropPos(Patakuri* pPatakuri);
    bool isRailOrBind() const;
    void exeRail();
    void exePatakuriBlow();
    void exeLandSwoon();
    void exePatakuriStaggerEnd();
    void exePatakuriHipDropDown();
    void exeCoinPop();
    void exeHitStep();
    void exePatakuriDead();

    /* 0x90 */ Kuribo* mBody;
    /* 0x94 */ TQuat4f mQuat;
    /* 0xA4 */ TVec3f mFront;
    /* 0xB0 */ PatakuriWing* mWing;
    /* 0xB4 */ AnimScaleController* mScaleCtrl;
    /* 0xB8 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xBC */ YoshiLockOnTarget* mLockOnTarget;
};
