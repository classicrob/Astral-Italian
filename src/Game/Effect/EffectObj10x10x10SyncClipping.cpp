#include "Game/Effect/EffectObj10x10x10SyncClipping.hpp"

bool EffectObj10x10x10SyncClipping::isSyncClipping() const {
    return true;
}

// NONMATCHING: target calls the out-of-line JGeometry::TVec3<f>(f32,f32,f32) ctor; ours inlines it (shared TVec.hpp, lead decision)
TVec3f* EffectObj10x10x10SyncClipping::getClippingCenterOffset() const {
    return &TVec3f(0.0f, 580.0f, 0.0f);
}

f32 EffectObj10x10x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj10x10x10SyncClipping::getClippingRadius() const {
    return 1000.0f;
}
