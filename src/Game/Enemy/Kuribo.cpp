#include "Game/Enemy/Kuribo.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/ItemGenerator.hpp"
#include "Game/Enemy/WalkerStateChase.hpp"
#include "Game/Enemy/WalkerStateFindPlayer.hpp"
#include "Game/Enemy/WalkerStateParam.hpp"
#include "Game/Enemy/WalkerStateStagger.hpp"
#include "Game/Enemy/WalkerStateWander.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/Air.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/VectorUtil.hpp"
#include <revolution/mtx.h>
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util/YoshiUtil.hpp"




namespace {
    class KuriboParam {
    public:
        KuriboParam();

        /* 0x00 */ WalkerStateParam mStateParam;
        /* 0x18 */ WalkerStateStaggerParam mStaggerParam;
        /* 0x48 */ WalkerStateFindPlayerParam mFindPlayerParam;
        /* 0x54 */ WalkerStateChaseParam mChaseParam;
        /* 0x68 */ WalkerStateWanderParam mWanderParam;
    };

    KuriboParam::KuriboParam() {
        mStateParam.mGravityAccel = 1.5f;
        mStateParam.mAirFriction = 0.99f;
        mStateParam.mGroundFriction = 0.93f;
        mStateParam.mPlayerNearDistance = 1000.0f;
        mStateParam.mPlayerSightFanDegreeH = 70.0f;
        mStateParam.mPlayerSightFanDegreeV = 30.0f;
        mWanderParam.mSpeed = 0.2f;
        mWanderParam.mWaitTime = 120;
        mWanderParam.mWalkTime = 120;
        mWanderParam.mTurnMaxRateDegree = 3.0f;
        mChaseParam.mChaseSpeed = 0.6f;
        mFindPlayerParam.mTurnMaxRateDegree = 5.0f;
        mFindPlayerParam.mJumpVelocity = 20.0f;
    }

    static KuriboParam sParam;
};  // namespace

namespace NrvKuribo {
    NEW_NERVE_ONEND(KuriboNrvNonActive, Kuribo, NonActive, NonActive);
    NEW_NERVE(KuriboNrvAppearFromBox, Kuribo, AppearFromBox);
    class KuriboNrvLaunch : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KuriboNrvLaunch sInstance;
    };
    KuriboNrvLaunch KuriboNrvLaunch::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KuriboNrvBlow, Kuribo, Blow);
    NEW_NERVE(KuriboNrvBlowLand, Kuribo, BlowLand);
    NEW_NERVE(KuriboNrvWander, Kuribo, Wander);
    NEW_NERVE(KuriboNrvFindPlayer, Kuribo, FindPlayer);
    NEW_NERVE(KuriboNrvChase, Kuribo, Chase);
    NEW_NERVE(KuriboNrvAttackSuccess, Kuribo, AttackSuccess);
    NEW_NERVE(KuriboNrvBindStarPointer, Kuribo, BindStarPointer);
    NEW_NERVE(KuriboNrvStagger, Kuribo, Stagger);
    NEW_NERVE(KuriboNrvFlatDown, Kuribo, FlatDown);
    NEW_NERVE(KuriboNrvHipDropDown, Kuribo, HipDropDown);
    NEW_NERVE(KuriboNrvPressDown, Kuribo, PressDown);
    NEW_NERVE(KuriboNrvBlowDown, Kuribo, BlowDown);
    class KuriboNrvDead : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KuriboNrvDead sInstance;
    };
    KuriboNrvDead KuriboNrvDead::sInstance ATTRIBUTE_WEAK;
    class KuriboNrvEaten : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static KuriboNrvEaten sInstance;
    };
    KuriboNrvEaten KuriboNrvEaten::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKuribo

Kuribo::Kuribo(const char* pName)
    : LiveActor(pName), mScaleController(nullptr), mStateWander(nullptr), mStateFindPlayer(nullptr), mBindStarPointer(nullptr),
      mStateStagger(nullptr), mStateChase(nullptr), mYoshiLockOn(nullptr), _A8(0.0f, 0.0f, 0.0f, 1.0f), _B8(0, 0, 1), _C8(-1),
      _C4(0), _C5(1), _CE(0) {
}

