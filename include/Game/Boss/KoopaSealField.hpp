#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class GravityLightRoad;
class AttackRockTutorial;
class ActorCameraInfo;
template < typename T >
class DeriveActorGroup;
class KoopaJrRobotSeal;

class KoopaSealField : public LiveActor {
public:
    KoopaSealField(const char*);

    virtual ~KoopaSealField();
    virtual void init(const JMapInfoIter&) override;
    virtual void makeActorAppeared() override;
    virtual void makeActorDead() override;
    virtual void control() override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;

    bool tryStartPuppet();
    void breakAllSeals();
    void exeWait();
    void exeOpen();
    void exeOpenEnd();
    void exeDemoCamera();
    void exeDemoPuppet();
    void exeDemoIn();
    void exeDemoEnd();

    /* 0x90 */ GravityLightRoad* mGravityLightRoad;
    /* 0x94 */ AttackRockTutorial* mAttackRockTutorial;
    /* 0x98 */ DeriveActorGroup< KoopaJrRobotSeal >* mSealGroup;
    /* 0x9C */ ActorCameraInfo* mCameraInfo;
    /* 0xA0 */ int mAlreadyDoneFlag;
    /* 0xA4 */ bool mIsSealField;
};
