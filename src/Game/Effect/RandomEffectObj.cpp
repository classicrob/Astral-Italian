#include "Game/Effect/RandomEffectObj.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/MathUtil.hpp"

namespace {
    static const s32 sLifeTime = 600;
    static const s32 sRandWidth = 180;
};  // namespace

RandomEffectObj::RandomEffectObj(const char* pName) : SimpleEffectObj(pName), mStep(), mSeStep(-1), mLifeTime(::sLifeTime), mRandWidth(::sRandWidth) {
}

void RandomEffectObj::init(const JMapInfoIter& rIter) {
    SimpleEffectObj::init(rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mLifeTime);
    MR::getJMapInfoArg1NoInit(rIter, &mRandWidth);

    mStep = getRandomTime();
}

void RandomEffectObj::appear() {
    LiveActor::appear();

    mStep = getRandomTime();
}

s32 RandomEffectObj::getRandomTime() const {
    s32 time = MR::repeat(mPosition.z + (mPosition.x + mPosition.y), 0.0f, mRandWidth);
    return time < 0 ? -time : time;
}

void RandomEffectObj::control() {
    if (!isStateMove()) {
        return;
    }

    if (mStep < 0) {
        return;
    }

    if (mSeStep >= 0) {
        mSeStep++;

    }

    mStep--;

    if (mStep != 0) {
        return;
    }

    MR::emitEffect(this, _8C);


    mStep = MR::getRandom(mLifeTime - mRandWidth, mLifeTime + mRandWidth);
    mSeStep = 0;
}

f32 RandomEffectObj::getFarClipDistance() const {
    return 400.0f;
}

f32 RandomEffectObj::getClippingRadius() const {
    return 1500.0f;
}
