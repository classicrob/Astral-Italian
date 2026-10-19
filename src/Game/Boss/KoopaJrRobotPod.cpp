#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/PartsModel.hpp"

// ---- Pod ("ポッド") ----
// NONMATCHING: constant load scheduled after the Euler z-component load
KoopaJrRobotPod::KoopaJrRobotPod(LiveActor* pHost, MtxPtr pMtx) : LiveActor("\x83|\x83""b\x83h"), mHost(pHost), mMtx(pMtx), mCrack(nullptr) {
    TPos3f mtx;
    mtx.set(pMtx);
    mtx.getTrans(mPosition);
    TVec3f euler;
    mtx.getEuler(euler);
    TVec3f scaled;
    JMath::gekko_ps_copy3(&scaled, &euler);
    scaled.x *= 57.29578f;
    scaled.y *= 57.29578f;
    scaled.z *= 57.29578f;
    mRotation.z = scaled.z;
    mRotation.x = scaled.x;
    mRotation.y = scaled.y;
}

void KoopaJrRobotPod::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "KoopaJrRobotPod", false);
    MR::invalidateClipping(this);
    mCrack = MR::createPartsModelEnemy(this, "\x82\xd0\x82\xd1\x8a\x84\x82\xea\x83\x82\x83""f\x83\x8b", "KoopaJrRobotPodCrack", MR::getJointMtx(this, "Base"));
    mCrack->kill();
    makeActorDead();
}

void KoopaJrRobotPod::appear() {
    LiveActor::appear();
    MR::startAction(this, "Damage0");
}

void KoopaJrRobotPod::setMtx(MtxPtr pMtx) {
    mMtx = pMtx;
}

void KoopaJrRobotPod::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.set(mMtx);
    mtx.getTrans(mPosition);
    MR::setBaseTRMtx(this, mMtx);
}

KoopaJrRobotPod::~KoopaJrRobotPod() {}

