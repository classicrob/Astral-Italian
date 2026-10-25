#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

namespace MR {
    bool appearCoinPop(const NameObj*, const TVec3f&, s32);
};  // namespace MR

namespace NrvSandyBall {
    NEW_NERVE(SandyBallNrvWait, SandyBall, Wait);
    NEW_NERVE(SandyBallNrvFly, SandyBall, Fly);
    NEW_NERVE(SandyBallNrvBreak, SandyBall, Break);
    NEW_NERVE(SandyBallNrvLand, SandyBall, Land);
    NEW_NERVE(SandyBallNrvRoll, SandyBall, Roll);
    NEW_NERVE(SandyBallNrvExplosion, SandyBall, Explosion);
    NEW_NERVE(SandyBallNrvBindStarPointer, SandyBall, BindStarPointer);
};  // namespace NrvSandyBall

static const char* const sSandyBallNames[] = {"\203T\203{\203e\203\223", "\203S\201[\203\213\203h", "\203T\203\223\203h", "\224\232\222e", "\203T\203{\203e\203\223\226[", nullptr};
static TVec3f sBombTargetPos;

// 800FBA60
SandyBall::SandyBall(s32 type, const char* pName)
    : LiveActor(pName), mOwner(nullptr), mPath(new ParabolicPath()), mFlyFrame(0), mStep(0), mVel(TVec3f(0.0f)), mStartPos(TVec3f(0.0f)),
      mScaleRate(0.0f), mType(type), mCoinPopped(false), mSensorRadius(0.0f), mScaleController(nullptr), mBindStarPointer(nullptr),
      mPrevNerve(nullptr) {
    setName(sSandyBallNames[type]);
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

// 800FBB60
void SandyBall::init(const JMapInfoIter& rIter) {
    const char* pActorName;
    if (mType == 2) {
        pActorName = "SandySandBall";
    } else if (mType == 3) {
        pActorName = "SandyCactusBallBig";
    } else {
        pActorName = "SandyCactusBall";
    }
    MR::initActor(this, rIter, pActorName, false);
    initNerve(&NrvSandyBall::SandyBallNrvWait::sInstance, 1);
    mScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, &NrvSandyBall::SandyBallNrvBindStarPointer::sInstance, nullptr);
    MR::offBind(this);
    if (mType == 1) {
        MR::declareCoin(this, 1);
    }
    makeActorDead();
}

// 800FBC60
void SandyBall::appear() {
    LiveActor::appear();
    setBtpFrame();
    if (mType == 1) {
        MR::tryHitReaction(this, "\217o\214\273(\203S\201[\203\213\203h)");
    } else {
        MR::tryHitReactionAppear(this);
    }
    MR::invalidateShadow(this, nullptr);
    setNerve(&NrvSandyBall::SandyBallNrvWait::sInstance);
}

// 800FBCE0
void SandyBall::kill() {
    mOwner = nullptr;
    MR::offBind(this);
    MR::validateClipping(this);
    MR::invalidateHitSensors(this);
    mStep = 0;
    setNerve(&NrvSandyBall::SandyBallNrvWait::sInstance);
    LiveActor::kill();
}

