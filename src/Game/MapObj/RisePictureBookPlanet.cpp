#pragma auto_inline off
#include "Game/MapObj/RisePictureBookPlanet.hpp"
#include "Game/MapObj/PhantomTorch.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util.hpp"
namespace {
const char* sCollisionJoints[9] = {"RisePictureBookPrismA", "RisePictureBookPrismATop", "RisePictureBookPrismB", "RisePictureBookPrismBTop", "RisePictureBookPrismC", "RisePictureBookPrismCTop", "RisePictureBookPrismD", "RisePictureBookPrismDTop", "RisePictureBookWallParts"};
const char* sFireJoints[2] = {"Fire00", "Fire01"};
}
namespace NrvRisePictureBookPlanet {
NEW_NERVE(RisePictureBookPlanetNrvWait, RisePictureBookPlanet, Wait);
NEW_NERVE(RisePictureBookPlanetNrvMove, RisePictureBookPlanet, Move);
NEW_NERVE(RisePictureBookPlanetNrvReverse, RisePictureBookPlanet, Reverse);
}
RisePictureBookPlanet::RisePictureBookPlanet(const char* name) : LiveActor(name), mParts(0), mTorches(0) {}
// NONMATCHING: nonvolatile registers differ in collision-part initialization.
void RisePictureBookPlanet::init(const JMapInfoIter& iter) {
    MtxPtr mtx;
    MR::initActor(this, iter, false);
    f32 radius = 0.0f;
    MR::calcModelBoundingRadius(&radius, this);
    MR::setClippingTypeSphere(this, radius);
    MR::setClippingFarMax(this);
    mParts = new CollisionParts*[9];
    for (s32 i=0; i<9; ++i) {
        mtx = MR::getJointMtx(this, sCollisionJoints[i]);
        mParts[i] = MR::createCollisionPartsFromLiveActor(this, sCollisionJoints[i], getSensor("Body"), mtx, MR::CollisionScaleType_Default);
        MR::validateCollisionParts(mParts[i]);
    }
    mTorches = new PhantomTorch*[2];
    for (s32 i=0; i<2; ++i) {
        mTorches[i] = new PhantomTorch("絵本用たいまつ");
        mTorches[i]->initFromRisePictureBook();
        MR::setAllEffectHostMtx(mTorches[i], MR::getJointMtx(this, sFireJoints[i]));
    }
    calcFirePos();
    MR::initUseStageSwitchReadA(this, iter);
    initNerve(&NrvRisePictureBookPlanet::RisePictureBookPlanetNrvWait::sInstance);
    makeActorAppeared();
}
void RisePictureBookPlanet::calcAnim() {
    LiveActor::calcAnim();
    for (u32 i=0; i<9; ++i) mParts[i]->setMtx();
    calcFirePos();
}
void RisePictureBookPlanet::exeWait() {
    if (MR::isOnSwitchA(this)) setNerve(&NrvRisePictureBookPlanet::RisePictureBookPlanetNrvMove::sInstance);
    if (MR::isOnSwitchA(this)) setNerve(&NrvRisePictureBookPlanet::RisePictureBookPlanetNrvMove::sInstance);
}
void RisePictureBookPlanet::exeMove() { if (MR::isFirstStep(this)) MR::startAction(this, "Move"); if (!MR::isOnSwitchA(this)) setNerve(&NrvRisePictureBookPlanet::RisePictureBookPlanetNrvReverse::sInstance); }
void RisePictureBookPlanet::exeReverse() { if (MR::isFirstStep(this)) MR::startAction(this, "Reverse"); if (MR::isOnSwitchA(this)) setNerve(&NrvRisePictureBookPlanet::RisePictureBookPlanetNrvMove::sInstance); }
// NONMATCHING: count 2 is folded into cmpwi; target keeps it in a register.
void RisePictureBookPlanet::calcFirePos() {
    s32 count = 2;
    for (s32 i=0; i<count; ++i) {
        TPos3f mtx;
        mtx.set(MR::getJointMtx(this, sFireJoints[i]));
        TVec3f pos(0.0f);
        TVec3f rotation(0.0f);
        mtx.getTrans(pos);
        MR::setPosition(mTorches[i], pos);
    }
}
RisePictureBookPlanet::~RisePictureBookPlanet() {}
