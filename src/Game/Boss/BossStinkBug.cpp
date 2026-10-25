#include "Game/Util/Functor.hpp"  // FunctorBase is defined before BossStinkBug (weak vtable order)
#include "Game/Boss/BossStinkBug.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/BossStinkBugActionSequencer.hpp"
#include "Game/Boss/BossStinkBugBomb.hpp"
#include "Game/Boss/BossStinkBugBombHolder.hpp"
#include "Game/Boss/BossStinkBugFollowValidater.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/GroundChecker.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JMath/JMath.hpp>
#include <revolution/mtx.h>
#include <revolution/types.h>

namespace {
    const char* sSensorNameList[] = {"Body", "BodyHipDrop"};

    struct collisionInfo {
        const char* _0;
        const char* _4;
        const char* _8;
    };

    const collisionInfo sCollisionInfo[] = {{"Body", "Body", "Body"}, {"BodyGround", "Body", "Body"}, {"BodyHipDrop", "Body", "BodyHipDrop"}};

    const char* sLegJointNameList[] = {"RightFootA3", "LeftFootA3", "RightFootB3", "LeftFootB3", "RightFootC3", "LeftFootC3"};

    static TVec3f sAppearStarPieceOffset(0.0f, 200.0f, 0.0f);
};  // namespace

BossStinkBug::BossStinkBug(const char* pName)
    : LiveActor(pName), mCollisionParts(), mBombHolder(), mJointCtrl(), mCameraInfo(), mActionSequencer(), mWingModel(), mBombLauncher(), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mFront(0.0f, 0.0f, 1.0f),
      mCameraTargetPos(0.0f, 0.0f, 0.0f), mCameraRegisterId(-1), mTurnRate(0.15f), mPoseBlendRate(1.0f), mIsPoseUpdated(), mAreLegsBound() {
    mFollowMtx.identity();
    mLegGroundCheckers = new GroundChecker*[6];

    for (int i = 0; i < 6; i++) {
        mLegGroundCheckers[i] = new GroundChecker("地面チェック", 30.0f, 0.0f);
        mLegGroundCheckers[i]->makeActorDead();
        MR::setBinderExceptActor(mLegGroundCheckers[i], this);
    }
}

void BossStinkBug::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    invalidateCollision();
}

void BossStinkBug::makeActorDead() {
    LiveActor::makeActorDead();
    invalidateCollision();
}

void BossStinkBug::kill() {
    LiveActor::kill();

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

void BossStinkBug::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "BossStinkBug", false);
    MR::connectToSceneCollisionEnemy(this);
    MR::makeQuatAndFrontFromRotate(&mPoseQuat, &mFront, this);

    mRotation.zero();
    initSensor();
    initCollision();
    initRailRider(rIter);
    initBinder(300.0f, 300.0f, false);
    MR::setBinderExceptActor(this, this);
    initEffectKeeper(0, nullptr, false);
    MR::onCalcGravity(this);
    initCamera(rIter);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::excludeCalcShadowToActorAll(this, this);
    mJointCtrl = new ActorJointCtrl(this);
    MR::addBaseMatrixFollowTarget(this, rIter, &mFollowMtx, new BossStinkBugFollowValidater(this));
    MR::invalidateClipping(this);
    mActionSequencer = new BossStinkBugActionSequencer(this, rIter);
    mBombHolder = new BossStinkBugBombHolder(this);
    mWingModel = MR::createPartsModelMapObj(this, "羽モデル", "BossStinkBugWing", nullptr);
    mWingModel->initFixedPosition("Switch");
    mBombLauncher = MR::createPartsModelMapObj(this, "ボム発射口", "BossStinkBugBombLauncher", nullptr);
    mBombLauncher->initFixedPosition("Body");
    MR::declarePowerStar(this);
    initSound(12, nullptr, nullptr, TVec3f(0.0f));
    MR::startBrk(this, "Normal");

    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::FunctorV0M< BossStinkBug*, void (BossStinkBug::*)() >(this, &BossStinkBug::start));
        makeActorDead();
    } else {
        makeActorAppeared();
    }

    MR::useStageSwitchAwake(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);
}

void BossStinkBug::start() {
    if (mActionSequencer->startAction()) {
        appear();
    }
}

void BossStinkBug::initCamera(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg7WithInit(rIter, &mCameraRegisterId);

    if (mCameraRegisterId != -1) {
        MR::declareCameraRegisterVec(this, mCameraRegisterId, &mCameraTargetPos);
        MR::declareCameraRegisterMtx(this, mCameraRegisterId, mFollowMtx.toMtxPtr());
    }

    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "Fall");
}

