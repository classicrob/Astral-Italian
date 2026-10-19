#include "Game/LiveActor/DemoObj.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
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

namespace GameSequenceFunction {
    bool hasStageResultSequence();
    bool hasNextDemoForStorySequenceEvent();
};  // namespace GameSequenceFunction

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
    : LiveActor(pName), mReturnDemoRailMove(), mStageResultInformer(), mPosition(gZeroVec), mFallGravity(0.0f, -1.0f, 0.0f), mPowerStar(), mLuma() {
    mTransform.identity();
}

// NONMATCHING: regalloc and string pool order
void StarReturnDemoStarter::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    mStageResultInformer = new StageResultInformer();
    mStageResultInformer->initWithoutIter();
    TPos3f* mtx = &mTransform;
    bool grandStar = GameDataConst::isGrandStar(GameSequenceFunction::getClearedStageName(), GameSequenceFunction::getClearedPowerStarId());
    mPowerStar = MR::createDemoObj("スターモデル", grandStar ? "GrandStar" : "PowerStar", "Demo", *mtx);
    mPowerStar->kill();
    mLuma = createSubModel("チコモデル", "SpinTico", false);
    mReturnDemoRailMove = new ReturnDemoRailMove(this, mPowerStar, rIter,
        GameDataConst::isGrandStar(GameSequenceFunction::getClearedStageName(), GameSequenceFunction::getClearedPowerStarId()), &mTransform);
    tryRegisterDemo("グランドスター帰還[２回目以降]", rIter);
    tryRegisterDemo("パワースター帰還", rIter);
    tryRegisterDemo("パワースター帰還[NPC紹介]", rIter);
    tryRegisterDemo("パワースター帰還[アイテム惑星]", rIter);
    tryRegisterDemo("パワースター帰還[初回]", rIter);
    initNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvAppearWait), 0);
    makeActorDead();
}

void StarReturnDemoStarter::appear() {
    LiveActor::appear();
    mReturnDemoRailMove->posToStart();
    PowerStar::setupColorAtResultSequence(mPowerStar,
        GameDataConst::isGrandStar(GameSequenceFunction::getClearedStageName(), GameSequenceFunction::getClearedPowerStarId()));
    if (MR::isDemoActive("天文ドームスター帰還")) {
        MR::onDrawSpinDriverPathAtOpa();
    }
    setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvAppearWait));
}

void StarReturnDemoStarter::kill() {
    LiveActor::kill();
    mLuma->kill();


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

    }
}

void StarReturnDemoStarter::tryStartStageResult() {
    if (MR::isDemoPartLastStep(::cDemoWaitPartName)) {
        if (GameSequenceFunction::hasStageResultSequence()) {
            mStageResultInformer->appear();
            MR::requestMovementOn(mStageResultInformer);
        }

        MR::pauseTimeKeepDemo(this);
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvStageResult));
    }
}

void StarReturnDemoStarter::tryStartResultWait() {
    if (MR::isBckOneTimeAndStoppedPlayer()) {
        MR::startBckPlayer("ResultWait", nullptr);
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

    }

    if (MR::isStep(this, ::cDemoAppearEffectStep)) {
        MR::emitEffect(mPowerStar, "DemoFlyLightDemo");
        MR::startActionSound(mPowerStar, "PmDemoReturnLight");
    }

    MR::setNerveAtStep(this, GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvMove), ::cDemoAppearWaitFrame);
}

