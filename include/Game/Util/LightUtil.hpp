#pragma once

#include "Game/Util/Color.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class DrawBuffer;
class LiveActor;

namespace MR {
    void loadLight(s32);
    void loadLightPlayer();
    void loadPointLightSub();
    void initActorLightInfoLightType(LiveActor*, s32);
    void initActorLightInfoDrawBuffer(LiveActor*, DrawBuffer*);
    // SMG2: 2nd float = reference distance (<= 0 selects the default); Tico passes -1.0f
    void requestPointLight(const LiveActor*, TVec3f, Color8, f32, s32, f32 = -1.0f);
};  // namespace MR

namespace MR {
    void resetPointLight();
    void requestPointLightPlayer(const LiveActor*, TVec3f, Color8, f32, s32, f32);
}
