#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/DirectDraw.hpp"
#include <revolution/gx.h>
#include <JSystem/JKernel/JKRHeap.hpp>

// NONMATCHING: equivalent commutative floating-point operand order.
MarioScreenStripWave::MarioScreenStripWave(f32 amplitude, u32 phase, s32 period, f32 frequency)
    : mAmplitude(amplitude), mPhase(2.0f * JGeometry::TUtil< f32 >::PI() * phase / period), mFrequency(6.2831855f / frequency) {
}

// NONMATCHING: equivalent scalar literal relocation labels.
f32 MarioScreenStripWave::calcOffset(u32 row) {
    f32 angle = MR::sin(mFrequency * row);
    angle *= 3.1415927f;
    angle += mPhase;
    return mAmplitude * MR::sin(angle);
}

MarioScreenStripCurve::MarioScreenStripCurve(u32 position, u32 phase) {
    mPosition = position;
    mPhase = 89.0f * MR::sin(6.2831855f * phase / 120.0f);
    if (mPhase > 0.0f) {
        mPhase = 0.0f;
    }
}

f32 MarioScreenStripCurve::calcOffset(u32 row) {
    return 5.0f * MR::sin(2.0f * (3.1415927f * ((mPosition + row * mPhase) / MR::getScreenHeight())));
}

MarioScreenStrip::MarioScreenStrip() {
    mBuffers[0] = new (0x20) u8[MR::getFrameBufferWidth() * 8];
    mBuffers[1] = new (0x20) u8[MR::getFrameBufferWidth() * 8];
    mEnabled = false;
    mProgress = 0.0f;
    mMode = 0;
    mBlend = 0.0f;
}

void MarioScreenStrip::fn_803DB9D0(s32 progress, s32 mode, s32 blend) {
    mMode = mode;
    mProgress = 0.9f * mProgress + 0.1f * progress;
    mBlend = blend;
    mEnabled = true;
}

void MarioScreenStrip::fn_803DBA40() {
    if (mEnabled) {
        mEnabled = false;
        return;
    }
    mProgress *= 0.9f;
    if (mProgress < 1.0f) {
        mProgress = 0.0f;
        mMode = 0;
    }
    if (mBlend < 1.0f) {
        mBlend = 0.0f;
    }
}

void MarioScreenStrip::fn_803DBAA0(u32 phase) {
    if (mProgress != 0.0f && mBlend != 0.0f && mMode != 0) {
        MarioScreenStripWave offset(mProgress, phase, mMode, mBlend);
        fn_803DBB50(&offset);
    }
}

void MarioScreenStrip::fn_803DBB10(u32 position, u32 phase) {
    MarioScreenStripCurve offset(position, phase);
    fn_803DBB50(&offset);
}

#pragma push
#pragma opt_propagation off
// NONMATCHING: GPR/stack allocation and unsigned-to-float conversion scheduling.
void MarioScreenStrip::fn_803DBB50(MarioScreenStripOffset* offset) {
    GXDrawDone();
    GXTexModeSync();
    GXPixModeSync();
    u16 width = MR::getFrameBufferWidth();
    TDDraw::setup(1, 0, 2);
    GXTexObj textures[2];
    GXInitTexObj(&textures[0], mBuffers[0], width, 4, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXInitTexObj(&textures[1], mBuffers[1], width, 4, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GXSetLineWidth(6, GX_TO_ZERO);
    GXSetTexCopyDst(width, 4, GX_TF_RGB565, GX_FALSE);
    GXSetTexCopySrc(0, 0, width, 4);
    GXCopyTex(mBuffers[0], GX_FALSE);
    u16 current = 0;
    u32 stripHeight = 4;
    f32 texStep = 1.0f / static_cast<f32>(stripHeight);
    for (u32 row = 0; row < MR::getScreenHeight(); row += 4) {
        GXTexModeSync();
        GXPixModeSync();
        GXSetTexCopySrc(0, row + 4, width, 4);
        u32 next = 1 - current;
        GXCopyTex(mBuffers[next], GX_FALSE);
        GXLoadTexObj(&textures[current], GX_TEXMAP0);
        f32 texY = 0.0f;
        for (u32 y = row; y < row + 4; y++) {
            f32 x = offset->calcOffset(y);
            GXBegin(GX_LINES, GX_VTXFMT0, 2);
            GXPosition3f32(x, y, 0.0f);
            GXTexCoord2f32(0.0f, texY);
            GXPosition3f32(x + MR::getScreenWidth(), y, 0.0f);
            GXTexCoord2f32(1.0f, texY);
            texY += texStep;
        }
        current = next;
    }
    GXDrawDone();
}

#pragma pop
