#include "Game/Screen/DreamerButtonGuidance.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvDreamerButtonGuidance {
    NEW_NERVE(DreamerButtonGuidanceNrvAppear, DreamerButtonGuidance, Appear);
    class DreamerButtonGuidanceNrvWait : public Nerve { public: virtual void execute(Spine*) const; static DreamerButtonGuidanceNrvWait sInstance; };
    DreamerButtonGuidanceNrvWait DreamerButtonGuidanceNrvWait::sInstance;
    NEW_NERVE(DreamerButtonGuidanceNrvEnd, DreamerButtonGuidance, End);
}
using namespace NrvDreamerButtonGuidance;
DreamerButtonGuidance::DreamerButtonGuidance(const char* name) : LayoutActor(name, true) {}
void DreamerButtonGuidance::init(const JMapInfoIter&) {
    initLayoutManager("ButtonGuidance", 1);
    MR::connectToSceneLayout(this);
    MR::registerDemoSimpleCastAll(this);
    initNerve(&DreamerButtonGuidanceNrvAppear::sInstance);
    kill();
}
void DreamerButtonGuidance::kill() { LayoutActor::kill(); }
void DreamerButtonGuidance::appear() { LayoutActor::appear(); setNerve(&DreamerButtonGuidanceNrvAppear::sInstance); }
void DreamerButtonGuidance::exeAppear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Appear", 0);
    if (MR::isAnimStopped(this, 0)) setNerve(&DreamerButtonGuidanceNrvWait::sInstance);
}
void DreamerButtonGuidance::exeEnd() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "End", 0);
    if (MR::isAnimStopped(this, 0)) kill();
}
namespace {
    char sGuidanceWaitAnimation[] = "Wait";
}

void DreamerButtonGuidance::showDreamer() { MR::hidePane(this, "Esper"); MR::hidePane(this, "Exit"); MR::showPane(this, "Dreamer"); }
void DreamerButtonGuidance::showSuperDreamer() { MR::hidePane(this, "Dreamer"); MR::showPane(this, "Esper"); MR::showPane(this, "Exit"); }
void DreamerButtonGuidance::forceAppear() { appear(); setNerve(&DreamerButtonGuidanceNrvWait::sInstance); }
void DreamerButtonGuidance::disappear() { setNerve(&DreamerButtonGuidanceNrvEnd::sInstance); }
void DreamerButtonGuidance::hideExit() { MR::hidePane(this, "Exit"); }
void DreamerButtonGuidance::showExit() { MR::showPane(this, "Exit"); }
DreamerButtonGuidance::~DreamerButtonGuidance() {}
inline void NrvDreamerButtonGuidance::DreamerButtonGuidanceNrvWait::execute(Spine* spine) const {
    DreamerButtonGuidance* actor = static_cast<DreamerButtonGuidance*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startAnim(actor, sGuidanceWaitAnimation, 0);
}
