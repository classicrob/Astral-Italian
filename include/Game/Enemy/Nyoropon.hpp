#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class HitSensor;
class JointController;
struct JointControllerInfo;
class PartsModel;
class WalkerStateBindStarPointer;

/// SMG2-only enemy (stretchy 7-joint stalk with a head). LiveActor = 0x90 bytes. Written from the target asm.
class Nyoropon : public LiveActor {
public:
    Nyoropon(const char* pName);

    virtual ~Nyoropon();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void exeSleep();         // fn_801D8370
    void exeWait();          // fn_801D8460
    void exeFind();          // fn_801D8520
    void exeRecover();       // fn_801D8640
    void exeStarPiece();     // fn_801D8730
    void exeDownStretch();   // fn_801D8830
    void exeSpin();          // fn_801D8910
    void exeSearchLight();   // fn_801D89F0
    void exeDownSpin();      // fn_801D8AD0
    void exeSupportFreeze(); // fn_801D8BB0

    bool isHittableNerve() const;       // fn_801D8130
    bool isLightTargetNerve() const;    // fn_801D81E0
    bool isIdleNerve() const;           // fn_801D8260
    bool isStretchedNerve() const;      // fn_801D82D0
    f32 calcStretchScale() const;       // fn_801D8350
    void setFaceSleep();                // fn_801D8C80
    void setFaceNormal();               // fn_801D8CF0
    void setFaceAttack();               // fn_801D8D60
    void initJointControllers();        // fn_801D8DD0
    void initBody();                    // fn_801D8EE0
    void updateTarget();                // fn_801D9070
    void calcHeadTarget();              // fn_801D91A0
    void updateBody(f32, f32, f32);     // fn_801D93A0
    void calcBodyMatrices(f32);         // fn_801D9C60
    bool calcJointMatrix(TPos3f*, const JointControllerInfo&);  // fn_801DA150
    bool updateStretch(s32);            // fn_801DA300
    void playIdleSound();               // fn_801DA370

    /* 0x90 */ JointController** mJointCtrls;
    /* 0x94 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStar;
    /* 0x9C */ PartsModel* mHead;
    /* 0xA0 */ TPos3f* mMtxs;
    /* 0xA4 */ TPos3f mRootMtx;
    /* 0xD4 */ TPos3f mRootInvMtx;
    /* 0x104 */ u8 _104[0xC];
    /* 0x110 */ TVec3f* mPoints;
    /* 0x114 */ TVec3f* mDirs;
    /* 0x118 */ f32* mLengths;
    /* 0x11C */ TVec3f mFront;
    /* 0x128 */ TVec3f mTarget;
    /* 0x134 */ f32 mStretchRate;
    /* 0x138 */ f32 mStretch;
    /* 0x13C */ f32 mStretchGoal;
    /* 0x140 */ f32 mStretchStart;
    /* 0x144 */ TVec3f mVel;
    /* 0x150 */ TVec3f mPrevVel;
    /* 0x15C */ f32 mAmp;
    /* 0x160 */ s32 mFrame;
    /* 0x164 */ const Nerve* mSavedNerve;
    /* 0x168 */ bool mInitialized;
    /* 0x169 */ bool mIsAsleep;
};
