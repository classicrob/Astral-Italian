#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/Enemy/Teresa.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"

#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Map/SunshadeMapHolder.hpp"
#include "Game/MapObj/DummyDisplayModel.hpp"
#include "Game/MapObj/KeySwitch.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"

namespace {
    static Color8 sPointLightColor(255, 255, 255, 255);
};  // namespace

namespace NrvTeresa {
    NEW_NERVE_ONEND(TeresaNrvAppearFromWall, Teresa, AppearFromWall, AppearFromWall);
    NEW_NERVE_ONEND(TeresaNrvAppearFromGround, Teresa, AppearFromGround, AppearFromGround);
    NEW_NERVE(TeresaNrvWait, Teresa, Wait);
    NEW_NERVE(TeresaNrvWalk, Teresa, Walk);
    NEW_NERVE(TeresaNrvRailWalk, Teresa, RailWalk);
    NEW_NERVE(TeresaNrvRailTurn, Teresa, RailTurn);
    NEW_NERVE(TeresaNrvLoveFind, Teresa, LoveFind);
    NEW_NERVE(TeresaNrvLoveChase, Teresa, LoveChase);
    NEW_NERVE(TeresaNrvLoveHit, Teresa, LoveHit);
    NEW_NERVE(TeresaNrvLoveEnd, Teresa, LoveEnd);
    NEW_NERVE(TeresaNrvChase, Teresa, Chase);
    NEW_NERVE(TeresaNrvShay, Teresa, Shay);
    NEW_NERVE(TeresaNrvAggressive, Teresa, Aggressive);
    NEW_NERVE(TeresaNrvAttackSuccess, Teresa, AttackSuccess);
    NEW_NERVE_ONEND(TeresaNrvDrift, Teresa, Drift, Drift);
    NEW_NERVE(TeresaNrvHideWall, Teresa, HideWall);
    NEW_NERVE(TeresaNrvHideWater, Teresa, HideWater);
    NEW_NERVE(TeresaNrvAscension, Teresa, Ascension);
    NEW_NERVE(TeresaNrvStop, Teresa, Stop);
    NEW_NERVE(TeresaNrvEaten, Teresa, Eaten);
    NEW_NERVE(TeresaNrvFreeze, Teresa, Freeze);
};  // namespace NrvTeresa

Teresa::Teresa(const char* pName)
    : LiveActor(pName), mDisplayModel(), mKeySwitch(), mClipAreaDrop(), mScaleController(), mBindStarPointer(), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mWallNormal(0, 0, 1), mWallHitPos(0, 0, 0), mHitNormal(0, 0, 1),
      mHomePos(0, 0, 0), mSquashScale(1, 1, 1), mTargetPos(0, 0, 0) {
    _EC = 3000.0f;
    mMoveDistance = 600.0f;
    mTransparency = 1.0f;
    mAppearanceType = -1;
    mHasCoin = 1;
    mIsChief = 0;
    mHitGhostThroughWall = 0;
    mLockOnTarget = nullptr;
}

void Teresa::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Teresa", false);
    initFromJMapParam(rIter);
    mHomePos.set(mPosition);
    mWallHitPos.set(mPosition);
    MR::makeQuatAndFrontFromRotate(&mPoseQuat, &mWallNormal, this);
    MR::setBindTriangleFilter(this, MR::createTriangleFilterDelegator(this, &Teresa::filterBind));
    initDummyModel(rIter);

    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToNearestRailPos(this);
    }

    initNerve(GET_NERVE(Teresa, TeresaNrvWait), 1);
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(Teresa, TeresaNrvFreeze), "SupportFreeze");
    setTransparency(1.0f);

    if (MR::isObjectName(rIter, "TeresaChief")) {
        MR::declarePowerStar(this);
        mHasCoin = 0;
        mIsChief = 1;
        MR::invalidateClipping(this);
        MR::setShadowDropLength(this, nullptr, 3000.0f);
    }

    if (mHasCoin) {
        MR::declareCoin(this, 1);
    }

    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
    setStartNerve();
}

