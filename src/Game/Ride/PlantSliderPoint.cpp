#define JGEOMETRY_VEC3_MI_NOINLINE
#define JGEOMETRY_OUTLINE_MATRIX_SET
#include "Game/Ride/PlantSlider.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PlantRailInfo.hpp"
#include "Game/MapObj/StarPieceFlower.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include "Game/Util/Color.hpp"
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXVert.h>
#include <revolution/gx/GXTransform.h>
#include <revolution/gx/GXTev.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/mtx.h>
#include <revolution/types.h>



PlantSliderPoint::PlantSliderPoint(const TVec3f& pPos, const TVec3f& pUp, const TVec3f& pAxis, f32 thickness)
    : PlantPoint(pPos, pUp, thickness, pAxis), mBasePos(pPos), mVelocity(0.0f, 0.0f, 0.0f), mTargetPos(pPos) {
}

#pragma dont_inline on
// NONMATCHING: vector temporary stack placement differs.
void PlantSliderPoint::updateSpring(f32 pScale, f32 pDamping) {
    mVelocity.scale(pDamping);
    TVec3f diff = mBasePos - mPosition;
    TVec3f offset(diff);
    offset.scale(pScale);
    mVelocity += offset;
    mPosition += mVelocity;
    mTargetPos.x = mPosition.x;
    mTargetPos.y = mPosition.y;
    mTargetPos.z = mPosition.z;
}

#pragma dont_inline reset

// Vector operators are expanded differently by the compiler; the spring and blend operands follow the target.
// NONMATCHING: vector temporary stack placement and operator+= inlining differ.
void PlantSliderPoint::updateSprings(f32 pHeight, f32 pInterp, f32 pDamping, f32 pAccel) {
    TVec3f base(mBasePos);
    base.y -= pHeight;
    mVelocity.scale(pAccel);
    TVec3f offset = (mBasePos - mPosition).scaleInline(pDamping);
    mVelocity += offset;
    mTargetPos += mVelocity;
    TVec3f pos = base.scaleInline(pInterp) + mTargetPos.scaleInline(1.0f - pInterp);
    mPosition = pos;
}
