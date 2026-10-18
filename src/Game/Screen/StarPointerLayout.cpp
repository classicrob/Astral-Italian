#include "Game/Screen/StarPointerLayout.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/LayoutCoreUtil.hpp"
#include "Game/Screen/StarPointerBlur.hpp"
#include "Game/Screen/StarPointerCommandStream.hpp"
#include "Game/Screen/StarPointerController.hpp"
#include "Game/Screen/StarPointerDirector.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/System/WPadPointer.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/wpad.h>


namespace {
    const GXColor sYoshiPointerColor = {0xB4, 0x0A, 0x0A, 0xFF};
    // TODO: names
    static GXColor sColorA[] = {
        {0x00, 0x32, 0xFF, 0xFF},
        {0x46, 0x46, 0x00, 0xFF},
        {0x55, 0xFF, 0x00, 0xFF},
        {0x7F, 0x7F, 0x00, 0xFF},
    };

    static GXColor sColorB[] = {
        {0x00, 0x0A, 0x32, 0xFF},
        {0x32, 0x32, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF},
        {0x7F, 0x7F, 0x00, 0xFF},
    };

    static const s32 sTouchCount = 1;
    static const f32 sNormalRadius = 15;
    static const f32 sHoldRadius = 15;
    static const f32 sHandRadius = 20;

    // static const _32 sStepRadian =
    static const s32 hGripTime = 20;
    static const s32 hReleaseTime = 30;
    static const s32 hHideRumbleTime = 30;
    // static const _32 hInvalidateWaitFrame =
    // static const _32 sStarVelZeroRange =
    // static const _32 sStarRateScale =
    // static const _32 sStarRateMax =
    // static const _32 h16x9ScaleX =
    static const f32 sRotateMult = 900.0f;
    // static const f32 sRotateHandMult =
    static const f32 hEndAnimRate = 2.0f;
    static const s32 sBeginHideTime = 0;
    static const f32 sDegreeMin = 0.0f;
};  // namespace

namespace NrvStarPointerLayout {
    NEW_NERVE(HostTypeNrvWait, StarPointerLayout, Wait);
    NEW_NERVE(HostTypeNrvOnReaction, StarPointerLayout, OnReaction);
    NEW_NERVE(HostTypeNrvGrip, StarPointerLayout, Grip);
    NEW_NERVE_ONEND(HostTypeNrvHold, StarPointerLayout, Hold, Hold);
    NEW_NERVE_ONEND(HostTypeNrvHoldTouch, StarPointerLayout, Hold, Hold);
    NEW_NERVE(HostTypeNrvRelease, StarPointerLayout, Release);
    NEW_NERVE(HostTypeNrvOutScreen, StarPointerLayout, OutScreen);
    NEW_NERVE_ONEND(HostTypeNrvNotConnected, StarPointerLayout, Hide, Hide);
    NEW_NERVE(HostTypeNrvHide, StarPointerLayout, Hide);
    NEW_NERVE(HostTypeNrvHideImmediately, StarPointerLayout, Hide);
    NEW_NERVE_ONEND(HostTypeNrvInvalidate, StarPointerLayout, Hide, Hide);
    NEW_NERVE_ONEND(HostTypeNrvInvalidateWait, StarPointerLayout, Hide, Hide);
};  // namespace NrvStarPointerLayout

StarPointerLayout::StarPointerLayout(const char* pName)
    : LayoutActor(pName, true), mPosition(0.0f, 0.0f), mPointerKind(StarPointerKind_NULL), mAnimType(AnimType_HandPaa), mTouchTimer(),
      mPadChannel(-1), mRadius(::sNormalRadius), _48(), mDirector(), mBlur(), mNumber(), mNewTouchedID(), mTouchedID(), mStartTouch(),
      mStartDisableShoot(), mSingleTouch(), mShootDisabled(), mIsPointerValid(true), mIsAppear(true), mAppearTime() {
}

