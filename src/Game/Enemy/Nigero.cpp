#include "Game/Enemy/Nigero.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

class ParabolicPath;
class YoshiLockOnTarget;

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};  // namespace MR

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
};  // namespace YoshiFunction

// Unnamed SMG2 helpers.

// Nerve instances (sbss, defined by NEW_NERVE below). Names are provisional.
namespace NrvNigero {
    NEW_NERVE(NigeroNrvAppear, Nigero, Appear);
    NEW_NERVE(NigeroNrvWaitNormal, Nigero, WaitNormal);
    NEW_NERVE(NigeroNrvWalk, Nigero, Walk);
    NEW_NERVE(NigeroNrvWalkMove, Nigero, WalkMove);
    NEW_NERVE(NigeroNrvSearch, Nigero, Search);
    NEW_NERVE(NigeroNrvFind, Nigero, Find);
    NEW_NERVE(NigeroNrvAttackStart, Nigero, AttackStart);
    NEW_NERVE(NigeroNrvThrow, Nigero, Throw);
    NEW_NERVE(NigeroNrvAway, Nigero, Away);
    NEW_NERVE(NigeroNrvSwoon, Nigero, Swoon);
    NEW_NERVE(NigeroNrvDead, Nigero, Dead);
    NEW_NERVE(NigeroNrvHide, Nigero, Hide);
    NEW_NERVE(NigeroNrvHidden, Nigero, Hidden);
    NEW_NERVE(NigeroNrvAttackSuccess, Nigero, AttackSuccess);
    class NigeroNrvSupportFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static NigeroNrvSupportFreeze sInstance;
    };
    NigeroNrvSupportFreeze NigeroNrvSupportFreeze::sInstance ATTRIBUTE_WEAK;
    class NigeroNrvTongue : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static NigeroNrvTongue sInstance;
    };
    NigeroNrvTongue NigeroNrvTongue::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvNigero

Nigero::Nigero(const char* pName) : LiveActor(pName) {
    mRotate.set(0.0f, 0.0f, 0.0f, 1.0f);
    mRotateTarget.set(0.0f, 0.0f, 0.0f, 1.0f);
    mIsHolding = 0;
    mTargetSensor = nullptr;
    mPath = nullptr;
    mAnimScaleCtrl = nullptr;
    mBindStar = nullptr;
    mGroundPos.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mResetPos.set(0.0f, 0.0f, 0.0f);
    mUpCounter = 0;
    mAwayTimer = 0;
    mYoshiLockOn = nullptr;
}

void Nigero::init(const JMapInfoIter& rIter) {
    MR::calcPositionUpOffset(&mPosition, this, MR::getBinderRadius(this));
    MR::makeQuatAndFrontFromRotate(&mRotate, &mFront, this);
    mRotateTarget.set(mRotate);
    mResetPos.set(mPosition);
    initNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mBindStar = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStar, GET_NERVE(Nigero, NigeroNrvSupportFreeze), "SupportFreeze");
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, nullptr, TVec3f(0.0f, 0.0f, 0.0f));
    MR::resetPosition(this, mResetPos);
    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Nigero::control() {
    mAnimScaleCtrl->updateNerve();
    if (isEatenNerve()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (isActiveNerve() && mBindStar->tryStartPointBind()) {
        setNerve(GET_NERVE(Nigero, NigeroNrvSupportFreeze));
    }
}

void Nigero::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mRotate);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void Nigero::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!recvEyeAttack(pSender, pReceiver) && !recvItemGet(pSender, pReceiver) && !recvBodyNerve(pSender, pReceiver) &&
        !recvAttackSensor(pSender, pReceiver)) {
        recvPushDefault(pSender, pReceiver);
    }
}

bool Nigero::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return isEatenNerve();
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        if (isEatenNerve()) {
            setNerve(GET_NERVE(Nigero, NigeroNrvAttackSuccess));
        }
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        MR::fn_80010200(&mRotate, &mRotateTarget, this, pSender, pReceiver);
        MR::setVelocitySeparateHV(this, pSender, pReceiver, 25.0f, 25.0f);
        MR::startAction(this, "SupportFreeze");
        setNerve(GET_NERVE(Nigero, NigeroNrvSwoon));
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "SupportFreeze");
        setNerve(GET_NERVE(Nigero, NigeroNrvDead));
    }
    return false;
}

bool Nigero::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemy(pSender)) {
        MR::addVelocityFromPush(this, 30.0f, pSender, pReceiver);
    }
    return true;
}

