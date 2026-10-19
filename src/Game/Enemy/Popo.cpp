#define JGEOMETRY_VEC3_ZERO_NOINLINE
#include "Game/Enemy/Popo.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/MapObj/PetitPorter.hpp"

#include "Game/Util/YoshiLockOnUtil.hpp"
#include "Game/Enemy/PetoPopoParam.hpp"

// Named tuning constants: the target loads equal values from separate .sdata2 slots (e.g. both 30.0f
// blow-down speeds), so they are objects defined at the end of the TU, not folded literals.
namespace {
    extern const f32 sFloatKeepHeight;
    extern const f32 sFloatPushPower;
    extern const f32 sFloatPullPower;
    extern const f32 sFloatMaxPower;
    extern const f32 sBlowDownH;
    extern const f32 sBlowDownV;
}

struct SightParam {
    f32 a;
    f32 b;
    f32 c;
};

static const SightParam sSightParam = {5000.0f, 90.0f, 45.0f};

f32 clampRate01(f32) NO_INLINE;

// Nerve helpers whose bodies are inlined into the nerve (defined before NEW_NERVE).


void Popo::exeLick() {
}

void Popo::endWait() {
    MR::invalidateClipping(this);
}



void Popo::endReturn() {
    MR::onBind(this);
    MR::validateHitSensors(this);
}

namespace NrvPopo {
    NEW_NERVE_ONEND(PopoNrvWait, Popo, Wait, Wait);
    NEW_NERVE(PopoNrvFind, Popo, Find);
    NEW_NERVE(PopoNrvWalk, Popo, Walk);
    NEW_NERVE(PopoNrvEscape, Popo, Escape);
    NEW_NERVE(PopoNrvStop, Popo, Stop);
    NEW_NERVE(PopoNrvRest, Popo, Rest);
    NEW_NERVE_ONEND(PopoNrvReturn, Popo, Return, Return);
    NEW_NERVE(PopoNrvPressDown, Popo, PressDown);
    NEW_NERVE(PopoNrvBlowDown, Popo, BlowDown);
    NEW_NERVE(PopoNrvRecover, Popo, Recover);
    NEW_NERVE(PopoNrvStick, Popo, Stick);
    NEW_NERVE(PopoNrvStickStart, Popo, StickStart);
    NEW_NERVE(PopoNrvStickWait, Popo, StickWait);
    NEW_NERVE(PopoNrvLick, Popo, Lick);
    NEW_NERVE(PopoNrvPressDownWait, Popo, PressDownWait);
    NEW_NERVE(PopoNrvReaction, Popo, Reaction);
    NEW_NERVE(PopoNrvKicked, Popo, Kicked);
    NEW_NERVE_ONEND(PopoNrvTouch, Popo, Touch, Touch);
    NEW_NERVE(PopoNrvStickCancel, Popo, StickCancel);
};  // namespace NrvPopo

namespace {
    static PetoPopoParam sScaleParam;
};  // namespace

Popo::Popo(const char* pName) : LiveActor(pName), mFrontVec(0.0f, 0.0f, 0.0f), mYoshiLockOn(nullptr), mAwayCounter(nullptr), mScaleController(nullptr),
                                mLockOnOffset(0.0f, 0.0f, 0.0f), mIsMarioBlow(false) {
}

void Popo::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(&NrvPopo::PopoNrvWait::sInstance, 0);
    MR::createSceneObj(SceneObj_PopoStickingManager);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "AllRoot", mLockOnOffset);
    mAwayCounter = new PopoAwayCounter(this);
    mScaleController = new AnimScaleController(&sScaleParam);
}

// NONMATCHING: stack slot order of expression temporaries
void Popo::initAfterPlacement() {
    MR::calcGravity(this);
    TVec3f start(mPosition - mGravity * sFloatKeepHeight);
    TVec3f line = mGravity * sFloatKeepHeight * 2.0f;
    TVec3f hitPos;
    if (MR::getFirstPolyOnLineToMap(&hitPos, nullptr, start, line)) {
        TVec3f pos(hitPos - mGravity * sFloatKeepHeight);
        MR::setPosition(this, pos);
    }
    MR::calcFrontVec(&mFrontVec, this);
    makeActorAppeared();
    mHomePos = mPosition;
}

