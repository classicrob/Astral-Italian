#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class AnimScaleController;
class CameraTargetMtx;
class DinoPackunBall;
class DinoPackunDemoPosition;
class DinoPackunEggShell;
class DinoPackunSequencer;
class DinoPackunTail;
class JointControllerInfo;
class FootPrint;
class PartsModel;

template < typename T >
class JointControlDelegator;

class DinoPackun : public LiveActor {
public:
    DinoPackun(const char*);

    virtual ~DinoPackun();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initTail();
    void initFootPrint();
    void initDemoPosition(const JMapInfoIter&);
    void initEggShell();
    void initBall();
    void initCamera(const JMapInfoIter&);
    bool isVs1() const;
    bool isVs2() const;
    void initScaleJointController();
    void startHitReaction();
    bool isHitReaction(s32) const;
    bool hitScaleJoint(TPos3f*, const JointControllerInfo&);
    DinoPackunEggShell* getEggShell();
    PartsModel* getEggBrokenModel();
    PartsModel* getBallModel();
    void attackSensorTail(HitSensor*, HitSensor*);
    bool receiveMsgPlayerAttackTail(u32, HitSensor*, HitSensor*);
    void startSequence();
    void updatePose();
    void updateFootPrintNerve(s32, s32);
    void updateCameraInfo();
    void updateNormalVelocity();
    void updateRunVelocity();
    void appearStarPiece(s32);
    bool isSensorEgg(const HitSensor*) const;
    void resetPosition();
    void adjustTailRootPosition(const TVec3f&, f32);
    void activateParts();
    void onMovementParts();
    void onAimTailBall(s32);
    void offAimTailBall(s32);
    void startDemo();
    void startDemoAndReset();
    void endDemo();
    void startDamageCamera();
    void endDamageCamera();

    /* 0x90 */ DinoPackunTail* mTail;
    /* 0x94 */ DinoPackunBall* mBall;
    /* 0x98 */ FootPrint* mFootPrint;
    /* 0x9C */ DinoPackunEggShell* mShell;
    /* 0xA0 */ DinoPackunDemoPosition* mDemoPos;
    /* 0xA4 */ PartsModel* mShellBreakModel;
    /* 0xA8 */ PartsModel* mTailBall;
    /* 0xAC */ CameraTargetMtx* mCamTargetMtx;
    /* 0xB0 */ ActorCameraInfo* mCameraInfo;
    /* 0xB4 */ DinoPackunSequencer* mSequence;
    /* 0xB8 */ JointControlDelegator< DinoPackun >* mHitScaleJointCtrl;
    /* 0xBC */ AnimScaleController* mAnimScaleCtrl;
    /* 0xC0 */ TQuat4f mPoseQuat;
    /* 0xD0 */ TVec4f mInitialQuat;
    /* 0xE0 */ TVec3f mInitialPos;
    /* 0xEC */ TVec3f mFront;
    /* 0xF8 */ s32 mCameraRegisterId;
    /* 0xFC */ TVec3f mCameraVec;
    /* 0x108 */ f32 mTailCameraBlendStep;
    /* 0x10C */ f32 mTailCameraBlendRate;
    /* 0x110 */ s32 mDamageCameraState;
    /* 0x114 */ u8 mIsCalculatingAnim;
};

namespace MR {
    NameObj* createDinoPackunVs1(const char*);
    NameObj* createDinoPackunVs2(const char*);
};  // namespace MR
