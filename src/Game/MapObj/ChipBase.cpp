#include "Game/MapObj/ChipBase.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/FlashingCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/MapObj/ChipHolder.hpp"
#include "Game/MapObj/MapPartsRailMover.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/NativeActorObserve.hpp"
#ifdef GALAXY_NATIVE
#include <petari/milestone.hpp>
#endif

namespace {
    static const f32 sBubbleHitRadius = 150.0f;
    static const f32 sBodyHitRadius = 80.0f;
    static const f32 sShadowRadius = 50.0f;
    static const f32 sBaseScale = 1.0f;
    // static const f32 sLife = 0.0f;
    static const s32 sCannotGetTime = 40;
    static const f32 sBodyHitYOffset = 0.0f;
};  // namespace

inline void ChipBase::exeDeactive() {}

inline void ChipBase::exeHide() {}

inline void ChipBase::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateHitSensors(this);
    }
}

inline void ChipBase::exeControled() {
    MR::zeroVelocity(this);
}

namespace NrvChipBase {
    NEW_NERVE(ChipBaseNrvDeactive, ChipBase, Deactive);
    NEW_NERVE(ChipBaseNrvWait, ChipBase, Wait);
    NEW_NERVE(ChipBaseNrvFall, ChipBase, Fall);
    NEW_NERVE(ChipBaseNrvControled, ChipBase, Controled);
    NEW_NERVE(ChipBaseNrvFlashing, ChipBase, Flashing);
    NEW_NERVE(ChipBaseNrvHide, ChipBase, Hide);
    NEW_NERVE(ChipBaseNrvGot, ChipBase, Got);
};  // namespace NrvChipBase

ChipBase::ChipBase(const char* pName, s32 chipType, const char* pChipName)
    : LiveActor(pName), mFlashingCtrl(), mRailMover(), mAirBubble(), mChipName(pChipName), mHost(), mClippingRange(0.0f, 0.0f, 0.0f), mGroupID(-1),
      mChipType(chipType), mCounter(), mIsCalcShadow() {
}

void ChipBase::init(const JMapInfoIter& rIter) {
    MR::createChipHolder(mChipType);

    initJMapParam(rIter);
    initModel(rIter);
    initSensor();
    initShadow(rIter);
    initEffectKeeper(0, nullptr, false);
    initSound(4, "ChipBase", nullptr, TVec3f(0.0f));
    initNerve(GET_NERVE(ChipBase, ChipBaseNrvWait));

    if (mRailMover == nullptr) {
        initBinder(75.0f, 0.0f, 0);
        MR::offBind(this);
    }

    if (MR::isValidInfo(rIter)) {
        MR::setGroupClipping(this, rIter, 0x20);
    }

    mFlashingCtrl = new FlashingCtrl(this, true);

    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void ChipBase::initModel(const JMapInfoIter& rIter) {
    mScale *= ::sBaseScale;

    MR::initActor(this, rIter, mChipName, false);

    MR::connectToSceneNoSilhouettedMapObjStrongLight(this);

    if (isNeedBubble(rIter)) {
        mAirBubble = MR::createPartsModelNoSilhouettedMapObj(this, "アワ", "AirBubble", 0);
        mAirBubble->initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), 0);
        MR::startAction(mAirBubble, "Move");
    }
}

void ChipBase::initSensor() {
    f32 radius;
    f32 scale = mScale.x;

    initHitSensor(1);

    if (mAirBubble != nullptr) {
        radius = ::sBubbleHitRadius;
    } else {
        radius = ::sBodyHitRadius;
    }

    MR::addHitSensorEnemy(this, "Body", 8, radius * scale, TVec3f(0.0f, ::sBodyHitYOffset, 0.0f));
}

void ChipBase::initShadow(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg2WithInit(rIter, &mIsCalcShadow);
    } else {
        mIsCalcShadow = false;
    }

    s32 shadowShape = -1;
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg5NoInit(rIter, &shadowShape);
    }

    switch (shadowShape) {
    case 0:
        MR::initShadowVolumeCylinder(this, ::sShadowRadius * mScale.x);
        mIsCalcShadow = false;

        break;
    default:
        MR::initShadowVolumeSphere(this, ::sShadowRadius * mScale.x);

        break;
    }

    f32 dropLength = 2000.0f;
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg4NoInit(rIter, &dropLength);
    }

    MR::setShadowDropLength(this, 0, dropLength);

    if (mRailMover || mIsCalcShadow) {
        MR::onCalcShadowDropPrivateGravity(this, 0);
    } else {
        MR::onCalcShadowOneTime(this, 0);
        MR::onCalcShadowDropPrivateGravityOneTime(this, 0);
    }
}

