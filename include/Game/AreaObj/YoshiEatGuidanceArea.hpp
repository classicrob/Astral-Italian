#pragma once

#include "Game/AreaObj/AreaObj.hpp"

namespace MR {
    void requestYoshiEatGuidance(bool);
};  // namespace MR

class YoshiEatGuidanceArea : public AreaObj {
public:
    YoshiEatGuidanceArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();
};