void BossStinkBug::initSensor() {
    initHitSensor(5);
    MR::addHitSensorEye(this, "eye", 8, 1000.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::invalidateHitSensor(this, "eye");
    MR::addHitSensorEnemyAttack(this, "attack_left", 8, 200.0f, TVec3f(-120.0f, 50.0f, 300.0f));
    MR::addHitSensorEnemyAttack(this, "attack_right", 8, 200.0f, TVec3f(120.0f, 50.0f, 300.0f));
    MR::invalidateHitSensor(this, "attack_left");
    MR::invalidateHitSensor(this, "attack_right");

    for (int i = 0; i < ARRAY_SIZE(::sSensorNameList); i++) {
        MR::addMessageSensorReceiver(this, ::sSensorNameList[i]);
    }
}

void BossStinkBug::initCollision() {
    mCollisionParts = new CollisionParts*[3];

    for (int i = 0; i < 3; i++) {
        mCollisionParts[i] = MR::createCollisionPartsFromLiveActor(this, ::sCollisionInfo[i]._0, getSensor(::sCollisionInfo[i]._8),
                                                       MR::getJointMtx(this, ::sCollisionInfo[i]._4), MR::CollisionScaleType_AutoEqualScale);
    }
}

void BossStinkBug::disposeBomb() {
    mBombHolder->killAll();
}

void BossStinkBug::reuestMovementOnParts() {
    MR::requestMovementOn(mWingModel);
    MR::requestMovementOn(mBombLauncher);
}

LiveActor* BossStinkBug::getWingModel() {
    return mWingModel;
}

LiveActor* BossStinkBug::getBombLauncher() {
    return mBombLauncher;
}

void BossStinkBug::control() {
    updateAction();
    updatePose();
    mIsPoseUpdated = false;

    if (mAreLegsBound) {
        updateJointControl();
    }

    updateCamera();
}

void BossStinkBug::calcAndSetBaseMtx() {
    mJointCtrl->setCallBackFunction();
    TPos3f pos;
    pos.setQuat(mPoseQuat);
    pos.setTrans(mPosition);

    mFollowMtx.setInline(pos);

    MR::rotateMtxLocalXDegree(pos.toMtxPtr(), mRotation.x);
    MR::rotateMtxLocalYDegree(pos.toMtxPtr(), mRotation.y);
    MR::rotateMtxLocalZDegree(pos.toMtxPtr(), mRotation.z);

    MR::setBaseTRMtx(this, pos);
    MR::setBaseScale(this, mScale);
}

void BossStinkBug::calcUpVec(TVec3f* pOut) const {
    TPos3f pos;
    pos.identity();
    pos.setQuat(mPoseQuat);

    MR::rotateMtxLocalXDegree(pos.toMtxPtr(), mRotation.x);
    MR::rotateMtxLocalYDegree(pos.toMtxPtr(), mRotation.y);
    MR::rotateMtxLocalZDegree(pos.toMtxPtr(), mRotation.z);

    pos.getYDir(*pOut);
}

void BossStinkBug::calcAnim() {
    LiveActor::calcAnim();

    for (int i = 0; i < 3; i++) {
        mCollisionParts[i]->setMtx();
    }
}

void BossStinkBug::updateAction() {
    if (mActionSequencer != nullptr) {
        mActionSequencer->updateNerve();
    }
}

void BossStinkBug::updatePose() {
    if (!mIsPoseUpdated) {
        mFront.orthogonalize(mGravity);

        if (MR::isNearZero(mFront)) {
            mPoseQuat.getZDir(mFront);
        } else {
            MR::normalize(&mFront);
        }

        MR::blendQuatUpFront(&mPoseQuat, -mGravity, mFront, 0.2f, 0.2f);
        mIsPoseUpdated = true;
    }
}

void BossStinkBug::setPose(MtxPtr pMtx) {
    TPos3f pos(pMtx);
    pos.getQuat(mPoseQuat);
    pos.getTrans(mPosition);
    mPoseQuat.getZDir(mFront);
    MR::normalize(&mFront);
    mIsPoseUpdated = true;
}

void BossStinkBug::updateCamera() {
    mCameraTargetPos = mPosition;
}

bool BossStinkBug::isValidFollowId(s32 id) const {
    if (mActionSequencer != nullptr) {
        return mActionSequencer->isValidFollowId(id);
    } else {
        return false;
    }
}

void BossStinkBug::validateCollisionGround() {
    MR::invalidateCollisionParts(mCollisionParts[0]);
    MR::validateCollisionParts(mCollisionParts[1]);
    MR::validateCollisionParts(mCollisionParts[2]);
}

void BossStinkBug::validateCollisionFly() {
    MR::validateCollisionParts(mCollisionParts[0]);
    MR::invalidateCollisionParts(mCollisionParts[1]);
    MR::validateCollisionParts(mCollisionParts[2]);
}

void BossStinkBug::validateCollisionHitOnly() {
    MR::invalidateCollisionParts(mCollisionParts[0]);
    MR::invalidateCollisionParts(mCollisionParts[1]);
    MR::validateCollisionParts(mCollisionParts[2]);
}

void BossStinkBug::invalidateCollision() {
    for (int i = 0; i < 3; i++) {
        MR::invalidateCollisionParts(mCollisionParts[i]);
    }
}

void BossStinkBug::onBindLeg() {
    if (!mAreLegsBound) {
        for (int i = 0; i < 6; i++) {
            MR::copyJointPos(this, ::sLegJointNameList[i], &mLegGroundCheckers[i]->mPosition);
            mLegGroundCheckers[i]->appear();
            MR::resetPosition(mLegGroundCheckers[i], mLegGroundCheckers[i]->mPosition);
        }

        mAreLegsBound = true;
    }
}

void BossStinkBug::offBindLeg() {
    if (mAreLegsBound) {
        mJointCtrl->endIKCtrlAll();

        for (int i = 0; i < 6; i++) {
            mLegGroundCheckers[i]->kill();
        }

        mAreLegsBound = false;
    }
}

bool BossStinkBug::isLegInWater() const {
    for (int i = 0; i < 6; i++) {
        if (MR::isBindedGroundWater(mLegGroundCheckers[i])) {
            return true;
        }
    }

    return false;
}

void BossStinkBug::updateJointControl() {
    ActorJointCtrl* jointCtrl = mJointCtrl;
    jointCtrl->startUpdate();

    for (int i = 0; i < 6; i++) {
        TVec3f legJointPos;
        MR::copyJointPos(this, ::sLegJointNameList[i], &legJointPos);
        TVec3f groundCheckPos = mLegGroundCheckers[i]->mPosition;
        TVec3f dirGroundCheckToLegJoint = legJointPos - groundCheckPos;

        if (MR::isBinded(mLegGroundCheckers[i])) {
            mLegGroundCheckers[i]->bringFriction(mTurnRate, 0.2f);
        } else {
            mLegGroundCheckers[i]->bringFriction(1.0f, 0.08f);
        }

        dirGroundCheckToLegJoint.scale(mLegGroundCheckers[i]->mFrictionRate);
        mLegGroundCheckers[i]->mVelocity.set(dirGroundCheckToLegJoint);
        mLegGroundCheckers[i]->movement();
        jointCtrl->setIKEndPosition(::sLegJointNameList[i], mLegGroundCheckers[i]->mPosition, mPoseBlendRate);

        if (MR::isBinded(mLegGroundCheckers[i])) {
            TVec3f fix = MR::getBindedFixReactionVector(mLegGroundCheckers[i]);

            if (!MR::isNearZero(fix)) {
                MR::normalize(&fix);
                jointCtrl->setIKEndDirection(::sLegJointNameList[i], fix, 0.1f * mPoseBlendRate);
            }
        } else {
            TVec3f yDir;
            MR::extractMtxYDir(MR::getJointMtx(this, ::sLegJointNameList[i]), &yDir);
            jointCtrl->setIKEndDirection(::sLegJointNameList[i], yDir, 0.05f * mPoseBlendRate);
        }
    }

    jointCtrl->endUpdate();
}

void BossStinkBug::appearStarPiece(s32 num) {
    TVec3f offsetVec;
    PSMTXMultVec(MR::getJointMtx(this, "Body"), ::sAppearStarPieceOffset, offsetVec);
    MR::appearStarPieceToDirection(this, offsetVec, -mGravity, num, 30.0f, 40.0f, false);
}

void BossStinkBug::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mActionSequencer != nullptr) {
        mActionSequencer->attackSensor(pSender, pReceiver);
    }
}

