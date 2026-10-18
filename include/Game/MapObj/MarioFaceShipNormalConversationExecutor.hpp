#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class Meister;
class TalkMessageCtrl;
class LiveActorGroup;
class ActorCameraInfo;
class MarioFaceShipNormalConversationExecutor : public LiveActor {
public:
    struct Param {
        s32 mTalkIndex;
        u8 _4[0x1C];
        TalkMessageCtrl* mTalkCtrl;
        LiveActorGroup* mActorGroup;
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
