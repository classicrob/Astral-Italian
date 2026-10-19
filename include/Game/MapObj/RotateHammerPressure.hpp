#pragma once
#include "Game/MapObj/RotateMoveObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class CollisionParts;

class RotateHammerPressure : public RotateMoveObj {
public:
    RotateHammerPressure(const char*);
    virtual ~RotateHammerPressure();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    /* 0xD0 */ TPos3f mMatrices[4];
    /* 0x190 */ TVec3f mUpDir;
    /* 0x19C */ TVec3f mSideDir;
    /* 0x1A8 */ bool _1A8;
    /* 0x1A9 */ bool _1A9;
    /* 0x1AC */ CollisionParts* mPushCollision;
};
