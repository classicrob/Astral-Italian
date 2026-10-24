#define JGEOMETRY_VEC3_SUB_NEG_NOINLINE
#define JGEOMETRY_VEC3_ZERO_NOINLINE
#include "Game/Enemy/Wanwan.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/MapObj/KeySwitch.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"

Wanwan::Wanwan(f32 speed, const char* pName) : LiveActor(pName), mItemSource(nullptr), mType(0), mModel(nullptr), mSpeed(speed), mBreakAtGoal(false), mInitPos(gZeroVec), mRadius(225.0f), mRotSpeed(0.0f), mFallVel(gZeroVec), mGravityAcceleration(1.5f), mRailPointNo(-1), mLastFront(gZeroVec), mFront(0.0f, 0.0f, 1.0f), mLandCount(0), mHasBeenAirborne(false), mZeroGravity(false), mVoiceTimer(0), mFreezeReturnNerve(nullptr), mInvalidBindMoveStep(0), mAppearStep(8), mMoveStep(45), mRotateOffs(0.0f), mRollSoundTimer(0), mGoalPos(gZeroVec), mIsAtGoal(false), mRequestedPowerStar(false), mHideAtStart(false), mScaleParam(nullptr), mScaleCtrl(nullptr), mIsInLava(false), mKeySwitch(nullptr) {
    mBaseMtx.identity();
    mLavaMtx.identity();
}

namespace NrvWanwan {
    NEW_NERVE(WanwanNrvAppearSign, Wanwan, AppearSign);
    NEW_NERVE(WanwanNrvAppear, Wanwan, Appear);
    NEW_NERVE(WanwanNrvAppearMoveInvalidBind, Wanwan, AppearMoveInvalidBind);
    NEW_NERVE(WanwanNrvMove, Wanwan, Move);
    NEW_NERVE(WanwanNrvMoveInvalidBind, Wanwan, MoveInvalidBind);
    NEW_NERVE(WanwanNrvMoveLava, Wanwan, MoveLava);
    NEW_NERVE(WanwanNrvMoveToGoal, Wanwan, MoveToGoal);
    NEW_NERVE(WanwanNrvGoalIn, Wanwan, GoalIn);
    NEW_NERVE(WanwanNrvBreak, Wanwan, Break);
    NEW_NERVE(WanwanNrvFreeze, Wanwan, Freeze);
};  // namespace NrvWanwan

// NONMATCHING: addr-of-zero-vec arg scheduling and hoisted KeySwitch name pointer in the child loop
void Wanwan::init(const JMapInfoIter& rIter) {
    mType = getType(rIter);
    switch (mType) {
    case 0:
        setName("\x83\x8f\x83\x93\x83\x8f\x83\x93");
        break;
    case 1:
        setName("\x83\x7e\x83\x6a\x83\x8f\x83\x93\x83\x8f\x83\x93");
        break;
    case 2:
        setName("\x83\x53\x81\x5b\x83\x8b\x83\x68\x83\x8f\x83\x93\x83\x8f\x83\x93");
        MR::declarePowerStar(this);
        break;
    }
    initMapToolInfo(rIter);
    initModel();
    MR::connectToSceneNoShadowedMapObjStrongLight(this);
    MR::initLightCtrl(this);
    initSensor();
    initBinder(mRadius, 0.0f, 0);
    initRailRider(rIter);
    initEffect();
    TVec3f zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    MR::initStarPointerTarget(this, 275.0f * (mType == 1 ? 0.3f : mScale.x), zero);
    MR::initUseStageSwitchWriteDead(this, rIter);
    initSound(5, "Wanwan", nullptr, TVec3f(0.0f));
    f32 dropLength;
    if (MR::getJMapInfoArg4NoInit(rIter, &dropLength)) {
        MR::initShadowVolumeCylinder(this, 225.0f * (mType == 1 ? 0.3f : mScale.x));
        MR::setShadowDropLength(this, nullptr, dropLength);
    } else {
        MR::initShadowVolumeSphere(this, 225.0f * (mType == 1 ? 0.3f : mScale.x));
    }
    if (mType == 1) {
        s32 num = MR::getChildObjNum(rIter);
        s32 i = 0;
        if (num > 0) {
            for (; i < num; i++) {
                mKeySwitch = new KeySwitch("\x8c\xae\x83\x58\x83\x43\x83\x62\x83\x60");
                MR::initChildObj(mKeySwitch, rIter, i);
                mKeySwitch->prepareDemoStart();
            }
        }
    }
    if (mType == 0) {
        MR::getJMapInfoArg6NoInit(rIter, &mHideAtStart);
    }
    mScaleParam = new AnimScaleParam();
    mScaleParam->mSpringStrength = 0.5f;
    mScaleParam->mVelocityDamping = 0.85f;
    mScaleParam->mDpdVibrationTime = 8.0f;
    mScaleParam->mDpdVibrationBase = 0.95f;
    mScaleParam->mDpdVibrationAmplitude = 0.05f;
    mScaleParam->mDpdVibrationFrequency = 2.1f;
    mScaleCtrl = new AnimScaleController(mScaleParam);
    initNerve(&NrvWanwan::WanwanNrvAppear::sInstance, 0);
    makeActorDead();
}