void Teresa::initDummyModel(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        mDisplayModel = MR::createDummyDisplayModel(this, rIter, "SklRoot", -1,
                                                  TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
        if (mDisplayModel != nullptr) {
            mDisplayModel->kill();
            if (MR::getDummyDisplayModelId(rIter, -1) == 6) {
                mKeySwitch = new KeySwitch("鍵スイッチ");
                mKeySwitch->initKeySwitchByOwner(rIter);
                mHasCoin = 0;
            }
            MR::tryStartAllAnim(mDisplayModel, "InTeresa");
        }
    }
}

void Teresa::initFromJMapParam(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg0WithInit(rIter, &mAppearanceType);
        f32 dropRadius = -1.0f;
        MR::getJMapInfoArg1WithInit(rIter, &dropRadius);
        if (dropRadius >= 1.0f) {
            MR::createClipAreaDropHolder();
            mClipAreaDrop = MR::appearClipAreaDrop(mPosition, dropRadius);
        }
    }
}

void Teresa::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::onCalcGravity(this);
}

void Teresa::appear() {
    LiveActor::appear();
    setTransparency(0.0f);

    switch (mAppearanceType) {
    case 0:
        MR::invalidateClipping(this);
        MR::onBind(this);
        mWallHitPos.set(mPosition);
        setNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround));
        break;
    case 1:
        MR::invalidateClipping(this);
        MR::onBind(this);
        mWallHitPos.set(mPosition);
        setNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall));
        break;
    default:
        setStartNerve();
        break;
    }
}

void Teresa::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");

    if (mKeySwitch != nullptr) {
        mKeySwitch->appearKeySwitch(mPosition);
    }

    if (mIsChief) {
        MR::requestAppearPowerStar(this, mPosition);
    }

    if (mHasCoin) {
        MR::appearCoinPop(this, mPosition, 1);
    }
}

namespace {
    inline void blendTeresaQuat(TQuat4f* pQuat, const TVec3f& rGravity, const TVec3f& rFront) {
        MR::blendQuatUpFront(pQuat, -rGravity, rFront, 0.1f, 0.2f);
    }
};  // namespace

void Teresa::control() {
    mScaleController->updateNerve();
    if (!isNerve(GET_NERVE(Teresa, TeresaNrvDrift)) && !isNerve(GET_NERVE(Teresa, TeresaNrvAscension))) {
        mSquashScale.x = 1.0f + (0.95f * (mSquashScale.x - 1.0f));
        mSquashScale.z = 1.0f / mSquashScale.x;
    }

    tryHideWater();
    this->fn_8020E640();
    mHitGhostThroughWall = 0;

    if (mTransparency > 0.9f) {
        MR::requestPointLight(this, TVec3f(mPosition), ::sPointLightColor, 1.0f, -1);
    }

    if (mClipAreaDrop != nullptr) {
        MR::setPosition((LiveActor*)mClipAreaDrop, mPosition);
    }

    if (isEatable()) {
        YoshiFunction::tryYoshiLick(mLockOnTarget);
    }
}

void Teresa::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    TVec3f scale(mScale);
    scale.mul(mSquashScale);
    scale.mul(mScaleController->mScale);
    MR::setBaseScale(this, scale);
}

void Teresa::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (isFreeze()) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }
    } else {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }

    if (!requestAttack(pSender, pReceiver)) {
        if (MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool Teresa::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isSensorEnemy(pSender)) {
        TVec3f v9 = *MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender);
        MR::normalizeOrZero(&v9);
        mVelocity.add(v9 * 0.1f);
        return true;
    }

    return false;
}

bool Teresa::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSearchlightAttack(msg) || MR::isMsgFireBallAttack(msg) || MR::isMsgInvincibleAttack(msg)) {
        return requestSearchLightDead();
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return isEnableStarPieceStop();
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        if (isEnableStarPieceStop()) {
            setNerve(GET_NERVE(Teresa, TeresaNrvStop));
        }
    }

    return false;
}

bool Teresa::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::tryForceKillIfMsgStartPowerStarGet(this, msg)) {
        return true;
    }

    if (MR::isMsgSpinStormRange(msg)) {
        return requestDrift();
    }

    if (msg == 77 /* TERESA_PLAYER_TOUCH */) {
        return requestLoveHit(pSender, pReceiver);
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvStop));
        return false;
    }

    return false;
}