void StarPointerLayout::initWithPort(s32 channel) {
    mPadChannel = channel;
    if (mPadChannel == WPAD_CHAN1) {
        initLayoutManager("DPDPointerSub", 2);
    } else {
        initLayoutManager("DPDPointer", 1);
    }

    MR::createAndAddPaneCtrl(this, "StarPointer", 1);
    MR::createAndAddPaneCtrl(this, "HandPointer", 1);
    MR::createAndAddPaneCtrl(this, "StarFrame", 1);
    MR::createAndAddPaneCtrl(this, "Star", 1);
    if (mPadChannel == WPAD_CHAN0) {
        MR::createAndAddPaneCtrl(this, "PicNozzle", 1);
        MR::createAndAddPaneCtrl(this, "YoshiPointer", 2);
    }
    MR::createAndAddGroupCtrl(this, "GroupRing", 1);

    initNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately));

    mBlur = new StarPointerBlur(new JUTTexture(MR::loadTexFromArc("StarPointerBlur.arc", "Blur.bti", nullptr), 0));


    mBlur->setPortAndColor(mPadChannel, &::sColorA[mPadChannel], &::sColorB[mPadChannel]);

    mNumber = new StarPointerNumber(this, mPadChannel, "スターポインタナンバー");
    mNumber->initWithoutIter();
    mNumber->appear();

    changeLayout(StarPointerKind_StarPointer);
    hideAll();
    appear();
}

void StarPointerLayout::appear() {
    LayoutActor::appear();
    setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
}

void StarPointerLayout::kill() {
    LayoutActor::kill();
}

bool StarPointerLayout::startCommandStream(const LiveActor* pActor, const TVec3f* pPos, bool b) {
    hideBlur();
    mActor = pActor;
    MR::startStarPointerModeCommandStream(this);
    return true;
}

void StarPointerLayout::endCommandStream(const LiveActor* pActor) {
    forceEndCommandStream();
    MR::endStarPointerMode(this);
}

void StarPointerLayout::forceEndCommandStream() {
    mBlur->show();
    mActor = nullptr;
}

bool StarPointerLayout::isCommandStream(const LiveActor* pActor) {
    return mActor == pActor;
}

void StarPointerLayout::exeWait() {
    if (MR::isFirstStep(this)) {
        if (mPointerKind == StarPointerKind_StarPointer) {
            startAnimStarPointer("Wait");
        } else if (mPointerKind == StarPointerKind_HandPointer || mPointerKind == StarPointerKind_2 ||
                   mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor) {
            changeToHandPointer();
            startAnimHandPaa();
        } else if (mPointerKind == StarPointerKind_FingerPointer) {
            startAnimHandFinger();
        } else if (mPointerKind == StarPointerKind_5) {
            startAnimStarPointer("Wait");
        } else if (mPointerKind == StarPointerKind_YoshiPointer) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 0.0f, 1);
            mBlur->show();
        } else if (mPointerKind == StarPointerKind_YoshiPointerNoBlur) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 1.0f, 1);
            mBlur->hideAll();
        }

        if (mPointerKind == StarPointerKind_StarPointerNozzle) {
            MR::showPane(this, "PicNozzle");
            MR::stopPaneAnim(this, "PicNozzle", 0);
            startAnimStarPointer("Wait");

        } else if (mPadChannel == WPAD_CHAN0) {
            MR::hidePane(this, "PicNozzle");
        }
    }

    if (isTouch()) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOnReaction));
    } else {
        if (MR::testCorePadButtonA(mPadChannel)) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip));
        }
    }
}