// 800FBD50
// NONMATCHING: TVec3f operator+/negateInternal inline-vs-call decisions differ (target was a separate TU).
namespace {
    inline void calcSandyHandBallTarget(TVec3f* pOut, SandyBall* ball, SandyHand* pHand, Sandy* pSandy) {
        TVec3f groundPos;
        MR::getPlayerGroundPos(&groundPos);
        TVec3f diff = groundPos - pSandy->mPosition;
        if (PSVECMag(&diff) < 1700.0f) {
            diff.setLength(1700.0f);
        } else if (2500.0f < PSVECMag(&diff)) {
            diff.setLength(2500.0f);
        }
        groundPos.set(pSandy->mPosition + diff);
        TVec3f toGround = groundPos - ball->mPosition;
        TVec3f hitPos(ball->mPosition);
        TVec3f start = (ball->mPosition + toGround) - ball->mGravity.scaleInline(1000.0f);
        if (MR::getFirstPolyOnLineToMap(&hitPos, nullptr, start, ball->mGravity.scaleInline(2000.0f))) {
            toGround.set(hitPos - ball->mPosition);
        }
        TVec3f handUp;
        handUp.negate(pHand->mGravity);
        if (pSandy->isMinisDownOrDead()) {
            TVec3f side(toGround);
            MR::vecKillElement(side, ball->mGravity, &side);
            TVec3f cross;
            PSVECCrossProduct(&side, &handUp, &cross);
            if (!pHand->mIsRight) {
                cross.negate();
            }
            cross.setLength(1.3f * MR::getSensorRadius(ball, "Body"));
            pOut->set((ball->mPosition + toGround) + cross);
        } else {
            f32 angle = 17.0f;
            if (pHand->mIsRight) {
                angle *= -1.0f;
            }
            MR::rotateVecDegree(&toGround, handUp, angle);
            pOut->set(toGround + ball->mPosition);
        }
    }
}

void SandyBall::calcTargetPos(TVec3f* pOut) {
    if (mType == 3) {
        SandyHand* pHand = static_cast< SandyHand* >(mOwner);
        Sandy* pSandy = pHand->mSandy;
        calcSandyHandBallTarget(pOut, this, pHand, pSandy);
    } else {
        MR::getPlayerGroundPos(pOut);
    }
}

s32 SandyBall::getWaitSecond() const {
    return 30;
}

// 800FBFC0
bool SandyBall::tryShoot() {
    if (MR::isDead(this)) {
        return false;
    }
    if (isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        return false;
    }
    if (!isNerve(&NrvSandyBall::SandyBallNrvWait::sInstance)) {
        return false;
    }
    TVec3f target;
    calcTargetPos(&target);
    if (mType == 3) {
        sBombTargetPos.set(target);
    }
    TVec3f dir = target - mPosition;
    if (MR::normalizeOrZero(&dir)) {
        kill();
        return false;
    }
    MR::faceToVector(&mQuat, TVec3f(dir), 180.0f);
    shoot(mPosition, target);
    return true;
}

// 800FC0D0
void SandyBall::shoot(const TVec3f& rFrom, const TVec3f& rTo) {
    mPosition.set(rFrom);
    initPath(rFrom, rTo);
    MR::onBind(this);
    MR::invalidateClipping(this);
    mStep = 0;
    setNerve(&NrvSandyBall::SandyBallNrvFly::sInstance);
}

// 800FC150
void SandyBall::setOwner(LiveActor* pOwner) {
    mOwner = pOwner;
}

// 800FC160
bool SandyBall::isFree() const {
    bool ret = false;
    if (MR::isDead(this) && mOwner == nullptr) {
        ret = true;
    }
    return ret;
}

// 800FC1C0
bool SandyBall::tryBreak() {
    if (MR::isDead(this)) {
        return false;
    }
    if (isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        return false;
    }
    if (isNerve(&NrvSandyBall::SandyBallNrvRoll::sInstance)) {
        kill();
    } else {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
    }
    return true;
}

// 800FC260
bool SandyBall::tryKill() {
    if (MR::isDead(this)) {
        return false;
    }
    makeActorDead();
    return true;
}

// 800FC2C0
bool SandyBall::tryStartBindStarPointer() {
    if (!mBindStarPointer->tryStartPointBind()) {
        return false;
    }
    mPrevNerve = mSpine->getCurrentNerve();
    setNerve(&NrvSandyBall::SandyBallNrvBindStarPointer::sInstance);
    return true;
}

// 800FC320
bool SandyBall::tryLand() {
    if (MR::isNoBind(this)) {
        return false;
    }
    if (!MR::isBinded(this)) {
        return false;
    }
    MR::tryHitReaction(this, "\222\205\222n");
    if (mType == 3) {
        setNerve(&NrvSandyBall::SandyBallNrvExplosion::sInstance);
        return true;
    }
    MR::offBind(this);
    MR::invalidateShadow(this, nullptr);
    MR::setBckRate(this, 0.0f);
    mVel.set(mVelocity);
    mStartPos.set(mPosition);
    f32 speed = PSVECMag(&mVel);
    mScaleRate = MR::getSensorRadius(this, "Body") / speed;
    setNerve(&NrvSandyBall::SandyBallNrvLand::sInstance);
    return true;
}

