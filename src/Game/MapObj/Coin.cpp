#include "Game/MapObj/Coin.hpp"
#include "Game/LiveActor/FlashingCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/MapObj/CoinHolder.hpp"
#include "Game/MapObj/CoinRotater.hpp"
#include "Game/MapObj/PurpleCoinHolder.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/DrumTrampoline.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include <JSystem/JMath/JMath.hpp>

// PLACEHOLDER(objects): unnamed target symbol fn_800145C0, declared extern "C" so the reloc name matches; resolve when fn_800145C0 gets its real name in symbols.txt
extern "C" void fn_800145C0(LiveActor*);
// PLACEHOLDER(objects): unnamed target symbol fn_80014670, declared extern "C" so the reloc name matches; resolve when fn_80014670 gets its real name in symbols.txt
extern "C" void fn_80014670(LiveActor*);

namespace NrvCoin {
    NEW_NERVE(CoinNrvNonActive, Coin, NonActive);
    NEW_NERVE(CoinNrvFix, Coin, Fix);
    NEW_NERVE(CoinNrvFixHide, Coin, FixHide);
    NEW_NERVE(CoinNrvFixTimer, Coin, FixTimer);
    NEW_NERVE(CoinNrvControled, Coin, Controled);
    NEW_NERVE(CoinNrvSpinDrained, Coin, SpinDrained);
    NEW_NERVE(CoinNrvMove, Coin, Move);
    NEW_NERVE(CoinNrvHop, Coin, Hop);
    NEW_NERVE(CoinNrvSupportTicoThrow, Coin, SupportTicoThrow);
    NEW_NERVE(CoinNrvGot, Coin, Got);
};  // namespace NrvCoin

Coin::Coin(const char* pName) : LiveActor(pName) {
    mCoinHostInfo = nullptr;
    mFlashingCtrl = nullptr;
    mAirBubble = nullptr;
    mConnector = nullptr;
    mShadowDropPos.x = 0.0f;
    mShadowDropPos.y = 0.0f;
    mShadowDropPos.z = 0.0f;
    mClippingRange.x = 0.0f;
    mClippingRange.y = 0.0f;
    mClippingRange.z = 0.0f;
    mLifeTime = 600;
    mCannotTime = 0;
    mShadowType = -1;
    mInWater = false;
    mShadowCalcOn = false;
    mIgnoreGravity = false;
    mIsPurple = false;
    mIsInBubble = false;
}

Coin::~Coin() {
}

void Coin::init(const JMapInfoIter& rIter) {
    MR::createPlayerFollowPass();
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, mIsPurple ? "PurpleCoin" : "Coin", false);
    MR::connectToSceneItemStrongLight(this);
    initHitSensor(1);

    f32 scale = mScale.x;
    MR::addHitSensor(this, "coin", 0x44 /* ATYPE_COIN */, 4, 55.0f * scale, TVec3f(0.0f, 70.0f * scale, 0.0f));
    initBinder(55.0f * scale, 70.0f * scale, 0);
    MR::initStarPointerTarget(this, 55.0f * scale, TVec3f(0.0f, 70.0f * scale, 0.0f));
    MR::setClippingFar100m(this);
    initEffectKeeper(0, nullptr, false);
    initSound(1, "Coin", nullptr, TVec3f(0.0f));

    if (MR::isValidInfo(rIter)) {
        setShadowAndPoseModeFromJMapIter(rIter);
    }

    if (mIgnoreGravity) {
        TVec3f axis;
        MR::calcActorAxisY(&axis, this);
        mGravity.set(-axis);
    }

    initShadow(rIter);

    if (mIsInBubble) {
        mAirBubble = MR::createPartsModelNoSilhouettedMapObj(this, "アワ", "AirBubble", nullptr);
        mAirBubble->initFixedPosition(TVec3f(0.0f, 70.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), nullptr);
        mAirBubble->makeActorDead();
        MR::registerDemoSimpleCastAll(mAirBubble);
        MR::startAction(mAirBubble, "Move");
    }

    mFlashingCtrl = new FlashingCtrl(this, true);
    MR::tryCreateMirrorActor(this, mIsPurple ? "PurpleCoin" : "Coin");
    initNerve(GET_NERVE(Coin, CoinNrvFix));

    if (MR::isValidInfo(rIter)) {
        MR::setGroupClipping(this, rIter, 0x40);
    }

    if (MR::isValidInfo(rIter)) {
        if (!MR::useStageSwitchSyncAppear(this, rIter)) {
            appearFixInit();
        }
    }

    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOnB(this, MR::FunctorV0M< Coin*, void (Coin::*)() >(this, &Coin::makeActorDead));
    } else {
        makeActorDead();
    }

    MR::useStageSwitchAwake(this, rIter);
}

