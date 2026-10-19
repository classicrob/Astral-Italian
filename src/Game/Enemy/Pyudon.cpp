#include "Game/Enemy/Pyudon.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#include "Game/Util/YoshiLockOnUtil.hpp"

// Zero tuning consts the target loads from .sbss2 at runtime instead of folding them.
const volatile s32 cLockOnEndStep = 0;
const volatile s32 cJumpGravityStep = 0;
const volatile f32 cLockOnEaseEnd = 0.0f;
const volatile f32 cDamageHeightRate = 0.0f;

namespace {
    static const f32 cNearPlayerDist = 1000.0f;
    static const f32 cStepUpSpeed = 0.75f;
    extern const f32 cBlowHorizontal;
    extern const f32 cBlowVertical;
    extern const f32 cLockOnHeightAcceleration;
}  // namespace

#define PYUDON_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Pyudon*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

namespace NrvPyudon {
    PYUDON_NERVE(PyudonNrvWait, exePyudonWait);
    NEW_NERVE(PyudonNrvFind, Pyudon, Find);
    PYUDON_NERVE(PyudonNrvTrack, exeTrack);
    PYUDON_NERVE(PyudonNrvLockOn, exePyudonLockOn);
    PYUDON_NERVE(PyudonNrvAttack, exePyudonAttack);
    PYUDON_NERVE(PyudonNrvJump, exePyudonHipDrop);
    PYUDON_NERVE(PyudonNrvHipDropLand, exeHipDropLand);
    PYUDON_NERVE(PyudonNrvSwoon, exePyudonSwoon);
    PYUDON_NERVE(PyudonNrvStarPieceReactionAir, exeStarPieceReactionAir);
    PYUDON_NERVE(PyudonNrvStarPieceReaction, exePyudonStarPieceReaction);
    PYUDON_NERVE(PyudonNrvStarPieceReaction2, exePyudonStarPieceReaction);
    PYUDON_NERVE(PyudonNrvSwoonLand, exePyudonSwoonLand);
    NEW_NERVE(PyudonNrvSwoonEnd, Pyudon, SwoonEnd);
    PYUDON_NERVE(PyudonNrvRecovery, exeRecovery);
    PYUDON_NERVE(PyudonNrvRecoveryAir, exeRecoveryAir);
    NEW_NERVE(PyudonNrvStepDown, Pyudon, StepDown);
    PYUDON_NERVE(PyudonNrvBlowDown, exePyudonBlowDown);
    class PyudonNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static PyudonNrvNone sInstance;
    };
    PyudonNrvNone PyudonNrvNone::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PyudonNrvDamage, Pyudon, Damage);
    PYUDON_NERVE(PyudonNrvDamageAir, exeDamageAir);
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
    mStartPos = mPosition;
    MR::calcFrontVec(&mFront, this);
    initNerve(GET_NERVE(Pyudon, PyudonNrvWait), 2);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mBindStar1 = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    mBindStar2 = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStar1, GET_NERVE(Pyudon, PyudonNrvDamage), nullptr);
    MR::initActorState(this, mBindStar2, GET_NERVE(Pyudon, PyudonNrvDamageAir), nullptr);
    mConnector = new MapObjConnector(this);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
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

// exePyudonWait
void Pyudon::exePyudonWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        updatePyudonClipping();
        mSpeedRate = 0.0f;
        releaseGroundActor();
    }
    if (MR::isGreaterStep(this, 60)) {
        if (MR::isValidSwitchA(this)) {
            if (MR::isOnSwitchA(this)) {
                if (mIsSwitchActive) setNerve(GET_NERVE(Pyudon, PyudonNrvFind));
                else setNerve(GET_NERVE(Pyudon, PyudonNrvTrack));
            }
        } else {
            TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
            if (toPlayer.length() < 1000.0f) {
                TVec3f toStart = *MR::getPlayerPos() - mStartPos;
                bool far = toStart.length() > 2500.0f;
                if (!far) {
                    if (mIsSwitchActive) setNerve(GET_NERVE(Pyudon, PyudonNrvFind));
                    else setNerve(GET_NERVE(Pyudon, PyudonNrvTrack));
                }
            }
        }
    }
    mGroundPos = mStartPos;
    updateTargetPos();
    updateVelocityToTarget(MR::isGreaterStep(this, 60) ? 0.75f : 0.0f, 0.6f);
    dampVelocity(this, 0.97f, 0.98f);
    TVec3f camera = MR::getCamZdir();
    MR::vecBlendSphere(&mFront, mFront, -camera, 0.05f);
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

