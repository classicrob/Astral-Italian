#pragma once

#include "Game/MapObj/BenefitItemObj.hpp"

class BenefitItemLifeUp : public BenefitItemObj {
public:
    BenefitItemLifeUp(const char*);

    virtual void exeCatch();
};
