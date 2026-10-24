#include "Game/Enemy/BeeEater.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvBeeEater {
    NEW_NERVE(BeeEaterNrvWait, BeeEater, Wait);
    class BeeEaterNrvHit : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<BeeEater*>(pSpine->mExecutor)->exeBeeEaterWaitAfterHit();
        }
        static BeeEaterNrvHit sInstance;
    };
    BeeEaterNrvHit BeeEaterNrvHit::sInstance ATTRIBUTE_WEAK;
    class BeeEaterNrvFind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<BeeEater*>(pSpine->mExecutor)->exeBeeEaterFind();
        }
        static BeeEaterNrvFind sInstance;
    };

    class BeeEaterNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MR::updateActorStateAndNextNerve(static_cast<LiveActor*>(pSpine->mExecutor), &BeeEaterNrvFind::sInstance);
        }
        static BeeEaterNrvBindStarPointer sInstance;
    };
    BeeEaterNrvBindStarPointer BeeEaterNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
    BeeEaterNrvFind BeeEaterNrvFind::sInstance ATTRIBUTE_WEAK;
    class BeeEaterNrvAttack : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<BeeEater*>(pSpine->mExecutor)->exeBeeEaterAttackSign();
        }
        static BeeEaterNrvAttack sInstance;
    };
    BeeEaterNrvAttack BeeEaterNrvAttack::sInstance ATTRIBUTE_WEAK;
    class BeeEaterNrvAttackEnd : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<BeeEater*>(pSpine->mExecutor)->exeBeeEaterAttack();
        }
        static BeeEaterNrvAttackEnd sInstance;
    };
    BeeEaterNrvAttackEnd BeeEaterNrvAttackEnd::sInstance ATTRIBUTE_WEAK;
    class BeeEaterNrvAttackSuccess : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<BeeEater*>(pSpine->mExecutor)->exeBeeEaterAttackEnd();
        }
        static BeeEaterNrvAttackSuccess sInstance;
    };
    BeeEaterNrvAttackSuccess BeeEaterNrvAttackSuccess::sInstance ATTRIBUTE_WEAK;
    class BeeEaterNrvAttackDash : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<BeeEater*>(pSpine->mExecutor)->exeBeeEaterAttackSuccess();
        }
        static BeeEaterNrvAttackDash sInstance;
    };
    BeeEaterNrvAttackDash BeeEaterNrvAttackDash::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBeeEater

#define GET_BEE_EATER_NERVE(name) (&NrvBeeEater::name::sInstance)

static const char* sJointNames[] = {"Body1", "Body2", "Body3", "Body4", "Body5"};

BeeEaterJointControl::BeeEaterJointControl(LiveActor* pHost, const char* pJointName)
    : mController(nullptr), mPos(0.0f, 0.0f, 0.0f), mDir(0.0f, 1.0f, 0.0f) {
    mController = MR::createJointController<BeeEaterJointControl>(this, pHost, pJointName, &BeeEaterJointControl::calcBeeEaterJointMatrix, 0);
}

void BeeEaterJointControl::setBeeEaterJointDirection(const TVec3f* pDir) {
    TVec3f old(mDir);
    if (MR::normalizeOrZero(&mDir, *pDir)) {
        mDir.set(old);
    }
}

bool BeeEaterJointControl::calcBeeEaterJointMatrix(TPos3f* pMtx, const JointControllerInfo&) {
    MR::turnMtxToXDirRate(pMtx, mDir, 1.0f);
    pMtx->setTrans(mPos);
    return true;
}

BeeEaterBody::BeeEaterBody(BeeEater* pHost, const char* pName, const char* pJointName)
    : PartsModel(pHost, pName, pJointName, nullptr, -1, 0) {
    mJointCtrls = nullptr;
    mJointCount = 0;
}

void BeeEaterBody::init(const JMapInfoIter& rIter) {
    PartsModel::init(rIter);
    mJointCount = 5;
    mJointCtrls = new BeeEaterJointControl*[5];
    for (s32 i = 0; i < mJointCount; i++) {
        BeeEaterJointControl* pCtrl = new BeeEaterJointControl(this, sJointNames[i]);
        mJointCtrls[i] = pCtrl;
    }
}

void BeeEaterBody::calcAndSetBaseMtx() {
    PartsModel::calcAndSetBaseMtx();
    if (mJointCount > 0) {
        for (s32 i = 0; i < mJointCount; i++) {
            mJointCtrls[i]->mController->registerCallBack();
        }
    }
}

