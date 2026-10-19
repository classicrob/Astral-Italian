#define JGEOMETRY_VEC3_COPY_NOINLINE
#define J3DMODEL_BASE_SCALE_NOINLINE
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/HitSensorKeeper.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Map/StageSwitch.hpp"

#include "Game/Util.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/LiveActor/ShadowController.hpp"
#include "Game/LiveActor/ActorLightCtrl.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/EffectKeeper.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/LiveActor/ActorActionKeeper.hpp"
#include "Game/LiveActor/RailRider.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/ClippingDirector.hpp"

namespace MR {
    AllLiveActorGroup* getAllLiveActorGroup();
    void requestCalcActorShadow(LiveActor*);
    void disconnectToDrawTemporarily(NameObj*);
    void disconnectToSceneTemporarily(NameObj*);
    void connectToDrawTemporarily(NameObj*);
    void connectToSceneTemporarily(NameObj*);
    void addToClippingTarget(LiveActor*);
    void removeFromClippingTarget(LiveActor*);
}

void LiveActor::init(const JMapInfoIter&) {
}

void LiveActor::appear() {
    makeActorAppeared();
}

// J3DModel::setBaseScale
// LiveActor::calcViewAndEntry
// NONMATCHING: equivalent range-check lowering
bool LiveActor::receiveMessage(u32 msg, HitSensor* sender, HitSensor* receiver) {
    if (msg == 0x33) return receiveMsgPush(sender, receiver);
    if (msg != 0 && msg < 0x26) return receiveMsgPlayerAttack(msg, sender, receiver);
    if (msg > 0x59 && msg < 0x75) return receiveMsgEnemyAttack(msg, sender, receiver);
    return receiveOtherMsg(msg, sender, receiver);
}
// LiveActor::calcAndSetBaseMtx
// LiveActor::getTakingMtx

void LiveActor::setNerve(const Nerve* pNerve) {
    mSpine->setNerve(pNerve);
}

bool LiveActor::isNerve(const Nerve* pNerve) const {
    return mSpine->getCurrentNerve() == pNerve;
}

s32 LiveActor::getNerveStep() const {
    return mSpine->mStep;
}

HitSensor* LiveActor::getSensor(const char* pSensorName) const {
    if (mSensorKeeper != nullptr) {
        return mSensorKeeper->getSensor(pSensorName);
    }

    return nullptr;
}

// LiveActor::getBaseMtx
// LiveActor::startClipped
// LiveActor::endClipped
void LiveActor::initModelManagerWithAnm(const char* model, const char* anim, const char* action, bool flag) {
    mModelManager = new ModelManager(MR::getCameraProjectionMtx());
    mModelManager->init(model, anim, flag);
    MR::getJ3DModel(this)->setBaseScale(mScale);
    LiveActor::calcAndSetBaseMtx();
    MR::calcJ3DModel(this);
    mActionKeeper = ActorActionKeeper::tryCreate(this, action);
}

void LiveActor::initNerve(const Nerve* pNerve, s32 numStates) {
    mSpine = new Spine(this, pNerve, numStates);
}

void LiveActor::initHitSensor(s32 numSensors) {
    mSensorKeeper = new HitSensorKeeper(numSensors);
}

// LiveActor::initBinder
// LiveActor::initRailRider
// NONMATCHING: argument load ordering/regalloc
void LiveActor::initEffectKeeper(s32 count, const char* name, bool sort) {
    mEffectKeeper = new EffectKeeper(getName(), MR::getModelResourceHolder(this), count, name);
    if (sort) mEffectKeeper->enableSort();
    mEffectKeeper->init(this);
    if (mBinder != nullptr) mEffectKeeper->setBinder(mBinder);
}
void LiveActor::initSound(s32 count, const char* name, const TVec3f* position, const TVec3f& offset) {
    mSoundHolder = new ActorSoundHolder(this, count, name, position, TVec3f(offset));
}
// LiveActor::initShadowControllerList
// LiveActor::initActorCollisionParts

void LiveActor::initStageSwitch(const JMapInfoIter& rIter) {
    mStageSwitchCtrl = MR::createStageSwitchCtrl(this, rIter);
}

// LiveActor::initActorStarPointerTarget
// LiveActor::initActorLightCtrl

void LiveActor::attackSensor(HitSensor*, HitSensor*) {
}

// LiveActor::updateBinder

