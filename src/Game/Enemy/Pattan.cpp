#include "Game/Enemy/Pattan.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace NrvPattan {
    NEW_NERVE(PattanNrvAppear, Pattan, Appear);
    NEW_NERVE(PattanNrvWait, Pattan, Wait);
    NEW_NERVE(PattanNrvFind, Pattan, Find);
    NEW_NERVE(PattanNrvChase, Pattan, Chase);
    NEW_NERVE(PattanNrvAttackSign, Pattan, AttackSign);
    NEW_NERVE(PattanNrvAttack, Pattan, Attack);
    NEW_NERVE(PattanNrvDown, Pattan, Down);
    NEW_NERVE(PattanNrvJump, Pattan, Jump);
    NEW_NERVE(PattanNrvGuard, Pattan, Guard);
    NEW_NERVE(PattanNrvDead, Pattan, Dead);
    NEW_NERVE(PattanNrvPush, Pattan, Push);
    NEW_NERVE(PattanNrvPop, Pattan, Pop);
    NEW_NERVE(PattanNrvFreeze, Pattan, Freeze);
};  // namespace NrvPattan

#define GET_PATTAN_NERVE(name) (&NrvPattan::name::sInstance)

// Constants from SMG2 .sbss2 / .sdata2 (values from the DOL).
extern "C" s32 lbl_807E3050;
extern "C" s32 lbl_807E3054;
extern "C" f32 lbl_807E3058;
extern "C" f32 lbl_807E305C;

// Unnamed SMG2 helpers used by Pattan.
extern "C" bool fn_8008E060(const TPos3f*, const TVec3f*);
extern "C" void fn_800345E0(LiveActor*, const char*);
extern "C" void fn_800348C0(LiveActor*);
extern "C" void fn_8022BAA0(LiveActor*);
extern "C" void fn_80034910(LiveActor*);
extern "C" void fn_801E6F00(LiveActor*, const TVec3f*);
extern "C" void fn_8000F880(const TVec3f*);
extern "C" bool fn_801E7A80(LiveActor*);
extern "C" bool fn_801E7B00(LiveActor*);
extern "C" bool fn_801E7B90(LiveActor*);
extern "C" bool fn_801E7C20(LiveActor*);
extern "C" bool fn_801E7C70(LiveActor*);
extern "C" bool fn_801E7D10(LiveActor*, HitSensor*, HitSensor*);

// Zero offsets for the ground probes (SMG2 lbl_80729470 / lbl_8072947C).
static TVec3f sRayOffsetA(0.0f, 0.0f, 0.0f);
static TVec3f sRayOffsetB(0.0f, 0.0f, 0.0f);

// Nerve-set checks (fn_801E7A80 ... fn_801E7B90).
static bool isPattanNerve(const LiveActor* pActor, const Nerve* pNerve) {
    return pActor->isNerve(pNerve);
}

Pattan::Pattan(const char* pName) : LiveActor(pName) {
    mAnimScaleCtrl = nullptr;
    mStarPointBind = nullptr;
    mBreakModel = nullptr;
    mFront.x = 0.0f;
    mFront.y = 0.0f;
    mFront.z = 1.0f;
    mTimer = 0;
    mDamageTimer = 0;
}

Pattan::~Pattan() {
}

void Pattan::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Pattan", nullptr, false);
    initNerve(GET_PATTAN_NERVE(PattanNrvWait), 1);

    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mStarPointBind = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mStarPointBind, GET_PATTAN_NERVE(PattanNrvFreeze), "SupportFreeze");

    mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
    mBreakModel = new BreakModel(this, "パッタン壊れモデル", "PattanBreak", mBaseMtx.toMtxPtr(), "Break");
    mBreakModel->initWithoutIter();
    MR::invalidateClipping(this);
    makeActorDead();
}

