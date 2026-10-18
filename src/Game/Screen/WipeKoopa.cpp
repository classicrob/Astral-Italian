#include "Game/Screen/WipeKoopa.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>

namespace {
    static const s32 sAnimFrame = 90;
};  // namespace

namespace {
    NEW_NERVE(WipeKoopaNrvWait, WipeKoopa, Wait);
    NEW_NERVE(WipeKoopaNrvWipeOut, WipeKoopa, WipeOut);
    NEW_NERVE(WipeKoopaNrvClose, WipeKoopa, Close);
};  // namespace

WipeKoopa::WipeKoopa() : WipeLayoutBase("クッパ"), mFrame(::sAnimFrame) {
}

void WipeKoopa::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE_ANON(WipeKoopaNrvWait));
    initLayoutManager("WipeKoopa", 2);
}

void WipeKoopa::exeWait() {
}

void WipeKoopa::exeWipeOut() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "out", 0);

        if (mFrame > 0) {
            MR::setAnimRate(this, static_cast< f32 >(MR::getAnimCtrl(this, 0)->getEnd()) / mFrame, 0);
        }

        MR::showLayout(this);
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_ANON(WipeKoopaNrvClose));
    }
}

void WipeKoopa::exeClose() {
}

void WipeKoopa::wipe(s32 frame) {
    setNerve(GET_NERVE_ANON(WipeKoopaNrvWipeOut));
    setColor();
    MR::hideLayout(this);

    if (frame > 0) {
        mFrame = frame;
    } else {
        mFrame = ::sAnimFrame;
    }
}

void WipeKoopa::forceClose() {
    setNerve(GET_NERVE_ANON(WipeKoopaNrvClose));

    MR::showLayout(this);
    
    MR::startAnim(this, "out", 0);
    MR::setAnimFrameAndStop(this, MR::getAnimCtrl(this, 0)->getEnd(), 0);
    setColor();
}

void WipeKoopa::forceOpen() {
    setNerve(GET_NERVE_ANON(WipeKoopaNrvWait));
    MR::hideLayout(this);
    setColor();
}

bool WipeKoopa::isOpen() const {
    return isNerve(GET_NERVE_ANON(WipeKoopaNrvWait));
}

bool WipeKoopa::isClose() const {
    return isNerve(GET_NERVE_ANON(WipeKoopaNrvClose));
}

bool WipeKoopa::isWipeIn() const {
    return false;
}

bool WipeKoopa::isWipeOut() const {
    return isNerve(GET_NERVE_ANON(WipeKoopaNrvWipeOut));
}

void WipeKoopa::setColor() {
    MR::startAnim(this, "Color", 1);
    if (MR::isPowerStarGreenInCurrentStage()) {
        MR::setAnimFrameAndStop(this, 1.0f, 1);
    } else {
        MR::setAnimFrameAndStop(this, 0.0f, 1);
    }
}