void StarPointerLayout::exeGrip() {
    if (MR::isFirstStep(this)) {
        if (mPadChannel == WPAD_CHAN1 && isAppear()) {
            MR::startSystemSE("SE_SY_DPD_B_TRIG");
        }

        if (mPointerKind == StarPointerKind_StarPointer) {
            if (mPadChannel == WPAD_CHAN1) {
                startAnimStarPointer("Grip");
            } else {
                startAnimStarPointer("Wait");
            }
        } else if (mPointerKind == StarPointerKind_HandPointer) {
            startAnimHandGuu();
        } else if (mPointerKind == StarPointerKind_2 || mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor) {
            if (mAnimType != AnimType_Finger) {
                startAnimHandGuu();
            }
        } else if (mPointerKind == StarPointerKind_FingerPointer) {
            startAnimHandFinger();
        } else if (mPointerKind == StarPointerKind_5) {
            startAnimHandPaa();
        } else if (mPointerKind == StarPointerKind_YoshiPointer) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 0.0f, 1);
            mBlur->show();
        } else if (mPointerKind == StarPointerKind_YoshiPointerNoBlur) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 1.0f, 1);
            mBlur->hideAll();
        }

        if (mPointerKind == StarPointerKind_StarPointerNozzle) {
            MR::showPane(this, "PicNozzle");
            MR::startPaneAnim(this, "PicNozzle", "Touch", 0);
            startAnimStarPointer("Touch");
        } else {
            if (mPadChannel == WPAD_CHAN0) {
                MR::hidePane(this, "PicNozzle");
            }
        }
    }

    if (mPointerKind == StarPointerKind_2 && mAnimType == AnimType_Finger && !isTouch()) {
        startAnimHandGuu();
    }

    if (!MR::testCorePadButtonA(mPadChannel)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvRelease));
        return;
    }

    if (MR::isGreaterStep(this, ::hGripTime) || MR::isHiddenPane(this, "StarPointer")) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHold));
    }
}

void StarPointerLayout::exeHold() {
    if (MR::isFirstStep(this)) {
        if (mPointerKind == StarPointerKind_StarPointer) {
            if (mPadChannel == WPAD_CHAN1) {
                startAnimStarPointer("Hold");
                mRadius = ::sHoldRadius;
            } else {
                startAnimStarPointer("Wait");
            }
        } else if (mPointerKind == StarPointerKind_HandPointer) {
            startAnimHandGuu();
            mRadius = ::sHandRadius;
        } else if (mPointerKind == StarPointerKind_2 || mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor) {
            if (mAnimType != AnimType_Finger) {
                startAnimHandGuu();
            }
            mRadius = ::sHandRadius;
        } else if (mPointerKind == StarPointerKind_FingerPointer) {
            startAnimHandFinger();
        } else if (mPointerKind == StarPointerKind_5) {
            startAnimHandPaa();
        } else if (mPointerKind == StarPointerKind_YoshiPointer) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 0.0f, 1);
            mBlur->show();
        } else if (mPointerKind == StarPointerKind_YoshiPointerNoBlur) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 1.0f, 1);
            mBlur->hideAll();
        }

        if (mPointerKind == StarPointerKind_StarPointerNozzle) {
            MR::showPane(this, "PicNozzle");
            if (MR::isPaneAnimStopped(this, "PicNozzle", 0)) {
                MR::startPaneAnim(this, "PicNozzle", "Touch", 0);
            }
            startAnimStarPointer("Touch");
        } else {
            if (mPadChannel == WPAD_CHAN0) {
                MR::hidePane(this, "PicNozzle");
            }
        }
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHold))) {
        if (isTouch()) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHoldTouch));
            return;
        }

        if (mPointerKind == StarPointerKind_2 ||
            (mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor && mAnimType == AnimType_Finger)) {
            startAnimHandGuu();
        }
    } else {
        if (!isTouch()) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHold));
            return;
        }
    }

    if (!MR::testCorePadButtonA(mPadChannel)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvRelease));
    }
}

void StarPointerLayout::endHold() {
    tearDownHold();
}

void StarPointerLayout::tearDownHold() {
    if (mPointerKind == StarPointerKind_StarPointer && mPadChannel == WPAD_CHAN1) {
        if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHold)) || isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHoldTouch))) {
            mRadius = ::sHoldRadius;
        } else {
            mRadius = ::sNormalRadius;
        }
    } else if (mPointerKind == StarPointerKind_HandPointer || mPointerKind == StarPointerKind_2) {
        mRadius = ::sHandRadius;
    }
}

