#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/BreakModel.hpp"


static const char* sDemoName = "\x83N\x83""b\x83pJr\x83\x8d\x83{\x83""b\x83g\x83_\x83""E\x83\x93\x83""f\x83\x82";
static const char* sCameraName = "DemoDown";
static const char* sActionName = "DemoDown";
static const char* sHeadActionName = "DemoDownKoopaJrRobot";

namespace NrvKoopaJrRobotStateTU07 {
    NEW_NERVE(KoopaJrRobotStateTU07NrvWait, KoopaJrRobotStateTU07, Wait);
    NEW_NERVE(KoopaJrRobotStateTU07NrvDown, KoopaJrRobotStateTU07, Down);
    NEW_NERVE(KoopaJrRobotStateTU07NrvEnd, KoopaJrRobotStateTU07, End);
};  // namespace NrvKoopaJrRobotStateTU07

const char* KoopaJrRobotStateTU07::getCameraName() {
    return sCameraName;
}

KoopaJrRobotStateTU07::KoopaJrRobotStateTU07(KoopaJrRobot* pHost)
    : ActorStateBase< KoopaJrRobot >("State:\x83""f\x83\x82[\x83_\x83""E\x83\x93]", pHost), mBreakModel(nullptr) {
    mBreakModel = MR::createBreakModel(pHost, nullptr);
    initNerve(&NrvKoopaJrRobotStateTU07::KoopaJrRobotStateTU07NrvWait::sInstance, 0);
}

void KoopaJrRobotStateTU07::kill() {
    KoopaJrRobotFunction::showStarPieces(mHost);
    mIsDead = true;
}

void KoopaJrRobotStateTU07::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::preparePlayerForPositionReset();
    }
    if (MR::tryStartDemoMarioPuppetable(mHost, sDemoName)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sCameraName, 0, false, 1.0f);
        MR::overlayWithPreviousScreen(2);
        setNerve(&NrvKoopaJrRobotStateTU07::KoopaJrRobotStateTU07NrvDown::sInstance);
    }
}

void KoopaJrRobotStateTU07::exeDown() {
    if (MR::isFirstStep(this)) {
        const char* pAction = sActionName;
        MR::startAction(mHost, pAction);
        MR::startAction(mHost->mBody, pAction);
        MR::startAction(mHost->mHeadModel, sHeadActionName);
        MR::setPlayerPosOnGround("\x8aJ\x8en\x83""f\x83\x82");
        MR::startBckPlayer("BattleWait", nullptr);
        MR::hidePlayerDeleteEffect();
        mHost->mSeq->killLaunchers();
        mHost->mCollisionA->kill();
        mHost->mCollisionB->kill();
        KoopaJrRobotFunction::attachBodyToAllRoot(mHost);
        KoopaJrRobotFunction::hideStarPieces(mHost);
    }
    if (MR::isStep(this, 380)) {
        mBreakModel->appear();
        mHost->killParts();
    }
    if (MR::isActionEnd(mHost)) {
        setNerve(&NrvKoopaJrRobotStateTU07::KoopaJrRobotStateTU07NrvEnd::sInstance);
    }
}

void KoopaJrRobotStateTU07::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, sCameraName, 0, true);
        MR::appearPowerStarContinueCurrentDemo(mHost, mHost->mPosition);
        mHost->mBody->kill();
        mHost->mHeadModel->kill();
    }
    if (MR::isEndPowerStarAppearDemo(mHost)) {
        MR::showPlayer();
        MR::endDemo(mHost, sDemoName);
        kill();
    }
}

KoopaJrRobotStateTU07::~KoopaJrRobotStateTU07() {}