// exeTrack
// NONMATCHING: stack slot order of the gravity temporary and distance vector.
void Pyudon::exeTrack() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        mCounter = 0;
        updatePyudonClipping();
    }
    TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
    f32 dot = toPlayer.dot(mGravity);
    TVec3f horizontal = toPlayer - mGravity * dot;
    f32 length = horizontal.length();
    if (length < 200.0f && dot > 100.0f) {
        mCounter++;
        if (mCounter >= 30 && MR::isGreaterStep(this, 10)) {
            setNerve(GET_NERVE(Pyudon, PyudonNrvLockOn));
            MR::invalidateClipping(this);
        }
    } else if (length > 600.0f) mCounter = 0;
    bool far = (*MR::getPlayerPos() - mStartPos).length() > 2500.0f;
    if (far) setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    MR::getPlayerGroundPos(&mGroundPos);
    updateTargetPos();
    updateVelocityToTarget(0.75f, 0.6f);
    dampVelocity(this, 0.97f, 0.98f);
    TVec3f camera = MR::getCamZdir();
    MR::vecBlendSphere(&mFront, mFront, -camera, 0.05f);
}

// exePyudonLockOn
// NONMATCHING: target loads two separate 1.1f literal entries (fmr f2,f1 here).
void Pyudon::exePyudonLockOn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LockOn");
        mSpeedRate = 0.0f;
    } else if (MR::isActionEnd(this) && MR::isGreaterStep(this, cLockOnEndStep)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvAttack));
        MR::zeroVelocity(this);
    }
    mSpeedRate = MR::calcNerveEaseOutValue(this, 0, 60, 0.0f, cLockOnEaseEnd);
    if (!MR::isGreaterStep(this, 80)) MR::getPlayerGroundPos(&mGroundPos);
    updateTargetPos();
    if (MR::isLessStep(this, 80)) {
        dampVelocity(this, 0.8f, 0.7f);
        updateVelocityToTarget(1.1f, cLockOnHeightAcceleration);
    } else MR::zeroVelocity(this);
    TVec3f camera = MR::getCamZdir();
    MR::vecBlendSphere(&mFront, mFront, -camera, 0.05f);
}

// exePyudonAttack
void Pyudon::exePyudonAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropStart");
        MR::zeroVelocity(this);
        return;
    }
    if (MR::isActionEnd(this)) setNerve(GET_NERVE(Pyudon, PyudonNrvJump));
}

// exePyudonHipDrop
void Pyudon::exePyudonHipDrop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDrop");
        mBasePos = mPosition;
    }
    if (MR::isGreaterEqualStep(this, cJumpGravityStep)) MR::addVelocityToGravity(this, 9.0f);
    if (MR::isBindedGround(this)) {
        if (MR::sendMsgToGroundSensor(4, getSensor("Body"))) {
            MR::emitEffect(this, "LandSmokePyudon");
        } else {
            updateGroundActor();
            setNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand));
            return;
        }
        MR::zeroVelocity(this);
    }
    if ((mPosition - mBasePos).length() > 2500.0f) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir));
        MR::zeroVelocity(this);
    }
}

// exeHipDropLand
void Pyudon::exeHipDropLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropLand");
        MR::zeroVelocity(this);
    }
    if (MR::isOnGround(this)) {
        updateGroundActor();
        MR::zeroVelocity(this);
    } else if (tryReleaseGroundActor()) MR::addVelocityToGravity(this, 1.2f);
    if (MR::isActionEnd(this)) setNerve(GET_NERVE(Pyudon, PyudonNrvSwoon));
}

// exePyudonSwoon
void Pyudon::exePyudonSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Swoon");
        MR::zeroVelocity(this);
        releaseGroundActor();
    }
    if (MR::isOnGround(this)) {
        updateGroundActor();
        MR::zeroVelocity(this);
    } else if (tryReleaseGroundActor()) {
        MR::addVelocityToGravity(this, 1.2f);
    }
    if (MR::isGreaterStep(this, 115)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd));
    }
}

// exeStarPieceReactionAir
void Pyudon::exeStarPieceReactionAir() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceReactionAir");
        MR::zeroVelocity(this);
        releaseGroundActor();
    }
    if (MR::isGreaterStep(this, 115)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
}

