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

    void exeSleep();
    void exeWait();
    void exeFind();
    void exeRecover();
    void exeStarPiece();
    void exeDownStretch();
    void exeSpin();
    void exeSearchLight();
    void exeDownSpin();
    void exeSupportFreeze();

    bool isHittableNerve() const;
    bool isLightTargetNerve() const;
    bool isIdleNerve() const;
    bool isStretchedNerve() const;
    f32 calcBodyStretchScale() const;
    void setFaceSleep();
    void setFaceNormal();
    void setFaceAttack();
    void initBodyJointControllers();
    void resetBodySegments();
    void updateWanderTarget();
    void updatePlayerTarget();
    void updateBodySegments(f32, f32, f32);
    void updateBodyWaveMatrices(f32);
    bool calcJointMatrix(TPos3f*, const JointControllerInfo&);
    bool updateStretchTransition(s32);
    void playIdleSound();

    /* 0x90 */ JointController** mJointCtrls;
    /* 0x94 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStar;
    /* 0x9C */ PartsModel* mHead;
    /* 0xA0 */ TPos3f* mMtxs;
    /* 0xA4 */ TPos3f mRootMtx;
    /* 0xD4 */ TPos3f mRootInvMtx;
    /* 0x104 */ u8 mUnusedTransformData[0xC];
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
