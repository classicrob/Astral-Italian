#pragma once

#include "Game/Effect/SimpleEffectObj.hpp"

class EffectObj50x50x10SyncClipping : public SimpleEffectObj {
public:
    EffectObj50x50x10SyncClipping(const char* pName) : SimpleEffectObj(pName) {
    }

    virtual ~EffectObj50x50x10SyncClipping();
    virtual bool isSyncClipping() const;
    virtual TVec3f* getClippingCenterOffset() const;
    virtual f32 getFarClipDistance() const;
    virtual f32 getClippingRadius() const;
};
