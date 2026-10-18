#pragma once

#include "Game/NPC/NPCActor.hpp"

class TalkMessageCtrl;
class LayoutActor;
class PostManLetter;

class KinopioPostman : public NPCActor {
public:
    /// @brief Creates a new `KinopioPostman`.
    /// @param pName A pointer to the null-terminated name of the object.
    KinopioPostman(const char* pName);
    virtual ~KinopioPostman();

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void control();

    bool initTalkCtrl();
    bool tryRegisterDemo(const char* pName, const JMapInfoIter& rIter);
    void initChildObj(const JMapInfoIter& rIter);
    void onSwitchA();
    bool eventRead(u32 step);
    bool eventGive(u32 step);
    bool branchImportant(u32 step);
    bool branchLuigi(u32 step);
    bool eventReply(u32 step);
    bool trySelectDefaultTalk();
    bool trySelectEventTalk();
    bool tryEndLetter();
    bool tryStartLetter();
    void exeTalk();
    void exeReaction();
    void exeWait();
    void exeFinish();
    void exeDemo();
    void startDemo();

    /* 0x164 */ TalkMessageCtrl* mTalkCtrls[7];  // 0x164..0x17C
    /* 0x180 */ PostManLetter* mLayout;
    /* 0x184 */ bool mIsEventFlag;
    /* 0x185 */ bool mIsReplyFlag;
    /* 0x188 */ s32 mStepNum;
    /* 0x18C */ LiveActor* mChildObj;
    /* 0x190 */ LiveActor* mPostmanEvent;
    /* 0x194 */ s32 mWaitTimer;
};

namespace KinopioPostmanFunction {
    void turnQuaternion(TQuat4f*, const TVec3f&, const TVec3f&, f32);
}
