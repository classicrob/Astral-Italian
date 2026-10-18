#pragma once

#include "Game/NPC/NPCActor.hpp"

class TalkMessageCtrl;
class LayoutActor;

class KinopioPostman : public NPCActor {
public:
    /// @brief Creates a new `KinopioPostman`.
    /// @param pName A pointer to the null-terminated name of the object.
    KinopioPostman(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void control();

    /* 0x164 */ TalkMessageCtrl* mTalkCtrls[7];  // 0x164..0x17C
    /* 0x180 */ LayoutActor* mLayout;
    /* 0x184 */ bool mIsEventFlag;
    /* 0x185 */ bool mIsReplyFlag;
    /* 0x188 */ s32 mStepNum;
    /* 0x18C */ NameObj* mChildObj;
    /* 0x190 */ LiveActor* mPostmanEvent;
    /* 0x194 */ s32 mWaitTimer;
};
