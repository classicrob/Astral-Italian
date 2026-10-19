#pragma auto_inline off
#include "Game/Enemy/Pattan.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BattanKingCollisionPress.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#define BATCH_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Pattan*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

namespace NrvPattan {
    BATCH_NERVE(PattanNrvAppear, exePattanAppear);
    BATCH_NERVE(PattanNrvWait, exePattanWait);
    BATCH_NERVE(PattanNrvFind, exePattanFind);
    BATCH_NERVE(PattanNrvChase, exePattanChase);
    NEW_NERVE(PattanNrvAttackSign, Pattan, AttackSign);
    NEW_NERVE(PattanNrvAttack, Pattan, Attack);
    BATCH_NERVE(PattanNrvDown, exePattanDown);
    BATCH_NERVE(PattanNrvJump, exePattanJump);
    BATCH_NERVE(PattanNrvGuard, exePattanGuard);
    BATCH_NERVE(PattanNrvDead, exePattanDead);
    BATCH_NERVE(PattanNrvPush, exePattanPush);
    BATCH_NERVE(PattanNrvPop, exePattanPop);
    NEW_NERVE(PattanNrvFreeze, Pattan, Freeze);
};  // namespace NrvPattan

#define GET_PATTAN_NERVE(name) (&NrvPattan::name::sInstance)

extern const volatile s32 gPattanWaitMaximum;
extern const volatile s32 gPattanAttackSignDuration;
extern const volatile f32 gPattanAppearJumpSpeed;
extern const volatile f32 gPattanJumpSpeed;

static const MR::ActorMoveParam sMove = {1.6f, 2.0f, 0.9f, 1.5f};
static const MR::ActorMoveParam sJumpMove = {0.0f, 2.0f, 0.9f, 1.5f};

static TVec3f sRayOffsetA(300.0f, 500.0f, 500.0f);
static TVec3f sRayOffsetB(300.0f, 500.0f, 750.0f);
static TVec3f sBreakOffset(0.0f, 0.0f, 200.0f);

Pattan::Pattan(const char* pName) : LiveActor(pName) {
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mBreakModel = nullptr;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mTimer = 0;
    mDamageTimer = 0;
    PSMTXIdentity(mBaseMtx.toMtxPtr());
}

void Pattan::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Pattan", false);
    initNerve(GET_PATTAN_NERVE(PattanNrvWait), 1);

    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_PATTAN_NERVE(PattanNrvFreeze), "SupportFreeze");

    PSMTXCopy(MR::getJointMtx(this, "Root"), mBaseMtx.toMtxPtr());
    mBreakModel = new BreakModel(this, "パタン壊れモデル", "PattanBreak", mBaseMtx.toMtxPtr(), "Break");
    mBreakModel->initWithoutIter();
    MR::invalidateClipping(this);
    makeActorDead();
}

void Pattan::makeActorAppeared() {
    PSMTXCopy(MR::getJointMtx(this, "Root"), mBaseMtx.toMtxPtr());
    setNerve(GET_PATTAN_NERVE(PattanNrvWait));
    MR::onBind(this);
    LiveActor::makeActorAppeared();
    mDamageTimer = 0x708;
}

void appearPattanByJump(LiveActor* pActor, const TVec3f* pFront) {
    pActor->LiveActor::appear();
    MR::calcGravity(pActor);
    static_cast<Pattan*>(pActor)->mFront = *pFront;
    pActor->setNerve(GET_PATTAN_NERVE(PattanNrvAppear));
}

void Pattan::control() {
    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        kill();
        return;
    }
    if (MR::isPressedRoofAndGround(this)) {
        tryPattanPop();
        return;
    }
    if (canPattanBindStarPointer() && mStarPointBind->tryStartPointBind()) {
        setNerve(GET_PATTAN_NERVE(PattanNrvFreeze));
        return;
    }
    s32 timer = mDamageTimer;
    if (timer > 0) {
        mDamageTimer = timer - 1;
    }
}