void Kuribo::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Kuribo", false);
    if (!_C5) {
        MR::disableAllItemGenerator(this);
    }

    MR::makeQuatAndFrontFromRotate(&_A8, &_B8, this);

    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg0WithInit(rIter, &_C4);
    }

    if (!_C4) {
        MR::onCalcGravity(this);
        MR::calcGravity(this);
    } else {
        MR::offCalcGravity(this);
        TVec3f v8;
        _A8.getYDir(v8);
        mGravity.set(v8);
    }

    mScaleController = new AnimScaleController(nullptr);
    initState();
    initAppearState(rIter);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);

    if (MR::isValidInfo(rIter)) {
        MR::setGroupClipping(this, rIter, 32);
    }

    MR::useStageSwitchSyncAppear(this, rIter);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Center", TVec3f(0.0f));
}

void Kuribo::initAfterPlacement() {
    MR::turnQuatUpToGravity(&_A8, _A8, this);
    MR::trySetMoveLimitCollision(this);
}

void Kuribo::initState() {
    initNerve(GET_NERVE(Kuribo, KuriboNrvWander), 4);
    mStateFindPlayer = new WalkerStateFindPlayer(this, &_B8, &::sParam.mStateParam, &::sParam.mFindPlayerParam);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    mStateStagger = new WalkerStateStagger(this, &_B8, &::sParam.mStateParam, &::sParam.mStaggerParam);
    mStateWander = new WalkerStateWander(this, &_B8, &::sParam.mStateParam, &::sParam.mWanderParam);
    mStateChase = new WalkerStateChase(this, &_B8, &::sParam.mStateParam, &::sParam.mChaseParam);
    MR::initActorState(this, mStateStagger, GET_NERVE(Kuribo, KuriboNrvStagger), "Stagger");
    MR::initActorState(this, mBindStarPointer, GET_NERVE(Kuribo, KuriboNrvBindStarPointer), "SupportFreeze");
}

void Kuribo::initAppearState(const JMapInfoIter& rIter) {
    _C8 = -1;
    if (MR::isValidInfo(rIter)) {
        MR::getJMapInfoArg1NoInit(rIter, &_C8);
    }

    switch (_C8) {
    case -1:
        setNerve(GET_NERVE(Kuribo, KuriboNrvWander));
        break;
    case 0:
        setNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox));
        _A8.getZDir(_B8);
        break;
    case 1:
        setNerve(GET_NERVE(Kuribo, KuriboNrvLaunch));
        _A8.getZDir(_B8);
        break;
    }
}

void Kuribo::makeActorAppeared() {
    if (!isNerve(GET_NERVE(Kuribo, KuriboNrvDead))) {
        LiveActor::makeActorAppeared();
    }
}

void Kuribo::makeActorDead() {
    LiveActor::makeActorDead();
}


void Kuribo::kill() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    // nerve instances without source names (sbss); addresses kept as-is for matching
    if (!isNerve(GET_NERVE(Kuribo, KuriboNrvEaten))) {
        MR::emitEffect(this, "DeathSmokeKuribo");
        MR::startActionSound(this, "EmExplodeS", -1, -1, -1);
    }

    setNerve(GET_NERVE(Kuribo, KuriboNrvDead));
    LiveActor::kill();
}

// NONMATCHING: MWCC folds the explicit early-return branch after tryPointBind.
void Kuribo::control() {
    mScaleController->updateNerve();
    if (_C4) {
        MR::calcGravityOrZero(this);
    }

    MR::blendQuatFromGroundAndFront(&_A8, this, _B8, 0.05f, 0.5f);
    MR::fn_800145C0(this);
    MR::fn_80014670(this);
    if (!isNerve(GET_NERVE(Kuribo, KuriboNrvBindStarPointer))) {
        TVec3f v3(0, 0, 0);
        if (MR::calcVelocityAreaOrRailMoveOnGround(&v3, this)) {
            MR::addVelocity(this, v3 * 0.05f);
        }
    }

    if (!tryDead()) {
        switch (tryPointBind()) {
        case false:
            if (fn_801BAF60()) YoshiFunction::tryYoshiEat(mYoshiLockOn);
            break;
        default:
            break;
        }
    }
}

void Kuribo::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, _A8);
    MR::updateBaseScale(this, mScaleController);
}

