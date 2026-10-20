#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class YoshiLockOnTarget;

// SMG2-only unit (no Petari source). Fields from the ctor/init stores.
// LiveActor fields used: mPosition (0x14), mRotation (0x20), mScale (0x2C).
class BalloonFruit : public LiveActor {
public:
    BalloonFruit(const char* pName);

    virtual ~BalloonFruit();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeAppear();      // fn_80272170
    void exeWait();        // fn_80272200
    void exeTrampled();    // fn_80272280
    void exeSpin();        // fn_802722F0
    void exeShake();       // fn_80272360
    void exeHide();        // fn_802723D0
    void exeBreak();       // fn_80272440
    void exeHipDropCrash();  // fn_802724A0
    void exeDead();        // fn_802725B0
    void reset();          // fn_80272620
    bool isNerveNoYoshiEat() const;  // fn_80272D60
    bool isNerveYoshiAttackable() const;  // fn_80272E30
    bool isNerveSpinOrStill() const;  // fn_80272ED0
    bool isNerveDamage() const;       // fn_80272F70
    bool tryStartSpin();              // fn_80273010
    bool tryStartShake();             // fn_80273070
    bool tryStartBind();              // fn_802730E0
    bool isSwitchA() const;  // fn_80273150
    static void enableYoshiBalloonResource();  // fn_80272690

    /* 0x090 */ TVec3f mHomePos;
    /* 0x09C */ TVec3f mHomeRot;
    /* 0x0A8 */ f32 mBaseScale;
    /* 0x0AC */ TPos3f mMtxAC;
    /* 0x0DC */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x0E0 */ s32 mTimerE0;
    /* 0x0E4 */ f32 mAngleE4;
};