void Coin::initAfterPlacement() {
    if (mConnector != nullptr) {
        mConnector->attachToUnder();
    }

    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 150.0f);
}

void Coin::initShadow(const JMapInfoIter& rIter) {
    f32 arg_5 = -1.0f;
    s32 arg_6 = -1;

    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg5NoInit(rIter, &arg_5);
        MR::getJMapInfoArg6NoInit(rIter, &arg_6);
    }

    switch (arg_6) {
    case 0:
        MR::initShadowSurfaceCircle(this, 50.0f);
        break;
    case 1:
        MR::initShadowVolumeCylinder(this, 50.0f);
        break;
    default:
        MR::initShadowVolumeSphere(this, 50.0f);
        break;
    }

    MR::setShadowDropPositionPtr(this, nullptr, &mShadowDropPos);

    if (arg_5 > 0.0f) {
        MR::setShadowDropLength(this, nullptr, arg_5);
    }
}

void Coin::setShadowAndPoseModeFromJMapIter(const JMapInfoIter& rIter) {
    bool arg_3 = false;
    MR::getJMapInfoArg3WithInit(rIter, &arg_3);
    mShadowCalcOn = arg_3;

    s32 arg_4 = -1;
    MR::getJMapInfoArg4WithInit(rIter, &arg_4);

    switch (arg_4) {
    case 0:
        mIgnoreGravity = true;
        mCalcShadowPrivateGravity = true;
        break;
    case 1:
        mIgnoreGravity = true;
        mConnector = new MapObjConnector(this);
        mCalcShadowPrivateGravity = false;
        break;
    default:
        mIgnoreGravity = false;
        mCalcShadowPrivateGravity = true;
        break;
    }

    mIsInBubble = Coin::isNeedBubble(rIter);
}

void Coin::makeActorAppeared() {
    if (!isNerve(GET_NERVE(Coin, CoinNrvGot))) {
        MR::offCalcGravity(this);

        if (!mIgnoreGravity) {
            MR::calcGravityOrZero(this);
        }
    }

    if (!mIsPurple && MR::isGalaxyDarkCometAppearInCurrentStage()) {
        LiveActor::makeActorDead();
    } else {
        MR::showModel(this);
        LiveActor::makeActorAppeared();

        if (mIsInBubble) {
            mAirBubble->makeActorAppeared();
            MR::setSensorRadius(this, "coin", 150.0f * mScale.x);
        } else {
            MR::setSensorRadius(this, "coin", 55.0f * mScale.x);
        }

        mFlashingCtrl->end();
        setCalcShadowMode();
        MR::validateShadow(this, nullptr);

        if (!mIsPurple) {
            mCoinHostInfo->_8++;
        }

        mInWater = MR::isInWater(mPosition);
    }
}

void Coin::makeActorDead() {
    LiveActor::makeActorDead();

    if (mAirBubble != nullptr) {
        mAirBubble->makeActorDead();
    }

    if (isNerve(GET_NERVE(Coin, CoinNrvControled)) || isNerve(GET_NERVE(Coin, CoinNrvSpinDrained)) || isNerve(GET_NERVE(Coin, CoinNrvSupportTicoThrow))) {
        setNerve(GET_NERVE(Coin, CoinNrvGot));
    }
}

