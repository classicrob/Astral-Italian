#pragma once

#include "Game/NPC/NPCActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class JointController;
class JointControllerInfo;

class BombHeiRed : public NPCActor {
public:
    BombHeiRed(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void control();

    bool calcZenmaiRotation(TPos3f*, const JointControllerInfo&);

    // NPCActor ends at 0x164; the fields below are BombHeiRed's own.
    /* 0x164 */ JointController* mJointController;
    /* 0x168 */ f32 mZenmaiAngle;
};
