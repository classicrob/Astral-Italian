#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Sky object that turns to face or back to the camera depending on the player's gravity (SMG2-only).
class OmoteuLandSky : public LiveActor {
public:
    OmoteuLandSky(const char*);
    virtual ~OmoteuLandSky();

    virtual void init(const JMapInfoIter&);
    virtual void calcAnim();

    void exeFirstWait();
    void exeFaceWait();
    void exeFaceToBack();
    void exeBackWait();
    void exeBackToFace();
    bool isPlayerGravitySameDirection();
};
