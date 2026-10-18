#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry.hpp>

class MapPartsRailMover;
class MapPartsRotator;
class ModelObj;

class Gorogoro : public LiveActor {
public:
    Gorogoro(const char*);

    virtual ~Gorogoro() {}
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeMove();
    void exeDisappear();
    void doRailMove();

    /* 0x90 */ MapPartsRailMover* mRailMover;
    /* 0x94 */ MapPartsRotator* mRotator;
    /* 0x98 */ TVec3f mRailClipPos;
    /* 0xA4 */ ModelObj* mSandModel;
    /* 0xA8 */ TVec3f mSensorFollowPos;
    /* 0xB4 */ f32 mRollSpeed;
    /* 0xB8 */ TPos3f mBaseTRMtx;
    /* 0xE8 */ u32 _E8;
    /* 0xEC */ TPos3f mCollisionMtx;
    /* 0x11C */ TVec3f mSavedPos;
};

class GorogoroCylinderRock : public LiveActor {
public:
    GorogoroCylinderRock(const char*);

    virtual ~GorogoroCylinderRock() {}
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

    void exeHide();
    void exeAppear();
    void exeFall();
    void updateRoll();
    void updateFallOnRail();
    void updateRotateOnRail();
    void updateRotateNoGravity();
    bool tryLandOnGround();
    bool tryUpdateNerve();

    /* 0x90 */ TVec3f mClipVec;
    /* 0x9C */ TVec3f mRailDirection;
    /* 0xA8 */ TVec3f mGroundPos;
    /* 0xB4 */ f32 mRotateSpeed;
    /* 0xB8 */ TPos3f mBaseTRMtx;
    /* 0xE8 */ TPos3f mShadowMtx;
    /* 0x118 */ s32 mRailPointNo;
    /* 0x11C */ f32 mRotateCoef;
    /* 0x120 */ f32 mRailSpeed;
    /* 0x124 */ s32 mAppearStep;
};
