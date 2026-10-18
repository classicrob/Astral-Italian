#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class SceneChangeArea : public AreaObj {
public:
    SceneChangeArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();
};