// exePyudonStarPieceReaction
void Pyudon::exePyudonStarPieceReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceReaction");
        releaseGroundActor();
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Swoon");
    }
    if (MR::isGreaterStep(this, 115)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd));
    }
    MR::attenuateVelocity(this, 0.99f);
    if (MR::isOnGround(this)) {
        updateGroundActor();
        MR::zeroVelocity(this);
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2))) {
            setNerve(GET_NERVE(Pyudon, PyudonNrvSwoonLand));
        }
    } else {
        if (tryReleaseGroundActor()) {
            MR::addVelocityToGravity(this, 1.2f);
        }
    }
}

// exePyudonSwoonLand
void Pyudon::exePyudonSwoonLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonLand");
        releaseGroundActor();
    }
    if (MR::isActionEnd(this)) setNerve(GET_NERVE(Pyudon, PyudonNrvSwoon));
    if (MR::isOnGround(this)) {
        updateGroundActor();
        MR::zeroVelocity(this);
        return;
    }
    if (tryReleaseGroundActor()) MR::addVelocityToGravity(this, 1.2f);
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

// exeRecovery
void Pyudon::exeRecovery() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recovery");
    } else if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
    MR::getPlayerGroundPos(&mGroundPos);
    updateTargetPos();
    updateVelocityToTarget(0.0f, cDamageHeightRate);
    dampVelocity(this, 0.97f, 0.98f);
}

// exeRecoveryAir
void Pyudon::exeRecoveryAir() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RecoveryAir");
    } else if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvWait));
    }
    MR::getPlayerGroundPos(&mGroundPos);
    updateTargetPos();
    updateVelocityToTarget(0.0f, cDamageHeightRate);
    dampVelocity(this, 0.97f, 0.98f);
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

// exePyudonBlowDown
void Pyudon::exePyudonBlowDown() {
    if (MR::isFirstStep(this)) MR::startAction(this, "BlowDown");
    MR::turnDirectionDegree(this, &mFront, -mVelocity, 30.0f);
    updateBlownVelocity();
    if ((MR::isBindedGround(this) && MR::isGreaterStep(this, 15)) || MR::isGreaterStep(this, 35)) {
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

// exeDamageAir
// NONMATCHING: target builds both gravity temporaries before the start vector (instruction order only).
void Pyudon::exeDamageAir() {
    if (MR::isFirstStep(this)) MR::validateHitSensor(this, "Damage");
    if (MR::updateActorState(this)) {
        MR::invalidateHitSensor(this, "Damage");
        TVec3f upOffset = mGravity * 100.0f;
        TVec3f ray = mGravity * 200.0f;
        TVec3f start = mPosition - upOffset;
        if (!MR::getFirstPolyOnLineToMap(nullptr, nullptr, start, ray)) {
            setNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir));
            return;
        }
        setNerve(GET_NERVE(Pyudon, PyudonNrvRecovery));
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

// isStarPointBindable
bool Pyudon::isStarPointBindable() const {
    bool result = false;
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvTrack)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvLockOn)) || isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvJump)) || isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvSwoon)) ||
        (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) && MR::isGreaterStep(this, 60)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd)) ||
        (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction)) && MR::isGreaterStep(this, 60)) ||
        (isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2)) && MR::isGreaterStep(this, 60))) result = true;
    return result;
}

// fn_801F4F60
bool Pyudon::isBindMapObjNerve() const {
    return isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand)) || isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonLand)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvSwoon)) || isNerve(GET_NERVE(Pyudon, PyudonNrvSwoonEnd)) ||
           isNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
}

// tryStepDown
bool Pyudon::tryStepDown(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool blocked = false;
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) || isNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown))) {
        blocked = true;
    }
    if (!blocked) {
        bool waiting = false;
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvTrack))) {
            waiting = true;
        }
        if (!waiting) {
            if (!(isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) || isNerve(GET_NERVE(Pyudon, PyudonNrvRecovery)) ||
            isNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir)) || isNerve(GET_NERVE(Pyudon, PyudonNrvDamage)))) {
                MR::tryHitReaction(this, "\x93\xa5\x82\xdc\x82\xea\x83q\x83\x62\x83g");
                MR::zeroVelocity(this);
                ScoreAttackFunction::scoreByAttack(this, msg, 0);
                setNerve(GET_NERVE(Pyudon, PyudonNrvStepDown));
                return true;
            }
        }
    }
    return false;
}

// fn_5150
bool Pyudon::tryBlowDownSpin(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    bool blocked = false;
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) || isNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown))) {
        blocked = true;
    }
    if (!blocked) {
        bool waiting = false;
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
            isNerve(GET_NERVE(Pyudon, PyudonNrvTrack))) {
            waiting = true;
        }
        if (!waiting) {
            MR::tryHitReaction(this, "\x90\x81\x82\xab\x94\xf2\x82\xd1\x83q\x83\x62\x83g");  // lbl_806905E8
            MR::zeroVelocity(this);
            MR::setVelocityBlowAttack(this, pSender, pReceiver, cBlowHorizontal, cBlowVertical, 4);
            ScoreAttackFunction::scoreByAttack(this, msg, 0);
            setNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown));
            return true;
        }
    }
    return false;
}

