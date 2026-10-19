#pragma auto_inline off
#include "Game/Enemy/PackunPetit.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/NativeActorObserve.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include "JSystem/JMath/JMath.hpp"

namespace NrvPackunPetit {
    NEW_NERVE_ONEND(PackunPetitNrvNonActive, PackunPetit, NonActive, NonActive);
    NEW_NERVE(PackunPetitNrvWait, PackunPetit, Wait);
    NEW_NERVE(PackunPetitNrvFind, PackunPetit, Find);
    NEW_NERVE(PackunPetitNrvThreat, PackunPetit, Threat);
    NEW_NERVE(PackunPetitNrvLeftTurn, PackunPetit, Turn);
    NEW_NERVE(PackunPetitNrvRightTurn, PackunPetit, Turn);
    NEW_NERVE(PackunPetitNrvAttackStart, PackunPetit, AttackStart);
    NEW_NERVE(PackunPetitNrvAttack, PackunPetit, Attack);
    NEW_NERVE(PackunPetitNrvAttackBack, PackunPetit, AttackBack);
    NEW_NERVE(PackunPetitNrvHitWaitForAttack, PackunPetit, HitWaitForAttack);
    NEW_NERVE(PackunPetitNrvHit, PackunPetit, Hit);
    NEW_NERVE(PackunPetitNrvTrampleDown, PackunPetit, TrampleDown);
    NEW_NERVE(PackunPetitNrvPunchDown, PackunPetit, PunchDown);
    NEW_NERVE(PackunPetitNrvSwoonStart, PackunPetit, SwoonStart);
    NEW_NERVE(PackunPetitNrvSwoon, PackunPetit, Swoon);
    NEW_NERVE(PackunPetitNrvSwoonToThreat, PackunPetit, SwoonToThreat);
    class PackunPetitNrvDPDSwoon : public Nerve {
    public:
        virtual void execute(Spine* spine) const {
            PackunPetit* actor = static_cast<PackunPetit*>(spine->mExecutor);
            if (MR::updateActorState(actor)) actor->selectNrvWait();
        }
        static PackunPetitNrvDPDSwoon sInstance;
    };
    PackunPetitNrvDPDSwoon PackunPetitNrvDPDSwoon::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PackunPetitNrvPullStart, PackunPetit, DPDSwoon);
#define PACKUN_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<PackunPetit*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;
    PACKUN_NERVE(PackunPetitNrvPull, exeYoshiPull);
    PACKUN_NERVE(PackunPetitNrvPullEnd, exeYoshiPullEnd);
    PACKUN_NERVE(PackunPetitNrvEaten, exePetitEaten);
};  // namespace NrvPackunPetit

PackunPetit::PackunPetit(const char* pName)
    : LiveActor(pName), mScaleController(nullptr), mStarPointerState(nullptr), mFront(0.0f, 0.0f, 1.0f), mBlownModel(nullptr), mDontTurn(false), mUseGroundMovement(false), mYoshiLockOn(nullptr), mLastPlayerPos(0.0f, 0.0f, 0.0f) {
}

namespace {
    struct TurnParam {
        /* 0x00 */ f32 mMoveAccel;
        /* 0x04 */ f32 mGravityAccel;
        /* 0x08 */ f32 mVelocityDamping;
        /* 0x0C */ f32 mTurnMaxDegree;
    };

    const TurnParam sTurnParam = {0.0f, 3.0f, 0.98f, 0.0f};
};  // namespace

void PackunPetit::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "PackunPetit", false);
    MR::addEffectHitNormal(this, nullptr);
    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    mStarPointerState = new WalkerStateBindStarPointer(this, mScaleController);
    MR::getJMapInfoArg0NoInit(rIter, &mDontTurn);
    MR::getJMapInfoArg7NoInit(rIter, &mUseGroundMovement);
    if (mUseGroundMovement) initBinder(50.0f, 50.0f, 0);
    MR::calcFrontVec(&mFront, this);
    initBlowModel();
    initNerve(GET_NERVE(PackunPetit, PackunPetitNrvWait), 1);
    MR::initActorState(this, mStarPointerState, GET_NERVE(PackunPetit, PackunPetitNrvDPDSwoon), "SupportFreeze");
    if (MR::isValidSwitchDead(this)) MR::disableAllItemGenerator(this);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, mJointMtx.toMtxPtr(), TVec3f(0.0f));
    makeActorAppeared();
}