void Wanwan::appear() {
    mFallVel.zero();
    mGravityAcceleration = 1.5f;
    mLandCount = 0;
    mRailPointNo = -1;
    mInvalidBindMoveStep = 0;
    mIsAtGoal = false;
    mIsInLava = mType == 2;
    mScaleCtrl->stopAndReset();
    mPosition.set(mInitPos);
    mLastFront.set(mInitPos);
    mRotation.zero();
    if (!mZeroGravity) {
        MR::calcGravity(this);
    }
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::showModel(this);
    MR::invalidateClipping(this);
    MR::validateHitSensors(this);
    MR::vecKillElement(MR::getRailDirection(this), mGravity, &mFront);
    MR::normalize(&mFront);
    if (mType == 0) {
        setBtkForEnvironmentMap(this, "WanwanRolling");
    }
    LiveActor::appear();
    if (!MR::isLoopRail(this)) {
        MR::offBind(this);
        if (mHideAtStart) {
            MR::hideModel(this);
            MR::invalidateHitSensors(this);
            setNerve(&NrvWanwan::WanwanNrvAppearSign::sInstance);
        } else {
            setNerve(&NrvWanwan::WanwanNrvAppear::sInstance);
        }
    } else {
        MR::onBind(this);
        setNerve(&NrvWanwan::WanwanNrvMove::sInstance);
    }
    MR::requestMovementOn(this);
}

void Wanwan::kill() {
    LiveActor::kill();
    if (mType == 1) {
        tryOnSwitchDead();
        MR::emitEffect(this, "MiniBreak");
        MR::startActionSound(this, "EmWanwanrollminiExplosion", -1, -1, -1);
    }
    if (mKeySwitch != nullptr) {
        mKeySwitch->appearKeySwitch(mPosition);
    }
}

bool Wanwan::isGoalIn() const {
    return mIsAtGoal;
}

s32 Wanwan::getGoalStayStep() {
    return 27;
}

// NONMATCHING: final `return Gold ? 2 : 0` lowers to subfic/subfe/rlwinm mask instead of cmpwi/beq
s32 Wanwan::getType(const JMapInfoIter& rIter) {
    if (MR::isEqualObjectName(rIter, "WanwanRollingMini")) {
        return 1;
    }
    if (MR::isEqualObjectName(rIter, "WanwanRollingGold")) {
        return 2;
    }
    return 0;
}

s32 Wanwan::getAppearStarPieceNum(Rock::Type type) {
    // Wanwan mini is 1; Rock::WanwanRollingMini is a different tag (2).
    return static_cast<s32>(type) == TypeMini ? 3 : 9;
}

// NONMATCHING: stack slot order of toRail/g temporaries
void Wanwan::control() {
    if (!isNerve(&NrvWanwan::WanwanNrvAppearSign::sInstance) && !isNerve(&NrvWanwan::WanwanNrvBreak::sInstance)) {
        bool inMove;
        bool turn = false;
        bool moved = false;
        TVec3f dir;
        JMathInlineVEC::PSVECSubtract(&mPosition, &mLastFront, &dir);
        if (!MR::isNearZero(dir, 0.001f)) {
            MR::normalize(&dir);
            moved = true;
            mLastFront.set(mPosition);
        }
        turn = false;
        if (moved) {
            inMove = false;
            if (isNerve(&NrvWanwan::WanwanNrvAppear::sInstance) || isNerve(&NrvWanwan::WanwanNrvAppearMoveInvalidBind::sInstance) || isNerve(&NrvWanwan::WanwanNrvMove::sInstance) ||
                isNerve(&NrvWanwan::WanwanNrvMoveInvalidBind::sInstance) || isNerve(&NrvWanwan::WanwanNrvMoveToGoal::sInstance)) {
                inMove = true;
            }
            if (inMove) {
                turn = true;
            }
        }
        if (isInClippingRange()) {
            MR::hideModel(this);
            MR::deleteEffect(this, "Smoke");
            if (mType == 2) {
                MR::deleteEffect(this, "Light");
            }
            if (turn) {
                mFront.set(dir);
            }
        } else {
            MR::showModel(this);
            if (turn) {
                TVec3f cur(mFront);
                MR::turnVecToVecCos(&mFront, cur, dir, 0.999f, mGravity, 0.02f);
            }
        }
        if (MR::isOnGround(this)) {
            mFallVel.zero();
        }
        if (isNerve(&NrvWanwan::WanwanNrvMove::sInstance)) {
            TVec3f toRail(MR::getRailPos(this));
            JMathInlineVEC::PSVECSubtract(&toRail, &mPosition, &toRail);
            MR::vecKillElement(toRail, mGravity, &mVelocity);
            TVec3f g(mGravity);
            g.scale(mGravityAcceleration);
            JMathInlineVEC::PSVECAdd(&mFallVel, &g, &mFallVel);
            JMathInlineVEC::PSVECAdd(&mVelocity, &mFallVel, &mVelocity);
            if (MR::isOnGround(this)) {
                MR::clampLength(&mVelocity, mVelocity, 1.5f * mSpeed);
            } else {
                MR::clampLength(&mVelocity, mVelocity, 30.0f);
            }
        }
    }
    mScaleCtrl->update();
}

