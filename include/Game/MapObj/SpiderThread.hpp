#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JUTTexture;
class SpiderThreadHangInfo;
class SpiderThreadMainPoint;
class SpiderThreadPart;
class SpiderThreadPoint;
class SpiderThreadRadialLine;
class SpiderThreadWindCtrl;

struct CutPointEntry {
    s32 mRadialIdx;
    s32 mPointIdx;
};

class SpiderThread : public LiveActor {
public:
    SpiderThread(const char*);

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();
    virtual void draw() const;
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initThread(const TVec3f&);
    void startActorBind(LiveActor*, const TVec3f**, const TVec3f**, const TVec3f**, const TVec3f&, const TVec3f&, s32);
    bool touchActor(const TVec3f&, const TVec3f&);
    void tryPush(const TVec3f&, f32);
    void stopAllPartsPoint();
    void cutSpiderThread();
    void initMainThreads();
    void connectMainPoint(SpiderThreadPoint*, SpiderThreadPoint*, bool);
    void updatePointerInfo();
    void updateHangedPoint();
    void tryTouchPoint(s32);
    void findNearestPointPos(const TVec3f**, const TVec3f**, s32*, s32*, const TVec3f&) const;
    SpiderThreadHangInfo* findHangInfo(LiveActor*) const;
    void initDraw() const;

    SpiderThreadRadialLine* getMainRadialLine(int idx) const {
        return mRadialLines[idx];
    }

    SpiderThreadPart* getRadialPart(int idx) const {
        return mRadialParts[idx];
    }

    SpiderThreadPart* getCirclePart(int idx) const {
        return mCircleParts[idx];
    }

    SpiderThreadHangInfo* getActorHangInfo(int idx) const {
        return mHangInfos[idx];
    }

    /* 0x90 */ SpiderThreadMainPoint* mMainPoint;
    /* 0x94 */ SpiderThreadHangInfo** mHangInfos;
    /* 0x98 */ s32 mNumHangInfos;
    /* 0x9C */ SpiderThreadHangInfo* mShootHangInfo;
    /* 0xA0 */ s32 mNumRadialLines;
    /* 0xA4 */ SpiderThreadRadialLine** mRadialLines;
    /* 0xA8 */ SpiderThreadMainPoint** mAnchorPoints;
    /* 0xAC */ s32 mRadialPartsBufferSize;
    /* 0xB0 */ s32 mNumRadialParts;
    /* 0xB4 */ SpiderThreadPart** mRadialParts;
    /* 0xB8 */ s32 mCirclePartsBufferSize;
    /* 0xBC */ s32 mNumCircleParts;
    /* 0xC0 */ SpiderThreadPart** mCircleParts;
    /* 0xC4 */ s32 mCutPointsBufferSize;
    /* 0xC8 */ s32 mNumCutPoints;
    /* 0xCC */ SpiderThreadMainPoint** mCutPoints;
    /* 0xD0 */ TVec3f mFront;
    /* 0xDC */ SpiderThreadWindCtrl* mWindCtrl;
    /* 0xE0 */ bool mIsBloomOn;
    /* 0xE4 */ JUTTexture* mThreadTexture;
    /* 0xE8 */ JUTTexture* mIndirectTexture;
};

namespace MR {
    void initSpiderThread(const TVec3f&);
    void appearSpiderThread();
    void startSpiderThreadBattle();
    void startSpiderThreadChance();
    void startSpiderThreadBattleEnd();
    void pauseOffSpiderThread();
    f32 getSpiderThreadPosZ();
    void startActorBindToSpiderThread(LiveActor*, const TVec3f**, const TVec3f**, const TVec3f**, const TVec3f&, const TVec3f&);
    bool sendMsgToSpiderThread(u32, HitSensor*);
    bool touchActorToSpiderThread(const TVec3f&, const TVec3f&);
    void tryPushSpiderThread(const TVec3f&, f32);
    void onSpiderThreadBloom();
    void drawSpiderThreadBloom();
};  // namespace MR
