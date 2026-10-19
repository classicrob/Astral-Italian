#pragma auto_inline off
#include "Game/MapObj/BreakableCage.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/DummyDisplayModel.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MathUtil.hpp"

namespace {
    const Vec cHitSensorOffsetCage = {0.0f, 0.0f, 0.0f};
    const Vec cHitSensorOffsetFixation = {0.0f, 0.0f, 0.0f};
};  // namespace

namespace NrvBreakableCage {
    NEW_NERVE(BreakableCageNrvWait, BreakableCage, Wait);
    NEW_NERVE(BreakableCageNrvWaitStartDemoBreak, BreakableCage, WaitStartDemoBreak);
    NEW_NERVE(BreakableCageNrvBreak, BreakableCage, Break);
};  // namespace NrvBreakableCage

BreakableCage::BreakableCage(const char* pName)
    : LiveActor(pName), mBreakModel(), mCageType(CAGE_INVALID), mRotationSpeed(), mItemModel(), mIgnoreGravity(), mDelayDeadActivate(),
      mCameraInfo(), mIsRiddle() {
    mMtx.identity();
}

void BreakableCage::init(const JMapInfoIter& rIter) {
    TVec3f sensorOffset(::cHitSensorOffsetCage);
    const char* pName;
    f32 sensorRadius = 350.0f;

    MR::getObjectName(&pName, rIter);
    const char* pModelName = pName;

    if (MR::isEqualString(pName, "BreakableCageRotate")) {
        pModelName = "BreakableCage";
        mCageType = CAGE_NORMAL;
    } else if (MR::isEqualString(pName, "BreakableCageL")) {
        sensorRadius = 650.0f;
        mCageType = CAGE_LARGE;
    } else if (MR::isEqualString(pName, "BreakableFixation")) {
        mCageType = CAGE_FIX;
        sensorOffset.set(::cHitSensorOffsetFixation);
        sensorRadius = 425.0f;
    } else if (MR::isEqualString(pName, "BreakableTrash")) {
        mCageType = CAGE_TRASH;
        MR::joinToGroupArray(this, rIter, "ゴミ管理", 32);
    } else if (MR::isEqualString(pName, "BreakableCapsuleCage")) {
        pModelName = "BreakableCapsuleCage";
        sensorRadius = 650.0f;
        mCageType = CAGE_CAPSULE;
    }

    initMapToolInfo(rIter);

    f32 scaledRadius = sensorRadius * mScale.x;
    sensorOffset.scale(mScale.x);

    initModel(pModelName, rIter);
    MR::connectToSceneMapObjStrongLight(this);
    initHitSensor(1);
    MR::addHitSensor(this, "Body", 0x4C /* ATYPE_BREAKABLE_CAGE */, 20, scaledRadius, sensorOffset);
    MR::initCollisionParts(this, pModelName, getSensor("Body"), nullptr);

    if (!isTypeCage()) {
        initEffectKeeper(0, pName, false);
    }

    MR::setClippingTypeSphere(this, 100.0f + scaledRadius);
    MR::setGroupClipping(this, rIter, 0x20);

    if (mCageType == CAGE_LARGE) {
        initSound(4, "BreakableCageL", nullptr, TVec3f(0.0f));
    } else {
        initSound(4, "BreakableCage", nullptr, TVec3f(0.0f));
    }

    if (!mIgnoreGravity) {
        initBaseMtxForCage();
    } else {
        MR::makeMtxTR(mMtx.toMtxPtr(), this);
    }

    if (MR::tryRegisterDemoCast(this, rIter)) {
        if (isTypeCage()) {
            MR::tryRegisterDemoCast(mBreakModel, rIter);
        }

        if (mItemModel != nullptr) {
            MR::tryRegisterDemoCast(mItemModel, rIter);
        }
    }

    MR::addToAttributeGroupSearchTurtle(this);

    if (isAppearPowerStar()) {
        MR::declarePowerStar(this);
    }

    if (MR::createActorCameraInfoIfExist(rIter, &mCameraInfo)) {
        MR::initActorCamera(this, rIter, &mCameraInfo);
    }

    initNerve(GET_NERVE(BreakableCage, BreakableCageNrvWait));

    if (MR::isExistStageSwitchAwake(rIter)) {
        MR::useStageSwitchAwake(this, rIter);
        makeActorDead();
    } else {
        MR::useStageSwitchSyncAppear(this, rIter);
    }

    if (MR::isEqualString(pName, "BreakableCage") && mScale.y > 1.0f) {
        MR::startBtk(this, "Scale");
    }
}

