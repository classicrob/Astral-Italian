#include "Game/MapObj/ClipAreaShape.hpp"
#include "Game/Util/MtxUtil.hpp"
// NONMATCHING: the matrix copy expands inline instead of calling SMatrix34C::set.
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
    mulScaleVec(dimensions, scale, dimensions);
    MR::preScaleMtx(output->toMtxPtr(), dimensions);
}
