#pragma once

#include <revolution.h>

class TalkMessageCtrl;
class TalkBalloon;
class LiveActor;
class TalkSupportPlayerWatcher;
class IconAButton;
class LayoutActor;
class TalkBalloonShort;
class TalkBalloonEvent;
class TalkBalloonInfo;
class TalkBalloonSign;
class TalkBalloonCompose;

class TalkState {
public:
    TalkState();

    virtual void init(TalkMessageCtrl*, TalkBalloon*);
    virtual bool prep(const TalkMessageCtrl*) {
        return true;
    }

    virtual bool test() {
        return true;
    }

    virtual void open() {
    }

    virtual bool talk(const TalkMessageCtrl*) {
        return true;
    }

    virtual void clos() {
    }

    virtual bool term(const TalkMessageCtrl*) {
        return true;
    }

    virtual u32 getPageCount() const;

    bool isBalloonDead() const;
    bool isLostMessage(const TalkMessageCtrl*) const;
    bool isSelfInterrupt(const TalkMessageCtrl*) const;

    /* 0x04 */ TalkMessageCtrl* mTalkCtrl;
    /* 0x08 */ TalkBalloon* mBalloon;
    /* 0x0C */ u32 mMessageID;
};

class TalkStateShort : public TalkState {
public:
    TalkStateShort();

    virtual bool prep(const TalkMessageCtrl*);
    virtual void open();
    virtual bool talk(const TalkMessageCtrl*);
    virtual void clos();
};

class TalkStateEvent : public TalkState {
public:
    TalkStateEvent();

    virtual bool prep(const TalkMessageCtrl*);
    virtual bool test() NO_INLINE;
    virtual void open();
    virtual bool talk(const TalkMessageCtrl*);
    virtual void clos();
    virtual bool term(const TalkMessageCtrl*);
    virtual u32 getPageCount() const;

    /* 0x10 */ s32 mHeldButtonFrames;
    /* 0x14 */ s32 mCloseDelayFrames;
    /* 0x18 */ s32 mTalkSoundDelay;
    /* 0x1C */ TalkSupportPlayerWatcher* mPlayerWatcher;
    /* 0x20 */ u8 mIsAButtonHeld;
    /* 0x21 */ bool mUnusedFlag;
    /* 0x24 */ u32 mPageCount;
};

class TalkStateNormal : public TalkStateEvent {
public:
    TalkStateNormal();

    virtual bool prep(const TalkMessageCtrl*);
    virtual bool test();
    virtual void clos();
    virtual bool term(const TalkMessageCtrl*);
    void updateButton();

    /* 0x28 */ IconAButton* mAButton;
};

class TalkStateHolder {
public:
    TalkStateHolder();

    void update();
    void pauseOff();
    TalkState* getState(const TalkMessageCtrl*);

    /* 0x00 */ TalkSupportPlayerWatcher* mPlayerWatcher;
    /* 0x04 */ IconAButton* mAButton;
    /* 0x08 */ TalkStateShort* mTalkShort;
    /* 0x0C */ TalkStateNormal* mTalkNormal;
    /* 0x10 */ TalkStateEvent* mTalkEvent;
    /* 0x14 */ TalkState* mTalk;
};

namespace MR {
    void startTalkSound(u8, const LiveActor*);
};  // namespace MR
