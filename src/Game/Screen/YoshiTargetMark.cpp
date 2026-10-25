#include "Game/Screen/YoshiTargetMark.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvYoshiTargetMark {
    NEW_NERVE(YoshiTargetMarkNrvAppear, YoshiTargetMark, Appear);
    class YoshiTargetMarkNrvWait : public Nerve { public: virtual void execute(Spine*) const; static YoshiTargetMarkNrvWait sInstance; };
    YoshiTargetMarkNrvWait YoshiTargetMarkNrvWait::sInstance;
    NEW_NERVE(YoshiTargetMarkNrvDisappear, YoshiTargetMark, Disappear);
}
using namespace NrvYoshiTargetMark;
YoshiTargetMark::YoshiTargetMark(const char* name) : LayoutActor(name, true) {}
void YoshiTargetMark::init(const JMapInfoIter&) {
    initLayoutManager("YoshiTargetMark", 2);
    MR::connectToSceneLayout(this);
    initNerve(&YoshiTargetMarkNrvAppear::sInstance);
    kill();
    MR::registerDemoSimpleCastAll(this);
}
void YoshiTargetMark::appear() { LayoutActor::appear(); setNerve(&YoshiTargetMarkNrvAppear::sInstance); MR::hideLayout(this); }
void YoshiTargetMark::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "LockOn", 0);
        MR::startAnim(this, "Appear", 1);
        MR::showLayout(this);
    }
    if (MR::isAnimStopped(this, 0)) setNerve(&YoshiTargetMarkNrvWait::sInstance);
}
void YoshiTargetMark::exeDisappear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "End", 1);
    if (MR::isAnimStopped(this, 1)) kill();
}
// NONMATCHING: the compiler folds the target's conditional jump and following unconditional jump.
void YoshiTargetMark::disappear() {
    if (MR::isDead(this) || isNerve(&YoshiTargetMarkNrvDisappear::sInstance)) {
        return;
    }
    setNerve(&YoshiTargetMarkNrvDisappear::sInstance);
}
void YoshiTargetMark::control() {
    if (MR::isPlayerInRushOrDemoOrTalk()) {
        if (!MR::isHiddenLayout(this)) MR::hideLayout(this);
    } else {
        if (MR::isHiddenLayout(this)) MR::showLayout(this);
    }
    TVec2f position;
    MR::calcScreenPosition(&position, mTargetPosition);
    setTrans(position);
}
YoshiTargetMark::~YoshiTargetMark() {}
void NrvYoshiTargetMark::YoshiTargetMarkNrvWait::execute(Spine* spine) const {
    YoshiTargetMark* actor = static_cast<YoshiTargetMark*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startAnim(actor, "Wait", 0);
}
