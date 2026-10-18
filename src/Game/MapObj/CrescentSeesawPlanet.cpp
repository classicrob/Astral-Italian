#pragma auto_inline off
#include "Game/MapObj/CrescentSeesawPlanet.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <cmath>
extern const f32 lbl_807DD17C; // 45 degrees; negated at runtime by the target.
extern const f32 lbl_807DD198; // restitution 1.0, negated at runtime.
namespace NrvCrescentSeesawPlanet {
class CrescentSeesawPlanetNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static CrescentSeesawPlanetNrvWait sInstance;
};
CrescentSeesawPlanetNrvWait CrescentSeesawPlanetNrvWait::sInstance;
}
#pragma auto_inline on
CrescentSeesawPlanet::CrescentSeesawPlanet(const char* name) : LiveActor(name), mAxis(0.0f, 0.0f, 0.0f), mAngularVelocity(0.0f), mAngle(0.0f) {}
#pragma auto_inline off
void CrescentSeesawPlanet::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    MR::makeMtxTR(mInitialMtx, mPosition, mRotation);
    initNerve(&NrvCrescentSeesawPlanet::CrescentSeesawPlanetNrvWait::sInstance);
    mAxis.set<f32>(mInitialMtx.mMtx[0][2], mInitialMtx.mMtx[1][2], mInitialMtx.mMtx[2][2]);
    makeActorAppeared();
}
void CrescentSeesawPlanet::control() {
    move();
    mAngle += mAngularVelocity;
    if (mAngle > lbl_807DD17C || mAngle < -lbl_807DD17C) {
        f32 clamped;
        if (mAngle < -lbl_807DD17C) clamped = -lbl_807DD17C;
        else if (mAngle > lbl_807DD17C) clamped = lbl_807DD17C;
        else clamped = mAngle;
        mAngle = clamped;
        mAngularVelocity *= -lbl_807DD198;
    }
    f32 speed = __fabsf(mAngularVelocity);
    if (speed >= 0.1f) MR::startActionSound(this, "Move", static_cast<s32>(1000.0f * speed));
}
void CrescentSeesawPlanet::calcAndSetBaseMtx() {
    TPos3f rotation;
    rotation.identity();
    TPos3f initial;
    initial.set(mInitialMtx);
    TVec3f angles = mAxis * mAngle;
    MR::makeMtxRotate(rotation, angles);
    MR::fn_80046A70(getBaseMtx(), rotation, initial);
}
// NONMATCHING: scalar register allocation and component-load scheduling differ.
void CrescentSeesawPlanet::move() {
    f32 speed = mAngularVelocity * 0.999f;
    mAngularVelocity = speed + (mAngle * -0.0006f + speed * -0.012f);
    if (MR::isOnPlayer(this)) {
        TVec3f side(0.0f), toPlayer(0.0f);
        side.set<f32>(mInitialMtx.mMtx[0][0], mInitialMtx.mMtx[1][0], mInitialMtx.mMtx[2][0]);
        MR::calcVecToPlayer(&toPlayer, this);
        f32 force = 40.0f * ((side.dot(toPlayer) * 0.002f) / 10000.0f);
        if (MR::isPlayerHipDropLand()) force *= 80.0f;
        mAngularVelocity -= force;
    }
}
CrescentSeesawPlanet::~CrescentSeesawPlanet() {}
