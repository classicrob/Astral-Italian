#include "Game/MapObj/GhostMazePlanetMask.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvGhostMazePlanetMask {
NEW_NERVE(GhostMazePlanetMaskNrvWait, GhostMazePlanetMask, Wait);
NEW_NERVE(GhostMazePlanetMaskNrvAppear, GhostMazePlanetMask, Appear);
NEW_NERVE(GhostMazePlanetMaskNrvDisappear, GhostMazePlanetMask, Disappear);
}
GhostMazePlanetMask::GhostMazePlanetMask(const char* name) : LiveActor(name) {}
void GhostMazePlanetMask::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvGhostMazePlanetMask::GhostMazePlanetMaskNrvWait::sInstance);
    makeActorAppeared();
    MR::initUseStageSwitchReadA(this, iter);
    MR::initUseStageSwitchReadB(this, iter);
}
void GhostMazePlanetMask::exeWait() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "Disappear"); MR::setAllAnimFrameAtEnd(this, "Disappear"); }
    if (MR::isOnSwitchA(this)) setNerve(&NrvGhostMazePlanetMask::GhostMazePlanetMaskNrvAppear::sInstance);
}
void GhostMazePlanetMask::exeAppear() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Appear");
    if (MR::isEventCameraActive()) setNerve(&NrvGhostMazePlanetMask::GhostMazePlanetMaskNrvDisappear::sInstance);
}
void GhostMazePlanetMask::exeDisappear() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Disappear");
    if (!MR::isEventCameraActive()) setNerve(&NrvGhostMazePlanetMask::GhostMazePlanetMaskNrvAppear::sInstance);
}
GhostMazePlanetMask::~GhostMazePlanetMask() {}
