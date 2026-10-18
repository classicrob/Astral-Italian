#include "Game/Enemy/Pyudon.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

class MapObjConnector;
class YoshiLockOnTarget;

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};  // namespace MR

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
};  // namespace YoshiFunction


// Unnamed SMG2 helpers (other units or this unit's helpers, see wip/Pyudon.renames.txt).
// PLACEHOLDER(enemies): unnamed .sbss2 constant/static (owner unit not named); resolve when the owner unit names it.
extern u32 lbl_807E3060;
// PLACEHOLDER(enemies): unnamed .sbss2 constant/static (owner unit not named); resolve when the owner unit names it.
extern u32 lbl_807E3064;
// PLACEHOLDER(enemies): unnamed .sbss2 constant/static (owner unit not named); resolve when the owner unit names it.
extern f32 lbl_807E3068;
// PLACEHOLDER(enemies): unnamed .sbss2 constant/static (owner unit not named); resolve when the owner unit names it.
extern f32 lbl_807E306C;
// PLACEHOLDER(enemies): unnamed .sbss2 constant/static (owner unit not named); resolve when the owner unit names it.
extern f32 lbl_807E3030;

namespace {
    static const f32 cNearPlayerDist = 1000.0f;
    static const f32 cStepUpSpeed = 0.75f;
}  // namespace

namespace NrvPyudon {
    NEW_NERVE(PyudonNrvWait, Pyudon, Wait);
    NEW_NERVE(PyudonNrvFind, Pyudon, Find);
    NEW_NERVE(PyudonNrvTrack, Pyudon, Track);
    NEW_NERVE(PyudonNrvLockOn, Pyudon, LockOn);
    NEW_NERVE(PyudonNrvAttack, Pyudon, Attack);
    NEW_NERVE(PyudonNrvJump, Pyudon, Jump);
    NEW_NERVE(PyudonNrvHipDropLand, Pyudon, HipDropLand);
    NEW_NERVE(PyudonNrvSwoon, Pyudon, Swoon);
    NEW_NERVE(PyudonNrvStarPieceReactionAir, Pyudon, StarPieceReactionAir);
    NEW_NERVE(PyudonNrvStarPieceReaction, Pyudon, StarPieceReaction);
    NEW_NERVE(PyudonNrvStarPieceReaction2, Pyudon, StarPieceReaction);
    NEW_NERVE(PyudonNrvSwoonLand, Pyudon, SwoonLand);
    NEW_NERVE(PyudonNrvSwoonEnd, Pyudon, SwoonEnd);
    NEW_NERVE(PyudonNrvRecovery, Pyudon, Recovery);
    NEW_NERVE(PyudonNrvRecoveryAir, Pyudon, RecoveryAir);
    NEW_NERVE(PyudonNrvStepDown, Pyudon, StepDown);
    NEW_NERVE(PyudonNrvBlowDown, Pyudon, BlowDown);
    class PyudonNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static PyudonNrvNone sInstance;
    };
    PyudonNrvNone PyudonNrvNone::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PyudonNrvDamage, Pyudon, Damage);
    NEW_NERVE(PyudonNrvDamageAir, Pyudon, DamageAir);
};  // namespace NrvPyudon

Pyudon::Pyudon(const char* pName) : LiveActor(pName) {
    mAnimScaleCtrl = nullptr;
    mBindStar1 = nullptr;
    mBindStar2 = nullptr;
    mCounter = 0;
    mStartPos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mBasePos.set(0.0f, 0.0f, 0.0f);
    mGroundPos.set(0.0f, 0.0f, 0.0f);
    mTargetPos.set(0.0f, 0.0f, 0.0f);
    mSpeedRate = 0.0f;
    mIsSwitchActive = 1;
    mGroundActor = nullptr;
    mConnector = nullptr;
    mYoshiLockOn = nullptr;
}

void Pyudon::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Pyudon", false);
    MR::initFurEnemy(this);
    mStartPos.set(mPosition);
    MR::calcFrontVec(&mFront, this);
    initNerve(GET_NERVE(Pyudon, PyudonNrvWait), 2);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mBindStar1 = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    mBindStar2 = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStar1, GET_NERVE(Pyudon, PyudonNrvDamage), nullptr);
    MR::initActorState(this, mBindStar2, GET_NERVE(Pyudon, PyudonNrvDamageAir), nullptr);
    mConnector = new MapObjConnector(this);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, nullptr, TVec3f(0.0f, 0.0f, 0.0f));
    makeActorAppeared();
}

