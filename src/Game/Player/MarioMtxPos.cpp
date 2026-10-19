#define JGEOMETRY_OUTLINE_MATRIX_ROTATION
#pragma auto_inline off
#include "Game/Player/MarioMtxPos.hpp"
#include <JSystem/JGeometry/TQuat.hpp>

MarioMtxPos::MarioMtxPos() {}

MarioMtxPos::MarioMtxPos(TPos3f* src) { mMtx.set(*src); }
MarioMtxPos::MarioMtxPos(const TMtx34f& src) { mMtx.set(src); }

void MarioMtxPos::fn_80409650(const MarioMtxPos& src) { mMtx.concat(src.mMtx); }
// NONMATCHING: the three component getters load floating-point arguments in a different order.
void MarioMtxPos::getTranslation(TVec3f* out) const {
    out->set<f32>(mMtx.mMtx[0][3], mMtx.mMtx[1][3], mMtx.mMtx[2][3]);
}
void MarioMtxPos::getYAxis(TVec3f* out) const {
    out->set<f32>(mMtx.mMtx[0][1], mMtx.mMtx[1][1], mMtx.mMtx[2][1]);
}
void MarioMtxPos::getZAxis(TVec3f* out) const {
    out->set<f32>(mMtx.mMtx[0][2], mMtx.mMtx[1][2], mMtx.mMtx[2][2]);
}
void MarioMtxPos::identity() { mMtx.identity(); }
bool MarioMtxPos::invert() { return mMtx.invert(mMtx); }
void MarioMtxPos::fn_804096E0(TVec3f* v) const { mMtx.mult33(*v, *v); }
void MarioMtxPos::preConcat(const MarioMtxPos& src) { mMtx.concat(src.mMtx, mMtx); }
void MarioMtxPos::fn_80409700(MtxPtr src) { mMtx.set(src); }
void MarioMtxPos::fn_80409710(const TVec3f& from, const TVec3f& to) {
    TQuat4f q;
    q.setRotate(from, to);
    mMtx.setQuat(q);
}
void MarioMtxPos::setTranslation(const TVec3f& v) {
    mMtx.mMtx[0][3] = v.x;
    mMtx.mMtx[1][3] = v.y;
    mMtx.mMtx[2][3] = v.z;
}
