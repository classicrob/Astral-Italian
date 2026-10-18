#pragma once

#include "Game/NameObj/NameObj.hpp"

class LiveActor;
class LiveActorGroup;

/// SMG2: coordinates the actors of the co-op "support play" group (SceneObj_SupportPlayDirector).
class SupportPlayDirector : public NameObj {
public:
    SupportPlayDirector();

    virtual ~SupportPlayDirector();
    virtual void init(const JMapInfoIter&);
    virtual void movement();

    void registerActor(LiveActor*);
    void sendMsgGroupStart(LiveActor*);
    void sendMsgGroupStop(LiveActor*);
    bool isActive() const;

    /* 0x14 */ LiveActorGroup* mGroup;
    /* 0x18 */ s32 _18;
    /* 0x1C */ s32 mActiveCount;
    /* 0x20 */ s32 _20;
};
