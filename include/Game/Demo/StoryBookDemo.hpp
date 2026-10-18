#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class AreaObj;
class StoryBookText;
class JUTTexture;
class StoryBookDemo : public LiveActor {
public:
    StoryBookDemo();
    virtual ~StoryBookDemo();
    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void startText(bool);
    void startTurnPage();
    bool isActive() const;
    bool isTurnPageEnd() const;
    bool isTurningPage() const;
    bool isTurnPageStart() const;
    void exeWait();
    void exeWarpWait();
    void exeAreaText();
    void exeWarpStart();
    void exeWarpPlayer();
    void exeWarpCapture();
    void exeWarpTurn();
    void exeWarpEnd();
    void exeWaitIcon();
    void exeText();
    void exeCloseText();
    void exeTurnStart();
    void exeTurnCapture();
    void exeTurnEnd();
    void exeWaitDemo();
    void capture();
    bool registerDemo();
    /* 0x90 */ AreaObj* mArea;
    /* 0x94 */ StoryBookText* mText;
    /* 0x98 */ JUTTexture* mCurrentTexture;
    /* 0x9C */ JUTTexture* mNextTexture;
    /* 0xA0 */ LiveActor* mShadow;
    /* 0xA4 */ bool mCaptureRequested;
    /* 0xA8 */ s32 mMessageIndex;
    /* 0xAC */ s32 mTextFrames;
};
namespace MR {
    void stopStoryBookDemoCasts();
    void requestMovementOnStoryBook();
    void startStoryBookText(bool);
    void waitStoryBookDemo();
    void startStoryBookTimedText(s32);
    bool isStoryBookDemoName(const char*);
    bool isStoryBookEndingDemoName(const char*);
    bool isStoryBookTextAppeared();
    bool isStoryBookTurnPaused();
    bool isStoryBookTurnPageEnd();
    bool isStoryBookTurningPage();
    bool isStoryBookTurnPageStart();
    void openStoryBookTextIcon();
    void termStoryBookTextIcon();
    void startStoryBookTurnPage();
    void pauseStoryBookTurnPage();
    void startStoryBookAppear() NO_INLINE;
    void endStoryBookAppear();
    void resetStoryBookAppear();
}
