#pragma auto_inline off
#include "Game/MapObj/SpinDriverUtil.hpp"
#include "Game/Util.hpp"
// NONMATCHING: temporary pointers and saved-register allocation differ.
void SpinDriverUtil::setShadowAndClipping(LiveActor* actor, TVec3f* center, f32 length, f32 radius, f32* clippingRadius) {
    TVec3f start, position(actor->mPosition), gravity(actor->mGravity), end;
    start.set<f32>(position - gravity * radius);
    if (length < 0.0f) {
        if (MR::getFirstPolyOnLineToMap(&end, 0, start, gravity * (1500.0f + radius))) {
            f32 distance = PSVECDistance(&end, &position);
            distance += radius;
            length = distance;
            radius = 0.5f * length + radius;
        } else { end.set<f32>(position); length = 0.0f; }
    } else {
        end.set<f32>(position + gravity * length);
        radius = 0.5f * length + radius;
    }
    center->set<f32>((end + position) * 0.5f);
    MR::setClippingTypeSphere(actor, radius, center);
    if (clippingRadius) *clippingRadius = radius;
    MR::setShadowDropDirection(actor, 0, gravity);
    MR::setShadowDropLength(actor, 0, length);
}
