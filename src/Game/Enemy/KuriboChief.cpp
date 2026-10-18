#include "Game/Enemy/KuriboChief.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Enemy/WalkerStateChase.hpp"
#include "Game/Enemy/WalkerStateFindPlayer.hpp"
#include "Game/Enemy/WalkerStateFunction.hpp"
#include "Game/Enemy/WalkerStateParam.hpp"
#include "Game/Enemy/WalkerStateStagger.hpp"
#include "Game/Enemy/WalkerStateWander.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/KeySwitch.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>
#include <revolution/os.h>
#include <revolution/types.h>
#include "Game/LiveActor/ActorItemGenerator.hpp"


namespace {
    static const f32 sBinderSize = 300.0f;
    static const f32 sBinderYOffset = 300.0f;
    static const f32 sUpVecBlendRate = 0.05f;
    static const f32 sFrontVecBlendRate = 0.5f;
    static const f32 sTurnLimitDegree = 5.0f;
    static const s32 sAttackSuccessTime = 60;
    // sStarPieceAttackScaleAccel
    static const s32 sBlowDownLandTime = 5;
    // sPunchHorizontalAccel
    // sPunchUpperAccel
    // sPunchSceneStopTime
    static const s32 sBoundSoundStep = 27;
    static const s32 sSwoonSoundWait = 56;
    static const s32 sSpinSoundStartStep = 10;
    static const s32 sSpinSoundEndStep = 105;
};  // namespace

namespace {
    class KuriboChiefParam {
    public:
        KuriboChiefParam();

        WalkerStateParam mStateParam1;
        WalkerStateParam mStateParam2;
        WalkerStateStaggerParam mStaggerParam;
        WalkerStateFindPlayerParam mFindPlayerParam;
        WalkerStateChaseParam mChaseParam;
        WalkerStateWanderParam mWanderParam;
        AnimScaleParam mScaleParam;
    };

    KuriboChiefParam::KuriboChiefParam() {
        mStateParam1.mGravityAccel = 1.5f;
        mStateParam1.mAirFriction = 0.99f;
        mStateParam1.mGroundFriction = 0.93f;
        mStateParam1.mPlayerNearDistance = 1400.0f;
        mStateParam1.mPlayerSightFanDegreeH = 80.0f;
        mStateParam1.mPlayerSightFanDegreeV = 40.0f;
        mWanderParam.mSpeed = 0.1f;
        mWanderParam.mWaitTime = 120;
        mWanderParam.mWalkTime = 300;
        mWanderParam.mTurnMaxRateDegree = 1.0f;
        mChaseParam.mChaseSpeed = 0.2f;
        mStateParam2.mGravityAccel = 1.5f;
        mStateParam2.mAirFriction = 0.99f;
        mStateParam2.mGroundFriction = 0.6f;
        mStaggerParam.mStaggerTime = 240;
        mStaggerParam.mRotateRateDegree = 0.0f;
        mStaggerParam.mKickEnableStep = 70;
        mFindPlayerParam.mJumpStartStep = 56;
        mFindPlayerParam.mTurnMaxRateDegree = 2.0f;
        mFindPlayerParam.mJumpVelocity = 35.0f;
        mScaleParam._0 = 0.1f;
        mScaleParam._10 = 30.0f;
        mScaleParam._14 = 0.6f;
        mScaleParam._18 = 0.06f;
        mScaleParam._1C = 3.0f;
        mScaleParam._24 = 0.1f;
    }

    static KuriboChiefParam sParam;
};  // namespace

namespace NrvKuriboChief {
    NEW_NERVE(KuriboChiefNrvWander, KuriboChief, Wander);
    NEW_NERVE(KuriboChiefNrvFindPlayer, KuriboChief, FindPlayer);
    NEW_NERVE(KuriboChiefNrvChase, KuriboChief, Chase);
    NEW_NERVE(KuriboChiefNrvAttackSuccess, KuriboChief, AttackSuccess);
    NEW_NERVE(KuriboChiefNrvStagger, KuriboChief, Stagger);
    NEW_NERVE(KuriboChiefNrvTrample, KuriboChief, Trample);
    NEW_NERVE(KuriboChiefNrvBindStarPointer, KuriboChief, BindStarPointer);
    NEW_NERVE(KuriboChiefNrvBlowDown, KuriboChief, BlowDown);
    NEW_NERVE(KuriboChiefNrvBlowDownLand, KuriboChief, BlowDownLand);
    class KuriboChiefNrvDead : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KuriboChiefNrvDead sInstance;
    };
    KuriboChiefNrvDead KuriboChiefNrvDead::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKuriboChief

KuriboChief::KuriboChief(const char* pName) : LiveActor(pName) {
    mScaleController = nullptr;
    mStateWander = nullptr;
    mStateFindPlayer = nullptr;
    mStateChase = nullptr;
    mStateStagger = nullptr;
    mStateBindStarPointer = nullptr;
    mKeySwitch = nullptr;
    _AC.x = 0.0f;
    _AC.y = 0.0f;
    _AC.z = 0.0f;
    _AC.w = 1.0f;
    _BC.set(0.0f, 0.0f, 1.0f);
    _C8 = false;
}

void KuriboChief::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KuriboChief", false);
    MR::onCalcGravity(this);
    mScaleController = new AnimScaleController(&::sParam.mScaleParam);
    initState();
    initKeySwitch(rIter);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);

    if (MR::isValidInfo(rIter)) {
        s32 arg = -1;
        MR::getJMapInfoArg7WithInit(rIter, &arg);
        if (arg != -1) {
            MR::declareCameraRegisterVec(this, arg, &mPosition);
        }
    }

    MR::useStageSwitchSyncAppear(this, rIter);
}

void KuriboChief::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void KuriboChief::makeActorAppeared() {
    if (!isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvDead))) {
        LiveActor::makeActorAppeared();
    }
}

void KuriboChief::initState() {
    initNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander), 2);
    mStateFindPlayer = new WalkerStateFindPlayer(this, &_BC, &::sParam.mStateParam1, &::sParam.mFindPlayerParam);
    mStateWander = new WalkerStateWander(this, &_BC, &::sParam.mStateParam1, &::sParam.mWanderParam);
    mStateChase = new WalkerStateChase(this, &_BC, &::sParam.mStateParam1, &::sParam.mChaseParam);
    mStateStagger = new WalkerStateStagger(this, &_BC, &::sParam.mStateParam2, &::sParam.mStaggerParam);
    mStateBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mStateBindStarPointer, GET_NERVE(KuriboChief, KuriboChiefNrvBindStarPointer), "SupportFreeze");
    MR::initActorState(this, mStateStagger, GET_NERVE(KuriboChief, KuriboChiefNrvStagger), "Stagger");
}

void KuriboChief::initKeySwitch(const JMapInfoIter& rIter) {
    if (MR::isValidSwitchA(this)) {
        mKeySwitch = new KeySwitch("鍵スイッチ");
        mKeySwitch->initKeySwitchByOwner(rIter);
    }
}

void KuriboChief::makeArchiveList(NameObjArchiveListCollector* pCollector, const JMapInfoIter& rIter) {
    if (MR::isExistStageSwitchA(rIter)) {
        pCollector->addArchive("KeySwitch");
    }
}

void KuriboChief::kill() {
    MR::emitEffect(this, "Death");
    MR::startActionSound(this, "Explode", -1, -1, -1);

    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }

    if (mKeySwitch != nullptr) {
        mKeySwitch->appearKeySwitch(mPosition);
    }

    setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvDead));
    LiveActor::kill();
}

// NONMATCHING: epilogue restores r0 and r31 in the opposite order.
void KuriboChief::control() {
    if (mScaleController != nullptr) {
        mScaleController->updateNerve();
    }

    MR::blendQuatFromGroundAndFront(&_AC, this, _BC, ::sUpVecBlendRate, ::sFrontVecBlendRate);
    MR::changeShowModelFlagSyncNearClipping(this, 900.0f);
    fn_801BC140();
}

void KuriboChief::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, _AC);
    MR::updateBaseScale(this, mScaleController);
}

void KuriboChief::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (pSender != getSensor("Punch") &&
        ((isEnableAttack() || !MR::isSensorPlayer(pReceiver)) && !MR::isSensorEnemy(pReceiver) && !MR::isSensorYoshi(pReceiver) ||
         !MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) &&
        isEnableAttack() && MR::isSensorPlayer(pReceiver) && !MR::sendMsgEnemyAttack(pReceiver, pSender)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool KuriboChief::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        if (pReceiver != getSensor("Punch")) {
            if (!isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBindStarPointer))) {
                mScaleController->startHitReaction();
            }
            return true;
        }
        return false;
    }

    if (MR::isMsgPlayerHitProjectiles(msg)) {
        if (pReceiver != getSensor("Punch")) {
            return requestBlowDown(pSender, pReceiver);
        }
        return false;
    }

    if (MR::isMsgSupportTicoSpin(msg)) {
        if (isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvStagger))) {
            return false;
        }

        return requestStagger(pSender, pReceiver);
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        if (isEnableKick()) {
            return requestBlowDown(pSender, pReceiver);
        } else {
            return requestStagger(pSender, pReceiver);
        }
    }

    return false;
}

