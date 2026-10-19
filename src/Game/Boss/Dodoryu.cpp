#include "Game/Boss/Dodoryu.hpp"
#include "Game/Util.hpp"
#include "Game/Boss/DodoryuDemo.hpp"
#include "Game/Boss/DodoryuHill.hpp"
#include "Game/Boss/DodoryuMove.hpp"
#include "Game/Boss/DodoryuStateBase.hpp"
#include "Game/Boss/DodoryuStateLv1.hpp"
#include "Game/Boss/DodoryuStateLv2.hpp"
#include "Game/Boss/DodoryuStateWait.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/JointRumbler.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"
#include <cstdio>

// SMG2 2-arg tryFindNamePos (no symbol name in symbols.txt)

void DodoryuStateBase::catchSendPlayerFlying(HitSensor*, HitSensor*) {
}


namespace {
// Original read-only data starts with the signed-integer conversion bias.
#pragma push
#pragma force_active on
    extern const f64 sSignedConversionBias = 4503601774854144.0;
#pragma pop
    static const Vec sHeadOffset = {50.0f, 30.0f, 0.0f};
    static const Vec sBodyOffset = {0.0f, 0.0f, 30.0f};
    static const Vec sLegOffset = {40.0f, 0.0f, 0.0f};
    static const Vec sArmOffset = {0.0f, 0.0f, 0.0f};
    static const Vec sTailOffset = {50.0f, 0.0f, 0.0f};
    static const Vec sStarPieceSensorOffset = {-150.0f, 0.0f, 0.0f};
    static const char* sHeadSensorName = "head";
    static const char* sHeadJointName = "Head";
    static const f32 sHeadRadius = 210.0f;
    static const char* sBodySensorName = "body";
    static const f32 sBodyRadius = 250.0f;
    static const char* sLegLSensorName = "leg_left";
    static const char* sLegRSensorName = "leg_right";
    static const f32 sLegRadius = 110.0f;
    static const char* sArmLSensorName = "arm_left";
    static const char* sArmRSensorName = "arm_right";
    static const f32 sArmRadius = 50.0f;
    static const char* sTailSensorName = "tail";
    static const f32 sTailRadius = 120.0f;
    static const char* sMapObjSensorName = "eye";
    static const f32 sMapObjSensorRadius = 100.0f;
    static const char* sStarPieceSensorName = "star";
    static const f32 sStarPieceSensorRadius = 400.0f;
    // static const f32 sScale = _;
    // static const _ sEarthCheckLength = _;
    // static const _ sEarthCheckExtra = _;
    // static const TVec3f sCamRegOffset = _;
    static const s32 sJumpOutStopSceneFrame = 8;
    // static const s32 sRumbleInterval = _;
    static const char* sSpinOutCamera = "ドドリュウ吹っ飛び";
    // static const f32 sSpinOutCameraInterpolation = _;
    // static const f32 sSpinOutCameraBlendRate = _;
    static const f32 sRumblePadNearDist = 500.0f;
    static const f32 sRumblePadFarDist = 1000.0f;
    // static const char* sPrintStateLabel = _;
    static const f32 sRabbitDistMax = 1500.0f;
    static const f32 sRabbitDistMin = 650.0f;
    static const f32 sRabbitRestDist = 2000.0f;
    static const f32 sRabbitFastSpeed = 8.0f;
    static const f32 sRabbitSlowSpeed = 4.0f;
    // static const f32 sTalkDistanceLong = _;
    // static const f32 sTalkDistanceShort = _;
    // static const TVec3f sRotateVel = _;
    static DodoryuChaseParam sDodoryuChaseDataLv2 = {
        60,
        16.0f,
        0.1f,
        1.5f,
        0.0f,
        600.0f,
        30,
        9.0f,
        0.1f,
        1.0f,
        0.0f,
        45,
        55,
        800.0f,
        180.0f,
        5.0f,
        0.2f,
        2.0f,
        0.0f,
        120,
        150,
        5,
        250,
        1500.0f,
        50,
        70,
        8.0f,
        50.0f,
        1.5f,
        60,
        120,
        60,
        90,
        1200.0f,
        1500.0f,
        5.0f,
        80,
        -1,
        51,
        false,
        10.0f,
        1.0f,
        3.0f,
        0.92f,
        360,
        0.01f,
        7.0f,
        50,
        60,
        10,
        20,
        {"Burrow", nullptr, "Normal", "Normal", nullptr},
        {"UnderGroundWalk", nullptr, "Normal", "Normal", nullptr},
        {"UnderGroundWalk", nullptr, nullptr, "Normal", nullptr},
        {"EffectWalkAppear", nullptr, "Normal", nullptr, "EffectWalkAppear"},
        {"EffectWalk", nullptr, nullptr, nullptr, "EffectWalk"},
        {"EffectWalkHide", nullptr, nullptr, nullptr, "EffectWalkHide"},
        {"Appear", nullptr, nullptr, "Careless", "Appear"},
        {"Walk", nullptr, nullptr, "Normal", "Walk"},
        {"Hide", nullptr, nullptr, nullptr, "Hide"},
        {"SwoonWalkStart", nullptr, nullptr, "Cry", nullptr},
        {"SwoonWalk", nullptr, nullptr, nullptr, nullptr},
        {"SwoonWalkEnd", nullptr, nullptr, nullptr, nullptr},
        {"Dive", nullptr, nullptr, "Normal", nullptr},
        {"AngryPunchDamage", nullptr, nullptr, nullptr, nullptr},
        {"AttackHit", nullptr, nullptr, nullptr, "AttackHit"},
        {"Hide", nullptr, nullptr, nullptr, "Hide"},
        {"Jump", nullptr, nullptr, "Careless", "Jump"},
        {},
        {},
        {},
    };
    static DodoryuChaseParam sDodoryuChaseDataLv3 = {
        60,
        20.0f,
        0.2f,
        1.0f,
        0.0f,
        500.0f,
        30,
        9.0f,
        0.1f,
        1.0f,
        0.0f,
        55,
        65,
        750.0f,
        180.0f,
        6.0f,
        0.2f,
        2.0f,
        0.99f,
        60,
        75,
        5,
        140,
        1500.0f,
        40,
        60,
        9.0f,
        70.0f,
        2.0f,
        60,
        120,
        60,
        90,
        1500.0f,
        1600.0f,
        5.0f,
        -1,
        45,
        -1,
        true,
        18.0f,
        0.5f,
        0.1f,
        0.96f,
        240,
        0.1f,
        7.0f,
        50,
        60,
        10,
        20,
        {"Burrow", nullptr, "Angry", "Angry", nullptr},
        {"UnderGroundWalk", nullptr, "Angry", "Angry", nullptr},
        {"UnderGroundWalk", nullptr, "Angry", "Angry", nullptr},
        {"AngryEffectWalkAppear", nullptr, nullptr, nullptr, "EffectWalkAppear"},
        {"AngryEffectWalk", nullptr, nullptr, nullptr, "EffectWalk"},
        {"AngryEffectWalkHide", nullptr, nullptr, nullptr, "EffectWalkHide"},
        {"AngryAppear", nullptr, nullptr, nullptr, "Appear"},
        {"AngryWalk", nullptr, nullptr, nullptr, "Walk"},
        {"AngryHide", nullptr, nullptr, nullptr, "Hide"},
        {"SwoonWalkStart", nullptr, "CoolDown", "Cry", nullptr},
        {"SwoonWalk", nullptr, "Normal", nullptr, nullptr},
        {"SwoonWalkEnd", nullptr, nullptr, nullptr, nullptr},
        {"AngryDive", nullptr, "AngryStart", "Angry", nullptr},
        {"AngryPunchDamage", nullptr, nullptr, nullptr, nullptr},
        {"AngryAttackHit", nullptr, nullptr, nullptr, "AttackHit"},
        {"AngryHide", nullptr, nullptr, nullptr, "Hide"},
        {"AngryJump", nullptr, nullptr, "Cry", "Jump"},
        {"AngryChaseStart", nullptr, nullptr, "Cry", "AngryChaseStart"},
        {"AngryChase", nullptr, nullptr, "Angry", "AngryChase"},
        {"AngryChaseEnd", nullptr, nullptr, nullptr, "AngryChaseEnd"},
    };
};  // namespace