// NONMATCHING: vector temporary stack slots.
void Wanwan::calcAndSetBaseMtx() {
    TVec3f grav(mGravity);
    TPos3f mtx;
    calcBaseMtx(&mtx);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mScaleCtrl);
    if (isNerve(&NrvWanwan::WanwanNrvAppearMoveInvalidBind::sInstance) ||
        (isNerve(&NrvWanwan::WanwanNrvMove::sInstance) && MR::isOnGround(this)) || isNerve(&NrvWanwan::WanwanNrvMoveInvalidBind::sInstance)) {
        TVec3f up;
        if (isNerve(&NrvWanwan::WanwanNrvMove::sInstance)) {
            up.set(MR::getGroundNormal(this));
        } else {
            up.negate(grav);
        }
        f32 radius = mRadius;
        TVec3f pos;
        TVec3f off(up);
        off.scale(radius);
        JMathInlineVEC::PSVECSubtract(&mPosition, &off, &pos);
        if (MR::isSameDirection(up, mFront, 0.01f)) {
            MR::makeMtxUpNoSupportPos(&mBaseMtx, up, pos);
        } else {
            MR::makeMtxUpFrontPos(&mBaseMtx, up, mFront, pos);
        }
        f32 s = mType == 1 ? 0.3f : mScale.x;
        mBaseMtx.mMtx[0][0] *= s;
        mBaseMtx.mMtx[0][1] *= s;
        mBaseMtx.mMtx[0][2] *= s;
        mBaseMtx.mMtx[1][0] *= s;
        mBaseMtx.mMtx[1][1] *= s;
        mBaseMtx.mMtx[1][2] *= s;
        mBaseMtx.mMtx[2][0] *= s;
        mBaseMtx.mMtx[2][1] *= s;
        mBaseMtx.mMtx[2][2] *= s;
    }
}

static inline bool isBodySensor(LiveActor* pActor, const HitSensor* pSensor) {
    return pSensor == pActor->getSensor("Body");
}

void Wanwan::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    bool isBody = false;
    if (isBodySensor(this, pSender) && !isNerve(&NrvWanwan::WanwanNrvBreak::sInstance)) {
        isBody = true;
    }
    if (isBody) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            if (!isNerve(&NrvWanwan::WanwanNrvFreeze::sInstance) && !isNerve(&NrvWanwan::WanwanNrvAppear::sInstance)) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                    rumblePadAndCamera();
                } else {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            } else if (!MR::isSensorRide(pReceiver) || !MR::sendMsgIsRushEnable(pReceiver, pSender)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        } else {
            bool isTarget = false;
            if (MR::isSensorType(pReceiver, 47) || MR::isSensorType(pReceiver, 48)) {
                isTarget = true;
            }
            if (isTarget) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    if (mType != 2 && MR::isSensorType(pReceiver, 48)) {
                        setNerveBreak(true);
                    }
                }
            } else if (!isNerve(&NrvWanwan::WanwanNrvFreeze::sInstance)) {
                MR::sendMsgToEnemyAttackBlowOrTrample(pReceiver, pSender, 0.25f);
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    }
}

bool Wanwan::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvWanwan::WanwanNrvBreak::sInstance) && isBodySensor(this, pReceiver)) {
        if (MR::isMsgInvincibleAttack(msg)) {
            setNerveBreak(true);
            return true;
        }
        if (MR::isMsgStarPieceReflect(msg)) {
            return true;
        }
        if (MR::isMsgFireBallAttack(msg)) {
            return true;
        }
    }
    return false;
}

