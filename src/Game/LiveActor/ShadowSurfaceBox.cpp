#include "Game/LiveActor/ShadowSurfaceBox.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowController.hpp"
#include "Game/LiveActor/ShadowSurfaceDrawer.hpp"
#include "Game/Util/DirectDraw.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXVert.h>
#include <revolution/mtx.h>

ShadowSurfaceBox::~ShadowSurfaceBox() {
}

#ifndef GALAXY_NATIVE  // native: ShadowSurfaceDrawer.cpp (DOL split owner) defines these
ShadowSurfaceBox::ShadowSurfaceBox() : ShadowSurfaceDrawer("影描画[水面ボックス]"), mSize(100.0f) {
}

void ShadowSurfaceBox::setSize(const TVec3f& pSize) {
    mSize.set(pSize);
}
#endif

void ShadowSurfaceBox::draw() const {
    TPos3f base;
    ShadowController* controller = getController();

    if (!controller->isProjected() || !controller->isDraw()) {
        return;
    }

    TPos3f modelMtx;
    controller->calcBaseMtx(&base);
    TVec3f side;
    side.set<f32>(base.mMtx[0][0], base.mMtx[1][0], base.mMtx[2][0]);
    TVec3f up;
    up.set<f32>(base.mMtx[0][1], base.mMtx[1][1], base.mMtx[2][1]);
    TVec3f front;
    front.set<f32>(base.mMtx[0][2], base.mMtx[1][2], base.mMtx[2][2]);
    TVec3f normal;
    controller->getProjectionNormal(&normal);
    f32 sideDot = normal.dot(side);
    f32 upDot = normal.dot(up);
    f32 frontDot = normal.dot(front);
    TVec3f size(mSize);
    TVec3f position;

    if (controller->isFollowHostScale()) {
        TVec3f scale(controller->getHost()->mScale);
        scale /= 2.0f;
        size *= scale;
    } else {
        size /= 2.0f;
    }

    side -= normal * sideDot;
    up -= normal * upDot;
    front -= normal * frontDot;
    side.scale(size.x);
    up.scale(size.y);
    front.scale(size.z);
    modelMtx.setXYZDir(side, up, front);
    controller->getProjectionPos(&position);
    modelMtx.setTrans(position + normal);
    TDDraw::setModelMtx(modelMtx.toMtxPtr());

    sideDot = sideDot >= 0.0f ? 1.0f : -1.0f;
    upDot = upDot >= 0.0f ? 1.0f : -1.0f;
    frontDot = frontDot >= 0.0f ? 1.0f : -1.0f;

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(sideDot, -sideDot, -1.0f);
    GXColor1u32(0x80);
    GXPosition3f32(sideDot, -sideDot, 1.0f);
    GXColor1u32(0x80);
    GXPosition3f32(sideDot, sideDot, 1.0f);
    GXColor1u32(0x80);
    GXPosition3f32(sideDot, sideDot, -1.0f);
    GXColor1u32(0x80);
    GXEnd();

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-1.0f, upDot, -upDot);
    GXColor1u32(0x80);
    GXPosition3f32(1.0f, upDot, -upDot);
    GXColor1u32(0x80);
    GXPosition3f32(1.0f, upDot, upDot);
    GXColor1u32(0x80);
    GXPosition3f32(-1.0f, upDot, upDot);
    GXColor1u32(0x80);
    GXEnd();

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-frontDot, -1.0f, frontDot);
    GXColor1u32(0x80);
    GXPosition3f32(-frontDot, 1.0f, frontDot);
    GXColor1u32(0x80);
    GXPosition3f32(frontDot, 1.0f, frontDot);
    GXColor1u32(0x80);
    GXPosition3f32(frontDot, -1.0f, frontDot);
    GXColor1u32(0x80);
    GXEnd();
}
