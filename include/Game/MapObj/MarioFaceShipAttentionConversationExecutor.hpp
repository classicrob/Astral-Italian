#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;
class LiveActorGroup;
class Meister;
struct MarioFaceShipEventCast;
class TalkMessageCtrl;

class MarioFaceShipAttentionConversationExecutor : public LiveActor {
public:
    /// Parameter block passed to initConversation (layout from the target asm).
    struct Param {
        Param()
            : mTalkIndex(-1), mIsArg4(false), mIsArg5(false), mKind(0), mStageName(0), mOrStageName(0), mScenarioNo(-1), mOrScenarioNo(-1),
              mPowerStarNum(-1), mWatchedEventNo(-1), mTargetActor(0), mCameraInfo(0), mTalkCtrl(0), mDemoIndex(0), mActorGroup(0), mCast(0) {}

        /* 0x00 */ s32 mTalkIndex;
        /* 0x04 */ bool mIsArg4;
        /* 0x05 */ bool mIsArg5;
        /* 0x06 */ u8 _06[2];
        /* 0x08 */ const char* mKind;
        /* 0x0C */ const char* mStageName;
        /* 0x10 */ const char* mOrStageName;
        /* 0x14 */ s32 mScenarioNo;
        /* 0x18 */ s32 mOrScenarioNo;
        /* 0x1C */ s32 mPowerStarNum;
        /* 0x20 */ s32 mWatchedEventNo;
        /* 0x24 */ LiveActor* mTargetActor;
        /* 0x28 */ ActorCameraInfo* mCameraInfo;
        /* 0x2C */ TalkMessageCtrl* mTalkCtrl;
#ifdef GALAXY_NATIVE
        const char* mDemoIndex;
#else
        /* 0x30 */ s32 mDemoIndex;
#endif
        /* 0x34 */ LiveActorGroup* mActorGroup;
        /* 0x38 */ MarioFaceShipEventCast* mCast;
    };

    MarioFaceShipAttentionConversationExecutor(const char*);

    virtual ~MarioFaceShipAttentionConversationExecutor();
    virtual void init(const JMapInfoIter&);

    void initConversation(Param* pParam, Meister* pMeister);
    bool tryStartDemo();
    bool isWaitingNerve() const;
    bool isNerveDemoEnd() const;
    void setPositions(const char* pMeisterName, const char* pMarioName);

    void exeDemoStart();
    void exeTalkBegin();
    void exeCameraEnd();
    void exeTalkEnd();
    void exeDemoClose();
    void exeTalkHide();
    void exeTalkShow();
    void exeCameraClose();
    void exeWaitDemo();
    void exeWaitTimeKeep();
    void exeResetPose();

    /* 0x90 */ Meister* mMeister;
    /* 0x94 */ s32 mTalkIndex;
    /* 0x98 */ LiveActor* mTargetActor;
    /* 0x9C */ LiveActorGroup* mActorGroup;
#ifdef GALAXY_NATIVE
    const char* mDemoIndex;
#else
    /* 0xA0 */ s32 mDemoIndex;
#endif
    /* 0xA4 */ ActorCameraInfo* mCameraInfo;
    /* 0xA8 */ TalkMessageCtrl* mTalkCtrl;
    /* 0xAC */ ActorCameraInfo* mCameraInfoDefault;
    /* 0xB0 */ ActorCameraInfo* mCameraInfoTalk;
    /* 0xB4 */ ActorCameraInfo* mCameraInfoTalkUp;
    /* 0xB8 */ ActorCameraInfo* mCameraInfoItemPlanet;
    /* 0xBC */ bool mIsDefaultCamera;
    /* 0xBD */ bool mIsNpcIntro;
    /* 0xBE */ bool mIsItemPlanet;
    /* 0xBF */ bool mIsArg4;
    /* 0xC0 */ bool mIsArg5;
};