bool Wanwan::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isBody = false;
    if (isBodySensor(this, pReceiver) && !isNerve(&NrvWanwan::WanwanNrvBreak::sInstance)) {
        isBody = true;
    }
    if (isBody && mType != 2) {
        if (MR::isMsgExplosionAttack(msg) || MR::isMsgToEnemyAttackBlow(msg) || MR::isSensorType(pSender, 48)) {
            if (mType == 1 && MR::isSensorEnemy(pSender) && MR::isMsgToEnemyAttackBlow(msg)) {
                MR::tryHitReaction(this, "\x83\x70\x83\x93\x83\x60\x83\x93\x83\x4f\x83\x4c\x83\x6d\x83\x52\x83\x71\x83\x62\x83\x67");
            }
            setNerveBreak(true);
            return true;
        }
    }
    return false;
}

bool Wanwan::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 171 && isNerve(&NrvWanwan::WanwanNrvMove::sInstance)) {
        TVec3f up;
        MR::calcUpVec(&up, MR::getSensorHost(pSender));
        if (MR::calcDistanceHorizontal(this, *MR::getSensorPos(pSender), up) < 170.0f) {
            mGoalPos.set(MR::getSensorHostPos(pSender));
            setNerve(&NrvWanwan::WanwanNrvMoveToGoal::sInstance);
            return false;
        }
    }
    return false;
}

// NONMATCHING: floating-point operand/load scheduling.
void Wanwan::initMapToolInfo(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    mInitPos.set(mPosition);
    f32 scale = mType == 1 ? 0.3f : mScale.x;
    mRadius = 225.0f * scale;
    mRotSpeed = (1.1f * (180.0f * mSpeed)) / (mRadius * 3.1415927f);
    MR::getJMapInfoArg3NoInit(rIter, &mAppearStep);
    MR::getJMapInfoArg7NoInit(rIter, &mMoveStep);
    if (!MR::isZeroGravity(this)) {
        mZeroGravity = false;
        MR::onCalcGravity(this);
    } else {
        mZeroGravity = true;
        Mtx mtx;
        MR::makeMtxTR(mtx, this);
        mGravity.set<f32>(mtx[0][1], mtx[1][1], mtx[2][1]);
        mGravity.negate();
    }
}

void Wanwan::initModel() {
    switch (mType) {
    case 0:
        MR::initActor(this, "WanwanRolling", false);
        mModel = MR::createModelObjMapObjStrongLight("\x83\x8f\x83\x93\x83\x8f\x83\x93\x89\xf3\x82\xea\x83\x82\x83\x66\x83\x8b", "WanwanRollingBreak", nullptr);
        mModel->initWithoutIter();
        MR::setScale(mModel, mScale);
        mModel->makeActorDead();
        break;
    case 1:
        MR::initActor(this, "WanwanRollingMini", false);
        break;
    case 2:
        MR::initActor(this, "WanwanRollingGold", false);
        mModel = MR::createModelObjMapObjStrongLight("\x83\x53\x81\x5b\x83\x8b\x83\x68\x83\x8f\x83\x93\x83\x8f\x83\x93\x89\xf3\x82\xea\x83\x82\x83\x66\x83\x8b", "WanwanRollingGoldBreak", nullptr);
        mModel->initWithoutIter();
        MR::setScale(mModel, mScale);
        mModel->makeActorDead();
        break;
    }
    if (mModel != nullptr) {
        MR::registerDemoSimpleCastAll(mModel);
    }
}

void Wanwan::initSensor() {
    initHitSensor(1);
    f32 scale = mType == 1 ? 0.3f : mScale.x;
    MR::addHitSensor(this, "Body", 48, 16, 225.0f * scale, TVec3f(0.0f, 0.0f, 0.0f));
}

void Wanwan::initEffect() {
    if (mType == 1) {
        initEffectKeeper(0, "WanwanRolling", false);
    } else {
        initEffectKeeper(0, nullptr, false);
    }
    MR::setEffectHostMtx(this, "Smoke", mBaseMtx);
    MR::setEffectHostMtx(this, "Land", mBaseMtx);
    if (mType == 2) {
        MR::setEffectHostMtx(this, "AppearLava", mLavaMtx);
    }
}

bool Wanwan::isBreakByWall() const {
    if (MR::isBindedWall(this)) {
        const TVec3f& dir = MR::getRailDirection(this);
        return dir.dot(MR::getWallNormal(this)) < -0.5f;
    }
    return false;
}

