#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <JSystem/JKernel/JKRHeap.hpp>

MarioScreenStrip::MarioScreenStrip() {
    mBuffers[0] = new (0x20) u8[MR::getFrameBufferWidth() * 8];
    mBuffers[1] = new (0x20) u8[MR::getFrameBufferWidth() * 8];
    mEnabled = false;
    mProgress = 0.0f;
    mMode = 0;
    mBlend = 0.0f;
}

void MarioScreenStrip::fn_803DB9D0(s32 progress, s16 mode, s32 blend) {
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
