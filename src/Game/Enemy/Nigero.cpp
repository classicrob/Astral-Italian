#include "Game/Enemy/Nigero.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Player/YoshiFunction.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

class ParabolicPath;
class YoshiLockOnTarget;



// Unnamed SMG2 helpers.

// Nerve instances (sbss, defined by NEW_NERVE below). Names are provisional.
namespace NrvNigero {
    NEW_NERVE(NigeroNrvAppear, Nigero, Appear);
    NEW_NERVE(NigeroNrvWaitNormal, Nigero, WaitNormal);
    NEW_NERVE(NigeroNrvWalk, Nigero, Walk);
    NEW_NERVE(NigeroNrvWalkMove, Nigero, WalkMove);
    NEW_NERVE(NigeroNrvSearch, Nigero, Search);
    NEW_NERVE(NigeroNrvFind, Nigero, Find);
    NEW_NERVE(NigeroNrvAttackStart, Nigero, Chase);
    // NOTE: the next three class names (symbols.txt) are rotated against their behaviour:
    // slot Throw runs exeFlee, slot Away runs exeSwoon, slot Swoon runs exeGet.
    NEW_NERVE(NigeroNrvThrow, Nigero, Flee);
    NEW_NERVE(NigeroNrvAway, Nigero, Swoon);
    NEW_NERVE(NigeroNrvSwoon, Nigero, Get);
    NEW_NERVE(NigeroNrvDead, Nigero, Dead);
    NEW_NERVE(NigeroNrvHide, Nigero, Hide);
    NEW_NERVE(NigeroNrvHidden, Nigero, Hidden);
    NEW_NERVE(NigeroNrvAttackSuccess, Nigero, AttackSuccess);
};  // namespace NrvNigero

// Inlined into the SupportFreeze nerve (defined before its NEW_NERVE).
void Nigero::exeSupportFreeze() {
    if (MR::isFirstStep(this)) {
        recvStarPieceEnd();
    }
    MR::updateActorStateAndNextNerve(this, &NrvNigero::NigeroNrvWaitNormal::sInstance);
}

namespace NrvNigero {
    NEW_NERVE(NigeroNrvSupportFreeze, Nigero, SupportFreeze);
    class NigeroNrvTongue : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static NigeroNrvTongue sInstance;
    };
    NigeroNrvTongue NigeroNrvTongue::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvNigero

#define NRV(n) (&NrvNigero::NigeroNrv##n::sInstance)

Nigero::Nigero(const char* pName)
    : LiveActor(pName), mRotate(0.0f, 0.0f, 0.0f, 1.0f), mRotateTarget(0.0f, 0.0f, 0.0f, 1.0f), mHoldSensor(nullptr), mTargetSensor(nullptr),
      mPath(new ParabolicPath()), mAnimScaleCtrl(nullptr), mBindStar(nullptr), mMoveDir(0.0f, 0.0f, 1.0f), mFront(0.0f, 0.0f, 1.0f),
      mResetPos(0.0f, 0.0f, 0.0f), mUpCounter(0), mAwayTimer(0), mYoshiLockOn(nullptr) {
}

void Nigero::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    MR::calcPositionUpOffset(&mPosition, this, MR::getBinderRadius(this));
    MR::makeQuatAndFrontFromRotate(&mRotate, &mFront, this);
    mRotateTarget.set(mRotate);
    mResetPos.set(mPosition);
    initNerve(NRV(WaitNormal), 1);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    mBindStar = new WalkerStateBindStarPointer(this, mAnimScaleCtrl);
    MR::initActorState(this, mBindStar, NRV(SupportFreeze), "SupportFreeze");
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));
    MR::resetPosition(this);
    MR::useStageSwitchAwake(this, rIter);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Nigero::control() {
    mAnimScaleCtrl->updateNerve();
    if (isEatenNerve()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    if (isActiveNerve() && mBindStar->tryStartPointBind()) {
        setNerve(reinterpret_cast< const Nerve* >(NRV(SupportFreeze)));
    }
}

void Nigero::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mRotate);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void Nigero::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (recvEyeAttack(pSender, pReceiver)) {
        return;
    }
    if (recvItemGet(pSender, pReceiver)) {
        return;
    }
    if (recvBodyNerve(pSender, pReceiver)) {
        return;
    }
    if (recvAttackSensor(pSender, pReceiver)) {
        return;
    }
    if (recvPushDefault(pSender, pReceiver)) {
        return;
    }
}

bool Nigero::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, this, "Body")) {
        return false;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return isLockOnNerve();
    }
    if (MR::isMsgStarPieceAttack(msg) && isLockOnNerve()) {
        setNerve(NRV(Away));
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        MR::fn_80010200(&mRotate, &mFront, this, pReceiver, pSender);
        MR::setVelocitySeparateHV(this, pSender, pReceiver, 30.0f, 30.0f);
        MR::startAction(this, "BlowDown");
        setNerve(NRV(Dead));
        return true;
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "StepDown");
        setNerve(NRV(Hide));
        return true;
    }
    return false;
}

