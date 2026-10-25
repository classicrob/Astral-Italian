#include "Game/MapObj/BendSwanpPlanet.hpp"
#include "Game/LiveActor/CollisionObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <cstdio>
namespace { extern char sBendWaitC[]; }
namespace NrvBendSwanpPlanet {
NEW_NERVE(BendSwanpPlanetNrvWaitA, BendSwanpPlanet, WaitA);
NEW_NERVE(BendSwanpPlanetNrvMoveA, BendSwanpPlanet, MoveA);
NEW_NERVE(BendSwanpPlanetNrvWaitB, BendSwanpPlanet, WaitB);
NEW_NERVE(BendSwanpPlanetNrvMoveB, BendSwanpPlanet, MoveB);
class BendSwanpPlanetNrvWaitC : public Nerve {
public:
    virtual void execute(Spine* spine) const { LiveActor* actor = static_cast<LiveActor*>(spine->mExecutor); if (MR::isFirstStep(actor)) MR::startAction(actor, sBendWaitC); }
    static BendSwanpPlanetNrvWaitC sInstance;
};
BendSwanpPlanetNrvWaitC BendSwanpPlanetNrvWaitC::sInstance;
}
namespace {
const char* sParts[] = {"BendSwanpPlanetPartsA", "BendSwanpPlanetPartsB", "BendSwanpPlanetPartsC"};
}
BendSwanpPlanet::BendSwanpPlanet(const char* name) : LiveActor(name), mParts(0) {}
void BendSwanpPlanet::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    f32 radius = 0.0f;
    MR::calcModelBoundingRadius(&radius, this);
    MR::setClippingTypeSphere(this, radius);
    MR::setClippingFarMax(this);
    mParts = new CollisionObj*[3];
    for (s32 i = 0; i < 3; ++i) {
        char name[256];
        snprintf(name, sizeof(name), "Collision[%s]", sParts[i]);
        mParts[i] = MR::createCollisionObj(this, sParts[i], getSensor(0), MR::getJointMtx(this, sParts[i]));
        MR::validateCollisionParts(mParts[i]);
    }
    MR::initUseStageSwitchReadA(this, iter);
    MR::initUseStageSwitchReadB(this, iter);
    initNerve(&NrvBendSwanpPlanet::BendSwanpPlanetNrvWaitA::sInstance);
    makeActorAppeared();
}
namespace {
    char sBendWaitA[] = "WaitA";
    char sBendMoveA[] = "MoveA";
    char sBendWaitB[] = "WaitB";
    char sBendMoveB[] = "MoveB";
    char sBendWaitC[] = "WaitC";
}
void BendSwanpPlanet::exeWaitA() { if (MR::isFirstStep(this)) MR::startAction(this, sBendWaitA); if (MR::isOnSwitchA(this)) setNerve(&NrvBendSwanpPlanet::BendSwanpPlanetNrvMoveA::sInstance); }
void BendSwanpPlanet::exeMoveA() { if (MR::isFirstStep(this)) MR::startAction(this, sBendMoveA); if (MR::isActionEnd(this)) setNerve(&NrvBendSwanpPlanet::BendSwanpPlanetNrvWaitB::sInstance); }
void BendSwanpPlanet::exeWaitB() { if (MR::isFirstStep(this)) MR::startAction(this, sBendWaitB); if (MR::isOnSwitchB(this)) setNerve(&NrvBendSwanpPlanet::BendSwanpPlanetNrvMoveB::sInstance); }
void BendSwanpPlanet::exeMoveB() { if (MR::isFirstStep(this)) MR::startAction(this, sBendMoveB); if (MR::isActionEnd(this)) setNerve(&NrvBendSwanpPlanet::BendSwanpPlanetNrvWaitC::sInstance); }
BendSwanpPlanet::~BendSwanpPlanet() {}