bool KuriboChief::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (pReceiver == getSensor("Punch")) {
        return false;
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        return requestStagger(pSender, pReceiver);
    }

    if (MR::isMsgToEnemyAttackShockWave(msg)) {
        return requestStagger(pSender, pReceiver);
    }

    return false;
}

bool KuriboChief::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (pReceiver == getSensor("Punch")) {
        return false;
    }

    if (MR::isMsgInhaleBlackHole(msg)) {
        kill();
        return true;
    }

    if (MR::isMsgPlayerKick(msg) && MR::isSensorPlayer(pSender) && isEnableKick() && requestBlowDown(pSender, pReceiver)) {
        return true;
    }

    return false;
}

bool KuriboChief::requestStagger(HitSensor* pSender, HitSensor* pReceiver) {
    if (isDown()) {
        mStateStagger->setPunchDirection(pSender, pReceiver);
        mStateStagger->_28 = false;
        setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvStagger));
        return true;
    }

    return false;
}

bool KuriboChief::requestBlowDown(HitSensor* pSender, HitSensor* pReceiver) {
    bool isDwn = fn_801BC910() == false;
    if (!isDwn) {
        return false;
    }

    MR::setVelocityBlowAttack(this, pSender, pReceiver, 25.0f, 35.0f, 4);
    setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBlowDown));
    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    return true;
}

// NONMATCHING: floating-point register allocation differs; instructions and field accesses agree.
void KuriboChief::fn_801BBEC0(const TVec3f& position, const TQuat4f& rotation, const TVec3f& velocity) {
    MR::setPosition(this, position);
    f32 x = rotation.x;
    f32 y = rotation.y;
    f32 z = rotation.z;
    f32 w = rotation.w;
    f32 vx = velocity.x;
    f32 vy = velocity.y;
    f32 vz = velocity.z;
    mVelocity.x = vx;
    mVelocity.y = vy;
    mVelocity.z = vz;
    _AC.x = x;
    _AC.y = y;
    _AC.z = z;
    _AC.w = w;
    _BC.set<f32>(2.0f * (x * z) + 2.0f * (w * y),
                 2.0f * (y * z) - 2.0f * (w * x),
                 (1.0f - 2.0f * (x * x)) - 2.0f * (y * y));
}

void KuriboChief::fn_801BBFB0(const TVec3f& position, const TQuat4f& rotation, const TVec3f& velocity) {
    fn_801BBEC0(position, rotation, velocity);
    setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander));
    appear();
}

void KuriboChief::fn_801BC000(const TVec3f& position, const TQuat4f& rotation, const TVec3f& velocity,
                           HitSensor* sender, HitSensor* receiver) {
    fn_801BBEC0(position, rotation, velocity);
    if (sender != nullptr) {
        mStateStagger->setPunchDirection(receiver, receiver);
    }
    mStateStagger->_28 = false;
    setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvStagger));
    MR::startAction(this, "Stagger");
    appear();
}

bool KuriboChief::tryFind() {
    if (mStateFindPlayer->isInSightPlayer()) {
        setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvFindPlayer));
        return true;
    }

    return false;
}

bool KuriboChief::tryPointBind() {
    if (mStateBindStarPointer->tryStartPointBind()) {
        _C8 = mStateStagger->isUpsideDown();
        setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBindStarPointer));
        return true;
    }

    return false;
}

bool KuriboChief::fn_801BC140() {
    TVec3f offset(0.0f, 0.0f, 0.0f);
    if (MR::isInDeath(this, offset) || MR::isBindedGroundDamageFire(this) || MR::isInWater(mPosition)) {
        kill();
        return true;
    }
    return false;
}

void KuriboChief::exeWander() {
    MR::updateActorState(this, mStateWander);
    if (!tryFind() && tryPointBind()) {
        return;
    }
}

