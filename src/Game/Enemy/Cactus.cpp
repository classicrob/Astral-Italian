#include "Game/Util.hpp"
#include "Game/Enemy/Cactus.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvCactus {
    // Wait nerve: its execute is an empty function in the target (bare blr).
    class CactusNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static CactusNrvWait sInstance;
    };
    CactusNrvWait CactusNrvWait::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvCactus

Cactus::Cactus(const char* pName) : LiveActor(pName) {
}

void Cactus::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(&NrvCactus::CactusNrvWait::sInstance, 0);
    makeActorAppeared();
}

void Cactus::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body") && isNerveWait()) {
        MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
}

#pragma dont_inline on
bool Cactus::isNerveWait() const {
    return isNerve(&NrvCactus::CactusNrvWait::sInstance);
}
#pragma dont_inline off