void Pyudon::makeActorAppeared() {
    setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    MR::setPosition(this, mStartPos);
    LiveActor::makeActorAppeared();
}

void Pyudon::kill() {
    if (!isNerve(GET_NERVE(Pyudon, PyudonNrvNone))) {
        MR::tryHitReactionDead(this);
    }
    LiveActor::kill();
}

// fn_801F3840
void Pyudon::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        updateTargetPos();
        mSpeedRate = 0.0f;
        updateClipping();
    }

    if (MR::isGreaterStep(this, 60)) {
        bool isSwitchOn = false;
        if (MR::isValidSwitchA(this)) {
            if (MR::isOnSwitchA(this)) {
                setNerve(mIsSwitchActive ? static_cast< const Nerve* >(GET_NERVE(Pyudon, PyudonNrvFind)) : static_cast< const Nerve* >(GET_NERVE(Pyudon, PyudonNrvTrack)));
                isSwitchOn = true;
            }
        } else {
            TVec3f diff = -((*MR::getPlayerPos()) - mPosition);
            if (diff.length() < cNearPlayerDist) {
                setNerve(mIsSwitchActive ? static_cast< const Nerve* >(GET_NERVE(Pyudon, PyudonNrvFind)) : static_cast< const Nerve* >(GET_NERVE(Pyudon, PyudonNrvTrack)));
            }
        }
        (void)isSwitchOn;
    }

    mGroundPos.set(mStartPos);
    updateTargetPos();
}

// fn_801F3A00
void Pyudon::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
        mIsSwitchActive = 0;
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvTrack));
    }
}

// fn_801F3A70
void Pyudon::exeTrack() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        mCounter = 0;
        updateClipping();
    }

    TVec3f toPlayer = -((*MR::getPlayerPos()) - mPosition);
    f32 dot = toPlayer.dot(mGravity);
    TVec3f down(mGravity);
    down.scale(dot);
    TVec3f dir = -toPlayer;
    f32 len = dir.length();
    if (len < 200.0f && dot > 600.0f) {
        mCounter++;
        if (mCounter >= 30) {
            if (MR::isGreaterStep(this, 10)) {
                MR::invalidateClipping(this);
                setNerve(GET_NERVE(Pyudon, PyudonNrvLockOn));
            }
        }
    } else if (len > 600.0f) {
        mCounter = 0;
    }

    updateTargetPos();
    updateClipping();
    updateTargetPos();
    if (mIsSwitchActive) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
}

// fn_801F3C30
void Pyudon::exeLockOn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LockOn");
        mSpeedRate = 0.0f;
    } else if (MR::isActionEnd(this) && MR::isGreaterStep(this, static_cast< s32 >(lbl_807E3060))) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Pyudon, PyudonNrvAttack));
    }

    mSpeedRate = MR::calcNerveEaseOutValue(this, 60, 0.0f, lbl_807E3068);
    if (MR::isGreaterStep(this, 80)) {
        MR::getPlayerGroundPos(&mGroundPos);
    }
    updateTargetPos();
    if (MR::isLessStep(this, 80)) {
        reboundVelocityFromTargetDir(0.0f, 0.0f);
    } else {
        MR::zeroVelocity(this);
    }
}

// fn_801F3D70
void Pyudon::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Attack");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvJump));
    }
}

// fn_801F3DE0
void Pyudon::exeJump() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Jump");
        mBasePos.set(mPosition);
    }
    bool isAirborne = MR::isGreaterEqualStep(this, 15);
    if (isAirborne) {
        MR::addVelocityToGravity(this, 1.3f);
    }
    if (MR::isBindedGround(this)) {
        if (MR::sendMsgToGroundSensor(4, getSensor("Pyudon")) != 0) {
            MR::zeroVelocity(this);
            MR::emitEffect(this, "Pyudon");
        }
        clearGroundActor();
        setNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand));
    }
    TVec3f d = mPosition;
    if (d.length() > 2500.0f) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Pyudon, PyudonNrvStepDown));
    }
}

// fn_801F3F00
void Pyudon::exeHipDropLand() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "HipDropLand");
    }
    if (MR::isOnGround(this)) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Pyudon, PyudonNrvSwoon));
    } else {
        if (isGroundActorAlive()) {
            MR::addVelocityToGravity(this, 1.2f);
        }
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvStepDown));
    }
}