namespace {
    NEW_NERVE(DodoryuBankNrvBankAppear, DodoryuBank, Appear);
};  // namespace

namespace {
    NEW_NERVE(DodoryuRabbitNrvRabbitEscapeWaiting, DodoryuRabbit, EscapeWaiting);
    NEW_NERVE(DodoryuRabbitNrvRabbitEscape, DodoryuRabbit, Escape);
    NEW_NERVE(DodoryuRabbitNrvRabbitEscapeSlow, DodoryuRabbit, EscapeSlow);
    NEW_NERVE(DodoryuRabbitNrvRabbitRest, DodoryuRabbit, Rest);
    NEW_NERVE(DodoryuRabbitNrvRabbitJump, DodoryuRabbit, Jump);
    NEW_NERVE(DodoryuRabbitNrvRabbitWait, DodoryuRabbit, Wait);
    NEW_NERVE(DodoryuRabbitNrvRabbitReturn, DodoryuRabbit, Return);
    NEW_NERVE(DodoryuRabbitNrvRabbitPleasure, DodoryuRabbit, Pleasure);
};  // namespace

Dodoryu::Dodoryu(const char* pName)
    : LiveActor(pName), mCameraTargetPos(0.0f, 0.0f, 0.0f), mCameraFollowsPlayer(), mStateIndex(), mState(), mMoveStateHolder(), mHill(), mLeadHill(), mBank(), mRabbit(),
      mGroundNormal(0.0f, 1.0f, 0.0f), mMoveFront(1.0f, 0.0f, 0.0f), mClosedAreaObj(), mBattleStarted(), mInitialCameraTarget(new CameraTargetMtx("カメラターゲットダミー")),
      mSpinOutCameraTarget(new CameraTargetMtx("カメラターゲットダミー")), mRumbleEnabled(), mRumbleStep(), mAnimScaleCtrl(new AnimScaleController(nullptr)) {
    mBaseMtx.identity();
}