void Kuribo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if ((!MR::isSensorType(pSender, 0x1F /* ATYPE_KURIBO */) || (isEnableAttack() || !MR::isSensorPlayer(pReceiver)) && !(MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) ||
         !isEnablePushMove() || !MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) &&
        isEnableAttack() && MR::isSensorPlayer(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        if (!MR::isPlayerHipDropFalling() && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            requestAttackSuccess();
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool Kuribo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        return requestStagger(pSender, pReceiver);
    }

    if (MR::isMsgPlayerTrample(msg)) {
        return requestFlatDown(msg, pSender, pReceiver);
    }

    if (MR::isMsgPlayerHitProjectiles(msg)) {
        return requestBlowDown(msg, pSender, pReceiver);
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        return requestHipDropDown(msg, pSender, pReceiver);
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        if (isNerve(GET_NERVE(Kuribo, KuriboNrvStagger))) return false;
        return fn_801B9880(pSender, pReceiver);
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        if (!isEnableKick()) {
            return fn_801B9880(pSender, pReceiver);
        }

        return requestBlowDown(msg, pSender, pReceiver);
    }

    return false;
}

bool Kuribo::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgToEnemyAttackShockWave(msg)) {
        return fn_801B9880(pSender, pReceiver);
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        return requestPressDown();
    }

    if (MR::isMsgExplosionAttack(msg)) {
        return requestBlowDown(msg, pSender, pReceiver);
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        return requestBlowDown(msg, pSender, pReceiver);
    }

    return false;
}

bool Kuribo::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }

    if (MR::isSensorEnemy(pSender) || MR::isSensorRide(pSender) || MR::isSensorNpc(pSender) || (MR::isSensorYoshi(pSender) || (!isEnableAttack() && MR::isSensorPlayer(pSender)))) {
        if (isEnablePushMove()) {
            MR::addVelocityFromPush(this, 1.5f, pSender, pReceiver);
            return true;
        }
    }

    return false;
}

bool Kuribo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }

    if (MR::isMsgPlayerKick(msg) && isEnableKick()) {
        return requestBlowDown(msg, pSender, pReceiver);
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        MR::stopBck(this);
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(Kuribo, KuriboNrvEaten));
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) return true;
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        MR::zeroVelocity(this);
        MR::offBind(this);
        kill();
        return true;
    }
    return false;
}

bool Kuribo::requestDead() {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvNonActive)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvPressDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBlowDown))) {
        return false;
    }

    MR::deleteEffectAll(this);
    MR::clearHitSensors(this);
    MR::invalidateHitSensors(this);
    return true;
}

bool Kuribo::requestFlatDown(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }

    if (!isEnableStamp()) {
        return false;
    }

    if (requestDead()) {
        _CE = isUpsideDown();
        setNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown));
        MR::offBind(this);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        return true;
    }
    return false;
}

bool Kuribo::requestHipDropDown(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }

    if (requestDead()) {
        _CE = isUpsideDown();
        setNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown));
        MR::offBind(this);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        return true;
    }
    return false;
}

bool Kuribo::requestPressDown() {
    if (requestDead()) {
        _CE = isUpsideDown();
        setNerve(GET_NERVE(Kuribo, KuriboNrvPressDown));
        MR::zeroVelocity(this);
        MR::offBind(this);
        return true;
    }
    return false;
}

bool Kuribo::requestStagger(HitSensor* pSender, HitSensor* pReceiver) {
    if (fn_801BAD90()) {
        mStateStagger->setPunchDirection(pSender, pReceiver);
        mStateStagger->_28 = false;
        setNerve(GET_NERVE(Kuribo, KuriboNrvStagger));
        return true;
    }

    return false;
}

bool Kuribo::requestBlowDown(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isEnablePanch()) {
        return false;
    }

    MR::deleteEffectAll(this);
    MR::setVelocityBlowAttack(this, pSender, pReceiver, 36.0f, 30.0f, 4);
    setNerve(GET_NERVE(Kuribo, KuriboNrvBlowDown));
    ScoreAttackFunction::scoreByAttack(this, msg, pSender);
    return true;
}

bool Kuribo::requestAttackSuccess() {
    if (isEnableAttack()) {
        mScaleController->stopAndReset();
        MR::deleteEffectAll(this);
        setNerve(GET_NERVE(Kuribo, KuriboNrvAttackSuccess));
        return true;
    }
    return false;
}