bool Wanwan::move(f32 speed) {
    s32 prev = mRailPointNo;
    MR::moveCoord(this, speed);
    mRailPointNo = MR::getCurrentRailPointNo(this);
    if (prev != mRailPointNo) {
        s32 arg;
        MR::getCurrentRailPointArg0WithInit(this, &arg);
        if (arg >= 0) {
            mGravityAcceleration = 0.2f;
            mHasBeenAirborne = false;
        }
        MR::getCurrentRailPointArg1WithInit(this, &arg);
        if (arg >= 0) {
            setNerve(&NrvWanwan::WanwanNrvMoveInvalidBind::sInstance);
            return false;
        }
        MR::getCurrentRailPointArg3WithInit(this, &arg);
        if (arg >= 0) {
            setNerve(&NrvWanwan::WanwanNrvMoveLava::sInstance);
            return false;
        }
    }
    return true;
}

// NONMATCHING: matrix temporary stack layout and rotate-call argument scheduling.
#pragma auto_inline off
// NONMATCHING: literal load scheduling (0.0174 vs 1.0 order)
void Wanwan::calcBaseMtx(TPos3f* pMtx) const {
    TVec3f up;
    JGeometry::negateInternal(&mGravity.x, &up.x);
    if (MR::isSameDirection(up, mFront, 0.01f)) {
        MR::makeMtxUpNoSupportPos(pMtx, up, mPosition);
    } else {
        MR::makeMtxUpFrontPos(pMtx, up, mFront, mPosition);
    }
    f32 ang = 0.017453292f * mRotation.x;
    TPos3f rot;
    rot.makeRotate(TVec3f(1.0f, 0.0f, 0.0f), ang);
    pMtx->concat(*pMtx, rot);
}

#pragma auto_inline reset
bool Wanwan::isInClippingRange() const {
    f32 s = mType == 1 ? 0.3f : mScale.x;
    if (MR::isJudgedToClipFrustum(mPosition, 300.0f + 250.0f * s)) {
        return true;
    }
    s = mType == 1 ? 0.3f : mScale.x;
    if (MR::isJudgedToNearClip(mPosition, 400.0f * s)) {
        return true;
    }
    return false;
}

void Wanwan::startSoundWanwanVoice() {
    if (mVoiceTimer >= 120) {
        if (MR::calcDistanceToPlayer(this) <= 600.0f) {
            if (mType == 1) {
                MR::startActionSound(this, "EvWanwanrollminiWanwan", -1, -1, -1);
            } else {
                MR::startActionSound(this, "EvWanwanrollWanwan", -1, -1, -1);
            }
            mVoiceTimer = 0;
        }
    } else {
        mVoiceTimer++;
    }
}

void Wanwan::startRollLevelSound(bool onGround) {
    bool lava = isNerve(&NrvWanwan::WanwanNrvMoveLava::sInstance);
    if (onGround || lava) {
        mRollSoundTimer = 20;
    }
    if (mRollSoundTimer > 0) {
        if (mType == 1) {
            MR::startActionSound(this, "EmLvWanwanrollminiMove", -1, -1, -1);
        } else if (lava) {
            MR::startActionSound(this, "EmLvWanwanrollMoveRail", -1, -1, -1);
        } else {
            MR::startActionSound(this, "EmLvWanwanrollMove", -1, -1, -1);
        }
        if (MR::isBindedGroundBrake(this)) {
            MR::startActionSound(this, "OjLvRockMoveMud", -1, -1, -1);
        }
        mRollSoundTimer--;
    }
    if (mType == 2) {
        MR::startActionSound(this, "EmLvWanwanrollgoldGlitter", -1, -1, -1);
    }
}

bool Wanwan::tryFreeze(const Nerve* pNerve) {
    if (mIsInLava) {
        return false;
    }
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        mVelocity.zero();
        mScaleCtrl->startDpdHitVibration();
        MR::startDPDHitSound();
        mFreezeReturnNerve = pNerve;
        setNerve(&NrvWanwan::WanwanNrvFreeze::sInstance);
        return true;
    }
    return false;
}

void Wanwan::setBtkForEnvironmentMap(LiveActor* pActor, const char* pName) {
    f32 frame;
    if (mType == 0) {
        if (MR::isNearZero(mScale.x - 0.9f, 0.001f)) {
            frame = 1.0f;
        } else if (MR::isNearZero(mScale.x - 0.5f, 0.001f)) {
            frame = 2.0f;
        } else {
            frame = 0.0f;
        }
    }
    MR::startBtkAndSetFrameAndStop(pActor, pName, frame);
}