void StarPointerLayout::exeRelease() {
    if (isTouch()) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOnReaction));
        return;
    }

    if (MR::isFirstStep(this)) {
        if (mPadChannel == WPAD_CHAN1 && isAppear()) {
            MR::startSystemSE("SE_SY_DPD_B_RELEASE");
        }

        if (mPointerKind == StarPointerKind_StarPointer) {
            if (mPadChannel == WPAD_CHAN1) {
                startAnimStarPointer("Release");
            } else {
                changeToStarPointer();
            }
        } else if (mPointerKind == StarPointerKind_HandPointer || mPointerKind == StarPointerKind_2 ||
                   mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor) {
            startAnimHandPaa();
        } else if (mPointerKind == StarPointerKind_FingerPointer) {
            startAnimHandFinger();
        } else if (mPointerKind == StarPointerKind_5) {
            startAnimStarPointer("Wait");
        } else if (mPointerKind == StarPointerKind_YoshiPointer) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 0.0f, 1);
            mBlur->show();
        } else if (mPointerKind == StarPointerKind_YoshiPointerNoBlur) {
            startAnimYoshiPointer("Wait", 0);
            startAnimYoshiPointer("Transparency", 1);
            MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 1.0f, 1);
            mBlur->hideAll();
        }

        if (mPointerKind == StarPointerKind_StarPointerNozzle) {
            MR::showPane(this, "PicNozzle");
            MR::stopPaneAnim(this, "PicNozzle", 0);
            startAnimStarPointer("Wait");
        } else if (mPadChannel == WPAD_CHAN0) {
            MR::hidePane(this, "PicNozzle");
        }
    }

    if (MR::testCorePadButtonA(mPadChannel)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip));
        return;
    }

    if (MR::isGreaterStep(this, ::hReleaseTime)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
    }
}

void StarPointerLayout::exeOnReaction() {
    if (MR::isFirstStep(this)) {
        if (mPadChannel == WPAD_CHAN0 && mPointerKind == StarPointerKind_StarPointer) {
            startAnimHandPaa();
        } else {
            if (mPointerKind == StarPointerKind_StarPointer) {
                startAnimStarPointer("Touch");
            } else if (mPointerKind == StarPointerKind_HandPointer) {
                startAnimHandPaa();
            } else if (mPointerKind == StarPointerKind_2 || mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor) {
                startAnimHandFinger();
            } else if (mPointerKind == StarPointerKind_FingerPointer) {
                startAnimHandFinger();
            } else if (mPointerKind == StarPointerKind_5) {
                startAnimStarPointer("Touch");
            } else if (mPointerKind == StarPointerKind_YoshiPointer) {
                startAnimYoshiPointer("Touch", 0);
                startAnimYoshiPointer("Transparency", 1);
                MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 0.0f, 1);
                mBlur->show();
            } else if (mPointerKind == StarPointerKind_YoshiPointerNoBlur) {
                startAnimYoshiPointer("Touch", 0);
                startAnimYoshiPointer("Transparency", 1);
                MR::setPaneAnimFrameAndStop(this, "YoshiPointer", 1.0f, 1);
                mBlur->hideAll();
            }
        }

        if (mPointerKind == StarPointerKind_StarPointerNozzle) {
            MR::showPane(this, "PicNozzle");
            MR::stopPaneAnim(this, "PicNozzle", 0);
            startAnimStarPointer("Wait");
        } else if (mPadChannel == WPAD_CHAN0) {
            MR::hidePane(this, "PicNozzle");
        }
    }

    if (MR::testCorePadButtonA(mPadChannel)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip));
        return;
    }

    if (!isTouch()) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
    }
}

void StarPointerLayout::updateTouch() {
    mIsNewTouch = false;

    mSingleTouch = mStartSingleTouch;
    mStartSingleTouch = false;

    if (mStartTouch) {
        if (mTouchTimer == 0 || mNewTouchedID != mTouchedID) {
            mIsNewTouch = true;
            mTouchedID = mNewTouchedID;
        }
        mTouchTimer = ::sTouchCount;
        mStartTouch = false;
        return;
    }

    if (mTouchTimer > 0) {
        mTouchTimer--;
    }
}

bool StarPointerLayout::isTouch() {
    return mTouchTimer > 0;
}

