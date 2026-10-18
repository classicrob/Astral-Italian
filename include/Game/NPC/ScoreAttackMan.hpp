#pragma once

#include "Game/NPC/NPCActor.hpp"

class JMapIdInfo;
class ActorStatePlayerPosReset;
class ActorCameraInfo;
class LiveActor;
class TakeOutStar;
class TalkMessageCtrl;

class ScoreAttackMan : public NPCActor {
public:
    /// @brief Creates a new `ScoreAttackMan`.
    /// @param pName A pointer to the null-terminated name of the object.
    ScoreAttackMan(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();

    bool branchFunc(u32);
    bool eventFunc(u32);
    bool animeFunc(u32);

    void exeWait();
    void exeTalk();
    void startResult();
    bool hasDeclaredPowerStar();
    void exeResetPlayer();
    void exeTalkResult();
    void exeStartCamera();

    /* 0x164 */ ActorStatePlayerPosReset* mResetState;
    /* 0x168 */ TalkMessageCtrl* mTalkCtrl;
    /* 0x16C */ ActorCameraInfo* mCameraInfo;
    /* 0x170 */ TakeOutStar* mTakeOutStar;
    /* 0x174 */ f32 mNearRadius;
    /* 0x178 */ JMapIdInfo* mRestartInfo;
    /* 0x17C */ bool mIsInRange;
};