void Wanwan::rumblePadAndCamera() {
    MR::startRumbleWithShakeCameraWeak(this, "強", "中", 1000.0f, 2000.0f);
    f32 dist = MR::calcDistanceToPlayer(this);
    if (dist < 1000.0f) {
        MR::shakeCameraNormal();
    } else if (dist < 2000.0f) {
        MR::shakeCameraWeak();
    }
}

void Wanwan::setNerveBreak(bool star) {
    if (star) {
        if (mType == 0 || (mType == 1 && !MR::isValidSwitchDead(this) && mKeySwitch == nullptr)) {
            appearStarPiece();
        }
    }
    setNerve(&NrvWanwan::WanwanNrvBreak::sInstance);
}

void Wanwan::updateRotateX(f32 v) {
    mRotation.x = MR::repeatDegree(v);
}

void Wanwan::appearStarPiece() {
    TVec3f dir(mGravity);
    dir.negate();
    MR::appearStarPieceToDirection(mItemSource, mPosition, dir, getAppearStarPieceNum((Rock::Type)mType), 10.0f, 40.0f, false);
}

// NONMATCHING: vector temporary stack slots.
void Wanwan::moveOnRail(f32 speed, f32 rot, bool checkGround) {
    MR::moveCoordAndFollowTrans(this, speed);
    updateRotateX(mRotation.x + rot);
    if (checkGround) {
        f32 len = 2.0f * mRadius;
        TVec3f dir(mGravity);
        dir.scale(len);
        TVec3f hit;
        if (MR::getFirstPolyOnLineToMap(&hit, nullptr, mPosition, dir)) {
            f32 neg = -mRadius;
            TVec3f off(mGravity);
            off.scale(neg);
            JMathInlineVEC::PSVECAdd(&hit, &off, &mPosition);
        }
    }
}

bool Wanwan::isForceInvalidBindSection() const {
    for (s32 i = MR::getCurrentRailPointNo(this); i < MR::getRailPointNum(this); i++) {
        bool flag;
        MR::getNextRailPointArg2WithInit(this, &flag);
        if (flag) {
            return true;
        }
    }
    return false;
}

bool Wanwan::tryBreakReachedGoal() {
    if (MR::isRailReachedGoal(this)) {
        if (mBreakAtGoal) {
            setNerve(&NrvWanwan::WanwanNrvBreak::sInstance);
        } else {
            makeActorDead();
        }
        return true;
    }
    return false;
}

bool Wanwan::tryRequestAppearPowerStar() {
    MR::stopSceneAtStep(this, 2, 12);
    if (MR::isStep(this, 2)) {
        requestAppearPowerStar();
    }
    return true;
}

void Wanwan::requestAppearPowerStar() {
    MR::requestAppearPowerStar(this, mModel->mPosition);
    mRequestedPowerStar = true;
}

bool Wanwan::tryOnSwitchDead() {
    if (!MR::isValidSwitchDead(this)) {
        return false;
    }
    MR::onSwitchDead(this);
    return true;
}

void Wanwan::exeFreeze() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, mType == 1 ? "MiniTouch" : "Touch");
        MR::deleteEffect(this, "Smoke");
    }
    MR::startDPDFreezeLevelSound(this);
    if (mFreezeReturnNerve == &NrvWanwan::WanwanNrvMove::sInstance) {
        if (MR::isOnGround(this)) {
            mVelocity.zero();
        }
        MR::addVelocityToGravityOrGround(this, 10.0f);
        MR::clampLength(&mVelocity, mVelocity, 40.0f);
    }
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(&NrvWanwan::WanwanNrvFreeze::sInstance);
    } else if (MR::isStep(this, 20)) {
        if (mFreezeReturnNerve == &NrvWanwan::WanwanNrvMove::sInstance) {
            f32 coord = MR::getRailCoord(this);
            MR::moveCoordToNearestPos(this, mPosition);
            if (MR::getRailCoord(this) < coord) {
                MR::setRailCoord(this, coord);
            }
        }
        mScaleCtrl->startAnim();
        MR::deleteEffect(this, mType == 1 ? "MiniTouch" : "Touch");
        setNerve(mFreezeReturnNerve);
    }
}

void Wanwan::exeAppearSign() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Appear");
    }
    MR::startActionSound(this, "AppearSign", -1, -1, -1);
    if (MR::isStep(this, 60)) {
        MR::showModel(this);
        MR::validateHitSensors(this);
        setNerve(&NrvWanwan::WanwanNrvAppear::sInstance);
    }
}