void StarPointerLayout::exeOutScreen() {
    if (MR::isFirstStep(this)) {
        hideBlur();
        if (mPadChannel == WPAD_CHAN0) {
            MR::hidePane(this, "PicNozzle");
        }
    }

    if (MR::isStarPointerInScreen(mPadChannel)) {
        if (MR::testCorePadButtonA(mPadChannel)) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip));
        } else {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
        }
        return;
    }

    if (MR::isGreaterStep(this, 0)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide));
    }
}

void StarPointerLayout::hideAll() {
    hideBlur();
    if (mPadChannel == WPAD_CHAN0) {
        MR::hidePane(this, "PicNozzle");
        MR::hidePane(this, "HandArrow");
        updatePlayerPane();
    }
    MR::hidePaneRecursive(this, "HandPointer");
    MR::hidePaneRecursive(this, "StarPointer");
}

void StarPointerLayout::exeHide() {
    if (MR::isFirstStep(this)) {
        if (mPadChannel == WPAD_CHAN0) {
            MR::hidePane(this, "PicNozzle");
        }

        if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately))) {
            MR::hidePaneRecursive(this, "HandPointer");
            MR::hidePaneRecursive(this, "StarPointer");
            updatePlayerPane();
        } else if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvNotConnected)) || isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate)) ||
                   isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidateWait))) {
            MR::hideLayout(this);
            MR::stopAnimFrame(this);
            MR::offCalcAnim(this);
        } else {
            switch (mPointerKind) {
            case StarPointerKind_StarPointer:
            case StarPointerKind_5:
            case StarPointerKind_StarPointerNozzle:
                startAnimStarPointer("End");
                MR::setAnimRate(this, hEndAnimRate, 0);
                break;
            case StarPointerKind_YoshiPointer:
                startAnimYoshiPointer("End", 0);
                MR::setAnimRate(this, hEndAnimRate, 0);
                MR::stopPaneAnim(this, "YoshiPointer", 1);
                break;
            case StarPointerKind_YoshiPointerNoBlur:
                startAnimYoshiPointer("End", 0);
                MR::setAnimRate(this, hEndAnimRate, 0);
                MR::stopPaneAnim(this, "YoshiPointer", 1);
                break;
            case StarPointerKind_HandPointer:
            case StarPointerKind_2:
            case StarPointerKind_HandPointerReactionWithCrossCursor:
            case StarPointerKind_FingerPointer:
                MR::hidePane(this, "HandPointer");
                break;
            }
        }

        hideBlur();
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide)) || isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately))) {
        if (MR::isStarPointerInScreen(mPadChannel)) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
        }
    }
}

void StarPointerLayout::endHide() {
    tearDownHide();
}

void StarPointerLayout::tearDownHide() {
    if (mPointerKind == StarPointerKind_StarPointer || mPointerKind == StarPointerKind_5 || mPointerKind == StarPointerKind_StarPointerNozzle) {
        MR::setAnimRate(this, 1.0f, 0);
    } else if (mPointerKind == StarPointerKind_HandPointer || mPointerKind == StarPointerKind_2 || mPointerKind == StarPointerKind_FingerPointer) {
        MR::showPane(this, "HandPointer");
    }
    MR::showLayout(this);
    MR::releaseAnimFrame(this);
    MR::onCalcAnim(this);
    calcAnim();
}

void StarPointerLayout::calcAnim() {
    LayoutActor::calcAnim();
    mNumber->calcAnim();
}

void StarPointerLayout::draw() const {
    if (!isAppear()) {
        mBlur->hideAll();
        return;
    }

    if (mPadChannel == WPAD_CHAN0 && MR::isStarPointer1PInvalid2PValidMode()) {
        mBlur->hideAll();
        return;
    }

    drawDecoration();
    LayoutActor::draw();
    mNumber->draw();
}

void StarPointerLayout::drawDecoration() const {
    if (mPointerKind == StarPointerKind_HandPointer || mPointerKind == StarPointerKind_2 || mPointerKind == StarPointerKind_FingerPointer || mPointerKind == StarPointerKind_YoshiPointerNoBlur ||
        (mPointerKind == StarPointerKind_5 && MR::testCorePadButtonA(mPadChannel))) {
        mBlur->hideAll();
        return;
    }

    MR::loadProjectionMtxFor2D();
    mBlur->draw();
}

