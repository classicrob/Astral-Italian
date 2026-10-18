#pragma once

#include "Game/Effect/SimpleEffectObj.hpp"

class EffectObjR1500F400 : public SimpleEffectObj {
public:
    EffectObjR1500F400(const char* pName) : SimpleEffectObj(pName) {
    }

    virtual ~EffectObjR1500F400();
    virtual f32 getFarClipDistance() const;
    virtual f32 getClippingRadius() const;
};
