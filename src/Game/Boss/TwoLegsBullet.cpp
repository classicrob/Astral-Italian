#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "JSystem/JMath/JMATrigonometric.hpp"

namespace NrvTwoLegsBullet {
    NEW_NERVE(TwoLegsBulletNrvMove, TwoLegsBullet, Move);
    NEW_NERVE(TwoLegsBulletNrvStop, TwoLegsBullet, Stop);
    NEW_NERVE(TwoLegsBulletNrvWalk, TwoLegsBullet, Walk);
    NEW_NERVE(TwoLegsBulletNrvReflect, TwoLegsBullet, Reflect);
    NEW_NERVE(TwoLegsBulletNrvRise, TwoLegsBullet, Rise);
    NEW_NERVE(TwoLegsBulletNrvTrample, TwoLegsBullet, Trample);
    class TwoLegsBulletNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TwoLegsBullet* pActor = reinterpret_cast< TwoLegsBullet* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                pActor->killHill();
            }
            MR::updateActorStateAndNextNerve(pActor, &TwoLegsBulletNrvRise::sInstance);
        }
        static TwoLegsBulletNrvBindStarPointer sInstance;
    };
    TwoLegsBulletNrvBindStarPointer TwoLegsBulletNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvTwoLegsBullet

TwoLegsBullet::TwoLegsBullet(TwoLegs* pHost, const char* pName) : LiveActor(pName) {
    mHost = pHost;
    mFront.set(0.0f, 0.0f, 1.0f);
    mSide.set(1.0f, 0.0f, 0.0f);
    mShadowDir.set(0.0f, -1.0f, 0.0f);
    mMtx.identity();
    mHill = nullptr;
    mHillDir.zero();
    mScaleCtrl = nullptr;
    mStateBind = nullptr;
}

TwoLegsBullet::~TwoLegsBullet() {}

void TwoLegsBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Horino", false);
    initNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance, 1);
    mScaleCtrl = new AnimScaleController(nullptr);
    mStateBind = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mStateBind, &NrvTwoLegsBullet::TwoLegsBulletNrvBindStarPointer::sInstance, "SupportFreeze");
    MR::setShadowDropDirectionPtr(this, "Body", &mShadowDir);
    MR::setEffectHostMtx(this, "GroundBoss", mMtx);
    MR::setEffectHostMtx(this, "Underground", mMtx);
    MR::startBtkAndSetFrameAndStop(this, "Scale", 0.0f);
    MR::invalidateClipping(this);
    mHill = new HorinoHill(this, 10, "\x83\x7a\x83\x8a\x81\x5b\x83\x6d\x92\xcb\x82\xcc\x93\xb9", 100.0f);
    mHill->initWithoutIter();
    mHill->mUpDir = &mHillDir;
    makeActorDead();
}

void TwoLegsBullet::appear() {
    if (MR::isHalfProbability()) {
        MR::reverseRailDirection(this);
    }
    MR::calcGravity(this);
    mFront.set(mGravity);
    calcSideVec();
    MR::validateShadow(this, nullptr);
    mShadowDir.set(mSide);
    LiveActor::appear();
    setNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance);
}

void TwoLegsBullet::kill() {
    killHill();
    LiveActor::kill();
    MR::emitEffect(this, "Explosion");
    MR::startActionSound(this, "BulletExplosionAS", -1, -1, -1);
}

void TwoLegsBullet::reverseMove() {
    MR::reverseRailDirection(this);
    mSide.negate();
}

void TwoLegsBullet::control() {
    mHost->addCameraTarget(this);
    if (isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance)) {
        mHillDir.negate(*mHost->getBodyFront());
    } else {
        mHillDir.negate(mGravity);
    }
    mScaleCtrl->updateNerve();
}

