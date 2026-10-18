#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class Meister;
class TalkMessageCtrl;
class LiveActorGroup;
class ActorCameraInfo;
class MarioFaceShipNormalConversationExecutor : public LiveActor {
public:
    struct Param {
        Param()
            : mTalkIndex(-1), mFlagType(0), mStageName(0), mOrStageName(0), mScenarioNo(-1), mOrScenarioNo(-1), mPowerStarNum(-1),
              mWatchedEventNo(-1), mTalkCtrl(0), mActorGroup(0) {}

        /* 0x00 */ s32 mTalkIndex;
        /* 0x04 */ const char* mFlagType;
        /* 0x08 */ const char* mStageName;
        /* 0x0C */ const char* mOrStageName;
        /* 0x10 */ s32 mScenarioNo;
        /* 0x14 */ s32 mOrScenarioNo;
        /* 0x18 */ s32 mPowerStarNum;
        /* 0x1C */ s32 mWatchedEventNo;
        /* 0x20 */ TalkMessageCtrl* mTalkCtrl;
        /* 0x24 */ LiveActorGroup* mActorGroup;
    };
    MarioFaceShipNormalConversationExecutor(const char*);
    virtual ~MarioFaceShipNormalConversationExecutor();
    virtual void init(const JMapInfoIter&);
    void initConversation(Param*, Meister*);
    bool tryStartDemo();
    bool isDemoActive() const;
    bool isDemoEnd() const;
    void startCamera();
    void exeCameraOpen();
    void exeCameraClose();
    void exeTalk();
    Meister* mMeister; // 0x90
    s32 mTalkIndex;
    LiveActorGroup* mActorGroup;
    void* _9C;
    TalkMessageCtrl* mTalkCtrl;
    ActorCameraInfo* mDefaultCamera;
    ActorCameraInfo* mUpperCamera;
    ActorCameraInfo* mItemPlanetCamera;
    bool mIsNpcIntro;
    bool mIsItemPlanet;
};
