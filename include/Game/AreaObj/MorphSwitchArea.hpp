#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class MorphSwitchArea : public AreaObj {
public:
    MorphSwitchArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    bool checkCondition() const;

    /* 0x48 */ s32 mMorphType;
};