void PackunPetit::initAfterPlacement() {
    MR::initEffectAfterPlacement(this, nullptr);
}

void PackunPetit::exeNonActive() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::offCalcAnim(this);
        MR::offCalcShadow(this, "JawA");
    }

    if (MR::isNearPlayerAnyTime(this, 5000.0f)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvWait));
    }
}

void PackunPetit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startBckWithInterpole(this, "Wait", 60);
    }

    if (MR::isNearPlayer(this, 1700.0f)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvFind));
    } else {
        tryNonActive();
    }
}

void PackunPetit::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Suspect");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvThreat));
    }
}

void PackunPetit::exeThreat() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Threat");
    }

    MR::startActionSound(this, "EmLvPackunpetitThreat");

    if (!mDontTurn) {
        if (tryTurn()) {
            return;
        }

        TVec3f* pos = MR::getPlayerPos();
        MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, *pos, 2.0f);
    }

    if (MR::isGreaterStep(this, 50) && MR::isNearPlayer(this, 1100.0f) && (MR::isFaceToPlayerHorizontalDegree(this, mFront, 10.0f) || mDontTurn)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvAttackStart));
    } else if (!MR::isNearPlayer(this, 2000.0f)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvWait));
    }
}

void PackunPetit::exeTurn() {
    if (MR::isFirstStep(this)) {
        if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvLeftTurn))) {
            MR::startAction(this, "TurnLeft");
        } else {
            MR::startAction(this, "TurnRight");
        }
    }

    f32 step = (10.0f - getNerveStep());

    if (step < 2.0f) {
        step = 2.0f;
    }
    TVec3f* pos = MR::getPlayerPos();
    MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, *pos, step);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvThreat));
    }
}

void PackunPetit::exeAttackStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }

    if (MR::isStep(this, 70)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvAttack));
    }
}

void PackunPetit::exeHitWaitForAttack() {
    if (MR::isFirstStep(this) && !MR::isBckPlaying(this, "Attack")) {
        MR::startAction(this, "Impact");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvHit));
    }
}

void PackunPetit::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
    }

    if (MR::isActionEnd(this) && !tryTurn()) {
        selectNrvWait();
    }
}

void PackunPetit::exeTrampleDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        MR::startActionSound(this, "EmStompedS");
        MR::invalidateHitSensors(this);
    }

    if (MR::isActionEnd(this)) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitItemStomp(this);
        kill();
    }
}

void PackunPetit::exePunchDown() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensor(this, "Body");
        MR::invalidateHitSensor(this, "attack");
        TPos3f mtx;
        PSMTXCopy(getBaseMtx(), mtx);
        setBlownModelPosture(mtx, 1.0f);
        MR::copyJointPos(this, "Head", &mBlownModel->mPosition);
        mBlownModel->makeActorAppeared();
        MR::onCalcGravity(mBlownModel);
        MR::startAction(mBlownModel, "BlowHead");
        MR::startAction(this, "Blow");
    }
    mBlownModel->mVelocity.scaleAdd(2.5f, mBlownModel->mGravity, mBlownModel->mVelocity);
    if (!MR::isHiddenModel(mBlownModel) && (MR::isStep(this, 20) || MR::checkStrikeBallToMap(mBlownModel->mPosition, 50.0f))) {
        MR::emitEffect(mBlownModel, "Death");
        MR::startActionSound(mBlownModel, "EmExplodeS");
        MR::emitItemHitAtPos(this, &mBlownModel->mPosition);
        MR::hideModel(mBlownModel);
    }
    if (MR::isStep(this, 30)) kill();
}

void PackunPetit::exeSwoonStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonStart");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoon));
    }
}

void PackunPetit::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
    }

    MR::startActionSound(this, "EmLvSwoonS");

    if (MR::isStep(this, 250)) {
        selectNrvWait();
    }
}