bool Teresa::filterBind(const Triangle* pTriangle) {
    if (MR::isSensorType(pTriangle->mSensor, 67 /* CLIP_FIELD_MAP_PARTS */)) {
        return true;
    }

    if (MR::isWallCodeGhostThrough(pTriangle)) {
        mHitGhostThroughWall = 1;
        mHitNormal.set(*pTriangle->getNormal(0));
        return isNerve(GET_NERVE(Teresa, TeresaNrvDrift));
    }

    return false;
}

bool Teresa::requestAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemyAttack(pSender) || !MR::isSensorPlayer(pReceiver)) {
        return false;
    }

    if (!canAttack()) {
        return false;
    }

    if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvAttackSuccess));
        return true;
    }

    return false;
}

bool Teresa::requestDrift() {
    if (canDrift()) {
        setNerve(GET_NERVE(Teresa, TeresaNrvDrift));
        return true;
    }

    return false;
}

bool Teresa::requestLoveHit(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvLoveChase))) {
        MR::addVelocitySeparateHV(this, pSender, pReceiver, 20.0f, 0.0f);
        setNerve(GET_NERVE(Teresa, TeresaNrvLoveHit));
        return true;
    }

    return false;
}

bool Teresa::requestSearchLightDead() {
    if (canSearchLightDead()) {
        setNerve(GET_NERVE(Teresa, TeresaNrvAscension));
        return true;
    }

    return false;
}

void Teresa::setStartNerve() {
    if (MR::isExistRail(this)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvRailWalk));
        setTransparency(1.0f);
        MR::startAction(this, "Wait");
        MR::moveCoordToNearestPos(this, mPosition);
    } else {
        setNerve(GET_NERVE(Teresa, TeresaNrvWait));
    }
}

bool Teresa::tryAppearFromWallEnd() {
    if (MR::isActionEnd(this)) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryAppearFromGroundEnd() {
    if (MR::isActionEnd(this)) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryRailTurn() {
    if (MR::isRailReachedGoal(this)) {
        MR::reverseRailDirection(this);
        setNerve(GET_NERVE(Teresa, TeresaNrvRailTurn));
        return true;
    }

    return false;
}

bool Teresa::tryRailTurnEnd() {
    if (MR::isGreaterStep(this, 150)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvRailWalk));
        return true;
    }

    return false;
}

bool Teresa::tryWalk() {
    if (MR::isGreaterStep(this, 192)) {
        TVec3f v6;
        MR::getRandomVector(&v6, 1.0f);
        MR::normalizeOrZero(&v6);
        v6.orthogonalize(mGravity);
        v6 *= mMoveDistance;
        mTargetPos = v6 + mHomePos;
        setNerve(GET_NERVE(Teresa, TeresaNrvWalk));
        return true;
    }

    return false;
}

bool Teresa::tryWalkEnd() {
    if (MR::isGreaterStep(this, 300) || mTargetPos.distance(mPosition) <= 200.0f) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryChase() {
    if (MR::isPlayerElementModeTeresa()) {
        return false;
    }

    bool isShayCond = false;
    if (!MR::isPlayerSquat() && isShay() && MR::isNearPlayer(this, 1500.0f)) {
        isShayCond = true;
    }

    if (!isShayCond && MR::isNearPlayer(this, 1800.0f)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvChase));
        return true;
    }

    return false;
}

bool Teresa::tryCheseEnd() {
    if (MR::isGreaterStep(this, 240) || MR::isPlayerElementModeTeresa() || isNotNear()) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryFreeze() {
    if (mBindStarPointer->tryStartPointBind()) {
        setNerve(GET_NERVE(Teresa, TeresaNrvFreeze));
        return true;
    }

    return false;
}

bool Teresa::tryShay() {
    if (MR::isPlayerElementModeTeresa()) {
        return false;
    }

    bool isShayCond = false;
    if (!MR::isPlayerSquat() && isShay() && MR::isNearPlayer(this, 1500.0f)) {
        isShayCond = true;
    }

    if (isShayCond) {
        setNerve(GET_NERVE(Teresa, TeresaNrvShay));
        return true;
    }

    return false;
}

bool Teresa::tryShayEnd() {
    if (MR::isPlayerElementModeTeresa()) {
        setStartNerve();
        return true;
    }

    bool isShayCond = false;
    if (!MR::isPlayerSquat() && isShay() && MR::isNearPlayer(this, 1500.0f)) {
        isShayCond = true;
    }

    if (!isShayCond) {
        if (MR::isExistRail(this)) {
            setStartNerve();
        } else {
            setNerve(GET_NERVE(Teresa, TeresaNrvAggressive));
        }

        return true;
    } else {
        return false;
    }
}

bool Teresa::tryLoveFind() {
    if (MR::isPlayerElementModeTeresa() && MR::isNearPlayerAnyTime(this, 1500.0f)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvLoveFind));
        return true;
    }

    return false;
}

