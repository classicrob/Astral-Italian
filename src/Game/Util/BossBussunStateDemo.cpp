#include "Game/Util/BossBussun.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvBossBussunStateDemo {
    // Empty execute in the target (blr): hand-written, no exe call.
    class BossBussunStateDemoNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static BossBussunStateDemoNrvWait sInstance;
    };
    BossBussunStateDemoNrvWait BossBussunStateDemoNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(BossBussunStateDemoNrvDemo, BossBussunStateDemo, Demo);
    NEW_NERVE(BossBussunStateDemoNrvWaitEnd, BossBussunStateDemo, WaitEnd);
};  // namespace NrvBossBussunStateDemo

BossBussunStateDemo::BossBussunStateDemo(BossBussun* pHost, const char* pDemoName, bool pIsPowerStar)
    : ActorStateBase< BossBussun >("ボスブッスンデモ状態", pHost), mDemoName(pDemoName), mIsPowerStarDemo(pIsPowerStar) {
    initNerve(&NrvBossBussunStateDemo::BossBussunStateDemoNrvWait::sInstance, 0);
    MR::initAnimCamera(mHost, mHost->mCameraInfo, mDemoName);
}

void BossBussunStateDemo::appear() {
    mIsDead = false;
    MR::showModel(mHost);
    MR::requestStartDemo(this, mHost, mDemoName, GET_NERVE(BossBussunStateDemo, BossBussunStateDemoNrvDemo), &NrvBossBussunStateDemo::BossBussunStateDemoNrvWait::sInstance);
    mHost->resetAnimScale();
}

void BossBussunStateDemo::exeDemo() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(mHost);
        mHost->resetToHomePos();
        MR::startAction(mHost, mDemoName);
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, mDemoName, 0, false, 1.0f);
        MR::overlayWithPreviousScreen(2);
        MR::hidePlayer();
        MR::setPlayerLinkPosAndWait(mHost, "マリオ位置", false);
    }
    if (mIsPowerStarDemo) {
        if (MR::isActionEnd(mHost)) {
            MR::endDemo(mHost, mDemoName);
            MR::tryHitReactionDead(mHost);
            MR::requestAppearPowerStarFromBelow(mHost);
            MR::endAnimCamera(mHost, mHost->mCameraInfo, mDemoName, 0, true);
            MR::showPlayer();
            kill();
        }
    } else {
        BossBussun* host = mHost;
        if (MR::tryEndDemo(host, mDemoName, MR::isActionEnd(host))) {
            MR::endAnimCamera(mHost, mHost->mCameraInfo, mDemoName, 0, true);
            MR::showPlayer();
            mHost->mSequencer->startCamera();
            kill();
        }
    }
}

void BossBussunStateDemo::exeWaitEnd() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(mHost);
    }
    if (MR::isEndPowerStarAppearDemo(mHost)) {
        MR::endAnimCamera(mHost, mHost->mCameraInfo, mDemoName, 0, true);
        MR::showPlayer();
        kill();
    }
}

BossBussunStateDemo::~BossBussunStateDemo() {
}