// fn_801F3FB0
void Pyudon::exeSwoon() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Swoon");
        clearGroundActor();
    }
    if (MR::isOnGround(this)) {
        MR::zeroVelocity(this);
    } else if (isGroundActorAlive()) {
        MR::addVelocityToGravity(this, 1.2f);
    }
    if (MR::isGreaterStep(this, 115)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvSwoonLand));
    }
}

// fn_801F4070
void Pyudon::exeStarPieceReactionAir() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "StarPieceReactionAir");
        clearGroundActor();
    }
    if (MR::isGreaterStep(this, 115)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
}

// fn_801F40F0
void Pyudon::exeStarPieceReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceReaction");
        clearGroundActor();
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Swoon");
    }
    if (MR::isGreaterStep(this, 115)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvSwoonLand));
    }
    if (MR::isOnGround(this)) {
        MR::zeroVelocity(this);
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2))) {
            setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2));
        }
    } else {
        if (isGroundActorAlive()) {
            MR::addVelocityToGravity(this, 1.2f);
        }
    }
    MR::attenuateVelocity(this, 0.98f);
}

// fn_801F41F0
void Pyudon::exeSwoonLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonLand");
        clearGroundActor();
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd));
    }
    if (MR::isOnGround(this)) {
        MR::zeroVelocity(this);
    }
    if (isGroundActorAlive()) {
        MR::addVelocityToGravity(this, 1.2f);
    }
}

// fn_801F42A0
void Pyudon::exeSwoonEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonEnd");
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvRecovery));
    }
    MR::isOnGround(this);
}

// fn_801F4310
void Pyudon::exeRecovery() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recovery");
    } else if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
    MR::getPlayerGroundPos(&mGroundPos);
    updateTargetPos();
    reboundVelocityFromTargetDir(0.0f, lbl_807E306C);
    damperVelocity(0.0f, 0.0f);
}

// fn_801F43B0
void Pyudon::exeRecoveryAir() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RecoveryAir");
    } else if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
    MR::getPlayerGroundPos(&mGroundPos);
    updateTargetPos();
    reboundVelocityFromTargetDir(0.0f, lbl_807E306C);
    damperVelocity(0.0f, 0.0f);
}

// fn_801F4450
void Pyudon::exeStepDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StepDown");
    }
    if (MR::isActionEnd(this) || MR::isGreaterStep(this, 45)) {
        MR::emitItemStomp(this);
        kill();
    }
}

// fn_801F44E0
void Pyudon::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
    }
    TVec3f zero(0.0f, 0.0f, 0.0f);
    MR::turnDirectionDegree(this, &mFront, zero, 30.0f);
    if (MR::isBindedGround(this) && MR::isGreaterStep(this, 15)) {
        MR::emitItemHit(this);
        kill();
    }
}

// fn_801F45A0
void Pyudon::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensor(this, "Damage");
    }
    if (MR::updateActorStateAndNextNerve(this, GET_NERVE(Pyudon, PyudonNrvWait))) {
        MR::invalidateHitSensor(this, "Damage");
    }
}

// fn_801F4610
void Pyudon::exeDamageAir() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensor(this, "Damage");
    }
    if (MR::updateActorState(this)) {
        MR::invalidateHitSensor(this, "Damage");
        TVec3f up(mGravity);
        up.scale(30.0f);
        TVec3f side(mGravity);
        side.scale(200.0f);
        TVec3f pos = mPosition - mGravity;
        TVec3f hitPos;
        if (!MR::getFirstPolyOnLineToMap(&hitPos, nullptr, pos, side)) {
            setNerve(GET_NERVE(Pyudon, PyudonNrvDamage));
        }
        setNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
    }
}

// fn_801F4C40
bool Pyudon::isHipDropNerve() const {
    return isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand)) || isNerve(GET_NERVE(Pyudon, PyudonNrvSwoon)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) || isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction)) || isNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
}

// fn_801F4CF0
bool Pyudon::isAttackNerve() const {
    return isNerve(GET_NERVE(Pyudon, PyudonNrvLockOn)) || isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvJump)) || isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvSwoon)) || isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd)) || isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2)) || isNerve(GET_NERVE(Pyudon, PyudonNrvDamage)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
}

// fn_801F4E10
bool Pyudon::isFindNerve() const {
    return isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvTrack)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvLockOn)) || isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvJump)) || isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvSwoon)) ||
           (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) && MR::isGreaterStep(this, 60));
}