// fn_5270
bool Pyudon::tryBlowDownKick(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isHipDropNerve() || isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir))) {
        MR::tryHitReaction(this, "\x83L\x83\x62\x83N\x83q\x83\x62\x83g");  // lbl_806905F8
        MR::zeroVelocity(this);
        MR::setVelocityBlowAttack(this, pSender, pReceiver, cBlowHorizontal, cBlowVertical, 4);
        ScoreAttackFunction::scoreByAttack(this, msg, 0);
        setNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown));
        return true;
    }
    return false;
}

// fn_5330
bool Pyudon::tryPyudonStarPieceReaction(HitSensor* pSender, HitSensor* pReceiver) {
    bool approach = false;
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvTrack))) approach = true;
    if (approach || isNerve(GET_NERVE(Pyudon, PyudonNrvLockOn)) ||
        isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir)) || isNerve(GET_NERVE(Pyudon, PyudonNrvDamage))) {
        setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir));
        return true;
    }
    if (isAttackNerve()) {
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvRecovery)) || isNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir))) {
            if (MR::isGreaterStep(this, 35)) setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir));
            else setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2));
            return true;
        }
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) || isNerve(GET_NERVE(Pyudon, PyudonNrvJump))) {
            MR::setVelocitySeparateHV(this, pSender, pReceiver, 18.0f, 12.0f);
            setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction2));
            return true;
        }
        setNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReaction));
        return true;
    }
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
bool Pyudon::tryStartStarPointBind() {
    if (isStarPointBindable()) {
        if (isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
            isNerve(GET_NERVE(Pyudon, PyudonNrvLockOn)) || isNerve(GET_NERVE(Pyudon, PyudonNrvTrack)) ||
            isNerve(GET_NERVE(Pyudon, PyudonNrvRecovery)) || isNerve(GET_NERVE(Pyudon, PyudonNrvRecoveryAir)) ||
            isNerve(GET_NERVE(Pyudon, PyudonNrvStarPieceReactionAir))) {
            if (mBindStar1->tryStartPointBind()) {
                setNerve(GET_NERVE(Pyudon, PyudonNrvDamage));
                return true;
            }
        } else if (mBindStar2->tryStartPointBind()) {
            setNerve(GET_NERVE(Pyudon, PyudonNrvDamageAir));
            return true;
        }
    }
    return false;
}

// fn_5650: rebound the velocity away from the target direction.
void Pyudon::updateVelocityToTarget(f32 farg0, f32 farg1) {
    TVec3f toTarget = mTargetPos - mPosition;
    f32 dot = toTarget.dot(mGravity);
    TVec3f horizontal = toTarget - mGravity * dot;
    f32 length = horizontal.length();
    MR::addVelocityKeepHeight(this, mTargetPos, farg1, 100.0f);
    f32 limit = 100.0f;
    MR::addVelocityMoveToDirection3D(this, horizontal, farg0 * MR::normalizeAbs(length, -limit, limit));
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
}

// fn_5750: target position = ground position + up * (450) + up * speed rate.
void Pyudon::updateTargetPos() {
    TVec3f up(0.0f);
    MR::getPlayerUpVec(&up);
    f32 speed = mSpeedRate;
    mTargetPos = mGroundPos + up * 450.0f + up * speed;
}

// fn_5810
void Pyudon::updateBlownVelocity() {
    f32 friction = MR::isOnGround(this) ? 0.93f : 0.99f;
    MR::attenuateVelocity(this, friction);
    if (MR::isOnGround(this) == false) {
        MR::addVelocityToGravity(this, 1.3f);
    }
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
}

// fn_58A0
void Pyudon::updatePyudonClipping() {
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            MR::invalidateClipping(this);
            return;
        }
    }
    if (MR::calcDistanceToPlayer(this) < 1000.0f) {
        MR::invalidateClipping(this);
        return;
    }
    MR::validateClipping(this);
}

