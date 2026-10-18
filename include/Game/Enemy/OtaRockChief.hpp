#pragma once

#include "Game/LiveActor/PartsModel.hpp"

class AnimScaleController;
class OtaRockTank;

/// @brief Cosmetic head ("chief") model riding on the OtaRockTank. SMG2-only (owner unit: OtaRockChief.cpp).
class OtaRockChief : public PartsModel {
public:
    OtaRockChief(OtaRockTank* pHost, const char* pName);

    virtual ~OtaRockChief();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    void startHitReaction();

    /* 0xA0 */ AnimScaleController* mAnimScaleCtrl;
};