void Popo::appear() {
    LiveActor::appear();
}

void Popo::kill() {
    MR::startActionSound(this, "Explode", -1, -1, -1);
    LiveActor::kill();
}

void Popo::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
        MR::disableAllItemGenerator(this);
    }
    calcFloatVelocity();
    calcTurnVelocity(0.95f);
    YoshiFunction::tryYoshiLick(mYoshiLockOn);
    if (MR::isGreaterEqualStep(this, 20)) {
        if (tryFindYoshi()) {
            return;
        }
    }
}

void Popo::exeFind() {
    if (MR::isFirstStep(this)) {
        mVelocity.zero();
        MR::startAction(this, "Find");
    }
    calcFloatVelocity();
    calcTurnVelocity(0.95f);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFrontVec, 10.0f);
    YoshiFunction::tryYoshiLick(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        if (MR::isPlayerElementModeYoshi()) {
            setNerve(&NrvPopo::PopoNrvWalk::sInstance);
            return;
        }
        setNerve(&NrvPopo::PopoNrvEscape::sInstance);
    }
}

// NONMATCHING: register allocation (r30/r31, f0/f1)
void Popo::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        mAwayCounter->mCount = 0;
    }
    mAwayCounter->update(mHomePos);
    SightParam sight = sSightParam;
    if (!MR::isPlayerElementModeYoshi() || mAwayCounter->mCount >= 120 || !MR::isInSightFanPlayer(this, mFrontVec, sight.a, sight.b, sight.c)) {
        setNerve(&NrvPopo::PopoNrvStop::sInstance);
        return;
    }
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFrontVec, 5.0f);
    TPos3f mouthMtx;
    mouthMtx.set(MR::getYoshiMouthMtx());
    TVec3f mouthPos;
    mouthMtx.getTrans(mouthPos);
    TVec3f toMouth(mouthPos);
    toMouth -= mPosition;
    if (toMouth.dot(mGravity) < 0.0f) {
        const TVec3f* pGravity = &mGravity;
        JMAVECScaleAdd(pGravity, &toMouth, &toMouth, -pGravity->dot(toMouth));
    }
    if (MR::normalizeOrZero(&toMouth)) {
        toMouth = mFrontVec;
    }
    MR::addVelocityMoveToDirection3D(this, toMouth, 0.8f);
    if (MR::isShadowProjected(this, nullptr)) {
        TVec3f shadowPos;
        MR::getShadowProjectionPos(this, nullptr, &shadowPos);
        f32 heightRate = 1.0f - PSVECDistance(&shadowPos, &mPosition) / 400.0f;
        f32 heightScale = clampRate01(heightRate * heightRate);
        TVec3f horizontal;
        const TVec3f* pPlayerPos = MR::getPlayerPos();
        TVec3f toPlayer(mPosition);
        toPlayer -= *pPlayerPos;
        MR::vecKillElement(toPlayer, mGravity, &horizontal);
        f32 distRate = PSVECMag(&horizontal) / 300.0f;
        f32 distScale = clampRate01(distRate * distRate);
        mVelocity.add(-mGravity * 0.05f * heightScale * distScale);
    }
    calcTurnVelocity(0.95f);
    YoshiFunction::tryYoshiLick(mYoshiLockOn);
}

f32 clampRate01(f32 x) {
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

void Popo::exeEscape() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Escape");
    }
    if (MR::isPlayerElementModeYoshi()) {
        setNerve(&NrvPopo::PopoNrvFind::sInstance);
        return;
    }
    bool far = !MR::isNearPlayerHorizontal(this, 1000.0f);
    if (far) {
        setNerve(&NrvPopo::PopoNrvStop::sInstance);
        return;
    }
    calcFloatVelocity();
    MR::turnDirectionFromPlayerDegreeHorizon(this, &mFrontVec, 5.0f);
    MR::addVelocityMoveToDirection3D(this, mFrontVec, 0.8f);
    calcTurnVelocity(0.95f);
    YoshiFunction::tryYoshiLick(mYoshiLockOn);
}

