#include "Game/Demo/StarReturnDemoStarter.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/ReturnDemoRailMove.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Screen/StageResultInformer.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const s32 cDemoAppearEffectStep = 30;
    const s32 cDemoAppearWaitFrame = 45;
    const s32 cFadeOutFrame = 60;
    const s32 cOverlayFrame = 3;
    // const s32 cFlyAwayFrame = _;
    const s32 cCapHideStartStep = 21;
    const s32 cCapHideEndStep = 221;
    const s32 cFallStartFrameGrandStar = 360;
    const char* const cDemoMovePartName = "移動";
    const char* const cDemoWaitPartName = "ウェイト";
};  // namespace

namespace {
};  // namespace

namespace NrvStarReturnDemoStarter {
    NEW_NERVE(StarReturnDemoStarterNrvAppearWait, StarReturnDemoStarter, AppearWait);
    NEW_NERVE(StarReturnDemoStarterNrvMove, StarReturnDemoStarter, Move);
    NEW_NERVE(StarReturnDemoStarterNrvFlyWaitPowerStar, StarReturnDemoStarter, FlyWaitPowerStar);
    NEW_NERVE(StarReturnDemoStarterNrvFlyWaitGrandStar, StarReturnDemoStarter, FlyWaitGrandStar);
    NEW_NERVE(StarReturnDemoStarterNrvFall, StarReturnDemoStarter, Fall);
    NEW_NERVE(StarReturnDemoStarterNrvLand, StarReturnDemoStarter, Land);
    NEW_NERVE(StarReturnDemoStarterNrvWait, StarReturnDemoStarter, Wait);
    NEW_NERVE(StarReturnDemoStarterNrvStageResult, StarReturnDemoStarter, StageResult);
    NEW_NERVE(StarReturnDemoStarterNrvFadeOut, StarReturnDemoStarter, FadeOut);
    NEW_NERVE(StarReturnDemoStarterNrvStageResultAfter, StarReturnDemoStarter, StageResultAfter);
};  // namespace NrvStarReturnDemoStarter

StarReturnDemoStarter::StarReturnDemoStarter(const char* pName)
    : LiveActor(pName), mReturnDemoRailMove(), mStageResultInformer(), mPosition(gZeroVec), mPowerStar(), mLuma(), mHair(), mFrame() {
    mTransform.identity();
}



void StarReturnDemoStarter::kill() {
    LiveActor::kill();
    mLuma->kill();

    if (mHair != nullptr) {
        mHair->kill();
    }

    if (MR::isDrawSpinDriverPathAtOpa()) {
        MR::offDrawSpinDriverPathAtOpa();
    }
}


void StarReturnDemoStarter::control() {
    if (!isNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvStageResultAfter))) {
        MR::setPlayerBaseMtx(mTransform);
    }
}

LiveActor* StarReturnDemoStarter::createSubModel(const char* pName, const char* pModelName, bool isPlayerDecoration) {
    LiveActor* subModel =
        isPlayerDecoration ? MR::createModelObjPlayerDecoration(pName, pModelName, mTransform) : MR::createModelObjNpc(pName, pModelName, mTransform);

    if (MR::getLightNumMax(subModel) > 0) {
        MR::initLightCtrl(subModel);
    }

    MR::invalidateClipping(subModel);
    subModel->kill();

    return subModel;
}

void StarReturnDemoStarter::tryRegisterDemo(const char* pDemoName, const JMapInfoIter& rIter) {
    if (MR::isDemoExist(pDemoName) && MR::tryRegisterDemoCast(this, pDemoName, rIter)) {
        MR::tryRegisterDemoCast(mPowerStar, pDemoName, rIter);
        MR::tryRegisterDemoCast(mLuma, pDemoName, rIter);

        if (mHair != nullptr) {
            MR::tryRegisterDemoCast(mHair, pDemoName, rIter);
        }
    }
}



