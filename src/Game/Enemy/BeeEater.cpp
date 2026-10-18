#include "Game/Enemy/BeeEater.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvBeeEater {
    NEW_NERVE(BeeEaterNrvWait, BeeEater, Wait);
    NEW_NERVE(BeeEaterNrvHit, BeeEater, Hit);
    NEW_NERVE(BeeEaterNrvBindStarPointer, BeeEater, BindStarPointer);
    NEW_NERVE(BeeEaterNrvFind, BeeEater, Find);
    NEW_NERVE(BeeEaterNrvAttack, BeeEater, Attack);
    NEW_NERVE(BeeEaterNrvAttackEnd, BeeEater, AttackEnd);
    NEW_NERVE(BeeEaterNrvAttackSuccess, BeeEater, AttackSuccess);
    NEW_NERVE(BeeEaterNrvAttackDash, BeeEater, AttackDash);
};  // namespace NrvBeeEater

#define GET_BEE_EATER_NERVE(name) (&NrvBeeEater::name::sInstance)

// Joint-controller delegate table name blob: five pointers (SMG2 lbl_80683230).
// PLACEHOLDER(enemies): unnamed data at this address (owner unit not named); resolve when the owner unit names it.
extern const char* lbl_80683230[];

// PLACEHOLDER(enemies): fn_8015BA20 is the joint-delegate ctor shared with Battan (ctor-shaped: r3=new mem, r4=host, r5=name); this call passes BeeEaterBody*; resolve when the delegate class is named and fn_8015BA20 gets one signature in its owner header.
JointController* fn_8015BA20(BeeEaterBody*, const char*);

BeeEaterBody::BeeEaterBody(BeeEater* pHost, const char* pName, const char* pJointName)
    : PartsModel(pHost, pName, pJointName, nullptr, -1, 0) {
    mJointCtrls = nullptr;
    mJointCount = 0;
}

void BeeEaterBody::init(const JMapInfoIter& rIter) {
    PartsModel::init(rIter);
    mJointCount = 5;
    mJointCtrls = new JointController*[5];
    for (s32 i = 0; i < mJointCount; i++) {
        JointController* pCtrl = nullptr;
        pCtrl = fn_8015BA20(this, lbl_80683230[i]);
        mJointCtrls[i] = pCtrl;
    }
}

void BeeEaterBody::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    for (s32 i = 0; i < mJointCount; i++) {
        mJointCtrls[i]->registerCallBack();
    }
}

BeeEater::BeeEater(const char* pName) : LiveActor(pName) {
    mRot.set(0.0f, 0.0f, 0.0f, 1.0f);
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mSideVec.set(0.0f, 0.0f, 0.0f);
    mUpVec.set(0.0f, 0.0f, 0.0f);
    mRangeLimit = 450.0f;
    mTargetVec.set(0.0f, 0.0f, 0.0f);
    mFaceVec.set(0.0f, 0.0f, 1.0f);
    mBody = nullptr;
    mAnimScaleCtrl = nullptr;
}

void BeeEater::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, nullptr, false);
    MR::makeQuatFromRotate(&mRot, this);
    mBasePos = mPosition;
    MR::calcSideVec(&mSideVec, this);
    MR::calcUpVec(&mUpVec, this);
    mTargetVec = mUpVec;
    mGravity.set(0.0f, 0.0f, 0.0f);

    mBody = new BeeEaterBody(this, "茎モデル", "BeeEaterBody");
    mBody->initWithoutIter();
    MR::setPosition(mBody, mBasePos);

    initNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mAnimScaleCtrl->setParamTight();
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_BEE_EATER_NERVE(BeeEaterNrvBindStarPointer), "SupportFreeze");
    makeActorAppeared();
}

void BeeEater::control() {
    mAnimScaleCtrl->updateNerve();
    if (!isNerve(GET_BEE_EATER_NERVE(BeeEaterNrvBindStarPointer)) && !isNerve(GET_BEE_EATER_NERVE(BeeEaterNrvHit)) &&
        mStarPointBind->tryStartPointBind()) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvBindStarPointer));
    }
}

// NONMATCHING: the SMG2 body builds the pose from the joint controllers (0x57c bytes); approximated here.
void BeeEater::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void BeeEater::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
        if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttackDash));
            return;
        }
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }
    MR::sendMsgPush(pReceiver, pSender);
}

bool BeeEater::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (!isNerve(GET_BEE_EATER_NERVE(BeeEaterNrvHit))) {
            MR::invalidateClipping(this);
            setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvHit));
        }
        mAnimScaleCtrl->startHitReaction();
        return true;
    }

    return false;
}

// Nerve exes ---------------------------------------------------------------------------------------------

