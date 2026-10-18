#pragma once

#include "Game/AreaObj/AreaObj.hpp"

namespace MR {
    void createShadowColorSetter();
};  // namespace MR

class ShadowColorArea : public AreaObj {
public:
    ShadowColorArea(const char*);

    virtual void init(const JMapInfoIter&);
};
