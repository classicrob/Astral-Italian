#include "Game/Screen/TimeUpLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvTimeUpLayout {
    NEW_NERVE(TimeUpLayoutNrvAppear, TimeUpLayout, Appear);
    class TimeUpLayoutNrvWait : public Nerve { public: virtual void execute(Spine*) const; static TimeUpLayoutNrvWait sInstance; };
    TimeUpLayoutNrvWait TimeUpLayoutNrvWait::sInstance;
    NEW_NERVE(TimeUpLayoutNrvEnd, TimeUpLayout, End);
}
using namespace NrvTimeUpLayout;

TimeUpLayout::TimeUpLayout() : LayoutActor("タイムアップレイアウト", true) {}
void TimeUpLayout::init(const JMapInfoIter&) {
    initLayoutManager("TimeUp", 1);
    initNerve(&TimeUpLayoutNrvAppear::sInstance);
    MR::connectToSceneLayout(this);
    MR::registerDemoSimpleCastAll(this);
    kill();
}
void TimeUpLayout::appear() { LayoutActor::appear(); setNerve(&TimeUpLayoutNrvAppear::sInstance); }
void TimeUpLayout::disappear() {
    if (!isNerve(&TimeUpLayoutNrvEnd::sInstance)) setNerve(&TimeUpLayoutNrvEnd::sInstance);
}
void TimeUpLayout::exeAppear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Appear", 0);
    if (MR::isAnimStopped(this, 0)) setNerve(&TimeUpLayoutNrvWait::sInstance);
}
void TimeUpLayout::exeEnd() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "End", 0);
    if (MR::isAnimStopped(this, 0)) kill();
}
TimeUpLayout::~TimeUpLayout() {}
void NrvTimeUpLayout::TimeUpLayoutNrvWait::execute(Spine* spine) const {
    TimeUpLayout* actor = static_cast<TimeUpLayout*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startAnim(actor, "Wait", 0);
}