void Popo::exeStop() {
    calcFloatVelocity();
    mVelocity.x *= 0.98f;
    mVelocity.y *= 0.98f;
    mVelocity.z *= 0.98f;
    if (MR::isGreaterEqualStep(this, 60)) {
        setNerve(&NrvPopo::PopoNrvRest::sInstance);
    }
}

void Popo::exeRest() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }
    YoshiFunction::tryYoshiLick(mYoshiLockOn);
    MR::turnDirectionToTargetDegreeHorizon(this, &mFrontVec, mHomePos, 3.0f);
    if (!tryFindYoshi()) {
        if (isBehind()) {
            calcReturnVelocity();
            return;
        }
        decelerate();
        f32 sq = mVelocity.squared();
        f32 limit = 1.0f;
        bool slow = sq < limit * limit;
        if (slow) {
            setNerve(&NrvPopo::PopoNrvReturn::sInstance);
        }
    }
}

// NONMATCHING: f0/f3 allocation for the 0.9f/0.5f literals
void Popo::exeReturn() {
    MR::turnDirectionToTargetDegreeHorizon(this, &mFrontVec, mHomePos, 3.0f);
    YoshiFunction::tryYoshiLick(mYoshiLockOn);
    TVec3f toHome(mHomePos);
    toHome -= mPosition;
    f32 dist = PSVECMag(&toHome);
    if (MR::isNearZero(dist, 0.001f)) {
        toHome = mFrontVec;
    } else {
        toHome.scale(1.0f / dist);
    }
    f32 dotv = toHome.dot(mVelocity);
    f32 one = 1.0f;
    f32 damp = 0.9f;
    f32 diff = one - damp;
    f32 along = 0.5f + dotv;
    f32 limit = dist * diff;
    if (limit < along) {
        TVec3f v(toHome);
        v.scale(limit);
        mVelocity.set(v);
    } else {
        TVec3f v(toHome);
        v.scale(along);
        mVelocity.set(v);
    }
    TVec3f* pVel = &mVelocity;
    if (PSVECMag(pVel) > 10.0f) {
        pVel->setLength(10.0f);
    }
    if (!tryFindYoshi() && (PSVECDistance(&mPosition, &mHomePos) < 1.0f || MR::isGreaterEqualStep(this, 600))) {
        setNerve(&NrvPopo::PopoNrvWait::sInstance);
    }
}

void Popo::exePressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PressDown");
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void Popo::exeBlowDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
        if (mIsMarioBlow) {
            MR::startBrk(this, "Normal");
        }
    }
    TVec3f negVel = -mVelocity;
    MR::turnDirectionDegree(this, &mFrontVec, negVel, 30.0f);
    calcBlowDownVelocity();
    if ((MR::isBindedGround(this) && MR::isGreaterStep(this, 15)) || MR::isGreaterStep(this, 20)) {
        MR::enableAllItemGenerator(this);
        MR::emitItemHit(this);
        kill();
    }
}

void Popo::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::enableAllItemGenerator(this);
    }
    calcFloatVelocity();
    calcTurnVelocity(0.95f);
    if (MR::isGreaterEqualStep(this, 60)) {
        MR::disableAllItemGenerator(this);
        setNerve(&NrvPopo::PopoNrvRest::sInstance);
    }
}