bool Nigero::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, this, "Body")) {
        return false;
    }
    if (MR::isSensorEnemy(pSender)) {
        MR::addVelocityFromPush(this, 1.0f, pSender, pReceiver);
        return true;
    }
    return false;
}

bool Nigero::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg) && !isNerve(NRV(Tongue))) {
        recvLostTarget();
        MR::startAction(this, "Eat");
        setNerve(NRV(Tongue));
        MR::zeroVelocity(this);
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) {
        MR::setScale(this, 1.0f);
        MR::setRotation(this, 0.0f);
        setNerve(NRV(Hidden));
        return true;
    }
    if (msg == 0xCB && MR::isEqualSensor(pReceiver, this, "Body")) {
        return isNerve(NRV(Throw));
    }
    return false;
}

bool Nigero::recvEyeAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Eye")) {
        return false;
    }
    if (!MR::sendArbitraryMsg(0x8A, pReceiver, pSender)) {
        return false;
    }
    mTargetSensor = pReceiver;
    setNerve(NRV(Find));
    MR::invalidateClipping(this);
    return true;
}

bool Nigero::recvItemGet(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "ItemGet")) {
        return false;
    }
    if (!MR::sendArbitraryMsg(0x8B, pReceiver, pSender)) {
        return false;
    }
    mHoldSensor = pReceiver;
    setNerve(NRV(Swoon));
    return true;
}

bool Nigero::recvBodyNerve(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Eye")) {
        return false;
    }
    if (!isWalkNerve()) {
        return false;
    }
    if (!MR::sendArbitraryMsg(0xCB, pReceiver, pSender)) {
        return false;
    }
    if (isNerve(NRV(Search))) {
        if (mTargetSensor == nullptr) {
            mTargetSensor = pReceiver;
        } else if (mTargetSensor != pReceiver) {
            f32 dist = MR::calcDistance(pSender, mTargetSensor, nullptr);
            if (MR::calcDistance(pSender, pReceiver, nullptr) < dist) {
                mTargetSensor = pReceiver;
            }
        }
    } else {
        mTargetSensor = pReceiver;
        setNerve(NRV(Search));
        MR::invalidateClipping(this);
    }
    return true;
}

bool Nigero::recvAttackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Attack")) {
        return false;
    }
    if (!MR::isSensorPlayer(pReceiver)) {
        return false;
    }
    if (isBindedNerve() && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
        setNerve(NRV(AttackSuccess));
        return true;
    }
    if (MR::sendMsgPush(pReceiver, pSender)) {
        return true;
    }
    return false;
}

bool Nigero::recvPushDefault(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "Body")) {
        return false;
    }
    if (!MR::isSensorEnemy(pReceiver) && !MR::isSensorYoshi(pReceiver)) {
        return false;
    }
    if (MR::sendMsgPush(pReceiver, pSender)) {
        return true;
    }
    return false;
}

bool Nigero::isBindedNerve() const {
    if (isNerve(NRV(WaitNormal)) || isNerve(NRV(Walk)) || isNerve(NRV(WalkMove)) || isNerve(NRV(Search)) || isNerve(NRV(Find)) ||
        isNerve(NRV(AttackStart)) || isNerve(NRV(Throw))) {
        return true;
    }
    return false;
}

bool Nigero::isEatenNerve() const {
    if (isNerve(NRV(Appear)) || isNerve(NRV(Dead)) || isNerve(NRV(Hide)) || isNerve(NRV(Hidden)) || isNerve(NRV(Tongue))) {
        return false;
    }
    return true;
}

bool Nigero::isLockOnNerve() const {
    if (isNerve(NRV(Appear)) || isNerve(NRV(Dead)) || isNerve(NRV(Hide)) || isNerve(NRV(Hidden)) || isNerve(NRV(Tongue))) {
        return false;
    }
    return true;
}

bool Nigero::isActiveNerve() const {
    if (isNerve(NRV(Appear)) || isNerve(NRV(Dead)) || isNerve(NRV(Hide)) || isNerve(NRV(Hidden)) ||
        isNerve(reinterpret_cast< const Nerve* >(NRV(SupportFreeze))) || isNerve(NRV(Tongue))) {
        return false;
    }
    if (isNerve(NRV(Away))) {
        return MR::isGreaterStep(this, 20);
    }
    return true;
}

bool Nigero::isWalkNerve() const {
    if (isNerve(NRV(WaitNormal)) || isNerve(NRV(Walk)) || isNerve(NRV(WalkMove))) {
        return true;
    }
    return false;
}

