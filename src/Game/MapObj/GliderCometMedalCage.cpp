#include "Game/MapObj/GliderCometMedalCage.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvGliderCometMedalCage {
    NEW_NERVE(GliderCometMedalCageNrvWait,GliderCometMedalCage,Wait);
    NEW_NERVE(GliderCometMedalCageNrvOpen,GliderCometMedalCage,Open);
}
GliderCometMedalCage::GliderCometMedalCage(const char* name) : LiveActor(name),_90(0) {}
GliderCometMedalCage::~GliderCometMedalCage() {}
void GliderCometMedalCage::init(const JMapInfoIter& iter) {
    MR::initActor(this,iter,false);
    initNerve(&NrvGliderCometMedalCage::GliderCometMedalCageNrvWait::sInstance);
    makeActorAppeared();
}
void GliderCometMedalCage::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this,"Wait");
    if (MR::isOnSwitchB(this)) setNerve(&NrvGliderCometMedalCage::GliderCometMedalCageNrvOpen::sInstance);
}
void GliderCometMedalCage::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this,"Move");
        MR::tryHitReaction(this,"オープン");
        MR::invalidateCollisionParts(this);
    }
}