// 800FC420
void SandyBall::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

// 800FC430
void SandyBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pSender, this, "FireBall")) {
        if (mType == 2) {
            if (isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance)) {
                if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender) && (MR::isSensorPlayer(pReceiver) || MR::isSensorMapObj(pReceiver))) {
                    setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (mType == 3 && MR::isEqualSensor(pSender, this, "Explosion")) {
            if (isNerve(&NrvSandyBall::SandyBallNrvExplosion::sInstance)) {
                MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
            }
        } else {
            if (MR::sendMsgEnemyAttackStrong(pReceiver, pSender) && (MR::isSensorPlayer(pReceiver) || MR::isSensorMapObj(pReceiver))) {
                setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                return;
            }
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
    }
}

// 800FC5A0
void SandyBall::setBtpFrame() {
    if (mType == 2) {
        return;
    }
    if (mType == 1) {
        MR::setBtpFrameAndStop(this, 1.0f);
        return;
    }
    if (mType == 3) {
        return;
    }
    MR::setBtpFrameAndStop(this, 0.0f);
}

// 800FC5D0
bool SandyBall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg) && mType == 3) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg) && mType != 3) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        bool isActive = false;
        if (isNerve(&NrvSandyBall::SandyBallNrvWait::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance) ||
            isNerve(&NrvSandyBall::SandyBallNrvLand::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvBindStarPointer::sInstance)) {
            isActive = true;
        }
        if (isActive) {
            if (MR::isMsgFireBallAttack(msg)) {
                setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                return true;
            }
            if (MR::isMsgSupportTicoSpin(msg) && mType != 3) {
                setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                return true;
            }
            if (mType == 2) {
                bool isBreak = MR::isMsgLockOnStarPieceShoot(msg) ? false : !MR::isMsgPlayerJump(msg);
                if (isBreak) {
                    setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
                    return true;
                }
            }
        }
    }
    if (MR::isEqualSensor(pReceiver, this, "FireBall")) {
        bool isActive = false;
        if (isNerve(&NrvSandyBall::SandyBallNrvWait::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance) ||
            isNerve(&NrvSandyBall::SandyBallNrvLand::sInstance) || isNerve(&NrvSandyBall::SandyBallNrvBindStarPointer::sInstance)) {
            isActive = true;
        }
        if (isActive && MR::isMsgFireBallAttack(msg)) {
            setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
            return true;
        }
    }
    return false;
}

// 800FC820
bool SandyBall::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mType == 3) {
        if (MR::isEqualSensor(pReceiver, this, "Explosion")) {
            return false;
        }
    }
    if (MR::isMsgExplosionAttack(msg) && mType != 3 && !isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        return true;
    }
    if (MR::isEqualSensor(pReceiver, this, "Body") && !isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance) &&
        !isNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance)) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        return true;
    }
    return false;
}

// 800FC930
bool SandyBall::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (mType != 3) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Body")) {
        return false;
    }
    const char* pBombName = "\224\232\222e";
    if (!MR::isEqualString(MR::getSensorHost(pSender)->mName, pBombName)) {
        return false;
    }
    bool ret = isNerve(&NrvSandyBall::SandyBallNrvFly::sInstance);
    return ret;
}

// 800FC9E0
// NONMATCHING: negateInternal is expanded inline in the target (single call site per TU).
void SandyBall::initPath(const TVec3f& rFrom, const TVec3f& rTo) {
    TVec3f negGravity;
    negGravity.negate(mGravity);
    mPath->initFromUpVector(rFrom, rTo, negGravity, 650.0f);
    f32 speed;
    if (mType == 3) {
        speed = 24.0f;
    } else {
        speed = 20.0f;
    }
    s32 frame = mPath->getLength(0.0f, 1.0f, 32) / speed;
    s32 result;
    if (frame < 70) {
        result = 70;
    } else {
        result = 180;
        if (frame <= 180) {
            result = frame;
        }
    }
    mFlyFrame = result;
}

