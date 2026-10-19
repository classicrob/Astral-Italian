#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/Nerve.hpp"

class GrandGalaxyMap;
class TalkMessageCtrl;

/// State "State[開始デモ]": zooms the camera in when the grand galaxy map opens with its demo (0x14 bytes).
class GrandGalaxyMapStateEnterDemo : public ActorStateBase< GrandGalaxyMap > {
public:
    GrandGalaxyMapStateEnterDemo(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapStateEnterDemo();
    virtual void init();
    virtual void appear();
    virtual void kill();

    inline void exeStart();
    void exeZoom();

    /* 0x10 */ f32 mFovy;
};

/// State "State[開始状態]": opens the grand galaxy map directly, turning the rocket to face the screen (0x10 bytes).
class GrandGalaxyMapStateStart : public ActorStateBase< GrandGalaxyMap > {
public:
    GrandGalaxyMapStateStart(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapStateStart();
    virtual void init();
    virtual void appear();
    virtual void kill();

    void exeStart();
};

/// State "State[ウェイト]": waits on a world of the grand galaxy map (0x1C bytes).
class GrandGalaxyMapStateWait : public ActorStateBase< GrandGalaxyMap > {
public:
    GrandGalaxyMapStateWait(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapStateWait();
    virtual void init();
    virtual void appear();
    virtual void kill();

    void exeWait();
    void startPointing(s32 point);
    void endPointing();

    /* 0x10 */ s32 mWorldNo;
    /* 0x14 */ s32 mPointingPoint;
    /* 0x18 */ s32 mPointingTimer;
};

/// State "State[グランドギャラクシーニュース]": the Meister's grand galaxy news talks (0x18 bytes).
class GrandGalaxyMapStateNews : public ActorStateBase< GrandGalaxyMap > {
public:
    GrandGalaxyMapStateNews(GrandGalaxyMap* pHost);

    virtual ~GrandGalaxyMapStateNews();
    virtual void init();
    virtual void appear();
    virtual void kill();

    bool checkNews();
    void exeMeisterAppear();
    void exeTalk();
    void exeTalkEnd();
    inline void exeWait();
    void startNews();
    void onNewsFlag();
    void nextNews();

    /* 0x10 */ s32 mNewsIndex;
    /* 0x14 */ TalkMessageCtrl** mTalkCtrls;
};
