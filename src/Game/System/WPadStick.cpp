#include "Game/System/WPadStick.hpp"
#include "Game/System/WPad.hpp"
#include <JSystem/JGeometry/TUtil.hpp>

#define STICK_FLAG_NONE 0x0
#define STICK_FLAG_UP 0x1
#define STICK_FLAG_DOWN 0x2
#define STICK_FLAG_RIGHT 0x4
#define STICK_FLAG_LEFT 0x8

namespace {
    static const f32 sIsOnValue = 0.2f;
};  // namespace

WPadStick::WPadStick(const WPad* pPad)
    : mPad(pPad), mStick(0.0f, 0.0f), mSpeed(), mHold(), mTrigger(), mRelease(), mIsTriggerUp(), mIsTriggerDown(), mUpRepeatFrame(), mDownRepeatFrame(), mIsHoldUp(), mIsHoldDown() {
}

void WPadStick::update() {
    KPADStatus* pStatus = mPad->getKPadStatus(0);

    if (!MR::isDeviceFreeStyle(pStatus)) {
        return;
    }

    TVec2f stickPrev(mStick);
    mStick.set(pStatus->ex_status.fs.stick.x, pStatus->ex_status.fs.stick.y);
    mSpeed = mStick.distance(stickPrev);
    u32 flagPrev = STICK_FLAG_NONE;

    if (stickPrev.x > ::sIsOnValue) {
        flagPrev |= STICK_FLAG_RIGHT;
    }

    if (stickPrev.x < -::sIsOnValue) {
        flagPrev |= STICK_FLAG_LEFT;
    }

    if (stickPrev.y > ::sIsOnValue) {
        flagPrev |= STICK_FLAG_UP;
    }

    if (stickPrev.y < -::sIsOnValue) {
        flagPrev |= STICK_FLAG_DOWN;
    }

    u32 flag = STICK_FLAG_NONE;

    if (mStick.x > ::sIsOnValue) {
        flag |= STICK_FLAG_RIGHT;
    }

    if (mStick.x < -::sIsOnValue) {
        flag |= STICK_FLAG_LEFT;
    }

    if (mStick.y > ::sIsOnValue) {
        flag |= STICK_FLAG_UP;
    }

    if (mStick.y < -::sIsOnValue) {
        flag |= STICK_FLAG_DOWN;
    }

    mHold = flag;
    mTrigger = flag & ~flagPrev;
    mRelease = flagPrev & ~flag;

    if (mHold & STICK_FLAG_UP) {
        if (mUpRepeatFrame == 0) {
            mIsTriggerUp = true;
        } else if (mUpRepeatFrame - 25 > 0 && (mUpRepeatFrame - 25) % 5 == 0) {
            mIsTriggerUp = true;
        } else {
            mIsTriggerUp = false;
        }
        mUpRepeatFrame++;
    } else {
        mUpRepeatFrame = 0;
        mIsTriggerUp = false;
    }

    if (mHold & STICK_FLAG_DOWN) {
        if (mDownRepeatFrame == 0) {
            mIsTriggerDown = true;
        } else if (mDownRepeatFrame - 25 > 0 && (mDownRepeatFrame - 25) % 5 == 0) {
            mIsTriggerDown = true;
        } else {
            mIsTriggerDown = false;
        }
        mDownRepeatFrame++;
    } else {
        mDownRepeatFrame = 0;
        mIsTriggerDown = false;
    }
}

bool WPadStick::isChanged() const {
    return mSpeed > 0.0001f;
}
