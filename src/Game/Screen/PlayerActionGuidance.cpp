#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Util.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const s32 sSuspendFrame = 60;
    static const s32 sChangeSoundFrame = 40;
};  // namespace

namespace NrvPlayerActionGuidance {
    NEW_NERVE(PlayerActionGuidanceNrvWaitFocusIn, PlayerActionGuidance, WaitFocusIn);
    NEW_NERVE(PlayerActionGuidanceNrvFadein, PlayerActionGuidance, Fadein);
    NEW_NERVE(PlayerActionGuidanceNrvDisplay, PlayerActionGuidance, Display);
    NEW_NERVE(PlayerActionGuidanceNrvSuspend, PlayerActionGuidance, Suspend);
    NEW_NERVE(PlayerActionGuidanceNrvFadeout, PlayerActionGuidance, Fadeout);
};  // namespace

namespace MR {
    PlayerActionGuidance* getPlayerActionGuidance() {
        return MR::getSceneObj< PlayerActionGuidance >(SceneObj_PlayerActionGuidance);
    }
};  // namespace MR

PlayerActionGuidance::PlayerActionGuidance()
    : LayoutActor("プレイヤーアクションガイダンス", true), mSpinLayout(), mTamakoroLayout(), mCurrentLayout(), mGuidanceState(GuidanceState_Rotation),
      mGuidancePrevState(GuidanceState_Rotation), mIsVolumeCheckDisabled(), mIsInGuidanceArea(), mForceTamakoroGuidance(), mPrevForceTamakoroGuidance() {
}

void PlayerActionGuidance::control() {
    mGuidancePrevState = mGuidanceState;

    if (mSpinLayout != nullptr && isInVolumePlayer("SpinGuidanceCube", false)) {
        mCurrentLayout = mSpinLayout;
        mIsInGuidanceArea = true;
    } else if (mTamakoroLayout != nullptr && (mForceTamakoroGuidance || isInVolumePlayer("TamakoroMoveGuidanceArea", true))) {
        mCurrentLayout = mTamakoroLayout;
        mGuidanceState = GuidanceState_Rotation;
        mIsInGuidanceArea = true;
    } else if (mTamakoroLayout != nullptr && (mForceTamakoroGuidance || isInVolumePlayer("TamakoroJumpGuidanceArea", true))) {
        mCurrentLayout = mTamakoroLayout;
        mGuidanceState = GuidanceState_AButton;
        mIsInGuidanceArea = true;
    } else {
        mIsInGuidanceArea = false;
    }

    mPrevForceTamakoroGuidance = mForceTamakoroGuidance;
    mForceTamakoroGuidance = false;
}

void PlayerActionGuidance::init(const JMapInfoIter& rIter) {
    MR::connectToSceneLayoutMovement(this);
    initNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvWaitFocusIn));
    startAnimAllLayout("Appear");
    setAnimFrameAndStopAllLayout(0.0f);
    hideAllLayout();

    mCurrentLayout = this;

    appear();
}

void PlayerActionGuidance::createSpinLayout() {
    if (mSpinLayout != nullptr) {
        return;
    }

    mSpinLayout = new SimpleLayout("スピンガイダンス", "SpinGuidance", 1, -1);

    if (MR::isStageStoryBook()) {
        MR::hidePane(mSpinLayout, "SpinGuidance");
    } else {
        MR::hidePane(mSpinLayout, "SpinGuidanceDemo");
    }

    mSpinLayout->appear();
}

void PlayerActionGuidance::createTamakoroLayout() {
    if (mTamakoroLayout != nullptr) {
        return;
    }

    mTamakoroLayout = new SimpleLayout("たまころガイダンス", "BallGuidance", 1, -1);
    mTamakoroLayout->appear();
}

void PlayerActionGuidance::exeWaitFocusIn() {
    if (MR::isFirstStep(this)) {
        hideAllLayout();
    }

    if (mIsInGuidanceArea) {
        setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvSuspend));
    }
}

void PlayerActionGuidance::exeFadein() {
    if (MR::isFirstStep(this)) {
        showCurrentLayout();
        startWaitAnimAllLayout();
        setAnimFrameAndStopAllLayout(0.0f);
        calcAnim();
        startAnimAllLayout("Appear");
    }

    if (MR::isAnimStopped(mCurrentLayout, 0)) {
        setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvDisplay));
    }
}

void PlayerActionGuidance::startWaitAnimTamakoro() {
    if (mTamakoroLayout == nullptr) {
        return;
    }

    if (isNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvFadein))) {
        MR::startAnim(mTamakoroLayout, "WaitRotation", 0);
    }

    switch (mGuidanceState) {
    case GuidanceState_Raise:
        MR::startAnim(mTamakoroLayout, "WaitRaise", 0);
        break;
    case GuidanceState_RaiseHold:
        MR::startAnim(mTamakoroLayout, "WaitRaiseHold", 0);
        break;
    case GuidanceState_RaiseOK:
        MR::startAnim(mTamakoroLayout, "RaiseOK", 0);
        break;
    case GuidanceState_Rotation:
        MR::startAnim(mTamakoroLayout, "WaitRotation", 0);
        break;
    case GuidanceState_AButton:
        MR::startAnim(mTamakoroLayout, "WaitAButton", 0);
        break;
    }
}