void PackunPetit::exeSwoonToThreat() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonToThreat");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvThreat));
    }
}

void PackunPetit::exeDPDSwoon() {
    if (MR::isOnGroundPlayer()) {
        mLastPlayerPos.set(*MR::getPlayerPos());
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull));
    }
}

// NONMATCHING: compiler folds target boolean materialization into a direct branch.
void PackunPetit::exeYoshiPull() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pull");
        MR::invalidateClipping(this);
    }
    MR::calcVecToPlayerH(&mFront, this, nullptr);
    f32 distance = PSVECDistance(&mLastPlayerPos, &mPosition);
    bool isFar = !MR::isNearPlayer(this, 75.0f + distance);
    if (isFar) {
        MR::tryHitReaction(this, "ヨッシー舌引き抜き");
        MR::invalidateClipping(this);
        MR::setVelocity(mBlownModel, mFront * 15.0f);
        MR::addVelocity(mBlownModel, mGravity * -40.0f);
        ScoreAttackFunction::score100Points(this);
        startPunchDown();
    }
}

void PackunPetit::exeYoshiPullEnd() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "PullEnd");
    }
    if (MR::isActionEnd(this)) { selectNrvWait(); return; }
    YoshiFunction::tryYoshiPull(mYoshiLockOn);
}

// NONMATCHING: compiler emits an aggregate-copy loop instead of paired-single matrix copies; stack allocation differs.
void PackunPetit::exePetitEaten() {
    if (MR::isStep(this, 0)) {
        MR::hideModel(this);
        MR::tryHitReactionDead(this);
        MR::invalidateHitSensor(this, "Body");
        MR::invalidateHitSensor(this, "attack");
        mBlownModel->makeActorAppeared();
        TPos3f yoshi;
        MR::calcYoshiTongueTipMtx(&yoshi);
        yoshi.invert(yoshi);
        TPos3f head;
        head.setInline(MR::getJointMtx(this, "Head"));
        f32 scale = MR::getYoshiTongueScale();
        setBlownModelPosture(head, scale);
        mPullMtx.concat(yoshi, head);
    }
    if (MR::isGreaterStep(this, 0)) {
        TPos3f yoshi;
        MR::calcYoshiTongueTipMtx(&yoshi);
        TPos3f mtx;
        mtx.concat(yoshi, mPullMtx);
        f32 scale = MR::getYoshiTongueScale();
        setBlownModelPosture(mtx, scale);
    }
    if (MR::isStep(this, 1)) MR::startAction(this, "Blow");
}

void PackunPetit::makeActorDead() {
    LiveActor::makeActorDead();
    if (!MR::isDead(mBlownModel)) mBlownModel->makeActorDead();
}

void PackunPetit::kill() {
    if (!MR::isDead(mBlownModel)) {
        if (!MR::isHiddenModel(mBlownModel)) {
            MR::emitEffect(mBlownModel, "Death");
            MR::startActionSound(mBlownModel, "EmExplodeS", -1, -1, -1);
        }

        mBlownModel->kill();
    }

    MR::tryHitReactionDead(this);
    LiveActor::kill();
}

void PackunPetit::calcAndSetBaseMtx() {
    MR::calcAndSetBaseMtxFromFront(this, &mFront);
    MR::updateBaseScale(this, mScaleController);
}

void PackunPetit::control() {
    PSMTXCopy(MR::getJointMtx(this, "Head"), mJointMtx.toMtxPtr());
    MR::addTransMtxLocalX(mJointMtx.toMtxPtr(), -10.0f);

    mScaleController->updateNerve();
    tryDPDSwoon();

    if (mUseGroundMovement) {
        MR::moveAndTurnToPlayer(this, &mFront, sTurnParam.mMoveAccel, sTurnParam.mGravityAccel, sTurnParam.mVelocityDamping, sTurnParam.mTurnMaxDegree);
    }

    if (!isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) && !isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) && !isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten)) && !isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) && !isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown))) {
        MR::copyJointPos(this, "Head", &mBlownModel->mPosition);
        YoshiFunction::tryYoshiPull(mYoshiLockOn);
    }
}

