#pragma once
#include "Game/NameObj/NameObj.hpp"
class LiveActor;
class MarioFaceShipNpcRegister : public NameObj {
public:
    MarioFaceShipNpcRegister(LiveActor*, const char*, const char*);
    virtual ~MarioFaceShipNpcRegister();
    virtual void init(const JMapInfoIter&);
    LiveActor* mActor;
    const char* mActorName;
    s32 mTalkIndex;
};
namespace MR {
    void registerMarioFaceShipNpc(LiveActor*, const JMapInfoIter&);
}