void Kuribo::onNoGravitySupport() {
    _C4 = 1;
    MR::offCalcGravity(this);
}

void Kuribo::setUp(const TVec3f& a1, const TQuat4f& a2, const TVec3f& a3) {
    MR::setPosition(this, a1);
    mVelocity.set(a3);
    _A8.x = a2.x;
    _A8.y = a2.y;
    _A8.z = a2.z;
    _A8.w = a2.w;
    _A8.getZDir(_B8);
}

void Kuribo::appearBlowed(const TVec3f& a1, const TQuat4f& a2, const TVec3f& a3) {
    setUp(a1, a2, a3);
    setNerve(GET_NERVE(Kuribo, KuriboNrvBlow));
    MR::startAction(this, "Blow");
    appear();
    if (_C4) {
        MR::calcGravityOrZero(this);
    }
}

void Kuribo::appearHipDropped(const TVec3f& a1, const TQuat4f& a2) {
    setUp(a1, a2, TVec3f(0.0f, 0.0f, 0.0f));
    appear();
    _CE = false;
    setNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown));
    MR::offBind(this);
}

bool Kuribo::tryNonActive() {
    bool isNotNearPlayer = !MR::isNearPlayerAnyTime(this, 3000.0f);
    if (isNotNearPlayer && MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Kuribo, KuriboNrvNonActive));
        return true;
    }

    return false;
}

bool Kuribo::tryActive() {
    if (MR::isNearPlayerAnyTime(this, 3000.0f)) {
        setNerve(GET_NERVE(Kuribo, KuriboNrvWander));
        return true;
    }

    return false;
}

bool Kuribo::tryEndBlow() {
    if (MR::isBindedGround(this) && MR::isGreaterStep(this, 10)) {
        MR::startAction(this, "Land");
        MR::onBind(this);
        setNerve(GET_NERVE(Kuribo, KuriboNrvBlowLand));
        return true;
    }

    return false;
}

bool Kuribo::tryEndBlowLand() {
    if (MR::isGreaterStep(this, 15)) {
        setNerve(GET_NERVE(Kuribo, KuriboNrvWander));
        return true;
    }

    return false;
}

bool Kuribo::tryFind() {
    if (mStateFindPlayer->isInSightPlayer()) {
        setNerve(GET_NERVE(Kuribo, KuriboNrvFindPlayer));
        return true;
    }

    return false;
}

bool Kuribo::tryPointBind() {
    if (isEnablePointBind() && mBindStarPointer->tryStartPointBind()) {
        _CE = isUpsideDown();
        setNerve(GET_NERVE(Kuribo, KuriboNrvBindStarPointer));
        return true;
    }

    return false;
}

bool Kuribo::tryDead() {
    if (isEnableDead()) {
        if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) || MR::isBindedGroundDamageFire(this) || MR::isInWater(mPosition)) {
            kill();
            return true;
        }

        if (MR::isPressedRoofAndGround(this) && requestPressDown()) {
            return true;
        }
    }

    return false;
}

void Kuribo::exeWander() {
    MR::updateActorState(this, mStateWander);
    if (!tryFind()) {
        if (tryNonActive()) {
            return;
        }
    }
}

void Kuribo::exeFindPlayer() {
    if (!MR::updateActorStateAndNextNerve(this, mStateFindPlayer, GET_NERVE(Kuribo, KuriboNrvChase))) {
        if (mStateFindPlayer->isFindJumpBegin()) {
            MR::startActionSound(this, "EmKuriboFind");
        }
    }
}

void Kuribo::exeChase() {
    if (MR::updateActorStateAndNextNerve(this, mStateChase, GET_NERVE(Kuribo, KuriboNrvWander))) {
        mStateWander->setWanderCenter(mPosition);
    }

    if (mStateChase->isRunning()) {
        MR::startActionSound(this, "EmLvKuriboDash1");
    }
}

void Kuribo::exeStagger() {
    if (!MR::updateActorStateAndNextNerve(this, GET_NERVE(Kuribo, KuriboNrvWander))) {
        if (mStateStagger->isStaggerStart()) {
            MR::startActionSound(this, "EmCrashS");
            MR::startBlowHitSound(this);
        }

        if (mStateStagger->isSwooning(15)) {
            MR::startActionSound(this, "EmLvSwoonS");
        }

        if (mStateStagger->isRecoverStart()) {
            MR::startActionSound(this, "EmKuriboSwoonRecover");
        }
    }
}