void LiveActor::makeActorAppeared() {
    if (mSensorKeeper != nullptr) {
        mSensorKeeper->validateBySystem();
    }

    mFlags.mIsDead = false;

    if (MR::isClipped(this)) {
        endClipped();
    }

    if (mCollisionParts != nullptr) {
        MR::validateCollisionParts(this);
    }

    MR::resetPosition(this);

    if (mActorLightCtrl != nullptr) {
        mActorLightCtrl->reset();
    }

    MR::tryUpdateHitSensorsAll(this);
    MR::addToClippingTarget(this);
    MR::connectToSceneTemporarily(this);

    if (!MR::isNoEntryDrawBuffer(this)) {
        MR::connectToDrawTemporarily(this);
    }
}

void LiveActor::kill() {
    makeActorDead();

    // SMG2: the switch-dead flag sits at +0x10 of StageSwitchCtrl (not named in our header).
    if (mStageSwitchCtrl != nullptr && mStageSwitchCtrl->isValidSwitchDead() && *reinterpret_cast< bool* >(reinterpret_cast< u8* >(mStageSwitchCtrl) + 0x10)) {
        MR::onSwitchDead(this);
    }
}

void LiveActor::makeActorDead() {
    mVelocity.zero();

    MR::clearHitSensors(this);

    if (mSensorKeeper != nullptr) {
        mSensorKeeper->invalidateBySystem();
    }

    if (getBinder() != nullptr) {
        mBinder->clear();
    }

    if (mEffectKeeper != nullptr) {
        mEffectKeeper->clear();
    }

    if (mCollisionParts != nullptr) {
        MR::invalidateCollisionParts(this);
    }

    mFlags.mIsDead = true;

    MR::removeFromClippingTarget(this);
    MR::disconnectToSceneTemporarily(this);
    MR::disconnectToDrawTemporarily(this);
}

void LiveActor::movement() {
    if (mModelManager != nullptr && !mFlags.mIsNotReleasedAnimFrame) {
        mModelManager->update();

        if (mActionKeeper != nullptr) {
            mActionKeeper->updateAnimKeeper();
        }
    }

    if (MR::isCalcGravity(this)) {
        MR::calcGravity(this);
    }

    if (mSensorKeeper != nullptr) {
        mSensorKeeper->doObjCol();
    }

    if (mFlags.mIsDead) {
        return;
    }

    if (mSpine != nullptr) {
        mSpine->update();
    }

    if (mFlags.mIsDead) {
        return;
    }

    control();

    if (mFlags.mIsDead) {
        return;
    }

    updateBinder();

    if (mActionKeeper != nullptr) {
        mActionKeeper->update();
    }

    if (mEffectKeeper != nullptr) {
        mEffectKeeper->update();
    }


    if (mActorLightCtrl != nullptr) {
        MR::updateLightCtrl(this);
    }

    MR::tryUpdateHitSensorsAll(this);
    MR::actorSoundMovement(this);
    MR::requestCalcActorShadow(this);
}

void LiveActor::calcAnim() {
    if (mFlags.mIsOnCalcAnim) {
        return;
    }

    calcAnmMtx();

    if (mCollisionParts == nullptr) {
        return;
    }

    MR::setCollisionMtx(this);
}

void LiveActor::calcAnmMtx() {
    if (mModelManager == nullptr) {
        return;
    }

    MR::getJ3DModel(this)->setBaseScale(mScale);
    calcAndSetBaseMtx();
    mModelManager->calcAnim();
}

void LiveActor::calcViewAndEntry() {
    if (mFlags.mIsNoCalcView) {
        return;
    }

    if (mModelManager == nullptr) {
        return;
    }

    if (mFlags.mIsNoCalcView) {
        return;
    }

    mModelManager->calcView();
}


void LiveActor::calcAndSetBaseMtx() {
    if (MR::getTaken(this)) {
        MR::setBaseTRMtx(this, MR::getTaken(this)->mHost->getTakingMtx());
    } else {
        TPos3f mtx;

        if (mRotation.x == 0.0f && mRotation.z == 0.0f) {
            MR::makeMtxTransRotateY(mtx, this);
        } else {
            MR::makeMtxTR(mtx, this);
        }

        MR::setBaseTRMtx(this, mtx);
    }
}

MtxPtr LiveActor::getTakingMtx() const {
    return getBaseMtx();
}