// SMG2 fn_8015C9E0: Wait; after 60 steps the bee goes to Find.
void BeeEater::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Wait");
    }

    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvFind));
    }
}

// SMG2 fn_8015CC00: Hit reaction (HitEnd action); returns to Wait once it rebounds.
void BeeEater::exeHit() {
    if (moveAttackDash()) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttackSuccess));
        return;
    }

    if (MR::reboundVelocityFromCollision(this, 0.0f, 10.0f, 0.0f)) {
        MR::emitEffectHit(this, MR::getBindedHitPos(this), "HitSmoke");
        MR::tryHitReaction(this, "地形ヒット");
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

// SMG2 fn_8015C7B0: bind to the star pointer (turning toward the target and rebounding).
void BeeEater::exeBindStarPointer() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::tryStartActionNotPlaying(this, "Wait");
    }

    MR::fn_80010CB0(this, mUpVec, 1.0f);
    MR::attenuateVelocity(this, 0.85f);
    MR::turnQuatXDirRad(&mRot, mRot, mSideVec, 0.0349f);

    if (isPlayerNearFront()) {
        // NONMATCHING: the SMG2 face-target blend is simplified.
        TVec3f toTarget;
        MR::fn_8000E960(&mTargetVec, this, nullptr);
    }

    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

// SMG2 fn_8015CA50: Find; faces the player and attacks when in front.
void BeeEater::exeFind() {
    exeBindStarPointer();
    if (isFacingPlayer() && MR::isNearPlayer(this, 1000.0f)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttack));
    }
}

// SMG2 fn_8015CAD0: Attack (AttackSign); goes to AttackEnd after 120 steps.
void BeeEater::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
    }

    if (MR::isLessEqualStep(this, 80)) {
        if (isPlayerNearFront()) {
            MR::fn_8000E960(&mTargetVec, this, nullptr);
        }
        TVec3f push = mTargetVec;
        push.scale(2.0f);
        MR::addVelocity(this, push);
        MR::fn_80010CB0(this, mUpVec, 1.0f);
        MR::attenuateVelocity(this, 0.985f);
    } else {
        MR::zeroVelocity(this);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 0.0f);

    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttackEnd));
    }
}

// SMG2 fn_8015CCB0: AttackEnd (AttackEnd action); back to Wait after 40 steps.
void BeeEater::exeAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
    }

    if (MR::isLessStep(this, 25)) {
        TVec3f front = mFaceVec;
        front.scale(-1.0f);
        MR::attenuateVelocity(this, 0.97f);
    } else {
        TVec3f front = mFaceVec;
        front.scale(-2.5f);
        MR::addVelocity(this, front);
        MR::attenuateVelocity(this, 0.85f);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 0.0f);
    if (MR::isGreaterStep(this, 40)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

// SMG2 fn_8015CDC0: AttackSuccess action; returns to Wait when it ends.
void BeeEater::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }
    if (MR::isActionEnd(this)) {
        MR::zeroVelocity(this);
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

// SMG2 fn_8015C8C0: AttackDash body; the dash returns true once close or after 30 steps.
void BeeEater::exeAttackDash() {
    if (moveAttackDash()) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttackSuccess));
    }
}

// Helpers ---------------------------------------------------------------------------------------------

// SMG2 fn_8015C690: non-null when the player is not near enough; then the facing angle is checked.
bool BeeEater::isPlayerNearFront() const {
    if (MR::isNearPlayer(this, 120.0f)) {
        return false;
    }
    return true;
}

// SMG2 fn_8015C700: the player is in front of the bee (cross/dot sign test).
bool BeeEater::isFacingPlayer() const {
    TVec3f toPlayer;
    MR::calcVecToPlayer(&toPlayer, this);
    if (mUpVec.dot(toPlayer) == 0.0f) {
        return false;
    }
    TVec3f yDir;
    mRot.getYDir(yDir);
    TVec3f c1;
    PSVECCrossProduct(&yDir, &toPlayer, &c1);
    TVec3f c2;
    PSVECCrossProduct(&mUpVec, &toPlayer, &c2);
    return c1.dot(c2) != 0.0f;
}

// SMG2 fn_8015C8C0: dash toward the target; returns true when close or after 30 steps.
bool BeeEater::moveAttackDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        mRot.getYDir(mFaceVec);
    }

    TVec3f dash = mFaceVec;
    dash.scale(1.5f);
    MR::attenuateVelocity(this, 0.99f);
    f32 dist = MR::calcDistance(this, mBasePos);
    MR::turnQuatYDirRate(&mRot, mRot, dash, dist * 0.0f);
    if (dist > 0.0f || MR::isGreaterStep(this, 30)) {
        return true;
    }
    return false;
}