bool Nigero::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (!isNerve(GET_NERVE(Nigero, NigeroNrvTongue))) {
            recvLostTarget();
            MR::startAction(this, "Eat");
            MR::zeroVelocity(this);
            setNerve(GET_NERVE(Nigero, NigeroNrvTongue));
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
    }
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        MR::setScale(this, 1.0f);
        MR::setRotation(this, TVec3f(0.0f, 0.0f, 0.0f));
        setNerve(GET_NERVE(Nigero, NigeroNrvDead));
    }
    if (msg == 0xCB && MR::isEqualSensor(pReceiver, this, "Body")) {
        return isNerve(GET_NERVE(Nigero, NigeroNrvHidden));
    }
    return false;
}

// fn_801D1480: eye sensor (sends msg 0x8A, records the target, moves to the search nerve).
bool Nigero::recvEyeAttack(HitSensor* pSender, HitSensor* pReceiver) {
    return MR::isEqualSensor(pSender, this, "Eye") && isEatenNerve();
}

// fn_801D1520: item-get sensor (msg 0x8B).
bool Nigero::recvItemGet(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "ItemGet")) {
        return false;
    }
    if (MR::sendArbitraryMsg(0x8B, pReceiver, pSender)) {
        return false;
    }
    mIsHolding = 1;
    setNerve(GET_NERVE(Nigero, NigeroNrvHidden));
    return true;
}

// fn_801D15B0: body sensor (msg 0xCB).
bool Nigero::recvBodyNerve(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Eye")) {
        return false;
    }
    if (!isBindedNerve()) {
        if (MR::sendArbitraryMsg(0xCB, pReceiver, pSender)) {
            return false;
        }
        if (isNerve(GET_NERVE(Nigero, NigeroNrvSearch))) {
            if (mTargetSensor == nullptr) {
                mTargetSensor = pReceiver;
            }
        } else {
            mTargetSensor = pReceiver;
            MR::invalidateClipping(this);
            setNerve(GET_NERVE(Nigero, NigeroNrvSearch));
        }
    }
    return true;
}

// fn_801D16D0: attack sensor.
bool Nigero::recvAttackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Attack")) {
        return false;
    }
    if (MR::isSensorPlayer(pReceiver)) {
        return false;
    }
    if (isBindedNerve() && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvSwoon));
        return true;
    }
    return MR::sendMsgPush(pReceiver, pSender);
}

// fn_801D17A0: default push for enemies and yoshi sensors.
bool Nigero::recvPushDefault(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Body")) {
        return false;
    }
    bool isEnemy = MR::isSensorEnemy(pReceiver);
    if (!isEnemy && !MR::isSensorYoshi(pReceiver)) {
        return MR::sendMsgPush(pReceiver, pSender);
    }
    return false;
}

// fn_801D1850
bool Nigero::isBindedNerve() const {
    return isNerve(GET_NERVE(Nigero, NigeroNrvWalk)) || isNerve(GET_NERVE(Nigero, NigeroNrvWalkMove)) ||
           isNerve(GET_NERVE(Nigero, NigeroNrvSearch));
}

// fn_801D1910
bool Nigero::isEatenNerve() const {
    return isNerve(GET_NERVE(Nigero, NigeroNrvAway)) || isNerve(GET_NERVE(Nigero, NigeroNrvFind)) ||
           isNerve(GET_NERVE(Nigero, NigeroNrvAttackStart)) || isNerve(GET_NERVE(Nigero, NigeroNrvThrow));
}

// fn_801D19B0
bool Nigero::isLockOnNerve() const {
    return isNerve(GET_NERVE(Nigero, NigeroNrvAway)) || isNerve(GET_NERVE(Nigero, NigeroNrvFind)) ||
           isNerve(GET_NERVE(Nigero, NigeroNrvAttackStart)) || isNerve(GET_NERVE(Nigero, NigeroNrvThrow));
}

// fn_801D1A50
bool Nigero::isActiveNerve() const {
    if (isEatenNerve()) {
        return true;
    }
    if (isNerve(GET_NERVE(Nigero, NigeroNrvHide))) {
        return MR::isGreaterStep(this, 20);
    }
    return false;
}

// fn_801D1B20
bool Nigero::isWalkNerve() const {
    return isNerve(GET_NERVE(Nigero, NigeroNrvWalk)) || isNerve(GET_NERVE(Nigero, NigeroNrvWalkMove)) ||
           isNerve(GET_NERVE(Nigero, NigeroNrvSearch));
}