void Coin::calcAndSetBaseMtx() {
    TVec3f another_vec;
    another_vec.set(MR::isNearZero(mGravity) ? TVec3f(0.0f, 1.0f, 0.0f) : -mGravity);
    mShadowDropPos.scaleAdd(70.0f, another_vec, mPosition);

    TPos3f pos;

    if (mIgnoreGravity) {
        MR::makeMtxTR(pos.toMtxPtr(), this);
    } else {
        MR::makeMtxUpNoSupportPos(&pos, another_vec, mPosition);
    }

    if (isNerve(GET_NERVE(Coin, CoinNrvSpinDrained))) {
        pos.concat(pos, MR::getCoinHiSpeedRotateYMatrix());
    } else {
        if (mInWater) {
            pos.concat(pos, MR::getCoinInWaterRotateYMatrix());
        } else {
            pos.concat(pos, MR::getCoinRotateYMatrix());
        }
    }

    MR::setBaseTRMtx(this, pos);
}

bool Coin::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool result;

    if (MR::isMsgItemGet(msg)) {
        noticeGetCoin();
        MR::emitEffect(this, "CoinGet");
        mFlashingCtrl->end();
        kill();
        result = true;
    } else if (MR::isMsgItemPull(msg)) {
        result = requestGetCoin();
    } else if (MR::isMsgItemShow(msg)) {
        result = requestShow();
    } else if (MR::isMsgItemHide(msg)) {
        result = requestHide();
    } else if (MR::isMsgItemStartMove(msg)) {
        result = requestStartControl();
    } else if (MR::isMsgItemEndMove(msg)) {
        result = requestEndControl();
    } else if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        result = true;
    } else {
        result = false;
    }

    return result;
}

void Coin::exeNonActive() {
    if (MR::isFirstStep(this) || mShadowCalcOn) {
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, 150.0f);
    }

    if (mConnector != nullptr) {
        TVec3f up;
        MR::calcUpVec(&up, this);
        mGravity.set(-up);
    }
}

void Coin::exeFix() {
    if (MR::isFirstStep(this) || mShadowCalcOn) {
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, 150.0f);
    }

    if (mConnector != nullptr) {
        TVec3f up;
        MR::calcUpVec(&up, this);
        mGravity.set(-up);
    }

    MR::attachSupportTicoToTarget(this);
}

void Coin::exeFixTimer() {
    if (MR::isFirstStep(this)) {
        mFlashingCtrl->start(mLifeTime);
        MR::invalidateClipping(this);
        MR::offBind(this);
    }

    if (MR::isStep(this, mCannotTime)) {
        MR::validateHitSensors(this);
    }

    if (mFlashingCtrl->mIsEnded) {
        kill();
    }
}

void Coin::exeMove() {
    if (MR::isFirstStep(this)) {
        mFlashingCtrl->start(mLifeTime);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
    }

    if (MR::isStep(this, mCannotTime)) {
        MR::validateHitSensors(this);
    }

    if (MR::isInWater(mPosition)) {
        if (!mInWater) {
            MR::emitEffect(this, "WaterFall");
        }
        mInWater = true;
    } else {
        mInWater = false;
    }

    if (MR::isGreaterStep(this, mCannotTime) && MR::isBindedGroundDamageFire(this)) {
        MR::emitEffect(this, "LavaFall");
        MR::startActionSound(this, "OjFallInMagmaS", -1, -1, -1);
        kill();
        return;
    }

    if (MR::isBindedGroundSinkDeath(this)) {
        MR::emitEffect(this, "SandFall");
        kill();
        return;
    }

    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        kill();
        return;
    }

    TVec3f sensorDir;
    if (DrumTrampoline::tryGetGroundNormal(&sensorDir, this, getSensor(nullptr))) {
        TVec3f kick = sensorDir * 40.0f;
        MR::setVelocity(this, kick);
    }

    MR::calcGravityOrZero(this);

    if (MR::isPressedRoofAndGround(this)) {
        MR::offBind(this);
        MR::zeroVelocity(this);
    }

    if (!MR::isNoBind(this)) {
        calcRebouond();
        f32 rate = mInWater ? 0.3f : 1.0f;
        MR::addVelocityToGravity(this, rate);
        attenuateVelocity();
        MR::attenuateVelocity(this, rate);
    }

    if (mFlashingCtrl->mIsEnded) {
        kill();
    } else {
        fn_800145C0(this);
        fn_80014670(this);
        MR::attachSupportTicoToTarget(this);
    }
}