bool Teresa::tryLoveEnd() {
    if (!MR::isPlayerElementModeTeresa() && MR::isNearPlayerAnyTime(this, 1500.0f)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvLoveEnd));
        return true;
    }

    return false;
}

bool Teresa::tryLoveFindEnd() {
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvLoveChase));
        return true;
    }

    return false;
}

bool Teresa::tryLoveChaseEnd() {
    if (MR::isGreaterStep(this, 240)) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryLoveHitEnd() {
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvLoveChase));
        return true;
    }

    return false;
}

bool Teresa::tryAggressiveEnd() {
    if (MR::isGreaterStep(this, 75)) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryAttackSuccessEnd() {
    if (MR::isActionEnd(this)) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryDriftEnd() {
    if (MR::isGreaterStep(this, 50)) {
        setStartNerve();
        return true;
    }

    return false;
}

bool Teresa::tryHideWater() {
    TVec3f v4;
    v4.scaleAdd(70.0f * mScale.y, mGravity, mPosition);

    if (isCheckWater() && MR::isInWater(v4)) {
        TPos3f effectMtx;
        MR::makeMtxFrontNoSupportPos(&effectMtx, -mGravity, v4);
        MR::emitEffectHit(this, effectMtx, "HideWall");
        MR::invalidateClipping(this);
        MR::invalidateHitSensors(this);
        MR::offBind(this);
        MR::hideModel(this);
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Teresa, TeresaNrvHideWater));
        return true;
    }

    return false;
}

bool Teresa::tryHideWall() {
    if (MR::isBindedWall(this)) {
        mWallNormal.set(MR::getWallNormal(this));
        mWallHitPos.set(MR::getWallHitPos(this));
        TPos3f frontMtx;
        MR::makeMtxFrontNoSupportPos(&frontMtx, mWallNormal, mWallHitPos);
        MR::emitEffectHit(this, frontMtx, "HideWall");
        MR::invalidateClipping(this);
        MR::invalidateHitSensors(this);
        MR::offBind(this);
        MR::hideModel(this);
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Teresa, TeresaNrvHideWall));
        return true;
    }

    return false;
}

bool Teresa::tryHideWallEnd() {
    if (MR::isGreaterStep(this, 60)) {
        MR::validateHitSensors(this);
        MR::onBind(this);
        MR::showModel(this);
        setNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall));
        return true;
    }

    return false;
}

bool Teresa::tryAscension() {
    if (MR::isInAreaObj("GlaringLightArea", mPosition) || !MR::isInShadeFromTheSun(mPosition, 10000.0f)) {
        setNerve(GET_NERVE(Teresa, TeresaNrvAscension));
        return true;
    }

    return false;
}

void Teresa::endAppear() {
    if (!mIsChief) {
        MR::validateClipping(this);
    }

    MR::startBrk(this, "Erase");
    setTransparency(1.0f);
}

void Teresa::endDrift() {
    MR::forceDeleteEffect(this, "SpinBlur");
    setTransparency(1.0f);
    MR::setShadowVolumeSphereRadius(this, nullptr, 80.0f);
}

void Teresa::exeAppearFromWall() {
    TVec3f v4;
    mPoseQuat.getZDir(v4);

    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "AppearWall");
        MR::zeroVelocity(this);
        TPos3f frontMtx;
        MR::makeMtxFrontNoSupportPos(&frontMtx, v4, mWallHitPos);
        MR::emitEffectHit(this, frontMtx, "Appear");
        MR::startActionSound(this, "EmTeresaAppear");
    }

    TVec3f v3(v4);
    v3 *= 0.2f;
    mVelocity.add(v3);
    MR::attenuateVelocity(this, 0.9f);
    f32 rate = MR::calcNerveRate(this, 60);
    MR::setShadowVolumeSphereRadius(this, 0, (80.0f * (1.0f - (1.0f - rate) * (1.0f - rate))));
    if (tryAppearFromWallEnd()) {
        return;
    }
}

