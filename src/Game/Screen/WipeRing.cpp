#include "Game/Screen/WipeRing.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>


namespace {
    static const s32 sAnimFrame = 60;
    static const char* sOutAnimName = "Out";
    static const char* sInAnimName = "In";
    static const char* sColorAnimName = "Color";
};  // namespace

namespace NrvWipeRing {
    WipeRingNrvClose WipeRingNrvClose::sInstance;
    WipeRingNrvWipeIn WipeRingNrvWipeIn::sInstance;
    WipeRingNrvOpen WipeRingNrvOpen::sInstance;
    WipeRingNrvWipeOut WipeRingNrvWipeOut::sInstance;
}

WipeRing::WipeRing(bool isCenterOnPlayer, const char* pName)
    : WipeLayoutBase(pName), mCenterPos(0.0f, 0.0f), mAnimFrame(::sAnimFrame), mWipeOutFrame(), mWipeInFrame(), mIsCenterOnPlayer(isCenterOnPlayer), mIsCenterOnPlayerOnce(false) {
}

void WipeRing::init(const JMapInfoIter& rIter) {
    initLayoutManager("WipeRing", 2);
    MR::createAndAddPaneCtrl(this, "Ring", 1);
    MR::setFollowPos(&mCenterPos, this, "Ring");
    initNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvOpen));
    kill();
}

void WipeRing::exeWipeIn() {
    updatePlayerPos();

    if (MR::isFirstStep(this)) {
        f32 radiusRate = calcRadius() / calcMaxRadius();

        mWipeInFrame = radiusRate * MR::getAnimCtrl(this, 0)->getEnd();
        MR::getAnimCtrl(this, 0)->setRate(0.0f);
    }

    const f32 angle = (1.0f + static_cast< f32 >(getNerveStep()) / mAnimFrame) * JGeometry::TUtil< f32 >::PI();
    f32 f = MR::sin(0.5f * angle);

    MR::getAnimCtrl(this, 0)->setFrame(mWipeInFrame * (1.0f - f));

    if (MR::isStep(this, mAnimFrame)) {
        setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvOpen));
    }
}

void WipeRing::exeWipeOut() {
    updatePlayerPos();

    if (MR::isFirstStep(this)) {
        f32 invRadiusRate = 1.0f - calcRadius() / calcMaxRadius();

        mWipeOutFrame = invRadiusRate * MR::getAnimCtrl(this, 0)->getEnd();
        MR::getAnimCtrl(this, 0)->setRate(0.0f);
    }

    const f32 angle = (static_cast< f32 >(getNerveStep()) / mAnimFrame) * JGeometry::TUtil< f32 >::PI();
    f32 f = MR::sin(0.5f * angle);

    MR::getAnimCtrl(this, 0)->setFrame(mWipeOutFrame * (1.0f - f) + f * MR::getAnimCtrl(this, 0)->getEnd());

    if (MR::isStep(this, mAnimFrame)) {
        setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvClose));
    }
}

void WipeRing::wipe(s32 frame) {
    if (frame < 1) {
        mAnimFrame = 90;
    } else {
        mAnimFrame = frame;
    }

    if (isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvClose))) {
        setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeIn));
        startAnim(::sInAnimName);
        setColor();
    } else if (isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeIn))) {
        setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeOut));
        startAnim(::sOutAnimName);
        setColor();
    } else if (isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvOpen))) {
        appearWithColor();

        setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeOut));
        startAnim(::sOutAnimName);
        setColor();
    } else if (isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeOut))) {
        setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeIn));
        startAnim(::sInAnimName);
        setColor();
    }
    mIsCenterOnPlayerOnce = false;
}

void WipeRing::forceClose() {
    appearWithColor();

    MR::startAnim(this, ::sInAnimName, 0);
    MR::setAnimFrameAndStop(this, 0.0f, 0);
    setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvClose));
}

void WipeRing::forceOpen() {
    kill();
    setNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvOpen));
}