bool BossStinkBug::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mActionSequencer != nullptr) {
        return mActionSequencer->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    } else {
        return false;
    }
}

bool BossStinkBug::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mActionSequencer != nullptr) {
        return mActionSequencer->receiveOtherMsg(msg, pSender, pReceiver);
    } else {
        return false;
    }
}

void BossStinkBug::startEventCamera(const char* pName) {
    MR::startMultiActorCameraTargetSelf(this, mCameraInfo, pName);
}

void BossStinkBug::endEventCamera(const char* pName) {
    MR::endMultiActorCamera(this, mCameraInfo, pName, false);
}

bool BossStinkBug::isSensorBody(const HitSensor* pSensor) const {
    return pSensor == getSensor("BodyHipDrop");
}

bool BossStinkBug::throwBomb(f32 f1, f32 f2) {
    BossStinkBugBomb* throwBomb = mBombHolder->getDeadMember();

    if (throwBomb == nullptr) {
        return false;
    }

    TPos3f jointMtx(MR::getJointMtx(mBombLauncher, "DischargePosition"));

    TVec3f trans;
    TVec3f yDir;

    jointMtx.getTrans(trans);
    jointMtx.getYDir(yDir);
    yDir *= -f1;
    MR::addRandomVector(&yDir, yDir, f2);

    throwBomb->start(trans, yDir);

    MR::startActionSound(this, "BmBossBugBombEmit");
    MR::startAction(mBombLauncher, "Discharge");
    return true;
}