// NONMATCHING: stack slot order of the three expression temporaries (rotated by one)
void Popo::exeStick() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Stick");
        MR::stopSceneForDefaultHit(8);
        mVelocity.zero();
    }
    tryReleaseYoshi();
    PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
    TVec3f mouthPos;
    pManager->getMouthMtx()->getTrans(mouthPos);
    TVec3f playerUp;
    MR::getPlayerUpVec(&playerUp);
    const TVec3f* pPlayerPos = MR::getPlayerPos();
    f32 height = playerUp.dot(mouthPos - *pPlayerPos);
    TVec3f center(*MR::getPlayerPos() + playerUp * height * 0.5f);
    TVec3f dirMouth(mouthPos);
    dirMouth -= center;
    f32 lenMouth = PSVECMag(&dirMouth);
    if (MR::isNearZero(lenMouth, 0.001f)) {
        MR::getPlayerFrontVec(&dirMouth);
    } else {
        dirMouth.scale(1.0f / lenMouth);
    }
    TVec3f dirSelf(mPosition);
    dirSelf -= center;
    f32 lenSelf = PSVECMag(&dirSelf);
    if (MR::isNearZero(lenSelf, 0.001f)) {
        MR::getPlayerFrontVec(&dirSelf);
    } else {
        dirSelf.scale(1.0f / lenSelf);
    }
    s32 stickTime = 20;
    f32 rate = (f32)getNerveStep() / (f32)stickTime;
    if (rate > 1.0f) {
        rate = 1.0f;
    }
    TQuat4f quat;
    quat.setRotate(dirSelf, dirMouth, rate);
    quat.transform(dirSelf, mFrontVec);
    TVec3f newPos;
    JMAVECScaleAdd(&mFrontVec, &center, &newPos, lenSelf + rate * (lenMouth - lenSelf));
    MR::setPosition(this, newPos);
    mFrontVec.negate();
    if (MR::isGreaterEqualStep(this, 20)) {
        setNerve(&NrvPopo::PopoNrvStickWait::sInstance);
    }
}

void Popo::exeStickWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StickWait");
    }
    tryReleaseYoshi();
}

void Popo::exeStickStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Stick");
        MR::stopSceneForDefaultHit(8);
    }
    tryReleaseYoshi();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvPopo::PopoNrvStickWait::sInstance);
    }
}

void Popo::exePressDownWait() {
    if (MR::isActionEnd(this)) {
        MR::enableAllItemGenerator(this);
        MR::emitItemStomp(this);
        kill();
    }
}

void Popo::exeReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Reaction");
    }
    tryReleaseYoshi();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvPopo::PopoNrvStickWait::sInstance);
    }
}

void Popo::exeKicked() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BlowDown");
        MR::startBrk(this, "Normal");
        MR::enableAllItemGenerator(this);
    }
    calcBlowVelocity(30, getNerveStep());
    if (MR::isGreaterEqualStep(this, 5)) {
        MR::emitItemHit(this);
        kill();
    }
}

void Popo::endTouch() {
    mScaleController->startAnim();
    MR::deleteEffect(this, "Touch");
}

void Popo::exeTouch() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
        MR::stopBck(this);
        mScaleController->startDpdHitVibration();
        MR::emitEffect(this, "Touch");
        mVelocity.zero();
    }
    MR::startDPDFreezeLevelSound(this);
    if (!MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(&NrvPopo::PopoNrvRest::sInstance);
    }
}

// NONMATCHING: vector temporary stack slots differ after matching copy construction.
void Popo::exeStickCancel() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StickCancel");
        mVelocity.zero();
        Triangle tri;
        f32 radius = MR::getSensorRadius(this, "Body");
        TVec3f hitPos;
        TVec3f offset = mGravity * radius;
        TVec3f from(mPosition);
        from -= offset;
        TVec3f line = mGravity * radius * 2.0f;
        if (MR::getFirstPolyOnLineToMap(&hitPos, &tri, from, line)) {
            TVec3f up = *tri.getNormal(0) * radius;
            TVec3f pos(hitPos);
            pos.add(up);
            MR::setPosition(this, pos);
        }
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvPopo::PopoNrvEscape::sInstance);
    }
}

void Popo::tryShootOut() {
    bool sticking = false;
    if (isNerve(&NrvPopo::PopoNrvStickStart::sInstance) || isNerve(&NrvPopo::PopoNrvStickWait::sInstance) ||
        isNerve(&NrvPopo::PopoNrvReaction::sInstance)) {
        sticking = true;
    }
    if (sticking || isNerve(&NrvPopo::PopoNrvStick::sInstance)) {
        PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
        TVec3f front;
        pManager->releaseStickPopo(this);
        MR::getPlayerFrontVec(&front);
        startBlowDown(front, false);
        MR::startActionSound(this, "TakeOff", -1, -1, -1);
    }
}

