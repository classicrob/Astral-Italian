#pragma once
#include "Game/NameObj/NameObj.hpp"
class LiveActor;
class LiveActorGroup;
struct MarioFaceShipEventCast;
struct MarioFaceShipNpcInfo {
    MarioFaceShipNpcInfo();
    s32 mIndex;
    s32 mDemoCastId;
    bool mIsEnabled;
    LiveActor* mActor;
};
class MarioFaceShipNpcHolder : public NameObj {
public:
    MarioFaceShipNpcHolder(const char*);
    virtual ~MarioFaceShipNpcHolder();
    virtual void init(const JMapInfoIter&);
    virtual void movement();
    void registerActor(LiveActor*, s32, const JMapInfoIter&);
    bool isRegistered(const LiveActor*, s32) const;
    void requestMovement();
    void registerGroup(LiveActorGroup*, const MarioFaceShipEventCast*) const;
    MarioFaceShipNpcInfo* mNpcs;
    s32 mCount;
};
namespace MR {
    void createMarioFaceShipNpcHolder();
    void registerMarioFaceShipNpcActor(LiveActor*, s32, const JMapInfoIter&);
    void registerMarioFaceShipNpcGroup(LiveActorGroup*, const MarioFaceShipEventCast*);
    void requestMarioFaceShipNpcMovement();
}