BeeEater::BeeEater(const char* pName) : LiveActor(pName) {
    mRot.x = 0.0f;
    mRot.y = 0.0f;
    mRot.z = 0.0f;
    mRot.w = 1.0f;
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mSideVec.set(1.0f, 0.0f, 0.0f);
    mUpVec.set(0.0f, 1.0f, 0.0f);
    mRangeLimit = 450.0f;
    mTargetVec.set(0.0f, 1.0f, 0.0f);
    mFaceVec.set(0.0f, 0.0f, 1.0f);
    mBody = nullptr;
    mAnimScaleCtrl = nullptr;
}

void BeeEater::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::makeQuatFromRotate(&mRot, this);
    mBasePos.set(mPosition);
    MR::calcSideVec(&mSideVec, this);
    MR::calcUpVec(&mUpVec, this);
    mTargetVec.set(mUpVec);
    mGravity.set(-mUpVec);

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

// NONMATCHING: quaternion matrix instruction scheduling and curve temporary stack allocation.
void BeeEater::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.setQuat(mRot);
    mtx.setTrans(mPosition);
    MR::addTransMtxLocalY(mtx, 76.0f * -(1.0f - mAnimScaleCtrl->mScale.y));
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mAnimScaleCtrl);

    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f upOffset(up);
    upOffset.scale(76.0f);
    TVec3f end = mPosition - upOffset;
    TVec3f baseUp(mUpVec);
    f32 distance = PSVECDistance(&end, &mBasePos);
    f32 length = distance / 3.0f;
    length = 100.0f >= length ? length : 100.0f;
    f32 blend = MR::normalize(distance, 270.0f, 450.0f);
    TVec3f delta = end - mBasePos;
    TVec3f direction;
    if (MR::normalizeOrZero(&direction, delta)) {
        direction.set(baseUp);
    }
    TVec3f endUp(up);
    if (direction.dot(up) < 0.0f) {
        endUp.killElement(up, direction);
        MR::normalizeOrZero(&endUp);
    }
    mCurveEnd.set(end);
    TVec3f endOffset(endUp);
    endOffset.scale(-(distance * 0.5f));
    mCurveControl2.set(mCurveEnd + endOffset);
    TVec3f baseOffset(baseUp);
    baseOffset.scale(length);
    mCurveControl1.set(mBasePos + baseOffset);
    mCurveStart.set(mBasePos);
    if (blend > 0.0f) {
        TVec3f step1(delta);
        step1.scale(1.0f);
        TVec3f third1(step1);
        third1.scale(1.0f / 3.0f);
        TVec3f control1 = mBasePos + third1;
        TVec3f step2(delta);
        step2.scale(2.0f);
        TVec3f third2(step2);
        third2.scale(1.0f / 3.0f);
        TVec3f control2 = mBasePos + third2;
        MR::vecBlend(&mCurveControl1, mCurveControl1, control1, blend);
        MR::vecBlend(&mCurveControl2, mCurveControl2, control2, blend);
    }
    s32 count = mBody->mJointCount;
    for (s32 i = 0; i < count; i++) {
        f32 t = (f32)i / (f32)(count - 1);
        f32 u = 1.0f - t;
        f32 a = u * (u * u);
        f32 b = u * (3.0f * t * u);
        f32 c = u * (3.0f * t * t);
        f32 d = t * (t * t);
        TVec3f p3(mCurveEnd);
        p3.scale(d);
        TVec3f p2(mCurveControl2);
        p2.scale(c);
        TVec3f p1(mCurveControl1);
        p1.scale(b);
        TVec3f p0(mCurveStart);
        p0.scale(a);
        TVec3f pos = p0 + p1 + p2 + p3;
        mBody->mJointCtrls[i]->mPos.set(pos);
    }
    mBody->mJointCtrls[0]->setBeeEaterJointDirection(&mUpVec);
    for (s32 i = 1; i < count - 1; i++) {
        TVec3f dir = mBody->mJointCtrls[i + 1]->mPos - mBody->mJointCtrls[i]->mPos;
        mBody->mJointCtrls[i]->setBeeEaterJointDirection(&dir);
    }
    mBody->mJointCtrls[count - 1]->setBeeEaterJointDirection(&endUp);
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

bool BeeEater::isPlayerInBeeEaterAimRange() const {
    if (MR::isNearPlayer(mBasePos, 120.0f)) {
        return false;
    }
    return MR::isNearAngleDegree(*MR::getPlayerPos() - mBasePos, mUpVec, 100.0f);
}

bool BeeEater::isPlayerBehindBeeEaterRotation() const {
    TVec3f toPlayer;
    MR::calcVecToPlayer(&toPlayer, this);
    if (mUpVec.dot(toPlayer) >= 0.0f) {
        return false;
    }
    TVec3f yDir;
    mRot.getYDir(yDir);
    TVec3f c1;
    PSVECCrossProduct(&yDir, &toPlayer, &c1);
    TVec3f c2;
    PSVECCrossProduct(&mUpVec, &toPlayer, &c2);
    return !(c1.dot(c2) >= 0.0f);
}

