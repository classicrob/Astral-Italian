#pragma once

#include "Game/AreaObj/AreaObj.hpp"

namespace MR {
    void createSwingActionWatcher();
};  // namespace MR

class SpinSwitchArea : public AreaObj {
public:
    SpinSwitchArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();
};
