#pragma auto_inline off
#include "Game/MapObj/RockReflector.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
extern const f32 lbl_807DE3B8;
extern const f32 lbl_807DE3BC;
extern const f32 lbl_807E30D0; // Target's read-only zero in .sbss2.
namespace NrvRockReflector {
class RockReflectorNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static RockReflectorNrvWait sInstance;
};
RockReflectorNrvWait RockReflectorNrvWait::sInstance;
NEW_NERVE(RockReflectorNrvReflect, RockReflector, Reflect);
}
RockReflector::RockReflector(const char* name) : LiveActor(name) {}
void RockReflector::init(const JMapInfoIter& iter) { MR::initActor(this, iter, "RockReflector", false); initNerve(&NrvRockReflector::RockReflectorNrvWait::sInstance); makeActorAppeared(); }
bool RockReflector::isNerveWait() const { return isNerve(&NrvRockReflector::RockReflectorNrvWait::sInstance); }
bool RockReflector::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor*) {
    if (MR::isMsgReflect(msg)) {
        if (isNerveWait()) {
            if (MR::reboundVelocityFromCollision(MR::getSensorHost(sender), lbl_807DE3B8, lbl_807E30D0, lbl_807DE3BC)) {
                MR::attenuateVelocity(MR::getSensorHost(sender), 0.98f);
                setNerve(&NrvRockReflector::RockReflectorNrvReflect::sInstance);
            }
        }
        return true;
    }
    return false;
}
void RockReflector::exeReflect() { if (MR::isFirstStep(this)) MR::startAction(this, "Reflect"); if (MR::isActionEnd(this)) setNerve(&NrvRockReflector::RockReflectorNrvWait::sInstance); }
RockReflector::~RockReflector() {}
