#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Player/RaceDataReader.hpp"
#include "Game/Util/JointController.hpp"

class GhostAttackGhostFootprint;
class GhostAttackAccessor;
class HitSensor;
class JointController;
class JointControllerInfo;
class XanimePlayer;
class XanimeResourceTable;

class GhostAttackGhost : public LiveActor {
public:
    /// @brief Creates a new `GhostAttackGhost`.
    /// @param pName A pointer to the null-terminated name of the object.
    GhostAttackGhost(const char* pName);

    virtual ~GhostAttackGhost();
    virtual void init(const JMapInfoIter& rIter);
    virtual void makeActorAppeared();
    virtual MtxPtr getBaseMtx() const;
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    bool calcHandJoint(TPos3f* pMtx, const JointControllerInfo& rInfo);

    const char* getRecordedAnimationName() const;
    const char* getCurrentAnimationName() const;
    bool tryAttachToHost(HitSensor*, HitSensor*);
    bool tryStartRace(HitSensor*, HitSensor*);
    void startSleep();
    void updateSensorRadius();
    void exeStart();
    void exeAppear();
    void exeRun();
    void exeWait();
    void exeBind();
    void exeStarWaitStart();
    void exeStarWait();

    /* 0x90 */ RaceDataReader* mRaceDataReader;
    /* 0x94 */ GhostAttackGhostFootprint* mFootprint;
    /* 0x98 */ HitSensor* mHostSensor;
    /* 0x9C */ XanimeResourceTable* mXanimeResTable;
    /* 0xA0 */ s32 mObjArg0;
    /* 0xA4 */ TVec3f mMovementDelta;
    /* 0xB0 */ TVec3f mPreviousPosition;
    /* 0xBC */ JointController* mHandJointCtrl;
    /* 0xC0 */ JointController* mHeadJointCtrl;
};

namespace MR {
    GhostAttackAccessor* registerGhostAttackGhost(GhostAttackGhost*);
}
