#pragma auto_inline off
#include "Game/NPC/MonteThrow.hpp"
#include "Game/NPC/Monte.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/ParabolicPath.hpp"
const char* const sMonteThrowTargetPositionName = "モンテ投げターゲット位置";
const char* const sMonteThrowPlayerAnimation = "JumpBack";
namespace NrvMonteThrow {
NEW_NERVE(Wait, MonteThrow, Wait);
NEW_NERVE(Throw, MonteThrow, Throw);
class Prepare : public Nerve {
public:
    virtual void execute(Spine* spine) const { reinterpret_cast<MonteThrow*>(spine->mExecutor)->exePrepare(); }
    virtual void executeOnEnd(Spine* spine) const {
        MonteThrow* actor = reinterpret_cast<MonteThrow*>(spine->mExecutor);
        actor->mBindPosition->copyTrans(&actor->mPosition);
    }
    static Prepare sInstance;
};
Prepare Prepare::sInstance ATTRIBUTE_WEAK;
NEW_NERVE(ThrowUp, MonteThrow, ThrowUp);
}
namespace { const TVec3f sBindOffset(0.0f, 0.0f, 100.0f); const TVec3f sBindRotation(0.0f, 0.0f, 0.0f); }
inline f32 defaultThrowHeight() ALWAYS_INLINE { s32 n = 700; return n; }
inline f32 defaultThrowSpeed() ALWAYS_INLINE { s32 n = 50; return n; }
// NONMATCHING: floating conversion temporaries use different registers.
MonteThrow::MonteThrow(Monte* host) : LiveActor("モンテ投げ"), mHost(host), mBoundActor(0), mBindPosition(0), mCameraInfo(0), mPath(new ParabolicPath), mThrowHeight(defaultThrowHeight()), mThrowSpeed(defaultThrowSpeed()), mThrowTime(0), mTargetPosition(TVec3f(0.0f)), mThrowUp(false) {}
MonteThrow::~MonteThrow() {}
// NONMATCHING: register allocation and arithmetic scheduling differ.
void MonteThrow::init(const JMapInfoIter& iter) {
    MR::connectToSceneNpcMovement(this);
    initBinder(100.0f, 0.0f, 0);
    initHitSensor(2);
    MR::addHitSensorBinder(this, "Body", 4, 110.0f, TVec3f(0.0f,90.0f,0.0f));
    MR::addHitSensorNpc(this, "ItemGet", 4, 100.0f, TVec3f(0.0f,90.0f,0.0f));
    mPosition.set(mHost->mPosition);
    mRotation.set(mHost->mRotation);
    initNerve(&NrvMonteThrow::Wait::sInstance, 0);
    MR::onCalcGravity(this);
    mBindPosition = new FixedPosition(mHost, sBindOffset, sBindRotation);
    MR::initActorCamera(mHost, iter, &mCameraInfo);
    s32 height = 700;
    MR::getJMapInfoArg2NoInit(iter, &height);
    mThrowHeight = height;
    s32 speed = 50;
    MR::getJMapInfoArg3NoInit(iter, &speed);
    mThrowSpeed = speed;
    const char* targetName = sMonteThrowTargetPositionName;
    if (MR::tryRegisterNamePosLinkObj(mHost, iter, targetName)) {
        MR::tryFindLinkNamePos(mHost, targetName, &mTargetPosition, 0);
    } else {
        mThrowUp = true;
        f32 speedSquared = 2.0f * mThrowHeight;
        speedSquared *= 0.4f;
        mThrowSpeed = MR::fastSqrtf(speedSquared);
    }
    makeActorAppeared();
}
void MonteThrow::initAfterPlacement() {
    mBindPosition->calc();
    mBindPosition->copyTrans(&mPosition);
    initThrowPath(mPosition, mTargetPosition);
}
void MonteThrow::control() {
    if (isNerve(&NrvMonteThrow::Throw::sInstance)) MR::addVelocityToGravity(this, 0.4f);
}
void MonteThrow::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (isNerve(&NrvMonteThrow::Throw::sInstance) || isNerve(&NrvMonteThrow::ThrowUp::sInstance)) MR::tryGetItem(sender,receiver);
}
bool MonteThrow::receiveOtherMsg(u32 msg, HitSensor* sender, HitSensor*) {
    if (MR::isMsgAutoRushBegin(msg) && MR::isSensorPlayer(sender)) return tryBind(sender);
    if (MR::isMsgUpdateBaseMtx(msg)) { updatePlayerMtx(); return true; }
    return false;
}
void MonteThrow::startThrow() {
    const Nerve* nrv = &NrvMonteThrow::Throw::sInstance;
    if (mThrowUp) nrv = &NrvMonteThrow::ThrowUp::sInstance;
    setNerve(nrv);
    MR::startLevelSoundPlayer("JumpLongV", -1, -1);
}
bool MonteThrow::tryBind(HitSensor* sensor) {
    if (mBoundActor) return false;
    if (!isNerve(&NrvMonteThrow::Wait::sInstance)) return false;
    if (mHost->_E4) return false;
    MR::invalidateClipping(this);
    MR::invalidateHitSensor(this, "Body");
    mBoundActor = MR::getSensorHost(sensor);
    setNerve(&NrvMonteThrow::Prepare::sInstance);
    return true;
}
void MonteThrow::endThrow() {
    MR::validateClipping(this);
    MR::endActorCamera(mHost, mCameraInfo, true, -1);
    mBoundActor = 0;
    setNerve(&NrvMonteThrow::Wait::sInstance);
    if (MR::isPlayerInRush()) MR::endBindAndSpinDriverJump(this, mVelocity);
    MR::invalidateHitSensor(this, "ItemGet");
    MR::zeroVelocity(this);
}
void MonteThrow::releasePlayer() {
    mBoundActor = 0;
    if (MR::isPlayerInRush()) MR::endBindAndPlayerJump(this, mVelocity, 0);
    MR::invalidateHitSensor(this, "ItemGet");
    MR::zeroVelocity(this);
}
void MonteThrow::updatePlayerMtx() {
    TPos3f mtx;
    TVec3f up;
    TVec3f front;
    MR::calcUpVec(&up, mHost);
    MR::calcFrontVec(&front, mHost);
    front.negate();
    MR::makeMtxFrontUpPos(&mtx, front, up, mPosition);
    MR::setBaseTRMtx(mBoundActor, mtx);
}
bool MonteThrow::isPreparingThrow() const { return isNerve(&NrvMonteThrow::Prepare::sInstance); }
void MonteThrow::initThrowPath(const TVec3f& start, const TVec3f& end) {
    TVec3f up(-mGravity);
    mPath->initFromUpVector(start, end, up, mThrowHeight);
    mThrowTime = mPath->getLength(0.0f, 1.0f, 32) / mThrowSpeed;
}
void MonteThrow::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::validateHitSensor(this, "Body");
        mPosition.set(mHost->mPosition);
    }
}
// NONMATCHING: JMAVECLerp argument setup order differs.
void MonteThrow::exePrepare() {
    if (MR::isFirstStep(this)) MR::setPosition(this, *MR::getPlayerPos());
    TVec3f position;
    mBindPosition->copyTrans(&position);
    JMAVECLerp(&mPosition, &position, &mPosition, 0.2f);
}
// NONMATCHING: vector subtraction register allocation and scheduling differ.
void MonteThrow::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startActorCameraNoTarget(mHost, mCameraInfo, -1);
        MR::startBckPlayer(sMonteThrowPlayerAnimation, 0);
        MR::validateHitSensor(this, "ItemGet");
    }
    if (MR::isStep(this, 25)) MR::startLevelSoundPlayer("JumpStandAuto", -1, -1);
    TVec3f position;
    mPath->calcPosition(&position, MR::calcNerveRate(this, mThrowTime));
    TVec3f velocity(position);
    velocity -= mPosition;
    MR::setVelocity(this, velocity);
    if (MR::isGreaterStep(this, 60)) {
        bool end = false;
        if (MR::isBinded(this) || MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) end = true;
        if (end) endThrow();
    }
}
void MonteThrow::exeThrowUp() {
    if (MR::isFirstStep(this)) {
        MR::startActorCameraNoTarget(mHost, mCameraInfo, -1);
        MR::startBckPlayer(sMonteThrowPlayerAnimation, 0);
        MR::setVelocity(this, -mGravity * mThrowSpeed);
        MR::validateHitSensor(this, "ItemGet");
    }
    if (MR::isStep(this, 25)) MR::startLevelSoundPlayer("JumpStandAuto", -1, -1);
    if (MR::isStep(this, 10)) releasePlayer();
    bool airborne;
    if (MR::isOnGroundPlayer()) airborne = false;
    else if (MR::isPlayerOnWaterSurface()) airborne = false;
    else airborne = !MR::isPlayerInWaterMode();
    if (!airborne) {
        MR::validateClipping(this);
        MR::endActorCamera(mHost, mCameraInfo, true, -1);
        setNerve(&NrvMonteThrow::Wait::sInstance);
    }
}
