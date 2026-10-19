#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"


namespace { const char* sOpeningDemoName = "Opening"; }

namespace NrvTwoLegsStateOpening {
    NEW_NERVE(TwoLegsStateOpeningNrvWaitDemo, TwoLegsStateOpening, WaitDemo);
    NEW_NERVE(TwoLegsStateOpeningNrvDemo, TwoLegsStateOpening, Demo);
};  // namespace NrvTwoLegsStateOpening

TwoLegsStateOpening::TwoLegsStateOpening(TwoLegs* pHost) : ActorStateBase< TwoLegs >("State[\x83I\x81[\x83v\x83j\x83\x93\x83O\x83""f\x83\x82]", pHost) {}

TwoLegsStateOpening::~TwoLegsStateOpening() {}

void TwoLegsStateOpening::init() {
    MR::createCenterScreenBlur();
    initNerve(&NrvTwoLegsStateOpening::TwoLegsStateOpeningNrvWaitDemo::sInstance, 0);
}

void TwoLegsStateOpening::appear() {
    mIsDead = false;
    setNerve(&NrvTwoLegsStateOpening::TwoLegsStateOpeningNrvWaitDemo::sInstance);
}

void TwoLegsStateOpening::kill() {
    MR::emitEffect(mHost, "Eye");
    MR::showPlayer();
    mIsDead = true;
}

void TwoLegsStateOpening::exeWaitDemo() {
    if (MR::tryStartDemo(mHost, sOpeningDemoName)) {
        MR::stopStageBGM(30);
        MR::startAnimCameraTargetSelf(mHost, mHost->mCameraInfo, "Opening", nullptr, 0, 1.0f);
        MR::overlayWithPreviousScreen(2);
        setNerve(&NrvTwoLegsStateOpening::TwoLegsStateOpeningNrvDemo::sInstance);
    }
}

void TwoLegsStateOpening::exeDemo() {
    if (MR::isFirstStep(this)) {
        mHost->resetToInitPos();
        MR::startAction(mHost, "Opening");
        MR::hidePlayer();
    }
    if (MR::isStep(this, 180)) {
        MR::emitEffect(mHost, "Eye");
    }
    if (MR::isStep(this, 200)) {
        mHost->jointSpinOn();
    }
    if (MR::isStep(this, 470)) {
        MR::startCenterScreenBlur(100, 15.0f, 80, 5, 30);
    }
    if (MR::tryEndDemoAndAnimCamera(mHost, mHost->mCameraInfo, sOpeningDemoName, 0)) {
        kill();
    }
}
