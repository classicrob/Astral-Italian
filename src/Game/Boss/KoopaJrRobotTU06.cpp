#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

extern "C" const char* fn_800DA9C0();
extern "C" bool fn_804D7060();

static const char* sCameraName = "DemoOpeningKoopaJrRobot";
static const char* sActionName = "DemoOpeningKoopaJrRobot";

namespace NrvKoopaJrRobotStateIntro {
    NEW_NERVE(KoopaJrRobotStateIntroNrvWait, KoopaJrRobotStateIntro, Wait);
    NEW_NERVE(KoopaJrRobotStateIntroNrvDemo, KoopaJrRobotStateIntro, Demo);
};  // namespace NrvKoopaJrRobotStateIntro

const char* KoopaJrRobotStateIntro::getCameraName() {
    return sCameraName;
}

// NONMATCHING: instruction scheduling of the Root joint mtx load (lwz r3,236 hoisted in target)
KoopaJrRobotStateIntro::KoopaJrRobotStateIntro(KoopaJrRobot* pHost, const JMapInfoIter& rIter)
    : KoopaJrRobotPhase("State:\x83""f\x83\x82[\x83o\x83g\x83\x8b\x8aJ\x8en]", pHost) {
    initNerve(&NrvKoopaJrRobotStateIntro::KoopaJrRobotStateIntroNrvWait::sInstance, 0);
    KoopaJrRobot* pRobot = mHost;
    const char* pAction = sActionName;
    MR::initDemoSheetTalkAnim(pRobot, rIter, fn_800DA9C0(), pAction, nullptr, TVec3f(0.0f, 100.0f, 0.0f), MR::getJointMtx(pRobot->mHeadModel, "Root"));
    LiveActor* pBody = mHost->mBody;
    MR::initTalkAnimDemoCast(pBody, rIter, fn_800DA9C0(), pAction);
    LiveActor* pHead = mHost->mHeadModel;
    MR::initTalkAnimDemoCast(pHead, rIter, fn_800DA9C0(), pAction);
}

void KoopaJrRobotStateIntro::kill() {
    if (MR::isDemoEndWaitCameraInterpolating()) {
        MR::startBckNoInterpole(mHost, "Wait");
    }
    KoopaJrRobotFunction::attachBodyToHead(mHost);
    mHost->mSeq->fn_800DC5B0();
    KoopaJrRobotFunction::showStarPieces(mHost);
    mIsDead = true;
}

void KoopaJrRobotStateIntro::exeWait() {
    if (MR::isFirstStep(this) && MR::isGalaxyDarkCometAppearInCurrentStage()) {
        if (fn_804D7060()) {
            MR::forceOpenSystemWipeCircle();
        }
    }
    KoopaJrRobot* pRobot = mHost;
    if (MR::tryStartTimeKeepDemoMarioPuppetable(pRobot, fn_800DA9C0(), nullptr)) {
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
        mHost->mSeq->fn_800DC6B0();
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
    if (MR::tryEndDemoAndKillEffects(pHost, fn_800DA9C0())) {
        KoopaJrRobotFunction::showAll(mHost);
        kill();
    }
}

KoopaJrRobotStateIntro::~KoopaJrRobotStateIntro() {}
