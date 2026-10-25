#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <revolution.h>
#include "JSystem/JUtility/TColor.hpp"

class FallOutFieldDraw : public NameObj {
public:
    /// @brief Creates a new `FallOutFieldDraw`.
    FallOutFieldDraw();

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void draw() const;

    void setUpFillScreen() const;
    void setUpEdgeAndClearAlpha() const;
    void activate();
    void deactivate();
    void resetEdgeColor();

    /* 0x14 */ u32 mEdgeIndex;
    /* 0x18 */ u8 mUseTexAlpha;
    /* 0x19 */ u8 _19[3];
    /* 0x1C */ JUtility::TColor mEdgeColor;
    /* 0x20 */ bool mIsEdgeColorOverridden;
};

namespace MR {
    NameObj* createFallOutFieldDraw();
};  // namespace MR

#include <revolution.h>
namespace MR {
    void setFallOutFieldDrawEdgeColor(const GXColor&);
    void resetFallOutFieldDrawEdgeColor();
}