// fn_801D1B90: release the held sensor host.
void Nigero::recvLostTarget() {
    if (mTargetSensor != nullptr) {
        MR::getSensorHost(mTargetSensor)->mPosition.set(mPosition);
        MR::setVelocity(MR::getSensorHost(mTargetSensor), mGravity * 0.0f);
        MR::sendArbitraryMsg(0x8D, mTargetSensor, getSensor("Body"));
        mTargetSensor = nullptr;
    }
}

// fn_801D1C20: bounce the held sensor host away.
void Nigero::recvStarPieceEnd() {
    if (mTargetSensor != nullptr) {
        LiveActor* pHost = MR::getSensorHost(mTargetSensor);
        MR::zeroVelocity(pHost);
        TVec3f up;
        MR::fn_8000EAB0(&up, this);
        f32 d = mFront.dot(up);
        if (d > 0.0f) {
            up.negate();
        }
        MR::setVelocitySeparateHV(MR::getSensorHost(mTargetSensor), up, 0.0f, 0.0f);
        MR::sendArbitraryMsg(0x8D, mTargetSensor, getSensor("Body"));
        mTargetSensor = nullptr;
    }
}

// fn_801D1CF0
void Nigero::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::resetPosition(this, mResetPos);
        MR::zeroVelocity(this);
        MR::startAction(this, "Appear");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal));
    }
}

// fn_801D1D70
void Nigero::exeWaitNormal() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "WaitNormal");
    }
    MR::attenuateVelocity(this, MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f));
    updateUpVec();
    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvSearch));
    }
}

// fn_801D1E10
void Nigero::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f dir(0.0f, 0.0f, 1.0f);
    if (MR::normalizeOrZero(&dir)) {
        mFront.set(dir);
    }
    MR::rotateVecDegree(&mFront, up, 0.0f);
    if (mUpCounter <= 10) {
        MR::addVelocity(this, TVec3f(0.0f, 0.0f, 0.0f));
        MR::attenuateVelocity(this, 0.9f);
    } else {
        MR::addVelocityToGravity(this, 1.0f);
        MR::attenuateVelocity(this, 0.8f);
    }
    updateUpVec();
    if (MR::isGreaterStep(this, 40)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvWalkMove));
    }
}

// fn_801D1F30
void Nigero::exeWalkMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        mGroundPos.set(mFront);
    }
    MR::turnDirectionToGround(this, &mGroundPos);
    moveToTarget(mGroundPos, 0.0f, 1.0f, 0.0f, 0.9f, 0.9f);
    updateUpVec();
    if (MR::isGreaterStep(this, 300)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvAway));
    }
}

// fn_801D1FE0
void Nigero::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Walk");
    }
    if (mTargetSensor == nullptr) {
        setNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal));
    } else {
        TVec3f toTarget = -(*MR::getSensorPos(mTargetSensor) - mPosition);
        moveToTarget(toTarget, 0.0f, 1.0f, 0.0f, 0.9f, 0.9f);
        updateUpVec();
    }
    if (!MR::sendArbitraryMsg(0xCB, mTargetSensor, getSensor("Body"))) {
        mTargetSensor = nullptr;
        setNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal));
    }
    mTargetSensor = nullptr;
}

// fn_801D20C0
void Nigero::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Find");
    }
    MR::moveAndTurnToTarget(this, &mFront, *MR::getSensorPos(mTargetSensor), 0.0f, 0.0f, 0.0f, 0.0f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvAttackStart));
    }
    updateUpVec();
}

// fn_801D2160
void Nigero::exeAttackStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Chase");
    }
    TVec3f toTarget = -(*MR::getSensorPos(mTargetSensor) - mPosition);
    moveToTarget(toTarget, 0.0f, 1.2f, 0.0f, 0.95f, 0.95f);
    updateUpVec();
    if (!MR::sendArbitraryMsg(0x8A, mTargetSensor, getSensor("Body"))) {
        mTargetSensor = nullptr;
        setNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal));
    }
}

// fn_801D2220
void Nigero::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(MR::getSensorHost(mTargetSensor));
        TVec3f up;
        MR::calcPositionUpOffset(&up, this, 0.0f);
        TVec3f neg(mGravity);
        neg.negate();
        mPath->initFromUpVector(mTargetSensor ? mPosition : mPosition, up, neg, 0.0f);
    }
    mPath->calcPosition(&MR::getSensorHost(mTargetSensor)->mPosition, MR::calcNerveRate(this, 20));
    updateUpVec();
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvAway));
    }
}