// NONMATCHING: isMini() lowers to cmpwi/bne instead of cntlzw bool + frsp copy of the start angle
void Wanwan::exeAppear() {
    f32 rate = 1386.0f / (mRadius * 3.1415927f);
    if (MR::isFirstStep(this)) {
        f32 start = isMini() ? MR::getRandomDegree() : 0.0f;
        mRotateOffs = start;
        updateRotateX(start - rate * (f32)mAppearStep);
    }
    if (MR::isLessStep(this, mAppearStep)) {
        moveOnRail(7.0f, rate, false);
        if (!mHideAtStart) {
            MR::startActionSound(this, "EmLvWanwanrollStandby", -1, -1, -1);
        }
    }
    if (MR::isStep(this, mAppearStep)) {
        MR::startActionSound(this, "EmWanwanrollStandbyEnd", -1, -1, -1);
    }
    if (MR::isGreaterStep(this, mAppearStep) && MR::isLessStep(this, mAppearStep + 15)) {
        s32 t = getNerveStep() - mAppearStep;
        f32 s = MR::sinDegree(MR::repeatDegree(100.0f * (f32)t - 0.0f));
        mRotation.x = ((f32)(15 - t) * (5.0f * s)) / 15.0f;
        updateRotateX(mRotation.x + mRotateOffs);
    }
    if (MR::isStep(this, mAppearStep + 19)) {
        setNerve(&NrvWanwan::WanwanNrvAppearMoveInvalidBind::sInstance);
    }
}

void Wanwan::exeAppearMoveInvalidBind() {
    bool force = isForceInvalidBindSection();
    if (!MR::isHiddenModel(this)) {
        MR::emitEffect(this, "Smoke");
        if (mType == 2) {
            MR::emitEffect(this, "Light");
        }
    }
    startRollLevelSound(MR::isOnGround(this));
    startSoundWanwanVoice();
    if (!tryFreeze(&NrvWanwan::WanwanNrvAppearMoveInvalidBind::sInstance)) {
        moveOnRail(mSpeed, mRotSpeed, force);
        mInvalidBindMoveStep++;
        if (!force && mMoveStep < mInvalidBindMoveStep) {
            MR::onBind(this);
            setNerve(&NrvWanwan::WanwanNrvMove::sInstance);
        }
    }
}

void Wanwan::exeMove() {
    if (MR::isOnGround(this)) {
        if (!MR::isHiddenModel(this)) {
            MR::emitEffect(this, "Smoke");
            if (mType == 2) {
                MR::emitEffect(this, "Light");
            }
        }
        if (mLandCount >= 20 && !MR::isHiddenModel(this)) {
            MR::emitEffect(this, "Land");
            rumblePadAndCamera();
        }
        if (mLandCount > 10) {
            MR::startActionSound(this, "EmWanwanrollBound", mLandCount, -1, -1);
        }
        mLandCount = 0;
    } else if (mLandCount < 20) {
        mLandCount++;
    } else {
        MR::deleteEffect(this, "Smoke");
    }
    startRollLevelSound(MR::isOnGround(this));
    startSoundWanwanVoice();
    if (MR::isOnGround(this)) {
        if (mHasBeenAirborne && mGravityAcceleration != 1.5f) {
            mGravityAcceleration = 1.5f;
        }
    } else if (!mHasBeenAirborne) {
        mHasBeenAirborne = true;
    }
    if (isBreakByWall()) {
        setNerve(&NrvWanwan::WanwanNrvBreak::sInstance);
    } else if (!tryBreakReachedGoal()) {
        move(mSpeed);
        f32 speed = mRotSpeed;
        updateRotateX(mRotation.x + speed);
        if (tryFreeze(&NrvWanwan::WanwanNrvMove::sInstance)) {
            return;
        }
    }
}

void Wanwan::exeMoveInvalidBind() {
    if (MR::isFirstStep(this)) {
        MR::moveCoordAndTransToNearestRailPos(this);
        mVelocity.zero();
        MR::offBind(this);
        MR::deleteEffect(this, "Smoke");
    }
    startRollLevelSound(MR::isOnGround(this));
    startSoundWanwanVoice();
    if (!tryBreakReachedGoal()) {
        moveOnRail(mSpeed, mRotSpeed, false);
    }
}