void Coin::exeControled() {
    if (MR::isFirstStep(this)) {
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
    }

    MR::zeroVelocity(this);
}

void Coin::exeSpinDrained() {
    if (MR::isFirstStep(this)) {
        MR::onCalcShadow(this, nullptr);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
    }

    MR::emitEffect(this, "CoinSpinBlur");
}

void Coin::exeHop() {
    if (MR::isFirstStep(this)) {
        noticeGetCoin();
    }

    mVelocity += mGravity;

    if (MR::isGreaterStep(this, 30)) {
        MR::emitEffect(this, "CoinGet");
        mFlashingCtrl->end();
        kill();
    }
}

void Coin::exeSupportTicoThrow() {
    if (MR::isFirstStep(this)) {
        MR::onCalcShadow(this, nullptr);
        MR::offBind(this);
        MR::onCalcShadowDropPrivateGravity(this, nullptr);
        MR::invalidateHitSensors(this);
        MR::zeroVelocity(this);
        MR::offCalcGravity(this);
        mGravity = *MR::getPlayerGravity();
        _B8 = mPosition;
    }

    MR::emitEffect(this, "CoinSpinBlur");

    TVec3f center(*MR::getPlayerCenterPos());
    f32 rate = MR::calcNerveEaseOutRate(this, 20);
    MR::vecBlend(&mPosition, _B8, center, rate);

    f32 parabola = MR::calcNerveParabolaValue(this, 30) * 250.0f;
    mPosition += (-mGravity) * parabola;

    if (MR::isGreaterStep(this, 30)) {
        requestGetCoin();
    }
}

void Coin::appear() {
    appearFix();
}

void Coin::appearFixInit() {
    setNerve(GET_NERVE(Coin, CoinNrvFix));
    makeActorAppeared();
    MR::validateClipping(this);
    MR::validateHitSensors(this);
    MR::offBind(this);
    mCannotTime = 0;
}

void Coin::appearFix() {
    setNerve(GET_NERVE(Coin, CoinNrvFix));
    makeActorAppeared();
    MR::validateClipping(this);
    MR::validateHitSensors(this);
    MR::offBind(this);
    mCannotTime = 0;
}

void Coin::appearFixTimer(s32 lifeTime, s32 cannotTime) {
    setNerve(GET_NERVE(Coin, CoinNrvFixTimer));
    makeActorAppeared();
    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    MR::offBind(this);
    setLife(lifeTime);
    setCannotTime(cannotTime);
}

void Coin::appearMove(const TVec3f& a1, const TVec3f& a2, s32 a3, s32 a4) {
    mPosition.set(a1);
    makeActorAppeared();
    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    MR::onBind(this);
    MR::onCalcShadow(this, nullptr);
    mVelocity.set(a2);

    TVec3f stack_14;
    if (MR::normalizeOrZero(a2, &stack_14)) {
        mGravity.x = 0.0f;
        mGravity.y = -1.0f;
        mGravity.z = 0.0f;
    } else {
        mGravity.set(-stack_14);
    }

    MR::calcGravityOrZero(this);
    setLife(a3);
    setCannotTime(a4);
    setNerve(GET_NERVE(Coin, CoinNrvMove));
}