void Teresa::exeAppearFromGround() {
    TVec3f v4;
    mPoseQuat.getYDir(v4);

    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::startAction(this, "AppearGround");
        MR::zeroVelocity(this);
        TPos3f frontMtx;
        MR::makeMtxFrontNoSupportPos(&frontMtx, v4, mWallHitPos);
        MR::emitEffectHit(this, frontMtx, "Appear");
        MR::startActionSound(this, "EmTeresaAppear");
    }

    TVec3f v3(v4);
    v3 *= 0.2f;
    mVelocity.add(v3);
    MR::attenuateVelocity(this, 0.9f);
    f32 rate = MR::calcNerveRate(this, 60);
    MR::setShadowVolumeSphereRadius(this, 0, (80.0f * (1.0f - (1.0f - rate) * (1.0f - rate))));
    if (tryAppearFromGroundEnd()) {
        return;
    }
}

void Teresa::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::startActionSound(this, "EmLvTeresaMove");
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze() && !tryChase() && !tryWalk()) {
        if (tryLoveFind()) {
            return;
        }
    }
}

void Teresa::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startBtp(this, "Normal");
    }

    MR::turnDirectionToTarget(this, &mWallNormal, mTargetPos, 0.9995f);
    MR::addVelocityMoveToDirection(this, mWallNormal, 0.2f);
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze() && !tryWalkEnd()) {
        if (tryLoveFind()) {
            return;
        }
    }
}

void Teresa::exeRailWalk() {
    TVec3f v7;
    v7.killElement(MR::getRailPos(this) - mPosition, mGravity);

    if (v7.length() < 10.0f) {
        MR::moveCoord(this, 10.0f);
    }

    MR::turnDirectionToTarget(this, &mWallNormal, MR::getRailPos(this), 0.9995f);
    MR::addVelocityMoveToTarget(this, MR::getRailPos(this), 0.35f);
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze() && !tryRailTurn()) {
        if (tryLoveFind()) {
            return;
        }
    }
}

void Teresa::exeRailTurn() {
    if (MR::isGreaterStep(this, 60)) {
        MR::turnDirection(this, &mWallNormal, MR::getRailDirection(this), 0.9997f);
    }

    MR::addVelocityMoveToTarget(this, MR::getRailPos(this), 0.1f);
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze() && !tryRailTurnEnd()) {
        if (tryLoveFind()) {
            return;
        }
    }
}

void Teresa::exeLoveFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LoveFind");
        MR::startActionSound(this, "EmTeresaLoveFind");
        MR::startActionSound(this, "EvTeresaLaugh");
    }

    MR::turnDirectionToTarget(this, &mWallNormal, *MR::getPlayerPos(), 0.98f);
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze() && !tryLoveFindEnd()) {
        if (tryLoveEnd()) {
            return;
        }
    }
}

void Teresa::exeLoveChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LoveChase");
    }

    MR::turnDirectionToTarget(this, &mWallNormal, *MR::getPlayerPos(), 0.999f);
    MR::addVelocityMoveToDirection(this, mWallNormal, (0.75f * MR::calcNerveRate(this, 20)));
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze() && !tryLoveEnd()) {
        if (tryLoveChaseEnd()) {
            return;
        }
    }
}

void Teresa::exeLoveHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LoveHit");
        MR::startActionSound(this, "EmTeresaLoveHit");
        MR::startActionSound(this, "EvTeresaLoveHit");
    }

    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze()) {
        if (tryLoveHitEnd()) {
            return;
        }
    }
}

void Teresa::exeLoveEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LoveEnd");
    }

    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze()) {
        if (tryAggressiveEnd()) {
            return;
        }
    }
}

void Teresa::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Chase");
        MR::startActionSound(this, "EvTeresaLaugh");
    }

    MR::startActionSound(this, "EmLvTeresaMove");
    MR::turnDirectionToTarget(this, &mWallNormal, *MR::getPlayerPos(), 0.999f);
    MR::addVelocityMoveToDirection(this, mWallNormal, (0.75f * MR::calcNerveRate(this, 20)));
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryShay() && !tryFreeze()) {
        if (tryCheseEnd()) {
            return;
        }
    }
}