void Dodoryu::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);

    mScale.set(1.0f, 1.0f, 1.0f);

    MR::initActor(this, rIter, "Dodoryu", false);

    mMoveStateHolder = new DodoryuSub::MoveStateHolder(this);

    initState();
    MR::connectToSceneEnemy(this);
    MR::initLightCtrl(this);
    MR::initFur(this);
    initHitSensor();
    TVec3f zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    initSound(8, "Dodoryu", nullptr, zero);
    initRailRider(rIter);
    MR::moveCoordToNearestPos(this, mPosition);
    initSwitch(rIter);
    initEffectKeeper(0, nullptr, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::declareCameraRegisterVec(this, 0, &mCameraTargetPos);
    MR::declareGlobalEventCameraFixedThere(::sSpinOutCamera, true, 0.0f);
    createMogucchiHill();
    mLeadHill = new DodoryuLeadHill(this);
    createDodoryuBank();
    MR::declareStarPiece(this, 24);
    MR::createCenterScreenBlur();
    mInitialCameraTarget->mMatrix.setInline(MR::getZonePlacementMtx(rIter));
    MR::startBckWithInterpole(this, "UnderGroundWalk", 0);
    MR::startBva(this, "Normal");
    MR::startBrk(this, "Normal");
    mState[mStateIndex]->start();

    if (MR::isValidSwitchB(this)) {
        MR::onSwitchB(this);
    }

    MR::useStageSwitchWriteAutoDead(this, rIter);
    makeActorAppeared();
}

void Dodoryu::initAfterPlacement() {
    snapToGround();
}

void Dodoryu::control() {
    mMoveStateHolder->execute();
    checkHipDrop();
    mState[mStateIndex]->movement();
    updateRumblePad();
    updateCameraTarget();
    mAnimScaleCtrl->updateNerve();
}

void Dodoryu::startClipped() {
    LiveActor::startClipped();
    mState[mStateIndex]->catchStartClipped();
}

void Dodoryu::endClipped() {
    LiveActor::endClipped();
    mState[mStateIndex]->catchEndClipped();
}

void Dodoryu::snapToGround() {
    MR::calcGravityVector(this, &mGravity, nullptr, false);

    Triangle triangle;
    TVec3f v4;

    if (MR::getFirstPolyOnLineToMap(&v4, &triangle, mPosition - mGravity.scaleInline(200.0f), mGravity.scaleInline(2200.0f))) {
        mGroundNormal = *triangle.getNormal(0);

        turnUpVecTo(-mGravity);
        mPosition.set(v4);
        mBaseMtx.setTrans(v4);
    }
}

bool Dodoryu::snapToWall() {
    TVec3f cylinderPos;
    MR::calcCylinderPos(&cylinderPos, mClosedAreaObj);

    TVec3f cylinderUpVec;
    MR::calcCylinderUpVec(&cylinderUpVec, mClosedAreaObj);

    f32 cylinderRadius = MR::getCylinderRadius(mClosedAreaObj);

    TVec3f v1 = mPosition - cylinderPos;
    v1.orthogonalize(cylinderUpVec);

    if (MR::isNearZero(v1)) {
        return false;
    }

    MR::normalize(&v1);
    mMoveFront.set(v1);
    v1 *= cylinderRadius;
    TVec3f v2 = cylinderPos + v1;
    mPosition.set(v2);
    mBaseMtx.setTrans(v2);

    return true;
}

void Dodoryu::setMtx(const TPos3f& rMtx) {
    mBaseMtx.setInline(&rMtx);
    mBaseMtx.getTrans(mPosition);

    if (mCameraFollowsPlayer) {
        mCameraTargetPos = *MR::getPlayerPos();
    } else {
        mCameraTargetPos = mPosition - mGravity * 150.0f;
    }
}