void Popo::tryReaction() {
    bool sticking = false;
    if (isNerve(&NrvPopo::PopoNrvStickStart::sInstance) || isNerve(&NrvPopo::PopoNrvStickWait::sInstance) ||
        isNerve(&NrvPopo::PopoNrvReaction::sInstance)) {
        sticking = true;
    }
    if (sticking && !isNerve(&NrvPopo::PopoNrvReaction::sInstance)) {
        setNerve(&NrvPopo::PopoNrvReaction::sInstance);
    }
}

void Popo::control() {
    if (isStarPointerTargetable() && MR::isStarPointerPointing2PAttachTarget(this)) {
        setNerve(&NrvPopo::PopoNrvTouch::sInstance);
    }
    *reinterpret_cast< TVec3f* >(reinterpret_cast< u8* >(mYoshiLockOn) + 8) = mLockOnOffset;
    mScaleController->updateNerve();
}

void Popo::calcAndSetBaseMtx() {
    bool sticking = false;
    if (isNerve(&NrvPopo::PopoNrvStickStart::sInstance) || isNerve(&NrvPopo::PopoNrvStickWait::sInstance) ||
        isNerve(&NrvPopo::PopoNrvReaction::sInstance)) {
        sticking = true;
    }
    if (sticking) {
        PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
        MR::setBaseTRMtx(this, *pManager->getMouthMtx());
        pManager->getMouthMtx()->getTrans(mPosition);
        TPos3f* pMtx = pManager->getMouthMtx();
        f32 z = pMtx->mMtx[2][2];
        f32 y = pMtx->mMtx[1][2];
        f32 x = pMtx->mMtx[0][2];
        mFrontVec.set< f32 >(x, y, z);
    } else {
        MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontVec);
    }
    MR::updateBaseScale(this, mScaleController);
}

void Popo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isPlayerElementModeYoshi()) {
            if (isStickable()) {
                PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
                if (!pManager->isStick()) {
                    PopoStickingManager* pManager2 = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
                    if (pManager2->trySetStickPopo(this)) {
                        setNerve(&NrvPopo::PopoNrvStick::sInstance);
                    }
                }
            }
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else if (isAttackable()) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Popo::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvPopo::PopoNrvTouch::sInstance)) {
        return false;
    }
    if (MR::isSensorPlayerOrRide(pSender)) {
        MR::addVelocityFromPushHorizon(this, 0.8f, pSender, pReceiver);
        return true;
    }
    return false;
}

bool Popo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        if (isStarPieceReactable()) {
            mScaleController->startHitReaction();
            MR::startActionSound(this, "StarPieceHit", -1, -1, -1);
        }
        return true;
    }
    if (!isBlowable()) {
        return false;
    }
    if (MR::isMsgPlayerTrample(msg)) {
        MR::startAction(this, "PressDown");
        mVelocity.zero();
        MR::offBind(this);
        MR::invalidateHitSensors(this);
        setNerve(&NrvPopo::PopoNrvPressDownWait::sInstance);
        return true;
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::sendMsgAwayJump(pSender, pReceiver);
        MR::startAction(this, "PressDown");
        mVelocity.zero();
        MR::offBind(this);
        MR::invalidateHitSensors(this);
        setNerve(&NrvPopo::PopoNrvPressDownWait::sInstance);
        return true;
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (MR::isPlayerElementModeYoshi() && !MR::isMsgSupportTicoSpin(msg)) {
            if (isStickable()) {
                PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
                if (!pManager->isStick()) {
                    PopoStickingManager* pManager2 = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
                    if (pManager2->trySetStickPopo(this)) {
                        setNerve(&NrvPopo::PopoNrvStick::sInstance);
                    }
                }
            }
        } else {
            TVec3f dir = *MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender);
            if (MR::normalizeOrZero(&dir)) {
                MR::getPlayerFrontVec(&dir);
            }
            startBlowDown(dir, true);
        }
        return true;
    }
    return false;
}

