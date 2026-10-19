#include "Game/Player/MarioFpView.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/YoshiTongueAction.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static bool sIsDisabled;
};  // namespace

bool Mario::isDisableStayHere() const {
    if (((getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        return true;
    }

    if (!((getMovementStates().mLowWord >> 30) & 1U)) {
        return true;
    }

    if (_3CE < 5) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 21) & 1U)) {
        return true;
    }

    if (isDamaging()) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 4) & 1U)) {
        return true;
    }

    if (MR::getKarikariClingNum() != 0) {
        return true;
    }

    if (((getPlayer()->getMovementStates().mHighWord >> 28) & 1U)) {
        return true;
    }

    if (mWalkSpeed > 0.01f) {
        return true;
    }

    if (getStickP() != 0.0f) {
        return true;
    }

    if (((mFlags14.mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return true;
    }

    if (_960 == 27) {
        return true;
    }

    if (_960 == 28) {
        return true;
    }

    if (((getDrawStates().mWord >> 20) & 1U)) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 7) & 1U)) {
        return true;
    }

    if (!MR::isNearZero(_928)) {
        return true;
    }

    if (!MR::isNearZero(_350)) {
        return true;
    }

    if (!MR::isNearZero(_35C)) {
        return true;
    }

    if (!MR::isNearZero(_184)) {
        return true;
    }

    if (mActor->mBeeWallWalk != 0) {
        return true;
    }

    if (((getMovementStates().mHighWord >> 8) & 1U)) {
        return true;
    }

    if (((getMovementStates().mHighWord >> 5) & 1U)) {
        return true;
    }

    if (((mFlags14.mLowWord >> 10) & 1U)) {
        return true;
    }

    if (mSinkTimer) {
        return true;
    }

    if (mActor->_EA4) {
        return true;
    }

    return mActor->_3C0;
}

bool Mario::isDisableFpViewMode() const {
    if (_97C) {
        return true;
    }

    return isDisableStayHere();
}

MarioFpView::MarioFpView(MarioActor* pActor) : MarioState(pActor, MarioStatus_FpView), mDelay(), mIsActive(), mBlurOffset(), mFrame() {
}

bool MarioFpView::start() {
    MR::startSubjectiveCamera(-1);
    changeAnimation("見る");

    ::sIsDisabled = true;

    mDelay = 0;
    mIsActive = true;
    mBlurOffset = 30.0f;
    mFrame = 0;
    if (isPlayerModeCloud()) stopEffect("雲マリオ頭煙");
    if (isPlayerModeYoshi()) mActor->fn_803B9810()->fn_8041FF80();

    return true;
}

bool MarioFpView::update() {
    if (::sIsDisabled) {
        ::sIsDisabled = false;
        return true;
    }

    if (mFrame == 20 && mDelay == 0) fn_803EE8F0("ヨッシー主観カメラ開始");
    if (mFrame == 25 && mDelay == 0 && isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040CC20();
    mBlurOffset -= 0.5f;
    ++mFrame;
    if (mBlurOffset < 0.0f) {
        mBlurOffset = 0.0f;
    }

    if (getPlayer()->isDisableStayHere()) {
        return false;
    }

    if (MR::testFpViewOutTrigger() && mDelay == 0) {
        MR::startSystemSE("SE_SY_CAMERA_RESET");

        MR::endSubjectiveCamera(15);

        mDelay = 20;
        mIsActive = false;
        fn_803EE8F0("ヨッシー主観カメラ終了");
        if (isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040CC00();
    }

    if (mDelay != 0 && --mDelay == 0) {
        return false;
    }

    return true;
}

bool MarioFpView::close() {
    if (mIsActive) {
        MR::endSubjectiveCamera(15);
    }

    stopAnimation("見る");

    mActor->showBeeFur();
    if (isPlayerModeCloud()) playEffect("雲マリオ頭煙");
    if (isPlayerModeYoshi()) {
        if (isAnimationRun("ヨッシー主観カメラ開始")) fn_803EE8F0("ヨッシー主観カメラ終了");
        mActor->fn_803B97F0()->fn_8040CC00();
        mActor->fn_803B9810()->fn_80420000();
    }

    return true;
}

void MarioFpView::forceClose() {
    MR::endSubjectiveCamera(1);

    mIsActive = false;
}

void Mario::tryFpViewMode() {
    if (isStatusActive(MarioStatus_FpView)) {
        return;
    }

    stopWalk();

    changeStatus(mFpView);

    MR::startSystemSE("SE_SY_CAMERA_FP_VIEW");
}

f32 MarioFpView::getBlurOffset() const {
    return mBlurOffset;
}