void StarPointerLayout::control() {
    mAppearTime++;
    mNumber->movement();
    MR::convertPaneLocalPosToScreenPos(&mPosition, MR::getRootPane(this), TVec2f(0.0f, 0.0f));

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvNotConnected))) {
        if (MR::isConnectedWPad(mPadChannel)) {
            hideAll();
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately));
        }
        return;
    }

    if (!MR::isConnectedWPad(mPadChannel)) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvNotConnected));
        return;
    }

    if (mPadChannel == WPAD_CHAN0) {
        if (mPointerKind == StarPointerKind_HandPointerReactionWithCrossCursor &&
            (static_cast<u32>(getAnimType() - AnimType_HandGuu) <= 1) && !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide)) &&
            !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately)) && !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOutScreen))) {
            MR::showPane(this, "HandArrow");
        } else {
            MR::hidePane(this, "HandArrow");
        }
    }

    updateDecoration();
    rotateTest();
    updateTouch();

    mShootDisabled = mStartDisableShoot;
    mStartDisableShoot = false;

    if (!isAppear()) {
        if (!isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate))) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate));
        }
        setPosition();
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate)) || isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidateWait))) {
        if (MR::isStarPointerInScreen(mPadChannel)) {
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
        } else {
            hideAll();
            setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately));
        }
        setPosition();
        return;
    }

    if (!MR::isStarPointerInScreen(mPadChannel)) {
        StarPointerInput* controller = StarPointerFunction::getStarPointerDirector()->getStarPointerController(mPadChannel);
        if (!isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOutScreen)) && !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide)) &&
            !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately))) {
            if (controller->isOutScreenLong()) {
                if (mPointerKind == StarPointerKind_StarPointer) {
                    if (MR::isHiddenPane(this, "StarPointer")) {
                        startAnimHandGuu();
                    } else {
                        startAnimStarPointer("Blink");
                    }
                } else if (mPointerKind == StarPointerKind_HandPointer) {
                    startAnimHandPaa();
                }
                setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOutScreen));

                MR::tryRumblePadWeak(this, mPadChannel);
            }
            
            return;
        }
        return;
    }

    if (mPadChannel == WPAD_CHAN1) {
        MR::startAnim(this, "Transparency", 1);

        if (MR::isStarPointer2PTransparencyMode() && !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide))) {
            mBlur->setPortAndColor(mPadChannel, &::sColorB[mPadChannel], &::sColorB[mPadChannel]);
            MR::setAnimFrameAndStop(this, 1.0f, 1);
        } else {
            mBlur->setPortAndColor(mPadChannel, &::sColorA[mPadChannel], &::sColorB[mPadChannel]);
            MR::setAnimFrameAndStop(this, 0.0f, 1);
        }
    }

    setPosition();
}

void StarPointerLayout::setPosition() {
    StarPointerInput* controller = StarPointerFunction::getStarPointerDirector()->getStarPointerController(mPadChannel);
    TVec2f pos(controller->getScreenPosition());
    setTrans(pos);
    mNumber->setTrans(pos);
}

void StarPointerLayout::rotateTest() {
    TVec2f horiz;
    MR::getWPad(mPadChannel)->mPointer->getHorizonVec(&horiz);
    f32 angle = MR::normalizeAbs(MR::toDegree(MR::atan2(horiz.y, horiz.x)), ::sDegreeMin, 180.0f);

    if (mPointerKind == StarPointerKind_StarPointer) {
        angle *= ::sRotateMult;
        TVec3f rotate;
        MR::copyPaneRotate(&rotate, this, "StarPointer");
        MR::setPaneRotate(this, rotate.x, rotate.y, -angle, "StarPointer");
    } else if (mPointerKind == StarPointerKind_StarPointerNozzle && mPadChannel == WPAD_CHAN0) {
        TVec2f targetScreenPos;
        MR::calcScreenPosition(&targetScreenPos, StarPointerFunction::getStarPointerDirector()->mNozzleAimPos);
        TVec2f pointerPos = MR::getStarPointerScreenPositionOrEdge(mPadChannel);
        calcPoseNozzle(targetScreenPos, pointerPos);
    }
}

