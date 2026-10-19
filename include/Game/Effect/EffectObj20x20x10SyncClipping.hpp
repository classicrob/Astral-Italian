#pragma once

#include "Game/Effect/SimpleEffectObj.hpp"

class EffectObj20x20x10SyncClipping : public SimpleEffectObj {
public:
    EffectObj20x20x10SyncClipping(const char* pName) : SimpleEffectObj(pName) {
    }

    virtual ~EffectObj20x20x10SyncClipping();
    virtual bool isSyncClipping() const;
    virtual TVec3f* getClippingCenterOffset() const;
    virtual f32 getFarClipDistance() const;
    virtual f32 getClippingRadius() const;
};