void TwoLegsBullet::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::makeMtxFrontSidePos(&mtx, mFront, mSide, mPosition);
    MR::setBaseTRMtx(this, mtx);
    if (isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance)) {
        TVec3f up(*mHost->getBodyFront());
        TVec3f pos;
        JMAVECScaleAdd(up, mPosition, pos, 300.0f);
        MR::makeMtxFrontUpPos(&mMtx, mFront, up, pos);
    }
    MR::updateBaseScale(this, mScaleCtrl);
}

void TwoLegsBullet::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvTrample::sInstance)) {
        if (MR::isSensorPlayerOrRide(pReceiver) && MR::isEqualSensor(pSender, this, "Attack")) {
            if (!isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvBindStarPointer::sInstance)) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender) &&
                    !isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance)) {
                    MR::startAction(this, "AttackSuccess");
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
        if (MR::isSensorEnemy(pReceiver) && isReflectSource(pSender, pReceiver) && MR::sendMsgReflect(pReceiver, pSender)) {
            if (!isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvWalk::sInstance)) {
                MR::moveCoordAndTransToNearestRailPos(this);
            }
            setNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvReflect::sInstance);
        }
    }
}

bool TwoLegsBullet::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgDrillAttack(msg)) {
        mHost->popCoins(mPosition);
        kill();
        return true;
    }
    if (isTramplable()) {
        if (MR::isMsgPlayerTrample(msg)) {
            setNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvTrample::sInstance);
            return true;
        }
        if (MR::isMsgPlayerHipDrop(msg)) {
            mHost->popCoins(mPosition);
            kill();
            return true;
        }
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    return false;
}

bool TwoLegsBullet::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance)) {
        MR::startActionSound(this, "BulletStepDowmByEnemyAS", -1, -1, -1);
        kill();
        return true;
    }
    return false;
}

bool TwoLegsBullet::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgReflect(msg)) {
        if (isReflectSource(pReceiver, pSender)) {
            MR::emitEffect(this, "Reflect");
            if (!isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvWalk::sInstance)) {
                MR::moveCoordAndTransToNearestRailPos(this);
            }
            setNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvReflect::sInstance);
            return true;
        }
        return false;
    }
    return false;
}

void TwoLegsBullet::calcSideVec() {
    TVec3f posA;
    MR::calcRailPosAtCoord(&posA, this, 0.0f);
    f32 coord;
    if (MR::isRailGoingToEnd(this)) {
        coord = 8.0f;
    } else {
        coord = -8.0f;
    }
    TVec3f posB;
    MR::calcRailPosAtCoord(&posB, this, coord);
    TVec3f gravity;
    MR::calcGravityVector(this, posA, &gravity, nullptr, 0);
    TVec3f dir(posB);
    dir.sub(posA);
    TVec3f up;
    up.negate(gravity);
    PSVECCrossProduct(up, dir, mSide);
    MR::normalize(&mSide);
}

void TwoLegsBullet::calcPosAhead(f32 scale) {
    TVec3f offs;
    offs.scale(scale, mFront);
    mPosition.add(offs);
}

void TwoLegsBullet::turnToward(const TVec3f& rTo, const TVec3f& rAxis, f32 angle) {
    if (angle < 0.0f) {
        angle = -angle;
    }
    f32 cos = JMath::sSinCosTable.table[static_cast< s32 >(45.511112f * angle) & 0x3FFF].b1;
    MR::turnVecToVecCosOnPlane(&mFront, rTo, rAxis, cos);
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "HorinoBoss");
    }
}

bool TwoLegsBullet::isTramplable() const {
    if (isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvMove::sInstance)) {
        return false;
    }
    if (isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvTrample::sInstance)) {
        return false;
    }
    if (isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvStop::sInstance)) {
        return MR::isGreaterStep(this, 15);
    }
    return true;
}

void TwoLegsBullet::killHill() {
    if (!MR::isDead(mHill)) {
        mHill->kill();
        MR::deleteEffect(this, "GroundBoss");
        MR::deleteEffect(this, "Underground");
    }
}