void Pattan::makeActorAppeared() {
    mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
    setNerve(GET_PATTAN_NERVE(PattanNrvWait));
    MR::onBind(this);
    LiveActor::makeActorAppeared();
    mDamageTimer = 0x708;
}

// Appear-by-jump setup used by the spawner (fn_801E6F00).
extern "C" void fn_801E6F00(LiveActor* pActor, const TVec3f* pFront) {
    MR::calcGravity(pActor);
    pActor->LiveActor::appear();
    static_cast<Pattan*>(pActor)->mFront.z = pFront->z;
    pActor->setNerve(GET_PATTAN_NERVE(PattanNrvAppear));
}

void Pattan::control() {
    if (MR::isInDeath(this, TVec3f(0.0f))) {
        kill();
        return;
    }
    if (MR::isPressedRoofAndGround(this)) {
        fn_801E7C20(this);
        return;
    }
    if (fn_801E7B90(this) && mStarPointBind->tryStartPointBind()) {
        setNerve(GET_PATTAN_NERVE(PattanNrvFreeze));
        return;
    }
    s32 timer = mDamageTimer;
    if (timer > 0) {
        mDamageTimer = timer - 1;
    }
}

void Pattan::calcAndSetBaseMtx() {
    fn_8000F880(&mFront);
}

// NONMATCHING: the SMG2 push/attack sensor flow is approximated from the draft.
void Pattan::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver)) {
        if (isNerve(GET_PATTAN_NERVE(PattanNrvAttack))) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                fn_801E7C20(this);
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
    if (fn_801E7B00(this)) {
        MR::addVelocityFromPush(this, 1.5f, pSender, pReceiver);
        return true;
    }
    return false;
}

bool Pattan::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDrop(msg) || MR::isMsgPlayerTrample(msg)) {
        fn_801E7C70(this);
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        if (fn_801E7D10(this, pSender, pReceiver)) {
            return true;
        }
    }
    return MR::isMsgStarPieceReflect(msg);
}

bool Pattan::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    (void)pSender;
    (void)pReceiver;
    if (msg == 0xF4) {
        fn_801E7C20(this);
        return true;
    }
    if (MR::isMsgInhaleBlackHole(msg)) {
        fn_801E7C20(this);
    }
    return false;
}

// True when the Pattan is not in Attack/Dead/Pop (fn_801E7A80).
extern "C" bool fn_801E7A80(LiveActor* pActor) {
    return !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvDown)) && !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvDead)) &&
           !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvPop));
}

// True when the Pattan is not in Attack/Down/Pop/Dead (fn_801E7B00).
extern "C" bool fn_801E7B00(LiveActor* pActor) {
    return !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvAttack)) && !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvDown)) &&
           !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvPop)) && !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvDead));
}

// True when the Pattan is not in Down/Dead/Pop/Freeze (fn_801E7B90).
extern "C" bool fn_801E7B90(LiveActor* pActor) {
    return !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvDown)) && !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvDead)) &&
           !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvPop)) && !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvFreeze));
}

// Enters the Pop nerve when the hit is allowed (fn_801E7C20).
extern "C" bool fn_801E7C20(LiveActor* pActor) {
    if (fn_801E7A80(pActor)) {
        pActor->setNerve(GET_PATTAN_NERVE(PattanNrvPop));
        return true;
    }
    return false;
}

// Attack/hit reaction chooser (fn_801E7C70).
extern "C" bool fn_801E7C70(LiveActor* pActor) {
    if (isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvAttack)) && MR::isGreaterStep(pActor, 0xA)) {
        pActor->setNerve(GET_PATTAN_NERVE(PattanNrvPush));
        return true;
    }
    if (fn_801E7A80(pActor)) {
        fn_800345E0(pActor, "ヒップドロップ");
        pActor->setNerve(GET_PATTAN_NERVE(PattanNrvDead));
        return true;
    }
    return false;
}

