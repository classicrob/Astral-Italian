#pragma once

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/KuriboChief.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class YoshiLockOnTarget;

/// Wing parts of a big Patakuri (SMG2-only, TU01).
class PatakuriBigWing : public PartsModel {
public:
    PatakuriBigWing(const char*);
    void fn_801E6474();

    virtual ~PatakuriBigWing();
    virtual void calcAndSetBaseMtx();
};

/// Big Patakuri: a KuriboChief body with hit sensors on the joints and a wing (SMG2-only, TU01).
class PatakuriBig : public LiveActor {
public:
    PatakuriBig(const char*);

    virtual ~PatakuriBig();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool fn_801E6370();
    void fn_801E6510();
    void fn_801E6730();
    void fn_801E67B0();
    void fn_801E6810();
    void fn_801E68B0();
    void fn_801E6930();
    void fn_801E6980();
    void fn_801E69E0();
    static TVec3f fn_801E6280(PatakuriBig* pPatakuri);

    /* 0x90 */ KuriboChief* mBody;
    /* 0x94 */ TQuat4f mQuat;
    /* 0xA4 */ TVec3f mFront;
    /* 0xB0 */ PatakuriBigWing* mWing;
    /* 0xB4 */ YoshiLockOnTarget* mLockOnTarget;
    /* 0xB8 */ AnimScaleController* mScaleCtrl;
    /* 0xBC */ WalkerStateBindStarPointer* mBindStarPointer;
};