void Dodoryu::notifyOnSwitchA() {
    MR::invalidateClipping(this);
    MR::invalidateClipping(mHill);
    MR::invalidateClipping(mLeadHill);
    MR::invalidateClipping(mBank);
    mBattleStarted = 1;
    mHill->setAppearNum(mHill->mPieceCount);
}

void Dodoryu::nextState() {
    mState[mStateIndex]->end();

    mStateIndex++;

    if (mStateIndex >= mState.size()) {
        MR::startAfterBossRushBGM();

        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }

        kill();
    } else {
        mState[mStateIndex]->start();
    }
}

void Dodoryu::pauseOff() {
    MR::requestMovementOn(this);
    mHill->pauseOff();
    MR::requestMovementOn(mLeadHill);
    MR::requestMovementOn(mBank);
    MR::requestEffectStopSceneEnd();
}

void Dodoryu::startPlayerCenterCamera() {
    if (MR::isValidSwitchB(this)) {
        MR::onSwitchB(this);
    }
}

void Dodoryu::endPlayerCenterCamera() {
    if (MR::isValidSwitchB(this)) {
        MR::offSwitchB(this);
    }
}

void Dodoryu::getPlayerResetMtx(TPos3f* pMtx, u32 param2) const {
    char buf[64];
    snprintf(buf, sizeof(buf), "マリオ再セット%d", param2 + 1);

    MR::tryFindNamePos(buf, (MtxPtr)pMtx);
}

bool Dodoryu::checkWallWithVelocity() {
    mClosedAreaObj = nullptr;

    AreaObj* closedAreaObj = MR::getAreaObj("DodoryuClosedArea", mPosition);

    if (closedAreaObj == nullptr) {
        return false;
    }

    if (MR::isNearZero(mVelocity)) {
        return false;
    }

    TVec3f cylinderPos;
    MR::calcCylinderPos(&cylinderPos, closedAreaObj);

    TVec3f cylinderUpVec;
    MR::calcCylinderUpVec(&cylinderUpVec, closedAreaObj);

    TVec3f deltaPos = mPosition - cylinderPos;
    deltaPos.orthogonalize(cylinderUpVec);

    if (MR::isNearZero(deltaPos)) {
        return false;
    }

    if (deltaPos.dot(mVelocity) >= 0.0f) {
        return false;
    }

    MR::normalize(&deltaPos);

    mMoveFront = deltaPos;
    mClosedAreaObj = closedAreaObj;

    return true;
}

void Dodoryu::reactJumpOutCommon() const {
    MR::stopSceneForDefaultHit(::sJumpOutStopSceneFrame);
}

void Dodoryu::startSpinOutCamera() {
    MR::startGlobalEventCamera(::sSpinOutCamera, CameraTargetArg(mSpinOutCameraTarget), 120);
}

void Dodoryu::endSpinOutCamera() {
    MR::endGlobalEventCamera(::sSpinOutCamera, 0, true);
}

bool Dodoryu::keepOffFromClosedArea(TVec3f* pDirection) {
    AreaObj* pArea = MR::getAreaObj("DodoryuClosedArea", mPosition);
    if (pArea == nullptr) {
        return false;
    }

    TVec3f center;
    MR::calcCylinderPos(&center, pArea);
    TVec3f up;
    MR::calcCylinderUpVec(&up, pArea);
    TVec3f direction = mPosition - center;
    f32 height = up.dot(direction);
    direction.orthogonalize(up);

    if (MR::isNearZero(direction)) {
        TVec3f oldUp;
        mBaseMtx.getYDir(oldUp);
        TQuat4f rotation;
        rotation.setRotate(oldUp, up);
        TVec3f side;
        mBaseMtx.getXDir(side);
        rotation.transform(side);
        direction.set(side);
    } else {
        MR::normalize(&direction);
    }

    if (pDirection != nullptr) {
        pDirection->set(direction);
    }

    direction.setLength(MR::getCylinderRadius(pArea));
    TPos3f mtx;
    mtx.set(mBaseMtx);
    mtx.setTrans(center + up * height + direction);
    setMtx(mtx);
    return true;
}

void Dodoryu::tryRumblePad() {
    f32 playerDistance = MR::calcDistanceToPlayer(this);

    if (playerDistance < ::sRumblePadNearDist) {
        MR::tryRumblePadWeak(this, WPAD_CHAN0);
    } else if (playerDistance < ::sRumblePadFarDist) {
        MR::tryRumblePadVeryWeak(this, WPAD_CHAN0);
    }
}

