#pragma once

#include "Game/NameObj/NameObj.hpp"

class AreaObj;

namespace MR {
    GXColor calcShadowColor(u8);
};  // namespace MR

class ShadowColorSetter : public NameObj {
public:
    ShadowColorSetter(const char*);

    virtual void movement();

    /* 0x14 */ AreaObj* mArea;
    /* 0x18 */ GXColor mCurColor;
    /* 0x1C */ GXColor mPrevColor;
    /* 0x20 */ GXColor mTargetColor;
    /* 0x24 */ f32 mBlendT;
};
