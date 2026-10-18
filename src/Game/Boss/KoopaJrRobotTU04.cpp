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

// ---- Pod ("ポッド") ----
// NONMATCHING: euler-angle copy/scale sequence differs (TVec3 temp handling)
KoopaJrRobotPod::KoopaJrRobotPod(LiveActor* pHost, MtxPtr pMtx) : LiveActor("\x83|\x83""b\x83h"), mHost(pHost), mMtx(pMtx), mCrack(nullptr) {
    TPos3f mtx;
    mtx.set(pMtx);
    mtx.getTrans(mPosition);
    TVec3f euler;
    mtx.getEuler(euler);
    mRotation = euler * 57.295776f;
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

// ---- Beam ("ビーム") ----
// NONMATCHING: float param save order / RingBeam is 0xF0 in SMG2 (our RingBeam.hpp is smaller), init() allocs 0xDC
KoopaJrRobotBeam::KoopaJrRobotBeam(const char* pName, LiveActor* pHost, MtxPtr pMtx, const TVec3f& rOffset, s32 life, f32 speed, f32 a, f32 b)
    : LiveActor(pName), mHost(pHost), mRingBeam(nullptr), mMtx(pMtx), mSpeed(speed), mLife(life), _E0(a), _E4(b) {
    mOffset = rOffset;
    mBaseMtx.set(pMtx);
}

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

// ---- phase state base ----
KoopaJrRobotStateBase::KoopaJrRobotStateBase(const char* pName, KoopaJrRobot* pHost) : ActorStateBase< KoopaJrRobot >(pName, pHost), _10(0.0f), _14(0.0f, 0.0f, 1.0f) {}

void KoopaJrRobotStateBase::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if ((!mHost->fn_800DB9D0(pSender) || !MR::isSensorType(pReceiver, 0x2D) || !MR::sendMsgEnemyAttackExplosion(pReceiver, pSender)) &&
        MR::isSensorPlayerOrYoshiOrRide(pReceiver) && !MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool KoopaJrRobotStateBase::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    for (s32 i = 0; i < 4; i++) {
        if (mHost->mParts[i]->receiveMsgPlayerAttack(msg, pSender, pReceiver)) {
            return true;
        }
    }
    return false;
}

void KoopaJrRobotStateBase::turnToPlayerFast() {
    MR::turnDirectionToPlayerDegreeHorizon(mHost, &mHost->mZAxis, 180.0f);
}

void KoopaJrRobotStateBase::turnToPlayerSlow() {
    MR::turnDirectionToPlayerDegreeHorizon(mHost, &mHost->mZAxis, 2.0f);
}

s32 KoopaJrRobotStateBase::updateSearch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, "Search");
        mHost->mSeq->fn_800DC9D0();
    }
    if (MR::isStep(this, 110)) {
        mHost->mCollisionC->appear();
    }
    if (MR::isActionEnd(mHost)) {
        return calcTurnDir();
    }
    return 0;
}

// NONMATCHING: target inlines -gravity (ps_neg) and materializes the sign compare via mfcr; ours calls operator-
s32 KoopaJrRobotStateBase::calcTurnDir() {
    MR::calcVecToPlayerH(&_14, mHost, nullptr);
    _10 = 57.295776f * MR::diffAngleSigned(_14, mHost->mZAxis, -mHost->mGravity);
    bool neg = _10 < 0.0f;
    return neg ? 1 : 2;
}

s32 KoopaJrRobotStateBase::updateTurn(s32 dir) {
    if (dir != 1 && dir != 2) {
        return 0;
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, dir == 1 ? "TurnL" : "TurnR");
        return 0;
    }
    s32 r = 0;
    // MR::turnDirectionToPlayerDegreeHorizon returns bool in SMG2 (core header still says void)
    if (reinterpret_cast< bool (*)(const LiveActor*, TVec3f*, f32) >(&MR::turnDirectionToPlayerDegreeHorizon)(mHost, &mHost->mZAxis, 2.0f)) {
        r = dir;
    }
    return r;
}

bool KoopaJrRobotStateBase::updateTurnEnd(s32 dir) {
    if (dir != 1 && dir != 2) {
        return false;
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, dir == 1 ? "TurnLEnd" : "TurnREnd");
    }
    return MR::isActionEnd(mHost);
}

bool KoopaJrRobotStateBase::isPlayerNearFront() {
    return mHost->fn_800DBB30(45.0f);
}

const char* KoopaJrRobotStateBase::getWaitActionName() {
    return "Wait";
}

KoopaJrRobotStateBase::~KoopaJrRobotStateBase() {}

// ---- attack state ----
KoopaJrRobotStateAttack::KoopaJrRobotStateAttack(KoopaJrRobot* pHost, s32 type) : KoopaJrRobotPhase("State:\x8dU\x8c\x82", pHost), mType(type), mMode(0) {
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

bool KoopaJrRobotStateAttack::fn_800DEA50() {
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
        mHost->mSeq->fn_800DC9F0();
    }
    if (MR::isStep(this, 50)) {
        mHost->mSeq->fn_800DC870();
    }
    fn_800DEA50();
    if (MR::isActionEnd(mHost)) {
        kill();
    }
}

// NONMATCHING: -gravity / TVec3 temporaries (operator- inline vs call), stack layout
bool KoopaJrRobot::fn_800DEB50(const char* pJoint, const TVec3f& rOffset) {
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
    for (s32 i = 0; i < 12; i++) {
        TVec3f v;
        MR::vecKillElement(xDir, negGravity, &v);
        MR::normalize(&v);
        MR::rotateVecDegree(&v, negGravity, (360.0f / 12.0f) * i);
        TVec3f scaled = v * range;
        TVec3f start(origin);
        start += scaled;
        TVec3f dir = gravity * range;
        Triangle tri;
        TVec3f hit;
        HitSensor* pHit = MR::getFirstPolyOnLineToMapExceptSensor(&hit, &tri, start, dir, pBody) ? tri.getSensor() : nullptr;
        if (pHit != nullptr && MR::sendMsgEnemyAttack(pHit, pBody)) {
            return true;
        }
    }
    return false;
}

// NONMATCHING: string literal pool base offsets only (target pools the file strings)
void KoopaJrRobotStateAttack::exeAttackA() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mHost, isNerve(&NrvKoopaJrRobotStateAttack::KoopaJrRobotStateAttackNrvAttackB::sInstance) ? "AttackB" : "AttackC");
        mHost->mSeq->fn_800DC9F0();
        MR::onSwitchA(mHost);
    }
    if (MR::isStep(this, 140)) {
        mHost->fn_800DEB50("HandL", sOffsetL);
        mHost->fn_800DEB50("HandR", sOffsetR);
    }
    tryAppearFire();
    if (MR::isLessStep(this, 60)) {
        fn_800DEA50();
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
