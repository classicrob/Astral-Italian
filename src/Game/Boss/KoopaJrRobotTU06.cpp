#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"

static const char* sCameraName = "DemoOpeningKoopaJrRobot";
static const char* sActionName = "DemoOpeningKoopaJrRobot";

namespace NrvKoopaJrRobotStateIntro {
    NEW_NERVE(KoopaJrRobotStateIntroNrvWait, KoopaJrRobotStateIntro, Wait);
    NEW_NERVE(KoopaJrRobotStateIntroNrvDemo, KoopaJrRobotStateIntro, Demo);
};  // namespace NrvKoopaJrRobotStateIntro

const char* KoopaJrRobotStateIntro::getCameraName() {
    return sCameraName;
}

KoopaJrRobotStateIntro::KoopaJrRobotStateIntro(KoopaJrRobot* pHost, const JMapInfoIter& rIter)
    : KoopaJrRobotStateBase("State:\x83""f\x83\x82[\x83o\x83g\x83\x8b\x8aJ\x8en]", pHost) {
    initNerve(&NrvKoopaJrRobotStateIntro::KoopaJrRobotStateIntroNrvWait::sInstance, 0);
    KoopaJrRobot* pRobot = mHost;
    const char* pAction = sActionName;
    LiveActor* head = pRobot->mHeadModel;
    MR::initDemoSheetTalkAnim(pRobot, rIter, KoopaJrRobotFunction::getDemoName(), pAction, nullptr, TVec3f(0.0f, 100.0f, 0.0f), MR::getJointMtx(head, "Root"));
    LiveActor* pBody = mHost->mBody;
    MR::initTalkAnimDemoCast(pBody, rIter, KoopaJrRobotFunction::getDemoName(), pAction);
    LiveActor* pHead = mHost->mHeadModel;
    MR::initTalkAnimDemoCast(pHead, rIter, KoopaJrRobotFunction::getDemoName(), pAction);
}

void KoopaJrRobotStateIntro::kill() {
    if (MR::isDemoEndWaitCameraInterpolating()) {
        MR::startBckNoInterpole(mHost, "Wait");
    }
    KoopaJrRobotFunction::attachBodyToHead(mHost);
    mHost->mSeq->appearLaunchers();
    KoopaJrRobotFunction::showStarPieces(mHost);
    mIsDead = true;
}

void KoopaJrRobotStateIntro::exeWait() {
    if (MR::isFirstStep(this) && MR::isGalaxyDarkCometAppearInCurrentStage()) {
        if (GameSequenceFunction::isResultMiss()) {
            MR::forceOpenSystemWipeCircle();
        }
    }
    KoopaJrRobot* pRobot = mHost;
    if (MR::tryStartTimeKeepDemoMarioPuppetable(pRobot, KoopaJrRobotFunction::getDemoName(), nullptr)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sCameraName, 0, false, 1.0f);
        MR::overlayWithPreviousScreen(2);
        setNerve(&NrvKoopaJrRobotStateIntro::KoopaJrRobotStateIntroNrvDemo::sInstance);
    }
}

void KoopaJrRobotStateIntro::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::setPlayerPosOnGround("\x8aJ\x8en\x83""f\x83\x82");
        MR::startBckPlayer("BattleWait", nullptr);
        const char* pAction = sActionName;
        MR::startAction(mHost, pAction);
        MR::startAction(mHost->mHeadModel, pAction);
        MR::startAction(mHost->mBody, pAction);
        MR::showModel(mHost);
        MR::showModel(mHost->mBody);
        MR::showModel(mHost->mHeadModel);
        MR::stopStageBGM(30);
        mHost->mSeq->killLaunchers();
        KoopaJrRobotFunction::attachBodyToAllRoot(mHost);
        KoopaJrRobotFunction::hideStarPieces(mHost);
    }
    if (MR::checkPassBckFrame(mHost->mBody, 360.0f)) {
        KoopaJrRobotFunction::hideAll(mHost);
    }
    if (MR::checkPassBckFrame(mHost->mBody, 902.0f)) {
        KoopaJrRobotFunction::showAll(mHost);
    }
    KoopaJrRobot* pHost = mHost;
    if (MR::tryEndDemoAndKillEffects(pHost, KoopaJrRobotFunction::getDemoName())) {
        KoopaJrRobotFunction::showAll(mHost);
        kill();
    }
}

KoopaJrRobotStateIntro::~KoopaJrRobotStateIntro() {}