MtxPtr LiveActor::getBaseMtx() const {
    if (MR::getJ3DModel(this) != nullptr) {
        return MR::getJ3DModel(this)->mBaseTransformMtx;
    }

    return nullptr;
}

void LiveActor::startClipped() {
    mFlags.mIsClipped = true;

    if (getSensorKeeper() != nullptr) {
        mSensorKeeper->invalidateBySystem();
    }

    if (mEffectKeeper != nullptr) {
        mEffectKeeper->stopEmitterOnClipped();
    }

    MR::disconnectToSceneTemporarily(this);

    if (MR::isNoEntryDrawBuffer(this)) {
        return;
    }

    MR::disconnectToDrawTemporarily(this);
}

void LiveActor::endClipped() {
    mFlags.mIsClipped = false;

    if (getSensorKeeper() != nullptr) {
        mSensorKeeper->validateBySystem();
        MR::updateHitSensorsAll(this);
    }

    if (mEffectKeeper != nullptr) {
        mEffectKeeper->playEmitterOffClipped();
    }

    MR::connectToSceneTemporarily(this);

    if (MR::isNoEntryDrawBuffer(this)) {
        return;
    }

    MR::connectToDrawTemporarily(this);
}

void LiveActor::initBinder(f32 radius, f32 offsetY, u32 planeNum) {
    mBinder = new Binder(getBaseMtx(), &mPosition, &mGravity, radius, offsetY, planeNum);

    MR::onBind(this);

    if (mEffectKeeper != nullptr) {
        mEffectKeeper->setBinder(mBinder);
    }
}

void LiveActor::initRailRider(const JMapInfoIter& rIter) {
    mRailRider = new RailRider(rIter);
}

void LiveActor::initShadowControllerList(u32 shadowNum) {
    mShadowControllerList = new ShadowControllerList(this, shadowNum);
}

// NONMATCHING: matrix/argument register scheduling
void LiveActor::initActorCollisionParts(const char* pParam1, HitSensor* pParam2, ResourceHolder* pParam3, MtxPtr pParam4, bool param5, bool param6) {
    MR::CollisionScaleType scaleType;

    if (param6) {
        scaleType = MR::CollisionScaleType_NotUsingScale;
    } else {
        scaleType = MR::CollisionScaleType_Unk2;

        if (param5) {
            scaleType = MR::CollisionScaleType_AutoEqualScale;
        }
    }

    if (pParam3 != nullptr) {
        TPos3f mtx;

        if (pParam4 != nullptr) {
            mtx.set(pParam4);
        } else {
            MR::makeMtxTRS(mtx.toMtxPtr(), this);
        }

        mCollisionParts = MR::createCollisionPartsFromResourceHolder(pParam3, pParam1, pParam2, mtx, pParam4, scaleType);
    } else if (pParam4 == nullptr) {
        mCollisionParts = MR::createCollisionPartsFromLiveActor(this, pParam1, pParam2, scaleType);
    } else {
        mCollisionParts = MR::createCollisionPartsFromLiveActor(this, pParam1, pParam2, pParam4, scaleType);
    }

    MR::invalidateCollisionParts(this);
}

void LiveActor::initActorStarPointerTarget(f32 radius, const TVec3f* pTrans, MtxPtr pMtx, TVec3f offset) {
    mStarPointerTarget = new StarPointerTarget(radius, pTrans, pMtx, offset);
}

void LiveActor::initActorLightCtrl() {
    mActorLightCtrl = new ActorLightCtrl(this);
}

void LiveActor::updateBinder() {
    if (mBinder == nullptr) {
        mPosition += mVelocity;
    } else if (mFlags.mIsOnBind) {
        mPosition += mVelocity;
        mBinder->clear();
    } else {
        mPosition += getBinder()->bind(mVelocity);
    }
}

LiveActor::LiveActor(const char* pName)
    : NameObj(pName), mPosition(0.0f, 0.0f, 0.0f), mRotation(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), mVelocity(0.0f, 0.0f, 0.0f),
      mGravity(0.0f, -1.0f, 0.0f), mActionKeeper(), mModelManager(), mSpine(), mSensorKeeper(), mBinder(), mRailRider(), mEffectKeeper(),
      mSoundHolder(), mShadowControllerList(), mCollisionParts(), mStageSwitchCtrl(), mStarPointerTarget(), mActorLightCtrl()  {
    MR::getAllLiveActorGroup()->registerActor(this);
    MR::getClippingDirector()->registerActor(this);
}