void Kuribo::exeNonActive() {
    if (MR::isFirstStep(this)) {
        calcAnim();
        MR::offBind(this);
        MR::offCalcGravity(this);
        MR::offCalcShadow(this, nullptr);
    }

    MR::zeroVelocity(this);
    if (tryActive()) {
        return;
    }
}

void Kuribo::endNonActive() {
    MR::onBind(this);
    MR::onCalcGravity(this);
    MR::onCalcShadow(this, nullptr);
}

void Kuribo::exeAppearFromBox() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AppearFromBox");
        TVec3f zDir;
        _A8.getZDir(zDir);
        MR::setVelocitySeparateHV(this, zDir, 7.0f, 30.0f);
        MR::offBind(this);
    }
    exeBlow();
}

void Kuribo::exeBlow() {
    calcPassiveMovement();
    if (MR::isStep(this, 10)) {
        MR::onBind(this);
    }

    if (tryEndBlow()) {
        return;
    }
}

void Kuribo::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
    }

    MR::turnDirectionToTargetUseGroundNormalDegree(this, &_B8, *MR::getPlayerPos(), 5.0f);
    calcPassiveMovement();
    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(Kuribo, KuriboNrvWander));
    }
}

void Kuribo::exeBindStarPointer() {
    if (MR::updateActorState(this)) {
        if (_CE) {
            // WalkerStateStagger byte at +0x28 (no named field in our header yet)
            mStateStagger->_28 = true;
            setNerve(GET_NERVE(Kuribo, KuriboNrvStagger));
        } else {
            setNerve(GET_NERVE(Kuribo, KuriboNrvWander));
        }
    }
}



void Kuribo::exeHipDropDown() {
    if (MR::isFirstStep(this)) {
        if (_CE) MR::startAction(this, "HipDropDownReverse");
        else MR::startAction(this, "HipDropDown");
        MR::startActionSound(this, "EmCrashS");
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, 40)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void Kuribo::exeFlatDown() {
    if (MR::isFirstStep(this)) {
        if (_CE) MR::startAction(this, "FlatDownReverse");
        else MR::startAction(this, "FlatDown");
        MR::startActionSound(this, "EmCrashS");
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, 20)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void Kuribo::exePressDown() {
    if (MR::isFirstStep(this)) {
        if (_CE) {
            MR::startAction(this, "HipDropDownReverse");
        } else {
            MR::startAction(this, "HipDropDown");
        }
        MR::startActionSound(this, "EmCrashS", -1, -1, -1);
        MR::zeroVelocity(this);
    }

    if (MR::isGreaterStep(this, 60)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void Kuribo::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
        MR::startActionSound(this, "EmCrashS");
        MR::tryHitReactionBlow(this, &mPosition);
    }

    calcPassiveMovement();
    MR::turnDirectionDegree(this, &_B8, -mVelocity, 30.0f);
    if (MR::isGreaterStep(this, 30)) {
        MR::emitItemHit(this);
        kill();
    }
}

void Kuribo::calcPassiveMovement() {
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 1.5f);
    }

    f32 vel;

    if (MR::isOnGround(this)) {
        vel = 0.93f;
    } else {
        vel = 0.99f;
    }

    MR::attenuateVelocity(this, vel);
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
}

bool Kuribo::isEnableKick() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvStagger))) {
        return mStateStagger->isEnableKick();
    }

    if (isNerve(GET_NERVE(Kuribo, KuriboNrvBindStarPointer))) {
        return _CE;
    }

    return false;
}

bool Kuribo::isUpsideDown() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvStagger))) {
        return mStateStagger->isUpsideDown();
    }

    if (isNerve(GET_NERVE(Kuribo, KuriboNrvBindStarPointer))) {
        return _CE;
    }

    return false;
}

bool Kuribo::isEnableDead() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox)) || isNerve(GET_NERVE(Kuribo, KuriboNrvLaunch)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvWander)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFindPlayer)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvChase)) || isNerve(GET_NERVE(Kuribo, KuriboNrvStagger)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBindStarPointer)) || isNerve(GET_NERVE(Kuribo, KuriboNrvNonActive)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvAttackSuccess))) {
        return true;
    }

    return false;
}