void Dodoryu::invalidateEyeSensor() {
    MR::invalidateHitSensor(this, ::sMapObjSensorName);
}

void Dodoryu::validateEyeSensor() {
    MR::validateHitSensor(this, ::sMapObjSensorName);
}

void Dodoryu::invalidateStarPieceSensor() {
    MR::invalidateHitSensor(this, ::sStarPieceSensorName);
}

void Dodoryu::validateStarPieceSensor() {
    MR::validateHitSensor(this, ::sStarPieceSensorName);
}

bool Dodoryu::isHeadNeedle(HitSensor* pReceiver, HitSensor* pSender) const {
    if (pReceiver != getSensor(::sHeadSensorName)) {
        return false;
    }

    TVec3f deltaPos = pSender->mPosition - pReceiver->mPosition;

    TVec3f xDir;
    TPos3f jointMtx;
    jointMtx.set(MR::getJointMtx(this, ::sHeadJointName));
    jointMtx.getXDir(xDir);

    return xDir.dot(deltaPos) >= 105.0f;
}

void Dodoryu::shiftMoveStateRail(f32 param1) {
    mMoveStateHolder->shiftRail(param1);
}

void Dodoryu::shiftMoveStateChase(f32 param1, f32 param2, f32 param3, f32 param4) {
    mMoveStateHolder->shiftChase(param1, param2, param3, param4);
}

void Dodoryu::shiftMoveStateEscape(f32 param1, s32 param2, s32 param3, f32 param4, f32 param5) {
    mMoveStateHolder->shiftEscape(param1, param2, param3, param4, param5);
}

void Dodoryu::shiftMoveStateJumpOut(f32 param1, f32 param2) {
    mMoveStateHolder->shiftJumpOut(param1, param2);
}

void Dodoryu::shiftMoveStateNull() {
    mMoveStateHolder->shiftNull();
}

void Dodoryu::setMoveRailSpeed(f32 railSpeed) {
    mMoveStateHolder->setRailSpeed(railSpeed);
}

void Dodoryu::startHill() {
    mHill->startNaturally();
}

void Dodoryu::stopHill() {
    mHill->endNaturally();
}

void Dodoryu::stopHillForce() {
    mHill->end();
}

void Dodoryu::killAllHillPieces() {
    mHill->killAllPieces();
}

void Dodoryu::setHillAppearNumHalf() {
    mHill->setAppearNum(mHill->mPieceCount / 2);
}

void Dodoryu::setHillAppearNumMax() {
    mHill->setAppearNum(mHill->mPieceCount - 10);
}

void Dodoryu::startLeadHillBck(const char* pBckName) {
    if (MR::isDead(mLeadHill)) {
        mLeadHill->appear();
    }

    MR::startAction(mLeadHill, pBckName);
}

void Dodoryu::killLeadHill() {
    mLeadHill->kill();
}

void Dodoryu::appearBank() {
    if (MR::isDead(mBank)) {
    }
}

void Dodoryu::leaveRabbit() {
    DodoryuRabbit* rabbit = mRabbit;

    if (MR::isDead(rabbit)) {
        return;
    }

    rabbit->setNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitJump));
}

void Dodoryu::resetRabbit() {
    mRabbit->reset(true);
}

void Dodoryu::calcRabbitDir(TVec3f* pDir) const {
    pDir->set(mRabbit->mPosition - mPosition);
    pDir->orthogonalize(mGravity);
    MR::normalizeOrZero(pDir);
}

void Dodoryu::displayRabbitMessage() {
    mRabbit->mIsDisplayMessage = true;
}

void Dodoryu::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void Dodoryu::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, ::sStarPieceSensorName)) {
        return;
    }

    if (MR::isSensorMapObj(pReceiver)) {
        if (MR::isEqualSensor(pSender, this, ::sMapObjSensorName)) {
            MR::sendMsgEnemyAttack(pReceiver, pSender);
        }
    } else {
        mState[mStateIndex]->catchAttackSensor(pSender, pReceiver);
    }
}

bool Dodoryu::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, ::sStarPieceSensorName)) {
        if (MR::isMsgStarPieceAttack(msg)) {
            MR::startActionSound(this, "OjStarPieceHitStop", -1, -1, -1);
            mAnimScaleCtrl->startHitReaction();
            mMoveStateHolder->brake();

            return true;
        }

        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        return isHeadNeedle(pReceiver, pSender);
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        return true;
    }

    return mState[mStateIndex]->catchPlayerAttack(msg, pSender, pReceiver);
}