void StarReturnDemoStarter::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::showPlayer();
        MR::showModel(mPowerStar);
        mReturnDemoRailMove->start();
    }
    const char* part = cDemoMovePartName;
    s32 step = MR::getDemoPartStep(part) - 44;
    s32 total = MR::getDemoPartTotalStep(part) - 45;
    mReturnDemoRailMove->update(step, total);
    PowerStar::requestPointLightAtResultSequence(mPowerStar);
    MR::startLevelSoundPlayer("SE_PM_LV_SPIN_DRV_FLY", -1);
    if (MR::isDemoPartLastStep(part)) {
        bool grandStar = GameDataConst::isGrandStar(GameSequenceFunction::getClearedStageName(), GameSequenceFunction::getClearedPowerStarId());
        setNerve(grandStar ? static_cast< const Nerve* >(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvFlyWaitGrandStar))
                          : GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvFlyWaitPowerStar));
    }
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

    if (MR::isLessStep(this, static_cast< s16 >(MR::getBckFrameMaxPlayer("ResultFlyGrandStarEnd")) - mReturnDemoRailMove->getDemoFlyBrakeFrame())) {
        MR::startActionSound(mPowerStar, "OjLvRetPowStarFly");
    }

    if (MR::isBckOneTimeAndStoppedPlayer()) {
        MR::startBckPlayer("ResultFlyGrandStarFlyAway", nullptr);
        MR::startAction(mPowerStar, "ResultFlyGrandStarFlyAway");
        MR::startActionSound(mPowerStar, "OjGrandStarEscape");
        MR::shakeCameraWeak();
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
    }

    if (MR::isStep(this, ::cFallStartFrameGrandStar)) {
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvFall));
    }
}

// NONMATCHING: vector operation scheduling
void StarReturnDemoStarter::exeFall() {
    TVec3f pos;
    pos.set< f32 >(mTransform.mMtx[0][3], mTransform.mMtx[1][3], mTransform.mMtx[2][3]);
    if (MR::isFirstStep(this)) {
        MR::startSoundPlayer("SE_PV_DM_ASTRO_RET_LANDING", -1, -1);
        MR::startBckPlayer("Fall", nullptr);
        mPowerStar->kill();
        MR::calcGravityVector(this, pos, &mFallGravity, nullptr, 0);
        TVec3f line;
        line.scale(3000.0f, mFallGravity);
        MR::getFirstPolyOnLineToMap(&mPosition, nullptr, pos, line);
    }
    f32 speed = 1.8f * getNerveStep();
    speed = speed >= 35.0f ? 35.0f : speed;
    if (PSVECDistance(&pos, &mPosition) < speed) {
        pos.x = mPosition.x;
        pos.y = mPosition.y;
        pos.z = mPosition.z;
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvLand));
    } else {
        TVec3f velocity;
        velocity.scale(speed, mFallGravity);
        pos += velocity;
    }
    mTransform.mMtx[0][3] = pos.x;
    mTransform.mMtx[1][3] = pos.y;
    mTransform.mMtx[2][3] = pos.z;
}

void StarReturnDemoStarter::exeLand() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("Land", nullptr);
        MR::tryRumblePadMiddle(this, 0);
    }
    if (MR::isBckStoppedPlayer()) {
        LiveActor* luma = mLuma;
        luma->appear();
        MR::startAction(luma, playerAnimName());
        setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvWait));
    }
}

void StarReturnDemoStarter::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::changePlayerAnimationAndBva(playerAnimName());
    }
    tryStartResultWait();
    tryStartStageResult();
}

void StarReturnDemoStarter::exeStageResult() {
    tryStartResultWait();
    if (MR::isDead(mStageResultInformer) && MR::isEqualStringCase(MR::getPlayerCurrentBckName(), "ResultWait")) {
        if (GameSequenceFunction::isMarioFaceShipEventEmpty() && !MR::isWipeBlank()) {
            setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvFadeOut));
        } else {
            setNerve(GET_NERVE(StarReturnDemoStarter, StarReturnDemoStarterNrvStageResultAfter));
        }
    }
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
        MR::startCurrentStageBGM(false);
        MR::overlayWithPreviousScreen(3);
        MR::endStartPosCamera();
    }
    if (!MR::isTimeKeepDemoActive()) {
        kill();
    }
}

StarReturnDemoStarter::~StarReturnDemoStarter() {}