const char* playerAnimName() {
    return MR::isPlayerLuigi() ? "LuigiResultWaitStart" : "ResultWaitStart";
}

void StarReturnDemoStarter::exeAppearWait() {
    if (MR::isFirstStep(this)) {
        MR::hidePlayer();
        mPowerStar->appear();
        MR::hideModelAndOnCalcAnim(mPowerStar);

        if (MR::isEqualStageName("AstroGalaxy")) {
            MR::startStageBGM("MBGM_GALAXY_24", false);
        }
    }

    if (MR::isStep(this, ::cDemoAppearEffectStep)) {
        MR::emitEffect(mPowerStar, "DemoFlyLightDemo");
        MR::startActionSound(mPowerStar, "PmDemoReturnLight");
    }

    MR::setNerveAtStep(this, GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvMove), ::cDemoAppearWaitFrame);
}


void StarReturnDemoStarter::exeFlyWaitPowerStar() {
    if (MR::isFirstStep(this)) {
        mReturnDemoRailMove->posToEnd();
    }

    PowerStar::requestPointLightAtResultSequence(mPowerStar);

    int brakeFrame = mReturnDemoRailMove->getDemoFlyBrakeFrame();

    if (MR::isLessStep(this, 115 - brakeFrame)) {
        MR::startActionSound(mPowerStar, "OjLvRetPowStarFly");
    }

    if (MR::isStep(this, 115 - brakeFrame)) {
        MR::startActionSound(mPowerStar, "OjRetPowStarLeave");
    }

    if (MR::isBckStoppedPlayer()) {
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvFall));
    }
}

void StarReturnDemoStarter::exeFlyWaitGrandStar() {
    if (MR::isFirstStep(this)) {
        mReturnDemoRailMove->posToEnd();
    }

    PowerStar::requestPointLightAtResultSequence(mPowerStar);

    if (MR::isLessStep(this, MR::getBckFrameMaxPlayer("ResultFlyGrandStarEnd") - mReturnDemoRailMove->getDemoFlyBrakeFrame())) {
        MR::startActionSound(mPowerStar, "OjLvRetPowStarFly");
    }

    if (MR::isBckOneTimeAndStoppedPlayer()) {
        MR::startBckPlayer("ResultFlyGrandStarFlyAway");
        MR::startAction(mPowerStar, "ResultFlyGrandStarFlyAway");
        MR::shakeCameraWeak();
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
    }

    if (MR::isStep(this, ::cFallStartFrameGrandStar)) {
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvFall));
    }
}


void StarReturnDemoStarter::exeLand() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("Land");
    }

    if (MR::isBckStoppedPlayer()) {
        LiveActor* lumaModel = mLuma;
        lumaModel->appear();
        MR::startAllAnim(lumaModel, playerAnimName());

        LiveActor* hairModel = mHair;
        if (hairModel != nullptr) {
            hairModel->appear();
            MR::startAllAnim(hairModel, playerAnimName());
        }

        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvWait));
    }
}

void StarReturnDemoStarter::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer(playerAnimName());

        mFrame = 0;

        if (MR::isEqualStageName("AstroGalaxy")) {
            MR::stopStageBGM(90);
        }
    }

    tryStartResultWait();
    tryStartStageResult();
}


void StarReturnDemoStarter::exeFadeOut() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(::cFadeOutFrame);
    }

    if (!MR::isWipeActive()) {
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvStageResultAfter));
    }
}

void StarReturnDemoStarter::exeStageResultAfter() {
    if (MR::isFirstStep(this)) {
        mReturnDemoRailMove->offPathDraw();
        MR::resumeTimeKeepDemo(this);

        if (MR::isEqualStageName("AstroGalaxy")) {
            MR::startCurrentStageBGM();
            MR::overlayWithPreviousScreen(::cOverlayFrame);
            MR::endStartPosCamera();
        }
    }

    if (!MR::isTimeKeepDemoActive()) {
        kill();
    }
}