// Push-back on a horizontal hit (fn_801E7D10).
extern "C" bool fn_801E7D10(LiveActor* pActor, HitSensor* pSender, HitSensor* pReceiver) {
    if (fn_801E7A80(pActor) && !isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvGuard))) {
        fn_800345E0(pActor, "ヒップドロップ");
        MR::addVelocityFromPushHorizon(pActor, 15.0f, pSender, pReceiver);
        pActor->setNerve(GET_PATTAN_NERVE(PattanNrvGuard));
        return true;
    }
    return false;
}

// Moves to Freeze when the nerve is in the attack group (fn_801E77A0).
extern "C" void fn_801E77A0(LiveActor* pActor) {
    if (isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvAppear)) || isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvWait)) ||
        isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvFind)) || isPattanNerve(pActor, GET_PATTAN_NERVE(PattanNrvChase))) {
        pActor->setNerve(GET_PATTAN_NERVE(PattanNrvJump));
    }
}

void Pattan::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::setVelocityJump(this, lbl_807E3058);
        MR::startAction(this, "Appear");
    }
    MR::attenuateVelocity(this, 0.9f);
    MR::addVelocityToGravity(this, 2.0f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvFind));
    }
}

void Pattan::exeWait() {
    if (MR::isFirstStep(this)) {
        mTimer = MR::getRandom(60, lbl_807E3050);
        MR::zeroVelocity(this);
        MR::onBind(this);
        MR::startAction(this, "Wait");
    }
    if (MR::isGreaterEqualStep(this, mTimer)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvFind));
    }
}

void Pattan::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::onBind(this);
        MR::startAction(this, "Find");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvChase));
    }
}

void Pattan::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }
    mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
    if (!fn_8008E060(&mBaseMtx, &sRayOffsetA) || mDamageTimer < 0) {
        MR::zeroVelocity(this);
        MR::offBind(this);
        setNerve(GET_PATTAN_NERVE(PattanNrvAttackSign));
        return;
    }
    if (MR::isGreaterStep(this, 0x258)) {
        mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
        if (!fn_8008E060(&mBaseMtx, &sRayOffsetB)) {
            MR::zeroVelocity(this);
            setNerve(GET_PATTAN_NERVE(PattanNrvWait));
        }
    }
    MR::moveAndTurnToPlayer(this, &mFront, 1.6f, 2.0f, 0.9f, 1.5f);
}

void Pattan::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSign");
        MR::offBind(this);
    }
    if (MR::isGreaterStep(this, lbl_807E3054)) {
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

void Pattan::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackEnd");
    }
    if (MR::isActionEnd(this)) {
        fn_800348C0(this);
        mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
        mBreakModel->appear();
        kill();
    }
}

void Pattan::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::setVelocityJump(this, lbl_807E305C);
        MR::startAction(this, "Jump");
    }
    MR::attenuateVelocity(this, 0.9f);
    MR::addVelocityToGravity(this, 2.0f);
    MR::moveAndTurnToPlayer(this, &mFront, 0.0f, 2.0f, 0.9f, 1.5f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvChase));
    }
}

void Pattan::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
    }
    MR::attenuateVelocity(this, 0.9f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_PATTAN_NERVE(PattanNrvWait));
    }
}

void Pattan::exeDead() {
    if (MR::isActionEnd(this)) {
        fn_8022BAA0(this);
        fn_80034910(this);
        kill();
    }
}

void Pattan::exePush() {
    if (MR::isFirstStep(this)) {
        fn_800348C0(this);
        mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
        mBreakModel->appear();
        kill();
    }
}

void Pattan::exePop() {
    if (MR::isFirstStep(this)) {
        fn_800348C0(this);
        mBaseMtx = TPos3f(MR::getJointMtx(this, "Root"));
        mBreakModel->appear();
        kill();
    }
}

// Freeze (star pointer) state: hands control to the state object.
void Pattan::exeFreeze() {
    MR::updateActorStateAndNextNerve(this, GET_PATTAN_NERVE(PattanNrvWait));
}
