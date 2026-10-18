#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class TalkMessageCtrl;

class MiniStarCheckPoint : public LiveActor {
public:
    MiniStarCheckPoint(const char* pKind, s32 starNum, s32 partsIndex, s32 param1, s32 worldId);
    virtual ~MiniStarCheckPoint();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();

    void startOpen();
    void resetOpen();
    bool isDone() const;
    bool isStandby() const;
    void talk();
    bool isOpening() const;
    bool isReadyToOpen() const;
    bool update(s32 distance);
    void setEffectDrawOrder(s32 order);
    void setDone();    // was fn_804E5CE0
    void exeOpen();    // was fn_804E5EF0

    const char* mKind;       // 0x90 (name string, compared by isStarCheckPointGrand)
    s32 mStarNum;             // 0x94
    s32 mUnk98;               // 0x98
    s32 mUnk9C;               // 0x9C
    s32 mUnkA0;               // 0xA0
    LiveActor* mChild;        // 0xA4
    TalkMessageCtrl* mTalkCtrl;  // 0xA8
    bool mIsDone;             // 0xAC
};

bool isStarCheckPointGrand(const char* pKind);  // was fn_804E5A10 (defined in MiniPlayerRocket.cpp)