void Teresa::exeShay() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shay");
        MR::startActionSound(this, "EmTeresaStop");
        MR::zeroVelocity(this);
    }

    if (mIsChief) {
        MR::startActionSound(this, "OjLvPowStarExist");
    }

    this->fn_8020E910();
    updateNormalVelocity();

    if (!tryAscension() && !tryFreeze()) {
        if (tryShayEnd()) {
            return;
        }
    }
}

void Teresa::exeAggressive() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Aggressive");
    }

    MR::startActionSound(this, "EmLvTeresaMove");
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryFreeze() && !tryShay()) {
        if (tryAggressiveEnd()) {
            return;
        }
    }
}

void Teresa::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Laugh");
        MR::setShadowVolumeSphereRadius(this, nullptr, 80.0f);
        MR::startActionSound(this, "EvTeresaAttackSuccess");
    }

    MR::turnDirectionToTarget(this, &mWallNormal, *MR::getPlayerPos(), 0.99f);
    updateNormalTransparency();
    updateNormalVelocity();

    if (!tryAscension() && !tryFreeze()) {
        if (tryAttackSuccessEnd()) {
            return;
        }
    }
}

void Teresa::exeDrift() {
    if (MR::isFirstStep(this)) {
        MR::forceDeleteEffectAll(this);
        MR::startAction(this, "Drift");
        MR::emitEffect(this, "SpinBlur");
        mSquashScale.x = 1.0f;
        mSquashScale.y = 1.0f;
        mSquashScale.z = 1.0f;
        MR::startActionSound(this, "EmTeresaTornade");

        if (mDisplayModel != nullptr) {
            mDisplayModel->appear();
        }
    }

    MR::turnDirectionToTarget(this, &mWallNormal, *MR::getPlayerPos(), 0.96f);
    addDriftVelocity();
    updateNormalVelocity();
    updateDriftAnimScale();
    updateDriftTransparency();

    if (!tryAscension() && !tryHideWall()) {
        if (tryDriftEnd()) {
            return;
        }
    }
}

void Teresa::exeHideWall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::setShadowVolumeSphereRadius(this, nullptr, 0.0f);
    }

    if (tryHideWallEnd()) {
        return;
    }
}

void Teresa::exeHideWater() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::setShadowVolumeSphereRadius(this, nullptr, 0.0f);
        mWallHitPos.set(mHomePos);
    }

    if (MR::isGreaterStep(this, 60)) {
        MR::validateHitSensors(this);
        MR::onBind(this);
        MR::showModel(this);
        setNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround));
    }
}

void Teresa::exeAscension() {
    if (MR::isFirstStep(this)) {
        mSquashScale.x = 1.0f;
        mSquashScale.y = 1.0f;
        mSquashScale.z = 1.0f;
        MR::setBaseScale(this, mScale);
        MR::startActionSound(this, "EmTeresaDead");
        MR::startActionSound(this, "EvTeresaDead");
        kill();
    }
}

void Teresa::exeStop() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Wait");
        setTransparency(0.0f);
    }

    if (!tryAscension()) {
        if (MR::isGreaterStep(this, 60)) {
            setStartNerve();
        }
    }
}

inline void Teresa::exeFreeze() {
    if (MR::updateActorState(this)) {
        setStartNerve();
    }
}

inline void Teresa::exeEaten() {
}

inline void Teresa::endAppearFromGround() {
    endAppear();
}

inline void Teresa::endAppearFromWall() {
    endAppear();
}

void Teresa::fn_8020E640() {
    TVec3f up;
    up.negate(this->mGravity);
    MR::blendQuatUpFront(&this->mPoseQuat, up, this->mWallNormal, 0.1f, 0.2f);
}

