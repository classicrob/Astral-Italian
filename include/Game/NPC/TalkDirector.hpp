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
    /* 0x06 */ u16 _6;
    /* 0x08 */ u32 _8;
    /* 0x0C */ TVec2f mScreenPos;
    /* 0x14 */ TVec3f _14;
    /* 0x20 */ f32 _20;
    /* 0x24 */ u8 _pad[0x18];
    /* 0x3C */ f32 _3C;
    /* 0x40 */ u8 _pad2[0x14];
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

    /* 0x20 */ MR::Vector< MR::AssignableArray< TalkMessageCtrl* > > mMsgControls;
    /* 0x2C */ TalkPeekZ* mPeekZ;
    /* 0x30 */ TalkBalloonHolder* mBalloonHolder;
    /* 0x34 */ TalkStateHolder* mStateHolder;
    /* 0x38 */ TalkMessageCtrl* mMsgCtrl;
    /* 0x3C */ TalkMessageCtrl* _3C;
    /* 0x40 */ TalkMessageCtrl* _40;
    /* 0x44 */ TalkMessageCtrl* _44;
    /* 0x48 */ TalkState* mTalkState;
    /* 0x4C */ bool _4C;
    /* 0x4D */ bool _4D;
    /* 0x4E */ bool _4E;
    /* 0x4F */ bool mIsInvalidClipping;
    /* 0x50 */ bool _50;
    /* 0x54 */ s32 mDemoType;
    /* 0x58 */ bool _58;
    /* 0x59 */ bool _59;
    /* 0x5A */ bool _5A;
    /* 0x5B */ bool _5B;
    /* 0x5C */ TalkMessageInfo mMessageInfo;
    /* 0x68 */ ActorCameraInfo* mCameraInfo;
    /* 0x6C */ LiveActor* mHostActor;

};

namespace MR {
    void pauseOffTalkDirector();
    void validateTalkDirector();
    void invalidateTalkDirector();
    void setTalkDirectorDrawSyncToken();
    bool isActiveTalkBalloonShort();
    void endNPCTalkCamera(bool, s32 = -1);
};  // namespace MR
