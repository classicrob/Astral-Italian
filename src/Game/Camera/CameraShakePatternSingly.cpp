#include "Game/Camera/CameraShakePatternImpl.hpp"
#include "Game/AreaObj/MercatorTransformCube.hpp"
#include "Game/Util/MathUtil.hpp"

static const s32 sSinglyMaxFrame = 25;


  // namespace

CameraShakePatternSingly::CameraShakePatternSingly(f32 intensity) : CameraShakePattern(), mIntensity(intensity), mDirection(0.0f, 1.0f) {
}

bool CameraShakePatternSingly::isEnd() const {
    return mFrame >= ::sSinglyMaxFrame;
}

void CameraShakePatternSingly::getOffset(TVec2f* pOffset) const {
    pOffset->set(mOffset);
}

void CameraShakePatternSingly::start() {
    mOffset.zero();
}

void CameraShakePatternSingly::update() {
    s32 framesLeft = ::sSinglyMaxFrame - mFrame;

    if (framesLeft < 0) {
        framesLeft = 0;
    }

    f32 amount = mIntensity * MR::sin(MR::pi() * 4.0f * framesLeft / ::sSinglyMaxFrame);
    amount *= MR::sin(MR::pi() * 0.5f * framesLeft / ::sSinglyMaxFrame);

    mOffset.set(mDirection * amount);
}

void CameraShakePatternSingly::setDirection(const TVec2f& rDir) {
    mDirection.set(rDir);
    MR::normalize(&mDirection);
}
