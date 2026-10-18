#pragma once

#include "Game/Effect/SimpleEffectObj.hpp"

class EffectObjR1000F50 : public SimpleEffectObj {
public:
    EffectObjR1000F50(const char* pName) : SimpleEffectObj(pName) {
    }

    virtual ~EffectObjR1000F50();
    virtual f32 getFarClipDistance() const;
    virtual f32 getClippingRadius() const;
};