// 800FCAB0
void SandyBall::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateHitSensors(this);
        if (mType == 3) {
            MR::showModel(this);
            MR::invalidateHitSensor(this, "Explosion");
        }
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, 0.0f);
    }
    if (MR::isDead(mOwner)) {
        setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
    }
}

// 800FCB50
void SandyBall::exeFly() {
    if (mStep == 0) {
        MR::startAction(this, "SpinX");
        MR::setBckRate(this, 1.0f);
        MR::offBind(this);
        MR::validateShadow(this, nullptr);
    }
    MR::startActionSound(this, "Fly", -1, -1, -1);
    if (!tryStartBindStarPointer()) {
        mStep++;
        f32 rate = MR::clamp((f32)mStep / (f32)mFlyFrame, 0.0f, 1.0f);
        TVec3f pos;
        mPath->calcPosition(&pos, rate);
        TVec3f vel = pos - mPosition;
        MR::setVelocity(this, vel);
        if (mStep == 20) {
            MR::onBind(this);
        }
        if (!tryLand() && mStep >= 180) {
            MR::zeroVelocity(this);
            setNerve(&NrvSandyBall::SandyBallNrvBreak::sInstance);
        }
    }
}

// 800FCCB0
void SandyBall::exeBreak() {
    if (MR::isFirstStep(this)) {
        if (mType == 1) {
            MR::tryHitReaction(this, "\224j\211\363(\203S\201[\203\213\203h)");
        } else {
            MR::tryHitReaction(this, "\224j\211\363");
        }
        MR::invalidateHitSensors(this);
        if (mType == 1 && MR::appearCoinPop(this, mPosition, 1)) {
            mCoinPopped = true;
        }
        kill();
    }
}

// 800FCD60
void SandyBall::exeLand() {
    if ((s32)mScaleRate <= mStep) {
        MR::zeroVelocity(this);
    }
    if (!tryStartBindStarPointer()) {
        if (getWaitSecond() * 60 < mStep) {
            setNerve(&NrvSandyBall::SandyBallNrvRoll::sInstance);
            return;
        }
        mStep++;
    }
}

// 800FCDF0
void SandyBall::exeRoll() {
    if (MR::isFirstStep(this)) {
        TVec3f vel(mGravity);
        vel.scale(3.0f);
        MR::setVelocity(this, vel);
    }
    f32 dist = PSVECDistance(&mStartPos, &mPosition);
    f32 radius = MR::getSensorRadius(this, "Body");
    if (2.0f * radius < dist) {
        MR::zeroVelocity(this);
        kill();
    }
}

// 800FCEB0
void SandyBall::exeExplosion() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReaction(this, "\224\232\224\255");
        MR::invalidateHitSensors(this);
        MR::validateHitSensor(this, "Explosion");
        MR::hideModel(this);
        mSensorRadius = MR::getSensorRadius(this, "Explosion");
        MR::sendSimpleMsgToActor(100, mOwner);
    }
    s32 end = 12;
    MR::setSensorRadius(this, "Explosion", MR::getLinerValue(getNerveStep(), mSensorRadius, 300.0f, end));
    if (MR::isGreaterEqualStep(this, end)) {
        MR::setSensorRadius(this, "Explosion", mSensorRadius);
        MR::showModel(this);
        kill();
    }
}

// 800FCFE0
void SandyBall::exeBindStarPointer() {
    if (MR::updateActorState(this)) {
        if (mPrevNerve == &NrvSandyBall::SandyBallNrvLand::sInstance) {
            mStep = 0;
            MR::setBckRate(this, 0.0f);
        }
        setNerve(mPrevNerve);
        mPrevNerve = nullptr;
    }
}

// 800FD050
SandyBall::~SandyBall() {
}