// fn_801D2310
void Nigero::exeAway() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Away");
        mAwayTimer = 120;
    }
    bool isNear = MR::isNearPlayer(this, 1000.0f);
    f32 speed = isNear ? 0.75f : 0.0f;
    TVec3f toPlayer = -(*MR::getPlayerPos() - mPosition);
    moveToTarget(toPlayer, 0.0f, speed, 0.0f, 0.99f, 0.99f);
    updateUpVec();
    if (mAwayTimer <= 0) {
        if (MR::isNearPlayer(this, 1000.0f)) {
            MR::startActionSound(this, "NearPlayer", -1, -1, -1);
            mAwayTimer = 120;
        }
    } else {
        mAwayTimer--;
    }
}

// fn_801D2430
void Nigero::exeSwoon() {
    if (MR::isFirstStep(this)) {
        mAnimScaleCtrl->startHitReaction();
        MR::tryHitReaction(this, "\x83\x58\x83\x5e\x81\x5b\x83\x73\x81\x5b\x83\x58\x83\x71\x83\x62");
        recvLostTarget();
        MR::startAction(this, "Swoon");
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
    updateUpVec();
    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal));
    }
}

// fn_801D24F0
void Nigero::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::stopScene(5);
        MR::fn_80034820(this);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
    MR::turnQuatUpToGravity(&mRotate, mRotate, this);
    if (mTargetSensor != nullptr) {
        MR::calcPositionUpOffset(&MR::getSensorHost(mTargetSensor)->mPosition, this, 0.0f);
    }
    if (MR::isGreaterStep(this, 5)) {
        if (MR::isBindedGround(this)) {
            MR::zeroVelocity(this);
            MR::startAction(this, "Hide");
            setNerve(GET_NERVE(Nigero, NigeroNrvHide));
        }
    }
}

// fn_801D25E0
void Nigero::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::turnQuatUpToGravity(&mRotate, mRotate, this);
        recvLostTarget();
    }
    if (MR::isGreaterStep(this, 40)) {
        MR::startAction(this, "Hide");
        setNerve(GET_NERVE(Nigero, NigeroNrvHidden));
    }
}

// fn_801D2660
void Nigero::exeHidden() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        recvLostTarget();
    }
    if (MR::isStep(this, 1)) {
        MR::hideModel(this);
    }
    if (MR::isGreaterStep(this, 300)) {
        MR::resetPosition(this, mResetPos);
        mRotate.set(mRotateTarget);
        mFront.set(-mGravity);
        setNerve(GET_NERVE(Nigero, NigeroNrvAppear));
    }
}

// fn_801D2760
void Nigero::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
        MR::calcVecToPlayerH(&mFront, this, nullptr);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
    updateUpVec();
    if (mTargetSensor != nullptr) {
        MR::calcPositionUpOffset(&MR::getSensorHost(mTargetSensor)->mPosition, this, 0.0f);
    }
    if (MR::isActionEnd(this)) {
        if (mTargetSensor != nullptr) {
            setNerve(GET_NERVE(Nigero, NigeroNrvThrow));
        }
        setNerve(GET_NERVE(Nigero, NigeroNrvWaitNormal));
    }
}

// fn_801D2840: move along front vector toward the given direction (approximate).
void Nigero::moveToTarget(const TVec3f& rDir, f32 farg0, f32 farg1, f32 farg2, f32 farg3, f32 farg4) {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f front(0.0f, 0.0f, 1.0f);
    MR::normalizeOrZero(&front);
    mFront.set(front);
    TVec3f side(rDir);
    side.normalize();
    f32 turn = side.dot(up);
    f32 mag = side.length();
    MR::reboundVelocityFromCollision(this, farg0 * turn, farg1, 1.0f);
    if (mUpCounter <= 10) {
        TVec3f v(mFront);
        v.scale(farg1);
        MR::addVelocity(this, v);
        TVec3f w(mFront);
        w.scale(-farg2);
        MR::addVelocity(this, w);
        MR::attenuateVelocity(this, farg3);
    } else {
        MR::addVelocityToGravity(this, farg2);
        MR::attenuateVelocity(this, farg4);
    }
    (void)mag;
}

// fn_801D2C30: keeps the up vector aligned with the binder and steps the up counter.
void Nigero::updateUpVec() {
    bool isBinded = MR::isBinded(this);
    if (isBinded) {
        mUpCounter = 0;
    } else {
        mUpCounter++;
    }
    if (mUpCounter > 10) {
        TVec3f neg(mGravity);
        neg.negate();
        MR::turnQuatYDirRate(&mRotate, mRotate, neg, 0.1f);
    }
}

// Nigero::~Nigero
Nigero::~Nigero() {}
