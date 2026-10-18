#include "Game/Screen/WipeRing.hpp"
#include "Game/Screen/WorldMapArrow.hpp"
#include "Game/System/GalaxyWorldMapInfo.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvWipeRing {
    WipeRingNrvClose::WipeRingNrvClose() {}
    WipeRingNrvWipeIn::WipeRingNrvWipeIn() {}
    WipeRingNrvOpen::WipeRingNrvOpen() {}
    WipeRingNrvWipeOut::WipeRingNrvWipeOut() {}
    void WipeRingNrvWipeOut::execute(Spine* spine) const { static_cast<WipeRing*>(spine->mExecutor)->exeWipeOut(); }
    void WipeRingNrvOpen::execute(Spine* spine) const {
        WipeRing* actor = static_cast<WipeRing*>(spine->mExecutor);
        if (MR::isFirstStep(actor)) actor->kill();
    }
    void WipeRingNrvWipeIn::execute(Spine* spine) const { static_cast<WipeRing*>(spine->mExecutor)->exeWipeIn(); }
    void WipeRingNrvClose::execute(Spine*) const {}
}
namespace NrvWorldMapArrow {
    class WorldMapArrowNrvDelay : public Nerve { public: virtual void execute(Spine*) const; static WorldMapArrowNrvDelay sInstance; };
    class WorldMapArrowNrvAppear : public Nerve { public: virtual void execute(Spine*) const; static WorldMapArrowNrvAppear sInstance; };
    class WorldMapArrowNrvWait : public Nerve { public: virtual void execute(Spine*) const {} static WorldMapArrowNrvWait sInstance; };
    class WorldMapArrowNrvWaitOpen : public Nerve { public: virtual void execute(Spine*) const; static WorldMapArrowNrvWaitOpen sInstance; };
    class WorldMapArrowNrvDisappear : public Nerve { public: virtual void execute(Spine*) const; static WorldMapArrowNrvDisappear sInstance; };
    WorldMapArrowNrvDelay WorldMapArrowNrvDelay::sInstance;
    WorldMapArrowNrvAppear WorldMapArrowNrvAppear::sInstance;
    WorldMapArrowNrvWait WorldMapArrowNrvWait::sInstance;
    WorldMapArrowNrvWaitOpen WorldMapArrowNrvWaitOpen::sInstance;
    WorldMapArrowNrvDisappear WorldMapArrowNrvDisappear::sInstance;
}
namespace { const TVec2f cArrowOffset(0.0f, -21.0f); }
using namespace NrvWorldMapArrow;
WorldMapArrow::WorldMapArrow() : SimpleLayout("ワールドマップ矢印", "MapGalaxyArrow", 1, -1), mAppearDelay(0), mTarget(0), mScreenPosition(0.0f, 0.0f) {
    MR::registerDemoSimpleCastAll(this);
    initNerve(&WorldMapArrowNrvWait::sInstance);
    MR::setFollowPos(&mScreenPosition, this, 0);
}
void WorldMapArrow::control() {
    TVec2f position;
    MR::calcScreenPosition(&position, mTarget->mPosition);
    mScreenPosition = position + cArrowOffset;
}
void WorldMapArrow::start(GalaxyWorldMapInfo* target, s32 delay) {
    mAppearDelay = delay;
    mTarget = target;
    LayoutActor::appear();
    MR::hideLayout(this);
    setNerve(&WorldMapArrowNrvDelay::sInstance);
}
void WorldMapArrow::disappear() { if (!MR::isDead(this)) setNerve(&WorldMapArrowNrvWaitOpen::sInstance); }
void WorldMapArrow::exeDelay() {
    if (MR::isGreaterEqualStep(this, mAppearDelay)) {
        MR::startAnim(this, "Appear", 0);
        MR::showLayout(this);
        setNerve(&WorldMapArrowNrvAppear::sInstance);
    }
}
void WorldMapArrow::exeDisappear() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "End", 0);
    if (MR::isAnimStopped(this, 0)) kill();
}
WorldMapArrow::~WorldMapArrow() {}
namespace NrvWorldMapArrow {
    void WorldMapArrowNrvDisappear::execute(Spine* spine) const { static_cast<WorldMapArrow*>(spine->mExecutor)->exeDisappear(); }
    void WorldMapArrowNrvWaitOpen::execute(Spine* spine) const {
        WorldMapArrow* actor = static_cast<WorldMapArrow*>(spine->mExecutor);
        if (actor->mTarget->isNerveOpened()) actor->setNerve(&WorldMapArrowNrvDisappear::sInstance);
    }
    void WorldMapArrowNrvAppear::execute(Spine* spine) const {
        WorldMapArrow* actor = static_cast<WorldMapArrow*>(spine->mExecutor);
        if (MR::isAnimStopped(actor, 0)) {
            MR::startAnim(actor, "Wait", 0);
            actor->setNerve(&WorldMapArrowNrvWait::sInstance);
        }
    }
    void WorldMapArrowNrvDelay::execute(Spine* spine) const { static_cast<WorldMapArrow*>(spine->mExecutor)->exeDelay(); }
}