// NONMATCHING: one extra boolean register move remains.
// NONMATCHING: the `pushing` bool is copied to r3 before its compare (target compares r31 directly); the rest matches.
void PackunPetit::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        bool dying = false;
        if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown))) dying = true;
        if (!dying) {
            bool pulled = false;
            if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) pulled = true;
            if (!pulled && !isNerve(GET_NERVE(PackunPetit, PackunPetitNrvDPDSwoon)) && MR::isSensorEnemyAttack(pSender)) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender) && !MR::isPlayerHipDropFalling()) {
                    MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);
                    setNerve(GET_NERVE(PackunPetit, PackunPetitNrvHitWaitForAttack));
                    return;
                }
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
        }
    }
    bool pushing = isNerve(GET_NERVE(PackunPetit, PackunPetitNrvDPDSwoon));
    if (!pushing) {
        bool pulled = false;
        if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) pulled = true;
        pushing = pulled;
    }
    if (pushing && MR::isSensorPlayer(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }
    if (MR::isSensorEnemy(pReceiver) || MR::isSensorYoshi(pReceiver)) MR::sendMsgPush(pReceiver, pSender);
}

bool PackunPetit::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isTrampleOrPunch = isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown));

    if (isTrampleOrPunch) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return isTongueHittableNerve();
    }

    if (MR::isMsgStarPieceAttack(msg) && isTongueHittableNerve()) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoonStart));
        return true;
    }

    bool isWaitOrHit = isNerve(GET_NERVE(PackunPetit, PackunPetitNrvHitWaitForAttack)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvHit));

    if (isWaitOrHit) {
        return false;
    }

    if (MR::isMsgInvincibleAttack(msg) || MR::isMsgFireBallAttack(msg) || MR::isMsgRockAttack(msg)) {
        punchDown(msg, pSender, pReceiver);
        return true;
    }

    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvAttack))) {
        return false;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        MR::tryRumbleDefaultHit(this, 0);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        startTrampleDown();
        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::tryRumbleDefaultHit(this, 0);
        ScoreAttackFunction::scoreByAttack(this, msg, nullptr);
        startTrampleDown();
        return true;
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        MR::stopSceneForDefaultHit(5);
        punchDown(msg, pSender, pReceiver);
        return true;
    }

    return false;
}

bool PackunPetit::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool isTrampleOrPunch = isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown));

    if (isTrampleOrPunch) {
        return false;
    }

    if (MR::isMsgToEnemyAttackBlow(msg)) {
        punchDown(msg, pSender, pReceiver);
        return true;
    }

    if (MR::isMsgToEnemyAttackTrample(msg)) {
        startTrampleDown();
        return true;
    }

    return false;
}

bool PackunPetit::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg) && isTongueGrabbableNerve()) {
        MR::tryHitReaction(this, "ヨッシー舌アクセス");
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart));
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        bool valid = false;
        if (!isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) && !isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown))) valid = true;
        return valid;
    }
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull))) {
            MR::fn_804222D0();
            setNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullEnd));
        } else if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) {
            MR::tryHitReaction(this, "ヨッシー捕食");
            makeActorDead();
        } else {
            MR::fn_804222D0();
            setNerve(GET_NERVE(PackunPetit, PackunPetitNrvWait));
        }
        return true;
    }

    bool isSwoon = isNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoonStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoon));

    if (!isSwoon) {
        return false;
    }

    if (!MR::isSensorPlayer(pSender)) {
        return false;
    }

    if (MR::isMsgPlayerKick(msg)) {
        MR::stopSceneForDefaultHit(5);
        punchDown(msg, pSender, pReceiver);
        return true;
    }

    return false;
}

void PackunPetit::startTrampleDown() {
    bool pulled = false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) pulled = true;
    if (pulled) MR::fn_804222D0();
    setNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown));
}

void PackunPetit::startPunchDown() {
    bool pulled = false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) pulled = true;
    if (pulled) MR::fn_804222D0();
    setNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown));
}

