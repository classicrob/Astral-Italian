#pragma once

#include "Game/NPC/TalkMessageInfo.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/System/DrawSyncManager.hpp"
#include "Game/Util/Array.hpp"

class ActorCameraInfo;
class LiveActor;
class TalkMessageCtrl;
class TalkBalloonHolder;
class TalkStateHolder;
class TalkState;

class TalkPeekZ : DrawSyncCallback {
public:
    TalkPeekZ();

    virtual void drawSyncCallback(u16);

    void setDrawSyncToken();
    /* 0x04 */ u16 mDrawSync;
    /* 0x06 */ u16 mDrawSyncReserved;
    /* 0x08 */ u32 mDepthValue;
    /* 0x0C */ TVec2f mScreenPos;
    /* 0x14 */ TVec3f mWorldPosition;
    /* 0x20 */ f32 mProjectionParameters[7];
    /* 0x3C */ f32 mViewportParameters[6];
};

class TalkDirector : public LayoutActor {
public:
    TalkDirector(const char*);

    virtual ~TalkDirector();
    virtual void init(const JMapInfoIter&);
    virtual void movement();

    bool request(TalkMessageCtrl*, bool);
    bool test(TalkMessageCtrl*, bool, bool);
    bool start(TalkMessageCtrl*, bool, bool, bool);
    void updateMessage();
    void prepTalk(TalkMessageCtrl*, bool, bool, bool);
    void termTalk();
    TalkState* initState(TalkMessageCtrl*);
    bool isInvalidTalk() const;
    void appearYesNoSelector(const TalkMessageCtrl*) const;
    s32 getDemoType(const TalkMessageCtrl*, bool) const;
    void pauseOff();
    bool isSystemTalking() const;
    bool isNormalTalking() const;
    LiveActor* getTalkingActor() const;
    void exePrep();
    void exeTalk();
    void exeClose();
    void exeSlct();
    void exeNext();
    void exeTerm();
    void exeWait();
    /* 0x2C */ MR::Vector< MR::AssignableArray< TalkMessageCtrl* > > mMsgControls;
    /* 0x38 */ TalkPeekZ* mPeekZ;
    /* 0x3C */ TalkBalloonHolder* mBalloonHolder;
    /* 0x40 */ TalkStateHolder* mStateHolder;
    /* 0x44 */ TalkMessageCtrl* mMsgCtrl;
    /* 0x48 */ TalkMessageCtrl* mPreviousRequestedCtrl;
    /* 0x4C */ TalkMessageCtrl* mNextRequestedCtrl;
    /* 0x50 */ TalkMessageCtrl* mLastTalkCtrl;
    /* 0x54 */ TalkState* mTalkState;
    /* 0x58 */ bool mTalkStartedThisFrame;
    /* 0x59 */ bool mTalkEndedThisFrame;
    /* 0x5A */ bool mIsTalkDisabled;
    /* 0x5B */ bool mIsInvalidClipping;
    /* 0x5C */ bool mContinueMessage;
    /* 0x60 */ s32 mDemoType;
    /* 0x64 */ bool mForceTalk;
    /* 0x65 */ bool mUsePlayerTalk;
    /* 0x66 */ bool mTalkReserved0;
    /* 0x67 */ bool mTalkReserved1;
    /* 0x68 */ TalkMessageInfo mMessageInfo;
    /* 0x74 */ ActorCameraInfo* mCameraInfo;
    /* 0x78 */ LiveActor* mHostActor;

};

namespace MR {
    void pauseOffTalkDirector();
    void validateTalkDirector();
    void invalidateTalkDirector();
    void setTalkDirectorDrawSyncToken();
    bool isActiveTalkBalloonShort();
    void endNPCTalkCamera(bool, s32 = -1);
};  // namespace MR