void BeeEater::updateBeeEaterHover() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateClipping(this);
    }
    MR::addVelocityMoveToDirection3D(this, mUpVec, 1.0f);
    MR::addVelocityMoveToTarget3D(this, mBasePos, 0.0f, 2.0f, 100.0f, 200.0f);
    MR::attenuateVelocity(this, 0.85f);
    MR::turnQuatXDirRad(&mRot, mRot, mSideVec, 0.03490658476948738f);
    if (isPlayerInBeeEaterAimRange()) {
        if (isPlayerBehindBeeEaterRotation()) {
            mTargetVec.set(mUpVec);
        } else {
            MR::calcDirToTarget(&mTargetVec, this, MR::getPlayerCenterPos());
        }
    }
    MR::turnQuatYDirRad(&mRot, mRot, mTargetVec, 0.03490658476948738f);
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
}

// NONMATCHING: dash/direction temporary stack allocation.
bool BeeEater::updateBeeEaterDash() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        mRot.getYDir(mFaceVec);
    }
    MR::addVelocity(this, mFaceVec.scaleInline(1.5f));
    MR::attenuateVelocity(this, 0.99f);
    f32 distance = MR::calcDistance(this, mBasePos);
    f32 rate = MR::normalize(distance, 445.5f, 450.0f);
    TVec3f dir = mPosition - mBasePos;
    MR::normalizeOrZero(&dir);
    MR::turnQuatYDirRate(&mRot, mRot, dir, rate);
    if (distance > 450.0f || MR::isGreaterStep(this, 30)) {
        return true;
    }
    return false;
}

void BeeEater::exeBeeEaterWaitAfterHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::zeroVelocity(this);
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvFind));
    }
}

void BeeEater::exeBeeEaterFind() {
    updateBeeEaterHover();
    TVec3f up;
    MR::calcUpVec(&up, this);
    if (MR::isFaceToPlayerDegree(this, up, 20.0f) && MR::isNearPlayer(this, 1000.0f)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttack));
        MR::invalidateClipping(this);
    }
}

void BeeEater::exeBeeEaterAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
    }
    if (MR::isLessEqualStep(this, 80)) {
        if (isPlayerInBeeEaterAimRange()) {
            MR::calcDirToTarget(&mTargetVec, this, MR::getPlayerCenterPos());
            MR::turnQuatYDirRad(&mRot, mRot, mTargetVec, 0.008726646192371845f);
        }
        TVec3f retreat(mTargetVec);
        retreat.scale(-0.5f);
        MR::addVelocity(this, retreat);
        MR::addVelocityMoveToDirection3D(this, mUpVec, 1.0f);
        MR::addVelocityMoveToTarget3D(this, mBasePos, 0.0f, 2.0f, 100.0f, 400.0f);
        MR::attenuateVelocity(this, 0.85f);
    } else {
        MR::zeroVelocity(this);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttackEnd));
    }
}

void BeeEater::exeBeeEaterAttack() {
    if (updateBeeEaterDash()) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvAttackSuccess));
        return;
    }
    if (MR::reboundVelocityFromCollision(this, 0.0f, 10.0f, 0.0f)) {
        MR::startAction(this, "HitEnd");
        MR::emitEffectHit(this, MR::getBindedHitPos(this), "HitSmoke");
        MR::tryHitReaction(this, "地形ヒット");
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

void BeeEater::exeBeeEaterAttackEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
    }
    if (MR::isLessStep(this, 25)) {
        MR::attenuateVelocity(this, 0.0f);
    } else {
        TVec3f retreat(mFaceVec);
        retreat.scale(-2.5f);
        MR::addVelocity(this, retreat);
        MR::attenuateVelocity(this, 0.85f);
        if (isPlayerInBeeEaterAimRange()) {
            if (isPlayerBehindBeeEaterRotation()) {
                mTargetVec.set(mUpVec);
            } else {
                MR::calcDirToTarget(&mTargetVec, this, MR::getPlayerCenterPos());
            }
        }
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 0.0f);
    if (MR::isGreaterStep(this, 40)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

void BeeEater::exeBeeEaterAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
    }
    MR::zeroVelocity(this);
    if (MR::isActionEnd(this)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvWait));
    }
}

void BeeEater::exeWait() {
    updateBeeEaterHover();
    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_BEE_EATER_NERVE(BeeEaterNrvFind));
    }
}

