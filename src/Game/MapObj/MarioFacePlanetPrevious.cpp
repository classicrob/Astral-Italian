#pragma auto_inline off
#include "Game/MapObj/MarioFacePlanetPrevious.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Util.hpp"
#include <cstdio>
namespace {
const char* sScreenEffect = "MetamorphoseScreenEffect";
const char* sScreenLightEffect = "MetamorphoseScreenEffectLight";
const char* sChangeDemo = "マリオ顔惑星変化";
const char* sStartDemo = "スター１個目直後のゲームスタート";
}
namespace NrvMarioFacePlanetPrevious {
class MarioFacePlanetPreviousNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static MarioFacePlanetPreviousNrvWait sInstance;
};
MarioFacePlanetPreviousNrvWait MarioFacePlanetPreviousNrvWait::sInstance;
NEW_NERVE(MarioFacePlanetPreviousNrvDemo, MarioFacePlanetPrevious, Demo);
}
MarioFacePlanetPrevious::MarioFacePlanetPrevious(const char* name) : LiveActor(name) { mScreenMtx.identity(); mScreenLightMtx.identity(); }
void MarioFacePlanetPrevious::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvMarioFacePlanetPrevious::MarioFacePlanetPreviousNrvWait::sInstance);
    MR::useStageSwitchWriteAutoDead(this, iter);
    MR::useStageSwitchAwake(this, iter);
    MR::initUseStageSwitchWriteA(this, iter);
    makeActorAppeared();
    MR::tryRegisterDemoCast(this, sStartDemo, iter);
    if (MR::tryRegisterDemoCast(this, sChangeDemo, iter)) MR::tryRegisterDemoActionNerveDirect(this, &NrvMarioFacePlanetPrevious::MarioFacePlanetPreviousNrvDemo::sInstance, sChangeDemo, 0);
}
void MarioFacePlanetPrevious::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::onSwitchA(this);
    char name[64];
    for (s32 i=0; i<7; ++i) { snprintf(name, sizeof(name), "MarioFacePlanetPrevious%02d", i); MR::emitEffect(this, name); }
}
void MarioFacePlanetPrevious::makeActorDead() {
    LiveActor::makeActorDead();
    MR::offSwitchA(this);
    char name[64];
    for (s32 i=0; i<7; ++i) { snprintf(name, sizeof(name), "MarioFacePlanetPrevious%02d", i); MR::forceDeleteEffect(this, name); }
}
// NONMATCHING: temporary vector stack slots and instruction scheduling differ.
void MarioFacePlanetPrevious::exeDemo() {
    if (MR::isFirstStep(this)) {
        TVec3f direction = mPosition - MR::getCamPos();
        MR::normalizeOrZero(&direction);
        TVec3f offset(direction);
        offset.x *= 500.0f; offset.y *= 500.0f; offset.z *= 500.0f;
        TVec3f effectPos = MR::getCamPos() + offset;
        MR::makeMtxFrontUpPos(&mScreenMtx, -MR::getCamZdir(), MR::getCamYdir(), effectPos);
        MR::makeMtxFrontUpPos(&mScreenLightMtx, -MR::getCamZdir(), MR::getCamYdir(), mPosition);
        MR::emitEffect(this, sScreenEffect)->setHostMtx(mScreenMtx.toMtxPtr());
        MR::emitEffect(this, sScreenLightEffect)->setHostMtx(mScreenLightMtx.toMtxPtr());
        MR::shakeCameraInfinity(this, 0.2f, 2.0f);
    }
    if (MR::isStep(this, 75)) MR::stopShakingCamera(this);
    if (MR::isStep(this, 76)) MR::shakeCameraVeryStrong();
    if (MR::isGreaterEqualStep(this, 120)) { MR::deleteEffect(this, sScreenEffect); MR::deleteEffect(this, sScreenLightEffect); setNerve(&NrvMarioFacePlanetPrevious::MarioFacePlanetPreviousNrvWait::sInstance); }
}
MarioFacePlanetPrevious::~MarioFacePlanetPrevious() {}