void PackunPetit::initBlowModel() {
    mBlownModel = MR::createModelObjMapObjStrongLight("吹っ飛びモデル", "PackunPetitHead", nullptr);
    mBlownModel->initWithoutIter();
    MR::initShadowVolumeSphere(mBlownModel, 70.0f);
    MR::invalidateClipping(mBlownModel);
    MR::initLightCtrl(mBlownModel);
    mBlownModel->makeActorDead();
}

void PackunPetit::punchDown(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    TVec3f v6;
    v6.sub(*MR::getSensorPos(pReceiver), *MR::getSensorPos(pSender));
    v6.orthogonalize(mGravity);
    MR::normalize(&v6);

    TVec3f v5;
    v5.scale(20.0f, v6);
    v5.scaleAdd(-40.0f, mGravity, v5);
    mBlownModel->mVelocity.set(v5);
    ScoreAttackFunction::scoreByAttack(this, msg, pSender);
    MR::startBlowHitSound(this);
    startPunchDown();
}

void PackunPetit::selectNrvWait() {
    if (!MR::isNearPlayer(this, 1700.0f)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvWait));
    } else if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoon))) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoonToThreat));
    } else {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvThreat));
    }
}

bool PackunPetit::tryNonActive() {
    if (MR::isNearPlayerAnyTime(this, 5000.0f)) {
        return false;
    }

    setNerve(GET_NERVE(PackunPetit, PackunPetitNrvNonActive));
    return true;
}

bool PackunPetit::tryTurn() {
    if (mDontTurn) {
        return false;
    }

    if (MR::isFaceToPlayerHorizontalDegree(this, mFront, 90.0f)) {
        return false;
    }
    TVec3f playerDist;
    playerDist.sub(*MR::getPlayerPos(), mPosition);
    TVec3f sideVec;
    MR::calcSideVec(&sideVec, this);

    if (playerDist.dot(sideVec) > 0.0f) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvLeftTurn));
    } else {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvRightTurn));
    }

    return true;
}

bool PackunPetit::tryDPDSwoon() {
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvDPDSwoon))) {
        return false;
    }

    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart))) return false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull))) return false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) return false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvSwoonStart)) && MR::isLessStep(this, 60)) return false;

    bool isTrampleOrPunch = isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown));

    if (isTrampleOrPunch) {
        return false;
    }

    if (!mStarPointerState->tryStartPointBind()) {
        return false;
    }

    setNerve(GET_NERVE(PackunPetit, PackunPetitNrvDPDSwoon));
    return true;
}

bool PackunPetit::isTongueGrabbableNerve() const {
    bool dying = false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown))) dying = true;
    if (dying || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) return false;
    return true;
}

// NONMATCHING: branch sense/return-block layout of the nested dying/pulled tests (the single-exit form scores worse).
bool PackunPetit::isTongueHittableNerve() const {
    bool dying = false;
    if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvTrampleDown)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPunchDown))) dying = true;
    if (!dying) {
        bool pulled = false;
        if (isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPullStart)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvPull)) || isNerve(GET_NERVE(PackunPetit, PackunPetitNrvEaten))) pulled = true;
        if (!pulled) return true;
    }
    return false;
}

// NONMATCHING: argument load scheduling differs.
void PackunPetit::setBlownModelPosture(const TPos3f& mtx, f32 scale) {
    ModelObj* model = mBlownModel;
    MR::setPosition(model, mtx(0,3), mtx(1,3), mtx(2,3));
    mtx.getEuler(mBlownModel->mRotation);
    TVec3f& rotation = mBlownModel->mRotation;
    rotation.x *= 57.295776f;
    rotation.y *= 57.295776f;
    rotation.z *= 57.295776f;
    MR::setScale(mBlownModel, scale);
}

void PackunPetit::endNonActive() {
    MR::validateHitSensors(this);
    MR::onCalcAnim(this);
    MR::onCalcShadow(this, "JawA");
}

void PackunPetit::exeAttack() {
    if (MR::isStep(this, 12)) {
        setNerve(GET_NERVE(PackunPetit, PackunPetitNrvAttackBack));
    }
}

void PackunPetit::exeAttackBack() {
    if (MR::isActionEnd(this)) {
        if (!tryTurn()) {
            selectNrvWait();
        }
    }
}