bool Popo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvPopo::PopoNrvTouch::sInstance)) {
        if (MR::isMsgPlayerKick(msg)) {
            TVec3f dir = *MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender);
            if (MR::normalizeOrZero(&dir)) {
                TVec3f neg = -mFrontVec;
                dir = neg;
            }
            dir -= mGravity;
            MR::normalizeOrZero(&dir);
            TVec3f vel(dir);
            vel.scale(80.0f);
            mVelocity.set(vel);
            setNerve(&NrvPopo::PopoNrvKicked::sInstance);
            return true;
        }
    }
    if (isFree()) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            setNerve(&NrvPopo::PopoNrvLick::sInstance);
            return true;
        }
        return false;
    }
    if (isNerve(&NrvPopo::PopoNrvLick::sInstance)) {
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
            if (pManager->trySetStickPopo(this)) {
                setNerve(&NrvPopo::PopoNrvStickStart::sInstance);
            } else {
                setNerve(&NrvPopo::PopoNrvWait::sInstance);
            }
            return true;
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            setNerve(&NrvPopo::PopoNrvWait::sInstance);
            return true;
        }
        return false;
    }
    return false;
}

bool Popo::isFree() const {
    bool result = false;
    if (isNerve(&NrvPopo::PopoNrvWait::sInstance) || isNerve(&NrvPopo::PopoNrvFind::sInstance) || isNerve(&NrvPopo::PopoNrvWalk::sInstance) ||
        isNerve(&NrvPopo::PopoNrvEscape::sInstance) || isNerve(&NrvPopo::PopoNrvStop::sInstance) || isNerve(&NrvPopo::PopoNrvRest::sInstance) ||
        isNerve(&NrvPopo::PopoNrvReturn::sInstance) || isNerve(&NrvPopo::PopoNrvStickCancel::sInstance)) {
        result = true;
    }
    return result;
}

// NONMATCHING: lfs vs fmr for the duplicated 0.1f argument
void Popo::calcFloatVelocity() {
    TVec3f vert;
    f32 speed = MR::vecKillElement(mVelocity, mGravity, &vert);
    if (!MR::isShadowProjected(this, nullptr)) {
        mVelocity.set(vert + mGravity * speed * 0.99f * 0.99f);
        return;
    }
    mVelocity.set(vert + mGravity * speed * 0.99f);
    if (!MR::addVelocityKeepHeightUseShadow(this, sFloatKeepHeight, sFloatPushPower, sFloatPullPower, sFloatMaxPower, "Body")) {
        mVelocity.add(mGravity * sFloatPushPower);
    }
}

void Popo::calcTurnVelocity(f32 rate) {
    f32 speed = MR::vecKillElement(mVelocity, mGravity, &mVelocity);
    mVelocity.scale(rate);
    f32 v = clampVelocity(speed);
    mVelocity.add(mGravity * v);
}

f32 Popo::clampVelocity(f32 x) {
    f32 maxFall = 18.0f;
    f32 low = -maxFall;
    if (x < low) {
        return low;
    }
    if (x > 25.0f) {
        return 25.0f;
    }
    return x;
}

bool Popo::isStickable() const {
    bool result = false;
    if (isNerve(&NrvPopo::PopoNrvWait::sInstance) || isNerve(&NrvPopo::PopoNrvFind::sInstance) || isNerve(&NrvPopo::PopoNrvWalk::sInstance) ||
        isNerve(&NrvPopo::PopoNrvStop::sInstance) || isNerve(&NrvPopo::PopoNrvRecover::sInstance)) {
        result = true;
    }
    return result;
}

bool Popo::isAttackable() const {
    bool result = false;
    if (isNerve(&NrvPopo::PopoNrvWait::sInstance) || isNerve(&NrvPopo::PopoNrvFind::sInstance) || isNerve(&NrvPopo::PopoNrvWalk::sInstance) ||
        isNerve(&NrvPopo::PopoNrvEscape::sInstance) || isNerve(&NrvPopo::PopoNrvStop::sInstance) ||
        isNerve(&NrvPopo::PopoNrvStickCancel::sInstance)) {
        result = true;
    }
    return result;
}

void Popo::tryReleaseYoshi() {
    if (!MR::isPlayerElementModeYoshi() || PetitPorterFunction::isDemo()) {
        PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
        pManager->releaseStickPopo(this);
        setNerve(&NrvPopo::PopoNrvStickCancel::sInstance);
    }
}

