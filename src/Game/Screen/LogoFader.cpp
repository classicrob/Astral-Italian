#include "Game/Screen/LogoFader.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"

#define RATE_MIN 0.0f
#define RATE_MAX 1.0f

namespace NrvLogoFader {
    LogoFaderNrvBlank LogoFaderNrvBlank::sInstance;
    LogoFaderNrvDisplay LogoFaderNrvDisplay::sInstance;
    LogoFaderNrvFadeIn LogoFaderNrvFadeIn::sInstance;
    LogoFaderNrvFadeOut LogoFaderNrvFadeOut::sInstance;
}

LogoFader::LogoFader(const char* pName) : LayoutActor(pName, true), _20(true), mMaxStep(30), mRate(0.0f) {
    initNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvDisplay));
    kill();
}

void LogoFader::draw() const {
    if (mRate > RATE_MIN) {
        GXSetColorUpdate(GX_TRUE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);

        GXColor fillColor = {0, 0, 0, mRate * 255.0f};
        MR::fillScreen(fillColor);
    }
}

void LogoFader::exeBlank() {
    if (MR::isFirstStep(this)) {
        mRate = RATE_MAX;
    }
}

void LogoFader::exeDisplay() {
    if (MR::isFirstStep(this)) {
        mRate = RATE_MIN;
    }
}

void LogoFader::exeFadeIn() {
    f32 step = getNerveStep();
    f32 maxStep = mMaxStep;

    mRate = RATE_MAX - (step / maxStep);

    if (mRate < RATE_MIN) {
        mRate = RATE_MIN;
    }

    if (MR::isStep(this, mMaxStep)) {
        setNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvDisplay));
    }
}

void LogoFader::exeFadeOut() {
    f32 step = getNerveStep();
    f32 maxStep = mMaxStep;

    mRate = step / maxStep;

    if (mRate > RATE_MAX) {
        mRate = RATE_MAX;
    }

    if (MR::isStep(this, mMaxStep)) {
        setNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvBlank));
    }
}

void LogoFader::setBlank() {
    setNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvBlank));
}

void LogoFader::startFadeIn() {
    setNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvFadeIn));
}

void LogoFader::startFadeOut() {
    setNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvFadeOut));
}

bool LogoFader::isFadeEnd() const {
    return isNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvDisplay)) || isNerve(GET_NERVE_DIRECT(NrvLogoFader, LogoFaderNrvBlank));
}