void Teresa::updateNormalVelocity() {
    MR::addVelocityKeepHeightUseShadow(this, 150.0f + (85.0f * (mScale.y - 1.0f)), 0.40f, 40.0f, nullptr);
    MR::reboundVelocityFromCollision(this, -0.2f, 4.0f, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
}

void Teresa::addDriftVelocity() {
    if (MR::isLessEqualStep(this, 35)) {
        TVec3f v5;
        MR::calcPlayerSpinPullVelocity(&v5, mPosition);
        TVec3f v4(v5);
        v4 *= 0.12f;
        mVelocity.add(v4);
    }

    MR::fn_80011180(this, *MR::getPlayerPos(), -1.25f);
}

void Teresa::updateDriftAnimScale() {
    TVec3f v4;
    mPoseQuat.getXDir(v4);
    f32 v2 = v4.dot(mVelocity);
    mSquashScale.x = 1.0f + 0.5f * MR::normalize(MR::abs(v2), 3.0f, 53.0f);
    mSquashScale.z = 1.0f / mSquashScale.x;
}

void Teresa::updateDriftTransparency() {
    f32 v2 = MR::calcNerveRate(this, 50);
    f32 v3 = ((4.0f * v2) * (v2 - 1.0f));
    f32 v4 = (1.0f - (v3 * v3));
    f32 v5 = MR::calcNerveValue(this, 25, mTransparency, v4);
    setTransparency(v5);
    MR::setShadowVolumeSphereRadius(this, nullptr, (80.0f * v4));
}

void Teresa::updateNormalTransparency() {
    if (mTransparency < 1.0f) {
        mTransparency += 0.02f;
        f32 v1 = mTransparency;

        if (v1 > 1.0f) {
            mTransparency = 1.0f;
        }
    }

    setTransparency(mTransparency);
}

void Teresa::fn_8020E910() {
    if (this->mDisplayModel != nullptr) {
        this->setTransparency(MR::calcNerveEaseInOutValue(this, 60, this->mTransparency, 0.5f));
    } else {
        this->updateNormalTransparency();
    }
}

void Teresa::setTransparency(f32 val) {
    MR::setBrkFrameAndStop(this, (1.0f - val) * MR::getBrkFrameMax(this));

    if (val < 1.0f) {
        if (mDisplayModel != nullptr) {
            mDisplayModel->appear();
        }
    } else {
        if (mDisplayModel != nullptr) {
            mDisplayModel->kill();
        }
    }

    mTransparency = val;
}

bool Teresa::canAttack() const {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvAttackSuccess)) || isNerve(GET_NERVE(Teresa, TeresaNrvDrift)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvAscension)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvEaten)) || isNerve(GET_NERVE(Teresa, TeresaNrvFreeze))) {
        return false;
    }

    return true;
}

bool Teresa::canDrift() const {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvDrift)) || isNerve(GET_NERVE(Teresa, TeresaNrvAscension)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvEaten))) {
        return false;
    }

    return true;
}

bool Teresa::canSearchLightDead() const {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvHideWater)) || isNerve(GET_NERVE(Teresa, TeresaNrvHideWall)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvAscension)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvEaten))) {
        return false;
    }

    return true;
}

bool Teresa::isEnableStarPieceStop() const {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvDrift)) || isNerve(GET_NERVE(Teresa, TeresaNrvHideWater)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvHideWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAscension)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvStop)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvEaten))) {
        return false;
    }

    return true;
}

bool Teresa::isCheckWater() const {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvHideWater)) || isNerve(GET_NERVE(Teresa, TeresaNrvHideWall)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvAscension)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvEaten))) {
        return false;
    }

    return true;
}

bool Teresa::isEatable() const {
    if (isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAppearFromGround)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvDrift)) || isNerve(GET_NERVE(Teresa, TeresaNrvHideWater)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvHideWall)) || isNerve(GET_NERVE(Teresa, TeresaNrvAscension)) ||
        isNerve(GET_NERVE(Teresa, TeresaNrvStop)) || isNerve(GET_NERVE(Teresa, TeresaNrvEaten))) {
        return false;
    }

    return true;
}

bool Teresa::isFreeze() const {
    return isNerve(GET_NERVE(Teresa, TeresaNrvFreeze));
}

namespace {
    inline bool isFacingTeresa(const TVec3f& rPosition) {
        TVec3f playerFront;
        MR::getPlayerFrontVec(&playerFront);

        return playerFront.dot(rPosition - *MR::getPlayerPos()) > 0.0f;
    }
};  // namespace

bool Teresa::isShay() const {
    TVec3f playerFront;
    MR::getPlayerFrontVec(&playerFront);
    TVec3f toPlayer;
    MR::calcVecToPlayerH(&toPlayer, this, nullptr);
    toPlayer.negate();
    return MR::diffAngleAbs(playerFront, toPlayer) < 0.87266463f;
}