// fn_801F4F60
bool Pyudon::isBindMapObjNerve() const {
    return isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand)) || isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonLand)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvSwoon)) || isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
}

// fn_801F5000
void Pyudon::receiveMsgPlayerTrample(u32 msg, HitSensor* pSender) {
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) || isNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown))) {
        return;
    }
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvTrack))) {
        return;
    }
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) || isNerve(GET_NERVE(Pyudon, PyudonNrvRecovery)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir)) || isNerve(GET_NERVE(Pyudon, PyudonNrvDamage))) {
        return;
    }
    MR::zeroVelocity(this);
    MR::tryHitReaction(this, "\x93\xa5\x82\xdc\x82\xea\x83q\x83b\x83g");  // hit sound (string lbl_806905D8)
    ScoreAttackFunction::scoreByAttack(this, msg, 0);
    setNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown));
}

// fn_5150
bool Pyudon::receiveMsgSpinAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) || isNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown))) {
        return false;
    }
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvTrack))) {
        return false;
    }
    MR::zeroVelocity(this);
    MR::tryHitReaction(this, "\x90\x81\x82\xab\x94\xf2\x82\xd1\x83q\x83b\x83g");  // lbl_806905E8
    MR::fn_80012B90(this, 30.0f, 30.0f, pSender, pReceiver, 4);
    ScoreAttackFunction::scoreByAttack(this, msg, 0);
    setNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown));
    return true;
}

// fn_5270
bool Pyudon::receiveMsgKick(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isHipDropNerve() || isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir))) {
        MR::zeroVelocity(this);
        MR::tryHitReaction(this, "\x83L\x83b\x83N\x83q\x83b\x83g");  // lbl_806905F8
        MR::fn_80012B90(this, 30.0f, 30.0f, pSender, pReceiver, 4);
        ScoreAttackFunction::scoreByAttack(this, msg, 0);
        setNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown));
        return true;
    }
    return false;
}

// fn_5330
bool Pyudon::receiveMsgStarPiece(HitSensor* pSender, HitSensor* pReceiver) {
    bool isApproach = isFindNerve();
    if (isApproach || isNerve(GET_NERVE(Pyudon, PyudonNrvTrack)) || isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvDamage))) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir));
    }
    if (isAttackNerve()) {
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvRecovery)) || isNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir))) {
            if (MR::isGreaterStep(this, 35)) {
                setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir));
            } else {
                setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2));
            }
        }
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) || isNerve(GET_NERVE(Pyudon, PyudonNrvJump))) {
            MR::setVelocitySeparateHV(this, pSender, pReceiver, 18.0f, 12.0f);
            setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2));
        }
    }
    setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction));
    return true;
}

// fn_54E0
bool Pyudon::receiveMsgYoshiTongue() {
    if (isAttackNerve()) {
        MR::invalidateHitSensors(this);
        setNerve(GET_NERVE(Pyudon, PyudonNrvNone));
        return true;
    }
    return false;
}

// fn_5540
void Pyudon::updateBindStarPointer() {
    if (isFindNerve()) {
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind))) {
            if (mBindStar1->tryStartPointBind()) {
                setNerve(GET_NERVE(Pyudon, PyudonNrvDamage));
            }
        }
    }
    if (mBindStar2->tryStartPointBind()) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
    }
}

// fn_5650: rebound the velocity away from the target direction (approximate).
void Pyudon::reboundVelocityFromTargetDir(f32 farg0, f32 farg1) {
    TVec3f toTarget = -mTargetPos;
    f32 d = toTarget.dot(mGravity);
    TVec3f vert(mGravity);
    vert.scale(d);
    TVec3f horiz = toTarget - vert;
    f32 len = horiz.length();
    MR::reboundVelocityFromCollision(this, farg0 * MR::normalizeAbs(len, 1.0f, 1.0f), farg1, 1.0f);
}

// fn_5750: target position = ground position + up * (450) + up * speed rate.
void Pyudon::updateTargetPos() {
    TVec3f up(0.0f, 0.0f, 0.0f);
    MR::getPlayerUpVec(&up);
    TVec3f s(up);
    s.scale(mSpeedRate);
    TVec3f b(up);
    b.scale(450.0f);
    TVec3f c(mGroundPos);
    c.add(b);
    TVec3f d(c);
    d.add(s);
    mTargetPos = d;
}