// fn_5920: damps the velocity along gravity and the horizontal plane.
// NONMATCHING: register allocation and register-save sequence differ; actor argument and vector operations agree.
void Pyudon::dampVelocity(LiveActor* actor, f32 farg0, f32 farg1) {
    TVec3f horizontal;
    TVec3f& velocity = actor->mVelocity;
    TVec3f& gravity = actor->mGravity;
    JMAVECScaleAdd(&gravity, &velocity, &horizontal, -gravity.dot(velocity));
    horizontal.scale(farg0);
    velocity.scale(gravity.dot(velocity), gravity);
    velocity.scale(farg1);
    velocity.add(horizontal);
}

// fn_59D0: remembers the ground actor under the pyudon.
bool Pyudon::updateGroundActor() {
    LiveActor* pGround = MR::getGroundActor(this);
    mGroundActor = pGround;
    if (pGround != nullptr) {
        mConnector->attachToActor(pGround);
        return true;
    }
    return false;
}

// fn_5A20
bool Pyudon::tryReleaseGroundActor() {
    if (mGroundActor == nullptr) {
        return true;
    }
    bool isDead = MR::isDead(mGroundActor);
    if (!isDead && MR::isValidCollisionParts(mGroundActor)) {
        return false;
    }
    mConnector->detach();
    mGroundActor = nullptr;
    return true;
}

// fn_5AA0
bool Pyudon::releaseGroundActor() {
    if (mGroundActor == nullptr) {
        return true;
    }
    mConnector->detach();
    mGroundActor = nullptr;
    return true;
}

// fn_801F4DE0 / control
void Pyudon::control() {
    mAnimScaleCtrl->updateNerve();
    if (isAttackNerve()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    tryStartStarPointBind();
}

// calcAndSetBaseMtx
void Pyudon::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScaleCtrl);
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFront);
    if (isBindMapObjNerve()) {
        mConnector->connect();
        return;
    }
    releaseGroundActor();
}

// attackSensor
// NONMATCHING: compiler folds a redundant unconditional branch and drops the final unused-result comparison.
void Pyudon::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayer(pReceiver)) {
            if (!isHipDropNerve() && !MR::isPlayerHipDropFalling() && !MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
            return;
        }
        return;
    } else if (MR::isSensorPlayerOrRide(pReceiver)) {
        bool isFind = isNerve(GET_NERVE(Pyudon, PyudonNrvWait)) || isNerve(GET_NERVE(Pyudon, PyudonNrvFind)) ||
                      isNerve(GET_NERVE(Pyudon, PyudonNrvTrack));
        if (!isFind && !isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) && !isNerve(GET_NERVE(Pyudon, PyudonNrvAttack)) &&
            !isNerve(GET_NERVE(Pyudon, PyudonNrvJump)) && !isNerve(GET_NERVE(Pyudon, PyudonNrvHipDropLand))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else {
        if (MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
            return;
        }
        if (MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) {
            return;
        }
    }
}

// receiveMsgPlayerAttack
bool Pyudon::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Damage")) {
        if (MR::isMsgPlayerTrample(msg)) return tryStepDown(msg, pSender, pReceiver);
        if (MR::isMsgPlayerHipDrop(msg)) return tryStepDown(msg, pSender, pReceiver);
        if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
            return tryBlowDownSpin(msg, pSender, pReceiver);
        }
        if (MR::isMsgLockOnStarPieceShoot(msg)) return true;
        if (MR::isMsgStarPieceAttack(msg)) {
            return tryPyudonStarPieceReaction(pSender, pReceiver);
        }
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        return tryPyudonStarPieceReaction(pSender, pReceiver);
    }
    return false;
}

// receiveOtherMsg
bool Pyudon::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Damage") && MR::isMsgPlayerKick(msg)) {
        return tryBlowDownKick(msg, pSender, pReceiver);
    }
    if (isAttackNerve()) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            return receiveMsgYoshiTongue();
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) return true;
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        kill();
        return true;
    }
    if (MR::isMsgYoshiCancelEaten(msg)) {
        kill();
        return true;
    }
    return false;
}

// receiveMsgPush
bool Pyudon::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    bool blocked = false;
    if (isNerve(GET_NERVE(Pyudon, PyudonNrvStepDown)) || isNerve(GET_NERVE(Pyudon, PyudonNrvBlowDown))) blocked = true;
    if (blocked) return false;
    if (!MR::isSensorPlayerOrRide(pSender)) {
        MR::addVelocityFromPush(this, 1.0f, pSender, pReceiver);
        return true;
    }
    return false;
}

// releaseGroundActor-ish: dtor
Pyudon::~Pyudon() {}

namespace {
    const f32 cBlowHorizontal = 30.0f;
    const f32 cBlowVertical = 30.0f;
    const f32 cLockOnHeightAcceleration = 1.1f;
}  // namespace