// NONMATCHING: order of the initial li for the two flags
bool Popo::isBlowable() const {
    bool free = false;
    bool result = true;
    if (isNerve(&NrvPopo::PopoNrvWait::sInstance) || isNerve(&NrvPopo::PopoNrvFind::sInstance) || isNerve(&NrvPopo::PopoNrvWalk::sInstance) ||
        isNerve(&NrvPopo::PopoNrvEscape::sInstance) || isNerve(&NrvPopo::PopoNrvStop::sInstance) || isNerve(&NrvPopo::PopoNrvTouch::sInstance) ||
        isNerve(&NrvPopo::PopoNrvStickCancel::sInstance)) {
        free = true;
    }
    if (!free && !isNerve(&NrvPopo::PopoNrvTouch::sInstance)) {
        result = false;
    }
    return result;
}

// NONMATCHING: f30/f31 allocation
void Popo::calcBlowVelocity(s32 total, s32 step) {
    f32 rate = (f32)step / (f32)total;
    rate = rate * rate;
    f32 speed = MR::vecKillElement(mVelocity, mGravity, &mVelocity);
    mVelocity.add((TVec3f(0.0f) - mVelocity) * rate);
    f32 v = clampVelocity(speed);
    mVelocity.add(mGravity * v);
}

bool Popo::isBehind() const {
    TVec3f diff(mPosition);
    diff -= mHomePos;
    return mGravity.dot(diff) > 0.0f;
}

// NONMATCHING: order of the two literal loads
void Popo::calcReturnVelocity() {
    TVec3f horizontal;
    f32 speed = MR::vecKillElement(mVelocity, mGravity, &horizontal);
    horizontal.scale(0.98f);
    f32 decay = 0.5f;
    f32 maxDown = 10.0f;
    f32 v = speed - decay;
    if (v < -maxDown) {
        v = -maxDown;
    }
    TVec3f result = mGravity * v + horizontal;
    mVelocity.set(result);
}

void Popo::decelerate() {
    mVelocity.x *= 0.95f;
    mVelocity.y *= 0.95f;
    mVelocity.z *= 0.95f;
}

// NONMATCHING: f0/f5 allocation of the x/z matrix column loads
bool Popo::tryFindYoshi() {
    MtxPtr pMtx = MR::getJointMtx(this, "JointRoot");
    TVec3f dir;
    SightParam sight;
    f32 z = pMtx[2][0];
    f32 y = pMtx[1][0];
    f32 x = pMtx[0][0];
    dir.x = x;
    dir.y = y;
    dir.z = z;
    sight.a = 800.0f;
    sight.b = 90.0f;
    sight.c = 45.0f;
    if (MR::isInSightFanPlayer(this, dir, sight.a, sight.b, sight.c)) {
        PopoStickingManager* pManager = MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager);
        if (!pManager->isStick()) {
            setNerve(&NrvPopo::PopoNrvFind::sInstance);
            return true;
        }
    }
    return false;
}

bool Popo::isStarPointerTargetable() const {
    bool result = false;
    if (isNerve(&NrvPopo::PopoNrvWait::sInstance) || isNerve(&NrvPopo::PopoNrvFind::sInstance) || isNerve(&NrvPopo::PopoNrvWalk::sInstance) ||
        isNerve(&NrvPopo::PopoNrvEscape::sInstance) || isNerve(&NrvPopo::PopoNrvStop::sInstance) || isNerve(&NrvPopo::PopoNrvRest::sInstance) ||
        isNerve(&NrvPopo::PopoNrvReturn::sInstance) || isNerve(&NrvPopo::PopoNrvRecover::sInstance) ||
        isNerve(&NrvPopo::PopoNrvStickCancel::sInstance)) {
        result = true;
    }
    return result;
}

void Popo::startBlowDown(const TVec3f& rDir, bool isMario) {
    mVelocity.zero();
    MR::setVelocitySeparateHV(this, rDir, sBlowDownH, sBlowDownV);
    setNerve(&NrvPopo::PopoNrvBlowDown::sInstance);
    mIsMarioBlow = isMario;
    if (!MR::isPlayerElementModeYoshi()) {
        MR::startActionSound(this, "MarioBlowDown", -1, -1, -1);
    }
}