void BreakableCage::appear() {
    mRotation.zero();
    MR::showModel(this);
    MR::validateHitSensor(this, "Body");
    MR::validateClipping(this);

    if (isTypeCage()) {
        mBreakModel->kill();
    }

    if (mCageType == CAGE_TRASH && MR::isValidSwitchDead(this)) {
        MR::offSwitchDead(this);
    }

    setNerve(GET_NERVE(BreakableCage, BreakableCageNrvWait));
    LiveActor::appear();
}

void BreakableCage::kill() {
    LiveActor::kill();

    if (isTypeCage()) {
        mBreakModel->kill();
    }

    if (mItemModel != nullptr) {
        mItemModel->kill();
    }
}

void BreakableCage::calcAndSetBaseMtx() {
    if (mCageType == CAGE_NORMAL) {
        TPos3f rotationMtx;
        rotationMtx.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), MR::toRadian(mRotation.y));
        TPos3f baseMtx;
        baseMtx.concat(mMtx, rotationMtx);
        MR::setBaseTRMtx(this, baseMtx);
    } else {
        MR::setBaseTRMtx(this, mMtx);
    }
}

bool BreakableCage::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool result;

    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg)) {
        result = tryBreak();
    } else {
        result = false;
    }

    return result;
}

bool BreakableCage::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool result;

    if (MR::isMsgEnemyAttackFire(msg) || MR::isMsgEnemyAttackFireStrong(msg)) {
        result = false;
    } else {
        result = tryBreak();
    }

    return result;
}

void BreakableCage::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
    MR::initUseStageSwitchWriteDead(this, rIter);

    if (mCageType == CAGE_NORMAL) {
        MR::getJMapInfoArg0WithInit(rIter, &mRotationSpeed);
        mRotationSpeed *= 0.01f;
    }

    if (isTypeCage()) {
        MR::getJMapInfoArg1NoInit(rIter, &mIgnoreGravity);
    } else {
        mIgnoreGravity = true;
    }

    MR::getJMapInfoArg2NoInit(rIter, &mDelayDeadActivate);

    s32 riddleArg = -1;
    MR::getJMapInfoArg3NoInit(rIter, &riddleArg);
    mIsRiddle = riddleArg != -1;
}

// NONMATCHING: paired-single copy registers and load scheduling differ.
void BreakableCage::initModel(const char* pName, const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, pName, false);

    if (isTypeCage()) {
        ModelObj* pBreakModel = MR::createModelObjMapObjStrongLight("壊れる籠壊れモデル", "BreakableCageBreak", mMtx.toMtxPtr());
        mBreakModel = pBreakModel;
        pBreakModel->initWithoutIter();
        MR::setScale(mBreakModel, mScale);

        if (mCageType == CAGE_LARGE) {
            TVec3f largeScale(mScale);
            largeScale.scale(1.4f);
            MR::setScale(mBreakModel, largeScale);
        }

        MR::invalidateClipping(mBreakModel);
        MR::registerDemoSimpleCastAll(mBreakModel);
        mBreakModel->makeActorDead();
        mItemModel = MR::createDummyDisplayModel(this, rIter, -1, TVec3f(0.0f, 150.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));

        if (mItemModel != nullptr) {
            if (MR::getDummyDisplayModelId(rIter, -1) == 4) {
                MR::startAction(mItemModel, "Rotation");
                MR::declareStarPiece(this, 5);
            }
        }
    }

}

void BreakableCage::initBaseMtxForCage() {
    MR::calcGravity(this);

    TPos3f baseMtx;
    MR::makeMtxRotate(baseMtx, mRotation);
    TVec3f front;
    baseMtx.getZDir(front);

    MR::makeMtxUpFrontPos(&mMtx, -mGravity, front, mPosition);
}

bool BreakableCage::isTypeCage() const {
    bool isCage = true;
    bool isTrashCage = true;
    bool isStandardCage = true;
    bool isFixedCage = true;
    CageType type = mCageType;
    if (type != CAGE_INVALID && type != CAGE_LARGE) {
        isFixedCage = false;
    }
    if (!isFixedCage && type != CAGE_NORMAL) {
        isStandardCage = false;
    }
    if (!isStandardCage && type != CAGE_TRASH) {
        isTrashCage = false;
    }
    if (!isTrashCage && type != CAGE_CAPSULE) {
        isCage = false;
    }

    return isCage;
}