void Pattan::exePattanAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::setVelocityJump(this, gPattanAppearJumpSpeed);
    }
    MR::attenuateVelocity(this, sMove._8);
    MR::addVelocityToGravity(this, sMove._4);
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvFind));
    }
}

void Pattan::exePattanWait() {
    if (MR::isFirstStep(this)) {
        mTimer = MR::getRandom(60, gPattanWaitMaximum);
        MR::startAction(this, "Wait");
        MR::onBind(this);
        MR::zeroVelocity(this);
    }
    if (MR::isGreaterEqualStep(this, mTimer)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvFind));
    }
}

void Pattan::exePattanFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
        MR::onBind(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvChase));
    }
}

void Pattan::exePattanChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Run");
    }
    if (MR::isPlayerInCollisionBox(reinterpret_cast<const TPos3f*>(getBaseMtx()), &sRayOffsetA) || mDamageTimer < 0) {
        MR::zeroVelocity(this);
        setNerve(GET_PATTAN_NERVE(PattanNrvAttackSign));
        MR::offBind(this);
        return;
    }
    if (MR::isGreaterStep(this, 0x258)) {
        if (!MR::isPlayerInCollisionBox(reinterpret_cast<const TPos3f*>(getBaseMtx()), &sRayOffsetB)) {
            MR::zeroVelocity(this);
            setNerve(GET_PATTAN_NERVE(PattanNrvWait));
        }
    }
    MR::moveAndTurnToPlayer(this, &mFront, sMove._0, sMove._4, sMove._8, sMove._C);
}

void Pattan::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
        MR::offBind(this);
    }
    if (MR::isGreaterEqualStep(this, gPattanAttackSignDuration)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvAttack));
    }
}

void Pattan::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvDown));
    }
}

void Pattan::exePattanDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
    }
    if (MR::isActionEnd(this)) {
        MR::tryHitReactionBreak(this);
        PSMTXCopy(MR::getJointMtx(this, "Root"), mBaseMtx.toMtxPtr());
        MR::addTransMtxLocal(mBaseMtx.toMtxPtr(), sBreakOffset);
        mBreakModel->mAnimName = "Break";
        mBreakModel->appear();
        kill();
    }
}

void Pattan::exePattanJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Jump");
        MR::setVelocityJump(this, gPattanJumpSpeed);
    }
    MR::attenuateVelocity(this, sMove._8);
    MR::addVelocityToGravity(this, sMove._4);
    MR::moveAndTurnToPlayer(this, &mFront, sJumpMove._0, sJumpMove._4, sJumpMove._8, sJumpMove._C);
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvChase));
    }
}

void Pattan::exePattanGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
    }
    MR::attenuateVelocity(this, sMove._8);
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvWait));
    }
}

void Pattan::exePattanDead() {
    if (MR::isFirstStep(this)) {
        PSMTXCopy(MR::getJointMtx(this, "Root"), mBaseMtx.toMtxPtr());
        mBreakModel->mAnimName = "BreakPress";
        mBreakModel->appear();
        MR::zeroVelocity(this);
        MR::startAction(this, "PressDown");
    }
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        MR::tryHitReactionDead(this);
        kill();
    }
}

void Pattan::exePattanPush() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReactionBreak(this);
        PSMTXCopy(MR::getJointMtx(this, "Root"), mBaseMtx.toMtxPtr());
        MR::addTransMtxLocal(mBaseMtx.toMtxPtr(), sBreakOffset);
        mBreakModel->mAnimName = "Break";
        mBreakModel->appear();
        kill();
    }
}

void Pattan::exePattanPop() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReactionBreak(this);
        PSMTXCopy(MR::getJointMtx(this, "Root"), mBaseMtx.toMtxPtr());
        mBreakModel->mAnimName = "BreakPress";
        mBreakModel->appear();
        kill();
    }
}

void Pattan::tryPattanJump() {
    if (isNerve(GET_PATTAN_NERVE(PattanNrvWait)) || isNerve(GET_PATTAN_NERVE(PattanNrvFind)) ||
        isNerve(GET_PATTAN_NERVE(PattanNrvChase)) || isNerve(GET_PATTAN_NERVE(PattanNrvAttackSign))) {
        setNerve(GET_PATTAN_NERVE(PattanNrvJump));
    }
}

