#pragma once

#include "Game/Screen/LayoutActor.hpp"

class TalkMessageCtrl;
class TalkTextFormer;
class JMapInfoIter;
class IconAButton;

class TalkBalloon : public LayoutActor {
public:
    TalkBalloon(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void kill();

    virtual void open(TalkMessageCtrl*);
    virtual void close();
    virtual void updateBalloon();
    virtual void updateTalking();
    virtual void pauseOff();
    virtual bool turnPage();
    virtual bool hasNextPage();
    virtual void skipMessage();
    virtual bool isTextAppearedAll();

    void create(const char*, bool, bool);

    /* 0x2C */ TalkMessageCtrl* mMessageCtrl;
    /* 0x30 */ TalkTextFormer* mTextFormer;
    /* 0x34 */ bool mHasBeak;
    /* 0x35 */ bool mIsBeakTurning;
    /* 0x36 */ u8 mDisableTalkSound;
};

class TalkBalloonShort : public TalkBalloon {
public:
    TalkBalloonShort(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void open(TalkMessageCtrl*);
    virtual void close();
    virtual void updateBalloon();

    void exeClose();
    void exeOpen();

};

class TalkBalloonEvent : public TalkBalloon {
public:
    TalkBalloonEvent(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void open(TalkMessageCtrl*);
    virtual void close();
    virtual bool turnPage();
    virtual void skipMessage();
    virtual bool isTextAppearedAll();

    void exeTalk();
    void exeWait();
    void exeOpen();
    void updateBeak();
    void exeClose();

    /* 0x38 */ u32 mTextFormatType;
    /* 0x3C */ IconAButton* mAButton;
};

class TalkBalloonSign : public TalkBalloonEvent {
public:
    TalkBalloonSign(const char*);

    virtual void init(const JMapInfoIter&);
    virtual bool turnPage();
    void exeNextPage();
};

class TalkBalloonInfo : public TalkBalloon {
public:
    TalkBalloonInfo(const char*);

    virtual void open(TalkMessageCtrl*);
    virtual void close();
    virtual bool isTextAppearedAll() {
        return true;
    }
    virtual void updateBalloon() {
    }
    virtual void updateTalking() {
    }
};

class TalkBalloonIcon : public TalkBalloonShort {
public:
    TalkBalloonIcon(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void open(TalkMessageCtrl*);
};

class TalkBalloonHolder {
public:
    TalkBalloonHolder();

    void pauseOff();
    void skipMessage();
    TalkBalloon* getBalloon(const TalkMessageCtrl*);
    void balloonOff();
    void update();
    bool isActiveBalloonShort() const;

    /* 0x00 */ TalkBalloonShort** mBalloonShortArray;
    /* 0x04 */ TalkBalloonEvent* mBalloonEvent;
    /* 0x08 */ TalkBalloonSign* mBalloonSign;
    /* 0x0C */ TalkBalloonIcon* mBalloonIcon;
    /* 0x10 */ s32 mShortBalloonIndex;
    /* 0x14 */ IconAButton* mAButton;
};

