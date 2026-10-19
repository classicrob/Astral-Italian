#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Effect/EffectObj50x50x10SyncClipping.hpp"

bool EffectObj50x50x10SyncClipping::isSyncClipping() const {
    return true;
}

TVec3f* EffectObj50x50x10SyncClipping::getClippingCenterOffset() const {
    return &TVec3f(0.0f, 200.0f, 0.0f);
}

f32 EffectObj50x50x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj50x50x10SyncClipping::getClippingRadius() const {
    return 2500.0f;
}