void PlayerActionGuidance::exeDisplay() {
    if (MR::isFirstStep(this)) {
        startWaitAnimAllLayout();

        if (mSpinLayout != nullptr) {
            MR::startCSSound("CS_NOTICE_USE_DPD", "SE_SY_CS_NOTICE_USE_DPD", 0, 0, 1.0f);
        }

        if (mTamakoroLayout != nullptr && mGuidanceState == GuidanceState_Raise) {
            MR::startSystemSE("SE_SY_CTRL_GUIDE_APPEAR");
        }
    }

    if (mGuidanceState == GuidanceState_Raise) {
        if (!MR::isFirstStep(this) && MR::getAnimFrame(mCurrentLayout, 0) == 0.0f) {
            MR::startSystemSE("SE_SY_CTRL_GUIDE_CHANGE", 500);
        }

        if (MR::getAnimFrame(mCurrentLayout, 0) == ::sChangeSoundFrame) {
            MR::startSystemSE("SE_SY_CTRL_GUIDE_CHANGE2", 1000);
        }
    }

    if (mGuidancePrevState != mGuidanceState) {
        startWaitAnimTamakoro();
    }

    if (MR::isAnimStopped(mCurrentLayout, 0) || !mIsInGuidanceArea) {
        setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvFadeout));
    }
}

void PlayerActionGuidance::exeSuspend() {
    if (MR::isStep(this, ::sSuspendFrame)) {
        if (!mIsInGuidanceArea) {
            hideAllLayout();
            setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvWaitFocusIn));
        } else {
            setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvFadein));
        }
    }
}

void PlayerActionGuidance::exeFadeout() {
    if (MR::isFirstStep(this)) {
        startAnimAllLayout("End");
    }

    if (MR::isAnimStopped(mCurrentLayout, 0)) {
        if (mIsInGuidanceArea) {
            setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvSuspend));
        } else {
            hideAllLayout();
            setNerve(GET_NERVE(PlayerActionGuidance, PlayerActionGuidanceNrvWaitFocusIn));
        }
    }
}

void PlayerActionGuidance::startAnimAllLayout(const char* pAnimName) {
    if (mSpinLayout != nullptr) {
        MR::startAnim(mSpinLayout, pAnimName, 0);
    }

    if (mTamakoroLayout != nullptr) {
        MR::startAnim(mTamakoroLayout, pAnimName, 0);
    }
}

void PlayerActionGuidance::startWaitAnimAllLayout() {
    if (mSpinLayout != nullptr) {
        MR::startAnim(mSpinLayout, "Wait", 0);
    }

    if (mTamakoroLayout != nullptr && mCurrentLayout == mTamakoroLayout) {
        startWaitAnimTamakoro();
    }
}

void PlayerActionGuidance::setAnimFrameAndStopAllLayout(f32 animFrame) {
    if (mSpinLayout != nullptr) {
        MR::setAnimFrameAndStop(mSpinLayout, animFrame, 0);
    }

    if (mTamakoroLayout != nullptr) {
        MR::setAnimFrameAndStop(mTamakoroLayout, animFrame, 0);
    }
}

void PlayerActionGuidance::hideAllLayout() {
    if (mSpinLayout != nullptr) {
        MR::hideLayout(mSpinLayout);
    }

    if (mTamakoroLayout != nullptr) {
        MR::hideLayout(mTamakoroLayout);
    }
}

bool PlayerActionGuidance::isInVolumePlayer(const char* pObjName, bool param2) const {
    if (mIsVolumeCheckDisabled) {
        return false;
    }

    AreaObj* pObj = MR::getAreaObj(pObjName, *MR::getPlayerPos());

    if (pObj == nullptr) {
        return false;
    }

    if (pObj->mObjArg0 == 0 && !MR::isPlayerElementModeCloud()) {
        return false;
    }

    if (pObj->mObjArg0 == 1 && !MR::isPlayerElementModeTeresa()) {
        return false;
    }

    if (pObj->mObjArg0 == 2 && !MR::isCurrentRushTamakoro()) {
        return false;
    }

    if (pObj->isValidSwitchA()) {
        if (pObj->isValidSwitchB() && pObj->isOnSwitchB()) {
            return false;
        }

        if (param2) {
            return pObj->isOnSwitchA() == true;
        } else {
            return pObj->isOnSwitchA() == false;
        }
    }

    return true;
}

void PlayerActionGuidance::showCurrentLayout() {
    hideAllLayout();
    if (mIsInGuidanceArea) MR::showLayout(mCurrentLayout);
}
namespace MR {
void hidePlayerActionGuidance() { getPlayerActionGuidance()->hideAllLayout(); }
void showPlayerActionGuidance() { getPlayerActionGuidance()->showCurrentLayout(); }
}