void Pattan::calcAndSetBaseMtx() {
    MR::calcAndSetBaseMtxFromFrontAndGravity(this, &mFront);
}

void Pattan::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (isNerve(GET_PATTAN_NERVE(PattanNrvAttack))) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                tryPattanPop();
            }
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else {
        if (MR::isSensorEnemy(pReceiver) || MR::isSensorEnemyAttack(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool Pattan::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (canPattanPushMove()) {
        MR::addVelocityFromPush(this, 1.5f, pSender, pReceiver);
        return true;
    }
    return false;
}

bool Pattan::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
        tryPattanPressDown();
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        if (tryPattanGuard(pSender, pReceiver)) {
            return true;
        }
    }
    if (MR::isMsgStarPieceReflect(msg)) return true;
    return false;
}

bool Pattan::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    (void)pSender;
    (void)pReceiver;
    if (msg == 0xF4) {
        tryPattanPop();
        return true;
    }
    if (MR::isMsgInhaleBlackHole(msg)) {
        tryPattanPop();
    }
    return false;
}

bool Pattan::isPattanAliveState() {
    bool result = false;
    if (!isNerve(GET_PATTAN_NERVE(PattanNrvDown)) && !isNerve(GET_PATTAN_NERVE(PattanNrvDead)) &&
    !isNerve(GET_PATTAN_NERVE(PattanNrvPop))) result = true;
    return result;
}

bool Pattan::canPattanPushMove() {
    bool result = false;
    if (!isNerve(GET_PATTAN_NERVE(PattanNrvAttack)) && !isNerve(GET_PATTAN_NERVE(PattanNrvDown)) &&
    !isNerve(GET_PATTAN_NERVE(PattanNrvPop)) && !isNerve(GET_PATTAN_NERVE(PattanNrvDead))) result = true;
    return result;
}

bool Pattan::canPattanBindStarPointer() {
    bool result = false;
    if (!isNerve(GET_PATTAN_NERVE(PattanNrvDown)) && !isNerve(GET_PATTAN_NERVE(PattanNrvDead)) &&
    !isNerve(GET_PATTAN_NERVE(PattanNrvPop)) && !isNerve(GET_PATTAN_NERVE(PattanNrvFreeze))) result = true;
    return result;
}

bool Pattan::tryPattanPop() {
    if (isPattanAliveState()) {
        this->setNerve(GET_PATTAN_NERVE(PattanNrvPop));
        return true;
    }
    return false;
}

bool Pattan::tryPattanPressDown() {
    if (isNerve(GET_PATTAN_NERVE(PattanNrvAttack)) && MR::isGreaterStep(this, 0xA)) {
        this->setNerve(GET_PATTAN_NERVE(PattanNrvPush));
        return true;
    }
    if (isPattanAliveState()) {
        MR::tryHitReaction(this, "踏み");
        this->setNerve(GET_PATTAN_NERVE(PattanNrvDead));
        return true;
    }
    return false;
}

bool Pattan::tryPattanGuard(HitSensor* pSender, HitSensor* pReceiver) {
    if (isPattanAliveState() && !isNerve(GET_PATTAN_NERVE(PattanNrvGuard))) {
        MR::tryHitReaction(this, "ガード");
        MR::addVelocityFromPushHorizon(this, 15.0f, pSender, pReceiver);
        this->setNerve(GET_PATTAN_NERVE(PattanNrvGuard));
        return true;
    }
    return false;
}

Pattan::~Pattan() {
}

void Pattan::exeFreeze() {
    MR::updateActorStateAndNextNerve(this, GET_PATTAN_NERVE(PattanNrvWait));
}


extern const volatile s32 gPattanWaitMaximum = 0;

extern const volatile s32 gPattanAttackSignDuration = 0;

extern const volatile f32 gPattanAppearJumpSpeed = 0.0f;

extern const volatile f32 gPattanJumpSpeed = 0.0f;