void ChipBase::initJMapParam(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::initDefaultPos(this, rIter);
        MR::getJMapInfoArg0NoInit(rIter, &mGroupID);

        if (MR::isConnectedWithRail(rIter)) {
            initRailRider(rIter);
            mRailMover = new MapPartsRailMover(this);
            mRailMover->init(rIter);
        }
    }
}

void ChipBase::initAfterPlacement() {
    MR::registerChip(mChipType, this, mGroupID);
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 100.0f);
}

void ChipBase::deactive() {
    makeActorDead();
    setNerve(GET_NERVE(ChipBase, ChipBaseNrvDeactive));
}

void ChipBase::setGroupID(s32 id) {
    mGroupID = id;
}

void ChipBase::setHost(LiveActor* pActor) {
    mHost = pActor;
}

void ChipBase::makeActorAppeared() {
    if (isNerve(GET_NERVE(ChipBase, ChipBaseNrvDeactive))) {
        return;
    }

    LiveActor::makeActorAppeared();

    if (mRailMover != nullptr) {
        mRailMover->start();
    }

    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 100.0f);
}

void ChipBase::makeActorDead() {
    if (mRailMover != nullptr) {
        mRailMover->end();
    }

    LiveActor::makeActorDead();
}

void ChipBase::control() {
    if (mCounter > 0) {
        mCounter--;
    }

    if (mRailMover != nullptr) {
        mRailMover->movement();
        MR::setPosition(this, mRailMover->mPosition);
        MR::setClippingRangeIncludeShadow(this, &mClippingRange, 100.0f);
    }
}

void ChipBase::appearWait() {
    if (!isNerve(GET_NERVE(ChipBase, ChipBaseNrvDeactive))) {
        mCounter = ::sCannotGetTime;

        makeActorAppeared();

        MR::validateClipping(this);
        MR::updateChipCounterUIRange(mChipType, this, mGroupID);

        setNerve(GET_NERVE(ChipBase, ChipBaseNrvWait));
    }
}

void ChipBase::appearFlashing(s32 a1) {
    if (!isNerve(GET_NERVE(ChipBase, ChipBaseNrvDeactive))) {
        mCounter = ::sCannotGetTime;

        appear();

        MR::invalidateClipping(this);
        MR::updateChipCounterUIRange(mChipType, this, mGroupID);

        mFlashingCtrl->start(a1);

        setNerve(GET_NERVE(ChipBase, ChipBaseNrvFlashing));
    }
}

bool ChipBase::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgItemGet(msg)) {
        return requestGet(pReceiver, pSender);
    }

    if (MR::isMsgItemShow(msg)) {
        return requestShow();
    }

    if (MR::isMsgItemHide(msg)) {
        return requestHide();
    }

    if (MR::isMsgItemNigeroTake(msg)) {
        return isNigeroTakeable();
    }

    if (MR::isMsgItemStartMove(msg)) {
        return requestStartControl();
    }

    if (MR::isMsgItemEndMove(msg)) {
        return requestEndControl();
    }

    if (MR::isMsgItemNigeroDrop(msg)) {
        return requestNigeroDrop();
    }

    return false;
}

bool ChipBase::requestGet(HitSensor* pSender, HitSensor* pReceiver) {
    if (isGettable()) {
        MR::noticeGetChip(mChipType, this, mGroupID);
        exeGot();

        if (mHost != nullptr) {
            mHost->receiveMessage(ACTMES_ITEM_GET, pSender, pReceiver);
        }

        return true;
    }

    return false;
}

bool ChipBase::requestShow() {
    if (isNerve(GET_NERVE(ChipBase, ChipBaseNrvHide))) {
        MR::startAction(this, "Wait");
        MR::showModel(this);

        setNerve(GET_NERVE(ChipBase, ChipBaseNrvWait));

        return true;
    }

    return false;
}

bool ChipBase::requestHide() {
    if (isGettable()) {
        MR::invalidateHitSensors(this);

        MR::hideModel(this);
        MR::stopBck(this);

        MR::forceDeleteEffectAll(this);

        setNerve(GET_NERVE(ChipBase, ChipBaseNrvHide));

        return true;
    }

    return false;
}

