#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class LiveActor;

class ActorStatePlayerPosReset : public ActorStateBase<LiveActor> {
public:
    ActorStatePlayerPosReset(LiveActor*, const char*, bool);
    virtual ~ActorStatePlayerPosReset();
    virtual void appear();
    bool isResetStart();

    const char* mPositionName;
    MtxPtr mPositionMtx;
    TVec3f mPosition;
    bool mUseDemo;
};