// fn_5810
void Pyudon::updateVelocityOnGround() {
    f32 friction = MR::isOnGround(this) ? 0.8f : 0.9f;
    MR::attenuateVelocity(this, friction);
    if (MR::isOnGround(this) == false) {
        MR::addVelocityToGravity(this, 1.3f);
    }
    if (MR::isBindedWall(this)) {
        TVec3f n = MR::getWallNormal(this);
        MR::calcReboundVelocity(&mVelocity, n, 1.0f);
    }
}

// fn_58A0
void Pyudon::updateClipping() {
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            MR::invalidateClipping(this);
        }
    }
    if (MR::calcDistanceToPlayer(this) < 1000.0f) {
        MR::invalidateClipping(this);
    }
    MR::validateClipping(this);
}

// fn_5920: damps the velocity along gravity and the horizontal plane.
void Pyudon::damperVelocity(f32 farg0, f32 farg1) {
    TVec3f vert(mGravity);
    vert.scale(-mVelocity.dot(mGravity));
    TVec3f horiz(mVelocity);
    horiz.add(vert);
    horiz.scale(farg0);
    mVelocity.scale(farg1, vert);
    mVelocity.add(horiz);
}

// fn_59D0: remembers the ground actor under the pyudon.
LiveActor* Pyudon::tryFindGroundActor() {
    LiveActor* pGround = MR::getGroundActor(this);
    mGroundActor = pGround;
    if (pGround != nullptr) {
        mConnector->fn_802CC510();
    }
    return pGround;
}

// fn_5A20
bool Pyudon::isGroundActorAlive() {
    if (mGroundActor == nullptr) {
        return false;
    }
    bool isDead = MR::isDead(mGroundActor);
    if (!isDead && MR::isValidCollisionParts(mGroundActor)) {
        return true;
    }
    mConnector->fn_802CC5D0();
    mGroundActor = nullptr;
    return false;
}

// fn_5AA0
void Pyudon::clearGroundActor() {
    if (mGroundActor == nullptr) {
        return;
    }
    mConnector->fn_802CC5D0();
    mGroundActor = nullptr;
}

// fn_801F4DE0 / control
void Pyudon::control() {
    mAnimScaleCtrl->updateNerve();
    if (isAttackNerve()) {
        if (mYoshiLockOn != nullptr) {
            YoshiFunction::tryYoshiEat(mYoshiLockOn);
        }
    }
    updateBindStarPointer();
}

// calcAndSetBaseMtx
void Pyudon::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScaleCtrl);
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    if (isBindMapObjNerve()) {
        mConnector->connect();
    }
    clearGroundActor();
}

// attackSensor
void Pyudon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayer(pReceiver)) {
            if (!isHipDropNerve() && !MR::isPlayerHipDropFalling() && !MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else if (MR::isSensorPlayerOrRide(pReceiver)) {
        bool isFind = isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
                      isNerve(GET_NERVE(Pyudon, PyudonNrvTrack)) || isNerve(GET_NERVE(Pyudon, PyudonNrvLockOn));
        if (!isFind && !isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) && !isNerve(GET_NERVE(Pyudon, PyudonNrvAttack))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else {
        if (MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

// receiveMsgPlayerAttack
bool Pyudon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Damage")) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            receiveMsgPlayerTrample(msg, pSender);
            return true;
        }
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            return receiveMsgSpinAttack(msg, pSender, pReceiver);
        }
        if (MR::isMsgLockOnStarPieceShoot(msg)) {
        }
        if (MR::isMsgStarPieceAttack(msg)) {
            return receiveMsgStarPiece(pSender, pReceiver);
        }
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        return receiveMsgStarPiece(pSender, pReceiver);
    }
    return false;
}

// receiveOtherMsg
bool Pyudon::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Damage") && MR::isMsgPlayerKick(msg)) {
        return receiveMsgKick(msg, pSender, pReceiver);
    }
    if (isAttackNerve()) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            return receiveMsgYoshiTongue();
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        kill();
    }
    if (MR::isMsgYoshiCancelEaten(msg)) {
        kill();
    }
    return false;
}

// receiveMsgPush
bool Pyudon::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    bool isNoPush = isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) || isNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown));
    if (!isNoPush && !MR::isSensorPlayerOrRide(pSender)) {
        MR::addVelocityFromPush(this, 4.0f, pSender, pReceiver);
    }
    return true;
}

// fn_801F5AA0-ish: dtor
Pyudon::~Pyudon() {}