bool ChipBase::requestStartControl() {
    if (isNigeroTakeable()) {
        MR::invalidateClipping(this);
        MR::onCalcShadow(this, 0);
        MR::onCalcShadowDropPrivateGravity(this, 0);

        setNerve(GET_NERVE(ChipBase, ChipBaseNrvControled));
        return true;
    }

    return false;
}

bool ChipBase::requestEndControl() {
    if (isNerve(GET_NERVE(ChipBase, ChipBaseNrvControled))) {
        mCounter = ::sCannotGetTime;
        returnToWait();
        return true;
    }

    return false;
}

bool ChipBase::requestNigeroDrop() {
    if (isNerve(GET_NERVE(ChipBase, ChipBaseNrvControled))) {
        mCounter = ::sCannotGetTime;
        startFall();
        return true;
    }

    return false;
}

void ChipBase::returnToWait() {
    MR::validateClipping(this);

    if (mRailMover != nullptr || mIsCalcShadow) {
        MR::onCalcShadow(this, 0);
        MR::onCalcShadowDropPrivateGravity(this, 0);
    } else {
        MR::onCalcShadowOneTime(this, 0);
        MR::onCalcShadowDropPrivateGravityOneTime(this, 0);
    }

    MR::offCalcGravity(this);
    MR::offBind(this);
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 100.0f);
    MR::zeroVelocity(this);

    setNerve(GET_NERVE(ChipBase, ChipBaseNrvWait));
}

void ChipBase::startFall() {
    MR::invalidateClipping(this);
    MR::onCalcGravity(this);
    MR::onCalcShadow(this, 0);
    MR::onCalcShadowDropPrivateGravity(this, 0);
    MR::onBind(this);
    MR::setClippingRangeIncludeShadow(this, &mClippingRange, 100.0f);

    setNerve(GET_NERVE(ChipBase, ChipBaseNrvFall));
}

void ChipBase::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateHitSensors(this);
    }

    MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.995f);
    MR::reboundVelocityFromCollision(this, 0.5f, 0.0f, 0.0f);

    if (MR::isGreaterStep(this, 70) && MR::isBindedGround(this)) {
        returnToWait();
    }
}

void ChipBase::exeFlashing() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateHitSensors(this);
    }

    if (mFlashingCtrl->mIsEnded) {
        makeActorDead();
        setNerve(GET_NERVE(ChipBase, ChipBaseNrvDeactive));
    }
}

void ChipBase::exeGot() {
    if (hasAirBubble()) {
        MR::emitEffect(mAirBubble, "RecoveryBubbleBreak");

        MR::incPlayerOxygen(8);

        mAirBubble->kill();
    }

    MR::emitEffect(this, "Get");

    MR::tryRumblePadMiddle(this, WPAD_CHAN0);

    if (mChipType == Type_Blue) {
        MR::startActionSound(this, "SyBluechipGet", MR::getGotChipCount(mChipType, mGroupID), -1, -1);
    } else if (mChipType == Type_Yellow) {
        MR::startActionSound(this, "SyYellowchipGet", MR::getGotChipCount(mChipType, mGroupID), -1, -1);
    }

    makeActorDead();
    setNerve(GET_NERVE(ChipBase, ChipBaseNrvDeactive));
}

bool ChipBase::isGettable() const {
    bool result;

    if (MR::isDead(this)) {
        return false;
    }

    if (mCounter > 0) {
        result = false;
    } else {
        result = false;

        if (isNerve(GET_NERVE(ChipBase, ChipBaseNrvWait)) || isNerve(GET_NERVE(ChipBase, ChipBaseNrvFall)) ||
            isNerve(GET_NERVE(ChipBase, ChipBaseNrvFlashing))) {
            result = true;
        }
    }

    return result;
}

bool ChipBase::isNigeroTakeable() const {
    bool result;

    if (MR::isDead(this)) {
        return false;
    }

    if (mRailMover != nullptr) {
        result = false;
    } else {
        result = false;

        if (isNerve(GET_NERVE(ChipBase, ChipBaseNrvWait)) || isNerve(GET_NERVE(ChipBase, ChipBaseNrvFall))) {
            result = true;
        }
    }

    return result;
}

bool ChipBase::isNeedBubble(const JMapInfoIter& rIter) {
    if (!MR::isValidInfo(rIter)) {
        return false;
    }

    bool arg3 = false;
    MR::getJMapInfoArg3WithInit(rIter, &arg3);

    return arg3;
}
