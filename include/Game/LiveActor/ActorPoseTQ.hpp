#pragma once

#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class ActorPoseTQ : public TQuat4f {
public:
    ActorPoseTQ();

    TVec3f mFront;
};
