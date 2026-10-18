#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class ClipDrawArea : public AreaObj {
public:
    ClipDrawArea(const char*);

    virtual void init(const JMapInfoIter&);
};
