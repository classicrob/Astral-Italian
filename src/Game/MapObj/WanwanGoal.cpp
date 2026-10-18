#include "Game/MapObj/WanwanGoal.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvWanwanGoal {
    class WanwanGoalNrvWait : public Nerve { public: void execute(Spine*) const {} static WanwanGoalNrvWait sInstance; };
    WanwanGoalNrvWait WanwanGoalNrvWait::sInstance ATTRIBUTE_WEAK;
}
WanwanGoal::WanwanGoal(const char* name) : LiveActor(name), mIsReached(false) {}
WanwanGoal::~WanwanGoal() {}
void WanwanGoal::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvWanwanGoal::WanwanGoalNrvWait::sInstance);
    MR::calcGravity(this);
    makeActorAppeared();
}
void WanwanGoal::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (!mIsReached) {
        f32 radius = MR::getSensorRadius(sender);
        if (MR::calcDistance(sender, receiver, nullptr) <= radius && MR::sendArbitraryMsg(0xAB, receiver, sender))
            mIsReached = true;
    }
}
