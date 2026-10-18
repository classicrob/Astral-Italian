#pragma once

#include "Game/NameObj/NameObj.hpp"

class FallOutFieldDraw : public NameObj {
public:
    /// @brief Creates a new `FallOutFieldDraw`.
    /// @param pName A pointer to the null-terminated name of the object.
    FallOutFieldDraw(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void draw() const;

    void setUpFillScreen() const;
    void setUpEdgeAndClearAlpha() const;
    void activate();
    void deactivate();

    /* 0x14 */ u32 _14;
    /* 0x18 */ bool _C;
    /* 0x1C */ u32 _1C;
    /* 0x20 */ bool _20;
};

namespace MR {
    NameObj* createFallOutFieldDraw(const char*);
};  // namespace MR

#include <revolution.h>
namespace MR {
    void setFallOutFieldDrawEdgeColor(const GXColor&);
    void resetFallOutFieldDrawEdgeColor();
}