void Nigero::recvLostTarget() {
    if (mHoldSensor != nullptr) {
        MR::getSensorHost(mHoldSensor)->mPosition.set(mPosition);
        MR::setVelocity(MR::getSensorHost(mHoldSensor), mGravity * -30.0f);
        MR::sendArbitraryMsg(0x8D, mHoldSensor, getSensor("Body"));
        mHoldSensor = nullptr;
    }
}

void Nigero::recvStarPieceEnd() {
    if (mHoldSensor != nullptr) {
        MR::getSensorHost(mHoldSensor)->mPosition.set(mPosition);
        MR::zeroVelocity(MR::getSensorHost(mHoldSensor));
        TVec3f up;
        MR::fn_8000EAB0(&up, this);
        if (mFront.dot(up) > 0.0f) {
            up.negate();
        }
        MR::calcGravity(MR::getSensorHost(mHoldSensor));
        MR::setVelocitySeparateHV(MR::getSensorHost(mHoldSensor), up, 6.0f, 20.0f);
        MR::sendArbitraryMsg(0x8D, mHoldSensor, getSensor("Body"));
        mHoldSensor = nullptr;
    }
}

void Nigero::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::resetPosition(this, mResetPos);
        MR::startAction(this, "Appear");
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(NRV(WaitNormal));
    }
}

void Nigero::exeWaitNormal() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "WaitNormal");
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravityOrGround(this, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
    updateUpVec();
    if (MR::isGreaterStep(this, 60)) {
        setNerve(NRV(Walk));
    }
}

void Nigero::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f dir(0.0f);
    dir.killElement(mFront, up);
    if (!MR::normalizeOrZero(&dir)) {
        mFront.set(dir);
    }
    MR::rotateVecDegree(&mFront, up, 3.0f);
    if (mUpCounter <= 10) {
        MR::addVelocity(this, up * -1.0f);
        MR::attenuateVelocity(this, 0.9f);
    } else {
        MR::addVelocityToGravity(this, 1.0f);
        MR::attenuateVelocity(this, 0.99f);
    }
    updateUpVec();
    if (MR::isGreaterStep(this, 40)) {
        setNerve(NRV(WalkMove));
    }
}

void Nigero::exeWalkMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        mMoveDir.set(mFront);
    }
    MR::turnDirectionToGround(this, &mMoveDir);
    moveToTarget(mMoveDir, 0.7f, 1.0f, 1.0f, 0.9f, 0.99f);
    updateUpVec();
    if (MR::isGreaterStep(this, 300)) {
        setNerve(NRV(Walk));
    }
}

void Nigero::exeSearch() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Walk");
    }
    if (mTargetSensor == nullptr) {
        setNerve(NRV(WaitNormal));
        return;
    }
    TVec3f dir = *MR::getSensorPos(mTargetSensor) - mPosition;
    moveToTarget(dir, 1.0f, 2.0f, 1.0f, 0.9f, 0.99f);
    updateUpVec();
    if (!MR::sendArbitraryMsg(0xCB, mTargetSensor, getSensor("Eye"))) {
        mTargetSensor = nullptr;
        setNerve(NRV(WaitNormal));
        return;
    }
    mTargetSensor = nullptr;
}

void Nigero::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
        MR::zeroVelocity(this);
    }
    MR::moveAndTurnToTarget(this, &mFront, *MR::getSensorPos(mTargetSensor), 0.0f, 1.0f, 0.9f, 10.0f);
    updateUpVec();
    if (MR::isActionEnd(this)) {
        setNerve(NRV(AttackStart));
    }
}

void Nigero::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Chase");
    }
    TVec3f dir = *MR::getSensorPos(mTargetSensor) - mPosition;
    moveToTarget(dir, 1.5f, 3.0f, 1.0f, 0.9f, 0.99f);
    updateUpVec();
    if (!MR::sendArbitraryMsg(0x8A, mTargetSensor, getSensor("Eye"))) {
        mTargetSensor = nullptr;
        setNerve(NRV(WaitNormal));
    }
}

void Nigero::exeGet() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Get");
        MR::zeroVelocity(this);
        TVec3f from(MR::getSensorHost(mHoldSensor)->mPosition);
        TVec3f to;
        MR::calcPositionUpOffset(&to, this, 150.0f);
        TVec3f up = -mGravity;
        mPath->initFromUpVector(from, to, up, 230.0f);
    }
    LiveActor* pHost = MR::getSensorHost(mHoldSensor);
    mPath->calcPosition(&pHost->mPosition, MR::calcNerveRate(this, 20));
    updateUpVec();
    if (MR::isActionEnd(this)) {
        setNerve(NRV(Throw));
    }
}

