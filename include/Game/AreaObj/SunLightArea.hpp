#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class SunLightArea : public AreaObj {
public:
    SunLightArea(const char*);

    virtual const char* getManagerName() const {
        return "SunLightArea";
    }
};
