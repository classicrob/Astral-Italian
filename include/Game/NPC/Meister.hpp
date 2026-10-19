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
    /* 0x164 */ u8 mWasHitByStarPiece;
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
    bool isGeneralTalkInitiallyEnabled(s32);
    /* 0x00 */ Meister* mHost;
    /* 0x04 */ TalkMessageCtrl* mMainTalk;
    /* 0x08 */ TalkMessageCtrl* mReactionTalk;
    /* 0x0C */ TalkMessageCtrl* mDefaultTalk;
    /* 0x10 */ TalkMessageCtrl** mGeneralTalks;
    /* 0x14 */ s32 mGeneralTalkCount;
    /* 0x18 */ TalkMessageCtrl** mEventTalks;
    /* 0x1C */ s32 mEventTalkCount;
    /* 0x20 */ u8* mGeneralTalkFlags;
    /* 0x24 */ s32 mGeneralFlagCount;
    /* 0x28 */ s32 mTalkIndex;
    /* 0x2C */ s32 mHungryLumaId;
    /* 0x30 */ s32 mRequiredStarPieces;
    /* 0x34 */ s32 mStarCount;
    /* 0x38 */ bool mIsEvenVisit;
};

extern TVec3f gMeisterTalkOffset;

extern const char* gMeisterGeneralTalkMessages[15];
