#pragma once
#include "Game/NPC/NPCActor.hpp"
class NerveExecutor;
class MeisterTalkHolder;
class Meister : public NPCActor {
public:
    Meister(const char*);
    virtual ~Meister();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    const TVec3f& getTalkOffset() const;
    void refreshEventTalk();
    void startDemoState(NerveExecutor*);
    void startWait();
    void resetPose();
    void endDemoState();
    void exeWait();
    void exeTalk();
    void exeReaction();
    bool tryReaction() NO_INLINE;
    void updateTalkDistance() NO_INLINE;
    u8 _164;
    NerveExecutor* mDemoState;
    MeisterTalkHolder* mTalkControl;
};

class MeisterTalkHolder {
public:
    MeisterTalkHolder(Meister*, const JMapInfoIter&, const TVec3f&);
    TalkMessageCtrl* getTalkCtrl() NO_INLINE;
    void nextTalk();
    void removeInvalidTalk();
    bool branchFunc(u32);
    TalkMessageCtrl* getEventTalk() NO_INLINE;
    TalkMessageCtrl* getGeneralTalk() NO_INLINE;
    TalkMessageCtrl* getReactionTalk() NO_INLINE;
    bool isGeneralTalkEnabled(s32) NO_INLINE;
    Meister* mHost;
    TalkMessageCtrl* mMainTalk;
    TalkMessageCtrl* mReactionTalk;
    TalkMessageCtrl* mDefaultTalk;
    TalkMessageCtrl** mGeneralTalks;
    s32 mGeneralTalkCount;
    TalkMessageCtrl** mEventTalks;
    s32 mEventTalkCount;
    u8* mGeneralTalkFlags;
    s32 mGeneralFlagCount;
    s32 mTalkIndex;
    s32 _2C;
    s32 _30;
    s32 mStarCount;
    bool _38;
};

extern TVec3f gMeisterTalkOffset;