void Popo::calcBlowDownVelocity() {
    MR::attenuateVelocity(this, MR::isOnGround(this) ? 0.93f : 0.99f);
    if (!MR::isOnGround(this)) {
        MR::addVelocityToGravity(this, 1.3f);
    }
    if (MR::isBindedWall(this)) {
        MR::calcReboundVelocity(&mVelocity, MR::getWallNormal(this), 0.2f, 0.7f);
    }
}

bool Popo::isStarPieceReactable() const {
    bool result = false;
    if (isNerve(&NrvPopo::PopoNrvWait::sInstance) || isNerve(&NrvPopo::PopoNrvFind::sInstance) || isNerve(&NrvPopo::PopoNrvWalk::sInstance) ||
        isNerve(&NrvPopo::PopoNrvEscape::sInstance) || isNerve(&NrvPopo::PopoNrvStop::sInstance) || isNerve(&NrvPopo::PopoNrvRest::sInstance) ||
        isNerve(&NrvPopo::PopoNrvReturn::sInstance) || isNerve(&NrvPopo::PopoNrvRecover::sInstance) ||
        isNerve(&NrvPopo::PopoNrvStickWait::sInstance) || isNerve(&NrvPopo::PopoNrvStickCancel::sInstance)) {
        result = true;
    }
    return result;
}

void PopoAwayCounter::update(const TVec3f& rHome) {
    if (MR::calcDistanceHorizontal(mHost, rHome) > 1200.0f || MR::calcDistanceVertical(mHost, rHome) > 1200.0f) {
        if (mCount < 120) {
            mCount++;
        }
    } else {
        mCount = 0;
    }
}

Popo::~Popo() {
}

PopoStickingManager::PopoStickingManager(const char* pName) : NameObj(pName), mMouthPosition(nullptr), mStickPopo(nullptr) {
}

void PopoStickingManager::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, -1, 10, -1, -1);
}

void PopoStickingManager::initAfterPlacement() {
    if (MR::fn_80421D10()) {
        mMouthPosition = new FixedPosition(MR::getYoshiMouthMtx(), TVec3f(85.0f, -65.0f, 0.0f), TVec3f(-65.0f, 90.0f, -280.0f));
    }
}

void PopoStickingManager::calcAnim() {
    if (mMouthPosition != nullptr) {
        mMouthPosition->calc();
    }
}

bool PopoStickingManager::trySetStickPopo(Popo* pPopo) {
    if (mStickPopo != nullptr) {
        return false;
    }
    mStickPopo = pPopo;
    return true;
}

void PopoStickingManager::releaseStickPopo(Popo* pPopo) {
    if (mStickPopo == pPopo) {
        mStickPopo = nullptr;
    }
}

bool PopoStickingManager::isStick() const {
    return mStickPopo != nullptr;
}

TPos3f* PopoStickingManager::getMouthMtx() {
    return &mMouthPosition->mMtx;
}

void PopoStickingManager::tryShootOut() {
    if (mStickPopo != nullptr) {
        mStickPopo->tryShootOut();
    }
}

void PopoStickingManager::tryReaction() {
    if (mStickPopo != nullptr) {
        mStickPopo->tryReaction();
    }
}

PopoStickingManager::~PopoStickingManager() {
}

bool isPopoSticking() {
    if (!MR::isExistSceneObj(SceneObj_PopoStickingManager)) {
        return false;
    }
    return MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager)->mStickPopo != nullptr;
}

void tryShootStickingPopo() {
    if (MR::isExistSceneObj(SceneObj_PopoStickingManager)) {
        MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager)->tryShootOut();
    }
}

void tryReactStickingPopo() {
    if (MR::isExistSceneObj(SceneObj_PopoStickingManager)) {
        MR::getSceneObj< PopoStickingManager* >(SceneObj_PopoStickingManager)->tryReaction();
    }
}

namespace {
    const f32 sFloatKeepHeight = 120.0f;
    const f32 sFloatPushPower = 0.1f;
    const f32 sFloatPullPower = 0.1f;
    const f32 sFloatMaxPower = 200.0f;
    const f32 sBlowDownH = 30.0f;
    const f32 sBlowDownV = 30.0f;
}
