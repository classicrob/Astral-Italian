#include "Game/AudioLib/AudEffector.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>

JASDsp::FxlineConfig_ AudEffector::mFxLine[] = {
    {1, 0, 0x0, 0, 3, 0, 0x0, 0x0, {0, 0, 0, 0, 0, 0, 0, 0}},
    {1, 0, 0x2, 0, 1, 0, 0x0, 0x0, {0, 0, 0, 0, 0, 0, 0, 0}},
    {1, 0, 0x0, 0, 5, 0, 0x0, 0x0, {0, 0, 0, 0, 0, 0, 0, 0}},
    {1, 0, 0x4, 0, 1, 0, 0x0, 0x0, {0, 0, 0, 0, 0, 0, 0, 0}},
};

// Pitch override applied by modifySoundPitch (the second float is unused).
f32 sSpecialFxPitch = 0.5f;
f32 sSpecialFxPitchUnused = 0.0f;

namespace {
    // Second line configuration (selected by setFxLineType(1)).
    static JASDsp::FxlineConfig_ sFxLineType1[] = {
        {1, 0, 0x0, 32767, 3, 10000, 0x0, 0x45, {1573, -1861, 1573, -1574, -1288, 2152, 12445, -3864}},
        {1, 0, 0x2, 10000, 1, 32767, 0x0, 0x45, {1573, -1861, 1573, -1574, -1288, 2152, 12445, -3864}},
        {1, 0, 0x0, 32767, 5, 10000, 0x0, 0x64, {1573, -1861, 1573, -1574, -1288, 2152, 12445, -3864}},
        {1, 0, 0x4, 10000, 1, 32767, 0x0, 0x55, {1573, -1861, 1573, -1574, -1288, 2152, 12445, -3864}},
    };
};  // namespace

AudEffector::AudEffector() : mFxSend(), mCutoff(0x7F), mSpecialFx(-1) {
    for (u8 i = 0; i < 4; i++) {
        mFxLines[i] = nullptr;
    }
}

void AudEffector::initFxLine(JKRHeap* pHeap) {
    for (u8 i = 0; i < 4; i++) {
        mFxLines[i] = new (pHeap, 0x20) s16[0x2800];
        JASDsp::setFXLine(i, mFxLines[i], &mFxLine[i]);
    }
}

void AudEffector::setFxLineType(s32 type) {
    if (type == 0) {
        for (u8 i = 0; i < 4; i++) {
            JASDsp::setFXLine(i, mFxLines[i], &mFxLine[i]);
        }
    } else if (type == 1) {
        for (u8 i = 0; i < 4; i++) {
            JASDsp::setFXLine(i, mFxLines[i], &sFxLineType1[i]);
        }
    }
}

void AudEffector::setFxSend(f32 fxSend) {
    if (fxSend < 0.0f) {
        fxSend = 0.0f;
    }
    if (fxSend > 1.0f) {
        fxSend = 1.0f;
    }
    mFxSend = fxSend;
}

void AudEffector::setCutoff(s32 cutoff) {
    if (cutoff < 0 || cutoff > 0x7F) {
        cutoff = 0x7F;
    }
    mCutoff = cutoff;
}

void AudEffector::setSpecialFx(s32 specialFx) {
    mSpecialFx = specialFx;
}

void AudEffector::modifySoundPitch(JAISoundHandle* pHandle, f32* pPitch, u32*) {
    if (pHandle != nullptr) {
        JAISound* pSound = pHandle->mSound;
        if (pSound != nullptr && mSpecialFx == 0) {
            JAISoundID id = pSound->mSoundID;
            if (id.fn_80089670()) {
                *pPitch = sSpecialFxPitch;
            }
        }
    }
}

void AudEffector::update() {}

void AudEffector::initParams() {
    mCutoff = 0x7F;
    mFxSend = 0.0f;
    mSpecialFx = 0;
}
