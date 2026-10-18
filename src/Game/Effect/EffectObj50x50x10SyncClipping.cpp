#include "Game/Effect/EffectObj50x50x10SyncClipping.hpp"

bool EffectObj50x50x10SyncClipping::isSyncClipping() const {
    return true;
}

// NONMATCHING: target calls the out-of-line JGeometry::TVec3<f>(f32,f32,f32) ctor; ours inlines it (shared TVec.hpp, lead decision)
TVec3f* EffectObj50x50x10SyncClipping::getClippingCenterOffset() const {
    return &TVec3f(0.0f, 200.0f, 0.0f);
}

f32 EffectObj50x50x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj50x50x10SyncClipping::getClippingRadius() const {
    return 2500.0f;
}