void Dodoryu::initHitSensor() {
    LiveActor::initHitSensor(9);

    MR::addHitSensorAtJointEnemy(this, ::sHeadSensorName, ::sHeadJointName, 8, ::sHeadRadius, ::sHeadOffset);
    MR::addHitSensorAtJointEnemy(this, ::sBodySensorName, "Hip", 8, ::sBodyRadius, ::sBodyOffset);
    MR::addHitSensorAtJointEnemy(this, ::sLegLSensorName, "LegL2", 8, ::sLegRadius, ::sLegOffset);
    MR::addHitSensorAtJointEnemy(this, ::sLegRSensorName, "LegR2", 8, ::sLegRadius, ::sLegOffset);
    MR::addHitSensorAtJointEnemy(this, ::sArmLSensorName, "HandL", 8, ::sArmRadius, ::sArmOffset);
    MR::addHitSensorAtJointEnemy(this, ::sArmRSensorName, "HandR", 8, ::sArmRadius, ::sArmOffset);
    MR::addHitSensorAtJointEnemy(this, ::sTailSensorName, "Tail", 8, ::sTailRadius, ::sTailOffset);
    MR::addHitSensorEye(this, ::sMapObjSensorName, 16, ::sMapObjSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointMapObjSimple(this, ::sStarPieceSensorName, "Hip", 16, ::sStarPieceSensorRadius, ::sStarPieceSensorOffset);
}

void Dodoryu::initSwitch(const JMapInfoIter& rIter) {
    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::Functor_Inline(this, &Dodoryu::notifyOnSwitchA));
    }

    MR::initUseStageSwitchWriteB(this, rIter);
}

void Dodoryu::createMogucchiHill() {
    mHill = new DodoryuHill(this);
    mHill->setAppearNum(5);
}

void Dodoryu::createDodoryuBank() {
    mBank = new DodoryuBank();
    mBank->initWithoutIter();
}

void Dodoryu::turnUpVecTo(const TVec3f& rVec) {
    TPos3f rotation;
    TVec3f zDir, yDir, xDir;
    TVec3f dir;
    mBaseMtx.getYDir(dir);
    rotation.setRotate(dir, rVec);
    mBaseMtx.concat(rotation, mBaseMtx);
    mBaseMtx.getXYZDir(xDir, yDir, zDir);
    Vec* pZ;
    Vec* pY = &yDir;
    pZ = &zDir;
    PSVECCrossProduct(pZ, &xDir, pY);
    PSVECCrossProduct(&xDir, pY, pZ);
    PSVECMag(pY);
    PSVECNormalize(pY, pY);
    PSVECMag(pZ);
    PSVECNormalize(pZ, pZ);
    mBaseMtx.setXDir(xDir);
    mBaseMtx.setYDir(yDir);
    mBaseMtx.setZDir(zDir);
}

void Dodoryu::checkHipDrop() {
    if (MR::isPlayerHipDropLand()) {
        mState[mStateIndex]->catchHipDrop();
    }
}

void Dodoryu::initState() {
    mState.push_back(new DodoryuStateWait(this, "DodoryuStateWait"));
    mState.push_back(new DodoryuDemoOpening(this, "DodoryuDemoOpening"));
    mState.push_back(new DodoryuStateLv1(this, "DodoryuStateLv1"));
    mState.push_back(new DodoryuDemoAppear(this, "DodoryuDemoAppear"));
    mState.push_back(new DodoryuStateLv2(this, &::sDodoryuChaseDataLv2, "DodoryuStateLv2"));
    mState.push_back(new DodoryuDemoAngry(this, "DodoryuDemoAngry"));
    mState.push_back(new DodoryuStateLv2(this, &::sDodoryuChaseDataLv3, "DodoryuStateLv3"));
    mState.push_back(new DodoryuDemoDown(this, "DodoryuDemoDown"));
    mStateIndex = 0;
}

void Dodoryu::updateRumblePad() {
    if (!mRumbleEnabled) {
        return;
    }

    mRumbleStep++;

    MR::tryRumblePadWeak(this, WPAD_CHAN0);
}

void Dodoryu::updateCameraTarget() {
    TPos3f m1;
    m1.setInline(MR::getJointMtx(this, "Hip"));

    TVec3f v1;
    m1.getTrans(v1);

    TVec3f v4 = v1 * 0.7f + *MR::getPlayerPos() * 0.3f;

    v1 = v4;
    m1.setTrans(v1);
    mSpinOutCameraTarget->mMatrix.setInline(m1);
}

DodoryuBank::DodoryuBank() : ModelObj("ドドリュウ盛土", "DodoryuBank", mBaseMtx, MR::DrawBufferType_MapObjStrongLight, -2, -2, false) {
    mBaseMtx.identity();
}

