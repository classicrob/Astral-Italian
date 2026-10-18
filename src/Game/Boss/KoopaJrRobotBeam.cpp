#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/RingBeam.hpp"

// ---- Beam ("ビーム") ----
// NONMATCHING: floating-point register assignment in the vector copy
KoopaJrRobotBeam::KoopaJrRobotBeam(const char* pName, LiveActor* pHost, MtxPtr pMtx, const TVec3f& rOffset, s32 life, f32 speed, f32 a, f32 b)
    : LiveActor(pName), mHost(pHost), mRingBeam(nullptr), mMtx(pMtx) {
    JMath::gekko_ps_copy3(&mOffset, &rOffset);
    mSpeed = speed;
    mLife = life;
    _E0 = a;
    _E4 = b;
    mBaseMtx.set(pMtx);
}

// NONMATCHING: target calls fn_801F6DB0 and fn_801F6DC0; pending RingBeam declarations from enemies lane
void KoopaJrRobotBeam::init(const JMapInfoIter& rIter) {
    MR::connectToSceneEnemyMovement(this);
    mRingBeam = new RingBeam("\x83r\x81[\x83\x80", this, false, false, false, false, "JrRobotBeam");
    mRingBeam->init(rIter);
    mRingBeam->setSpeed(mSpeed);
    mRingBeam->setLife(mLife);
    mRingBeam->setColor(_E0);
    mRingBeam->setRadius(_E4);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrRobotBeam::movement() {
    LiveActor::movement();
    updateMtx();
}

void KoopaJrRobotBeam::updateMtx() {
    mBaseMtx.set(mMtx);
    MR::addTransMtxLocal(mBaseMtx.toMtxPtr(), mOffset);
    MR::extractMtxTrans(mBaseMtx.toMtxPtr(), &mPosition);
}

void KoopaJrRobotBeam::appear() {
    LiveActor::appear();
    updateMtx();
    mRingBeam->appear();
    mRingBeam->initPos(this);
}

void KoopaJrRobotBeam::kill() {
    mRingBeam->kill();
    LiveActor::kill();
}

MtxPtr KoopaJrRobotBeam::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx).toMtxPtr();
}

KoopaJrRobotBeam::~KoopaJrRobotBeam() {}