void KuriboChief::exeFindPlayer() {
    if (!MR::updateActorStateAndNextNerve(this, mStateFindPlayer, GET_NERVE(KuriboChief, KuriboChiefNrvChase))) {
        if (mStateFindPlayer->isFindJumpBegin()) {
            MR::startActionSound(this, "EmKuribochiefFind");
        }

        if (mStateFindPlayer->isLandStart()) {
            MR::startActionSound(this, "EmKuribochiefLand");
        }

        if (tryPointBind()) {
            return;
        }
    }
}

void KuriboChief::exeChase() {
    if (MR::updateActorStateAndNextNerve(this, mStateChase, GET_NERVE(KuriboChief, KuriboChiefNrvWander))) {
        mStateWander->setWanderCenter(mPosition);
    }

    if (tryPointBind()) {
        return;
    }
}

void KuriboChief::exeStagger() {
    if (!MR::updateActorStateAndNextNerve(this, GET_NERVE(KuriboChief, KuriboChiefNrvWander))) {
        _C8 = mStateStagger->isUpsideDown();
        if (mStateStagger->isSwooning(20) && tryPointBind()) {
            return;
        }

        if (mStateStagger->isStaggerStart()) {
            MR::startActionSound(this, "EmKuribochiefBlow");
            MR::startBlowHitSound(this);
        }

        if (MR::isStep(this, ::sBoundSoundStep)) {
            MR::startActionSound(this, "EmKuribochiefBound");
        }

        if (mStateStagger->isSwooning(::sSwoonSoundWait)) {
            MR::startActionSound(this, "EmLvSwoonS");
        }

        if (mStateStagger->isSpinning(::sSpinSoundStartStep, ::sSpinSoundEndStep)) {
            MR::startActionSound(this, "EmLvKuribochiefStagger");
        }

        if (mStateStagger->isRecoverStart()) {
            MR::startActionSound(this, "EmKuribochiefRecover");
        }
    }
}

void KuriboChief::exeTrample() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "EmKuribochiefTrample");
    }

    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander));
    }
}

void KuriboChief::exeBindStarPointer() {
    if (MR::updateActorState(this)) {
        if (_C8) {
            mStateStagger->_28 = true;
            setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvStagger));
        } else {
            setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander));
        }
    }
}

void KuriboChief::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
    }

    MR::turnDirectionToPlayerDegree(this, &_BC, ::sTurnLimitDegree);
    WalkerStateFunction::calcPassiveMovement(this, &::sParam.mStateParam1);

    if (MR::isGreaterStep(this, ::sAttackSuccessTime)) {
        setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander));
    }

    if (tryPointBind()) {
        return;
    }
}

void KuriboChief::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
        MR::startBlowHitSound(this);
        MR::startActionSound(this, "EmKuribochiefBlow");
        MR::startActionSound(this, "SyVsBossLastHit");
    }

    WalkerStateFunction::calcPassiveMovement(this, &::sParam.mStateParam1);
    MR::turnDirectionDegree(this, &_BC, -mVelocity, 30.0f);

    if (MR::isGreaterStep(this, ::sBlowDownLandTime) && MR::isBindedGround(this)) {
        setNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBlowDownLand));
        MR::zeroVelocity(this);
    }
}

void KuriboChief::exeBlowDownLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDownLand");
        MR::startActionSound(this, "EmKuribochiefBoundLast", -1, -1, -1);
    }

    if (MR::isActionEnd(this)) {
        if (mKeySwitch == nullptr) {
            MR::emitItemHit(this);
        }

        kill();
    }
}

bool KuriboChief::isEnableAttack() const {
    if (isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander)) || isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvFindPlayer)) ||
        isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvChase))) {
        return true;
    }

    return false;
}

bool KuriboChief::isEnableKick() const {
    if (isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvStagger))) {
        return mStateStagger->isEnableKick();
    }

    if (isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBindStarPointer))) {
        return _C8;
    }

    return false;
}

bool KuriboChief::isDown() const {
    if (isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvWander)) || isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvFindPlayer)) ||
        isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvChase)) || isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvAttackSuccess)) ||
        isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvTrample)) || isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBindStarPointer))) {
        return true;
    }

    return false;
}

#pragma push
#pragma dont_inline on
bool KuriboChief::fn_801BC910() const {
    if (isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBlowDown)) ||
        isNerve(GET_NERVE(KuriboChief, KuriboChiefNrvBlowDownLand))) {
        return true;
    }
    return false;
}
#pragma pop