bool BreakableCage::isAppearPowerStar() const {
    bool res = false;

    if (mItemModel != nullptr && MR::getDummyDisplayModelId(mItemModel) == 7) {
        res = true;
    }

    return res;
}

bool BreakableCage::tryBreak() {
    if (isNerve(GET_NERVE(BreakableCage, BreakableCageNrvWait))) {
        MR::invalidateClipping(this);

        if (mCameraInfo != nullptr) {
            MR::requestStartDemoWithoutCinemaFrame(this, "破壊", GET_NERVE(BreakableCage, BreakableCageNrvBreak),
                                                   GET_NERVE(BreakableCage, BreakableCageNrvWaitStartDemoBreak));
        } else {
            setNerve(GET_NERVE(BreakableCage, BreakableCageNrvBreak));
        }

        return true;
    }

    return false;
}

void BreakableCage::exeWait() {
    // SMG2 target only rotates the cage here; the Petari item-model spin block is not present.
    if (mCageType == CAGE_NORMAL) {
        mRotation.y = MR::repeat(mRotation.y + mRotationSpeed, 0.0f, 360.0f);
    }
}

void BreakableCage::exeWaitStartDemoBreak() {
}

// NONMATCHING: equivalent enum comparison folding and register allocation differ.
void BreakableCage::exeBreak() {
    u32 activateDeadEarly = 1;
    u32 hasImmediateDeadCondition = 0;

    if (mDelayDeadActivate || mItemModel != nullptr || mCageType == static_cast< s32 >(CAGE_FIX) || mCageType == CAGE_TRASH) {
        hasImmediateDeadCondition = 1;
    }

    if (!hasImmediateDeadCondition && mCageType != CAGE_NORMAL) {
        activateDeadEarly = 0;
    }

    if (MR::isFirstStep(this)) {
        if (mCageType == CAGE_FIX) {
            MR::startActionSound(this, "OjBreakFixationBreak", -1, -1, -1);
        } else {
            MR::startActionSound(this, "Break", -1, -1, -1);
        }

        if (mIsRiddle) {
            MR::startActionSound(this, "Riddle", -1, -1, -1);
        }

        MR::hideModel(this);
        MR::invalidateHitSensor(this, "Body");
        MR::invalidateCollisionParts(this);

        if (mCameraInfo != nullptr) {
            MR::startActorCameraTargetSelf(this, getCamInfo(), -1);

            if (mBreakModel != nullptr) {
                MR::requestMovementOn(mBreakModel);
            }
        }

        if (isTypeCage()) {
            mBreakModel->appear();
            MR::startAction(mBreakModel, "Break");
        } else {
            MR::emitEffect(this, "Break");
        }

        if (mItemModel != nullptr && MR::getDummyDisplayModelId(mItemModel) == 4) {
            MR::appearStarPiece(this, mItemModel->mPosition, 5, 10.0f, 40.0f, false);
        }

        if (isAppearPowerStar()) {
            if (mItemModel != nullptr) {
                MR::requestAppearPowerStar(this, mItemModel->mPosition);
            } else {
                MR::requestAppearPowerStar(this, this, 150.0f);
            }

            MR::requestMovementOn(this);
        }

        if (mItemModel != nullptr) {
            mItemModel->kill();
        }

        if (activateDeadEarly && MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }
    }

    if (isAppearPowerStar() || mCameraInfo != nullptr) {
        MR::stopSceneAtStep(this, 2, 16);
    } else if (MR::isNearPlayer(this, 1500.0f)) {
        f32 radius = MR::getSensorRadius(this, "Body");

        if (!MR::isJudgedToClipFrustum(mPosition, radius)) {
            MR::stopSceneAtStep(this, 2, 6);
        }
    }

    bool canDoSwitch;

    if (mCameraInfo != nullptr) {
        canDoSwitch = MR::isStep(this, 120);
    } else if (isTypeCage()) {
        canDoSwitch = MR::isActionEnd(mBreakModel);
    } else {
        canDoSwitch = MR::isEffectValid(this, "Break") == false;
    }

    if (canDoSwitch) {
        if (!activateDeadEarly && MR::isValidSwitchDead(this)) {
            MR::onSwitchDead(this);
        }

        if (MR::isValidSwitchB(this)) {
            MR::offSwitchB(this);
        }

        if (mCameraInfo != nullptr) {
            MR::endActorCamera(this, getCamInfo(), false, -1);
            MR::endDemo(this, "破壊");
        }

        kill();
    }
}

BreakableCage::~BreakableCage() {
}
