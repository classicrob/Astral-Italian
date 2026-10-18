#pragma once

#include "Game/AreaObj/AreaObj.hpp"
#include "Game/Util/StarPointerUtil.hpp"

class YoshiEatGuidanceArea : public AreaObj {
public:
    YoshiEatGuidanceArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();
};
