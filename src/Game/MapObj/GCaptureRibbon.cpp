#include "Game/MapObj/GCaptureRibbon.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorLightCtrl.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/gx/GXEnum.h>
#include <revolution/gx/GXGeometry.h>
#include <revolution/gx/GXPixel.h>
#include <revolution/gx/GXVert.h>
#include <revolution/types.h>

GCaptureRibbon::GCaptureRibbon(const char* pName) : LiveActor(pName), mTexture(), mPointCount(), _998() {
}

void GCaptureRibbon::draw() const {
    if (mPointCount >= 2) {
        TDDraw::cameraInit3D();
        TDDraw::setup(1, 2, 0);
        f32 width = calcLineWidth();
        mTexture->load(GX_TEXMAP0);
        GXSetZMode(true, GX_LEQUAL, false);

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 2 * mPointCount);
        {
            for (s32 i = 0; i < mPointCount; i++) {
                f32 v2 = i / (mPointCount - 1.0f);

                TVec3f v1 = mUpVecs[i] * width;

                GXPosition3f32(v1.x + mPoints[i].x, v1.y + mPoints[i].y, v1.z + mPoints[i].z);
                GXTexCoord2f32(0.0f, v2);
                GXPosition3f32(mPoints[i].x - v1.x, mPoints[i].y - v1.y, mPoints[i].z - v1.z);
                GXTexCoord2f32(1.0f, v2);
            }
        }
        GXEnd();

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 2 * mPointCount);
        {
            for (s32 j = 0; j < mPointCount; j++) {
                f32 v2 = j / (mPointCount - 1.0f);

                TVec3f v1 = mSideVecs[j] * width;

                GXPosition3f32(v1.x + mPoints[j].x, v1.y + mPoints[j].y, v1.z + mPoints[j].z);
                GXTexCoord2f32(0.0f, v2);
                GXPosition3f32(mPoints[j].x - v1.x, mPoints[j].y - v1.y, mPoints[j].z - v1.z);
                GXTexCoord2f32(1.0f, v2);
            }
        }
        GXEnd();
    }
}

void GCaptureRibbon::reset() {
    mPointCount = 0;
    _998 = 0;
}

void GCaptureRibbon::lengthen(const TVec3f& rStart, const TVec3f& rEnd) {
    mPoints[0].set(rStart);
    mPoints[1].set(rEnd);
    mPointCount = 2;
    updateAxis();
}

void GCaptureRibbon::shorten(const TVec3f& rStart, const TVec3f& rEnd) {
    mPoints[0].set(rStart);
    mPoints[1].set(rEnd);
    mPointCount = 2;
    updateAxis();
}

// NONMATCHING: loop index and array pointer lifetimes use one extra saved register.
void GCaptureRibbon::updateAxis() {
    TQuat4f quat;
    if (mPointCount >= 2) {
        TVec3f v1 = mPoints[1] - mPoints[0];
        MR::normalizeOrZero(&v1);

        if (MR::isNearZero(v1)) {
            mSideVecs[0] = TVec3f(1, 0, 0);
            mUpVecs[0] = TVec3f(0, 1, 0);
            v1 = TVec3f(0, 0, 1);
        } else {
            MR::makeAxisVerticalZX(&mSideVecs[0], v1);
            mUpVecs[0].cross(v1, mSideVecs[0]);
        }

        for (s32 i = 1; i < mPointCount - 1; i++) {
            TVec3f v2 = mPoints[i + 1] - mPoints[i];
            if (MR::isNearZero(v2)) {
                mSideVecs[i] = mSideVecs[i - 1];
                mUpVecs[i] = mUpVecs[i - 1];
            } else {
                MR::normalize(&v2);
                if (v1.dot(v2) < 0.0f) {
                    v2 = -v2;
                }
                quat.setRotate(v1, v2);
                quat.transform(mSideVecs[i - 1], mSideVecs[i]);
                quat.transform(mUpVecs[i - 1], mUpVecs[i]);
                v1.set(v2);
            }
        }
        mSideVecs[mPointCount - 1] = mSideVecs[mPointCount - 2];
        mUpVecs[mPointCount - 1] = mUpVecs[mPointCount - 2];
    }
}

f32 GCaptureRibbon::calcLineWidth() const {
    f32 normalize = MR::normalize(mPoints[0].distance(mPoints[mPointCount - 1]), 500.0f, 2000.0f);
    return (20.0f * normalize) + (90.0f * (1.0f - normalize));
}

GCaptureRibbon::~GCaptureRibbon() {
}

void GCaptureRibbon::init(const JMapInfoIter& rIter) {
    mTexture = new JUTTexture(MR::loadTexFromArc("GCaptureRibbon.arc", "GCaptureRibbon.bti", nullptr), 0);
    MR::invalidateClipping(this);
    makeActorAppeared();
}
