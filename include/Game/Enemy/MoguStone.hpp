#pragma once

#include "Game/LiveActor/ModelObj.hpp"

class ParabolicPath;

class MoguStone : public ModelObj {
public:
    MoguStone(const char*, const char*);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    virtual void doBehavior();
    virtual void startBreakSound();
    virtual void startThrowLevelSound();

    void exeTaken();
    void exeThrow();
    void exeFall();
    bool isTaken();
    void emit(bool, const TVec3f&, const TVec3f&, f32);

    /* 0x94 */ TQuat4f mPoseQuat;
    /* 0xA4 */ TQuat4f mSpinQuat;
    /* 0xB4 */ TVec3f mThrowDirection;
    /* 0xC0 */ f32 mThrowSpeed;
    /* 0xC4 */ bool mProjectDirectionOnGround;
};

class ThrowingIce : public MoguStone {
public:
    ThrowingIce(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void doBehavior();
    virtual void startBreakSound();
    virtual void startThrowLevelSound();

    void emitIce(const TVec3f&, const TVec3f&, f32, const TVec3f&);

    /* 0xC4 */ ParabolicPath* mPath;
};