void StarPointerLayout::calcPoseNozzle(const TVec2f& rTargetPos, const TVec2f& rPointerPos) {
    TVec2f diff = rPointerPos;
    diff.sub(rTargetPos);

    if (MR::isScreen16Per9()) {
        // stripped
    }

    if (!MR::isNearZero(diff)) {
        f32 dist;
        MR::separateScalarAndDirection(&dist, &diff, diff);

        f32 angle = MR::toDegree(MR::atan2(diff.x, diff.y));
        TVec3f rotate;
        MR::copyPaneRotate(&rotate, this, "PicNozzle");
        MR::setPaneRotate(this, rotate.x, rotate.y, angle, "PicNozzle");
    }
}

void StarPointerLayout::changeLayout(StarPointerKind pointerKind) {
    if (pointerKind == mPointerKind) {
        return;
    }

    switch (pointerKind) {
    case StarPointerKind_StarPointer:
        mAnimType = AnimType_NULL;
        mRadius = ::sNormalRadius;
        break;
    case StarPointerKind_HandPointer:
        mRadius = ::sHandRadius;
        break;
    case StarPointerKind_2:
        mRadius = ::sHandRadius;
        break;
    case StarPointerKind_HandPointerReactionWithCrossCursor:
        mRadius = ::sHandRadius;
        break;
    case StarPointerKind_FingerPointer:
        mRadius = ::sHandRadius;
        break;
    case StarPointerKind_5:
        mRadius = ::sHandRadius;
        break;
    case StarPointerKind_StarPointerNozzle:
        mRadius = ::sNormalRadius;
        break;
    case StarPointerKind_YoshiPointerNoBlur:
        mBlur->hideAll();
    case StarPointerKind_YoshiPointer:
        mRadius = sNormalRadius;
        break;
    }

    mPointerKind = pointerKind;

    if (!isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide)) && !isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately))) {
        calcAnim();
    }

    resetSameNerve();
}

bool StarPointerLayout::isChanceToRumble() const {
    if (mIsNewTouch) {
        return true;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOnReaction)) && MR::isStep(this, 1)) {
        return true;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip)) && MR::isStep(this, 1)) {
        return true;
    }

    return false;
}

void StarPointerLayout::hideBlur() {
    mBlur->hideAll();
}

void StarPointerLayout::resetSameNerve() {
    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvWait));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOnReaction))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOnReaction));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvGrip));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHold))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHold));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHoldTouch))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHoldTouch));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvRelease))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvRelease));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOutScreen))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOutScreen));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvNotConnected))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvNotConnected));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate));
        return;
    }

    if (isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidateWait))) {
        setNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidateWait));
        return;
    }
}

void StarPointerLayout::changeToStarPointer() {
    if (mBlur->mBlurPoints[0].mIsHidden) {
        mBlur->show();
    }

    MR::hidePaneRecursive(this, "HandPointer");
    MR::showPaneRecursive(this, "StarPointer");
    updatePlayerPane();
}

void StarPointerLayout::changeToHandPointer() {
    hideBlur();
    MR::hidePaneRecursive(this, "StarPointer");
    MR::showPaneRecursive(this, "HandPointer");
    updatePlayerPane();

    if (mPadChannel == WPAD_CHAN0) {
        MR::showPaneRecursive(this, "PicPlayer1");
        MR::hidePaneRecursive(this, "PicPlayer2");
    } else {
        MR::showPaneRecursive(this, "PicPlayer2");
        MR::hidePaneRecursive(this, "PicPlayer1");
    }
}

// NONMATCHING: data-base setup and pointer arithmetic instruction scheduling differ.
void StarPointerLayout::showYoshiPointer() {
    if (mPadChannel == WPAD_CHAN0) {
        MR::showPaneRecursive(this, "YoshiPointer");
        mBlur->setPortAndColor(mPadChannel, &sYoshiPointerColor, &sYoshiPointerColor);
    }
}

