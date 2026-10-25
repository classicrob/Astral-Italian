#include "Game/Enemy/BrosBase.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/BrosProjectileBase.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"


BrosBase::BrosBase(const char* pName)
    : LiveActor(pName), mHeld(nullptr), mProjectileIdx(0), mFront(0.0f, 0.0f, 1.0f), mAllowRailJump(true), mIsInitialAttack(false), mItemType(1), mJointCtrl(nullptr), mUnusedCC(0), mConnector(nullptr), mConnected(false) {
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));
}

void BrosBase::init(const JMapInfoIter& rIter) {
    mJointCtrl = new ActorJointCtrl(this);
    mJointCtrl->endFaceCtrl(0);
    mScaleCtrl = new AnimScaleController(nullptr);
    mScaleCtrl->setParamTight();
    MR::disableAllItemGenerator(this);
    bool connect = false;
    MR::getJMapInfoArg3WithInit(rIter, &connect);
    if (connect) {
        mConnector = new MapObjConnector(this);
    }
}

void BrosBase::calcHoldMtx(MtxPtr pDst) {
    const TVec3f* rot;
    MtxPtr pJoint = MR::getJointMtx(this, "HandR");
    Mtx local;
    PSMTXIdentity(local);
    rot = &getHoldRotate();
    const TVec3f& trans = getHoldTrans();
    MR::makeMtxTR(local, trans, *rot);
    PSMTXConcat(pJoint, local, pDst);
}

bool BrosBase::isExistActiveProjectile() const {
    for (s32 i = 0; i < 5; i++) {
        if (!MR::isDead(mProjectiles[i]) && !mProjectiles[i]->v7C() && !mProjectiles[i]->v84()) {
            return true;
        }
    }
    return false;
}

bool BrosBase::isExistProjectileNear(f32 dist) const {
    for (s32 i = 0; i < 5; i++) {
        if (!MR::isDead(mProjectiles[i]) && MR::calcDistance(this, mProjectiles[i]) < dist) {
            return true;
        }
    }
    return false;
}

void BrosBase::tryHoldProjectile() {
    if (mHeld == nullptr) {
        mProjectileIdx = (mProjectileIdx + 1) % 5;
        if (MR::isDead(mProjectiles[mProjectileIdx])) {
            mHeld = mProjectiles[mProjectileIdx];
            mHeld->attachToBros(this);
        }
    }
}

void BrosBase::killHeldProjectile() {
    if (mHeld != nullptr) {
        mHeld->makeActorDead();
        mHeld = nullptr;
    }
}

void BrosBase::tryThrowProjectile() {
    if (mHeld != nullptr) {
        throwProjectile(mHeld);
        mHeld = nullptr;
    } else {
        mProjectileIdx = (mProjectileIdx + 1) % 5;
        if (MR::isDead(mProjectiles[mProjectileIdx])) {
            throwProjectile(mProjectiles[mProjectileIdx]);
        }
    }
}

void BrosBase::startFaceCtrl() {
    mJointCtrl->startFaceCtrl(10);
}

void BrosBase::endFaceCtrl() {
    mJointCtrl->endFaceCtrl(10);
}

void BrosBase::updateVelocity() {
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 1.5f);
    } else {
        MR::killVelocityVertical(this);
    }
    MR::attenuateVelocity(this, MR::isBindedGround(this) ? 0.94f : 0.98f);
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.5f);
    }
}

void BrosBase::calcAndSetBaseMtx() {
    mJointCtrl->setCallBackFunction();
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    MR::updateBaseScale(this, mScaleCtrl);
    if (mConnector != nullptr && mConnected) {
        mConnector->connect();
    }
}

void BrosBase::initByArg(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg1NoInit(rIter, &mItemType);
    if (mItemType == 0) {
        MR::disableAllItemGenerator(this);
    }
    MR::declareStarPiece(this, 9);
    MR::declareCoin(this, 3);
}

void BrosBase::appearStarPiece() {
    if (mItemType != 0) {
        MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false);
    }
}

void BrosBase::appearCoin() {
    if (mItemType != 0) {
        MR::appearCoinPop(this, mPosition, 1);
    }
}

void BrosBase::emitBrosYoshiEatItem() {
    MR::emitItemYoshiEat(this);
}

namespace {
    inline const TVec3f& getBrosProjectilePosition(const LiveActor& actor) {
        return actor.mPosition;
    }
}

void BrosBase::killProjectiles() {
    const TVec3f* pos;
    for (s32 i = 0; i < 5; i++) {
        BrosProjectileBase* p = mProjectiles[i];
        switch (p->mState) {
        case 1:
            p->clearState();
            break;
        case 2: {
            pos = &getBrosProjectilePosition(*p);
            if (MR::getDeclareRemnantStarPieceCount(this) > 3) {
                MR::appearStarPiece(this, *pos, 1, 10.0f, 40.0f, false);
            }
            mProjectiles[i]->clearState();
            break;
        }
        case 3:
            p->clearState();
            break;
        }
    }
}

void BrosBase::attachToUnder() {
    if (mConnector != nullptr) {
        mConnected = true;
        mConnector->attachToUnder();
    }
}