void DodoryuBank::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE_ANON(DodoryuBankNrvBankAppear));
    makeActorDead();
}

void DodoryuBank::exeAppear() {
    if (MR::isActionEnd(this)) {
        kill();
        MR::validateClipping(this);
    }
}

namespace {
    char sRabbitJumpAction[] = "Jump";
}

DodoryuRabbit::DodoryuRabbit(Dodoryu* pHost, const JMapInfoIter& rIter)
    : ModelObj("ドドリュウに追われるウサギ", "DodoryuRabbit", mBaseMtx, MR::DrawBufferType_NPC, -2, -2, false), mHost(pHost), mRailCoord(), mTalkCtrl(), mTalkInterval(300),
      mTalkMessageIndex(), mLongRumbler(), mShortRumbler(), mIsDisplayMessage() {
    mBaseMtx.identity();

    initWithoutIter();

    mTalkMtx.identity();
    mTalkMtx.setTrans(mPosition);

    mTalkCtrl = MR::createTalkCtrl(this, rIter, "DodoryuRabbit", TVec3f(100.0f, 150.0f, 100.0f), mTalkMtx);
    MR::setDistanceToTalk(mTalkCtrl, 1500.0f);
}

void DodoryuRabbit::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscape));
    initHitSensor(4);
    MR::addHitSensorAtJointEnemy(this, "body", "Spine", 8, 50.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");

    mLongRumbler = new JointRumbler(this, "Spine", 4.0f, 0.2f, 60, true, 0);
    mShortRumbler = new JointRumbler(this, "Spine", 4.0f, 0.2f, 30, true, 0);

    MR::initLightCtrl(this);
    reset(false);
    MR::invalidateClipping(this);
    makeActorAppeared();
}

void DodoryuRabbit::control() {
    bool isEscape = isNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscapeWaiting)) || isNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscape)) ||
                    isNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscapeSlow));

    if (isEscape) {
        mTalkInterval--;

        if (mTalkInterval <= 0) {
            mTalkInterval = 300;
            mTalkMessageIndex = (mTalkMessageIndex + 1) % 2;

            MR::resetAndForwardNode(mTalkCtrl, mTalkMessageIndex);
        }
    }

    if (mLongRumbler->isRumbling()) {
        mLongRumbler->update();
    }

    if (mShortRumbler->isRumbling()) {
        mShortRumbler->update();
    }

    if (isNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitPleasure)) && getNerveStep() > 30) {
        return;
    }

    mTalkMtx.setInline(MR::getJointMtx(this, "Spine"));

    if (MR::isShadowProjected(this, nullptr)) {
        TVec3f shadowPos;
        MR::getShadowProjectionPos(this, nullptr, &shadowPos);
        mTalkMtx.setTrans(shadowPos);
    }
}

void DodoryuRabbit::exeEscapeWaiting() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::startBtp(this, "Blink");
    }

    tryTalk();
}

void DodoryuRabbit::exeEscape() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
        MR::startBtp(this, "Blink");
    }

    if (MR::checkPassBckFrame(this, 3.0f)) {
        MR::startActionSound(this, "SmRabbitJump", -1, -1, -1);
    }

    updatePos(::sRabbitFastSpeed);
    tryTalk();

    if (calcCoordDiff() > ::sRabbitDistMax) {
        setNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscapeSlow));
    }
}

void DodoryuRabbit::exeEscapeSlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        MR::startBtp(this, "blink");
    }

    if (MR::checkPassBckFrame(this, 3.0f)) {
        MR::startActionSound(this, "SmRabbitJump", -1, -1, -1);
    }

    updatePos(::sRabbitSlowSpeed);
    tryTalk();

    f32 coordDiff = calcCoordDiff();

    if (coordDiff < ::sRabbitDistMin) {
        setNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscape));
    } else if (coordDiff > ::sRabbitRestDist) {
        setNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitRest));
    }
}

void DodoryuRabbit::exeRest() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Tired");
        MR::startBtp(this, "Tired");
    }

    tryTalk();

    if (calcCoordDiff() < ::sRabbitDistMin) {
        setNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscape));
    }
}

void DodoryuRabbit::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, sRabbitJumpAction);
        MR::startBtp(this, "Blink");
        mBaseMtx.setInline(mHost->mInitialCameraTarget->mMatrix);
    }

    MR::setNerveAtBckStopped(this, GET_NERVE_ANON(DodoryuRabbitNrvRabbitWait));
}

void DodoryuRabbit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBtp(this, "Blink");
    }
}

void DodoryuRabbit::exeReturn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarJump");
        MR::startBtp(this, "blink");
    }

    MR::setNerveAtBckStopped(this, GET_NERVE_ANON(DodoryuRabbitNrvRabbitPleasure));
}