bool WipeRing::isOpen() const {
    return isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvOpen));
}

bool WipeRing::isClose() const {
    return isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvClose));
}

bool WipeRing::isWipeIn() const {
    return isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeIn));
}

bool WipeRing::isWipeOut() const {
    return isNerve(GET_NERVE_DIRECT(NrvWipeRing, WipeRingNrvWipeOut));
}

void WipeRing::setCenterPos(const TVec3f& rPos) {
    if (MR::calcScreenPosition(&mCenterPos, rPos)) {
        return;
    }

    mCenterPos.set(MR::getScreenWidth() * 0.5f, MR::getScreenHeight() * 0.5f);
}

void WipeRing::startAnim(const char* pAnimName) {
    MR::startAnim(this, pAnimName, 0);
    MR::getAnimCtrl(this, 0)->setRate(static_cast< f32 >(MR::getAnimCtrl(this, 0)->getEnd()) / mAnimFrame);
}

bool WipeRing::getMarioCenterPos(TVec3f* pCenterPos) {
    if (MR::isExistMario()) {
        pCenterPos->set(static_cast< const Vec& >(*MR::getPlayerCenterPos()));

        return true;
    }

    pCenterPos->zero();
    return false;
}

f32 WipeRing::calcRadius() const {
    f32 radius = mCenterPos.length();

    TVec2f screenHeightPos = TVec2f(0.0f, static_cast< f32 >(MR::getScreenHeight()));
    f32 screenHeightDistance = mCenterPos.distance(screenHeightPos);

    if (radius < screenHeightDistance) {
        radius = screenHeightDistance;
    }

    TVec2f screenWidthPos = TVec2f(static_cast< f32 >(MR::getScreenWidth()), 0.0f);
    f32 screenWidthDistance = mCenterPos.distance(screenWidthPos);

    if (radius < screenWidthDistance) {
        radius = screenWidthDistance;
    }

    TVec2f screenMaxPos = TVec2f(static_cast< f32 >(MR::getScreenWidth()), static_cast< f32 >(MR::getScreenHeight()));
    f32 screenMaxDistance = mCenterPos.distance(screenMaxPos);

    if (radius < screenMaxDistance) {
        radius = screenMaxDistance;
    }

    return radius;
}

f32 WipeRing::calcMaxRadius() const {
    return MR::fastSqrtf(659344 + MR::getScreenHeight() * MR::getScreenHeight());
}

void WipeRing::updatePlayerPos() {
    if (!mIsCenterOnPlayer && !mIsCenterOnPlayerOnce) {
        return;
    }

    if (!MR::isExistSceneObj(SceneObj_CameraContext)) {
        return;
    }

    TVec3f marioCenterPos;

    if (!getMarioCenterPos(&marioCenterPos) && !MR::isFirstStep(this)) {
        return;
    }

    if (MR::calcScreenPosition(&mCenterPos, marioCenterPos)) {
        return;
    }

    mCenterPos.set(MR::getScreenWidth() * 0.5f, MR::getScreenHeight() * 0.5f);
}

void WipeRing::appearWithColor() {
    if (MR::isDead(this)) {
        appear();
        setColor();
    }
    mIsCenterOnPlayerOnce = false;
}

void WipeRing::setColor() {
    MR::startAnim(this, ::sColorAnimName, 1);
    bool green = false;
    if (MR::isStageMarioFaceShip()) {
        if (GameSequenceFunction::hasStageResultSequence()) {
            s32 starId = GameSequenceFunction::getClearedPowerStarId();
            if (MR::isPowerStarGreen(GameSequenceFunction::getClearedStageName(), starId)) {
                green = true;
            }
        }
    } else {
        if (MR::isPowerStarGreenInCurrentStage()) {
            green = true;
        }
    }
    if (green) {
        MR::setAnimFrameAndStop(this, 1.0f, 1);
    } else {
        MR::setAnimFrameAndStop(this, 0.0f, 1);
    }
}
