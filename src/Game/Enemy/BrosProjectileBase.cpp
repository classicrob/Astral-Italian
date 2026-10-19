#include "Game/Enemy/BrosProjectileBase.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/BrosBase.hpp"


BrosProjectileBase::BrosProjectileBase(const char* pName)
    : LiveActor(pName), mOwner(nullptr), mScaleCtrl(nullptr), mFront(0.0f, 0.0f, 1.0f), mLocalVel(0.0f, 0.0f, 0.0f), mState(0), _B4(false) {
}

void BrosProjectileBase::init(const JMapInfoIter& rIter) {
    mScaleCtrl = new AnimScaleController(nullptr);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, getBaseMtx(), TVec3f(0.0f));
}

void BrosProjectileBase::clearState() {
    mState = 0;
}

void BrosProjectileBase::killWithState1() {
    mState = 1;
    kill();
}

void BrosProjectileBase::killWithState2() {
    mState = 2;
    kill();
}

void BrosProjectileBase::killWithState3() {
    mState = 3;
    kill();
}

void BrosProjectileBase::killWithState4() {
    mState = 4;
    kill();
}

void BrosProjectileBase::appear() {
    LiveActor::appear();
    MR::setScale(this, TVec3f(0.0f));
    MR::validateHitSensors(this);
    mLocalVel.z = 0.0f;
    mLocalVel.y = 0.0f;
    mLocalVel.x = 0.0f;
    mState = 0;
    MR::calcGravity(this);
    mOwner = nullptr;
    _B4 = false;
}

void BrosProjectileBase::control() {
    mScaleCtrl->updateNerve();
    if (!v7C() && !v80() && !v78()) {
        YoshiFunction::tryYoshiProjectile(mYoshiLockOn);
    }
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    MR::calcFrontVec(&mFront, this);
    if (!_B4) {
        TVec3f vel(0.0f);
        PSMTXMultVecSR(getBaseMtx(), &mLocalVel, &vel);
        MR::setVelocity(this, vel);
    } else {
        MR::setVelocity(this, TVec3f(0.0f));
    }
}

void BrosProjectileBase::calcAndSetBaseMtx() {
    if (mOwner != nullptr) {
        Mtx mtx;
        PSMTXIdentity(mtx);
        mOwner->calcHoldMtx(mtx);
        MR::setBaseTRMtx(this, mtx);
        MR::extractMtxTrans(mtx, &mPosition);
    }
    MR::calcFrontVec(&mFront, this);
    MR::updateBaseScale(this, mScaleCtrl);
}

void BrosProjectileBase::attachToBros(BrosBase* pBros) {
    mOwner = pBros;
    if (MR::isDead(this)) {
        Mtx mtx;
        PSMTXIdentity(mtx);
        mOwner->calcHoldMtx(mtx);
        MR::setBaseTRMtx(this, mtx);
        MR::extractMtxTrans(mtx, &mPosition);
        MR::calcFrontVec(&mFront, this);
        appear();
        mOwner = pBros;
    }
    MR::zeroVelocity(this);
    MR::tryDeleteEffectAll(this);
}

void BrosProjectileBase::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (v84()) {
        if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver) || MR::isSensorNpc(pReceiver)) {
            if (MR::sendArbitraryMsg(8, pReceiver, pSender)) {
                mState = 4;
                kill();
            }
        }
    } else if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (!_B4 && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            mState = 3;
            kill();
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (MR::isSensorYoshi(pReceiver)) {
        if (!_B4) {
            if (MR::sendMsgPush(pReceiver, pSender)) {
                mState = 3;
                kill();
                return;
            }
        }
        return;
    } else {
        MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
}

bool BrosProjectileBase::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHitAll(msg) || MR::isMsgStarPieceAttack(msg)) {
        mState = 2;
        kill();
        kill();
        return true;
    }
    return false;
}

bool BrosProjectileBase::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isDead(this)) {
        return false;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        return v88();
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiStartEaten(msg)) {
        return v8C();
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        return v90();
    }
    if (MR::isMsgYoshiCancelEaten(msg)) {
        mState = 4;
        kill();
        kill();
        return true;
    }
    return false;
}

void BrosProjectileBase::exeEaten() {
    TPos3f mtx;
    MR::calcYoshiTongueTipMtx(&mtx);
    mPosition.set<f32>(mtx[0][2], mtx[1][2], mtx[2][2]);
    mFront.set<f32>(mtx[0][1], mtx[1][1], mtx[2][1]);
}