void Nigero::exeFlee() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Away");
        mAwayTimer = 120;
    }
    f32 turn = MR::isNearPlayer(this, 1200.0f) ? 2.0f : 0.0f;
    TVec3f dir = mPosition - *MR::getPlayerPos();
    moveToTarget(dir, 1.2f, turn, 1.0f, 0.9f, 0.99f);
    updateUpVec();
    MR::calcPositionUpOffset(&MR::getSensorHost(mHoldSensor)->mPosition, this, 150.0f);
    if (mAwayTimer <= 0) {
        if (MR::isNearPlayer(this, 1200.0f)) {
            MR::startActionSound(this, "NearPlayer", -1, -1, -1);
            mAwayTimer = 120;
        }
    } else {
        mAwayTimer--;
    }
}

void Nigero::exeSwoon() {
    if (MR::isFirstStep(this)) {
        mAnimScaleCtrl->startHitReaction();
        MR::tryHitReaction(this, "\x83\x58\x83\x5e\x81\x5b\x83\x73\x81\x5b\x83\x58\x83\x71\x83\x62\x83\x67");
        recvStarPieceEnd();
        MR::startAction(this, "Swoon");
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravityOrGround(this, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
    updateUpVec();
    if (MR::isGreaterStep(this, 120)) {
        setNerve(NRV(WaitNormal));
    }
}

void Nigero::exeDead() {
    if (MR::isFirstStep(this)) {
        MR::stopScene(5);
        MR::fn_80034820(this);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravityOrGround(this, 1.2f);
    MR::attenuateVelocity(this, 0.99f);
    MR::turnQuatUpToGravity(&mRotate, mRotate, this);
    if (mHoldSensor != nullptr) {
        MR::calcPositionUpOffset(&MR::getSensorHost(mHoldSensor)->mPosition, this, 150.0f);
    }
    if (MR::isGreaterStep(this, 5) && MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
        MR::tryStartActionNotPlaying(this, "Hide");
        setNerve(NRV(Hidden));
    }
}

void Nigero::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::turnQuatUpToGravity(&mRotate, mRotate, this);
        recvLostTarget();
    }
    if (MR::isGreaterStep(this, 40)) {
        MR::tryStartActionNotPlaying(this, "Hide");
        setNerve(NRV(Hidden));
    }
}

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
        mRotate.getZDir(mFront);
        setNerve(NRV(Appear));
    }
}

void Nigero::exeAttackSuccess() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackSuccess");
        MR::calcVecToPlayerH(&mFront, this, nullptr);
    }
    MR::reboundVelocityFromCollision(this, 0.0f, 0.0f, 1.0f);
    MR::addVelocityToGravityOrGround(this, 1.0f);
    MR::attenuateVelocity(this, 0.9f);
    updateUpVec();
    if (mHoldSensor != nullptr) {
        MR::calcPositionUpOffset(&MR::getSensorHost(mHoldSensor)->mPosition, this, 150.0f);
    }
    if (MR::isActionEnd(this)) {
        if (mHoldSensor != nullptr) {
            setNerve(NRV(Throw));
            return;
        }
        setNerve(NRV(WaitNormal));
    }
}

void Nigero::moveToTarget(const TVec3f& rDir, f32 speed, f32 turn, f32 grav, f32 atten, f32 airAtten) {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f front(0.0f);
    front.killElement(mFront, up);
    if (!MR::normalizeOrZero(&front)) {
        mFront.set(front);
    }
    TVec3f dir;
    dir.killElement(rDir, mGravity);
    if (!MR::normalizeOrZero(&dir)) {
        TQuat4f q;
        TVec3f upDir;
        mRotate.getYDir(upDir);
        TVec3f negGrav = -mGravity;
        q.setRotate(negGrav, upDir, 1.0f);
        q.transform(dir);
        TVec3f axis;
        axis.set(0.0f, 1.0f, 0.0f);
        MR::turnVecToVecDegree(&mFront, mFront, dir, turn, axis);
    }
    MR::reboundVelocityFromCollision(this, -0.1f, 0.0f, 1.0f);
    if (mUpCounter <= 10) {
        MR::addVelocity(this, mFront * speed);
        MR::addVelocity(this, up * -grav);
        MR::attenuateVelocity(this, atten);
    } else {
        MR::addVelocityToGravity(this, grav);
        MR::attenuateVelocity(this, airAtten);
    }
}

void Nigero::updateUpVec() {
    MR::turnQuatZDirRate(&mRotate, mRotate, mFront, 0.3f);
    if (MR::isBinded(this)) {
        MR::turnQuatYDirRate(&mRotate, mRotate, MR::getBindedFixReactionVector(this), 0.15f);
        mUpCounter = 0;
    } else {
        mUpCounter++;
    }
    if (mUpCounter > 10) {
        TVec3f up = -mGravity;
        MR::turnQuatYDirRate(&mRotate, mRotate, up, 0.15f);
    }
}

Nigero::~Nigero() {
}
