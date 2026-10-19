#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Enemy/RingBeam.hpp"
#include "Game/Map/HitInfo.hpp"
#include <cmath>

namespace NrvKoopaJrRobotStateAttack {
    class KoopaJrRobotStateAttackNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotStateAttackNrvWait sInstance;
    };
    KoopaJrRobotStateAttackNrvWait KoopaJrRobotStateAttackNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KoopaJrRobotStateAttackNrvUnused, KoopaJrRobotStateAttack, AttackA);
    NEW_NERVE(KoopaJrRobotStateAttackNrvAttackSearch, KoopaJrRobotStateAttack, AttackSearch);
    NEW_NERVE(KoopaJrRobotStateAttackNrvAttackB, KoopaJrRobotStateAttack, AttackA);
    NEW_NERVE(KoopaJrRobotStateAttackNrvAttackC, KoopaJrRobotStateAttack, AttackA);
};  // namespace NrvKoopaJrRobotStateAttack

static TVec3f sOffsetL(594.0f, 0.0f, -760.0f);
static TVec3f sOffsetR(594.0f, 0.0f, 760.0f);

// ---- attack state ----
KoopaJrRobotStateAttack::KoopaJrRobotStateAttack(KoopaJrRobot* pHost, s32 type) : KoopaJrRobotStateBase("State:\x8dU\x8c\x82", pHost), mType(type), mMode(0) {
    initNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvWait::sInstance, 0);
}

void KoopaJrRobotStateAttack::updateMode() {
    switch (mType) {
    case 0:
        mMode = 0;
        return;
    case 1:
        mMode = 1;
        return;
    case 2:
        mMode = 2;
        return;
    }
}

void KoopaJrRobotStateAttack::appear() {
    mIsDead = false;
    updateMode();
    switch (mMode) {
    case 0:
        setNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackSearch::sInstance);
        return;
    case 1:
        setNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackB::sInstance);
        return;
    case 2:
        setNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackC::sInstance);
        return;
    }
}

void KoopaJrRobotStateAttack::kill() {
    if (mMode != 2) {
        if (!MR::isDead(mHost->mCollisionA)) {
            mHost->mCollisionA->kill();
        }
        if (!MR::isDead(mHost->mCollisionB)) {
            mHost->mCollisionB->kill();
        }
    }
    MR::offSwitchA(mHost);
    mIsDead = true;
}

bool KoopaJrRobotStateAttack::tryAppearFire() {
    if (isNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackB::sInstance)) {
        if (MR::isStep(this, 138)) {
            mHost->mCollisionA->appear();
            mHost->mCollisionB->appear();
            return true;
        }
        return false;
    }
    if (isNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackC::sInstance)) {
        if (MR::isStep(this, 112) || MR::isStep(this, 241)) {
            if (!MR::isDead(mHost->mCollisionA)) {
                mHost->mCollisionA->kill();
            }
            if (!MR::isDead(mHost->mCollisionB)) {
                mHost->mCollisionB->kill();
            }
            mHost->mCollisionA->appear();
            mHost->mCollisionB->appear();
            return true;
        }
        return false;
    }
    return false;
}

bool KoopaJrRobotStateAttack::tryTurnToPlayer() {
    if (KoopaJrRobotFunction::isPlayerInRushAir()) {
        return false;
    }
    if (isPlayerNearFront()) {
        return false;
    }
    turnToPlayerSlow();
    return true;
}

void KoopaJrRobotStateAttack::exeAttackSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "AttackA");
        mHost->mSeq->stopLauncherSequence();
    }
    if (MR::isStep(this, 50)) {
        mHost->mSeq->startMagnumShot();
    }
    tryTurnToPlayer();
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

// NONMATCHING: shared matrix-copy helper expansion and vector temporary stack/register allocation
bool KoopaJrRobot::tryAttackMapSensorsAroundJoint(const char* pJoint, const TVec3f& rOffset) {
    f32 range = 600.0f;
    MtxPtr pJointMtx = MR::getJointMtx(this, pJoint);
    HitSensor* pBody = getSensor("Body");
    TVec3f negGravity = -MR::getSensorHost(pBody)->mGravity;
    TVec3f xDir;
    MR::extractMtxXDir(pJointMtx, &xDir);
    TPos3f mtx;
    mtx.set(pJointMtx);
    MR::addTransMtxLocal(mtx.toMtxPtr(), rOffset);
    TVec3f origin;
    MR::extractMtxTrans(mtx.toMtxPtr(), &origin);
    TVec3f gravity = -negGravity;
    s32 num = 12;
    for (s32 i = 0; i < num; i++) {
        TVec3f v;
        MR::vecKillElement(xDir, negGravity, &v);
        MR::normalize(&v);
        MR::rotateVecDegree(&v, negGravity, (360.0f / num) * i);
        TVec3f scaled = v * range;
        TVec3f start(origin);
        start += scaled;
        TVec3f copyStart;
        copyStart.x = start.x;
        copyStart.y = start.y;
        copyStart.z = start.z;
        TVec3f dir = gravity * range;
        Triangle tri;
        TVec3f hit;
        HitSensor* pHit = MR::getFirstPolyOnLineToMapExceptSensor(&hit, &tri, copyStart, dir, pBody) ? tri.getSensor() : nullptr;
        if (pHit != nullptr && MR::sendMsgEnemyAttack(pHit, pBody)) {
            return true;
        }
    }
    return false;
}

void KoopaJrRobotStateAttack::exeAttackA() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, isNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackB::sInstance) ? "AttackB" : "AttackC");
        mHost->mSeq->stopLauncherSequence();
        MR::onSwitchA(mHost);
    }
    if (MR::isStep(this, 140)) {
        mHost->tryAttackMapSensorsAroundJoint("HandL", sOffsetL);
        mHost->tryAttackMapSensorsAroundJoint("HandR", sOffsetR);
    }
    tryAppearFire();
    if (MR::isLessStep(this, 60)) {
        tryTurnToPlayer();
    }
    if ((isNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackB::sInstance) && MR::isGreaterStep(this, 198)) ||
        (isNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackC::sInstance) && MR::isGreaterStep(this, 172))) {
        if (isPlayerNearFront()) {
            kill();
            return;
        }
    }
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

KoopaJrRobotStateAttack::~KoopaJrRobotStateAttack() {}
