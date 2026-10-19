#include "Game/System/GrandGalaxyMapStates.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapStateEnterDemo {
    NEW_NERVE(GrandGalaxyMapStateEnterDemoStart, GrandGalaxyMapStateEnterDemo, Start);
    NEW_NERVE(GrandGalaxyMapStateEnterDemoZoom, GrandGalaxyMapStateEnterDemo, Zoom);
};  // namespace NrvGrandGalaxyMapStateEnterDemo

using namespace NrvGrandGalaxyMapStateEnterDemo;

GrandGalaxyMapStateEnterDemo::GrandGalaxyMapStateEnterDemo(GrandGalaxyMap* pHost) : ActorStateBase< GrandGalaxyMap >("State[開始デモ]", pHost) {}

void GrandGalaxyMapStateEnterDemo::init() {
    initNerve(&GrandGalaxyMapStateEnterDemoZoom::sInstance, 0);
    mIsDead = true;
}

void GrandGalaxyMapStateEnterDemo::appear() {
    mIsDead = false;
    mHost->changeRocketActionBck("Wait");
    mHost->startAreaCamera(-1, 0);
    MR::openSystemWipeWhiteFade(-1);
    setNerve(&GrandGalaxyMapStateEnterDemoStart::sInstance);
}

void GrandGalaxyMapStateEnterDemo::kill() {
    mHost->startAreaCamera(-1, -1);
    mIsDead = true;
}

void GrandGalaxyMapStateEnterDemo::exeZoom() {
    if (MR::isFirstStep(this)) {
        mFovy = MR::getFovy();
    }

    s32 frames = 20;
    MR::setFovy(MR::getEaseOutValue((f32)getNerveStep() / (f32)frames, 0.01f, mFovy, 1.0f));
    if (MR::isStep(this, 20)) {
        kill();
    }
}

GrandGalaxyMapStateEnterDemo::~GrandGalaxyMapStateEnterDemo() {}

inline void GrandGalaxyMapStateEnterDemo::exeStart() {
    mFovy = MR::getFovy();
    setNerve(&GrandGalaxyMapStateEnterDemoZoom::sInstance);
}
