#pragma once
#include "Game/NameObj/NameObj.hpp"
class LiveActor;
class LiveActorGroup;
class SupportPlayGuidance;
class SupportPlayDirector : public NameObj {
public:
    SupportPlayDirector();
    virtual ~SupportPlayDirector();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    void registerActor(LiveActor*);
    void sendMsgGroupStart();
    void sendMsgGroupStop();
    bool isActive() const;
    void updateGuidance();
    SupportPlayGuidance* mGuidance;
    LiveActorGroup* mGroup;
    s32 mActiveCount;
    s32 mGuidanceFrames;
};
