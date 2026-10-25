#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"


namespace {
    char sDemoNameText[] = "\x83N\x83""b\x83pJr\x83\x8d\x83{\x83""b\x83g\x8e\xe3\x93_\x8fo\x8c\xbb\x83""f\x83\x82";
    char sCameraNameText[] = "DemoWeakPointHead";
    extern const char* const sDemoName;
    extern const char* const sCameraName;
    extern const char* const sActionName;
}

namespace NrvKoopaJrRobotStateTU08 {
    NEW_NERVE(KoopaJrRobotStateTU08NrvWait, KoopaJrRobotStateTU08, Wait);
    NEW_NERVE(KoopaJrRobotStateTU08NrvDemo, KoopaJrRobotStateTU08, Demo);
};  // namespace NrvKoopaJrRobotStateTU08

const char* KoopaJrRobotStateTU08::getCameraName() {
    return sCameraName;
}

KoopaJrRobotStateTU08::KoopaJrRobotStateTU08(KoopaJrRobot* pHost)
    : ActorStateBase< KoopaJrRobot >("State:\x83""f\x83\x82[\x8d\xc5\x8c\xe3\x82\xcc\x8e\xe3\x93_\x8fo\x8c\xbb]", pHost) {
    initNerve(&NrvKoopaJrRobotStateTU08::KoopaJrRobotStateTU08NrvWait::sInstance, 0);
}

void KoopaJrRobotStateTU08::kill() {
    if (MR::isDemoEndWaitCameraInterpolating()) {
        MR::setAllAnimFrameAtEnd(mHost->mBody, sActionName);
    }
    mHost->mSeq->appearLaunchers();
    KoopaJrRobotFunction::attachBodyToHead(mHost);
    KoopaJrRobotFunction::showStarPieces(mHost);
    KoopaJrRobotFunction::onDrawEffect(mHost);
    mIsDead = true;
}

void KoopaJrRobotStateTU08::exeWait() {
    KoopaJrRobot* host = mHost;
    if (MR::tryStartDemo(host, sDemoName)) {
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, sCameraName, 0, false, 1.0f);
        MR::overlayWithPreviousScreen(2);
        setNerve(&NrvKoopaJrRobotStateTU08::KoopaJrRobotStateTU08NrvDemo::sInstance);
    }
}

namespace {
    const char* const sDemoName = sDemoNameText;
    const char* const sCameraName = sCameraNameText;
    const char* const sActionName = sCameraNameText;
}

void KoopaJrRobotStateTU08::exeDemo() {
    if (MR::isFirstStep(this)) {
        if (!KoopaJrRobotFunction::isPlayerInRushAir()) {
            MR::turnDirectionToPlayerDegreeHorizon(mHost, &mHost->mZAxis, 180.0f);
        }
        const char* pAction = sActionName;
        MR::startAction(mHost, pAction);
        MR::startAction(mHost->mBody, pAction);
        MR::startAction(mHost->mHeadModel, "DemoWeakPointHeadKoopaJrRobot");
        mHost->mParts[3]->appear();
        MR::validateHitSensor(mHost, "WeakPointHead");
        mHost->mSeq->killLaunchers();
        if (!MR::isDead(mHost->mCollisionA)) {
            mHost->mCollisionA->kill();
        }
        if (!MR::isDead(mHost->mCollisionB)) {
            mHost->mCollisionB->kill();
        }
        if (!MR::isDead(mHost->mCollisionC)) {
            mHost->mCollisionC->kill();
        }
        MR::hidePlayer();
        KoopaJrRobotFunction::offDrawEffect(mHost);
        KoopaJrRobotFunction::attachBodyToAllRoot(mHost);
        KoopaJrRobotFunction::hideStarPieces(mHost);
    }
    KoopaJrRobot* pHost = mHost;
    if (MR::tryEndDemo(pHost, sDemoName, MR::isActionEnd(pHost))) {
        MR::showPlayer();
        MR::endAnimCamera(mHost, mHost->mCameraInfo, sCameraName, 0, true);
        kill();
    }
}

KoopaJrRobotStateTU08::~KoopaJrRobotStateTU08() {}
