#define JGEOMETRY_VEC3_WMR_OUTLINE
#include "Game/Effect/EffectObj20x20x10SyncClipping.hpp"
#include "Game/Effect/EffectObjR1500F400.hpp"
#include "Game/Effect/EffectObjR1000F50.hpp"

bool EffectObj20x20x10SyncClipping::isSyncClipping() const {
    return true;
}

TVec3f* EffectObj20x20x10SyncClipping::getClippingCenterOffset() const {
#ifdef GALAXY_NATIVE
    static TVec3f offset(0.0f, 200.0f, 0.0f);
    return &offset;
#else
    return &TVec3f(0.0f, 200.0f, 0.0f);
#endif
}

f32 EffectObj20x20x10SyncClipping::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObj20x20x10SyncClipping::getClippingRadius() const {
    return 1000.0f;
}

f32 EffectObjR1500F400::getFarClipDistance() const {
    return 400.0f;
}

f32 EffectObjR1500F400::getClippingRadius() const {
    return 1500.0f;
}

f32 EffectObjR1000F50::getFarClipDistance() const {
    return 50.0f;
}

f32 EffectObjR1000F50::getClippingRadius() const {
    return 1000.0f;
}
