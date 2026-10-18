#pragma once
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class LiveActor;
namespace ItemDiceFunction {
    s32 calcFaceIndex(MtxPtr, const TVec3f*);
    void initItemPose(LiveActor*, const TVec3f*, TRot3f*, f32);
}
