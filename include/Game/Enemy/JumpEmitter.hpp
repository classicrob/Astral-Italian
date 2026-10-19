#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class ActorStateParamScale;
class MultiEventCamera;
class PartsModel;

// SMG2-only unit (JumpEmitter: camera-event emitter used by JumpGuarder). LiveActor = 0x90 bytes.
class JumpEmitter : public LiveActor {
public:
    JumpEmitter(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void movement();

    void startEventCamera();
    void updateEventCamera();
    void endEventCamera();
    void updateRotate();

public:
    /* 0x90 */ MtxPtr mBodyJointMtx;
    /* 0x94 */ TPos3f mHeadHostMtx;
    /* 0xC4 */ PartsModel* mHeadModel;
    /* 0xC8 */ ActorStateParamScale* mParamScale;
    /* 0xCC */ TVec3f mStartPos;
    /* 0xD8 */ bool mIsEventCamera;
    /* 0xDC */ ActorCameraInfo* mCameraInfo;
    /* 0xE0 */ MultiEventCamera* mMultEventCamera;
};

namespace MR {
    bool enableGroupAttack(LiveActor*, f32, f32);
};  // namespace MR
