#include "Game/Screen/SupportPlayGuidance.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/NPC/SupportTico.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
namespace NrvSupportPlayGuidance {
    class SupportPlayGuidanceNrvWaitDemo : public Nerve { public: virtual void execute(Spine*) const; static SupportPlayGuidanceNrvWaitDemo sInstance; };
    SupportPlayGuidanceNrvWaitDemo SupportPlayGuidanceNrvWaitDemo::sInstance;
    NEW_NERVE(SupportPlayGuidanceNrvAppear, SupportPlayGuidance, Appear);
    NEW_NERVE(SupportPlayGuidanceNrvWait, SupportPlayGuidance, Wait);
    NEW_NERVE(SupportPlayGuidanceNrvDisappear, SupportPlayGuidance, Disappear);
}
using namespace NrvSupportPlayGuidance;
SupportPlayGuidance::SupportPlayGuidance() : LayoutActor("LayoutActor", true), mIconAButton(0) {}
void SupportPlayGuidance::init(const JMapInfoIter&) {
    initLayoutManager("SupportPlayGuidance", 1);
    MR::createAndAddPaneCtrl(this, "AButtonPosition", 1);
    initActionSound(2, 0, 0);
    initNerve(&SupportPlayGuidanceNrvAppear::sInstance);
    MR::connectToSceneLayout(this);
    mIconAButton = new IconAButton(true, false);
    mIconAButton->initWithoutIter();
    mIconAButton->setFollowActorPane(this, "AButtonPosition");
    MR::registerDemoSimpleCastAll(mIconAButton);
    kill();
}
void SupportPlayGuidance::appear() {
    LayoutActor::appear();
    MR::hideScreen(this);
    setNerve(&SupportPlayGuidanceNrvWaitDemo::sInstance);
}
void SupportPlayGuidance::kill() {
    LayoutActor::kill();
    mIconAButton->kill();
    MR::hideScreen(this);
}
void SupportPlayGuidance::requestAppear() { if (MR::isDead(this)) appear(); }
namespace {
    char sSupportGuidanceDemo[] = "サポートプレイガイダンスデモ";
}

void SupportPlayGuidance::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::showScreen(this);
        MR::startAnim(this, "Appear", 0);
        MR::startActionSoundAuto(this, "Appear");
        mIconAButton->openWithoutMessage();
    }
    if (MR::isAnimStopped(this, 0)) setNerve(&SupportPlayGuidanceNrvWait::sInstance);
}
void SupportPlayGuidance::exeWait() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Wait", 0);
    if (SupportTicoStateFunction::isTriggerA()) setNerve(&SupportPlayGuidanceNrvDisappear::sInstance);
}
void SupportPlayGuidance::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
        MR::startActionSoundAuto(this, "Disappear");
        MR::startCSSound2P("CS_CLICK_OPEN", 0, 0, 1.0f);
        mIconAButton->term();
    }
    if (MR::isAnimStopped(this, 0)) {
        MR::endDemo(this, sSupportGuidanceDemo);
        kill();
    }
}
SupportPlayGuidance::~SupportPlayGuidance() {}
namespace NrvSupportPlayGuidance {
    void SupportPlayGuidanceNrvWaitDemo::execute(Spine* spine) const {
        SupportPlayGuidance* actor = static_cast<SupportPlayGuidance*>(spine->mExecutor);
        if (MR::tryStartDemo(actor, sSupportGuidanceDemo)) actor->setNerve(&SupportPlayGuidanceNrvAppear::sInstance);
    }
}
