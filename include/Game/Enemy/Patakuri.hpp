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
    PatakuriWing(const char*);

    virtual ~PatakuriWing();

    // 0x801E4A20: start the damage action and kill the wing when it ends.
    void fn_801E4A20();
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

    bool fn_801E4E50();
    bool fn_801E4ED0();
    static void fn_801E4F40(TVec3f* pOut, Patakuri* pPatakuri);
    bool fn_801E58C0() const;
    void fn_801E52A0();
    void fn_801E54C0();
    void fn_801E5540();
    void fn_801E55A0();
    void fn_801E5640();
    void fn_801E5730();
    void fn_801E57D0();
    void fn_801E5830();

    /* 0x90 */ Kuribo* mBody;
    /* 0x94 */ TQuat4f mQuat;
    /* 0xA4 */ TVec3f mFront;
    /* 0xB0 */ PatakuriWing* mWing;
    /* 0xB4 */ AnimScaleController* mScaleCtrl;
    /* 0xB8 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xBC */ YoshiLockOnTarget* mLockOnTarget;
};