bool Kuribo::isEnablePointBind() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvWander)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFindPlayer)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvChase)) || isNerve(GET_NERVE(Kuribo, KuriboNrvAttackSuccess))) {
        return true;
    }

    if (isNerve(GET_NERVE(Kuribo, KuriboNrvStagger))) {
        return mStateStagger->isEnableKick();
    }

    return false;
}

bool Kuribo::isEnableAttack() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvWander)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFindPlayer)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvChase))) {
        return true;
    }

    return false;
}

bool Kuribo::isEnableStamp() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvNonActive)) || isNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvLaunch)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvPressDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBlowDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvBlow))) {
        return false;
    }

    return true;
}

bool Kuribo::isEnablePanch() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox)) || isNerve(GET_NERVE(Kuribo, KuriboNrvLaunch)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvPressDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvBlowDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBlow))) {
        return false;
    }

    return true;
}

bool Kuribo::isEnablePushMove() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox)) || isNerve(GET_NERVE(Kuribo, KuriboNrvLaunch)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvPressDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBlowDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvEaten))) {
        return false;
    }

    return true;
}

void Kuribo::exeBlowLand() {
    calcPassiveMovement();
    tryEndBlowLand();
}

namespace MR {
    LiveActor* createNoItemKuriboActor(const char* pName) {
        Kuribo* kuribo = new Kuribo(pName);
        kuribo->_C5 = 0;
        return kuribo;
    }
};  // namespace MR

Kuribo::~Kuribo() {
}

bool Kuribo::fn_801B9880(HitSensor* sender, HitSensor* receiver) {
    if (fn_801BACC0()) {
        mStateStagger->setPunchDirection(sender, receiver);
        mStateStagger->_28 = false;
        setNerve(GET_NERVE(Kuribo, KuriboNrvStagger));
        return true;
    }
    return false;
}
void Kuribo::fn_801B9B10(const TVec3f& pos, const TQuat4f& quat, const TVec3f& velocity) {
    setUp(pos, quat, velocity);
    setNerve(GET_NERVE(Kuribo, KuriboNrvWander));
    appear();
}
void Kuribo::fn_801B9BD0(const TVec3f& pos, const TQuat4f& quat) {
    setUp(pos, quat, TVec3f(0.0f, 0.0f, 0.0f));
    setNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox));
    MR::validateHitSensors(this);
    appear();
}
void Kuribo::fn_801B9C60(const TVec3f& pos, const TQuat4f& quat) {
    setUp(pos, quat, TVec3f(0.0f, 0.0f, 0.0f));
    appear();
    _CE = false;
    setNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown));
    MR::offBind(this);
}
void Kuribo::fn_801BA420() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AppearFromBox");
        TVec3f front;
        _A8.getZDir(front);
        MR::setVelocitySeparateHV(this, front, -2.0f, 30.0f);
        MR::offBind(this);
    }
    exeBlow();
}
bool Kuribo::fn_801BACC0() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvAppearFromBox)) || isNerve(GET_NERVE(Kuribo, KuriboNrvLaunch)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvStagger)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFlatDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvHipDropDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvPressDown)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBlowDown)) || isNerve(GET_NERVE(Kuribo, KuriboNrvBlow))) return false;
    return true;
}
bool Kuribo::fn_801BAD90() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvStagger))) return true;
    return fn_801BACC0();
}
bool Kuribo::fn_801BAF60() const {
    if (isNerve(GET_NERVE(Kuribo, KuriboNrvBlow)) || isNerve(GET_NERVE(Kuribo, KuriboNrvBlowLand)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvWander)) || isNerve(GET_NERVE(Kuribo, KuriboNrvFindPlayer)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvChase)) || isNerve(GET_NERVE(Kuribo, KuriboNrvAttackSuccess)) ||
        isNerve(GET_NERVE(Kuribo, KuriboNrvBindStarPointer)) || isNerve(GET_NERVE(Kuribo, KuriboNrvStagger))) return true;
    return false;
}
void NrvKuribo::KuriboNrvDead::execute(Spine*) const {}
void NrvKuribo::KuriboNrvEaten::execute(Spine*) const {}

void NrvKuribo::KuriboNrvLaunch::execute(Spine* spine) const {
    static_cast<Kuribo*>(spine->mExecutor)->fn_801BA420();
}
