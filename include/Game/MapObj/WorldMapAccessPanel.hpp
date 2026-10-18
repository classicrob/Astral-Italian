#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class ActorCameraInfo;

class WorldMapAccessPanel : public LiveActor {
public:
    WorldMapAccessPanel(const char*);

    virtual ~WorldMapAccessPanel();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

    bool calcJointMtx(TPos3f*, const JointControllerInfo&);
    bool isPlayerOnAccessSensor();
    bool isPlayerNearAccessGround();
    bool isPlayerOutOfRange();
    void setWorldIdsAndReset();
    void startWorldMapWipe();

    void exeWait();
    void exeStand();
    void exeDemoStart();
    void exeDemoStep();
    void exeAccess();
    void exeReturn();

    /* 0x90 */ s32 mWorldIdA;
    /* 0x94 */ s32 mWorldIdB;
    /* 0x98 */ s32 mCounterA;
    /* 0x9C */ s32 mCounterB;
    /* 0xA0 */ JointController* mJointCtrl;
    /* 0xA4 */ u8 mIsHipDropped;
    /* 0xA8 */ ActorCameraInfo* mCameraInfo;
    /* 0xAC */ f32 mFloatAC;
    /* 0xB0 */ f32 mFloatB0;
    /* 0xB4 */ f32 mFloatB4;
};
