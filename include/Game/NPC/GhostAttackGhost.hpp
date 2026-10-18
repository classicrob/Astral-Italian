#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class GhostAttackGhostFootprint;
class HitSensor;
class JointController;
class JointControllerInfo;
class XanimePlayer;
class XanimeResourceTable;

/// @brief Race data playback used by the Ghost Attack ghost (SMG2 only).
class RaceDataReader {
public:
    RaceDataReader(LiveActor* pHost, const char* pName, const XanimeResourceTable* pTable);

    s32 receivePacket();
    void startActionName(const char* pName);

    /* 0x00 */ void* mVtbl;
    /* 0x04 */ s32 mIsEnable;
    /* 0x38 */ XanimePlayer* mXanimePlayer;
};

class GhostAttackGhost : public LiveActor {
public:
    /// @brief Creates a new `GhostAttackGhost`.
    /// @param pName A pointer to the null-terminated name of the object.
    GhostAttackGhost(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void makeActorAppeared();
    virtual MtxPtr getBaseMtx() const;
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

    bool calcHandJoint(TPos3f* pMtx, const JointControllerInfo& rInfo);

    void exeWait();
    void exeAppear();
    void exeNear();
    void exeAttack();
    void exeHit();
    void exeDemo();
    void exeStop();
    void exeStart();
    void exeTerminate();

    /* 0x90 */ RaceDataReader* mRaceDataReader;
    /* 0x94 */ GhostAttackGhostFootprint* mFootprint;
    /* 0x98 */ HitSensor* mHostSensor;
    /* 0x9C */ XanimeResourceTable* mXanimeResTable;
    /* 0xA0 */ s32 mObjArg0;
    /* 0xA4 */ TVec3f mPrevPos;
    /* 0xB0 */ TVec3f mPrevPos2;
    /* 0xBC */ JointController* mHandJointCtrl;
    /* 0xC0 */ JointController* mHeadJointCtrl;
};
