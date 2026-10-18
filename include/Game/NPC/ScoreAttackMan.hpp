#pragma once

#include "Game/NPC/NPCActor.hpp"

class JMapIdInfo;
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

    s32 branchFunc(u32);
    s32 eventFunc(u32);
    s32 animeFunc(u32);

    void exeWait();
    void exeTalk();
    void exeStart();
    void exeReaction();
    void exeEnd();
    void exeEventWait();

    /* 0x15C */ s32 _15C;
    /* 0x160 */ s32 _160;
    /* 0x164 */ JMapIdInfo* mJMapIdInfo;
    /* 0x168 */ TalkMessageCtrl* mTalkCtrl;
    /* 0x16C */ LiveActor* mCameraTarget;
    /* 0x170 */ TakeOutStar* mTakeOutStar;
    /* 0x174 */ f32 mNearRadius;
    /* 0x178 */ JMapIdInfo* mRestartInfo;
    /* 0x17C */ bool mIsInRange;
};