void DodoryuRabbit::exePleasure() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarWait");
        MR::startBtp(this, "blink");
        MR::resetAndForwardNode(mTalkCtrl, 2);
        MR::setDistanceToTalk(mTalkCtrl, 350.0f);
    }

    tryTalk();
}

void DodoryuRabbit::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    } else if (MR::isSensorMapObj(pReceiver)) {
        MR::sendMsgEnemyAttack(pReceiver, pSender);
    }
}

bool DodoryuRabbit::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerTrample(msg)) {
        if (mShortRumbler->isRumbling()) {
            mShortRumbler->reset();
        }

        mLongRumbler->start();

        return true;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        if (mLongRumbler->isRumbling()) {
            mLongRumbler->reset();
        }

        mShortRumbler->start();

        return false;
    }

    return false;
}

void DodoryuRabbit::reset(bool param1) {
    mRailCoord = MR::getRailCoord(mHost) + 1075.0f;

    setNerve(GET_NERVE_ANON(DodoryuRabbitNrvRabbitEscape));

    if (param1) {
        updatePos(0.0f);
    }
}

void DodoryuRabbit::updatePos(f32 f1) {
    mRailCoord += f1;

    if (mRailCoord >= MR::getRailTotalLength(mHost)) {
        mRailCoord -= MR::getRailTotalLength(mHost);
    }

    TVec3f railPos;
    MR::calcRailPosAtCoord(&railPos, mHost, mRailCoord);
    mPosition.set(railPos);

    MR::calcGravityVector(this, &mGravity, nullptr, false);

    TVec3f upVec(-mGravity);
    TVec3f railDir;
    MR::calcRailDirectionAtCoord(&railDir, mHost, mRailCoord);

    TVec3f vec98;
    vec98.cross(upVec, railDir);

    Triangle triangle = Triangle();
    TVec3f vecA4;
    if (!MR::getFirstPolyOnLineToMap(&vecA4, &triangle, mPosition - mGravity.scaleInline(200.0f), mGravity.scaleInline(2200.0f))) {
        vecA4.set(railPos);
    }

    mBaseMtx.setXYZDir(vec98, upVec, railDir);
    mBaseMtx.setTrans(vecA4);
}

f32 DodoryuRabbit::calcCoordDiff() const {
    f32 coord = MR::getRailCoord(mHost);

    if (mRailCoord < coord) {
        return (mRailCoord + MR::getRailTotalLength(mHost)) - coord;
    }

    return mRailCoord - coord;
}

bool DodoryuRabbit::tryTalk() {
    if (mIsDisplayMessage) {
        return MR::tryTalkNearPlayer(mTalkCtrl);
    }

    return false;
}

DodoryuLeadHill::DodoryuLeadHill(Dodoryu* pHost) : LiveActor("ドドリュウ塚先頭"), mHostBaseMtx(pHost->getBaseMtx()), _90() {
    for (int i = 0; i < ARRAY_SIZE(_94); i++) {
        _94[i] = nullptr;
    }

    initWithoutIter();
}

void DodoryuLeadHill::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "DodoryuLeadHill", false);
    MR::connectToSceneMapObjDecorationStrongLight(this);
    MR::initLightCtrl(this);
    TVec3f zero;
    zero.setAll<f32>(0.0f);
    initSound(8, "Dodoryu", nullptr, zero);
    initEffectKeeper(8, nullptr, false);
    mPosition.setTrans(mHostBaseMtx);
    initJoint();
    makeActorDead();
}

void DodoryuLeadHill::control() {
    _90 += 0.07f;
}

bool DodoryuLeadHill::calcJoint(TPos3f* pDst, const JointControllerInfo&) {
    TPos3f rotateMtx;
    rotateMtx.makeRotate(TVec3f(1.0f, 0.0f, 0.0f), _90);
    pDst->concat(rotateMtx);

    return true;
}

void DodoryuLeadHill::calcAndSetBaseMtx() {
    for (int i = 0; i < ARRAY_SIZE(_94); i++) {
        _94[i]->registerCallBack();
    }

    if (mHostBaseMtx != nullptr) {
        mPosition.setTrans(mHostBaseMtx);
        MR::setBaseTRMtx(this, mHostBaseMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}

void DodoryuLeadHill::initJoint() {
    for (int i = 0; i < ARRAY_SIZE(_94); i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "DodoryuLeadHill%d", i + 1);

        _94[i] = MR::createJointDelegatorWithNullChildFunc(this, &DodoryuLeadHill::calcJoint, buf);
    }
}
