#include "Game/Camera/CameraShakePatternImpl.hpp"
#include "Game/AreaObj/MercatorTransformCube.hpp"
#include "Game/Util/MathUtil.hpp"

CameraShakePatternVerticalSin::CameraShakePatternVerticalSin(f32 intensity, f32 speed) : mIntensity(intensity), mSpeed(speed), mOffset(0.0f, 0.0f) {
}

bool CameraShakePatternVerticalSin::isEnd() const {
    return mFrame >= mSpeed;
}

void CameraShakePatternVerticalSin::getOffset(TVec2f* pOffset) const {
    pOffset->set(mOffset);
}

void CameraShakePatternVerticalSin::start() {
    mOffset.set(0.0f, 0.0f);
}

void CameraShakePatternVerticalSin::update() {
    if (mSpeed < 0.01) {
        mOffset.zero();
        return;
    }

    f32 amount = mIntensity * -MR::sin(TWO_PI * mFrame / mSpeed);

    mOffset.set(0.0f, amount);
}