void Coin::appearHop(const TVec3f& a1, const TVec3f& a2) {
    if (MR::isNearZero(a2)) {
        mGravity.x = 0.0f;
        mGravity.y = -1.0f;
        mGravity.z = 0.0f;
    } else {
        MR::normalize(-a2, &mGravity);
    }

    mPosition.set(a1);
    makeActorAppeared();
    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    MR::offBind(this);
    MR::invalidateShadow(this, nullptr);
    mVelocity.set(a2 * 30.0f);
    mCannotTime = 0;
    setNerve(GET_NERVE(Coin, CoinNrvHop));
}

bool Coin::requestGetCoin() {
    noticeGetCoin();
    MR::emitEffect(this, "CoinGet");
    mFlashingCtrl->end();
    setNerve(GET_NERVE(Coin, CoinNrvGot));
    kill();
    return true;
}

bool Coin::requestStartControl() {
    if (MR::isDead(this)) {
        return false;
    }

    if (!isNerve(GET_NERVE(Coin, CoinNrvFix)) && !isNerve(GET_NERVE(Coin, CoinNrvFixHide))) {
        return false;
    }

    if (isNerve(GET_NERVE(Coin, CoinNrvFixHide))) {
        MR::showModel(this);
    }

    MR::validateClipping(this);
    MR::invalidateHitSensors(this);
    MR::offBind(this);
    MR::onCalcShadow(this, nullptr);
    setNerve(GET_NERVE(Coin, CoinNrvControled));
    mFlashingCtrl->end();
    return true;
}

bool Coin::requestEndControl() {
    if (MR::isDead(this) || !isNerve(GET_NERVE(Coin, CoinNrvControled))) {
        return false;
    }

    MR::validateClipping(this);
    MR::validateHitSensors(this);
    MR::offBind(this);
    MR::offCalcShadow(this, nullptr);
    MR::offCalcGravity(this);
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 150.0f);
    setCalcShadowMode();
    mCannotTime = 0;
    setNerve(GET_NERVE(Coin, CoinNrvFix));
    return true;
}

bool Coin::requestMove() {
    if (MR::isDead(this) || !isNerve(GET_NERVE(Coin, CoinNrvControled))) {
        return false;
    }

    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    MR::onBind(this);
    MR::onCalcShadow(this, nullptr);
    MR::onCalcGravity(this);
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 150.0f);
    setCalcShadowMode();
    mCannotTime = 30;
    setNerve(GET_NERVE(Coin, CoinNrvMove));
    return true;
}

bool Coin::requestThrow() {
    if (MR::isDead(this) || !isNerve(GET_NERVE(Coin, CoinNrvControled))) {
        return false;
    }

    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    MR::onBind(this);
    MR::onCalcShadow(this, nullptr);
    MR::onCalcGravity(this);
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 150.0f);
    setCalcShadowMode();
    mCannotTime = 30;
    setNerve(GET_NERVE(Coin, CoinNrvSupportTicoThrow));
    return true;
}

bool Coin::requestShow() {
    if (isNerve(GET_NERVE(Coin, CoinNrvFixHide)) && !MR::isDead(this)) {
        MR::showModel(this);
        MR::validateHitSensors(this);
        setNerve(GET_NERVE(Coin, CoinNrvFix));
    }

    return false;
}

bool Coin::requestHide() {
    if (isNerve(GET_NERVE(Coin, CoinNrvFix)) && !MR::isDead(this)) {
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(Coin, CoinNrvFixHide));
    }

    return false;
}

void Coin::noticeGetCoin() {
    if (mIsInBubble) {
        MR::emitEffect(mAirBubble, "RecoveryBubbleBreak");
        MR::incPlayerOxygen(8);
        mAirBubble->kill();
        mIsInBubble = false;
    }

    if (mIsPurple) {
        MR::incPurpleCoin();
        MR::startActionSound(this, "SyPurpleCoin");
    } else {
        MR::incCoin(1, this);
        MR::startActionSound(this, "SyCoin");
        mCoinHostInfo->_C++;
    }
}