// NONMATCHING: vector temporary stack slots.
void Wanwan::exeMoveLava() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
        MR::offBind(this);
        MR::deleteEffect(this, "Smoke");
        if (mIsInLava) {
            MR::emitEffect(this, "AppearLava");
        }
    }
    startRollLevelSound(MR::isOnGround(this));
    startSoundWanwanVoice();
    if (!tryBreakReachedGoal()) {
        TVec3f hit;
        TVec3f a;
        TVec3f up;
        s32 prevNo = mRailPointNo;
        MR::moveCoordAndFollowTrans(this, 15.0f);
        f32 neg = -mRadius;
        TVec3f off(mGravity);
        off.scale(neg);
        JMathInlineVEC::PSVECAdd(&mPosition, &off, &mPosition);
        updateRotateX(mRotation.x + (15.0f * mRotSpeed) / mSpeed);
        mFront.set(MR::getRailDirection(this));
        if (mIsInLava) {
            Triangle tri;
            a.scale(1000.0f, mGravity);
            TVec3f from(mPosition);
            JMathInlineVEC::PSVECSubtract(&from, &a, &from);
            TVec3f dir(a);
            dir.scale(2.0f);
            bool found = MR::getFirstPolyOnLineToMap(&hit, &tri, from, dir);
            if (!found) {
                hit.set(mPosition);
            }
            up.negate(mGravity);
            MR::makeMtxUpFrontPos(&mLavaMtx, up, mFront, hit);
            MR::startActionSound(this, "AppearLava", -1, -1, -1);
            if (found && !MR::isGroundCodeDamageFire(&tri)) {
                mIsInLava = false;
            }
        }
        if (!tryFreeze(&NrvWanwan::WanwanNrvMoveLava::sInstance)) {
            mRailPointNo = MR::getCurrentRailPointNo(this);
            if (prevNo != mRailPointNo) {
                s32 arg;
                MR::getCurrentRailPointArg3WithInit(this, &arg);
                if (arg >= 0) {
                    MR::onBind(this);
                    setNerve(&NrvWanwan::WanwanNrvMove::sInstance);
                }
            }
        }
    }
}

void Wanwan::exeMoveToGoal() {
    if (MR::isFirstStep(this)) {
        MR::offBind(this);
        MR::deleteEffect(this, "Smoke");
    }
    MR::startActionSound(this, "MoveToGoal", -1, -1, -1);
    TVec3f dir;
    MR::calcDirToTarget(&dir, this, &mGoalPos);
    TVec3f v(dir);
    v.scale(2.8f);
    JMathInlineVEC::PSVECAdd(&mVelocity, &v, &mVelocity);
    mVelocity.x *= 0.86f;
    mVelocity.y *= 0.86f;
    mVelocity.z *= 0.86f;
    f32 mag = PSVECMag(&mVelocity);
    updateRotateX(mRotation.x + (mRotSpeed * mag) / mSpeed);
    if (MR::calcDistance(this, mGoalPos) < 40.0f) {
        mVelocity.zero();
        MR::setPosition(this, mGoalPos);
        setNerve(&NrvWanwan::WanwanNrvGoalIn::sInstance);
    }
}

void Wanwan::exeGoalIn() {
    if (MR::isFirstStep(this)) {
        mIsAtGoal = true;
        MR::startActionSound(this, "GoalIn", -1, -1, -1);
    }
    if (MR::isStep(this, 30)) {
        setNerve(&NrvWanwan::WanwanNrvBreak::sInstance);
    }
}

// NONMATCHING: Euler-angle fp register allocation (f0..f3)
void Wanwan::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mType != 2 && !mIsAtGoal && (mModel == nullptr || isInClippingRange())) {
            kill();
            return;
        }
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        mRotation.zero();
        mVelocity.zero();
        mFallVel.zero();
        MR::deleteEffectAll(this);
        MR::setPosition(mModel, mPosition);
        TPos3f mtx;
        calcBaseMtx(&mtx);
        f32 rnd = MR::getRandom(0.0f, 6.2831855f);
        TPos3f rot;
        rot.makeRotate(mGravity, rnd);
        mtx.concat(rot, mtx);
        TVec3f euler;
        mtx.getEulerXYZ(euler);
        mModel->mRotation.x = euler.x * 57.29578f;
        mModel->mRotation.y = euler.y * 57.29578f;
        mModel->mRotation.z = euler.z * 57.29578f;
        mModel->appear();
        MR::invalidateClipping(mModel);
        MR::startAction(mModel, "Break");
        MR::startActionSound(this, "EmWanwanrollExplosion", -1, -1, -1);
        if (mType == 0) {
            setBtkForEnvironmentMap(mModel, "WanwanRollingBreak");
        }
        rumblePadAndCamera();
        if (mIsAtGoal) {
            tryOnSwitchDead();
        }
    }
    bool star = false;
    if (mIsAtGoal && mType == 2) {
        star = true;
    }
    if (star) {
        tryRequestAppearPowerStar();
    }
    if (MR::isActionEnd(mModel)) {
        if (star && !mRequestedPowerStar) {
            requestAppearPowerStar();
        }
        mModel->kill();
        kill();
    }
}



Wanwan::~Wanwan() {
}

