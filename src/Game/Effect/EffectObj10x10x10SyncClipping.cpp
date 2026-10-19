#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Effect/EffectObj10x10x10SyncClipping.hpp"

bool EffectObj10x10x10SyncClipping::isSyncClipping() const {
    return true;
}

TVec3f* EffectObj10x10x10SyncClipping::getClippingCenterOffset() const {
    return &TVec3f(0.0f, 580.0f, 0.0f);
}

f32 EffectObj10x10x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj10x10x10SyncClipping::getClippingRadius() const {
    return 1000.0f;
}
