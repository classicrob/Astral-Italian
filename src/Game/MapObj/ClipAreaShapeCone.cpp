#include "Game/MapObj/ClipAreaShape.hpp"
#include "Game/Util/MtxUtil.hpp"
// NONMATCHING: the shared vector multiply helper currently expands inline.
void ClipAreaShapeCone::calcVolumeMatrix(TPos3f* output, const TPos3f& original, const TVec3f& scale) const {
    output->set(original);
    TVec3f dimensions;
    f32 radius = _8 / 500.0f;
    f32 height = _C / 500.0f;
    dimensions.x = radius;
    dimensions.y = height;
    dimensions.z = radius;
    if (_10 == 0) {
        dimensions.y = -height;
        dimensions.z = -radius;
        MR::addTransMtxLocalY(output->toMtxPtr(), _C * scale.y);
    }
    dimensions.mul(dimensions, scale);
    MR::preScaleMtx(output->toMtxPtr(), dimensions);
}
