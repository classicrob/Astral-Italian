#include "Game/NPC/TicoFatStarPieceTransformation.hpp"
#include "Game/NPC/TicoFatStarPiece.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Screen/FullnessMeter.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/Util.hpp"

namespace NrvTicoFatStarPieceTransformation {
    NEW_NERVE(WaitDemo, TicoFatStarPieceTransformation, WaitDemo);
    NEW_NERVE(WaitTalk, TicoFatStarPieceTransformation, WaitTalk);
    NEW_NERVE(Fullness, TicoFatStarPieceTransformation, Fullness);
    NEW_NERVE(Transform, TicoFatStarPieceTransformation, Transform);
    NEW_NERVE(Fly, TicoFatStarPieceTransformation, Fly);
    NEW_NERVE(CreateRoute, TicoFatStarPieceTransformation, CreateRoute);
}

#pragma auto_inline on
TicoFatStarPieceState::~TicoFatStarPieceState() {}
#pragma auto_inline reset
TicoFatStarPieceTransformation::~TicoFatStarPieceTransformation() {}

TicoFatStarPieceTransformation::TicoFatStarPieceTransformation(TicoFatStarPiece* host, const JMapInfoIter& iter)
    : TicoFatStarPieceState("ハラペコスターピースチコ[変身]", host), mCameraInfo(nullptr) {
    MR::initMultiActorCamera(host, iter, &mCameraInfo, "変身");
    MR::initMultiActorCamera(mHost, iter, &mCameraInfo, "飛行");
    initNerve(&NrvTicoFatStarPieceTransformation::WaitDemo::sInstance);
}

void TicoFatStarPieceTransformation::exeWaitDemo() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(mHost);
        TicoFatStarPiece* host = mHost;
        MR::tryStartActionNotPlaying(host, host->getActionName("Excite"));
    }
    if (MR::tryStartDemo(mHost, "変身")) {
        MR::invalidateHitSensors(mHost);
        MR::stopPlayerFpView();
        MR::requestMovementOn(mHost->mGoods0);
        MR::requestMovementOn(mHost->mGoods1);
        mHost->mFullnessMeter->pauseOff();
        setNerve(&NrvTicoFatStarPieceTransformation::WaitTalk::sInstance);
    }
}

void TicoFatStarPieceTransformation::exeWaitTalk() {
    if (!MR::isLessStep(this, 20) && MR::isTalkNone(mHost->mEatTalkCtrl)) {
        mHost->selectEatMessage(2);
        setNerve(&NrvTicoFatStarPieceTransformation::Fullness::sInstance);
    }
}

void TicoFatStarPieceTransformation::exeFullness() {
    if (MR::isFirstStep(this)) {
        TicoFatStarPiece* host = mHost;
        MR::tryStartActionNotPlaying(host, host->getActionName("Joy"));
        MR::startActionSound(mHost, "SmTicofatMax", -1, -1, -1);
        mHost->tryEndEventCamera();
    }
    mHost->startEatCompleteSound();
    if (MR::tryTalkForceWithoutDemoAtEnd(mHost->mEatTalkCtrl)) setNerve(&NrvTicoFatStarPieceTransformation::Transform::sInstance);
}

// NONMATCHING: remaining differences are local literal relocation identities.
void TicoFatStarPieceTransformation::exeTransform() {
    if (MR::isFirstStep(this)) {
        MR::startMultiActorCameraTargetSelf(mHost, mCameraInfo, "変身", -1);
        TicoFatStarPiece* host = mHost;
        MR::startAction(host, host->getActionName("Demo"));
    }
    MR::startActionSound(mHost, "VolDownBgm", -1, -1, -1);
    if (MR::isActionEnd(mHost)) {
        MR::hideModelAndOnCalcAnim(mHost->mGoods0);
        MR::hideModelAndOnCalcAnim(mHost->mGoods1);
        MR::endMultiActorCamera(mHost, mCameraInfo, "変身", false, -1);
        setNerve(&NrvTicoFatStarPieceTransformation::Fly::sInstance);
    }
}

// NONMATCHING: remaining differences are local literal relocation identities.
void TicoFatStarPieceTransformation::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(mHost);
        MR::tryRumblePadMiddle(this, 0);
        MR::startAction(mHost, "Fly");
        MR::startActionSound(mHost, "Fly", -1, -1, -1);
        MR::startMultiActorCameraTargetSelf(mHost, mCameraInfo, "飛行", -1);
        mStartPos.set<f32>(mHost->mPosition);
        MR::calcRailEndPos(&mEndPos, mHost);
        TVec3f direction = mEndPos - mStartPos;
        MR::normalizeOrZero(&direction);
        mEndPos.set<f32>(mStartPos + direction * 50000.0f);
        MR::hideModelAndOnCalcAnim(mHost);
    }
    MR::startActionSound(mHost, "VolDownBgm", -1, -1, -1);
    f32 rate = MR::calcNerveRate(this, 120);
    TicoFatStarPiece* host = mHost;
    MR::setPosition(host, mStartPos * (1.0f - rate) + mEndPos * rate);
    if (MR::isStep(this, 120)) {
        MR::turnPlayerToActor(mHost, 180.0f);
        setNerve(&NrvTicoFatStarPieceTransformation::CreateRoute::sInstance);
        mHost->mGoods0->kill();
        mHost->mGoods1->kill();
    }
}

// NONMATCHING: remaining differences are local literal relocation identities.
void TicoFatStarPieceTransformation::exeCreateRoute() {
    if (MR::isFirstStep(this)) {
        MR::getSceneObj<WorldMapResultCommand>(SceneObj_WorldMapResultCommand)->startRouteCreateTico();
    }
    if (MR::isSystemWipeActive()) {
        TVec3f direction = mEndPos - mStartPos;
        MR::normalizeOrZero(&direction);
        s32 frames = 120;
        TicoFatStarPiece* host = mHost;
        MR::setPosition(host, mEndPos + direction * (50000.0f / static_cast<f32>(frames)));
        MR::startActionSound(mHost, "VolDownBgm", -1, -1, -1);
    }
    MR::getSceneObj<WorldMapResultCommand>(SceneObj_WorldMapResultCommand)->updateController();
    if (MR::getSceneObj<WorldMapResultCommand>(SceneObj_WorldMapResultCommand)->isStateDead()) {
        MR::endMultiActorCamera(mHost, mCameraInfo, "飛行", false, -1);
        MR::getSceneObj<WorldMapResultCommand>(SceneObj_WorldMapResultCommand)->endCurrent();
        MR::endDemo(mHost, "変身");
        kill();
    }
}