bool TwoLegsBullet::isReflectSource(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvWalk::sInstance) && !isNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvRise::sInstance)) {
        return false;
    }
    const TVec3f* pRecvPos = MR::getSensorPos(pReceiver);
    TVec3f dir;
    MR::calcDirectionVec(&dir, *MR::getSensorPos(pSender), *pRecvPos);
    if (dir.dot(MR::getRailDirection(this)) < 0.0f) {
        return false;
    }
    return true;
}

bool TwoLegsBullet::tryStartPointBind() {
    if (mStateBind->tryStartPointBind()) {
        setNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvBindStarPointer::sInstance);
        return true;
    }
    return false;
}

void TwoLegsBullet::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HorinoBoss");
        MR::emitEffect(this, "MoveBoss");
        MR::emitEffect(this, "Shoot");
        mHill->appear();
        MR::emitEffect(this, "Underground");
    }
    calcPosAhead(12.0f);
    MR::tryRumblePadVeryWeak(this, 0);
    MR::shakeCameraWeak();
    if (MR::isStep(this, 0x42)) {
        killHill();
        MR::emitEffect(this, "AppearBoss");
    }
    if (MR::isStep(this, 0x3F)) {
        MR::startActionSound(this, "BulletAppearAS", -1, -1, -1);
    } else if (MR::isLessStep(this, 0x3F)) {
        MR::startActionSound(this, "BulletMoveUndergroundAS", -1, -1, -1);
    }
    if (MR::isStep(this, 0x53)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        setNerve(&NrvTwoLegsBullet::TwoLegsBulletNrvStop::sInstance);
    }
}

void TwoLegsBullet::exeStop() {
    if (MR::isFirstStep(this)) {
        MR::deleteEffect(this, "MoveBoss");
        MR::invalidateShadow(this, nullptr);
    }
    turnToward(MR::getRailDirection(this), mSide, 4.0f);
    if (tryStartPointBind()) {
        mFront.set(MR::getRailDirection(this));
        return;
    }
    MR::setNerveAtStep(this, &NrvTwoLegsBullet::TwoLegsBulletNrvRise::sInstance, 45);
}

void TwoLegsBullet::exeWalk() {
    MR::moveCoordAndFollowTrans(this, 8.0f);
    turnToward(MR::getRailDirection(this), mSide, 4.0f);
    MR::startActionSound(this, "BulletMoveGoundAS", -1, -1, -1);
    tryStartPointBind();
}

void TwoLegsBullet::exeReflect() {
    if (MR::isFirstStep(this)) {
        reverseMove();
        killHill();
        MR::startActionSound(this, "BulletReflectAS", -1, -1, -1);
    }
    f32 value = MR::calcNerveEaseOutValue(this, 15, 8.0f, 0.0f);
    MR::moveCoordAndFollowTrans(this, value);
    turnToward(MR::getRailDirection(this), mGravity, 10.0f);
    if (!tryStartPointBind()) {
        MR::setNerveAtStep(this, &NrvTwoLegsBullet::TwoLegsBulletNrvRise::sInstance, 15);
    }
}

void TwoLegsBullet::exeRise() {
    if (MR::isFirstStep(this)) {
        mHill->appear();
        MR::emitEffect(this, "GroundBoss");
    }
    f32 value = MR::calcNerveEaseInValue(this, 30, 0.0f, 8.0f);
    MR::moveCoordAndFollowTrans(this, value);
    turnToward(MR::getRailDirection(this), mGravity, 10.0f);
    if (!tryStartPointBind()) {
        MR::setNerveAtStep(this, &NrvTwoLegsBullet::TwoLegsBulletNrvWalk::sInstance, 30);
    }
}

void TwoLegsBullet::exeTrample() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StepDown");
        mFront.set(MR::getRailDirection(this));
        killHill();
    }
    if (MR::isActionEnd(this)) {
        mHost->popCoins(mPosition);
        kill();
    }
}