void Coin::setLife(s32 lifeTime) {
    if (lifeTime < 0) {
        mLifeTime = 0x258;
    } else {
        mLifeTime = lifeTime;
    }
}

void Coin::setHostInfo(CoinHostInfo* pInfo) {
    mCoinHostInfo = pInfo;
}

void Coin::setCalcShadowMode() {
    if (mShadowCalcOn) {
        MR::onCalcShadow(this, nullptr);
        if (mCalcShadowPrivateGravity) {
            MR::onCalcShadowDropPrivateGravity(this, nullptr);
        }
    } else {
        MR::onCalcShadowOneTime(this, nullptr);
        if (mCalcShadowPrivateGravity) {
            MR::onCalcShadowDropPrivateGravityOneTime(this, nullptr);
        }
    }
}

void Coin::setCannotTime(s32 cannotTime) {
    if (cannotTime < 0) {
        mCannotTime = 0x1E;
    } else {
        mCannotTime = cannotTime;
    }
}

void Coin::calcRebouond() {
    if (MR::isBindedRoof(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getRoofNormal(this), 0.60f, 0.5f);
    }

    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.60f, 0.5f);
    }

    if (MR::isBindedGround(this)) {
        f32 dot = mVelocity.dot(-MR::getGroundNormal(this));

        f32 vel = 0.0f;

        if (dot >= 2.0f) {
            if (dot > 30.0f) {
                dot = 30.0f;
            }

            int what = (100.0f * dot) / 30.0f;

            if (what > 5) {
                MR::startSound(this, "SE_OJ_COIN_BOUND", what);
            }

            vel = 0.75f;
        }

        MR::calcReboundVelocity(&mVelocity, -mGravity, vel, 0.5f);
    }
}

void Coin::attenuateVelocity() {
    f32 velocity;

    if (mInWater) {
        velocity = 0.8f;
    } else if (MR::isOnGround(this)) {
        velocity = 0.9f;
    } else {
        velocity = 0.995f;
    }

    MR::attenuateVelocity(this, velocity);
}

void Coin::setShadowCylinder() {
    mShadowType = 1;
}

bool Coin::canStartMove() const {
    if (MR::isDead(this)) {
        return false;
    }

    if (isNerve(GET_NERVE(Coin, CoinNrvFix)) || isNerve(GET_NERVE(Coin, CoinNrvFixTimer))) {
        return true;
    }

    return isNerve(GET_NERVE(Coin, CoinNrvMove));
}

bool Coin::isNeedBubble(const JMapInfoIter& rIter) {
    if (!MR::isValidInfo(rIter)) {
        return false;
    }

    bool arg = false;
    MR::getJMapInfoArg7WithInit(rIter, &arg);
    return arg;
}

namespace MR {
    Coin* createDirectSetCoin(const char* pName) {
        Coin* coin = new Coin(pName);
        MR::createCoinHolder();
        MR::createCoinRotater();
        MR::addToCoinHolder(coin, coin);
        return coin;
    }

    Coin* createCoin(const NameObj* pObj, const char* pName) {
        Coin* coin = new Coin(pName);
        MR::createCoinHolder();
        MR::createCoinRotater();
        MR::addToCoinHolder(pObj, coin);
        return coin;
    }

    Coin* createDirectSetPurpleCoin(const char* pName) {
        Coin* coin = new Coin(pName);
        coin->mIsPurple = true;
        MR::createPurpleCoinHolder();
        MR::createCoinRotater();
        MR::addToPurpleCoinHolder(coin, coin);
        return coin;
    }

    Coin* createPurpleCoin(const NameObj* pObj, const char* pName) {
        Coin* coin = new Coin(pName);
        coin->mIsPurple = true;
        MR::createPurpleCoinHolder();
        MR::createCoinRotater();
        MR::addToPurpleCoinHolder(pObj, coin);
        return coin;
    }
};  // namespace MR
