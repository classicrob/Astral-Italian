#pragma once

#include "Game/Effect/SimpleEffectObj.hpp"

class EffectObjR500F50 : public SimpleEffectObj {
public:
    EffectObjR500F50(const char* pName) : SimpleEffectObj(pName) {
    }

    virtual ~EffectObjR500F50() {
    }
    virtual f32 getFarClipDistance() const { return 50.0f; }
    virtual f32 getClippingRadius() const { return 500.0f; }
};
