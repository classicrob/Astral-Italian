#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class JUTTexture;

class MarioWallShadow {
public:
    MarioWallShadow();
    void draw(JUTTexture*);

    u8 mActive;
    TVec3f mPosition;
    TVec3f mNormal;
    f32 mDistance;
};