void StarPointerLayout::startAnimYoshiPointer(const char* animation, u32 index) {
    MR::hidePaneRecursive(this, "HandPointer");
    MR::hidePaneRecursive(this, "StarPointer");
    showYoshiPointer();
    MR::startPaneAnim(this, "YoshiPointer", animation, index);
    mAnimType = AnimType_YoshiPointer;
}

void StarPointerLayout::updatePlayerPane() {
    if (mPadChannel == WPAD_CHAN0) {
        MR::hidePaneRecursive(this, "YoshiPointer");
        mBlur->setPortAndColor(mPadChannel, &sColorA[mPadChannel], &sColorB[mPadChannel]);
    }
}

void StarPointerLayout::startAnimStarPointer(const char* pAnimName) {
    changeToStarPointer();
    MR::startPaneAnim(this, "StarPointer", pAnimName, 0);
    mAnimType = AnimType_StarPointer;
}

void StarPointerLayout::startAnimHandPaa() {
    changeToHandPointer();
    MR::startPaneAnim(this, "HandPointer", "HandType", 0);
    MR::setPaneAnimFrameAndStop(this, "HandPointer", 2.0f, 0);
    mAnimType = AnimType_HandPaa;
}

void StarPointerLayout::startAnimHandGuu() {
    changeToHandPointer();
    MR::startPaneAnim(this, "HandPointer", "HandType", 0);
    MR::setPaneAnimFrameAndStop(this, "HandPointer", 1.0f, 0);
    mAnimType = AnimType_HandGuu;
}

void StarPointerLayout::startAnimHandFinger() {
    changeToHandPointer();
    MR::startPaneAnim(this, "HandPointer", "HandType", 0);
    MR::setPaneAnimFrameAndStop(this, "HandPointer", 0.0f, 0);
    mAnimType = AnimType_Finger;
}

void StarPointerLayout::updateDecoration() {
    if (mPointerKind == StarPointerKind_HandPointer || mPointerKind == StarPointerKind_2) {
        return;
    }

    mBlur->movement();
}

StarPointerNumber::StarPointerNumber(StarPointerLayout* pParent, s32 padChannel, const char* pName)
    : LayoutActor(pName, true), mParent(pParent), mPadChannel(padChannel), mAnimType(AnimType_NULL), mIsAppear(false), mAppearTime(0) {
    initLayoutManager("DPDPointerNumber", 1);
    startAnimPosition(0);
}

void StarPointerNumber::control() {
    if (MR::isConnectedWPad(WPAD_CHAN0) && MR::isConnectedWPad(WPAD_CHAN1)) {
        mIsAppear = true;
        mAppearTime++;
    } else {
        mIsAppear = false;
        mAppearTime = 0;
    }

    if (mAppearTime == ::hHideRumbleTime) {
        MR::tryRumblePadWeak(this, mPadChannel);
    }

    if (mAppearTime == 40) {
        MR::startCSSound("CS_CONT_CONNECT", "SE_SY_CS_CONT_CONNECT", mPadChannel, 0, 1.0f);
    }

    if (!mIsAppear || isOffState()) {
        startAnimPosition(0);
        mAnimType = AnimType_NULL;
        return;
    }

    switch (mParent->mAnimType) {
    case AnimType_Finger:
        startAnimPosition(2);
        break;
    case AnimType_HandGuu:
        startAnimPosition(3);
        break;
    case AnimType_HandPaa:
        startAnimPosition(3);
        break;
    case AnimType_StarPointer:
        startAnimPosition(1);
        break;
    case AnimType_YoshiPointer:
        startAnimPosition(1);
        break;
    }

    mAnimType = mParent->mAnimType;
}

bool StarPointerNumber::isOffState() const {
    return mParent->isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvOutScreen)) ||
           mParent->isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvNotConnected)) ||
           mParent->isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHide)) ||
           mParent->isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvHideImmediately)) ||
           mParent->isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidate)) ||
           mParent->isNerve(GET_NERVE(StarPointerLayout, HostTypeNrvInvalidateWait));
}

void StarPointerNumber::startAnimPosition(u32 animType) {
    MR::startAnimAndSetFrameAndStop(this, "Position", animType + (mPadChannel == WPAD_CHAN0 ? 0 : 10), 0);
}
